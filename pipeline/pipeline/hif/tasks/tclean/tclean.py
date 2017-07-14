import os
import shutil
import glob
import decimal
import commands

import pipeline.domain.measures as measures
from pipeline.hif.heuristics import imageparams_factory
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
from .nomaskthresholdsequence import NoMaskThresholdSequence
from .iterativesequence import IterativeSequence
from .iterativesequence2 import IterativeSequence2
from . import cleanbase

LOG = infrastructure.get_logger(__name__)


class TcleanInputs(cleanbase.CleanBaseInputs):
    def __init__(self, context, output_dir=None,
                 vis=None, imagename=None, intent=None, field=None, spw=None,
                 spwsel_lsrk=None, spwsel_topo=None,
                 uvrange=None, specmode=None, gridder=None, deconvolver=None,
                 nterms=None, outframe=None, imsize=None, cell=None,
                 phasecenter=None, stokes=None, nchan=None, start=None,
                 width=None, nbin=None, weighting=None,
                 robust=None, noise=None, npixels=None,
                 restoringbeam=None, hm_masking=None,
                 hm_sidelobethreshold=None, hm_noisethreshold=None,
                 hm_lownoisethreshold=None, hm_minbeamfrac=None,
                 hm_cleaning=None, mask=None,
                 niter=None, threshold=None, tlimit=None, masklimit=None,
                 maxncleans=None, cleancontranges=None, subcontms=None,
                 parallel=None,
                 # Extra parameters not in the CLI task interface
                 uvtaper=None, scales=None, nsigma=None,
                 cycleniter=None, cyclefactor=None, sensitivity=None,
                 reffreq=None,
                 # End of extra parameters
                 heuristics=None):
        self._init_properties(vars())
        if heuristics is not None:
            self.image_heuristics = heuristics
        else:
            # Need a local heuristics object if called standalone
            # TODO: imaging_mode should not be fixed
            image_heuristics_factory = imageparams_factory.ImageParamsHeuristicsFactory()
            self.image_heuristics = image_heuristics_factory.getHeuristics( \
                vislist = vis, \
                spw = spw, \
                observing_run = self.context.observing_run, \
                imagename_prefix = self.context.project_structure.ousstatus_entity_id, \
                proj_params = self.context.project_performance_parameters, \
                contfile = self.context.contfile, \
                linesfile = self.context.linesfile, \
                imaging_mode = 'ALMA')

        # For MOM0/8_FC and cube RMS we need the LSRK frequency ranges in
        # various places
        self.cont_freq_ranges = ''

    # Add extra getters and setters here
    spwsel_lsrk = basetask.property_with_default('spwsel_lsrk', {})
    spwsel_topo = basetask.property_with_default('spwsel_topo', [])
    hm_cleaning = basetask.property_with_default('hm_cleaning', 'rms')
    hm_masking = basetask.property_with_default('hm_masking', 'centralregion')
    hm_sidelobethreshold = basetask.property_with_default('hm_sidelobethreshold', -999.0)
    hm_noisethreshold = basetask.property_with_default('hm_noisethreshold', -999.0)
    hm_lownoisethreshold = basetask.property_with_default('hm_lownoisethreshold', -999.0)
    hm_minbeamfrac = basetask.property_with_default('hm_minbeamfrac', -999.0)
    masklimit = basetask.property_with_default('masklimit', 4.0)
    tlimit = basetask.property_with_default('tlimit', 2.0)
    cleancontranges = basetask.property_with_default('cleancontranges', False)
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
    def specmode(self):
        return self._specmode

    @specmode.setter
    def specmode(self, value):
        if value == 'repBW':
            self._specmode = 'cube'
        else:
            self._specmode = value

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
        return self._deconvolver

    @deconvolver.setter
    def deconvolver(self, value):
        self._deconvolver = value

    @property
    def nterms(self):
        return self._nterms

    @nterms.setter
    def nterms(self, value):
        self._nterms = value

    @property
    def robust(self):
        if self._robust == -999.0:
            # For Cycle 5 we disable the robust heuristic
            return 0.5
            if (self.spw.find(',') == -1):
                # TODO: Use real synthesized beam size
                hm_robust = self.image_heuristics.robust({'major': '1.0arcsec', 'minor': '1.0arcsec', 'positionangle': '0.0deg'})
                return hm_robust
            else:
                robust = 0.0
                spws = self.spw.split(',')
                for spw in spws:
                    # TODO: Use real synthesized beam size
                    hm_robust = self.image_heuristics.robust({'major': '1.0arcsec', 'minor': '1.0arcsec', 'positionangle': '0.0deg'})
                    robust += hm_robust
                robust /= len(spws)
                return robust
        else:
            return self._robust

    @robust.setter
    def robust(self, value):
        self._robust = value

    @property
    def uvtaper(self):
        if self._uvtaper is None:
            # TODO: Use heuristic
            self._uvtaper = []

        return self._uvtaper

    @uvtaper.setter
    def uvtaper(self, value):
        self._uvtaper = value

    @property
    def sigma(self):
        return self._sigma

    @sigma.setter
    def sigma(self, value):
        self._sigma = value
        
# tell the infrastructure to give us mstransformed data when possible by
# registering our preference for imaging measurement sets
basetask.ImagingMeasurementSetsPreferred.register(TcleanInputs)

class Tclean(cleanbase.CleanBase):
    Inputs = TcleanInputs

    def is_multi_vis_task(self):
        return True

    def copy_products(self, old_pname, new_pname):
        imlist = commands.getoutput('ls -d '+old_pname+'.*')
        imlist = imlist.split('\n')
        for image_name in imlist:
            newname = image_name.replace(old_pname, new_pname)
            if image_name == old_pname + '.workdirectory':
                mkcmd = 'mkdir '+ newname
                os.system(mkcmd)
                self.copy_products(os.path.join(image_name, old_pname), os.path.join(newname, new_pname))
            else:
                LOG.info('Copying %s to %s' % (image_name, newname))
                shutil.copytree(image_name, newname)

    def prepare(self):
        context = self.inputs.context
        inputs = self.inputs

        LOG.info('\nCleaning for intent "%s", field %s, spw %s\n',
                 inputs.intent, inputs.field, inputs.spw)

        result = None

        qaTool = casatools.quanta

        # delete any old files with this naming root. One of more
        # of these (don't know which) will interfere with this run.
        LOG.info('deleting %s*.iter*', inputs.imagename)
        shutil.rmtree('%s*.iter*' % inputs.imagename, ignore_errors=True)

        # Set initial masking limits
        self.pblimit_image = 0.2
        self.pblimit_cleanmask = 0.3
        inputs.pblimit = self.pblimit_image

        # Get the image parameter heuristics
        self.image_heuristics = inputs.image_heuristics

        # Generate the image name if one is not supplied.
        if inputs.imagename in ('', None):
            inputs.imagename = self.image_heuristics.imagename(intent=inputs.intent,
                                                      field=inputs.field,
                                                      spwspec=inputs.spw,
                                                      specmode=inputs.specmode)

        # Determine the default gridder
        if inputs.gridder in ('', None):
            inputs.gridder = self.image_heuristics.gridder(inputs.intent, inputs.field)

        # Determine deconvolver
        if inputs.deconvolver in ('', None):
            inputs.deconvolver = self.image_heuristics.deconvolver(inputs.specmode, inputs.spw)

        # Determine nterms
        if inputs.nterms in ('', None):
            if inputs.deconvolver == 'mtmfs':
                # ALMA heuristics
                inputs.nterms = 2

        # Determine the phase center
        if inputs.phasecenter in ('', None):
            field_id = self.image_heuristics.field(inputs.intent, inputs.field)
            inputs.phasecenter = self.image_heuristics.phasecenter(field_id)

        # If imsize not set then use heuristic code to calculate the
        # centers for each field  / spw
        imsize = inputs.imsize
        cell = inputs.cell
        if imsize == [] or cell == []:

            # The heuristics cell size  is always the same for x and y as
            # the value derives from a single value returned by imager.advise
            synthesized_beam = self.image_heuristics.synthesized_beam( \
                               field_intent_list=[(inputs.field, inputs.intent)], \
                               spwspec=inputs.spw, \
                               robust=inputs.robust, \
                               uvtaper=inputs.uvtaper)
            cell = self.image_heuristics.cell(beam=synthesized_beam)

            if inputs.cell == []:
                inputs.cell = cell
                LOG.info('Heuristic cell: %s' % cell)

            field_ids = self.image_heuristics.field(inputs.intent, inputs.field)
            largest_primary_beam = self.image_heuristics.largest_primary_beam_size(spwspec=inputs.spw)
            imsize = self.image_heuristics.imsize( \
                     fields=field_ids, \
                     cell=inputs.cell, \
                     primary_beam=largest_primary_beam)

            if inputs.imsize == []:
                inputs.imsize = imsize
                LOG.info('Heuristic imsize: %s', imsize)

        if (inputs.specmode == 'cube'):
            # To avoid noisy edge channels, use only the LSRK frequency
            # intersection and skip one channel on either end.
            if0, if1, channel_width = self.image_heuristics.lsrk_freq_intersection(inputs.vis, inputs.field, inputs.spw)

            if (if0 == -1) or (if1 == -1):
                LOG.error('No LSRK frequency intersect among selected MSs for Field %s SPW %s' % (inputs.field, inputs.spw))
                result.error = '%s/%s/spw%s clean error: %s' % (inputs.field, inputs.intent, inputs.spw)
                return result

            # Check for manually supplied values
            if0_auto = if0
            if1_auto = if1
            channel_width_auto = channel_width

            if inputs.start != '':
                if0 = qaTool.convert(inputs.start, 'Hz')['value']
                if if0 < if0_auto:
                    LOG.error('Supplied start frequency %s < f_low for Field %s SPW %s' % (inputs.start, inputs.field, inputs.spw))
                    result.error = '%s/%s/spw%s clean error: f_start < f_low_native' % (inputs.field, inputs.intent, inputs.spw)
                    return result
                LOG.info('Using supplied start frequency %s' % (inputs.start))

            if (inputs.width != '') and (inputs.nbin not in (None, -1)):
                LOG.error('Field %s SPW %s: width and nbin are mutually exclusive' % (inputs.field, inputs.spw))
                result.error = '%s/%s/spw%s clean error: width and nbin are mutually exclusive' % (inputs.field, inputs.intent, inputs.spw)
                return result

            if inputs.width != '':
                channel_width_manual = qaTool.convert(inputs.width, 'Hz')['value']
                if channel_width_manual < channel_width_auto:
                    LOG.error('User supplied channel width smaller than native value of %s GHz for Field %s SPW %s' % (channel_width_auto, inputs.field, inputs.spw))
                    result.error = '%s/%s/spw%s clean error: user channel width too small' % (inputs.field, inputs.intent, inputs.spw)
                    return result

                LOG.info('Using supplied width %s' % (inputs.width))
                channel_width = channel_width_manual
                if channel_width > channel_width_auto:
                    inputs.nbin = int(round(channel_width / channel_width_auto) + 0.5)
            elif inputs.nbin not in (None, -1):
                LOG.info('Applying binning factor %d' % (inputs.nbin))
                channel_width *= inputs.nbin

            if inputs.nchan not in (None, -1):
                if1 = if0 + channel_width * inputs.nchan
                if if1 > if1_auto:
                    LOG.error('Calculated stop frequency %s GHz > f_high_native for Field %s SPW %s' % (if1, inputs.field, inputs.spw))
                    result.error = '%s/%s/spw%s clean error: f_stop > f_high' % (inputs.field, inputs.intent, inputs.spw)
                    return result
                LOG.info('Using supplied nchan %d' % (inputs.nchan))

            # tclean interprets the start frequency as the center of the
            # first channel. We have, however, an edge to edge range.
            # Thus shift by 0.5 channels if no start is supplied.
            if inputs.start == '':
                inputs.start = '%sGHz' % ((if0 + 1.5 * channel_width) / 1e9)

            # Always adjust width to apply possible binning
            inputs.width = '%sMHz' % ((channel_width) / 1e6)

            # Skip edge channels if no nchan is supplied
            if inputs.nchan in (None, -1):
               inputs.nchan = int(round((if1 - if0) / channel_width - 2))

        # Make sure there are LSRK selections if cont.dat/lines.dat exist.
        # For ALMA this is already done at the hif_makeimlist step. For VLASS
        # this does not (yet) happen in hif_editimlist.
        if inputs.spwsel_lsrk == {}:
            for spwid in inputs.spw.split(','):
                spwsel_spwid = self.image_heuristics.cont_ranges_spwsel().get(utils.dequote(inputs.field), {}).get(spwid, 'NONE')
                if (inputs.intent == 'TARGET'):
                    if (spwsel_spwid == 'NONE'):
                        LOG.warn('No continuum frequency range information detected for %s, spw %s.' % (inputs.field, spwid))

                if spwsel_spwid in ('ALL', '', 'NONE'):
                    spwsel_spwid_freqs = ''
                    spwsel_spwid_refer = 'LSRK'
                else:
                    spwsel_spwid_freqs, spwsel_spwid_refer = spwsel_spwid.split()

                if (spwsel_spwid_refer != 'LSRK'):
                    LOG.warn('Frequency selection is specified in %s but must be in LSRK' % (spwsel_spwid_refer))

                inputs.spwsel_lsrk['spw%s' % (spwid)] = spwsel_spwid

        # Get TOPO frequency ranges for all MSs
        spw_topo_freq_param, \
        spw_topo_chan_param, \
        spw_topo_freq_param_dict, \
        spw_topo_chan_param_dict, \
        total_topo_bw, \
        aggregate_topo_bw, \
        aggregate_lsrk_bw = \
            self.image_heuristics.calc_topo_ranges(inputs)

        # Save continuum frequency ranges for later.
        if (inputs.specmode == 'cube') and (inputs.spwsel_lsrk.get('spw%s' % (inputs.spw), None) not in (None, 'NONE', '')):
            self.cont_freq_ranges = inputs.spwsel_lsrk['spw%s' % (inputs.spw)].split()[0]
        else:
            self.cont_freq_ranges = ''

        # Get sensitivity
        if inputs.sensitivity is not None:
            # Override with manually set value
            sensitivity = qaTool.convert(inputs.sensitivity, 'Jy')['value']
            # Dummies for weblog
            min_sensitivity = sensitivity
            max_sensitivity = sensitivity
            min_field_id = 0
            max_field_id = 0
            eff_ch_bw = 1.0
            sens_bw = 1.0
        else:
            # Get a noise estimate from the CASA sensitivity calculator
            sensitivity, \
            min_sensitivity, \
            max_sensitivity, \
            min_field_id, \
            max_field_id, \
            eff_ch_bw, \
            sens_bw = \
                self.image_heuristics.calc_sensitivities(inputs.vis, \
                                                         inputs.field, \
                                                         inputs.intent, \
                                                         inputs.spw, \
                                                         inputs.nbin, \
                                                         spw_topo_chan_param_dict, \
                                                         inputs.specmode, \
                                                         inputs.gridder, \
                                                         inputs.cell, \
                                                         inputs.imsize, \
                                                         inputs.weighting, \
                                                         inputs.robust, \
                                                         inputs.uvtaper)
        LOG.info('Sensitivity estimate: %s Jy', sensitivity)

        # Choose TOPO frequency selections
        if inputs.specmode != 'cube':
            inputs.spwsel_topo = spw_topo_freq_param
        else:
            inputs.spwsel_topo = ['%s' % (inputs.spw)] * len(inputs.vis)

        # Choose cleaning method.
        if inputs.hm_masking in ('centralregion', 'auto', 'manual', 'none'):
            # Determine threshold
            if inputs.hm_cleaning == 'manual':
                threshold = inputs.threshold
            elif inputs.hm_cleaning == 'sensitivity':
                raise Exception, 'sensitivity threshold not yet implemented'
            elif inputs.hm_cleaning == 'rms':
                if inputs.threshold not in (None, '', 0.0):
                    threshold = inputs.threshold
                else:
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
            elif inputs.hm_masking == 'manual':
                sequence_manager = ManualMaskThresholdSequence(
                    mask=inputs.mask,
                    gridder = inputs.gridder, threshold=threshold,
                    sensitivity = sensitivity, niter=inputs.niter)
            # No mask
            elif inputs.hm_masking == 'none':
                sequence_manager = NoMaskThresholdSequence(
                    gridder=inputs.gridder, threshold=threshold,
                    sensitivity=sensitivity, niter=inputs.niter)

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

        # Record aggregate LSRK bandwidth and mosaic field sensitivities for weblog
        # TODO: Record total bandwidth as opposed to range
        #       Save channel selection in result for weblog.
        result.set_aggregate_bw(aggregate_lsrk_bw)
        result.set_eff_ch_bw(eff_ch_bw)
        result.set_min_sensitivity(min_sensitivity)
        result.set_max_sensitivity(max_sensitivity)
        result.set_min_field_id(min_field_id)
        result.set_max_field_id(max_field_id)

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

        # Determine masking limits depending on PB
        extension = '.tt0' if result.multiterm else ''
        self.pblimit_image, self.pblimit_cleanmask = \
            self.image_heuristics.pblimits(result.flux+extension)
        inputs.pblimit = self.pblimit_image

        # Give the result to the sequence_manager for analysis
        model_sum, \
        residual_cleanmask_rms, \
        residual_non_cleanmask_rms, \
        residual_min, \
        residual_max, \
        nonpbcor_image_non_cleanmask_rms_min, \
        nonpbcor_image_non_cleanmask_rms_max, \
        nonpbcor_image_non_cleanmask_rms, \
        pbcor_image_min, \
        pbcor_image_max,\
        residual_robust_rms = \
            sequence_manager.iteration_result(iter=0, \
                multiterm=result.multiterm, psf=result.psf, model=result.model, \
                restored=result.image, residual=result.residual, \
                flux=result.flux, cleanmask=None, threshold=None, \
                pblimit_image=self.pblimit_image, \
                pblimit_cleanmask=self.pblimit_cleanmask, \
                cont_freq_ranges=self.cont_freq_ranges)

        LOG.info('Dirty image stats')
        LOG.info('    Residual rms: %s', residual_non_cleanmask_rms)
        LOG.info('    Residual max: %s', residual_max)
        LOG.info('    Residual min: %s', residual_min)
        LOG.info('    Residual scaled MAD: %s', residual_robust_rms)

        # Adjust threshold based on the dirty image statistics
        dirty_dynamic_range = residual_max / sequence_manager.sensitivity
        new_threshold, DR_correction_factor, maxEDR_used = \
            self.image_heuristics.dr_correction(sequence_manager.threshold, \
                                                dirty_dynamic_range, \
                                                residual_max, \
                                                inputs.intent, \
                                                inputs.tlimit)
        sequence_manager.threshold = new_threshold
        sequence_manager.dr_corrected_sensitivity = sequence_manager.sensitivity * DR_correction_factor

        # Adjust niter based on the dirty image statistics
        new_niter = self.image_heuristics.niter_correction(sequence_manager.niter, inputs.cell, inputs.imsize, residual_max, new_threshold)
        sequence_manager.niter = new_niter

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

            rms_threshold = self.image_heuristics.rms_threshold(residual_robust_rms, inputs.nsigma)
            if rms_threshold:
                sequence_manager.threshold = rms_threshold

            # perform an iteration.
            if (inputs.specmode == 'cube') and (not inputs.cleancontranges):
                seq_result = sequence_manager.iteration(new_cleanmask, self.pblimit_image, self.pblimit_cleanmask, inputs.spw, inputs.spwsel_lsrk)
            else:
                seq_result = sequence_manager.iteration(new_cleanmask, self.pblimit_image, self.pblimit_cleanmask)

            # Check the iteration status.
            if not seq_result.iterating:
                break

            # Use previous iterations's products as starting point
            old_pname = '%s.iter%s' % (rootname, iter-1)
            new_pname = '%s.iter%s' % (rootname, iter)
            self.copy_products(os.path.basename(old_pname), os.path.basename(new_pname))

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
            model_sum, \
            residual_cleanmask_rms, \
            residual_non_cleanmask_rms, \
            residual_min, \
            residual_max, \
            nonpbcor_image_non_cleanmask_rms_min, \
            nonpbcor_image_non_cleanmask_rms_max, \
            nonpbcor_image_non_cleanmask_rms, \
            pbcor_image_min, \
            pbcor_image_max, \
            residual_robust_rms = \
                sequence_manager.iteration_result(iter=iter, \
                    multiterm=result.multiterm, psf=result.psf, model=result.model, \
                    restored=result.image, residual=result.residual, \
                    flux=result.flux, cleanmask=new_cleanmask, threshold=seq_result.threshold, \
                    pblimit_image=self.pblimit_image, \
                    pblimit_cleanmask=self.pblimit_cleanmask, \
                    cont_freq_ranges=self.cont_freq_ranges)

            # Keep image cleanmask area min and max and non-cleanmask area RMS for weblog and QA
            result.set_image_min(pbcor_image_min)
            result.set_image_max(pbcor_image_max)
            result.set_image_rms(nonpbcor_image_non_cleanmask_rms)
            result.set_image_rms_min(nonpbcor_image_non_cleanmask_rms_min)
            result.set_image_rms_max(nonpbcor_image_non_cleanmask_rms_max)

            # Keep dirty DR, correction factor and information about maxEDR heuristic for weblog
            result.set_dirty_dynamic_range(dirty_dynamic_range)
            result.set_DR_correction_factor(DR_correction_factor)
            result.set_maxEDR_used(maxEDR_used)
            result.set_dr_corrected_sensitivity(sequence_manager.dr_corrected_sensitivity)

            LOG.info('Clean image iter %s stats' % iter)
            LOG.info('    Clean image non-cleanmask area rms: %s', nonpbcor_image_non_cleanmask_rms)
            LOG.info('    Clean image min: %s', pbcor_image_min)
            LOG.info('    Clean image max: %s', pbcor_image_max)
            LOG.info('    Residual non-cleanmask area rms: %s', residual_non_cleanmask_rms)
            LOG.info('    Residual cleanmask area rms: %s', residual_cleanmask_rms)
            LOG.info('    Residual max: %s', residual_max)
            LOG.info('    Residual min: %s', residual_min)

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
        # after continuum subtraction an image of the moment 0 / 8 integrated
        # intensity for the line-free channels.
        if inputs.specmode == 'cube':
            self._calc_mom0_8_fc(result)

        return result

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
                uvtaper=inputs.uvtaper,
                nterms=inputs.nterms,
                cycleniter=inputs.cycleniter,
                cyclefactor=inputs.cyclefactor,
                scales=inputs.scales,
                interactive=False, outframe=inputs.outframe, nchan=inputs.nchan,
                start=inputs.start, width=inputs.width, imsize=inputs.imsize,
                cell=inputs.cell, phasecenter=inputs.phasecenter,
                stokes=inputs.stokes,
                weighting=inputs.weighting,
                robust=inputs.robust,
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
                                                  reffreq=inputs.reffreq,
                                                  uvrange=inputs.uvrange,
                                                  specmode=inputs.specmode,
                                                  gridder=inputs.gridder,
                                                  deconvolver=inputs.deconvolver,
                                                  uvtaper=inputs.uvtaper,
                                                  nterms=inputs.nterms,
                                                  cycleniter=inputs.cycleniter,
                                                  cyclefactor=inputs.cyclefactor,
                                                  scales=inputs.scales,
                                                  outframe=inputs.outframe,
                                                  imsize=inputs.imsize,
                                                  cell=inputs.cell,
                                                  phasecenter=inputs.phasecenter,
                                                  stokes=inputs.stokes,
                                                  nchan=inputs.nchan,
                                                  start=inputs.start,
                                                  width=inputs.width,
                                                  weighting=inputs.weighting,
                                                  robust=inputs.robust,
                                                  noise=inputs.noise,
                                                  npixels=inputs.npixels,
                                                  restoringbeam=inputs.restoringbeam,
                                                  iter=iter,
                                                  mask=cleanmask,
                                                  hm_masking=inputs.hm_masking,
                                                  hm_sidelobethreshold=inputs.hm_sidelobethreshold,
                                                  hm_noisethreshold=inputs.hm_noisethreshold,
                                                  hm_lownoisethreshold=inputs.hm_lownoisethreshold,
                                                  hm_minbeamfrac=inputs.hm_minbeamfrac,
                                                  niter=niter,
                                                  threshold=threshold,
                                                  sensitivity=sensitivity,
                                                  pblimit=inputs.pblimit,
                                                  result=result,
                                                  parallel=parallel,
                                                  heuristics=inputs.image_heuristics)
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

    # Calculate a "mom0_fc" and "mom8_fc" image: this is a moment 0 and 8
    # integration over the line-free channels of the non-primary-beam
    # corrected image-cube, after continuum subtraction; where the "line-free"
    # channels are taken from those identified as continuum channels. 
    # This is a diagnostic plot representing the residual emission 
    # in the line-free (aka continuum) channels. If the continuum subtraction
    # worked well, then this image should just contain noise.
    def _calc_mom0_8_fc(self, result):
 
        # Find max iteration that was performed.
        maxiter = max(result.iterations.keys())

        # Get filename of image from result, and modify to select the  
        # non-PB-corrected image.
        imagename = result.iterations[maxiter]['image'].replace('.pbcor','')

        # Set output filename for MOM0_FC image.
        mom0_name = '%s.mom0_fc' % (imagename)

        # Set output filename for MOM8_FC image.
        mom8_name = '%s.mom8_fc' % (imagename)

        # Convert frequency ranges to channel ranges.
        cont_chan_ranges = utils.freq_selection_to_channels(imagename, self.cont_freq_ranges)

        # Only continue if there were continuum channel ranges for this spw.
        if cont_chan_ranges[0] != 'NONE':

            # Create a channel ranges string.
            cont_chan_ranges_str = ";".join(["%s~%s" % (ch0, ch1) for ch0, ch1 in cont_chan_ranges])

            # Execute job to create the MOM0_FC image.
            job = casa_tasks.immoments(imagename=imagename, moments=[0], outfile=mom0_name, chans=cont_chan_ranges_str)
            job.execute(dry_run=False)
            assert os.path.exists(mom0_name)

            # Update the metadata in the MOM0_FC image.
            cleanbase.set_miscinfo(name=mom0_name, spw=self.inputs.spw, 
              field=self.inputs.field, iter=maxiter, type='mom0_fc',
              intent=self.inputs.intent, specmode=self.inputs.specmode)

            # Update the result.
            result.set_mom0_fc(maxiter, mom0_name)

            # Execute job to create the MOM8_FC image.
            job = casa_tasks.immoments(imagename=imagename, moments=[8], outfile=mom8_name, chans=cont_chan_ranges_str)
            job.execute(dry_run=False)
            assert os.path.exists(mom8_name)

            # Update the metadata in the MOM8_FC image.
            cleanbase.set_miscinfo(name=mom8_name, spw=self.inputs.spw, 
              field=self.inputs.field, iter=maxiter, type='mom8_fc',
              intent=self.inputs.intent, specmode=self.inputs.specmode)

            # Update the result.
            result.set_mom8_fc(maxiter, mom8_name)
        else:
            LOG.warning('Cannot create MOM0_FC / MOM8_FC images for intent "%s", '
              'field %s, spw %s, no continuum ranges found.' %
              (self.inputs.intent, self.inputs.field, self.inputs.spw))
