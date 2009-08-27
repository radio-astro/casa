from taskinit import *
from imregion import *
from odict import odict
from math import *

# lists of units for dynamic selection of units for printing

_imfit_angunits_  = [ "deg    ",
                      "arcmin ",
                      "arcsec ",
                      "marcsec",
                      "uarcsec"]

_imfit_distunits_ = [ "km",
                      "m ",
                      "cm",
                      "mm",
                      "um"]

_imfit_fluxunits_ = [ "MJy",
                      "kJy",
                      "Jy ",
                      "mJy",
                      "uJy"]

_imfit_frequnits_ = [ "THz",
                      "GHz",
                      "MHz",
                      "kHz",
                      "Hz ",
                      "mHz"]

_imfit_timeunits_ = [ "h      ",
                      "min    ",
                      "s      ",
                      "ms     ",
                      "us     "]

def imfit (imagename=None,
           box=None,
           region=None,
           mask=None,
           fixed=None,
           usecleanbeam=None,
           estfile=None,
           residfile=None):
    """
    Fits 2-dimensional Gaussians to regions of the sky in a CASA image.

        imagename -- Input image that continuum subtraction is applied too.
                Default: none; Example: imagename='ngc5921_task.im'
        box --  A box region on the directional plane
                Only pixel values acceptable at this time.
                Default: none (whole 2-D plane); Example: box='10,10,50,50'
        region -- File path to an ImageRegion file.
                An ImageRegion file can be created with the CASA
                viewer's region manager.  Typically ImageRegion files
                will have the suffix '.rgn'.  If a region file is given
                then the box, chans, and stokes selections whill be 
                ignored.
                Default: none; Example: region='myimage.im.rgn'
        mask -- Mask expression or file to be applied to image before fitting
                Default: none;  Example: mask='ngc5921_mask'.  
        fixed -- Select which values to hold fixed and which to solve for. The
                selection string contains letters representing the fixed values.
                A selection string for each Region to fit can be given.
                Default: '';   
                Example: fixed='xf'
                Example: fixed=['x', 'a']
                Options: f - hold the peak flux (intensity) fixed.
                         x - hold the X location of the model fixed.
                         y - hold the U location of the model fixed.
                         a - hold the major axis width fixed
                         b - hold the minor axis width fixed
                         p - hold the position angle fixed.
        usecleanbeam -- Estimate the true source size by deconvolving the
                fitted beam with the clean beam
                Default: False; Example: usecleanbeam=True
        estfile -- File containing an estimate of the fit
                Default: none; Example: estfit='ngc5921_estimate.im'
        estfile -- Output file containing an the residual of the fit
                Default: none; Example: estfit='ngc5921_fit_resid.im'


# Fitting in a region specified in a region file that has been
# created with the CASA viewer.  Do not select the "all axes" nor
# the "all channels" option when saving the region file. This task
# takes 2-D regions on the sky only.  In this example the returned
# component tool is used to display there results.
ct = imfit( imagename='cleanedImage.im', region='cleanedImage.rgn' )
#print "Flux:       ", ct.getflux(0)
#print "Major Axis: ", ct.getshape(0)['majoraxis']
#print "Spectrum:   ", ct.getspectrum(0)


# Fit a box region, which is specified in pixels.  
imfit( imagename='prettyImage', box='23,54,32,76', deconvolve=True )

# Create an estimates of some point sources and use these estimates
# when fitting. We need to first read in the region we are interested
# in from a file that was created with the Viewer.
rg1 = rg.fromfiletorecord( 'myImage-3.rgn')
ia.open( 'myImage')
estimates = ia.findsources( nmax=5, region=rg1, mask='mask("myImage.mask")' )
ut = utilstool.create()
ut.toxml( input=estimates, filename='myImage.est' )
ia.done()
del rg1
default('imfit')
region='myImage-3.rgn'
mask='mask("myImage.mask")'
estfile='myImage.est'
fixed=['f', '', '', 'a', '']
go
    """
    try:
        casalog.origin('imfit')

        # Create the sky region(s) from the information provided by
        # the user.  Note, we are only interested in the "sky" portion
        # of the image, so we don't allow channels and stokes values
        # to be given.
        regions=[]
        if ( isinstance( region, str ) ):
             region=[region]
        if ( len(region)>0 and len(region[0])>1 ):
            if ( len(box)>1 ):
                casalog.post( str( "Ignoring box paramter value, using " )
                              + str(" \nregion information\nin file: ")
                              + region, 'WARN' );
            for i in range( len( region ) ):
                if ( len(region[i]) > 1 ):
                    if os.path.exists( region[i] ):
                        # We have a region file on disk!
                        regions.append( rg.fromfiletorecord( region[i] ) );
                    else:
                        # The name given is the name of a region stored
                        # with the image.
                        # Note that we accept:
                        #    'regionname'          -  assumed to be in imagename
                        #    'my.image:regionname' - in my.image
                        reg_names=region[i].split(':')
                        if ( len( reg_names ) == 1 ):
                            regions.append(rg.fromtabletorecord( imagename, region[i], False ) )
                        else:
                            regions.append( rg.fromtabletorecord( reg_names[0], reg_names[1], False ) )
                else:
                    regions.append({})
        else:
            if (len(box) == 0):
                ia.open(imagename)
                shape = ia.shape()
                ia.close()
                box = "0,0," + str(shape[0] - 1) + "," + str(shape[1] - 1)
            reg=imregion( imagename, '', '', box, '', '', True )
            regions.append(reg)

        # Loop through the list of regions, removing any of the
        # empty ones, and if in the end there are none then
        # throw an exception.
        try:
            while( 1 ):
                idx = regions.index( {} )
                regions.pop( idx )
        except:
            # If we are here we didn't find anymore empty dictionaries
            no_op='noop'
        if ( region!=None and len(region) > 0 and len( regions ) < 1 ):
            raise Exception, 'Ill-formed region: '+str(region)+'. can not continue.' 

        # The number of Gaussian models we'll be fitting must be
        # equivalent, also if the user has told us what to fix then
        # we need to make sure that is the same length as well.
        numFits = len( regions )
        if ( numFits <> 1 ):
            casalog.post( "Sorry! Only fits on a single region are supported", 'WARN' );
            return None

        # Now make sure we have enough models, estimate files, and
        # residual files, as well as "fixed" parameters.
        models=[]
        for i in range( numFits ):
            models.append( 'gaussian' )

        # If the user specified somthing to "fix" make sure we have
        # something for each region.
        if ( isinstance(fixed, str) ):
            fixed=[fixed]
        fixedVals=[]
        if ( len( fixed ) > 1 and numFits > 1 ) :
            if ( numFits != len(fixed) ) :
                casalog.post( str("Error: Number of fits with fixed" )
                              +str( " values must be the same as the" )
                              +str( " number of regions to be fitted." ),
                              'SEVERE' )
                return None
            fixedVals=fixed
        elif ( len( fixed[0] ) > 0 ):
            casalog.post( str("Using " )+str(fixed)
                          +str( " on all regions to be fitted: " ),
                          'NORMAL1' )		    
            for i in range( numFits ):
                fixedVals.append(fixed[0])
    
        if ( isinstance(estfile, str) ):
            estfile=[estfile]
        if ( len( estfile ) > 1 and len(estfile) != numFits ):
            casalog.post( str("Error: Number of estimate files" )
                          +str( " must be the same as the" )
                          +str( " number of regions to be fitted." ),
                          'SEVERE' )
            return None
    
        if ( isinstance(residfile, str) ):
            residfile=[residfile]
        if ( len( residfile ) > 1 and len(residfile) != numFits ):
            casalog.post( str("Error: Number of estimate files" )
                          +str( " must be the same as the" )
                          +str( " number of regions to be fitted." ),
                          'SEVERE' )
            return None		    
        
        retValue=[]
        beams=[]
        
        for regId in range( numFits ):
            # Read in the estimate file(s) if there is any
            estRecord={}
            if ( len(estfile)>regId and len(estfile[regId]) > 1 ):
                utils_in = utilstool.create()
                estRecord = utils_in.torecord( str("file:///")+estfile[i] )

            fixedparams=""
            if ( len(fixedVals)>regId ):
                fixedparams=fixedVals[regId]

            # Open the image, and get the restoring beam info.
            ia.open( imagename );
            
            # Save the image coordinate system
            csys=ia.coordsys();

            # If we've made it here then perform the fit
            # The ia.fitsky() tool function returns the following inf:
            #	  'return' : A component list record which can be used with
            #		     the component list tool.  Returned flux values
            #		     are integral, not peak.
            #	  'pixelmask': mask on the image where the residual values
            #		       are invalid
            #	  'pixels'   : float array containing the residuals after
            #		       subtracting the fitted components.
            #	  'converged': True if converged to a a fit.
            fit = ia.fitsky( region=regions[regId],
                             mask=mask,
                             models=models[regId],
                             estimate=estRecord,
                             fit=True,
                             deconvolve=False,
                             fixedparams=fixedparams )

            if ( not fit['converged'] ):
                casalog.post( "Fit did not converge, no fits found for region" \
                              +str(regId), "SEVERE" )
                ia.close()
                del csys
                retValue.append(None)
                continue

            # return the fit as a component tool.  Note that the only
            # values this component will have is a shape, label, flux,
            # refdir and spectrum
            results = cltool.create()
            if ( not results.fromrecord( fit['return'] ) ):
                casalog.post( "Error: Unable to create component tool for fit information", SEVERE )
                ia.close()
                del fit, csys, results
                retValue.append(None)
                continue
            if ( results.length() < 1 ):
                casalog.post( "Error: component tool is empty for fit information", SEVERE )
                ia.close()
                del fit, csys, results
                retValue.append(None)
                continue

            # Extract beam information from the image
            imageInfo=ia.summary(list=False)['header']
            component = results.getshape(0)
            deconvolved_component = None

            cleanbeam=None
            if ( imageInfo.has_key("restoringbeam") ):
                 cleanbeam=ia.restoringbeam()

            _imfit_print_ ( imagename, regId, cleanbeam, results, component, usecleanbeam )

            # Save residual image
            #print "RESID: ", residfile, "  length ", len( residfile )
            if ( len( residfile ) > regId and len(residfile[regId])>1 ):
                #print "Creating residual file"
                resid=fit['pixels']
                #print "DATA: ", resid
                #ia.newimagefromarray( outfile=residfile[regId], pixels=resid, csys=csys.torecord(), overwrite=True )
                regCsys=coordsystool.create()
                regCsys.fromrecord(regions[regId]['coordinates'])
                #ia.newimagefromarray( outfile=residfile[regId], pixels=resid, retCsys=csys.torecord(), overwrite=True )
                ia.newimagefromarray( outfile=residfile[regId], pixels=resid, overwrite=True )
                #ia.adddegaxes( spectral=True, stokes='Q', overwrite=True )
                ia.close()
                
                #TODO set the mask
                residMask=fit['pixelmask']

                #ia.maskhandler()
                #maskExpr='mask('+
                #ia.calcmask( mask='"'+)
                ia.done()

            retValue.append(results)
            csys.done()
            del csys
            
        if ( len( retValue ) < 2 ):
            return retValue[0]
        else:
            return retValue
    except Exception, instance:
        try:
            ia.done()
            del csys		    
        except Exception :
            noop='noop'
            
        casalog.post( str( '*** Error ***')+str(instance),'SEVERE' )
        return None

    return
        
# INPUTS:
#    imagename - file name of the image we found the fit for.
#    fit       - component tool descrbing a fit with only 1 component

def _imfit_print_ ( imagename, regId, cleanbeam, fit, component, usecleanbeam ):
    # Write to the logger the various bits of information
    # about the fit.
    casalog.post( "Fit on "+imagename+" region "+str(regId), 'NORMAL' );

    ### Position Information

    pos=fit.getrefdir(0)
    posErr=fit.torecord()['component0']['shape']['direction']['error']
    _imfit_posprint_ (pos['m0'],
                      pos['m1'],
                      qa.convert(posErr['longitude'],'rad')['value'],
                      qa.convert(posErr['latitude'],'rad')['value'])

    # The following assumes that positions are in radians and that a
    # single step is sufficient to estimate the error in pixels.

    pixelPos =ia.topixel([pos['m0']['value'],pos['m1']['value']])
    pixelPos2=ia.topixel([pos['m0']['value']+qa.convert(posErr['longitude'],'rad')['value'],
                         pos['m1']['value']+qa.convert(posErr['latitude'],'rad')['value']])
    _imfit_posprint2_ (pixelPos['numeric'][0],
                       pixelPos['numeric'][1],
                       abs(pixelPos['numeric'][0]-pixelPos2['numeric'][0]),
                       abs(pixelPos['numeric'][1]-pixelPos2['numeric'][1]))

    ### Component

    casalog.post ( "Image component size --- ", 'NORMAL' );
        
    _imfit_beamprint_ (component['majoraxis'], 
                       component['minoraxis'],
                       component['positionangle'],
                       component['majoraxiserror'], 
                       component['minoraxiserror'],
                       component['positionangleerror'])

    ### Deconvolved component

    if ( usecleanbeam ):

        pointOn, pointOff, dc = _imfit_deconvolve_component_with_errors_ ( component, cleanbeam )

        if (dc[0]["value"] == 0):
            casalog.post ( "Deconvolution of component failed", 'NORMAL' );
        else:
            casalog.post ( "Deconvolved component size --- ", 'NORMAL' );
            if (pointOn):
                casalog.post ( "                             Unresolved component", 'NORMAL'  )
            elif (pointOff):
                casalog.post ( "                             An unresolved solution is within the error box", 'NORMAL'  )
            _imfit_beamprint_ (*dc)

    ### Clean beam

    if ( cleanbeam != None ):
        casalog.post ( "Clean beam size --- ", 'NORMAL' );
        _imfit_beamprint_ (cleanbeam['major'],
                           cleanbeam['minor'],
                           cleanbeam['positionangle'])

    ### Flux

    # For some reason ia.fitsky() always returns the integrated flux value.
    # We need to calculate the peak flux as well, as follows:
    #    PEAK = clean beam area / fitted beam area * integrated flux.
    #          area of an ellipse = pi * minorRadius * majorRadius
    peakFlux=None
    fluxInfo = fit.torecord()['component0']['flux']

    if ( cleanbeam != None ):
         fittedArea = qa.convert(component['majoraxis'],'arcsec')['value'] \
                    * qa.convert(component['minoraxis'],'arcsec')['value'] 
         cleanArea  = qa.convert(cleanbeam['major'],'arcsec')['value'] \
                    * qa.convert(cleanbeam['minor'],'arcsec')['value']
         peakFlux = fit.getfluxvalue(0)[0] * cleanArea / fittedArea 
         peakFluxError = fluxInfo['error'][0] * cleanArea / fittedArea
    
    casalog.post( "Flux --- ", 'NORMAL' )
    casalog.post( "       --- integrated:       " \
                  +_imfit_flux_(qa.quantity(str(fit.getfluxvalue(0)[0])+fit.getfluxunit(0)),
                                 qa.quantity(str(fluxInfo['error'][0])+fit.getfluxunit(0)) ),
                  'NORMAL' );
    if ( peakFlux != None ):    
        casalog.post( "       --- peak:             " \
                      +_imfit_flux_(qa.quantity(str(peakFlux)+fit.getfluxunit(0)),
                                     qa.quantity(str(peakFluxError)+fit.getfluxunit(0)) ),
                      'NORMAL' )
    casalog.post( "       --- polarization:     "+str(fluxInfo['polarisation']), 'NORMAL' )

    ### Frequency info

    spect = fit.getspectrum(0)['frequency']
    casalog.post( str( "Spectrum --- " ), 'NORMAL' );
    casalog.post( str( "       --- type:             " )+str(spect['type']), 'NORMAL' )
    casalog.post(      "       --- frequency:        " + string.strip(_imfit_freq_(spect['m0']))
                       + " / " + string.strip(_imfit_leng_(qa.convertfreq(spect['m0'],"m"))), 'NORMAL' )
    casalog.post( str( "       --- reference type:   " )+str(spect['refer']), 'NORMAL' )

    return

def _imfit_posprint_ (ra, dec, dra, ddec):

    # ra, dec as angle quanta
    # dra, ddec as radians (float)

    # Form the basic ra/dec strings

    raout  = "  " + qa.angle ( ra,  prec=9, form="tim" )
    decout =        qa.angle ( dec, prec=8 ) + " "
    strdra  = ""
    strddec = ""

    # Now choose an unified error unit for both axes

    if ( dra == 0 and ddec == 0 ):
        delta = 0
    elif ( dra == 0 ):
        delta = abs(ddec)
    elif ( ddec == 0 ):
        delta = abs(dra)
    else:
        delta = sqrt ( dra*dra + ddec*ddec )

    # Add error estimates to ra/dec strings if an error is given (either >0)

    if ( delta <> 0 ):

        draval  = _imfit_round_(qa.convert(qa.quantity(str(dra)+"rad"),"s")['value'])
        ddecval = _imfit_round_(qa.convert(qa.quantity(str(ddec)+"rad"),"arcsec")['value'])
        format, decimals  = _imfit_format_2_(draval,draval,ddecval,ddecval)
        raout  = "  " + qa.angle ( ra,  prec=(6+decimals), form="tim" )
        decout =        qa.angle ( dec, prec=(6+decimals) )
        strdra  = " +/- " + str(format % draval)
        strddec = " +/- " + str(format % ddecval)

        ranges = []
        for x in _imfit_angunits_:
            ranges.append([qa.convert(qa.quantity("1"+x),"rad")["value"], x])
    
        # Find appropriate angle strings (dms)

        for r in ranges:
            if ( delta > r[0] ):
                draval  = dra/r[0]
                ddecval = ddec/r[0]
                format  = _imfit_format_(draval,draval,ddecval,ddecval)
                strdra  += " (" + str(format % draval)  + " " + r[1] + ")"
                strddec += " (" + str(format % ddecval) + " " + r[1] + ")"
                break

    casalog.post ( "Position ---", 'NORMAL' );
    casalog.post ( "       --- ra:  "+raout+strdra, 'NORMAL'  )
    casalog.post ( "       --- dec: "+decout+strddec, 'NORMAL'  )
    
    return

def _imfit_posprint2_ (raPix, decPix, raPixErr, decPixErr):

    # raPix, decPix, raPixErr, decPixErr as floats

    format = _imfit_format_(raPix,abs(raPixErr),decPix,abs(decPixErr))
    strra  = str(format % raPix)
    strdec = str(format % decPix)
    strra  += " +/- " + string.lstrip(str(format % raPixErr)) + " pixels"
    strdec += " +/- " + string.lstrip(str(format % decPixErr)) + " pixels"

    casalog.post ( "       --- ra:  "+strra,  'NORMAL' ) 
    casalog.post ( "       --- dec: "+strdec, 'NORMAL' ) 

    return

def _imfit_beamprint_ (major, minor, posangle,
                        majorErr=None, minorErr=None, posanErr=None,
                        majorMinus=None, minorMinus=None, posanMinus=None):

    # Inputs all as angle quanta

    # First force position angle to be between 0 and 180 deg

    if ( posangle["value"] < 0 ):
        posangle = qa.add ( posangle, qa.quantity(180,"deg"))

    # Find appropriate ranges in arcsec

    ranges = []
    for x in _imfit_angunits_:
        ranges.append([qa.convert(qa.quantity("1"+x),"arcsec")["value"], x])
    
    vmaj = qa.convert(major,"arcsec")['value']
    vmin = qa.convert(minor,"arcsec")['value']
    valmax=max(abs(vmaj),abs(vmin))

    if ( (majorMinus <> None and minorMinus <> None) and
         (majorMinus['value'] > 0 and minorMinus['value'] > 0) ):

        # Formatting as "value + poserr - negerr" for asymmetric errors

        majorPlus = majorErr
        minorPlus = minorErr
        posanPlus = posanErr

        dmajPlus  = qa.convert(majorPlus,"arcsec")['value']
        dminPlus  = qa.convert(minorPlus,"arcsec")['value']
        dmajMinus = qa.convert(majorMinus,"arcsec")['value']
        dminMinus = qa.convert(minorMinus,"arcsec")['value']

        dmaj=min(abs(dmajPlus),abs(dmajMinus))
        dmin=min(abs(dminPlus),abs(dminMinus))

        # Now choose a unified unit and format for both axes and their errors

        for r in ranges:
            if ( valmax > r[0] ):
                format = _imfit_format_(vmaj/r[0],dmaj/r[0],vmin/r[0],dmin/r[0])
                strmaj = str(format % (vmaj/r[0]))
                strmin = str(format % (vmin/r[0]))
                strmaj  += " + " + string.lstrip(str(format % (dmajPlus/r[0])))
                strmaj  += " - " + string.lstrip(str(format % (dmajMinus/r[0]))) + " " + r[1]
                strmin  += " + " + string.lstrip(str(format % (dminPlus/r[0])))
                strmin  += " - " + string.lstrip(str(format % (dminMinus/r[0]))) + " " + r[1]
                break

        pa       = qa.convert(posangle,"deg")['value']
        dpaPlus  = qa.convert(posanPlus,"deg")['value']
        dpaMinus = qa.convert(posanMinus,"deg")['value']
        format = _imfit_format_(pa,dpaPlus,pa,dpaMinus)
        strpos = string.lstrip(str(format % pa))
        strpos += " + " + string.lstrip(str(format % dpaPlus))
        strpos += " - " + string.lstrip(str(format % dpaMinus)) + " deg"

    elif ( (majorErr <> None and minorErr <> None) and
         (majorErr['value'] > 0 and minorErr['value'] > 0) ):

        # Formatting as "value +/- err" for symmetric errors

        dmaj = qa.convert(majorErr,"arcsec")['value']
        dmin = qa.convert(minorErr,"arcsec")['value']

        for r in ranges:
            if ( valmax > r[0] ):
                format = _imfit_format_(vmaj/r[0],dmaj/r[0],vmin/r[0],dmin/r[0])
                strmaj = str(format % (vmaj/r[0]))
                strmin = str(format % (vmin/r[0]))
                strmaj  += " +/- " + string.lstrip(str(format % (dmaj/r[0]))) + " " + r[1]
                strmin  += " +/- " + string.lstrip(str(format % (dmin/r[0]))) + " " + r[1]
                break

        pa  = qa.convert(posangle,"deg")['value']
        dpa = qa.convert(posanErr,"deg")['value']
        format = _imfit_format_(pa,dpa)
        strpos = string.lstrip(str(format % pa ))
        strpos += " +/- " + string.lstrip(str(format % dpa)) + " deg"

    else:
        for r in ranges:
            if ( valmax > r[0] ):
                strmaj = string.lstrip(str("%7.3f" % (vmaj/r[0]))) + " " + r[1]
                strmin = string.lstrip(str("%7.3f" % (vmin/r[0]))) + " " + r[1]
                break
        strpos = string.lstrip(str("%5.2f" % qa.convert(posangle,"deg")['value'])) + " deg"

    casalog.post ( "       --- major axis:       "+strmaj, 'NORMAL'  )
    casalog.post ( "       --- minor axis:       "+strmin, 'NORMAL'  )
    casalog.post ( "       --- position angle:   "+strpos, 'NORMAL'  )

    return

def _imfit_freq_ (fr):

    # fr in quanta

    ranges = []
    for x in _imfit_frequnits_:
        ranges.append([qa.convert(qa.quantity("1"+x),"Hz")["value"], x])
    
    # Find appropriate frequencies

    fr = qa.convert(fr, "Hz")
    frstr = ""

    for r in ranges:
        if ( fr['value'] > r[0] ):
            frstr = str("%7.3f" % (fr['value']/r[0])) + " " + r[1]
            break

    return frstr

def _imfit_leng_ (lgth):

    # lgth in quanta

    ranges = []
    for x in _imfit_distunits_:
        ranges.append([qa.convert(qa.quantity("1"+x),"m")["value"], x])
    
    # Find appropriate distances

    lgth = qa.convert(lgth, "m")

    for r in ranges:
        if ( lgth['value'] > r[0] ):
            lgstr = str("%7.3f" % (lgth['value']/r[0])) + " " + r[1]
            break

    return lgstr

def _imfit_flux_ (flux, fluxerr=None):

    # flux and fluxerr as quanta

    ranges = []
    for x in _imfit_fluxunits_:
        ranges.append([qa.convert(qa.quantity("1"+x),"Jy")["value"], x])
    
    flux = qa.convert (flux, "Jy")
    fv = flux['value']

    if ( fluxerr != None and fluxerr['value'] > 0 ):
        fluxerr = qa.convert (fluxerr, "Jy")
        fe = fluxerr['value']

        for r in ranges:
            if ( fv > r[0] ):
                format = _imfit_format_(fv/r[0],fe/r[0])
                flxstr = str(format % (fv/r[0]))
                flxstr += " +/- " + string.lstrip(str(format % (fe/r[0]))) + " " + r[1]
                break

    else:
        for r in ranges:
            if ( fv > r[0] ):
                flxstr = string.lstrip(str("%7.3f" % (fv/r[0]))) + " " + r[1]
                break

    return flxstr

def _imfit_format_ (*inputs):

    # Develop a standard format string for "value +/- error" printing

    format, after = _imfit_format_2_ (*inputs)

    return format

def _imfit_format_2_ (val1, err1, val2=None, err2=None):

    # Develop a standard format string for "value +/- error" printing
    # Inputs as floats

    # Develop value and error

    if ( val2 != None ):
        sign = 0
        if (val1 < 0 or val2 < 0):
            sign = 1
            val1 = abs(val1)
            val2 = abs(val2)
        value = max(val1,val2)
        if ( abs(err1) == 0 ):
            error = abs(err2)
        elif ( abs(err2) == 0 ):
            error = abs(err1)
        else:
            error = min(abs(err1),abs(err2))
    else:
        sign = 0
        if (val1 < 0):
            sign = 1
            val1 = abs(val1)
        value = val1
        error = abs(err1)

    # Here are a few general safeguards

    # If we are dealing with a value smaller than the estimated error
    # (e.g., 0.6 +/- 12) , the roles in formatting need to be
    # reversed.

    if ( value < error ):
        value = max(value,0.1*error)
        value, error = error, value

    # A value of precisely 0 formats as if it were 1.  If the error is
    # precisely 0, we print to 3 significant digits

    if ( value == 0 ):
        value = 1
    if ( error == 0 ):
        error = 0.1*value

    # Arithmetically we have to draw the limit somewhere.  It is
    # unlikely that numbers (and errors) < 1e-10 are reasonably
    # printed using this limited technique.

    value = max(value,1e-10)
    error = max(error,1e-8)

    # Print only to two significant digits in the error

    error = 0.1*error

    # Generate format

    before = max(int(log10(value))+1,1)		# In case value << 1 ==> log10 < 0
    after = 0
    format="%"+str(sign+before)+".0f"
    if ( log10(error) < 0 ):
        after = int(abs(log10(error)))+1
        format="%"+str(sign+before+1+after)+"."+str(after)+"f"

    return format, after

def _imfit_round_ (number):

    # This rounds a number to 2 or 3 significant digits for printing
    # If number is n>3.2*10**e, 2 digits; if 1<n<3.2*10**e, 3 digits

    if (number >= 0):			# preserve sign, force positive
        sign = 1
    else:
        sign = -1
        number = -number
    
    lgr = log10(number)			# shift number into range 32-320
    if ( lgr >= 0 ):
        i = int(lgr+.5)
    else:
        i = int(lgr-.5)
    temp = number * 10**(2-i)

    return sign*int(temp+.5)*10**(i-2)

def _imfit_deconvolve_component_with_errors_ ( component, beam ):

    # This function forms the deconvolved component and estimates the
    # error in the three parameters (major axis, minor axis, and
    # position angle).  It does the latter by stepping each value of
    # the undeconvolved parameters by the estimated error in those
    # parameters determined by the fit and by forming an array of
    # solutions.

    # Find floats of values for component axes and errors

    majorVal = qa.convert(component["majoraxis"],"rad")["value"]
    minorVal = qa.convert(component["minoraxis"],"rad")["value"]
    paVal    = qa.convert(component["positionangle"],"rad")["value"]
    majorErr = qa.convert(component["majoraxiserror"],"rad")["value"]
    minorErr = qa.convert(component["minoraxiserror"],"rad")["value"]
    paErr    = qa.convert(component["positionangleerror"],"rad")["value"]

    # Extract beam axes information

    majorAxisBeam     = qa.convert(beam["major"],"rad")["value"]
    minorAxisBeam     = qa.convert(beam["minor"],"rad")["value"]
    positionAngleBeam = qa.convert(beam["positionangle"],"rad")["value"]

    beamList = [str("%.15e" % majorAxisBeam)+"rad", \
                str("%.15e" % minorAxisBeam)+"rad", \
                str("%.15e" % positionAngleBeam)+"rad"]

    # First find the central solution and check for point source and "invalid Gaussian" 

    pointOff = False
    sourceList = [str("%.15e" % majorVal)+"rad", \
                  str("%.15e" % minorVal)+"rad", \
                  str("%.15e" % paVal)+"rad"]
    recout = ia.deconvolvefrombeam ( source=sourceList, beam=beamList )

    majorOn_qa = recout["fit"]["major"]
    minorOn_qa = recout["fit"]["minor"]
    paOn_qa    = recout["fit"]["pa"]
    pointOn    = recout["return"]

    majorOn = majorOn_qa["value"]
    minorOn = minorOn_qa["value"]
    paOn    = paOn_qa["value"]

    # If a main solution is not readily available, try setting the
    # component minor axis size to that of the beam minor axis

    if ( majorOn == 0.0 or pointOn ):
        if ( minorVal > minorAxisBeam ):
            # An (as yet) unknown problem
            return pointOn, pointOff, (majorOn_qa, minorOn_qa, paOn_qa)
        else:
            minorVal = minorAxisBeam

    # Try solutions at (value-err), (value), (value+err)

    majInc = [-majorErr,0,majorErr]
    minInc = [-minorErr,0,minorErr]
    paInc  = [-paErr,0,paErr]

    # Form lists of values obtained with the combination of the offsets

    majMinusList = []
    majPlusList = []
    minMinusList = []
    minPlusList = []
    paMinusList = []
    paPlusList = []

    for x in range(3):
        majTry = max(majorVal+majInc[x],0)
        for y in range(3):
            minTry = max(minorVal+minInc[y],0)
            for z in range(3):
                paTry = paVal+paInc[z]
                sourceList = [str("%.15e" % majTry)+"rad", \
                              str("%.15e" % minTry)+"rad", \
                              str("%.15e" % paTry)+"rad"]
                recout = ia.deconvolvefrombeam ( source=sourceList, beam=beamList )

                major = recout["fit"]["major"]["value"]
                minor = recout["fit"]["minor"]["value"]
                pa    = recout["fit"]["pa"]["value"]
                point = recout["return"]

                pointOff = pointOff or point
                if ( major != 0.0 and not point ):
                    if ( x == 0 ):
                        majMinusList.append ( major )
                    if ( x == 2 ):
                        majPlusList.append ( major )
                    if ( y == 0 ):
                        minMinusList.append ( minor )
                    if ( y == 2 ):
                        minPlusList.append ( minor )
                    if ( z == 0 ):
                        paMinusList.append ( pa )
                    if ( z == 2 ):
                        paPlusList.append ( pa )

    # We may need to examine these value lists more closely.  For the
    # moment, we will just take the average of the extrema to estimate
    # the value of the offsets.

    majorMinus = abs( majorOn - _imfit_maxminlist_(majMinusList) )
    majorPlus  = abs( majorOn - _imfit_maxminlist_(majPlusList) )
    minorMinus = abs( minorOn - _imfit_maxminlist_(minMinusList) )
    minorPlus  = abs( minorOn - _imfit_maxminlist_(minPlusList) )
    paMinus    = abs( paOn - _imfit_maxminlist_(paMinusList) )
    paPlus     = abs( paOn - _imfit_maxminlist_(paPlusList) )

    return pointOn, pointOff, \
           (majorOn_qa, minorOn_qa, paOn_qa, \
            qa.quantity(majorPlus,"rad"), \
            qa.quantity(minorPlus,"rad"), \
            qa.quantity(paPlus,"rad"), \
            qa.quantity(majorMinus,"rad"), \
            qa.quantity(minorMinus,"rad"), \
            qa.quantity(paMinus,"rad"))

def _imfit_maxminlist_ ( L ):

    # Return the average between min and max values of a list, zero if
    # empty

    if ( len(L) == 0 ):
        return 0
    else:
        return 0.5*(max(L)+min(L))
