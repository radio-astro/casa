from taskinit import *
import numpy
import sys

# Writes out regions above threshold to regionfile+'.box'

def boxit(imagename, regionfile, threshold, maskname, chanrange, polrange, minsize, diag, boxstretch, overwrite):

    casalog.origin('boxit')

    if not(regionfile):
        regionfile = imagename + '.box'
    if not regionfile.endswith('.box'):
        regionfile = regionfile + '.box'

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
    #print 'nx:', nx, 'ny:', ny, 'n2:', n2, 'n3:', n3

    #casa generated images always 4d and in order of [ra, dec, stokes, freq]
    #other images can be in the order of [ra, dec, freq, stokes]
    nms = csys.names()
    #axes=[]
    #for i in xrange(len(nms)):
    #   if nms[i]=='Right Ascension':
    #      axes[0]=i
    #   if nms[i]=='Declination':
    #      axes[1]=i
    #   if nms[i]=='Stokes'
    #      axes[2]=i
    #   if nms[i]=='Frequency':
    #      axes[3]=i

    chmax=n3
    pomax=n2
    chmin=0
    pomin=0
    if len(nms)==4 and nms[3]=='Stokes':
       chmax=n2
       pomax=n3

    if chanrange:
       print chanrange 
       try:
          if str.count(chanrange, '~') == 1:
             ch1=int(str.split(chanrange, '~')[0])
             ch2=int(str.split(chanrange, '~')[1])
          else:
             ch1=int(chanrange)
             ch2=ch1
          if ch1 > ch2 or ch1 < 0 or ch2 > chmax:
             print 'invalid channel range'
             return
          if ch1>chmin:
             chmin = ch1
          if ch2<chmax:
             chmax = ch2
       except:
          print 'bad format for chanrange'
          return
    #print 'chmin', chmin, 'chmax', chmax
          
    if polrange:
       print polrange 
       try:
          if str.count(polrange, '~') == 1:
             po1=int(str.split(polrange, '~')[0])
             po2=int(str.split(polrange, '~')[1])
          else:
             po1=int(polrange)
             po2=po1
          if po1 > po2 or po1 < 0 or po2 > pomax:
             print 'invalid stokes range'
             return
          if po1>pomin:
             pomin = po1
          if po2<pomax:
             pomax = po2
       except:
          print 'bad format for polrange'
          return
    #print 'pomin', pomin, 'pomax', pomax
          
    n1=pomin
    n2=pomax
    n3=chmax
    n4=chmin
    if len(nms)==4 and nms[3]=='Stokes':
       n1=chmin
       n2=chmax
       n3=pomax
       n4=pomin
    #print 'n1:', n1, 'n2:', n2, 'n4:', n4, 'n3:', n3
    
    f = open(regionfile, 'w')
    totregions = 0
    outputmask = []
    if writemask:
        outputmask = ia.getchunk()
        outputmask.fill(False)

    for i3 in xrange(n4, n3):
        for i2 in xrange(n1, n2):
            regions = {}
            boxRecord = {}
            if len(shape)==2:
                mask = fullmask
            if len(shape)==4:
                mask = fullmask[:,:,i2,i3].reshape(nx, ny)
            islands = []
            pos = numpy.unravel_index(mask.argmax(), mask.shape)
            #print pos
            while(mask[pos]):
                # found pixel in new island
                island = {}
                island['box'] = [pos[0], pos[1], pos[0], pos[1]]
                island['npix'] = 1
                find_nearby_island_pixels(island, mask, pos, shape[0], shape[1], diag)
                islands.append(island)
                pos = numpy.unravel_index(mask.argmax(), mask.shape)
            for record in islands:
                box = record['box']
                # check size of box
                if record['npix'] < minsize:
                    continue
                totregions += 1
                # need pixel corners, not pixel centers
                box[0] -= boxstretch
                box[1] -= boxstretch
                box[2] += boxstretch
                box[3] += boxstretch
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
                # note that the toworld() calls are likely very expensive for many boxes. But then 
                # again, the box-finding algorithm itself seems pretty inefficient, but resource
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
                freq = blc["spectral"]['frequency']['refer'] + " "
                freq += quantity_to_string(blc["spectral"]["frequency"]["m0"], "Hz", False)
                outstring += " ['" + freq + "', '" + freq + "']"
                # Stokes blc/trc
                outstring += " ['" + blc["stokes"] + "', '" + trc["stokes"] + "']"
                # add the mask flag
                outstring += " " + str(1)
                f.write(outstring + "\n")
    casalog.post("Wrote " + str(totregions) + " regions to file " + regionfile, 'INFO')
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

def find_nearby_island_pixels(island, mask, pos, xmax, ymax, diag):
    # blank this position so we don't deal with it again
    mask[pos] = False
    xref = pos[0]
    yref = pos[1]
    for x in range(max(xref-1, 0), min(xref+2, xmax)):
        for y in range(max(yref-1, 0), min(yref+2, ymax)):
            if x == xref and y == yref:
                # same position as reference
                continue
            if ( (not diag) and (x-xref != 0) and (y-yref != 0)):
                # diagonal pixels only used if diag is true
                continue
            if mask[x][y]:
                # found another pixel in this island
                island['box'][0] = min(island['box'][0],x)
                island['box'][1] = min(island['box'][1],y)
                island['box'][2] = max(island['box'][2],x)
                island['box'][3] = max(island['box'][3],y)
                island['npix'] += 1
                # look for island pixels next to this one
                find_nearby_island_pixels(island, mask, (x,y), xmax, ymax, diag)
    return


