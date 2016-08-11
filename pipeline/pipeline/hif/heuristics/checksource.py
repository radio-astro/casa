import collections

import pipeline.infrastructure.casatools as casatools

def checkimage (imagename, refdirection, refflux):

    '''
       imagename - The image of the source
    refdirection - The reference direction measure for the source that is imaged
         refflux - The reference flux quanta for the source that is imaged
    '''

    # Get the fit dictionary
    fitdict = fitimage(imagename)
    if not fitdict:
        return {}

    me = casatools.measures
    qa = casatools.quanta

    # Compare the reference direction to the fitted
    # direction. Compute the angular separation in
    # arcseconds and the ratio of the separation to
    # the estimate of the synthesized beams

    positionoffset = qa.convert (me.separation (refdirection, fitdict['fitdirection']), 'arcsec')
    beamoffset = qa.convert(qa.div (positionoffset, fitdict['syntheticbeam']), '')
    fluxloss = qa.convert(qa.div(qa.abs(qa.sub(refflux, fitdict['fitflux'])), refflux), '')
    fitdict['refdirection'] = refdirection
    fitdict['positionoffset'] = positionoffset
    fitdict['beamoffset'] = beamoffset
    fitdict['refflux'] = refflux
    fitdict['fluxloss'] = fluxloss

    return fitdict

def fitimage (imagename, fitradius=15):

    '''
       imagename - The image of the source
          radius - The radius of a circular region around the center of the imaged
                   used to computed the fit paramaters and image statistics
    '''

    # Open the image analysis tool
    with casatools.ImageReader(imagename) as image: 

        # Construct the regions string
        #    This is a circle centered at the center of the image with a radius of 15 pixels
        imshape=image.shape()
        region='circle[[%dpix , %dpix], %dpix ]' % (imshape[0] / 2, imshape[1] / 2, fitradius)

        # Get synthesized beam
        #    There are beam parameters in the imfit results as well. It is not clear if these
        #    are the same or not so keep this log in place. 
        restoring_beam = image.restoringbeam()
        if 'beams' in restoring_beam:
            restoring_beam = restoring_beam['beams']['*0']['*0']

        # Get the image peak
        imstatresults = image.statistics(region=region)
        imagepeak = imstatresults['max'][0]

        # Fit the source
        fitresults = image.fitcomponents(region=region)

    # Check that there is a restoring beam
    if not restoring_beam:
        return {}

    # Check validity of fit
    if not fitresults:
        return {}

    fitdict = collections.OrderedDict() 

    qa = casatools.quanta
    me = casatools.measures

    # Get the fitted position
    ra  = fitresults['results']['component0']['shape']['direction']['m0']
    dec = fitresults['results']['component0']['shape']['direction']['m1']
    refer = fitresults['results']['component0']['shape']['direction']['refer']

    # Get the beam
    #    Use instead of image value
    #    Can there be multiple beams ?
    restoring_beam =  fitresults['results']['component0']['beam']['beamarcsec']

    # Get the flux
    flux  = fitresults['results']['component0']['flux']['value']
    fluxunit  = fitresults['results']['component0']['flux']['unit']

    # Get the fitted peak
    peak  = fitresults['results']['component0']['peak']['value']
    peakunit  = fitresults['results']['component0']['peak']['unit']

    # Construct a proper direction measure
    fitdirection = me.direction(refer, qa.quantity(qa.getvalue(ra), qa.getunit(ra)),
        qa.quantity(qa.getvalue(dec), qa.getunit(dec))) 
    fitdict['fitdirection'] = fitdirection

    # Construct the synthesized beam estimate
    synthetic_beam = qa.convert(qa.sqrt(qa.mul(qa.convert(restoring_beam['major'], 'arcsec'),
        qa.convert(restoring_beam['minor'], 'arcsec'))), 'arcsec')
    fitdict['syntheticbeam'] = synthetic_beam

    # Get the fitted flux
    fitflux = qa.quantity (flux[0], fluxunit)
    fitdict['fitflux'] = fitflux

    # Get the peak flux
    fitpeak = qa.quantity (peak, peakunit)
    fitdict['fitpeak'] = fitpeak

    # The image peak
    fitdict['imagepeak'] = qa.quantity (imagepeak, 'Jy/beam')

    # Return the fit dictionary
    return fitdict
