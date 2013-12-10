import os
import shutil

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
from .basicboxworker import BasicBoxWorker
from .iterboxworker import IterBoxWorker
from . import cleanbase

from pipeline.hif.heuristics import cleanbox as cbheuristic

LOG = infrastructure.get_logger(__name__)


class CleanInputs(cleanbase.CleanBaseInputs):


    def __init__(self, context, output_dir=None, vis=None, imagename=None,
       intent=None, field=None, spw=None, uvrange=None, mode=None,
       imagermode=None, outframe=None, imsize=None, cell=None,
       phasecenter=None, nchan=None, start=None, width=None,
       weighting=None, robust=None, noise=None, npixels=None,
       restoringbeam=None, iter=None, mask=None, niter=None, threshold=None,
       hm_masking=None, hm_cleaning=None, tlimit=None, masklimit=None,
       maxncleans=None):

       super(CleanInputs, self ).__init__( context,
           output_dir=output_dir, vis=vis, imagename=imagename, intent=intent,
           field=field, spw=spw, uvrange=uvrange, mode=mode,
           imagermode=imagermode, outframe=outframe, imsize=imsize, cell=cell,
           phasecenter=phasecenter, nchan=nchan, start=start, width=width,
           weighting=weighting, robust=robust, noise=noise, npixels=npixels,
           restoringbeam=restoringbeam, iter=iter, mask=mask, niter=niter,
           threshold=threshold, result=None)

       # Define addon properties here

       self.hm_masking = hm_masking
       self.hm_cleaning = hm_cleaning
       self.tlimit = tlimit
       self.masklimit = masklimit
       self.maxncleans = maxncleans

    # Add extra getters and setters here

    @property
    def hm_masking(self):
        if self._hm_masking is None:
            return 'none'
        return self._hm_masking

    @hm_masking.setter
    def hm_masking(self, value):
         self._hm_masking = value

    @property
    def hm_cleaning(self):
        if self._hm_cleaning is None:
            return 'psf'
        return self._hm_cleaning

    @hm_cleaning.setter
    def hm_cleaning(self, value):
         self._hm_cleaning = value

    @property
    def tlimit(self):
        if self._tlimit is None:
            return 2.0
        return self._tlimit

    @tlimit.setter
    def tlimit(self, value):
         self._tlimit = value

    @property
    def masklimit(self):
        if self._masklimit is None:
            return 2.0
        return self._masklimit

    @masklimit.setter
    def masklimit(self, value):
         self._masklimit = value

    @property
    def maxncleans(self):
        if self._maxncleans is None:
            return 1
        return self._maxncleans

    @maxncleans.setter
    def maxncleans(self, value):
         self._maxncleans = value


class Clean(cleanbase.CleanBase):
    Inputs = CleanInputs

    def is_multi_vis_task(self):
        return True

    def prepare(self):
        inputs = self.inputs
       
        LOG.info('')
        LOG.info("Cleaning for intent '%s', field %s, spw %s" %
            (inputs.intent, inputs.field, inputs.spw))
        LOG.info('')

	try:

            # Remove rows in POINTING table - bug workaround.
	    #    May no longer be necesssary
	    result = None
	    bestrms = None
            if inputs.imagermode == 'mosaic':
                self._empty_pointing_table()
	    else:
                # Get an empirical noise estimate by generating Q image.
		#    This heuristics is ALMA specific
	        #    Assumes presence of XX and YY correlations
	        #    Assumes source is unpolarized
	        #    Make code more efficient (use MS XX and YY correlations) directly.
	        #    Update / replace  code when sensitity function is working.
	        model_sum, cleaned_rms, non_cleaned_rms, residual_max, \
	            residual_min, rms2d, image_max = \
		    self._do_noise_estimate (iter=0)
	        bestrms = non_cleaned_rms
	        LOG.info('Best rms estimate from Q image is %s' % bestrms)
	    LOG.info('Best rms estimate for cleaning is %s' % bestrms)

            # Compute the dirty image.
	    result = self._do_clean (iter=0, stokes='I', cleanmask='', niter=0,
	        threshold='0.0mJy', result=None) 
	    if result.empty():
	        return result

	    # Return dirty image if cleaning is disabled.
            if inputs.maxncleans == 0 or inputs.niter <= 0:
	        return result

	    # Choose between simple cleaning and iterative cleaning.
	    #   More code cleanup needed here.
	    #   Not good if bestrms not properly defined but ...
	    if inputs.hm_masking == 'psfiter':
	        result = self._do_iterative_imaging(bestrms=bestrms, result=result)
	    else:
	        result = self._do_simple_imaging(bestrms=bestrms, result=result)

	except Exception, e:
	    LOG.error('Iterative imaging error: %s' % (str(e)))
	finally:
            if inputs.imagermode == 'mosaic':
                # restore POINTING table to input state
                self._restore_pointing_table()

	return result

    def analyse(self, result):
        return result

    def _do_iterative_imaging (self, bestrms, result):
        inputs = self.inputs

	try:

	    # Create the box worker.
	    boxworker = IterBoxWorker(maxncleans=inputs.maxncleans)

	    # Determine iteration status
	    model_sum, cleaned_rms, non_cleaned_rms, residual_max, \
	        residual_min, rms2d, image_max = \
	        boxworker.iteration_result(iter=0, \
	        psf=result.psf, model= result.model, \
	        restored=result.image, \
	        residual=result.residual, fluxscale=result.flux, \
	        cleanmask=None, threshold=None)

	    LOG.info('Dirty image stats')
	    LOG.info('    Rms %s', non_cleaned_rms)
	    LOG.info('    Residual max %s', residual_max)
	    LOG.info('    Residual min %s', residual_min)
	    if not bestrms:
	        LOG.info('Rms estimate from dirty image is %s' * bestrms)
	        LOG.info('    This is a ver poor estimage')
	        bestrms = non_cleaned_rms

	    iterating = True; iter = 1
	    while iterating and iter <= inputs.maxncleans:

		# Create the clean mask from the root of the previous
		# residual image.
		rootname, ext = os.path.splitext(result.residual)
		rootname, ext = os.path.splitext(rootname)
	        new_cleanmask = '%s.iter%s.cleanmask' % (rootname, iter)
		try:
		    shutil.rmtree (new_cleanmask)
		except:
		    pass
		boxworker.new_cleanmask(new_cleanmask)

		# perform an interation.
		box_result = boxworker.iteration()

		# Check the iteration status.
		iterating = boxworker.iterating()
		if not iterating:
		    break

		# Determine the cleaning threshold
		#    Note this is different than the masking threshold
		threshold = self._do_threshold(iter=iter, bestrms=bestrms,
		    hm_cleaning=inputs.hm_cleaning)

	        LOG.info('Clean control parameters')
	        LOG.info('    Mask %s', new_cleanmask)
	        LOG.info('    Mask threshold %s', box_result.threshold)
	        LOG.info('    Iter %s', iter)
	        LOG.info('    Clean threshold %s', threshold)
	        #LOG.info('    Threshold %s', box_result.threshold)
	        LOG.info('    Niter %s', inputs.niter)

		# Clean
		#    First time through use cleaning threshold based on RMS.    
		#    
		if iter == 1:
		    clthreshold = min (box_result.threshold, threshold)
		else:
		    clthreshold = box_result.threshold
	        result = self._do_clean (iter=iter, stokes='I', cleanmask=new_cleanmask,
		    niter=inputs.niter, threshold=clthreshold, result=result) 

	        # Determine iteration status
	        model_sum, cleaned_rms, non_cleaned_rms, residual_max, \
	            residual_min, rms2d, image_max = \
		    boxworker.iteration_result(iter=iter, \
		    psf=result.psf, model= result.model, restored=result.image, residual= \
		    result.residual, fluxscale=result.flux, cleanmask=new_cleanmask, \
		    threshold=box_result.threshold)
		best_rms = cleaned_rms

	        LOG.info('Clean image iter %s stats' % iter)
	        LOG.info('    Clean rms %s', cleaned_rms)
	        LOG.info('    Nonclean rms %s', non_cleaned_rms)
	        LOG.info('    Residual max %s', residual_max)
	        LOG.info('    Residual min %s', residual_min)

		# Up the iteration counter
		iter = iter + 1

        finally:
	    pass

        return result

    # Do non iterative cleaning, i.e. simple cleaning.
    #    Clean iteration hooks have been left in place
    #    but are not used.
    def _do_simple_imaging (self, bestrms, result):
        inputs = self.inputs

	try:

	    # Create the box worker.
	    boxworker = BasicBoxWorker()

	    # Determine iteration status
	    model_sum, cleaned_rms, non_cleaned_rms, residual_max, \
	        residual_min, rms2d, image_max = \
	        boxworker.iteration_result(iter=0, \
	        psf=result.psf, model= result.model, \
	        restored=result.image, \
	        residual=result.residual, fluxscale=result.flux, \
	        cleanmask=None)

	    LOG.info('Dirty image stats')
	    LOG.info('    Rms %s', non_cleaned_rms)
	    LOG.info('    Residual max %s', residual_max)
	    LOG.info('    Residual min %s', residual_min)
	    if not bestrms:
	        LOG.info('Rms estimate from dirty image is %s' % bestrms)
	        bestrms = non_cleaned_rms

	    iterating = True; iter = 1
	    while iterating and iter <= inputs.maxncleans:

		# Create the clean mask from the root of the previous
		# residual image.
		rootname, ext = os.path.splitext(result.residual)
		rootname, ext = os.path.splitext(rootname)
	        new_cleanmask = '%s.iter%s.cleanmask' % (rootname, iter)
		try:
		    shutil.rmtree (new_cleanmask)
		except:
		    pass

		iterating = boxworker.iterating()
		if not iterating:
		    break

		# Create the mask
		#    The niter return is temporary until the controller is
		#    fully refactored
		niter  = self._do_mask(psf=result.psf, image=result.residual, \
                    cleanmask=new_cleanmask, usermask=inputs.mask,
		    hm_mask=inputs.hm_masking)

		# Determine the cleaning threshold
		threshold = self._do_threshold(iter=iter, bestrms=bestrms,
		    hm_cleaning=inputs.hm_cleaning)

	        LOG.info('Clean control parameters')
	        LOG.info('    Mask %s', new_cleanmask)
	        LOG.info('    Iter %s', iter)
	        LOG.info('    Threshold %s', threshold)
	        LOG.info('    Niter %s', niter)

		# Clean
	        result = self._do_clean (iter=iter, stokes='I', cleanmask=new_cleanmask,
		    niter=niter, threshold=threshold, result=result) 

	        # Determine iteration status
	        model_sum, cleaned_rms, non_cleaned_rms, residual_max, \
	            residual_min, rms2d, image_max = \
		    boxworker.iteration_result(iter=iter, \
		    psf=result.psf, model= result.model, restored=result.image, residual= \
		    result.residual, fluxscale=result.flux, cleanmask=new_cleanmask)
		best_rms = cleaned_rms

	        LOG.info('Clean image iter %s stats' % iter)
	        LOG.info('    Clean rms %s', cleaned_rms)
	        LOG.info('    Nonclean rms %s', non_cleaned_rms)
	        LOG.info('    Residual max %s', residual_max)
	        LOG.info('    Residual min %s', residual_min)

		# Up the iteration counter
		iter = iter + 1

        finally:
	    pass

        return result

    # Compute a noise estimage from the Q image
    def _do_noise_estimate (self, iter=0):

        model_sum = None,
        cleaned_rms = None
        non_cleaned_rms = None
        residual_max = None
        residual_min = None
        rms2d = None
        image_max = None

        # Compute the dirty Q image.
	try:
            result = self._do_clean (iter=0, stokes='Q', cleanmask='', niter=0,
                threshold='0.0mJy', result=None) 
	    if result.empty():
                return model_sum, cleaned_rms, non_cleaned_rms, residual_max, \
	            residual_min, rms2d, image_max 
	except Exception, e:
            LOG.error('Error creating stokes Q noise image: %s' % (str(e)))
            return model_sum, cleaned_rms, non_cleaned_rms, residual_max, \
	        residual_min, rms2d, image_max 

	# Create the box worker.
	boxworker = BasicBoxWorker()

        # Determine iteration status
        model_sum, cleaned_rms, non_cleaned_rms, residual_max, \
            residual_min, rms2d, image_max = \
	    boxworker.iteration_result(iter=0, \
	    psf=result.psf, model= result.model, restored=result.image, \
	    residual=result.residual, fluxscale=result.flux, cleanmask=None)

        LOG.info('Noise image stats')
        LOG.info('    Rms %s', non_cleaned_rms)
        LOG.info('    Residual max %s', residual_max)
        LOG.info('    Residual min %s', residual_min)

        return model_sum, cleaned_rms, non_cleaned_rms, residual_max, \
	    residual_min, rms2d, image_max 

    # Create the mask for simple imaging.
    def _do_mask(self, psf, image, cleanmask, usermask, hm_mask='none'):

        inputs = self.inputs
	niter = inputs.niter
	island_threshold = None
	island_peaks = None

	# Make new clean mask
	if hm_mask == 'none':

	    cm  = casatools.image.newimagefromimage( infile=image,
	        outfile=cleanmask, overwrite=True)
	    cm.set(pixels='1')
	    cm.done()

	elif hm_mask == 'centralquarter':

	    cm  = casatools.image.newimagefromimage( infile=image,
	        outfile=cleanmask, overwrite=True)
	    cm.set(pixels='0')
	    shape = cm.shape()
	    rg = casatools.regionmanager
	    region = rg.box([shape[0]/4, shape[1]/4],
	        [shape[0]-shape[0]/4, shape[1]-shape[1]/4])
	    cm.set(pixels='1', region=region)
	    rg.done()
	    cm.done()

	elif hm_mask == 'psf':
	    niter = cbheuristic.niter_and_mask(psf=psf,
	        residual=image, new_mask=cleanmask)

	elif hm_mask == 'manual':
	     if os.path.exists(usermask):
	        cm  = casatools.image.newimagefromimage( infile=usermask,
	              outfile=cleanmask, overwrite=True)
	        cm.done()
	     else:
	        cm  = casatools.image.newimagefromimage( infile=image,
	              outfile=cleanmask, overwrite=True)
	        cm.set(pixels='1')
	        cm.done()
	else:
	    cm  = casatools.image.newimagefromimage( infile=image,
	      outfile=cleanmask, overwrite=True)
	    cm.set(pixels='1')
	    cm.done()

	# temporary untils controller is working.
        return niter

    # Compute clean threshold.
    def _do_threshold (self, iter, bestrms, hm_cleaning='manual'):
        inputs = self.inputs
	if hm_cleaning == 'manual':
	    threshold = inputs.threshold
	elif hm_cleaning == 'sensitivity':
	    # Alias to rms option for now
	    threshold = '%sJy' % (inputs.tlimit * bestrms)
	    #threshold = inputs.threshold
	elif hm_cleaning == 'rms':
	    threshold = '%sJy' % (inputs.tlimit * bestrms)
	elif hm_cleaning == 'timesmask':
	    threshold = '0.0mJy'
	else:
	    threshold = inputs.threshold

	return threshold

    # Do basic cleaning.
    def _do_clean(self, iter, stokes, cleanmask, niter, threshold, result):
        inputs = self.inputs
	clean_inputs = cleanbase.CleanBase.Inputs(inputs.context,
	    output_dir=inputs.output_dir,
	    vis=inputs.vis,
	    imagename=inputs.imagename,
	    intent=inputs.intent,
	    field=inputs.field,
	    spw=inputs.spw,
	    uvrange=inputs.uvrange,
	    mode=inputs.mode,
	    imagermode=inputs.imagermode,
	    outframe=inputs.outframe,
	    imsize=inputs.imsize,
	    cell=inputs.cell,
	    phasecenter=inputs.phasecenter,
	    nchan=inputs.nchan,
	    start=inputs.start,
	    width=inputs.width,
	    stokes=stokes,
	    weighting=inputs.weighting,
	    robust=inputs.robust,
	    noise=inputs.noise,
	    npixels=inputs.npixels,
	    restoringbeam=inputs.restoringbeam,
	    iter=iter,
	    mask=cleanmask,
	    niter=niter,
	    threshold=threshold,
	    result=result)
	clean_task = cleanbase.CleanBase(clean_inputs)

	return self._executor.execute (clean_task)

    # Remove pointing table.
    def _empty_pointing_table(self):
        # Concerned that simply renaming things directly 
        # will corrupt the table cache, so do things using only the
        # table tool.
        for vis in self.inputs.vis:
            with casatools.TableReader(
              '%s/POINTING' % vis, nomodify=False) as table:
                # make a copy of the table
                LOG.debug('Making copy of POINTING table')
                copy = table.copy('%s/POINTING_COPY' % vis, valuecopy=True)
                LOG.debug('Removing all POINTING table rows')
                table.removerows(range(table.nrows()))
                copy.done()    

    # Restore pointing table
    def _restore_pointing_table(self):
        for vis in self.inputs.vis:
            # restore the copy of the POINTING table
            with casatools.TableReader(
              '%s/POINTING_COPY' % vis, nomodify=False) as table:
                LOG.debug('Copying back into POINTING table')
                original = table.copy('%s/POINTING' % vis, valuecopy=True)
                original.done()
