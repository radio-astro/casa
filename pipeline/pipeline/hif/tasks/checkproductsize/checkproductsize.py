from __future__ import absolute_import

from pipeline.hif.heuristics import checkproductsize
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.api as api
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.project as project
import pipeline.infrastructure.vdp as vdp
from .resultobjects import CheckProductSizeResult

LOG = infrastructure.get_logger(__name__)


class CheckProductSizeInputs(vdp.StandardInputs):
    parallel = vdp.VisDependentProperty(default='automatic')

    @vdp.VisDependentProperty(null_input=[None, '', -1, -1.0])
    def maxcubelimit(self):
        return project.PerformanceParameters().max_cube_size

    @vdp.VisDependentProperty(null_input=[None, '', -1, -1.0])
    def maxcubesize(self):
        return project.PerformanceParameters().max_cube_size

    @vdp.VisDependentProperty(null_input=[None, '', -1, -1.0])
    def maxproductsize(self):
        return project.PerformanceParameters().max_product_size

    def __init__(self, context, output_dir=None, vis=None, maxcubesize=None, maxcubelimit=None, maxproductsize=None,
                 parallel=None):
        super(CheckProductSizeInputs, self).__init__()

        self.context = context
        self.output_dir = output_dir
        self.vis = vis

        self.maxcubesize = maxcubesize
        self.maxcubelimit = maxcubelimit
        self.maxproductsize = maxproductsize
        self.parallel = parallel


# tell the infrastructure to give us mstransformed data when possible by
# registering our preference for imaging measurement sets
api.ImagingMeasurementSetsPreferred.register(CheckProductSizeInputs)


class CheckProductSize(basetask.StandardTaskTemplate):
    Inputs = CheckProductSizeInputs

    is_multi_vis_task = True

    def prepare(self):
        checkproductsize_heuristics = checkproductsize.CheckProductSizeHeuristics(self.inputs)

        # Clear any previous size mitigation parameters
        self.inputs.context.size_mitigation_parameters = {}

        size_mitigation_parameters, \
        original_maxcubesize, original_productsize, \
        cube_mitigated_productsize, \
        maxcubesize, productsize, error, reason = \
            checkproductsize_heuristics.mitigate_sizes()

        if error:
            status = 'ERROR'
        elif size_mitigation_parameters != {}:
            status = 'MITIGATED'
        else:
            status = 'OK'

        size_mitigation_parameters['status'] = status

        result = CheckProductSizeResult(self.inputs.maxcubesize,
                                        self.inputs.maxcubelimit,
                                        self.inputs.maxproductsize,
                                        original_maxcubesize,
                                        original_productsize,
                                        cube_mitigated_productsize,
                                        maxcubesize,
                                        productsize,
                                        size_mitigation_parameters,
                                        status,
                                        reason)

        # Log summary information
        LOG.info(str(result))

        return result

    def analyse(self, result):
        return result
