from __future__ import absolute_import

from pipeline.hif.heuristics import checkproductsize
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.api as api
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.project as project
from .resultobjects import CheckProductSizeResult

LOG = infrastructure.get_logger(__name__)


class CheckProductSizeInputs(basetask.StandardInputs):
    parallel = basetask.property_with_default('parallel', 'automatic')

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None,
                 maxcubesize=None, maxcubelimit=None, maxproductsize=None, parallel=None):
        self.performanceparameters = project.PerformanceParameters()
        self._init_properties(vars())

    @property
    def maxcubesize(self):
        if self._maxcubesize in (None, -1.0):
            return self.performanceparameters.max_cube_size
        else:
            return self._maxcubesize

    @maxcubesize.setter
    def maxcubesize(self, value):
        self._maxcubesize = value

    @property
    def maxcubelimit(self):
        if self._maxcubelimit in (None, -1.0):
            return self.maxcubesize
        else:
            return self._maxcubelimit

    @maxcubelimit.setter
    def maxcubelimit(self, value):
        self._maxcubelimit = value

    @property
    def maxproductsize(self):
        if self._maxproductsize in (None, -1.0):
            return self.performanceparameters.max_product_size
        else:
            return self._maxproductsize

    @maxproductsize.setter
    def maxproductsize(self, value):
        self._maxproductsize = value



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

        result = CheckProductSizeResult(self.inputs.maxcubesize, \
                                        self.inputs.maxcubelimit, \
                                        self.inputs.maxproductsize, \
                                        original_maxcubesize, \
                                        original_productsize, \
                                        cube_mitigated_productsize, \
                                        maxcubesize, \
                                        productsize, \
                                        size_mitigation_parameters, \
                                        status, \
                                        reason)

        # Log summary information
        LOG.info(str(result))

        return result

    def analyse(self, result):
        return result
