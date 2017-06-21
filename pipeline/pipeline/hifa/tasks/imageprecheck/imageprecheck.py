from __future__ import absolute_import
import os


from pipeline.hifa.heuristics import imageprecheck
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks

LOG = infrastructure.get_logger(__name__)


class ImagePreCheckResults(basetask.Results):
    def __init__(self, robust, uvtaper, beams):
        super(ImagePreCheckResults, self).__init__()
        self.robust = robust
        self.uvtaper = uvtaper
        self.beams = beams

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

    def prepare(self):

        repr_ms = self.inputs.context.observing_run.get_ms(inputs.vis[0])
        repr_target = representative_ms.representative_target

        if repr_target == (None, None, None):
            LOG.info('ImagePreCheck: No representative target found.')
            return ImagePreCheckResults(0.5, '', [])
        else:
            imageprecheck_heuristics = imageprecheck.ImagePreCheckHeuristics(self.inputs)
            repr_source, repr_spw = repr_ms.get_representative_source_spw()
            return ImagePreCheckResults(0.5, '', [])


    def analyse(self, results):
        return results
