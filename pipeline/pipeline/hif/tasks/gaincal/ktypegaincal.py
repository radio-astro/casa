from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure as infrastructure
from . import common
from . import gaincalworker

LOG = infrastructure.get_logger(__name__)



class KTypeGaincalInputs(common.CommonGaincalInputs):
    # simple vis-independent properties with default values
    combine = basetask.property_with_default('combine', '')
    minsnr  = basetask.property_with_default('minsnr', 3)
    solint  = basetask.property_with_default('solint', 'int')
    solnorm = basetask.property_with_default('solnorm', False)
    gaintype = basetask.property_with_default('gaintype', 'K')

    def __init__(self, context, output_dir=None,
                 # 
                 vis=None, caltable=None, 
                 # data selection arguments
                 field=None, spw=None, antenna=None, intent=None,
                 # solution parameters
                 smodel=None, gaintype=None, calmode=None, solint=None,
		 combine=None, refant=None, minblperant=None, minsnr=None,
		 solnorm=None, append=None, scan=None,
                 # preapply calibrations
                 opacity=None, parang=None,
                 # calibration target
                 to_intent=None, to_field=None):
        self._init_properties(vars())


class KTypeGaincal(gaincalworker.GaincalWorker):
    Inputs = KTypeGaincalInputs
