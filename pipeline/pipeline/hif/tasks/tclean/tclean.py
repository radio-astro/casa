import os
import shutil
import numpy

from mpi4casa.MPIEnvironment import MPIEnvironment

import pipeline.infrastructure as infrastructure
from pipeline.infrastructure import basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.domain.measures as measures
from pipeline.hif.heuristics import tclean
from .basecleansequence import BaseCleanSequence
from .imagecentrethresholdsequence import ImageCentreThresholdSequence
from .iterativesequence import IterativeSequence
from .iterativesequence2 import IterativeSequence2
from . import cleanbase

LOG = infrastructure.get_logger(__name__)


class TcleanInputs(cleanbase.CleanBaseInputs):
    def __init__(self, context, output_dir=None, vis=None, imagename=None,
                 intent=None, field=None, spw=None, spwsel=None, uvrange=None, specmode=None,
                 gridder=None, deconvolver=None, outframe=None, imsize=None, cell=None,
                 phasecenter=None, nchan=None, start=None, width=None,
                 weighting=None, robust=None, noise=None, npixels=None,
                 restoringbeam=None, iter=None, mask=None, niter=None, threshold=None,
                 noiseimage=None, hm_masking=None, hm_cleaning=None, tlimit=None,
                 masklimit=None, maxncleans=None, parallel=None):
        self._init_properties(vars())
        self.heuristics = tclean.TcleanHeuristics(self.context, self.vis, self.spw)

    # Add extra getters and setters here
    hm_cleaning = basetask.property_with_default('hm_cleaning', 'rms')
    hm_masking = basetask.property_with_default('hm_masking', 'centralquarter')
    masklimit = basetask.property_with_default('masklimit', 4.0)
    tlimit = basetask.property_with_default('tlimit', 4.0)

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


class Tclean(cleanbase.CleanBase):
    Inputs = TcleanInputs

    def is_multi_vis_task(self):
        return True

    def prepare(self):
        inputs = self.inputs

        LOG.info('\nCleaning for intent "%s", field %s, spw %s\n',
                 inputs.intent, inputs.field, inputs.spw)

        # try:
        result = None

        # delete any old files with this naming root. One of more
        # of these (don't know which) will interfere with this run.
        LOG.info('deleting %s*.iter*', inputs.imagename)
        shutil.rmtree('%s*.iter*' % inputs.imagename, ignore_errors=True)

        # Get an empirical noise estimate by generating Q or V image.
        #    Assumes presence of XX and YY, or RR and LL
        #    Assumes source is unpolarized
        #    Make code more efficient (use MS XX and YY correlations) directly.
        #    Update / replace  code when sensitity function is working.
        #model_sum, cleaned_rms, non_cleaned_rms, residual_max, \
        #  residual_min, rms2d, image_max = \
        #  self._do_noise_estimate(stokes=inputs.noiseimage)
        #sensitivity = non_cleaned_rms
        #LOG.info('Noise rms estimate from %s image is %s' %
        #  (inputs.noiseimage, sensitivity))

        # Get a noise estimate from the CASA sensitivity calculator
        sensitivity = self._do_sensitivity()
        LOG.info('Sensitivity estimate from CASA %s', sensitivity)

        # Choose cleaning method.
        if inputs.hm_masking == 'centralquarter':
            if inputs.hm_cleaning == 'manual':
                threshold = inputs.threshold
            elif inputs.hm_cleaning == 'sensitivity':
                raise Exception, 'sensitivity threshold not yet implemented'
            elif inputs.hm_cleaning == 'rms':
                threshold = '%sJy' % (inputs.tlimit * sensitivity)
            sequence_manager = ImageCentreThresholdSequence(
                gridder=inputs.gridder, threshold=threshold,
                sensitivity=sensitivity)

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

        # except Exception, e:
        #     raise Exception, '%s/%s/SpW%s Iterative imaging error: %s' % (
        #         inputs.intent, inputs.field, inputs.spw, str(e))

        return result

    def analyse(self, result):
        # Perform QA here if this is a sub-task
        context = self.inputs.context
        pipelineqa.registry.do_qa(context, result)

        return result

    def _do_iterative_imaging(self, sequence_manager, result):
        # Compute the dirty image.
        LOG.info('Compute the dirty image')
        iter = 0
        result = self._do_clean(iter=iter, stokes='I', cleanmask='', niter=0,
                                threshold='0.0mJy',
                                sensitivity=sequence_manager.sensitivity,
                                result=None)

        # Give the result to the sequence_manager for analysis
        model_sum, cleaned_rms, non_cleaned_rms, residual_max, residual_min,\
            rms2d, image_max = sequence_manager.iteration_result(iter=0,
                    psf=result.psf, model=result.model,
                    restored=result.image, residual=result.residual,
                    flux=result.flux, cleanmask=None, threshold=None)

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
                shutil.rmtree(new_cleanmask)
            except OSError:
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

            result = self._do_clean(iter=iter, stokes='I',
                    cleanmask=new_cleanmask, niter=seq_result.niter,
                    threshold=threshold,
                    sensitivity=sequence_manager.sensitivity, result=result)

            # Give the result to the clean 'sequencer'
            model_sum, cleaned_rms, non_cleaned_rms, residual_max, residual_min, rms2d, image_max = sequence_manager.iteration_result(
                iter=iter, psf=result.psf, model=result.model, restored=result.image, residual=result.residual,
                flux=result.flux, cleanmask=new_cleanmask, threshold=seq_result.threshold)

            # Keep RMS for QA
            result.set_rms(non_cleaned_rms)

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
            sequence_manager.iteration_result(iter=0,
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
        imTool = casatools.imager
        for msName in [msInfo.name for msInfo in context.observing_run.measurement_sets]:
            for intSpw in map(int, spw.split(',')):
                imTool.open(msName)
                imTool.selectvis(spw=intSpw, field=field)
                imTool.defineimage(mode=specmode, spw=intSpw,
                                   cellx=inputs.cell[0], celly=inputs.cell[0],
                                   nx=inputs.imsize[0], ny=inputs.imsize[1])
                # TODO: Mosaic switch needed ?
                imTool.weight(type=inputs.weighting, robust=inputs.robust)

                try:
                    result = imTool.apparentsens()
                    sensitivities.append(result[1])
                except Exception as e:
                    sensitivities.append(0.01)
                    LOG.warning('Exception in calculating sensitivity. Assuming 0.01 Jy/beam.')
                imTool.close()

        sensitivity = 1.0/numpy.sqrt(numpy.sum(1.0/numpy.array(sensitivities)**2))

        if inputs.specmode == 'cube':
            if inputs.nchan != -1:
                sensitivity *= numpy.sqrt(inputs.nchan)
            else:
                qaTool = casatools.quanta
                ms = context.observing_run.measurement_sets[0]
                spwDesc = ms.get_spectral_window(spw)
                if qaTool.convert(inputs.width, 'GHz')['value'] == 0.0:
                    sensitivity *= numpy.sqrt(len(spwDesc.channels))
                else:
                    min_frequency = float(spwDesc.min_frequency.to_units(measures.FrequencyUnits.GIGAHERTZ))
                    max_frequency = float(spwDesc.max_frequency.to_units(measures.FrequencyUnits.GIGAHERTZ))
                    sensitivity *= numpy.sqrt(abs((max_frequency - min_frequency) /
                                                  qaTool.convert(inputs.width, 'GHz')['value']))

        return sensitivity

    def _do_clean(self, iter, stokes, cleanmask, niter, threshold, sensitivity, result):
        """
        Do basic cleaning.
        """
        inputs = self.inputs

        if inputs.specmode in ['mfs', 'cont']:
            specmode = 'mfs'
        elif inputs.specmode == 'cube':
            specmode = 'cube'
        else:
            raise Exception, 'Unknown specmode "%s"' % inputs.specmode

        # ensure we don't try to run Tier 1 tclean on an MPI server
        parallel = inputs.parallel and MPIEnvironment.is_mpi_client

        clean_inputs = cleanbase.CleanBase.Inputs(inputs.context,
                                                  output_dir=inputs.output_dir,
                                                  vis=inputs.vis,
                                                  imagename=inputs.imagename,
                                                  intent=inputs.intent,
                                                  field=inputs.field,
                                                  spw=inputs.spw,
                                                  spwsel=inputs.spwsel,
                                                  uvrange=inputs.uvrange,
                                                  specmode=specmode,
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

