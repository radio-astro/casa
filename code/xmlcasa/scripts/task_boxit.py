from taskinit import *
import numpy
import sys

# Writes out regions above threshold to regionfile+'.rgn'

def boxit(imagename, regionfile, threshold, minsize, diag, boxstretch, overwrite):

    casalog.origin('boxit')

    if not(regionfile):
        regionfile = imagename
    exten = '.rgn'

    if not(overwrite):
        if(os.path.exists(regionfile+exten)):
            casalog.post('file "'+regionfile+exten+'" already exists.', 'WARN')
            return

    # If no units, assume mJy for consistency with auto/clean tasks.
    # But convert to Jy, because that's what units the images are in.
    threshold = qa.getvalue(qa.convert(qa.quantity(threshold,'mJy'),'Jy'))
    print "Setting threshold to " + str(threshold) + "Jy"

    newIsland = numpy.zeros(1, dtype=[('box','4i4'),('npix','i4')])
    # Find all pixels above the threshold
    ia.open(imagename)
    fullmask = ia.getregion(mask=imagename+'>'+str(threshold),getmask=True)
    if not(fullmask.max()):
        casalog.post('Maximum flux in image is below threshold.', 'WARN')
        return
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

    totregions = 0
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
                blccoord = [box[0]-0.5, box[1]-0.5, i2, i3]
                trccoord = [box[2]+0.5, box[3]+0.5, i2, i3]
                blc = ia.toworld(blccoord, 's')['string']
                trc = ia.toworld(trccoord, 's')['string']
                regions[tuple(box)]= rg.wbox(blc=blc, trc=trc,
                                             csys=csys.torecord())

            # CAS-1666, if this check is not done, catastrophic memory use may occur
            if len(regions) > 200:
                raise ValueError, "Too many regions ("+str(len(regions))+"). Please increase the threshold and run again"
            
            if(len(regions)):
                if len(regions)==1:
                    union = regions.values()[0]
                else:
                    print "computing union of "+str(len(regions))+" regions"
                    union = rg.makeunion(regions)
                polfile = regionfile + '.pol.' + str(i2) + exten
                if(os.path.exists(polfile)):
                    os.system('rm -f ' + polfile)
                rg.tofile(polfile, union)
                casalog.post('Writing out %d region(s) to file %s'
                             % (len(regions), polfile), 'INFO1')

        # combine all polarization files for this frequency into one channel file
        chanfile = regionfile + '.chan.' + str(i3) + exten
        concat_regions(chanfile, regionfile+'.pol.', exten, n2)
        #print 'combining pol files for chan '+str(i3)+' into file '+chanfile

    # combine all frequency channel files into one final output file
    concat_regions(regionfile+exten, regionfile+'.chan.', exten, n3)
    #print 'combining chan files into final output file '+regionfile+exten
    casalog.post('Found %d individual regions.' % totregions)
    print 'Found %d individual regions.' % totregions
    ia.done()
    return

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

# read in multiple region (.rgn) files; save to one file
def concat_regions(outfile='', fileroot='', exten='', number=0):
    regions = {}
    for infile in [fileroot + i + exten for i in map(str, range(number))]:
        if(os.path.exists(infile)):
            regions[infile] = rg.fromfiletorecord(infile)
            os.system('rm -f ' + infile)
    nregion = len(regions)
    if(nregion):
        if(os.path.exists(outfile)):
            os.system('rm -f '+outfile)
        casalog.post('Combining %d file(s) into single file %s' % (nregion, outfile), 'INFO1')
        if nregion > 1:
            rg.tofile(outfile, rg.makeunion(regions))
        else:
            rg.tofile(outfile, regions[infile])
