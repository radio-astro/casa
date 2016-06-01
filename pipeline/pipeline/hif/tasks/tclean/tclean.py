import os
import shutil
import glob
import numpy

import pipeline.domain.measures as measures
from pipeline.hif.heuristics import tclean
from pipeline.hif.heuristics import mosaicoverlap
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.mpihelpers as mpihelpers
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.infrastructure.utils as utils
from pipeline.infrastructure import casa_tasks
from .basecleansequence import BaseCleanSequence
from .imagecentrethresholdsequence import ImageCentreThresholdSequence
from .manualmaskthresholdsequence import ManualMaskThresholdSequence
from .iterativesequence import IterativeSequence
from .iterativesequence2 import IterativeSequence2
from . import cleanbase

from pipeline.hif.heuristics import makeimlist

LOG = infrastructure.get_logger(__name__)


class TcleanInputs(cleanbase.CleanBaseInputs):
    def __init__(self, context, output_dir=None, vis=None, imagename=None,
                 intent=None, field=None, spw=None, spwsel=None, uvrange=None, specmode=None,
                 gridder=None, deconvolver=None, outframe=None, imsize=None, cell=None,
                 phasecenter=None, nchan=None, start=None, width=None,
                 weighting=None, robust=None, noise=None, npixels=None,
                 restoringbeam=None, iter=None, mask=None, niter=None, threshold=None,
                 noiseimage=None, hm_masking=None, hm_cleaning=None, tlimit=None,
                 masklimit=None, maxncleans=None, subcontms=None, parallel=None):
        self._init_properties(vars())
        self.heuristics = tclean.TcleanHeuristics(self.context, self.vis, self.spw)

    # Add extra getters and setters here
    spwsel = basetask.property_with_default('spwsel', {})
    hm_cleaning = basetask.property_with_default('hm_cleaning', 'rms')
    hm_masking = basetask.property_with_default('hm_masking', 'centralregion')
    masklimit = basetask.property_with_default('masklimit', 4.0)
    tlimit = basetask.property_with_default('tlimit', 4.0)
    subcontms = basetask.property_with_default('subcontms', False)

    @property
    def imagename(self):
        return self._imagename

    @imagename.setter
    def imagename(self, value):
        if (value is None):
            self._imagename = ''
        else:
            self._imagename = value.replace('STAGENUMBER', str(self.context.stage))

    @property
    def noiseimage(self):
        return self._noiseimage

    @noiseimage.setter
    def noiseimage(self, value):
        if value is None:
            ms = self.context.observing_run.get_ms(name=self.vis[0])
            observatory = ms.antenna_array.name
            if 'VLA' in observatory:
                value = 'V'
            else:
                value = 'Q'
        self._noiseimage = value

    @property
    def maxncleans(self):
        if self._maxncleans is None:
            return 10
        return 10
        return self._maxncleans

    @maxncleans.setter
    def maxncleans(self, value):
        self._maxncleans = value

    @property
    def deconvolver(self):
        if not self._deconvolver:
            return self.heuristics.deconvolver(self.specmode, self.spw)
        else:
            return self._deconvolver

    @deconvolver.setter
    def deconvolver(self, value):
        self._deconvolver = value

    @property
    def robust(self):
        if self._robust == -999.0:
            if (self.spw.find(',') == -1):
                return self.heuristics.robust(self.spw)
            else:
                robust = 0.0
                spws = self.spw.split(',')
                for spw in spws:
                    robust += self.heuristics.robust(spw)
                robust /= len(spws)
                return robust
        else:
            return self._robust

    @robust.setter
    def robust(self, value):
        self._robust = value

# tell the infrastructure to give us mstransformed data when possible by
# registering our preference for imaging measurement sets
basetask.ImagingMeasurementSetsPreferred.register(TcleanInputs)

class Tclean(cleanbase.CleanBase):
    Inputs = TcleanInputs

    def is_multi_vis_task(self):
        return True

    def copy_products(self, old_pname, new_pname):
        try:
            LOG.info('Copying %s to %s' % (old_pname, new_pname))
            shutil.copytree(old_pname, new_pname)
        except Exception as e:
            LOG.warning('Exception copying %s: %s' % (old_pname, e))

    def prepare(self):
        context = self.inputs.context
        inputs = self.inputs

        LOG.info('\nCleaning for intent "%s", field %s, spw %s\n',
                 inputs.intent, inputs.field, inputs.spw)

        result = None

        # delete any old files with this naming root. One of more
        # of these (don't know which) will interfere with this run.
        LOG.info('deleting %s*.iter*', inputs.imagename)
        shutil.rmtree('%s*.iter*' % inputs.imagename, ignore_errors=True)

        # Set initial masking limits
        self.pblimit_image = 0.2
        self.pblimit_cleanmask = 0.3
        inputs.pblimit = self.pblimit_image

        # Instantiate the clean list heuristics class
        self.clheuristics = makeimlist.MakeImListHeuristics(context=context,
                                                       vislist=inputs.vis,
                                                       spw=inputs.spw,
                                                       contfile=context.contfile,
                                                       linesfile=context.linesfile)

        # Generate the image name if one is not supplied.
        if inputs.imagename in ('', None):
            inputs.imagename = self.clheuristics.imagename(intent=inputs.intent,
                                                      field=inputs.field,
                                                      spwspec=inputs.spw)

        # Determine the default gridder
        if inputs.gridder in ('', None):
            inputs.gridder = self.clheuristics.gridder(inputs.intent, inputs.field)

        # Determine the default deconvolver
        if inputs.deconvolver in ('', None):
            inputs.deconvolver = self.clheuristics.deconvolver(inputs.intent,
                                                          inputs.field)

        # Determine the phase center.
        if inputs.phasecenter in ('', None):
            field_id = self.clheuristics.field(inputs.intent, inputs.field)
            inputs.phasecenter = self.clheuristics.phasecenter(field_id)

        # If imsize not set then use heuristic code to calculate the
        # centers for each field  / spw
        imsize = inputs.imsize
        cell = inputs.cell
        if imsize == [] or cell == []:

            # The heuristics cell size  is always the same for x and y as
            # the value derives from a single value returned by imager.advise
            cell, valid_data = self.clheuristics.cell(field_intent_list=[(inputs.field, inputs.intent)],
                                           spwspec=inputs.spw)
            beam = self.clheuristics.beam(spwspec=inputs.spw)

            if inputs.cell == []:
                inputs.cell = cell
                LOG.info('Heuristic cell: %s' % cell)

            field_ids = self.clheuristics.field(inputs.intent, inputs.field)
            imsize = self.clheuristics.imsize(fields=field_ids,
                                         cell=inputs.cell, beam=beam)
            if inputs.imsize == []:
                inputs.imsize = imsize
                LOG.info('Heuristic imsize: %s', imsize)

        # Get a noise estimate from the CASA sensitivity calculator
        sensitivity, channel_rms_factor = self._do_sensitivity()
        LOG.info('Sensitivity estimate from CASA: %s Jy', sensitivity)

        # Choose cleaning method.
        if inputs.hm_masking in ('centralregion', 'manual'):
            # Determine threshold
            if inputs.hm_cleaning == 'manual':
                threshold = inputs.threshold
            elif inputs.hm_cleaning == 'sensitivity':
                raise Exception, 'sensitivity threshold not yet implemented'
            elif inputs.hm_cleaning == 'rms':
                threshold = '%sJy' % (inputs.tlimit * sensitivity)

            # Choose sequence manager
            # Central mask based on PB
            if inputs.hm_masking == 'centralregion':
                sequence_manager = ImageCentreThresholdSequence(
                    gridder = inputs.gridder, threshold=threshold,
                    channel_rms_factor = channel_rms_factor,
                    sensitivity = sensitivity, niter=inputs.niter)
            # Manually supplied mask
            else:
                sequence_manager = ManualMaskThresholdSequence(
                    mask=inputs.mask,
                    gridder = inputs.gridder, threshold=threshold,
                    channel_rms_factor = channel_rms_factor,
                    sensitivity = sensitivity, niter=inputs.niter)

        elif inputs.hm_masking == 'psfiter':
            sequence_manager = IterativeSequence(
                maxncleans=inputs.maxncleans,
                sensitivity=sensitivity,
                channel_rms_factor = channel_rms_factor)

        elif inputs.hm_masking == 'psfiter2':
            sequence_manager = IterativeSequence2(
                maxncleans=inputs.maxncleans,
                sensitivity=sensitivity,
                channel_rms_factor = channel_rms_factor)

        result = self._do_iterative_imaging(
            sequence_manager=sequence_manager, result=result)

        return result

    def analyse(self, result):
        # Perform QA here if this is a sub-task
        context = self.inputs.context
        pipelineqa.registry.do_qa(context, result)

        return result

    def _do_iterative_imaging(self, sequence_manager, result):

        context = self.inputs.context
        inputs = self.inputs

        if (inputs.specmode == 'cube'):
            inputs.width = 1
            inputs.start = 1
            inputs.nchan = context.observing_run.measurement_sets[0].get_spectral_window(inputs.spw).num_channels - 2

        # Check if a matching 'cont' image exists for continuum subtraction.
        # NOTE: For Cycle 3 we use 'mfs' images due to possible
        #       inaccuracies in the nterms=2 cont images.
        #       If 'cont' images are going to be used, then the
        #       startmodel parameter must be a list with
        #       ['<name>.model.tt0', '<name>.model.tt1'].
        # NOTE: For Cycle 4 we will implement uv continuum subtraction
        #       Leave this code in place as a reminder for the future
        #       when the pipeline may opt to choose between them
        #cont_image_name = ''
        #if (('TARGET' in inputs.intent) and (inputs.specmode == 'cube')):
            #imlist = self.inputs.context.sciimlist.get_imlist()
            #for iminfo in imlist[::-1]:
                #if ((iminfo['sourcetype'] == 'TARGET') and \
                    #(iminfo['sourcename'] == inputs.field) and \
                    #(iminfo['specmode'] == 'mfs') and \
                    #(inputs.spw in iminfo['spwlist'].split(','))):
                    #cont_image_name = iminfo['imagename'][:iminfo['imagename'].rfind('.image')]
                    #cont_image_name = cont_image_name.replace('.pbcor', '.model')
                    #break

            #if (cont_image_name != ''):
                #LOG.info('Using %s for continuum subtraction.' % (os.path.basename(cont_image_name)))
            #else:
                #LOG.warning('Could not find any matching continuum image. Skipping continuum subtraction.')

        # Do continuum subtraction for target cubes
        # NOTE: This currently needs to be done as a separate step.
        #       In the future the subtraction will be handled
        #       on-the-fly in tclean.
        #if (cont_image_name != ''):
            #self._do_continuum(cont_image_name = cont_image_name, mode = 'sub')

        # Compute the dirty image
        LOG.info('Compute the dirty image')
        iter = 0
        result = self._do_clean(iter=iter, stokes='I', cleanmask='', niter=0,
                                threshold='0.0mJy',
                                sensitivity=sequence_manager.sensitivity,
                                result=None)

        # Store modified sensitivity
        # TODO: Do this here rather than through cleanbase
        if (result.sensitivity != sequence_manager.sensitivity):
            LOG.info('Adjusting sensitivity for continuum selection from %s Jy to %s Jy by a factor %s' % (sequence_manager.sensitivity, result.sensitivity, result.sensitivity / sequence_manager.sensitivity))
            sequence_manager.sensitivity = result.sensitivity

        # Determine masking limits depending on PB
        self.pblimit_image, self.pblimit_cleanmask = \
            inputs.heuristics.pblimits(result.flux)
        inputs.pblimit = self.pblimit_image

        # Give the result to the sequence_manager for analysis
        model_sum, cleaned_rms, non_cleaned_rms, residual_max, residual_min,\
            rms2d, image_max = sequence_manager.iteration_result(iter=0,
                    multiterm = result.multiterm, psf = result.psf, model = result.model,
                    restored = result.image, residual = result.residual,
                    flux = result.flux, cleanmask=None, threshold = None,
                    pblimit_image = self.pblimit_image,
                    pblimit_cleanmask = self.pblimit_cleanmask)

        # Temporarily needed until CAS-8576 is fixed
        result._residual_max = residual_max

        LOG.info('Dirty image stats')
        LOG.info('    Rms %s', non_cleaned_rms)
        LOG.info('    Residual max %s', residual_max)
        LOG.info('    Residual min %s', residual_min)

        # Check dynamic range and adjust threshold
        qaTool = casatools.quanta
        dirty_dynamic_range = residual_max / sequence_manager.sensitivity

        old_threshold = qaTool.convert(sequence_manager.threshold, 'Jy')['value']
        if (inputs.intent == 'TARGET'):
            if (context.observing_run.get_measurement_sets()[0].antennas[0].diameter == 12.0):
                if (dirty_dynamic_range > 150.):
                    maxSciEDR = 150.0
                    new_threshold = max(2.5 * old_threshold, residual_max / maxSciEDR * inputs.tlimit)
                else:
                    if (dirty_dynamic_range > 100.):
                        n_dr = 2.5
                    elif (50. < dirty_dynamic_range <= 100.):
                        n_dr = 2.0
                    elif (20. < dirty_dynamic_range <= 50.):
                        n_dr = 1.5
                    elif (dirty_dynamic_range <= 20.):
                        n_dr = 1.0
                    new_threshold = old_threshold * n_dr
            else:
                if (dirty_dynamic_range > 30.):
                    maxSciEDR = 30.0
                    new_threshold = max(2.5 * old_threshold, residual_max / maxSciEDR * inputs.tlimit)
                else:
                    if (dirty_dynamic_range > 20.):
                        n_dr = 2.5
                    elif (10. < dirty_dynamic_range <= 20.):
                        n_dr = 2.0
                    elif (4. < dirty_dynamic_range <= 10.):
                        n_dr = 1.5
                    elif (dirty_dynamic_range <= 4.):
                        n_dr = 1.0
                    new_threshold = old_threshold * n_dr
        else:
            # Calibrators are usually dynamic range limited. The sensitivity from apparentsens
            # is not a valid estimate for the threshold. Use a heuristic based on the dirty peak
            # and some maximum expected dynamic range (EDR) values.
            if (context.observing_run.get_measurement_sets()[0].antennas[0].diameter == 12.0):
                maxCalEDR = 1000.0
            else:
                maxCalEDR = 200.0
            new_threshold = max(old_threshold, residual_max / maxCalEDR * inputs.tlimit)

        if (new_threshold != old_threshold):
            sequence_manager.threshold = '%sJy' % (new_threshold)
            LOG.info('Modified threshold from %s Jy to %s Jy based on dynamic range heuristic (dirty dynamic range: %.1f)' % (old_threshold, new_threshold, dirty_dynamic_range))

        # Compute automatic niter estimate
        old_niter = sequence_manager.niter
        kappa = 5
        loop_gain = 0.1
        r_mask = 0.45 * max(inputs.imsize[0], inputs.imsize[1]) * qaTool.convert(inputs.cell[0], 'arcsec')['value']
        beam = qaTool.convert(inputs.cell[0], 'arcsec')['value'] * 5.0
        new_niter_f = int(kappa / loop_gain * (r_mask / beam) ** 2 * residual_max / new_threshold)
        new_niter = int(round(new_niter_f, -int(numpy.log10(new_niter_f))))
        if (new_niter != old_niter):
            sequence_manager.niter = new_niter
            LOG.info('Modified niter from %d to %d based on mask vs. beam size heuristic' % (old_niter, new_niter))

        iterating = True
        iter = 1
        while iterating:
            # Create the name of the next clean mask from the root of the 
            # previous residual image.
            rootname, ext = os.path.splitext(result.residual)
            rootname, ext = os.path.splitext(rootname)

            # Delete any old files with this naming root
            filenames = glob.glob('%s.iter%s*' % (rootname, iter))
            for filename in filenames:
                try:
                    shutil.rmtree(filename)
                except Exception as e:
                    LOG.warning('Exception while deleting %s: %s' % (filename, e))

            new_cleanmask = '%s.iter%s.cleanmask' % (rootname, iter)

            # perform an iteration.
            seq_result = sequence_manager.iteration(new_cleanmask, self.pblimit_image, self.pblimit_cleanmask)

            # Check the iteration status.
            if not seq_result.iterating:
                break

            # Use previous iterations's products as starting point
            ptypes = ['residual', 'sumwt', 'psf', 'pb']
            if (inputs.gridder == 'mosaic'):
                ptypes.append('weight')
            for ptype in ptypes:
                old_pname = '%s.iter%s.%s' % (rootname, iter-1, ptype)
                new_pname = '%s.iter%s.%s' % (rootname, iter, ptype)
                if (ptype == 'pb'):
                    self.copy_products(old_pname, new_pname)
                else:
                    if (inputs.deconvolver == 'mtmfs'):
                        if (ptype in ['sumwt', 'psf', 'weight']):
                            exts = ['.tt0', '.tt1', '.tt2']
                        else:
                            exts = ['.tt0', '.tt1']
                    else:
                        exts = ['']
                    for ext in exts:
                        self.copy_products('%s%s' % (old_pname, ext), '%s%s' % (new_pname, ext))

            # Determine the cleaning threshold
            threshold = seq_result.threshold

            LOG.info('Iteration %s: Clean control parameters' % iter)
            LOG.info('    Mask %s', new_cleanmask)
            LOG.info('    Threshold %s', seq_result.threshold)
            LOG.info('    Niter %s', seq_result.niter)

            result = self._do_clean(iter=iter, stokes='I',
                    cleanmask=new_cleanmask, niter=seq_result.niter,
                    threshold=threshold,
                    sensitivity=sequence_manager.sensitivity, result=result)

            # Give the result to the clean 'sequencer'
            model_sum, cleaned_rms, non_cleaned_rms, residual_max, residual_min, rms2d, image_max = sequence_manager.iteration_result(
                iter=iter, multiterm=result.multiterm, psf=result.psf, model=result.model, restored=result.image, residual=result.residual,
                flux=result.flux, cleanmask=new_cleanmask, threshold=seq_result.threshold, pblimit_image = self.pblimit_image,
                pblimit_cleanmask = self.pblimit_cleanmask)

            # Keep RMS for QA
            result.set_rms(non_cleaned_rms)

            LOG.info('Clean image iter %s stats' % iter)
            LOG.info('    Clean rms %s', cleaned_rms)
            LOG.info('    Nonclean rms %s', non_cleaned_rms)
            LOG.info('    Residual max %s', residual_max)
            LOG.info('    Residual min %s', residual_min)

            # Keep tclean summary plot
            try:
                shutil.move('summaryplot_1.png', '%s.iter%s.summaryplot.png' % (rootname, iter))
            except:
                LOG.info('Could not save tclean summary plot.')

            # Up the iteration counter
            iter += 1

        # Re-add continuum so that the MS is unchanged afterwards.
        #if (cont_image_name != ''):
            #if (inputs.subcontms == False):
                #self._do_continuum(cont_image_name = cont_image_name, mode = 'add')
            #else:
                #LOG.warn('Not re-adding continuum model. MS is modified !')

        return result

    def _do_noise_estimate (self, stokes):
        """Compute a noise estimate from the specified stokes image.
        """
        # Compute the dirty Q or V image.
        try:
            LOG.info("Compute the 'noise' image")
            result = self._do_clean (iter=0, stokes=stokes,
                                     cleanmask='', niter=0, threshold='0.0mJy', sensitivity=0.0, result=None)
            if result.empty():
                raise Exception, '%s/%s/SpW%s Error creating Stokes %s noise image' % (
                    self.inputs.intent, self.inputs.field, self.inputs.spw, stokes)
        except Exception, e:
            raise Exception, '%s/%s/SpW%s Error creating Stokes %s noise image: %s' % (
                self.inputs.intent, self.inputs.field, self.inputs.spw, stokes, str(e))

        # Create the base sequence manager and use it to get noise stats
        sequence_manager = BaseCleanSequence()
        model_sum, cleaned_rms, non_cleaned_rms, residual_max, \
        residual_min, rms2d, image_max = \
            sequence_manager.iteration_result(iter=0, multiterm=result.multiterm,
                                              psf=result.psf, model=result.model, restored=result.image,
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

        # Calculate sensitivities
        sensitivities = []
        if inputs.specmode in ['mfs','cont']:
            specmode = 'mfs'
        elif inputs.specmode == 'cube':
            specmode = 'cube'
        else:
            raise Exception, 'Unknown specmode "%s"' % (inputs.specmode)

        targetmslist = [ms_do for ms_do in [context.observing_run.get_ms(name=ms) for ms in inputs.vis] if ms_do.is_imaging_ms]
        if (targetmslist == []):
            targetmslist = [context.observing_run.get_ms(name=ms) for ms in inputs.vis]
        for ms in targetmslist:
            for intSpw in [int(s) for s in spw.split(',')]:
                try:
                    if (inputs.gridder == 'mosaic'):
                        field_sensitivities = []
                        for field_id in [f.id for f in ms.fields if (utils.dequote(f.name) == utils.dequote(field) and inputs.intent in f.intents)]:
                            with casatools.ImagerReader(ms.name) as imTool:
                                imTool.selectvis(spw=intSpw, field=field_id)
                                # TODO: Add scan selection ?
                                imTool.defineimage(mode=specmode, spw=intSpw,
                                                   cellx=inputs.cell[0], celly=inputs.cell[0],
                                                   nx=inputs.imsize[0], ny=inputs.imsize[1])
                                # TODO: Mosaic switch needed ?
                                imTool.weight(type=inputs.weighting, robust=inputs.robust)
                                result = imTool.apparentsens()
                                if (result[1] != 0.0):
                                    field_sensitivities.append(result[1])

                        # Calculate mosaic overlap factor
                        source_name = [f.source.name for f in ms.fields if (utils.dequote(f.name) == utils.dequote(field) and inputs.intent in f.intents)][0]
                        diameter = numpy.median([a.diameter for a in ms.antennas])
                        overlap_factor = mosaicoverlap.mosaicOverlapFactorMS(ms, source_name, intSpw, diameter)
                        LOG.info('Applying mosaic overlap factor of %s.' % (overlap_factor))
                        sensitivities.append(numpy.median(field_sensitivities) / overlap_factor)
                    else:
                        with casatools.ImagerReader(ms.name) as imTool:
                            imTool.selectvis(spw=intSpw, field=field)
                            # TODO: Add scan selection ?
                            imTool.defineimage(mode=specmode, spw=intSpw,
                                               cellx=inputs.cell[0], celly=inputs.cell[0],
                                               nx=inputs.imsize[0], ny=inputs.imsize[1])
                            # TODO: Mosaic switch needed ?
                            imTool.weight(type=inputs.weighting, robust=inputs.robust)
                            result = imTool.apparentsens()
                            if (result[1] != 0.0):
                                sensitivities.append(result[1])
                except Exception as e:
                    # Simply pass as this could be a case of a source not
                    # being present in the MS.
                    pass

        if (len(sensitivities) != 0):
            sensitivity = 1.0 / numpy.sqrt(numpy.sum(1.0 / numpy.array(sensitivities)**2))
        else:
            defaultSensitivity = 0.001
            LOG.warning('Exception in calculating sensitivity. Assuming %g Jy/beam.' % (defaultSensitivity))
            sensitivity = defaultSensitivity

        if inputs.specmode == 'cube':
            if inputs.nchan != -1:
                channel_rms_factor = numpy.sqrt(inputs.nchan)
            else:
                qaTool = casatools.quanta
                ms = context.observing_run.measurement_sets[0]
                spwDesc = ms.get_spectral_window(spw)
                min_frequency = float(spwDesc.min_frequency.to_units(measures.FrequencyUnits.GIGAHERTZ))
                max_frequency = float(spwDesc.max_frequency.to_units(measures.FrequencyUnits.GIGAHERTZ))
                if qaTool.convert(inputs.width, 'GHz')['value'] == 0.0:
                    effective_channel_width = [float(ch.effective_bw.convert_to(measures.FrequencyUnits.GIGAHERTZ).value) for ch in spwDesc.channels][len(spwDesc.channels)/2]
                    channel_rms_factor = numpy.sqrt(abs((max_frequency - min_frequency) / effective_channel_width))
                else:
                    channel_rms_factor = numpy.sqrt(abs((max_frequency - min_frequency) / qaTool.convert(inputs.width, 'GHz')['value']))
            LOG.info('Applying effective channel width correction factor of %s.' % (channel_rms_factor))
            sensitivity *= channel_rms_factor
        else:
            channel_rms_factor = 1.0

        return sensitivity, channel_rms_factor

    def _do_continuum(self, cont_image_name, mode):
        """
        Add/Subtract continuum model.
        """

        context = self.inputs.context
        inputs = self.inputs

        LOG.info('Predict continuum model.')

        # Set the data column
        #   This routine is not used for Cycle 4. This is a reminder to
        #   consider the datacolumn default when making future modifications
        targetmslist = [vis for vis in inputs.vis if context.observing_run.get_ms(name=vis).is_imaging_ms]
        if len(targetmslist) > 0:
            datacolumn = 'data'
        else:
            datacolumn = 'corrected'

        # Predict continuum model
        job = casa_tasks.tclean(vis=inputs.vis, imagename='%s.I.cont_%s_pred' %
                (os.path.basename(inputs.imagename), mode),
                datacolumn=datacolumn,
                spw=inputs.spw,
                intent='*TARGET*',
                scan='', specmode='mfs', gridder=inputs.gridder,
                pblimit=self.pblimit_image, niter=0,
                threshold='0.0mJy', deconvolver=inputs.deconvolver,
                interactive=False, outframe=inputs.outframe, nchan=inputs.nchan,
                start=inputs.start, width=inputs.width, imsize=inputs.imsize,
                cell=inputs.cell, phasecenter=inputs.phasecenter,
                stokes='I',
                weighting=inputs.weighting, robust=inputs.robust,
                npixels=inputs.npixels,
                restoringbeam=inputs.restoringbeam, uvrange=inputs.uvrange,
                mask='', startmodel=cont_image_name,
                savemodel='modelcolumn',
                parallel=False)
        self._executor.execute(job)

        # Add/subtract continuum model
        if mode == 'sub':
            LOG.info('Subtract continuum model.')
        else:
            LOG.info('Add continuum model.')
        # Need to use MS tool to get the proper data selection.
        # The uvsub task does not provide this.
        cms = casatools.ms
        for vis in inputs.vis:
            ms_info = context.observing_run.get_ms(vis)

            field_ids = []
            field_infos = ms_info.get_fields()
            for i in xrange(len(field_infos)):
                if ((field_infos[i].name == inputs.field) and ('TARGET' in field_infos[i].intents)):
                    field_ids.append(str(i))
            field_ids = reduce(lambda x, y: '%s,%s' % (x, y), field_ids)

            scan_numbers = []
            for scan_info in ms_info.scans:
                if ((inputs.field in [f.name for f in scan_info.fields]) and ('TARGET' in scan_info.intents)):
                    scan_numbers.append(scan_info.id)
            scan_numbers = reduce(lambda x, y: '%s,%s' % (x, y), scan_numbers)

            if mode == 'sub':
                LOG.info('Subtracting continuum for %s.' % (os.path.basename(vis)))
            else:
                LOG.info('Adding continuum for %s.' % (os.path.basename(vis)))
            cms.open(vis, nomodify=False)
            cms.msselect({'field': field_ids, 'scan': scan_numbers, 'spw': inputs.spw})
            if mode == 'sub':
                cms.uvsub()
            else:
                cms.uvsub(reverse=True)
            cms.close()

    def _do_clean(self, iter, stokes, cleanmask, niter, threshold, sensitivity, result):
        """
        Do basic cleaning.
        """
        inputs = self.inputs

        parallel = mpihelpers.parse_mpi_input_parameter(inputs.parallel)

        clean_inputs = cleanbase.CleanBase.Inputs(inputs.context,
                                                  output_dir=inputs.output_dir,
                                                  vis=inputs.vis,
                                                  imagename=inputs.imagename,
                                                  intent=inputs.intent,
                                                  field=inputs.field,
                                                  spw=inputs.spw,
                                                  spwsel=inputs.spwsel,
                                                  uvrange=inputs.uvrange,
                                                  specmode=inputs.specmode,
                                                  gridder=inputs.gridder,
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
                                                  sensitivity=sensitivity,
                                                  pblimit=inputs.pblimit,
                                                  result=result,
                                                  parallel=parallel)
        clean_task = cleanbase.CleanBase(clean_inputs)

        return self._executor.execute(clean_task)

    # Remove pointing table.
    def _empty_pointing_table(self):
        # Concerned that simply renaming things directly 
        # will corrupt the table cache, so do things using only the
        # table tool.
        for vis in self.inputs.vis:
            with casatools.TableReader('%s/POINTING' % vis,
                                       nomodify=False) as table:
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
            with casatools.TableReader('%s/POINTING_COPY' % vis,
                                       nomodify=False) as table:
                LOG.debug('Copying back into POINTING table')
                original = table.copy('%s/POINTING' % vis, valuecopy=True)
                original.done()
