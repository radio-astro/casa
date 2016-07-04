import os
import shutil
import glob
import numpy
import scipy.special
import operator
import decimal

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
from .automaskthresholdsequence import AutoMaskThresholdSequence
from .manualmaskthresholdsequence import ManualMaskThresholdSequence
from .iterativesequence import IterativeSequence
from .iterativesequence2 import IterativeSequence2
from . import cleanbase

from pipeline.hif.heuristics import makeimlist

LOG = infrastructure.get_logger(__name__)


class TcleanInputs(cleanbase.CleanBaseInputs):
    def __init__(self, context, output_dir=None, vis=None, imagename=None,
                 intent=None, field=None, spw=None, spwsel_lsrk=None, spwsel_topo=None, uvrange=None, specmode=None,
                 gridder=None, deconvolver=None, outframe=None, imsize=None, cell=None,
                 phasecenter=None, nchan=None, start=None, width=None,
                 weighting=None, robust=None, noise=None, npixels=None,
                 restoringbeam=None, iter=None, mask=None, niter=None, threshold=None,
                 noiseimage=None, hm_masking=None, hm_cleaning=None, tlimit=None,
                 masklimit=None, maxncleans=None, subcontms=None, parallel=None):
        self._init_properties(vars())
        self.heuristics = tclean.TcleanHeuristics(self.context, self.vis, self.spw)

    # Add extra getters and setters here
    spwsel_lsrk = basetask.property_with_default('spwsel_lsrk', {})
    spwsel_topo = basetask.property_with_default('spwsel_topo', [])
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

        if (inputs.specmode == 'cube'):
            # To avoid noisy edge channels, use only the LSRK frequency
            # intersection and skip one channel on either end.
            if0, if1, channel_width = inputs.heuristics.lsrk_freq_intersection(inputs.vis, inputs.field, inputs.spw)
            if (if0 == -1) or (if1 == -1):
                LOG.error('No LSRK frequency overlap for Field %s SPW %s' % (inputs.field, inputs.spw))
                result.error = '%s/%s/spw%s clean error: %s' % (inputs.field, inputs.intent, inputs.spw)
                return result

            inputs.start = '%sGHz' % ((if0+channel_width) / 1e9)
            inputs.width = '%sMHz' % ((channel_width) / 1e6)
            inputs.nchan = int((if1 - if0 - 2 * channel_width) / channel_width)

        # Get a noise estimate from the CASA sensitivity calculator
        sensitivity, \
        spw_topo_freq_param, \
        spw_topo_chan_param, \
        spw_topo_freq_param_dict, \
        spw_topo_chan_param_dict, \
        total_topo_bw, aggregate_topo_bw = \
            self._do_sensitivity()
        LOG.info('Sensitivity estimate: %s Jy', sensitivity)

        # Choose TOPO frequency selections
        if inputs.specmode != 'cube':
            inputs.spwsel_topo = spw_topo_freq_param
        else:
            inputs.spwsel_topo = ['%s' % (inputs.spw)] * len(inputs.vis)

        # Choose cleaning method.
        if inputs.hm_masking in ('centralregion', 'auto', 'manual'):
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
                    sensitivity = sensitivity, niter=inputs.niter)
            # Auto-boxing
            elif inputs.hm_masking == 'auto':
                sequence_manager = AutoMaskThresholdSequence(
                    gridder = inputs.gridder, threshold=threshold,
                    sensitivity = sensitivity, niter=inputs.niter)
            # Manually supplied mask
            else:
                sequence_manager = ManualMaskThresholdSequence(
                    mask=inputs.mask,
                    gridder = inputs.gridder, threshold=threshold,
                    sensitivity = sensitivity, niter=inputs.niter)

        elif inputs.hm_masking == 'psfiter':
            sequence_manager = IterativeSequence(
                maxncleans=inputs.maxncleans,
                sensitivity=sensitivity)

        elif inputs.hm_masking == 'psfiter2':
            sequence_manager = IterativeSequence2(
                maxncleans=inputs.maxncleans,
                sensitivity=sensitivity)

        result = self._do_iterative_imaging(
            sequence_manager=sequence_manager, result=result)

        # Record aggregate bandwidth for weblog
        # TODO: Record total bandwidth as opposed to range
        #       Save channel selection in result for weblog.
        result.set_aggregate_bw(aggregate_topo_bw)

        return result

    def analyse(self, result):
        # Perform QA here if this is a sub-task
        context = self.inputs.context
        pipelineqa.registry.do_qa(context, result)

        return result

    def _do_iterative_imaging(self, sequence_manager, result):

        context = self.inputs.context
        inputs = self.inputs

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
            n_false_pos = 1
            r_mask_pixel = 0.45 * max(inputs.imsize[0], inputs.imsize[1])
            n_pixel_mask = numpy.pi * r_mask_pixel ** 2
            n_dr_min = scipy.special.erfcinv(n_false_pos / numpy.sqrt(2.0) / n_pixel_mask)
            n_dr_max = max(n_dr_min, 2.5)
            LOG.info('DR heuristic: N_DR_min=%s N_DR_max=%s' % (n_dr_min, n_dr_max))
            if (context.observing_run.get_measurement_sets()[0].antennas[0].diameter == 12.0):
                if (dirty_dynamic_range > 150.):
                    maxSciEDR = 150.0
                    new_threshold = max(n_dr_max * old_threshold, residual_max / maxSciEDR * inputs.tlimit)
                    LOG.info('DR heuristic: Applying maxSciEDR(Main array)=%s' % (maxSciEDR))
                else:
                    if (n_dr_min > 2.5):
                        n_dr = n_dr_min
                        LOG.info('DR heuristic: Applying N_DR=N_DR_min=%s' % (n_dr_min))
                    else:
                        if (dirty_dynamic_range > 100.):
                            n_dr = 2.5
                        elif (50. < dirty_dynamic_range <= 100.):
                            n_dr = 2.0
                        elif (20. < dirty_dynamic_range <= 50.):
                            n_dr = 1.5
                        elif (dirty_dynamic_range <= 20.):
                            n_dr = 1.0
                        LOG.info('DR heuristic: Applying N_DR=%s' % (n_dr))
                    new_threshold = old_threshold * n_dr
            else:
                if (dirty_dynamic_range > 30.):
                    maxSciEDR = 30.0
                    new_threshold = max(n_dr_max * old_threshold, residual_max / maxSciEDR * inputs.tlimit)
                    LOG.info('DR heuristic: Applying maxSciEDR(ACA)=%s' % (maxSciEDR))
                else:
                    if (n_dr_min > 2.5):
                        n_dr = n_dr_min
                        LOG.info('DR heuristic: Applying N_DR=N_DR_min=%s' % (n_dr_min))
                    else:
                        if (dirty_dynamic_range > 20.):
                            n_dr = 2.5
                        elif (10. < dirty_dynamic_range <= 20.):
                            n_dr = 2.0
                        elif (4. < dirty_dynamic_range <= 10.):
                            n_dr = 1.5
                        elif (dirty_dynamic_range <= 4.):
                            n_dr = 1.0
                        LOG.info('DR heuristic: Applying N_DR=%s' % (n_dr))
                    new_threshold = old_threshold * n_dr
        else:
            # Calibrators are usually dynamic range limited. The sensitivity from apparentsens
            # is not a valid estimate for the threshold. Use a heuristic based on the dirty peak
            # and some maximum expected dynamic range (EDR) values.
            if (context.observing_run.get_measurement_sets()[0].antennas[0].diameter == 12.0):
                maxCalEDR = 1000.0
            else:
                maxCalEDR = 200.0
            LOG.info('DR heuristic: Applying maxCalEDR=%s' % (maxCalEDR))
            new_threshold = max(old_threshold, residual_max / maxCalEDR * inputs.tlimit)

        if (new_threshold != old_threshold):
            sequence_manager.threshold = '%sJy' % (new_threshold)
            LOG.info('DR heuristic: Modified threshold from %s Jy to %s Jy based on dynamic range heuristic (dirty dynamic range: %.1f)' % (old_threshold, new_threshold, dirty_dynamic_range))

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
            LOG.info('niter heuristic: Modified niter from %d to %d based on mask vs. beam size heuristic' % (old_niter, new_niter))

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

            if inputs.hm_masking == 'auto':
                new_cleanmask = '%s.iter%s.mask' % (rootname, iter)
            else:
                new_cleanmask = '%s.iter%s.cleanmask' % (rootname, iter)

            # perform an iteration.
            if (inputs.specmode == 'cube'):
                seq_result = sequence_manager.iteration(new_cleanmask, self.pblimit_image, self.pblimit_cleanmask, inputs.spw, inputs.spwsel_lsrk)
            else:
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

        # If specmode is "cube", create from the non-pbcorrected cube 
        # after continuum subtraction an image of the moment 0 integrated 
        # intensity for the line-free channels.
        if inputs.specmode == 'cube':
            self._calc_mom0_fc(result)

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

        # Convert LSRK ranges to TOPO
        spw_topo_freq_param, spw_topo_chan_param, spw_topo_freq_param_dict, spw_topo_chan_param_dict, total_topo_bw, aggregate_topo_bw = self.inputs.heuristics.calc_topo_ranges(inputs)

        detailed_field_sensitivities = {}
        for ms in targetmslist:
            detailed_field_sensitivities[os.path.basename(ms.name)] = {}
            for intSpw in [int(s) for s in spw.split(',')]:
                spw_do = ms.get_spectral_window(intSpw)
                detailed_field_sensitivities[os.path.basename(ms.name)][intSpw] = {}
                try:
                    if (inputs.specmode == 'cube'):
                        # Use the center channel
                        chansel = '%d~%d' % (int(spw_do.num_channels / 2.0), int(spw_do.num_channels / 2.0))
                    else:
                        if (spw_topo_chan_param_dict[os.path.basename(ms.name)][str(intSpw)] != ''):
                            # Use continuum frequency selection
                            chansel = spw_topo_chan_param_dict[os.path.basename(ms.name)][str(intSpw)]
                        else:
                            # Use full spw
                            chansel = '0~%d' % (spw_do.num_channels - 1)

                    if (inputs.gridder == 'mosaic'):
                        field_sensitivities = []
                        for field_id in [f.id for f in ms.fields if (utils.dequote(f.name) == utils.dequote(field) and inputs.intent in f.intents)]:
                            try:
                                field_sensitivity = self._get_sensitivity(ms, field_id, intSpw, chansel)
                                field_sensitivities.append(field_sensitivity)
                                detailed_field_sensitivities[os.path.basename(ms.name)][intSpw][field_id] = field_sensitivity
                            except Exception as e:
                                LOG.warning('Could not calculate sensitivity for MS %s Field %s (ID %d) SPW %d ChanSel %s' % (os.path.basename(ms.name), utils.dequote(field), field_id, intSpw, chansel))

                        LOG.info('Using median of all mosaic field sensitivities for MS %s, Field %s, SPW %s: %s Jy' % (os.path.basename(ms.name), field, str(intSpw), numpy.median(field_sensitivities)))
                        min_field_id, min_sensitivity = min(detailed_field_sensitivities[os.path.basename(ms.name)][intSpw].iteritems(), key=operator.itemgetter(1))
                        LOG.info('Minimum mosaic field sensitivity for MS %s, Field %s (ID: %s), SPW %s: %s Jy' % (os.path.basename(ms.name), field, min_field_id, str(intSpw), min_sensitivity))
                        max_field_id, max_sensitivity = max(detailed_field_sensitivities[os.path.basename(ms.name)][intSpw].iteritems(), key=operator.itemgetter(1))
                        LOG.info('Maximum mosaic field sensitivity for MS %s, Field %s (ID: %s), SPW %s: %s Jy' % (os.path.basename(ms.name), field, max_field_id, str(intSpw), max_sensitivity))

                        # Calculate mosaic overlap factor
                        source_name = [f.source.name for f in ms.fields if (utils.dequote(f.name) == utils.dequote(field) and inputs.intent in f.intents)][0]
                        diameter = numpy.median([a.diameter for a in ms.antennas])
                        overlap_factor = mosaicoverlap.mosaicOverlapFactorMS(ms, source_name, intSpw, diameter)
                        LOG.info('Applying mosaic overlap factor of %s.' % (overlap_factor))
                        sensitivities.append(numpy.median(field_sensitivities) / overlap_factor)
                    else:
                        # Use field name for single field case
                        field_sensitivity = self._get_sensitivity(ms, field, intSpw, chansel)
                        sensitivities.append(field_sensitivity)
                        detailed_field_sensitivities[os.path.basename(ms.name)][intSpw][field] = field_sensitivity
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

        return sensitivity, spw_topo_freq_param, spw_topo_chan_param, spw_topo_freq_param_dict, spw_topo_chan_param_dict, total_topo_bw, aggregate_topo_bw

    def _get_sensitivity(self, ms_do, field, spw, chansel):
        """
        Get sensitivity for a field / spw / chansel combination from CASA's
        apparentsens method and a correction for effective channel widths
        in case of online smoothing.

        This heuristic is currently optimized for ALMA data only.
        """

        context = self.inputs.context
        inputs = self.inputs

        spw_do = ms_do.get_spectral_window(spw)
        spwchan = spw_do.num_channels
        physicalBW_of_1chan = float(spw_do.channels[0].getWidth().convert_to(measures.FrequencyUnits.HERTZ).value)
        effectiveBW_of_1chan = float(spw_do.channels[0].effective_bw.convert_to(measures.FrequencyUnits.HERTZ).value)

        BW_ratio = effectiveBW_of_1chan / physicalBW_of_1chan

        if (BW_ratio <= 1.0):
            N_smooth = 0
        elif (utils.approx_equal(BW_ratio, 2.667, 4)):
            N_smooth = 1
        elif (utils.approx_equal(BW_ratio, 1.600, 4)):
            N_smooth = 2
        elif (utils.approx_equal(BW_ratio, 1.231, 4)):
            N_smooth = 4
        elif (utils.approx_equal(BW_ratio, 1.104, 4)):
            N_smooth = 8
        elif (utils.approx_equal(BW_ratio, 1.049, 4)):
            N_smooth = 16
        else:
            LOG.warning('Could not determine channel bandwidths ratio. Physical: %s Effective: %s' % (physicalBW_of_1chan, effectiveBW_of_1chan))
            N_smooth = 0

        chansel_sensitivities = []
        for chanrange in chansel.split(';'):

            try:
                with casatools.ImagerReader(ms_do.name) as imTool:
                    imTool.selectvis(spw='%s:%s' % (spw, chanrange), field=field)
                    # TODO: Add scan selection ?
                    imTool.defineimage(mode=inputs.specmode if inputs.specmode=='cube' else 'mfs', spw=spw,
                                       cellx=inputs.cell[0], celly=inputs.cell[0],
                                       nx=inputs.imsize[0], ny=inputs.imsize[1])
                    # TODO: Mosaic switch needed ?
                    imTool.weight(type=inputs.weighting, robust=inputs.robust)
                    result = imTool.apparentsens()

                apparentsens_value = result[1]
                LOG.info('apparentsens result for MS %s Field %s SPW %s ChanRange %s: %s Jy/beam' % (os.path.basename(ms_do.name), field, spw, chanrange, apparentsens_value))
                if (N_smooth > 0):
                    cstart, cstop = map(int, chanrange.split('~'))
                    nchan = cstop - cstart + 1
                    optimisticBW = nchan * float(effectiveBW_of_1chan)
                    approximateEffectiveBW = (N_smooth * nchan + 1.12 * (spwchan - nchan) / spwchan) * float(physicalBW_of_1chan)
                    SCF = (optimisticBW / approximateEffectiveBW)**0.5
                    corrected_apparentsens_value = apparentsens_value * SCF
                    LOG.info('Correcting apparentsens result by %s from %s Jy/beam to %s Jy/beam' % (SCF, apparentsens_value, corrected_apparentsens_value))
                else:
                    corrected_apparentsens_value = apparentsens_value 

            except Exception as e:
                LOG.warning('Could not calculate sensitivity for MS %s Field %s SPW %s ChanRange %s: %s' % (os.path.basename(ms_do.name), field, spw, chanrange, e))

            chansel_sensitivities.append(corrected_apparentsens_value)

        return 1.0 / numpy.sqrt(numpy.sum(1.0 / numpy.array(chansel_sensitivities)**2))

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
                                                  spwsel=inputs.spwsel_topo,
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
                                                  hm_masking=inputs.hm_masking,
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
    
    # Calculate a "mom0_fc" image: this is a moment 0 integration over the 
    # line-free channels of the non-primary-beam corrected image-cube, 
    # after continuum subtraction; where the "line-free" channels are taken 
    # from those identified as continuum channels. 
    # This is a diagnostic plot representing the residual emission 
    # in the line-free (aka continuum) channels. If the continuum subtraction
    # worked well, then this image should just contain noise.
    def _calc_mom0_fc(self, result):
        
        # Find max iteration that was performed.
        maxiter = max(result.iterations.keys())

        # Get filename of image from result, and modify to select the  
        # non-PB-corrected image.
        imagename = result.iterations[maxiter]['image'].replace('.pbcor','')
        
        # Set output filename for MOM0_FC image.
        mom0_name = '%s.mom0_fc' % (imagename)
        
        # Get continuum frequency ranges.
        cont_freq_ranges = self.inputs.spwsel_lsrk['spw%s' % (self.inputs.spw)].split()[0]

        # Convert frequency ranges to channel ranges.
        cont_chan_ranges = utils.freq_selection_to_channels(imagename, cont_freq_ranges)
        
        # Only continue if there were continuum channel ranges for this spw.
        if cont_chan_ranges[0] != 'NONE':
            
            # Create a channel ranges string.
            cont_chan_ranges_str = ";".join(["%s,%s" % (ch0, ch1) for ch0, ch1 in cont_chan_ranges])
            
            # Execute job to create the MOM0_FC image.
            job = casa_tasks.immoments(imagename=imagename, moments=[0], outfile=mom0_name, chans=cont_chan_ranges_str)
            job.execute(dry_run=False)
            assert os.path.exists(mom0_name)
            
            # Update the metadata in the MOM0_FC image.
            cleanbase.set_miscinfo(name=mom0_name, spw=self.inputs.spw, 
              field=self.inputs.field, iter=maxiter, type='mom0_fc')
            
            # Update the result.
            result.set_mom0_fc(maxiter, mom0_name)
        else:
            LOG.warning('Cannot create MOM0_FC image for intent "%s", '
              'field %s, spw %s, no continuum ranges found.' %
              (self.inputs.intent, self.inputs.field, self.inputs.spw))
