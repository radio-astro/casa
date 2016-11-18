from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.basetask as basetask


LOG = infrastructure.get_logger(__name__)

class CheckProductSizeResult(basetask.Results):
    def __init__(self, allowedcubesize, allowedproductsize, maxcubesize, productsize, size_mitigation_parameters, status, reason):
        super(CheckProductSizeResult, self).__init__()
        self.maxcubesize = maxcubesize
        self.productsize = productsize
        self.allowedcubesize = allowedcubesize
        self.allowedproductsize = allowedproductsize
        self.size_mitigation_parameters = size_mitigation_parameters
        self.status = status
        self.reason = reason

    def merge_with_context(self, context):
        # Store mitigation parameters for subsequent hif_makeimlist calls.
        context.size_mitigation_parameters = self.size_mitigation_parameters

    def __repr__(self):
        repr = 'CheckProductSize:\n'
        repr += ' Status: %s\n' % (self.status)
        repr += ' Reason: %s\n' % (self.reason['longmsg'])
        repr += ' Max cube size: %s GB\n' % (self.maxcubesize)
        repr += ' Product size: %s GB\n' % (self.productsize)
        repr += ' Mitigation parameters:\n'
        for parameter, value in self.size_mitigation_parameters.iteritems():
            repr += '  %s: %s\n' % (parameter, value)

        return repr
