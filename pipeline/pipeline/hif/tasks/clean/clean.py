import os
import shutil

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
from .basecleansequence import BaseCleanSequence
from .imagecentrethresholdsequence import ImageCentreThresholdSequence
from .iterativesequence import IterativeSequence
from . import cleanbase

from pipeline.hif.heuristics import cleanbox as cbheuristic
from pipeline.hif.heuristics import makecleanlist

LOG = infrastructure.get_logger(__name__)


class CleanInputs(cleanbase.CleanBaseInputs):

    def __init__(self, context, output_dir=None, vis=None, imagename=None,
       intent=None, field=None, spw=None, uvrange=None, mode=None,
       imagermode=None, outframe=None, imsize=None, cell=None,
       phasecenter=None, nchan=None, start=None, width=None,
       weighting=None, robust=None, noise=None, npixels=None,
       restoringbeam=None, iter=None, mask=None, niter=None, threshold=None,
       noiseimage=None, hm_masking=None, hm_cleaning=None, tlimit=None,
       masklimit=None, maxncleans=None):

       super(CleanInputs, self ).__init__( context,
           output_dir=output_dir, vis=vis, imagename=imagename, intent=intent,
           field=field, spw=spw, uvrange=uvrange, mode=mode,
           imagermode=imagermode, outframe=outframe, imsize=imsize, cell=cell,
           phasecenter=phasecenter, nchan=nchan, start=start, width=width,
           weighting=weighting, robust=robust, noise=noise, npixels=npixels,
           restoringbeam=restoringbeam, iter=iter, mask=mask, niter=niter,
           threshold=threshold, result=None)

       # Define addon properties here

       self.noiseimage = noiseimage
       self.hm_masking = hm_masking
       self.hm_cleaning = hm_cleaning
       self.tlimit = tlimit
       self.masklimit = masklimit
       self.maxncleans = maxncleans

    # Add extra getters and setters here

    @property
    def noiseimage(self):
        if self._noiseimage is None:
            ms = self.context.observing_run.get_ms(name=self.vis[0])
            observatory=ms.antenna_array.name
            if 'VLA' in observatory:
                return 'V'
            else:
                return 'Q'
        return self._noiseimage

    @noiseimage.setter
    def noiseimage(self, value):
         self._noiseimage = value

    @property
    def hm_masking(self):
        if self._hm_masking is None:
            return 'centralquarter'
        return self._hm_masking

    @hm_masking.setter
    def hm_masking(self, value):
         self._hm_masking = value

    @property
    def hm_cleaning(self):
        if self._hm_cleaning is None:
            return 'rms'
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
            return 10
        return 10
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
	    result = None
            if inputs.imagermode == 'mosaic':
                # Remove rows in POINTING table - bug workaround.
                #    May no longer be necesssary
                self._empty_pointing_table()
	        LOG.info('Temporarily remove pointing table')

            # delete any old files with this naming root. One of more
            # of these (don't know which) will interfere with this run.
            LOG.info('deleting %s*.iter*' % inputs.imagename)
            os.system('rm -fr %s*.iter*' % inputs.imagename)

            # Get an empirical noise estimate by generating Q or V image.
            #    Assumes presence of XX and YY, or RR and LL
            #    Assumes source is unpolarized
            #    Make code more efficient (use MS XX and YY correlations) directly.
            #    Update / replace  code when sensitity function is working.
            model_sum, cleaned_rms, non_cleaned_rms, residual_max, \
              residual_min, rms2d, image_max = \
              self._do_noise_estimate(stokes=inputs.noiseimage)
            noise_rms = non_cleaned_rms
            LOG.info('Noise rms estimate from %s image is %s' % 
              (inputs.noiseimage, noise_rms))

	    # Choose cleaning method.
            if inputs.hm_masking == 'centralquarter':
                if inputs.hm_cleaning == 'manual':
	            threshold = inputs.threshold
                elif inputs.hm_cleaning == 'sensitivity':
                    raise Exception, 'sensitivity threshold not yet implemented'
                elif inputs.hm_cleaning == 'rms':
                    threshold = '%sJy' % (inputs.tlimit * noise_rms)
                sequence_manager = ImageCentreThresholdSequence(
                  imagermode=inputs.imagermode, threshold=threshold)

            elif inputs.hm_masking == 'psfiter':
                sequence_manager = IterativeSequence(
                  maxncleans=inputs.maxncleans)
    
            result = self._do_iterative_imaging(
              sequence_manager=sequence_manager, result=result)

	except Exception, e:
            raise Exception, '%s/%s/SpW%s Iterative imaging error: %s' % (
              inputs.intent, inputs.field, inputs.spw, str(e))
	finally:
            if inputs.imagermode == 'mosaic':
                # restore POINTING table to input state
                self._restore_pointing_table()
                LOG.info('Restored pointing table')

	return result

    def analyse(self, result):
        return result

    def _do_iterative_imaging (self, sequence_manager, result):
        inputs = self.inputs

        # Compute the dirty image.
        LOG.info('Compute the dirty image')
        iter = 0
        result = self._do_clean (iter=iter, stokes='I', cleanmask='',
          niter=0, threshold='0.0mJy', result=None) 

	# Give the result to the sequence_manager for analysis
	model_sum, cleaned_rms, non_cleaned_rms, residual_max, \
	  residual_min, rms2d, image_max = \
	  sequence_manager.iteration_result(iter=0,
	  psf=result.psf, model= result.model,
	  restored=result.image,
	  residual=result.residual, flux=result.flux,
          cleanmask=None, threshold=None)

        LOG.info('Dirty image stats')
	LOG.info('    Rms %s', non_cleaned_rms)
	LOG.info('    Residual max %s', residual_max)
	LOG.info('    Residual min %s', residual_min)

	iterating = True
        iter = 1
	while iterating:

            # Create the name of the next clean mask from the root of the 
            # previous residual image.
	    rootname, ext = os.path.splitext(result.residual)
	    rootname, ext = os.path.splitext(rootname)
	    new_cleanmask = '%s.iter%s.cleanmask' % (rootname, iter)
	    try:
	        shutil.rmtree (new_cleanmask)
	    except:
                pass

	    # perform an iteration.
	    seq_result = sequence_manager.iteration(new_cleanmask)

	    # Check the iteration status.
	    if not seq_result.iterating:
	        break

	    # Determine the cleaning threshold
            threshold = seq_result.threshold

	    LOG.info('Iteration %s: Clean control parameters' % iter)
	    LOG.info('    Mask %s', new_cleanmask)
	    LOG.info('    Threshold %s', seq_result.threshold)
	    LOG.info('    Niter %s', seq_result.niter)

	    result = self._do_clean (iter=iter, stokes='I',
              cleanmask=new_cleanmask, niter=seq_result.niter, 
              threshold=threshold, result=result) 

            # Give the result to the clean 'sequencer'
	    model_sum, cleaned_rms, non_cleaned_rms, residual_max, \
	      residual_min, rms2d, image_max = \
	      sequence_manager.iteration_result(iter=iter, \
	      psf=result.psf, model= result.model, restored=result.image, residual= \
	      result.residual, flux=result.flux, cleanmask=new_cleanmask, \
	      threshold=seq_result.threshold)

	    LOG.info('Clean image iter %s stats' % iter)
	    LOG.info('    Clean rms %s', cleaned_rms)
	    LOG.info('    Nonclean rms %s', non_cleaned_rms)
	    LOG.info('    Residual max %s', residual_max)
	    LOG.info('    Residual min %s', residual_min)

	    # Up the iteration counter
	    iter += 1

        return result

    def _do_noise_estimate (self, stokes):
        """Compute a noise estimate from the specified stokes image.
        """
        # Compute the dirty Q or V image.
	try:
            LOG.info("Compute the 'noise' image")
            result = self._do_clean (iter=0, stokes=stokes, 
              cleanmask='', niter=0, threshold='0.0mJy', result=None)
	    if result.empty():
                raise Exception, '%s/%s/SpW%s Error creating Stokes %s noise image' % (
                  inputs.intent, inputs.field, inputs.spw, stokes)
	except Exception, e:
            raise Exception, '%s/%s/SpW%s Error creating Stokes %s noise image: %s' % (
              inputs.intent, inputs.field, inputs.spw, stokes, str(e))

	# Create the base sequence manager and use it to get noise stats
	sequence_manager = BaseCleanSequence()
        model_sum, cleaned_rms, non_cleaned_rms, residual_max, \
          residual_min, rms2d, image_max = \
          sequence_manager.iteration_result(iter=0, \
          psf=result.psf, model= result.model, restored=result.image, \
          residual=result.residual, flux=result.flux, cleanmask=None)

        LOG.info('Noise image stats')
        LOG.info('    Rms %s', non_cleaned_rms)
        LOG.info('    Residual max %s', residual_max)
        LOG.info('    Residual min %s', residual_min)

        return model_sum, cleaned_rms, non_cleaned_rms, residual_max, \
          residual_min, rms2d, image_max 

    def _do_clean(self, iter, stokes, cleanmask, niter, threshold, result):
        """Do basic cleaning.
        """

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

	return self._executor.execute(clean_task)

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

