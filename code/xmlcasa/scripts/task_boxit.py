from taskinit import *
import numpy
import sys

# Writes out regions above threshold to regionfile+'.rgn'

def boxit(imagename, regionfile, threshold, maskname, minsize, diag, boxstretch, overwrite):

    casalog.origin('boxit')

    if not(regionfile):
        regionfile = imagename + '.rgn'

    if not(overwrite):
        if(os.path.exists(regionfile)):
            casalog.post('file "' + regionfile + '" already exists.', 'WARN')
            return
        if(maskname and os.path.exists(maskname)):
            casalog.post('output mask "' + maskname + '" already exists.', 'WARN')
            return

    # If no units, assume mJy for consistency with auto/clean tasks.
    # But convert to Jy, because that's what units the images are in.
    threshold = qa.getvalue(qa.convert(qa.quantity(threshold,'mJy'),'Jy'))
    casalog.post("Setting threshold to " + str(threshold) + "Jy", "INFO")

    newIsland = numpy.zeros(1, dtype=[('box','4i4'),('npix','i4')])
    # Find all pixels above the threshold
    ia.open(imagename)
    fullmask = ia.getregion(mask=imagename+'>'+str(threshold),getmask=True)
    if not(fullmask.max()):
        casalog.post('Maximum flux in image is below threshold.', 'WARN')
        return
    writemask = bool(maskname)
    csys = ia.coordsys()

    shape = fullmask.shape
    nx = shape[0]
    ny = shape[1]
    n2 = n3 = 1
    if len(shape)==3:
        n2 = shape[2]
    if len(shape)==4:
        n2 = shape[2]
        n3 = shape[3]

    f = open(regionfile, 'w')
    totregions = 0
    outputmask = []
    if writemask:
        outputmask = ia.getchunk()
        outputmask.fill(False)
    for i3 in xrange(n3):
        for i2 in xrange(n2):

            regions = {}
            boxRecord = {}
            if len(shape)==2:
                mask = fullmask
            if len(shape)==4:
                mask = fullmask[:,:,i2,i3]
            islandImage = 0-mask  # -1: belongs in an island;  0: below threshold
            # islandImage will become an image with each island's pixels labeled by number
            for x in xrange(nx):
                for y in xrange(ny):
                    if(mask[x,y]):
                        stretch = True # stretch: increase size of island

                        if (y>0) and (mask[x,y-1]):
                            islandImage[x,y] = islandImage[x,y-1]
                            if (x != 0) and (mask[x-1,y]) and \
                                   (islandImage[x,y] != islandImage[x-1,y]):
                                mergeIslands(boxRecord, islandImage,
                                             islandImage[x-1,y],
                                             islandImage[x,y])
                                stretch = False
                            if (diag) and (y != ny-1):
                                if (mask[x-1,y+1]) and \
                                       (islandImage[x,y] != islandImage[x-1,y+1]):
                                    mergeIslands(boxRecord, islandImage,
                                                 islandImage[x-1,y+1],
                                                 islandImage[x,y])
                                    stretch = False

                        elif (diag) and (x>0) and (y>0) and (mask[x-1,y-1]):
                            islandImage[x,y] = islandImage[x-1,y-1]
                            if y == ny-1 :
                                pass
                            else:
                                if (mask[x-1,y+1]) and \
                                       (islandImage[x,y] != islandImage[x-1,y+1]):
                                    mergeIslands(boxRecord, islandImage,
                                                 islandImage[x-1,y+1],
                                                 islandImage[x,y])
                                    stretch = False

                        elif (x>0) and (mask[x-1,y]):
                            islandImage[x,y] = islandImage[x-1,y]

                        elif (diag) and (x>0) and (y<ny-1) and (mask[x-1,y+1]):
                            islandImage[x,y] = islandImage[x-1,y+1]

                        else:
                            # make new island
                            stretch = False
                            if(boxRecord):
                                newID = max(boxRecord.keys()) + 1
                            else:
                                newID = 1
                            islandImage[x,y] = newID
                            newIsland['box'] = x,y,x,y
                            newIsland['npix'] = 1
                            boxRecord[newID] = newIsland.copy()
                        if(stretch):
                            stretchIsland( boxRecord, islandImage[x,y], x, y)
            for record in boxRecord.values():
                box = record['box'][0]
                npix = record['npix'][0]
                # check size of box
                if npix < minsize:
                    continue
                totregions += 1
                # need pixel corners, not pixel centers
                box[0:2] -= boxstretch
                box[2:4] += boxstretch
                # in case we used boxstretch < 0 and a one-pixel sized box:
                if box[0] > box[2]:
                    box[0] += boxstretch
                    box[2] -= boxstretch
                if box[1] > box[3]:
                    box[1] += boxstretch
                    box[3] -= boxstretch
                if writemask:
                    # avoid pixels in boxes that have been stretched beyond the image limits
                    for ii in range(max(0, box[0]), min(box[2], outputmask.shape[0] - 1)):
                        for jj in range(max(0, box[1]), min(box[3], outputmask.shape[1] - 1)):
                            outputmask[ii][jj][i2][i3] = True
                blccoord = [box[0]-0.5, box[1]-0.5, i2, i3]
                trccoord = [box[2]+0.5, box[3]+0.5, i2, i3]
                # Honglin prefers world to pixel coords for viewer handling, but note that
                # the toworld() calls are likely very expensive for many boxes. But then again,
                # the box-finding algorithm itself seems pretty inefficient, but resource
                # constraints only permit a band aid fix at this time.
                blc = ia.toworld(blccoord, 'm')['measure']
                trc = ia.toworld(trccoord, 'm')['measure']
                # RA/Dec reference frame
                outstring = "worldbox " + blc['direction']['refer']
                # RA blc/trc
                outstring += " [" + quantity_to_string(blc["direction"]["m0"], "rad") + ", "
                outstring += quantity_to_string(trc["direction"]["m0"], "rad") + "]"
                # Dec blc/trc
                outstring += " [" + quantity_to_string(blc["direction"]["m1"], "rad") + ", "
                outstring += quantity_to_string(trc["direction"]["m1"], "rad") + "]"
                # frequency blc/trc
                freqref = blc["spectral"]['frequency']['refer']
                outstring += " ['" + freqref + " " + quantity_to_string(blc["spectral"]["frequency"]["m0"], "Hz", False) + "', "
                outstring += "'" + freqref + " " + quantity_to_string(trc["spectral"]["frequency"]["m0"], "Hz", False) + "']"
                # Stokes blc/trc
                outstring += " ['" + blc["stokes"] + "', '" + trc["stokes"] + "']"
                # add the mask flag
                outstring += " " + str(1)
                f.write(outstring + "\n")
    casalog.post("Wrote " + str(totregions) + " regions to file " + regionfile, 'INFO1')
    if writemask:
        ia.fromimage(infile=imagename, outfile=maskname, overwrite=True)
        ia.done()
        ia.open(maskname)
        ia.putchunk(outputmask)
    ia.done()
    f.close()
    return True

# there may be a way to do this with the qa tool, but I cannot figure it out
def quantity_to_string(quantity, unit=None, quotes=True):
    if unit != None:
        quantity = qa.convert(quantity)
    string = str(quantity['value']) + quantity['unit']
    if quotes:
        string = "'" + string + "'"
    return string

# two previously separate islands should have same numID
def mergeIslands(boxRecord, islandImage, islandA, islandB):
    recordA = boxRecord[islandA][0]
    recordB = boxRecord[islandB][0]
    # make new box for combined island
    xmin = min(recordA['box'][0], recordB['box'][0])
    ymin = min(recordA['box'][1], recordB['box'][1])
    xmax = max(recordA['box'][2], recordB['box'][2])
    ymax = max(recordA['box'][3], recordB['box'][3])
    # put new box into island with higher flux
    islandImage[islandImage == islandB] = islandA
    boxRecord[islandA]['box'] = [xmin,ymin,xmax,ymax]
    boxRecord[islandA]['npix'] += boxRecord[islandB]['npix']
    # eliminate island with lower peak flux
    del boxRecord[islandB]

def stretchIsland(boxRecord, islandNum, x, y):
    # stretch box
    record = boxRecord[islandNum]
    record['box'][0][0] = min(record['box'][0][0], x)
    record['box'][0][1] = min(record['box'][0][1], y)
    record['box'][0][2] = max(record['box'][0][2], x)
    record['box'][0][3] = max(record['box'][0][3], y)
    record['npix'][0] += 1


