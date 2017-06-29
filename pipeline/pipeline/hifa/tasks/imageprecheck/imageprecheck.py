from __future__ import absolute_import
import os


from pipeline.hifa.heuristics import imageprecheck
from pipeline.hif.heuristics import imageparams_factory
from pipeline.h.tasks.common.sensitivity import Sensitivity
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.casatools as casatools
import pipeline.domain.measures as measures
import pipeline.infrastructure.imageparamsfilehandler as imageparamsfilehandler

LOG = infrastructure.get_logger(__name__)


class ImagePreCheckResults(basetask.Results):
    def __init__(self, have_real_repr_target=False, minAcceptableAngResolution='0.0arcsec', maxAcceptableAngResolution='0.0arcsec', hm_robust=0.5, hm_uvtaper='', sensitivities=[]):
        super(ImagePreCheckResults, self).__init__()
        self.have_real_repr_target = have_real_repr_target
        self.minAcceptableAngResolution = minAcceptableAngResolution
        self.maxAcceptableAngResolution = maxAcceptableAngResolution
        self.hm_robust = hm_robust
        self.hm_uvtaper = hm_uvtaper
        self.sensitivities = sensitivities

    def merge_with_context(self, context):
        """
        See :method:`~pipeline.infrastructure.api.Results.merge_with_context`
        """

        # Store imaging parameters in context
        context.imaging_parameters['robust'] = self.hm_robust
        context.imaging_parameters['uvtaper'] = self.hm_uvtaper

        # Write imageparams.dat file
        imageparams_filehandler = imageparamsfilehandler.ImageParamsFileHandler()
        imageparams_filehandler.write(self.hm_robust, self.hm_uvtaper)

        # Add sensitivities to be reported to AQUA
        # Note: for Cycle 5 we stay with robust=0.5. This will change for
        # Cycle 6 when we switch to the robust heuristic (self.hm_robust value).
        context.sensitivities.extend([s for s in self.sensitivities if s['robust']==0.5])

    def __repr__(self):
        return 'ImagePreCheckResults:\n\t{0}'.format(
            '\n\t'.join(['robust=%.2f' % (self.hm_robust), 'uvtaper=%s' % (self.hm_uvtaper)]))


class ImagePreCheckInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())


class ImagePreCheck(basetask.StandardTaskTemplate):
    Inputs = ImagePreCheckInputs

    def is_multi_vis_task(self):
        return True

    def prepare(self):

        inputs = self.inputs
        cqa = casatools.quanta

        repr_ms = self.inputs.context.observing_run.get_ms(inputs.vis[0])
        repr_target = repr_ms.representative_target

        if repr_target != (None, None, None):
            have_real_repr_target = True
            # Get representative source and spw
            repr_source, repr_spw = repr_ms.get_representative_source_spw()
        else:
            have_real_repr_target = False
            # Pick arbitrary source for pre-Cycle 5 data
            repr_source = [s.name for s in repr_ms.sources if 'TARGET' in s.intents][0]
            repr_spw_obj = repr_ms.get_spectral_windows()[0]
            repr_spw = repr_spw_obj.id
            repr_chan_obj = repr_spw_obj.channels[int(repr_spw_obj.num_channels/2)]
            repr_freq = cqa.quantity(float(repr_chan_obj.getCentreFrequency().convert_to(measures.FrequencyUnits.HERTZ).value), 'Hz')
            repr_bw = cqa.quantity(float(repr_chan_obj.getWidth().convert_to(measures.FrequencyUnits.HERTZ).value), 'Hz')
            repr_target = (repr_source, repr_freq, repr_bw)
            LOG.info('ImagePreCheck: No representative target found. Chosing %s SPW %d.' % (repr_source, repr_spw))

        # Get the array
        diameter = min([a.diameter for a in repr_ms.antennas])
        if diameter == 7.0:
            array = '7m'
        else:
            array = '12m'

        imageprecheck_heuristics = imageprecheck.ImagePreCheckHeuristics(self.inputs)
        image_heuristics_factory = imageparams_factory.ImageParamsHeuristicsFactory()
        image_heuristics = image_heuristics_factory.getHeuristics( \
            vislist = inputs.vis, \
            spw = str(repr_spw), \
            observing_run = inputs.context.observing_run, \
            imagename_prefix = inputs.context.project_structure.ousstatus_entity_id, \
            science_goals = inputs.context.project_performance_parameters, \
            contfile = inputs.context.contfile, \
            linesfile = inputs.context.linesfile, \
            imaging_mode = 'ALMA')

        physicalBW_of_1chan = float(inputs.context.observing_run.measurement_sets[0].get_spectral_window(repr_spw).channels[0].getWidth().convert_to(measures.FrequencyUnits.HERTZ).value)
        nbin = int(cqa.getvalue(cqa.convert(repr_target[2], 'Hz'))/physicalBW_of_1chan + 0.5)
        primary_beam_size = image_heuristics.largest_primary_beam_size(spwspec=str(repr_spw))
        field_ids = image_heuristics.field('TARGET', repr_source)
        cont_spw = ','.join([str(s.id) for s in inputs.context.observing_run.measurement_sets[0].get_spectral_windows()])

        beams = {}
        cells = {}
        imsizes = {}
        sensitivities = []
        for robust in [-0.5, 0.5, 2.0]:
            beams[robust] = image_heuristics.synthesized_beam([(repr_source, 'TARGET')], str(repr_spw), robust=robust)
            cells[robust] = image_heuristics.cell(beams[robust])
            imsizes[robust] = image_heuristics.imsize(field_ids, cells[robust], primary_beam_size, centreonly=True)

            # reprBW sensitivity
            sensitivity, min_sensitivity, max_sensitivity, min_field_id, max_field_id, eff_ch_bw, sens_bw = \
                image_heuristics.calc_sensitivities(inputs.vis, repr_source, 'TARGET', str(repr_spw), nbin, {}, 'cube', 'standard', cells[robust], imsizes[robust], 'briggs', robust)
            sensitivities.append(Sensitivity( \
                **{'array': array, \
                   'field': repr_source, \
                   'spw': str(repr_spw), \
                   'bandwidth': cqa.quantity(sens_bw, 'Hz'), \
                   'bwmode': 'reprBW', \
                   'beam': beams[robust], \
                   'cell': cells[robust], \
                   'robust': robust, \
                   'sensitivity': cqa.quantity(sensitivity, 'Jy/beam')}))

            # full cont sensitivity (no frequency ranges excluded)
            sensitivity, min_sensitivity, max_sensitivity, min_field_id, max_field_id, eff_ch_bw, sens_bw = \
                image_heuristics.calc_sensitivities(inputs.vis, repr_source, 'TARGET', cont_spw, nbin, {}, 'cont', 'standard', cells[robust], imsizes[robust], 'briggs', robust)
            sensitivities.append(Sensitivity( \
                **{'array': array, \
                   'field': repr_source, \
                   'spw': str(repr_spw), \
                   'bandwidth': cqa.quantity(sens_bw, 'Hz'), \
                   'bwmode': 'fullcont', \
                   'beam': beams[robust], \
                   'cell': cells[robust], \
                   'robust': robust, \
                   'sensitivity': cqa.quantity(sensitivity, 'Jy/beam')}))

        if have_real_repr_target:
            # Determine suggested robust value
            hm_robust, minAcceptableAngResolution, maxAcceptableAngResolution = image_heuristics.robust(beams[0.5])

            # Determine suggested UV taper value
            if hm_robust == 2.0:
                hm_uvtaper = image_heuristics.uvtaper(beams[2.0], cqa.quantity(minAcceptableAngResolution, 'arcsec'), cqa.quantity(maxAcceptableAngResolution, 'arcsec'))
            else:
                hm_uvtaper = []
        else:
            hm_robust = 0.5
            hm_uvtaper = []
            minAcceptableAngResolution = 0.0
            maxAcceptableAngResolution = 0.0

        return ImagePreCheckResults( \
                   have_real_repr_target, \
                   minAcceptableAngResolution=cqa.quantity(minAcceptableAngResolution, 'arcsec'), \
                   maxAcceptableAngResolution=cqa.quantity(maxAcceptableAngResolution, 'arcsec'), \
                   hm_robust=hm_robust, \
                   hm_uvtaper=hm_uvtaper, \
                   sensitivities=sensitivities)

    def analyse(self, results):
        return results
