from taskinit import *
from clean import clean
from cleanhelper import *
from immath import immath
from sys import stdout
from math import ceil
from math import sqrt
import numpy

#####################################################################
# iterative cleaning while automatically selecting new clean regions
#####################################################################

# Some places in code commented by #TMP# indicate temporary fixes that
# should be removed/modified once certain features or bugs in CASA have
# been implemented or fixed.

def autoclean(vis, imagename, field, spw, selectdata, timerange, uvrange,
              antenna, scan, mode, nchan, start, width, interpolation, doconcat,
              psfmode, imagermode, cyclefactor, cyclespeedup, imsize, cell,
              phasecenter, restfreq, stokes, weighting, robust, noise, npixels,
              interactive, mask, modelimage, uvtaper, outertaper, innertaper,
              niter, npercycle, npercycle_speedup, gain, pbcor, minpb,
              clean_threshold, Nrms, eps_maxres, useabsresid, allow_maxres_inc,
              island_rms, diag, peak_rms, gain_threshold, Npeak, shape,
              boxstretch, irregsize):

    casalog.origin('autoclean')

    orig_npercycle = npercycle

    # set up initial mask values if input by user
    inputmask = None
    if(mask):
        casalog.post('Using input mask from user.')
        imCln = imtool.create()
        imset = cleanhelper(imCln, vis)
        imset.defineimages(imsize=imsize, cell=cell, stokes=stokes,
                           mode=mode, spw=spw, nchan=1,
                           start=start,  width=width,
                           restfreq=restfreq, field=field,
                           phasecenter=phasecenter)
        imset.datselweightfilter(field=field, spw=spw, timerange=timerange,
                                 uvrange=uvrange, antenna=antenna, scan=scan,
                                 wgttype=weighting, robust=robust, noise=noise,
                                 npixels=npixels, mosweight=mosweight,
                                 innertaper=innertaper, outertaper=outertaper,
                                 calready=False)
        inputmask = '__temporary.mask'
        imset.makemaskimage(outputmask=inputmask, maskobject=mask)
        imCln.close()
        ia.open(inputmask)
        maskVals = ia.getchunk()
        ia.close()
        ia.removefile(inputmask)

    # begin loop over all channels
    for ichan in xrange(nchan):

        if imagermode!='mfs':
            casalog.post('***** Beginning channel %d of %d' % (ichan, nchan))

        if (mode=='mfs') or (nchan==1):
            thisImage = imagename
            thisStart = start
        else:
            thisImage = imagename + '.channel.' + str(ichan)
            if mode=='channel':
                thisStart = start + ichan*width
            else:
                thisStart = qa.add(start, qa.mul(width, qa.quantity(ichan)))

        # create dirty image from which to choose initial clean regions
        clean(vis=vis, imagename=thisImage, field=field, spw=spw, 
              selectdata=selectdata, timerange=timerange, uvrange=uvrange,
              antenna=antenna, scan=scan, mode=mode, nchan=1, start=thisStart,
              width=width, interpolation=interpolation, psfmode=psfmode,
              imagermode=imagermode, cyclefactor=cyclefactor,
              cyclespeedup=cyclespeedup, imsize=imsize, cell=cell,  
              phasecenter=phasecenter, restfreq=restfreq, stokes=stokes,
              weighting=weighting, robust=robust, noise=noise, npixels=npixels,
              modelimage=modelimage, uvtaper=uvtaper, outertaper=outertaper,
              innertaper=innertaper, niter=0, gain=gain, minpb=minpb)
        casalog.origin('autoclean')

        # make the mask image
        ia_tool = ia.newimagefromimage(infile=thisImage+'.image',
                                       outfile=thisImage+'.mask',
                                       overwrite=True)
        if(mask):
            ia_tool.putchunk(pixels=maskVals)
        else:
            ia_tool.set(pixels=0.0)
        ia_tool.close()

        # keep track of changes in maximum residual over iterations
        maxres_increased = 0
        post_max = 5000
        # iterations variable stores number of clean internal iterations
        iterations = 0
        # new_npercycle: number of minor cycles this iteration (can change)
        new_npercycle = orig_npercycle
        while iterations < niter:
            cleanmore, nregions = \
                       autowindow(imagename=thisImage, Npeak=Npeak,
                                  shape=shape, irregsize=irregsize,
                                  boxstretch=boxstretch, Nrms=Nrms,
                                  island_rms=island_rms, peak_rms=peak_rms,
                                  gain_threshold=gain_threshold,
                                  clean_threshold=clean_threshold, diag=diag,
                                  useabsresid=useabsresid, ichan=ichan) 

            # if first iteration, make sure CLEAN is cleaning
            if not(cleanmore):
                if not(iterations):
                    casalog.post('Input parameters do not induce cleaning of '
                                 'channel %d.' % ichan)
                break

            # did autowindow make new clean regions?
            if(nregions):
                new_npercycle = orig_npercycle
            else:
                casalog.post('Will continue cleaning with current clean regions.')
                new_npercycle = int(round(new_npercycle * npercycle_speedup))

            # maximum absolute residual before next clean iteration
            if (eps_maxres) or (allow_maxres_inc >= 0):
                pre_max = post_max
                
            # niter-iterations is the *total* number of iterations left, and might
            # be less than npercycle.  Use the minimum of (npercycle, niter-iterations)
            casalog.post('Starting clean at iteration %d' % iterations)

            #TMP# CAS-1623: an improvement will be made to CLEAN to allow it to
            # start from an existing residual image.  The call to clean may need
            # to be changed to reflect this.
            clean(vis=vis, imagename=thisImage, field=field, spw=spw, 
                  selectdata=selectdata, timerange=timerange, uvrange=uvrange,
                  antenna=antenna, scan=scan, mode=mode, nchan=1, width=width, 
                  start=thisStart, interpolation=interpolation, psfmode=psfmode,
                  imagermode=imagermode, imsize=imsize,
                  cyclefactor=cyclefactor, cyclespeedup=cyclespeedup,
                  cell=cell, phasecenter=phasecenter, restfreq=restfreq,
                  stokes=stokes, weighting=weighting, robust=robust,
                  noise=noise, npixels=npixels, modelimage=modelimage,
                  uvtaper=uvtaper, outertaper=outertaper, innertaper=innertaper,
                  niter=min(new_npercycle,niter-iterations),
                  mask=thisImage+'.mask',  gain=gain, minpb=minpb, npercycle=niter,
                  interactive=bool(nregions and interactive))
            casalog.origin('autoclean')
            iterations += new_npercycle

            # stop if we've reached maximum number of clean minor cycles
            if iterations >= niter:
                casalog.post('Reached maximum number of CLEAN cycles (%d)' % niter)
                break

            # check maximum residual value
            if (eps_maxres) or (allow_maxres_inc >= 0):
                residualImage = thisImage+'.residual'
                ia.open(residualImage)
                stats = ia.statistics(list=False)
                ia.close()
                if(useabsresid):
                    post_max = max([ stats['max'][0], abs(stats['min'][0]) ])
                else:
                    post_max = stats['max'][0]
                # check fractional change of maximum residual
                if(eps_maxres):
                    if abs(post_max-pre_max)/pre_max < eps_maxres:
                        casalog.post('Maximum residual has changed by less '
                                     'than %.3f; stopping' % eps_maxres)
                        break
                if post_max >= pre_max:
                    maxres_increased += 1
                    casalog.post('Maximum residual has increased.')#, 'INFO1')
                    if maxres_increased > allow_maxres_inc:
                        casalog.post('Maximum residual has increased %d times; '
                                     'stopping.' % maxres_increased)
                        break

    # finished with all channels: concatenate if there are multiple channels
    if (mode!='mfs') and (nchan!=1) and (doconcat):
        concat_images(imagename, '.image', nchan)
        concat_images(imagename, '.mask', nchan)
        concat_images(imagename, '.flux', nchan)
        concat_images(imagename, '.psf', nchan)
        concat_images(imagename, '.model', nchan)
        concat_images(imagename, '.residual', nchan)
        concat_regions(imagename, '.rgn', nchan)
        # The individual .channel. tables are no longer needed
        os.system('rm -rf '+imagename+'.channel.*' )

    if(pbcor):
        # user wants primary beam corrected .image file and .residual file
        ia.open(imagename+'.image')
        ia.calc(imagename+'.image/'+imagename+'.flux')
        ia.open(imagename+'.residual')
        ia.calc(imagename+'.residual/'+imagename+'.flux')
        ia.close()


# Autowindow selects the clean region based on peaks in each island above threshold
def autowindow(imagename='', island_rms=0, gain_threshold=0, peak_rms=0, Nrms=0,
               boxstretch=0, clean_threshold=0, Npeak=None, shape=0,
               irregsize=100, diag=False, useabsresid=False, ichan=0):

    maskImage = imagename+'.mask'
    residualImage = imagename+'.residual'

    # what is the rms of the residual image outside the current clean regions?
    # option: could modify this to get rms by iterative sigma clipping
    ia.open(residualImage)
    rms = ia.statistics(mask=maskImage+'==0',list=False)['rms'][0]
    max_residual = ia.statistics(list=False)['max'][0]
    if(useabsresid):
        min_residual = ia.statistics(list=False)['min'][0]
        if abs(min_residual) > max_residual:
            max_residual = abs(min_residual)
    ia.close()

    # threshold values for selecting clean regions
    threshold = max(peak_rms*rms, max_residual*gain_threshold)
    casalog.post('peak_threshold = %f' % threshold, 'INFO1')
    casalog.post('island_threshold = %f' % (island_rms*rms), 'INFO1')
    casalog.post('max_residual (%.5f) is %.1f times rms (%.5f)' %
                 (max_residual, max_residual/rms, rms))
    # If no units for clean_threshold, assume mJy for consistency with CLEAN.
    # But convert to Jy, because that's what units the images are in.
    clean_threshold = qa.convert(qa.quantity(clean_threshold,'mJy'),'Jy')
    clean_threshold_Jy = qa.getvalue(clean_threshold)

    # if max_res. is below thresholds, no need to continue
    if (max_residual < clean_threshold_Jy) or (max_residual < rms*Nrms):
        if max_residual < clean_threshold_Jy:
            casalog.post('Max_residual is less than clean_threshold.')
        if max_residual < rms*Nrms:
            casalog.post('Max_residual is less than '+str(Nrms)+' times rms.')
        return 0, 0

    # if max_res. below peak threshold, need more cleaning with current regions
    if max_residual < peak_rms*rms:
        casalog.post('Max_residual is less than box threshold for peaks.')
        return 1, 0
        
    # select the new clean regions; put them in the mask
    casalog.post('Selecting clean regions.')
    Nregions = get_islands(imagename, Npeak, island_rms*rms, threshold, shape,
                           boxstretch, irregsize, diag, ichan)
    
    if not(Nregions):
        casalog.post('No new peaks passed selection.')
        return 1, 0

    return 1, Nregions


# Starting with peak in image, find islands: contiguous pixels above threshold.
# Tests peak: bright enough to cleanbox?  Then adds chosen region shape to mask.
# Continues with next peak pixel until Npeak peaks have been found.
# Won't always add Npeak new clean regions if current clean regions still
# contain peaks.  But isolated bright pixels (<2.5*peak_threshold) are ignored.
def get_islands(imagename='', Npeak=3, island_threshold=0, peak_threshold=0,
                shape=0, boxstretch=0, irregsize=100, diag=False, ichan=0):

    maskImage = imagename+'.mask'
    residualImage = imagename+'.residual'

    # types for the recarrays that will store pixel and island info.
    island_dtype = [('box', '4i4'), ('peak_flux', 'f4')]
    pix_dtype = [('x','i4'),('y','i4'),('tmp_mask','f4'),('cln_mask','f4')]

    # Get mask for all current clean regions which we'll call cln_mask
    ia.open(maskImage)
    cln_mask = ia.getregion().squeeze()
    ia.close()
    # Find all pixels above the threshold; make temporary mask: tmp_mask
    ia.open(residualImage)
    tmp_mask = ia.getregion(mask=residualImage+'>'+str(island_threshold),
                            getmask=True).squeeze()
    # pixel values
    pixelVals = ia.getregion().squeeze()
    ia.close()
    # store pixel positions, mask values, clean region status in recarray
    grid = numpy.indices(tmp_mask.shape)
    xyMask = numpy.rec.fromarrays([grid[0], grid[1], tmp_mask, cln_mask],
                                  dtype=pix_dtype)
    nx, ny = tmp_mask.shape

    # keep going until we've found Npeak islands
    # or there are no more pixels above the island_threshold
    # or the peak is less than the peak_threshold
    Nregions = 0
    Nkept = 0
    while Nregions < Npeak:

        if not(xyMask['tmp_mask'].max()):
            # no more pixels above island threshold: we're done
            break

        # find the next peak and its location
        xok, yok = numpy.where(xyMask['tmp_mask'])
        pixok = pixelVals[xok,yok]
        peak = pixok.max()
        
        if peak < peak_threshold:
            # peak is below peak threshold for clean boxing: we're done
            break

        # store location of peak
        peakind = pixok == peak
        xpeak = xok[peakind][0]
        ypeak = yok[peakind][0]
        x = xpeak
        y = ypeak

        # since we've already checked this pixel, set its tmp mask to 0
        xyMask[x,y]['tmp_mask'] = 0
        listPix = [xyMask[x,y]]
        
        # find all above-threshold contiguous pixels of this island
        # python lets us loop over items in a list while adding to the list!
        for pixel in listPix:
            x = pixel['x']
            y = pixel['y']
            # search the pixels surrounding the pixel of interest
            xLook1 = max(0,x-1)      # in case we're at the image edge...
            xLook2 = min(x+2,nx-1)   #            |
            yLook1 = max(0,y-1)      #            |
            yLook2 = min(y+2,ny-1)   #            V
            # add new above-threshold pixels to the list for this island
            if(diag):
                contig_pix = xyMask[xLook1:xLook2, yLook1:yLook2]
                listPix += [pix for pix in contig_pix.ravel() if(pix['tmp_mask'])]
                # since we've already added these pixels, set their tmp mask to 0
                contig_pix['tmp_mask'] = 0
            else:
                contig_pix = []
                contig_pix += xyMask[xLook1:xLook2, y]
                contig_pix += xyMask[x, yLook1:yLook2]
                listPix += [pix for pix in contig_pix if(pix['tmp_mask'])]            
                # since we've already added these pixels, set their tmp mask to 0
                xyMask[xLook1:xLook2, y]['tmp_mask'] = 0
                xyMask[x, yLook1:yLook2]['tmp_mask'] = 0
                                
        if peak < peak_threshold:
            # reject islands with peak < peak_threshold
            continue

        # found all pixels in this island; get bounding box
        islandPix = numpy.rec.fromrecords(listPix, dtype=pix_dtype)
        xmin = islandPix['x'].min()
        xmax = islandPix['x'].max()
        ymin = islandPix['y'].min()
        ymax = islandPix['y'].max()
        
        if (xmin==xmax) or (ymin==ymax):
            # reject islands that are only 1 pixel wide, unless very bright
            if peak < 2.5 * peak_threshold:
                continue

        Nregions += 1  # This island should be in a clean region.

        # Is the peak already in a clean region?
        if(islandPix[0]['cln_mask']):
            casalog.post('Peak of %f ' % peak + 'at pixel ' +
                     str([xpeak,ypeak]) + ' is already in mask.', 'INFO1')
            continue

        # This is a new island for clean boxing.  Prepare to mask!
        Nkept += 1
        newIsland = numpy.array(([xmin, ymin, xmax, ymax], peak),
                                dtype=island_dtype)
        if (irregsize) and (min([xmax-xmin, ymax-ymin]) >= irregsize):
            # user wants clean region to be outline of island
            # for large islands
            mask_island(imagename, newIsland, islandPix)
        else:
            # region will be circle or box
            mask_region(imagename, newIsland, shape, boxstretch, ichan)
    return Nkept


# If user prefers, large islands (size >= irregsize) will be masked 'as-is'
# instead of surrounding them by a box-shaped or circular clean region
def mask_island(imagename='', island=None, pixels=None):
    ia.open(imagename+'.mask')
    mask = ia.getregion()
    mask[pixels['x'], pixels['y']] = 1
    ia.putchunk(mask)
    ia.close()
    casalog.post('Adding irregular region for peak of %f ' %
             island['peak_flux'] + 'inside box ' + str(island['box']))


# Chooses appropriate shape (box or circle) for clean region
def mask_region(imagename='', island=None, shape=0, boxstretch=0, ichan=0):
    
    peak_flux = island['peak_flux']
    box = island['box']

    if shape==2:
        xsize = box[2] - box[0]
        ysize = box[3] - box[1]
        if abs(xsize-ysize) <= 1:
            shape = 0
        else:
            shape = 1

    if(shape):
        add_box(imagename, ichan, box, peak_flux, boxstretch)
    else:
        add_circle(imagename, ichan, box, peak_flux, boxstretch)


# Add a circular region to the mask image
def add_circle(imagename, ichan, box, peak_flux, boxstretch=0):
    #TMP# At some point im.regiontoimagemask should allow circles that
    #     specify a particular channel
    xsize = box[2] - box[0]
    ysize = box[3] - box[1]
    radius = max(sqrt(xsize**2+ysize**2)/2. + boxstretch, 1)
    xcen = numpy.average([box[0], box[2]])
    ycen = numpy.average([box[1], box[3]])
    circle = [radius, xcen, ycen]
    im.regiontoimagemask(mask=imagename+'.mask', circles=circle)
    casalog.post('Adding circle for peak of %f with center (%.1f,%.1f) and '
             'radius %.1f' % (peak_flux, xcen, ycen, radius))


# Add a box region to the mask image; save new region in region file
def add_box(imagename, ichan, box, peak_flux, boxstretch=0):
    box[0:2] -= boxstretch
    box[2:4] += boxstretch
    # in case we used boxstretch < 0 and a one-pixel sized box:
    if box[0] > box[2]:
        box[0] += boxstretch
        box[2] -= boxstretch
    if box[1] > box[3]:
        box[1] += boxstretch
        box[3] -= boxstretch
    im.regiontoimagemask(mask=imagename+'.mask', boxes=box)
    casalog.post('Adding box for peak of %f ' % peak_flux + 'with coordinates '
                 + str(box))
    ia.open(imagename+'.mask')
    csys = ia.coordsys()
    # need pixel corners, not pixel centers, for region file
    blccoord = [box[0]-0.5, box[1]-0.5, 0, ichan]
    trccoord = [box[2]+0.5, box[3]-0.5, 0, ichan]
    blccoord = [pos - 0.5 for pos in box[0:2]]
    trccoord = [pos + 0.5 for pos in box[2:4]]
    blc = ia.toworld(blccoord, 's')['string']
    trc = ia.toworld(trccoord, 's')['string']
    ia.close()
    newregion = rg.wbox(blc=blc, trc=trc, csys=csys.torecord())
    regionfile = imagename+'.rgn'
    if(os.path.exists(regionfile)):
        oldregion = rg.fromfiletorecord(regionfile)
        regions = {'0':oldregion, '1':newregion}
        unionregion = rg.makeunion(regions)
        os.system('rm -f '+regionfile)
    else:
        unionregion = newregion
    rg.tofile(regionfile, unionregion)


# concatenate multiple-channel images
def concat_images(imagename='', suffix='', number=0, relax=False):
    imagelist = [imagename + '.channel.' + i + suffix
                 for i in map(str, range(number))]
    ia.imageconcat(imagename + suffix, imagelist, overwrite=True, relax=relax)
    ia.close()


# read in multiple region (.rgn) files; save to one file
def concat_regions(imagename='', suffix='', number=0):
    regions = {}
    for file in [imagename + '.channel.' + i + suffix
                 for i in map(str, range(number))]:
        if(os.path.exists(file)):
            regions[file] = rg.fromfiletorecord(file)
    nregion = len(regions)
    if(nregion):
        regionfile = imagename + '.rgn'
        if(os.path.exists(regionfile)):
            os.system('rm -f '+regionfile)
        if nregion > 1:
            rg.tofile(regionfile, rg.makeunion(regions))
        else:
            rg.tofile(regionfile, regions[file])
