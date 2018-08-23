#
# This file was generated using xslt from its XML file
#
# Copyright 2009, Associated Universities Inc., Washington DC
#
import sys
import os
from  casac import *
import string
from taskinit import casalog
from taskinit import xmlpath
#from taskmanager import tm
import task_hif_editimlist
def hif_editimlist(imagename='', search_radius_arcsec=1000.0, cell=[''], conjbeams=False, cyclefactor='', cycleniter=500, deconvolver='', editmode='', field=[''], imaging_mode='', imsize=[''], intent='', gridder='', mask='', nbin='', nchan='', niter=20000, nterms=2, parameter_file='', phasecenter='', reffreq='', robust=1.0, scales='', specmode='', spw='', start='', stokes='', sensitivity=0.0, threshold='', threshold_nsigma=4.0, uvtaper='', uvrange='', width='', pipelinemode='automatic', dryrun=False, acceptresults=True):

        """Add to a list of images to be produced with hif_makeimages()

            Edit a list of images to be cleaned.

            Keyword Arguments

            
            imagename --

            cell -- Cell size (x, y)
            default '' Compute cell size based on the UV coverage of all the fields
            to be imaged and use a 5 pix per beam sampling.
            The pix per beam specification uses the above default cell size
            ('5ppb') and scales it accordingly.
            example: ['0.5arcsec', '0.5arcsec'] '3ppb'

            conjbeams -- Use conjugate frequency in tclean for wideband A-terms.
            default: False

            cyclefactor -- Controls the depth of clean in minor cycles based on PSF.

            cycleniter -- Controls max number of minor cycle iterations in a single major cycle.

            deconvolver -- Minor cycle algorithm e.g. 'multiscale' or 'mtmfs'.
            default: 'multiscale'

            
            editmode --

            field -- Select fields to image. Use field name(s) NOT id(s). Mosaics
            are assumed to have common source / field names. If intent is specified
            only fields with data matching the intent will be selected. The fields
            will be selected from MeasurementSets in 'vis'.
            default: [''] Fields matching matching intent, one image per target source.
            example: ['3C279'], ['Centaurus*'], ['3C279' ,'J1427-421']

            imsize -- Image X and Y size in pixels or PB level for single field. The
            explicit sizes must be even and divisible by 2,3,5,7 only.
            default: '' The default values are derived as follows:
            1. Determine phase center and spread of field centers around it.
            2. Set the size of the image to cover the spread of field centers plus
            a border of width 0.75 * beam radius, to first null.
            3. Divide X and Y extents by cell size to arrive at the number of
            pixels required.
            The PB level setting for single fields leads to an imsize extending
            to the specified level plus 5% padding in all directions.
            example: [120, 120], '0.3pb'

            intent -- Select intents for which associated fields will be imaged.
            default: 'TARGET'
            example: 'PHASE,BANDPASS'

            
            gridder --

            mask -- Used to declare whether to use a predefined mask for tclean,
            or to use internal auto-masking, or to not mask at all.

            nbin -- Channel binning factors per spw
            default: '' Binning factors for each spw. Format: 'spw1:nb1,spw2:nb2,...'
            Optional wildcard: '*:nb'
            example: '9:2,11:4,13:2,15:8'
            '*:2'

            nchan -- Total number of channels in the output image(s)
            default: -1 Selects enough channels to cover the data selected by
            spw consistent with start and width.
            example: 100

            niter -- The maximum total number of minor cycle iterations allowed for tclean.

            
            nterms --

            
            parameter_file --

            phasecenter -- Direction measure or field id of the image center.
            default: '' The default phase center is set to the mean of the field
            directions of all fields that are to be image together.
            example: 0, 'J2000 19h30m00 -40d00m00'

	    
	    reffreq --


            robust -- Set the Briggs robustness factor for weighting. In general,
            we choose a value that gives suitable PSF size (~2.5? for the
            untapered images) and reasonable sensitivity. Tuning might be needed
            in certain regions.

            scales -- The scales for multi-scale imaging.

            specmode -- Frequency imaging mode, 'mfs', 'cont', 'cube'. \'\' defaults to
            'cube' if intent parameter includes 'TARGET' otherwise 'mfs'.
            default: ''
            example: 'mfs', 'cont', 'cube'
            example:
            mode='mfs' produce one image per source and spw
            mode='cont' produce one image per source and aggregate over all
            specified spws
            mode='cube' produce an LSRK frequency cube, channels are specified
            in frequency

            spw -- Select spectral window/channels to image.
            default: '' Individual images will be computed for all science spectral
            windows.
            example: '9'

            start -- First channel for frequency mode images.
            default '' Starts at first input channel of the spw.
            example: '22.3GHz'

            
            stokes --

            
            threshold --

            uvtaper -- Used to set a uv-taper during clean. At mid-declinations and
            reasonable elevations, not required for the full-resolution images
            and in the QLIP. Needed when making the tapered Images for other BDPs
            (SEIP, CIP). May be useful in QLIP to control PSF at low elevations
            or in hybrid.

            uvrange -- Select a set of uv ranges to image.
            default: '' All uv data is included
            example: '0~1000klambda', ['0~100klambda', 100~1000klambda]

            width -- Output channel width.
            default: '' Difference in frequency between first 2 selected channels.
            for frequency mode images.
            example: '24.2kHz'
            'pilotimage' for 15 MHz / 8 channel heuristic

            --- pipeline task execution modes

            pipelinemode -- The pipeline operating mode. In 'automatic' mode the pipeline
            determines the values of all context defined pipeline inputs automatically.
            In 'interactive' mode the user can set the pipeline context defined
            parameters manually. In 'getinputs' mode the user can check the settings
            of all pipeline parameters without running the task.
            default: 'automatic'.

            dryrun -- Run the commands (True) or generate the commands to be run but
            do not execute (False).
            default: False

            acceptresults -- Add the results of the task to the pipeline context (True) or
            reject them (False).
            default: True

            Output

            results -- If pipeline mode is 'getinputs' then None is returned. Otherwise
            the results object for the pipeline task is returned.

            Description

            Edit a list of images to be cleaned.

            Issues

            

            Examples

            

        
        """
        if type(cell)==str: cell=[cell]
        if type(field)==str: field=[field]
        if type(imsize)==str: imsize=[imsize]

#
#    The following is work around to avoid a bug with current python translation
#
        mytmp = {}

        mytmp['imagename'] = imagename
        mytmp['search_radius_arcsec'] = search_radius_arcsec
        mytmp['cell'] = cell
        mytmp['conjbeams'] = conjbeams
        mytmp['cyclefactor'] = cyclefactor
        mytmp['cycleniter'] = cycleniter
        mytmp['deconvolver'] = deconvolver
        mytmp['editmode'] = editmode
        mytmp['field'] = field
        mytmp['imaging_mode'] = imaging_mode
        mytmp['imsize'] = imsize
        mytmp['intent'] = intent
        mytmp['gridder'] = gridder
        mytmp['mask'] = mask
        mytmp['nbin'] = nbin
        mytmp['nchan'] = nchan
        mytmp['niter'] = niter
        mytmp['nterms'] = nterms
        mytmp['parameter_file'] = parameter_file
        mytmp['phasecenter'] = phasecenter
        mytmp['reffreq'] = reffreq
        mytmp['robust'] = robust
        mytmp['scales'] = scales
        mytmp['specmode'] = specmode
        mytmp['spw'] = spw
        mytmp['start'] = start
        mytmp['stokes'] = stokes
        mytmp['sensitivity'] = sensitivity
        mytmp['threshold'] = threshold
        mytmp['threshold_nsigma'] = threshold_nsigma
        mytmp['uvtaper'] = uvtaper
        mytmp['uvrange'] = uvrange
        mytmp['width'] = width
        mytmp['pipelinemode'] = pipelinemode
        mytmp['dryrun'] = dryrun
        mytmp['acceptresults'] = acceptresults
	pathname="file:///Users/ksugimot/devel/eclipsedev/pipeline-trunk/pipeline/hif/cli/"
	trec = casac.utils().torecord(pathname+'hif_editimlist.xml')

        casalog.origin('hif_editimlist')
        if trec.has_key('hif_editimlist') and casac.utils().verify(mytmp, trec['hif_editimlist']) :
	    result = task_hif_editimlist.hif_editimlist(imagename, search_radius_arcsec, cell, conjbeams, cyclefactor, cycleniter, deconvolver, editmode, field, imaging_mode, imsize, intent, gridder, mask, nbin, nchan, niter, nterms, parameter_file, phasecenter, reffreq, robust, scales, specmode, spw, start, stokes, sensitivity, threshold, threshold_nsigma, uvtaper, uvrange, width, pipelinemode, dryrun, acceptresults)

	else :
	  result = False
        return result
