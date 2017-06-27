from __future__ import absolute_import
import os


from pipeline.hifa.heuristics import imageprecheck
from pipeline.hif.heuristics import imageparams_factory
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.casatools as casatools

LOG = infrastructure.get_logger(__name__)


class ImagePreCheckResults(basetask.Results):
    def __init__(self, robust=0.5, uvtaper='', beams={}, sensitivities={}):
        super(ImagePreCheckResults, self).__init__()
        self.robust = robust
        self.uvtaper = uvtaper
        self.beams = beams
        self.sensitivities = sensitivities

    def merge_with_context(self, context):
        """
        See :method:`~pipeline.infrastructure.api.Results.merge_with_context`
        """

        # writing imageprecheck.out

    def __repr__(self):
        #return 'ImagePreCheckResults:\n\t{0}'.format(
        #    '\n\t'.join([ms.name for ms in self.mses]))
        return 'ImagePreCheckResults:'


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

        if repr_target == (None, None, None):
            LOG.info('ImagePreCheck: No representative target found.')
            return ImagePreCheckResults( \
                       beams = {-0.5: None, 0.5: None, 2.0: None}, \
                       sensitivities = {'reprBW': {-0.5: None, 0.5: None, 2.0: None}, \
                                        'cont': {-0.5: None, 0.5: None, 2.0: None}})
        else:
            # Get representative source and spw
            repr_source, repr_spw = repr_ms.get_representative_source_spw()

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

            # Robust heuristic

            # UV taper heuristic

            # Beams
            beam_m0p5 = image_heuristics.synthesized_beam([(repr_source, 'TARGET')], str(repr_spw), robust=-0.5)
            beam_0p5 = image_heuristics.synthesized_beam([(repr_source, 'TARGET')], str(repr_spw), robust=0.5)
            beam_2p0 = image_heuristics.synthesized_beam([(repr_source, 'TARGET')], str(repr_spw), robust=2.0)

            #print 'DEBUG_DM: beams:', beam_m0p5, beam_0p5, beam_2p0

            # Sensitivities
            primary_beam_size = image_heuristics.largest_primary_beam_size(spwspec=str(repr_spw))
            field_ids = image_heuristics.field('TARGET', repr_source)
            cont_spw = ','.join([str(s.id) for s in inputs.context.observing_run.measurement_sets[0].get_spectral_windows()])

            cell_m0p5 = image_heuristics.cell(beam_m0p5)
            imsize_m0p5 = image_heuristics.imsize(field_ids, cell_m0p5, primary_beam_size, centreonly=True)
            sens_m0p5 = image_heuristics.calc_sensitivities(inputs.vis, repr_source, 'TARGET', str(repr_spw), -1, {}, 'cube', 'standard', cell_m0p5, imsize_m0p5, 'briggs', -0.5)
            sens_cont_m0p5 = image_heuristics.calc_sensitivities(inputs.vis, repr_source, 'TARGET', cont_spw, -1, {}, 'cont', 'standard', cell_m0p5, imsize_m0p5, 'briggs', -0.5)

            cell_0p5 = image_heuristics.cell(beam_0p5)
            imsize_0p5 = image_heuristics.imsize(field_ids, cell_0p5, primary_beam_size, centreonly=True)
            sens_0p5 = image_heuristics.calc_sensitivities(inputs.vis, repr_source, 'TARGET', str(repr_spw), -1, {}, 'cube', 'standard', cell_0p5, imsize_0p5, 'briggs', 0.5)
            sens_cont_0p5 = image_heuristics.calc_sensitivities(inputs.vis, repr_source, 'TARGET', cont_spw, -1, {}, 'cont', 'standard', cell_0p5, imsize_0p5, 'briggs', 0.5)

            cell_2p0 = image_heuristics.cell(beam_2p0)
            imsize_2p0 = image_heuristics.imsize(field_ids, cell_2p0, primary_beam_size, centreonly=True)
            sens_2p0 = image_heuristics.calc_sensitivities(inputs.vis, repr_source, 'TARGET', str(repr_spw), -1, {}, 'cube', 'standard', cell_2p0, imsize_2p0, 'briggs', 2.0)
            sens_cont_2p0 = image_heuristics.calc_sensitivities(inputs.vis, repr_source, 'TARGET', cont_spw, -1, {}, 'cont', 'standard', cell_2p0, imsize_2p0, 'briggs', 2.0)

            #print 'DEBUG_DM: sensitivities:', sens_m0p5, sens_0p5, sens_2p0, sens_cont_m0p5, sens_cont_0p5, sens_cont_2p0

            return ImagePreCheckResults( \
                       beams = {-0.5: beam_m0p5, 0.5: beam_0p5, 2.0: beam_2p0}, \
                       sensitivities = {'reprBW': {-0.5: sens_m0p5, 0.5: sens_0p5, 2.0: sens_2p0}, \
                                        'cont': {-0.5: sens_cont_m0p5, 0.5: sens_cont_0p5, 2.0: sens_cont_2p0}})


    def analyse(self, results):
        return results
