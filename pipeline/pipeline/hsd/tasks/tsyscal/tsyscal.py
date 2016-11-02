from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.h.tasks.tsyscal.tsyscal as tsyscal

LOG = infrastructure.get_logger(__name__)


class TsyscalInputs(tsyscal.TsyscalInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, caltable=None,
                 chantol=None):
        self._init_properties(vars())


class Tsyscal(tsyscal.Tsyscal):
    Inputs = TsyscalInputs    
