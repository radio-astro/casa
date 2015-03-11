import os
import shutil
import numpy
import itertools
from casac import casac

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
from .basecleansequence import BaseCleanSequence
from .imagecentrethresholdsequence import ImageCentreThresholdSequence
from .iterativesequence import IterativeSequence
from .iterativesequence2 import IterativeSequence2
from . import cleanbase

from pipeline.hif.heuristics import cleanbox as cbheuristic
from pipeline.hif.heuristics import makeimlist

LOG = infrastructure.get_logger(__name__)


class CleanInputs(cleanbase.CleanBaseInputs):

    def __init__(self, context, output_dir=None, vis=None, imagename=None,
       intent=None, field=None, spw=None, uvrange=None, specmode=None,
       gridmode=None, deconvolver=None, outframe=None, imsize=None, cell=None,
       phasecenter=None, nchan=None, start=None, width=None,
       weighting=None, robust=None, noise=None, npixels=None,
       restoringbeam=None, iter=None, mask=None, niter=None, threshold=None,
       noiseimage=None, hm_masking=None, hm_cleaning=None, tlimit=None,
       masklimit=None, maxncleans=None):

       super(CleanInputs, self ).__init__( context,
           output_dir=output_dir, vis=vis, imagename=imagename, intent=intent,
           field=field, spw=spw, uvrange=uvrange, specmode=specmode, gridmode=gridmode,
           deconvolver=deconvolver, outframe=outframe, imsize=imsize, cell=cell,
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


class Tclean(cleanbase.CleanBase):
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

            # delete any old files with this naming root. One of more
            # of these (don't know which) will interfere with this run.
            LOG.info('deleting %s*.iter*' % inputs.imagename)
            shutil.rmtree('%s*.iter*' % inputs.imagename, ignore_errors=True)
#            os.system('rm -fr %s*.iter*' % inputs.imagename)

            # Get an empirical noise estimate by generating Q or V image.
            #    Assumes presence of XX and YY, or RR and LL
            #    Assumes source is unpolarized
            #    Make code more efficient (use MS XX and YY correlations) directly.
            #    Update / replace  code when sensitity function is working.
            #model_sum, cleaned_rms, non_cleaned_rms, residual_max, \
            #  residual_min, rms2d, image_max = \
            #  self._do_noise_estimate(stokes=inputs.noiseimage)
            #noise_rms = non_cleaned_rms
            #LOG.info('Noise rms estimate from %s image is %s' % 
            #  (inputs.noiseimage, noise_rms))

            # Get a noise estimate from the CASA sensitivity calculator
            noise_rms = self._do_sensitivity()
            LOG.info('Sensitivity estimate from CASA %s' % (noise_rms))

	    # Choose cleaning method.
            if inputs.hm_masking == 'centralquarter':
                if inputs.hm_cleaning == 'manual':
	            threshold = inputs.threshold
                elif inputs.hm_cleaning == 'sensitivity':
                    raise Exception, 'sensitivity threshold not yet implemented'
                elif inputs.hm_cleaning == 'rms':
                    threshold = '%sJy' % (inputs.tlimit * noise_rms)
                sequence_manager = ImageCentreThresholdSequence(
                  gridmode=inputs.gridmode, threshold=threshold)

            elif inputs.hm_masking == 'psfiter':
                sequence_manager = IterativeSequence(
                  maxncleans=inputs.maxncleans)

            elif inputs.hm_masking == 'psfiter2':
                sequence_manager = IterativeSequence2(
                  maxncleans=inputs.maxncleans)
    
            result = self._do_iterative_imaging(
              sequence_manager=sequence_manager, result=result)

	except Exception, e:
            raise Exception, '%s/%s/SpW%s Iterative imaging error: %s' % (
              inputs.intent, inputs.field, inputs.spw, str(e))
	finally:
            pass

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

    def _do_sensitivity(self):
        """Compute sensitivity estimate using CASA."""

        context = self.inputs.context
        inputs = self.inputs
        field = inputs.field
        spw = inputs.spw

        # Effective antenna area
        AgeomPerMS = []
        msNames = []
        for msInfo in context.observing_run.measurement_sets:
            msNames.append(msInfo.name)
            AgeomPerMS.append(reduce(lambda x,y: x+y, [4.0 * numpy.pi * antenna.diameter**2 for antenna in context.observing_run.measurement_sets[0].antennas]))
        AgeomPerMS = numpy.array(AgeomPerMS)

        # Aperture efficiency can not yet be used from CalAmpli table.
        # Assume reasonable fixed value.
        etaA = 0.9
        AeffPerMS = etaA * AgeomPerMS

        # Calculate average Tsys
        caTool = casac.calanalysis()
        tsysTables = list(context.callibrary.applied.get_caltable('tsys'))
        if (tsysTables != []):
            # chain is used to flatten the lists of CalFrom objects (1 per applycal) into a 1-D list
            tsysCalfroms = [cf for cf in itertools.chain(*context.callibrary.applied.merged().values()) if cf.gaintable in tsysTables]
            # TODO: There are two maps one for AMPLITUDE and BANDPASS one for PHASE and TARGET.
            #       They could be different. Need to get only the appropriate one.
            spwMaps = list(set([cf.spwmap for cf in tsysCalfroms]))

            avgTsysPerTable = []
            for i in xrange(len(tsysTables)):
                tsysTable = tsysTables[i]
                spwMap = spwMaps[i]
                caTool.open(tsysTable)
                tsysData = caTool.get(field = field, spw = str(spwMap[int(spw)]))
                if (len(tsysData.keys()) > 0):
                    avgTsysPerPol = []
                    for tsysPerPol in tsysData.itervalues():
                        if (tsysPerPol['flag'].all() == False):
                            avgTsysPerPol.append(numpy.ma.average(numpy.ma.array(tsysPerPol['value'], mask=tsysPerPol['flag'])))
                    if (len(avgTsysPerPol) > 0):
                        avgTsysPerTable.append(numpy.average(avgTsysPerPol))
                    else:
                        avgTsysPerTable.append(1.0)
                        LOG.warning('No Tsys data found. Using Tsys = 1.0 K.')
                caTool.close()
            # TODO: Calculate sensitivity per set of MSs calibrated with
            #       a given Tsys table.
            avgTsys = numpy.average(avgTsysPerTable)
        else:
            LOG.warning('No Tsys tables found. Using Tsys = 1.0 K.')
            avgTsys = 1.0

        # Calculate sensitivities
        sensitivities = []
        imTool = casac.imager()
        # Assume one Tsys table per MS for now
        for i in xrange(len(msNames)):
            imTool.open(msNames[i])
            imTool.selectvis(spw = spw, field = field)
            imTool.defineimage(mode = inputs.specmode, spw = int(spw))
            imTool.weight('natural')
            try:
                result = imTool.sensitivity()
                sensitivities.append(result[1]['value'] * avgTsysPerTable[i] / AeffPerMS[i])
            except Exception as e:
                sensitivities.append(0.1)
                LOG.warning('Exception in calculating sensitivity. Assuming 0.1 Jy/beam.')
            imTool.close()

        sensitivity = numpy.average(sensitivities)

        # Save sensitivity in context dictionary for image QA
        #self.inputs.context.... / results ?

        return sensitivity

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
	    specmode=inputs.specmode,
	    gridmode=inputs.gridmode,
	    deconvolver=inputs.deconvolver,
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

