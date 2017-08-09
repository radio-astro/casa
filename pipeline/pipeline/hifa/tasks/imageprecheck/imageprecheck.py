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
    def __init__(self, real_repr_target=False, repr_target='', repr_source='', repr_spw=None, minAcceptableAngResolution='0.0arcsec', maxAcceptableAngResolution='0.0arcsec', hm_robust=0.5, hm_uvtaper='', sensitivities=[], sensitivity_bandwidth=None):
        super(ImagePreCheckResults, self).__init__()
        self.real_repr_target = real_repr_target
        self.repr_target = repr_target
        self.repr_source = repr_source
        self.repr_spw = repr_spw
        self.minAcceptableAngResolution = minAcceptableAngResolution
        self.maxAcceptableAngResolution = maxAcceptableAngResolution
        self.hm_robust = hm_robust
        self.hm_uvtaper = hm_uvtaper
        self.sensitivities = sensitivities
        self.sensitivity_bandwidth = sensitivity_bandwidth

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
        # Cycle 6 when we switch to the robust heuristic (self.hm_robust
        # and sel.hm_uvtaper values).
        #context.sensitivities.extend([s for s in self.sensitivities if s['robust']==self.hm_robust and s['uvtaper']==self.hm_uvtaper])
        context.sensitivities.extend([s for s in self.sensitivities if s['robust']==0.5 and s['uvtaper']==[]])

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

        imageprecheck_heuristics = imageprecheck.ImagePreCheckHeuristics(self.inputs)
        image_heuristics_factory = imageparams_factory.ImageParamsHeuristicsFactory()
        image_heuristics = image_heuristics_factory.getHeuristics( \
            vislist = inputs.vis, \
            spw = '', \
            observing_run = inputs.context.observing_run, \
            imagename_prefix = inputs.context.project_structure.ousstatus_entity_id, \
            proj_params = inputs.context.project_performance_parameters, \
            contfile = inputs.context.contfile, \
            linesfile = inputs.context.linesfile, \
            imaging_mode = 'ALMA')

        repr_target, repr_source, repr_spw, reprBW_mode, real_repr_target, minAcceptableAngResolution, maxAcceptableAngResolution = image_heuristics.representative_target()

        repr_field = list(image_heuristics.field_intent_list('TARGET', repr_source))[0][0]

        repr_ms = self.inputs.context.observing_run.get_ms(inputs.vis[0])

        # Get the array
        diameter = min([a.diameter for a in repr_ms.antennas])
        if diameter == 7.0:
            array = '7m'
        else:
            array = '12m'

        # Approximate reprBW with nbin
        if reprBW_mode == 'cube':
            physicalBW_of_1chan = float(inputs.context.observing_run.measurement_sets[0].get_spectral_window(repr_spw).channels[0].getWidth().convert_to(measures.FrequencyUnits.HERTZ).value)
            nbin = int(cqa.getvalue(cqa.convert(repr_target[2], 'Hz'))/physicalBW_of_1chan + 0.5)
            cont_sens_bw_modes = ['aggBW']
        else:
            nbin = -1
            cont_sens_bw_modes = ['repBW', 'aggBW']

        primary_beam_size = image_heuristics.largest_primary_beam_size(spwspec=str(repr_spw))
        field_ids = image_heuristics.field('TARGET', repr_field, exclude_field='ATMOSPHERE')
        gridder = image_heuristics.gridder('TARGET', repr_field)
        if gridder == 'mosaic':
            # Only pass phase center for mosaics to avoid filtering single fields with
            # TARGET and ATMOSPHERE intents (CAS-10146).
            phasecenter = image_heuristics.phasecenter(field_ids)
        else:
            phasecenter = None
        cont_spw = ','.join([str(s.id) for s in inputs.context.observing_run.measurement_sets[0].get_spectral_windows()])
        num_cont_spw = len(cont_spw.split(','))

        beams = {}
        cells = {}
        imsizes = {}
        sensitivities = []
        sensitivity_bandwidth = None
        for robust in [-0.5, 0.5, 2.0]:
            beams[(robust, '[]')] = image_heuristics.synthesized_beam([(repr_field, 'TARGET')], str(repr_spw), robust=robust, uvtaper=[])
            cells[(robust, '[]')] = image_heuristics.cell(beams[(robust, '[]')])
            imsizes[(robust, '[]')] = image_heuristics.imsize(field_ids, cells[(robust, '[]')], primary_beam_size, centreonly=False)

            # reprBW sensitivity
            if reprBW_mode == 'cube':
                sensitivity, min_sensitivity, max_sensitivity, min_field_id, max_field_id, eff_ch_bw, sens_bw = \
                    image_heuristics.calc_sensitivities(inputs.vis, repr_field, 'TARGET', str(repr_spw), nbin, {}, 'cube', gridder, cells[(robust, '[]')], imsizes[(robust, '[]')], 'briggs', robust, [], phasecenter)
                sensitivities.append(Sensitivity( \
                    **{'array': array, \
                       'field': repr_field, \
                       'spw': str(repr_spw), \
                       'bandwidth': cqa.quantity(sens_bw, 'Hz'), \
                       'bwmode': 'repBW', \
                       'beam': beams[(robust, '[]')], \
                       'cell': [cqa.convert(cells[(robust, '[]')][0], 'arcsec'), cqa.convert(cells[(robust, '[]')][0], 'arcsec')], \
                       'robust': robust, \
                       'uvtaper': [], \
                       'sensitivity': cqa.quantity(sensitivity, 'Jy/beam')}))
                sensitivity_bandwidth = cqa.quantity(sens_bw, 'Hz')

            # full cont sensitivity (no frequency ranges excluded)
            sensitivity, min_sensitivity, max_sensitivity, min_field_id, max_field_id, eff_ch_bw, sens_bw = \
                image_heuristics.calc_sensitivities(inputs.vis, repr_field, 'TARGET', cont_spw, -1, {}, 'cont', gridder, cells[(robust, '[]')], imsizes[(robust, '[]')], 'briggs', robust, [], phasecenter)
            for cont_sens_bw_mode in cont_sens_bw_modes:
                sensitivities.append(Sensitivity( \
                    **{'array': array, \
                       'field': repr_field, \
                       'spw': str(repr_spw), \
                       'bandwidth': cqa.quantity(min(sens_bw, num_cont_spw * 1.875e9), 'Hz'), \
                       'bwmode': cont_sens_bw_mode, \
                       'beam': beams[(robust, '[]')], \
                       'cell': [cqa.convert(cells[(robust, '[]')][0], 'arcsec'), cqa.convert(cells[(robust, '[]')][0], 'arcsec')], \
                       'robust': robust, \
                       'uvtaper': [], \
                       'sensitivity': cqa.quantity(sensitivity, 'Jy/beam')}))

            if sensitivity_bandwidth is None:
                sensitivity_bandwidth = cqa.quantity(sens_bw, 'Hz')

        if real_repr_target:
            # Determine heuristic robust value
            hm_robust = image_heuristics.robust(beams[(0.5, '[]')])

            # Determine heuristic UV taper value
            if hm_robust == 2.0:
                hm_uvtaper = image_heuristics.uvtaper(beams[(2.0, '[]')])
                # For ALMA Cycle 5 the additional beam, cell and sensitivity values for a different
                # uvtaper are not to be calculated or shown.
                if False and (hm_uvtaper != []):
                    # Add sensitivity entries with actual tapering
                    beams[(hm_robust, str(hm_uvtaper))] = image_heuristics.synthesized_beam([(repr_field, 'TARGET')], str(repr_spw), robust=hm_robust, uvtaper=hm_uvtaper)
                    cells[(hm_robust, str(hm_uvtaper))] = image_heuristics.cell(beams[(hm_robust, str(hm_uvtaper))])
                    imsizes[(hm_robust, str(hm_uvtaper))] = image_heuristics.imsize(field_ids, cells[(hm_robust, str(hm_uvtaper))], primary_beam_size, centreonly=False)
                    if reprBW_mode == 'cube':
                        sensitivity, min_sensitivity, max_sensitivity, min_field_id, max_field_id, eff_ch_bw, sens_bw = \
                            image_heuristics.calc_sensitivities(inputs.vis, repr_field, 'TARGET', str(repr_spw), nbin, {}, 'cube', gridder, cells[(hm_robust, str(hm_uvtaper))], imsizes[(hm_robust, str(hm_uvtaper))], 'briggs', hm_robust, hm_uvtaper, phasecenter)
                        sensitivities.append(Sensitivity( \
                            **{'array': array, \
                               'field': repr_field, \
                               'spw': str(repr_spw), \
                               'bandwidth': cqa.quantity(sens_bw, 'Hz'), \
                               'bwmode': 'repBW', \
                               'beam': beams[(hm_robust, str(hm_uvtaper))], \
                               'cell': [cqa.convert(cells[(hm_robust, str(hm_uvtaper))][0], 'arcsec'), cqa.convert(cells[(hm_robust, str(hm_uvtaper))][0], 'arcsec')], \
                               'robust': hm_robust, \
                               'uvtaper': hm_uvtaper, \
                               'sensitivity': cqa.quantity(sensitivity, 'Jy/beam')}))

                    sensitivity, min_sensitivity, max_sensitivity, min_field_id, max_field_id, eff_ch_bw, sens_bw = \
                        image_heuristics.calc_sensitivities(inputs.vis, repr_field, 'TARGET', cont_spw, -1, {}, 'cont', gridder, cells[(hm_robust, str(hm_uvtaper))], imsizes[(hm_robust, str(hm_uvtaper))], 'briggs', hm_robust, hm_uvtaper, phasecenter)
                    for cont_sens_bw_mode in cont_sens_bw_modes:
                        sensitivities.append(Sensitivity( \
                            **{'array': array, \
                               'field': repr_field, \
                               'spw': str(repr_spw), \
                               'bandwidth': cqa.quantity(min(sens_bw, num_cont_spw * 1.875e9), 'Hz'), \
                               'bwmode': cont_sens_bw_mode, \
                               'beam': beams[(hm_robust, str(hm_uvtaper))], \
                               'cell': [cqa.convert(cells[(hm_robust, str(hm_uvtaper))][0], 'arcsec'), cqa.convert(cells[(hm_robust, str(hm_uvtaper))][0], 'arcsec')], \
                               'robust': hm_robust, \
                               'uvtaper': hm_uvtaper, \
                               'sensitivity': cqa.quantity(sensitivity, 'Jy/beam')}))
            else:
                hm_uvtaper = []
        else:
            hm_robust = 0.5
            hm_uvtaper = []
            minAcceptableAngResolution = cqa.quantity(0.0, 'arcsec')
            maxAcceptableAngResolution = cqa.quantity(0.0, 'arcsec')

        # For ALMA Cycle 5 the heuristics results should just be logged, but not carried along.
        LOG.info('Heuristics would have chosen robust=%.1f, uvtaper=%s. Overriding to 0.5 / [] for Cycle 5.' % (hm_robust, hm_uvtaper))
        hm_robust = 0.5
        hm_uvtaper = []

        return ImagePreCheckResults( \
                   real_repr_target, \
                   repr_target, \
                   repr_source, \
                   repr_spw, \
                   minAcceptableAngResolution=minAcceptableAngResolution, \
                   maxAcceptableAngResolution=maxAcceptableAngResolution, \
                   hm_robust=hm_robust, \
                   hm_uvtaper=hm_uvtaper, \
                   sensitivities=sensitivities, \
                   sensitivity_bandwidth=sensitivity_bandwidth)

    def analyse(self, results):
        return results
