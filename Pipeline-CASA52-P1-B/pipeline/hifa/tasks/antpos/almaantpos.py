from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

import pipeline.hif.tasks.antpos.antpos as antpos

LOG = infrastructure.get_logger(__name__)


class ALMAAntposInputs(antpos.AntposInputs):
    # These are ALMA specific settings and override the defaults in
    # the base class. 
    # Force the offset input to be from a file
    hm_antpos  = basetask.property_with_default('hm_antpos', 'file')

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, caltable=None,
                 hm_antpos=None, antposfile=None, antenna=None,
                 offsets=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())


class ALMAAntpos(antpos.Antpos):
    Inputs = ALMAAntposInputs
