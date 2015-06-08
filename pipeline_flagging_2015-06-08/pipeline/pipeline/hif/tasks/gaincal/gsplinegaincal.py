from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure as infrastructure
from . import common
from . import gaincalworker

LOG = infrastructure.get_logger(__name__)



class GSplineGaincalInputs(common.CommonGaincalInputs):
    # simple vis-independent properties with default values
    npointaver = basetask.property_with_default('npointaver', 3)
    phasewrap  = basetask.property_with_default('phasewrap', 180)
    splinetime = basetask.property_with_default('splinetime', 3600)

    @property
    def gaintype(self):
        return 'GSPLINE' 

    def __init__(self, context, output_dir=None,
                 # 
                 vis=None, caltable=None, 
                 # data selection arguments
                 field=None, spw=None, antenna=None, intent=None,
                 # solution parameters
                 smodel=None, calmode=None,  refant=None, splinetime=None,
                 npointaver=None, append=None, phasewrap=None,
                 # preapply calibrations
                 opacity=None, parang=None,
                 # calibration target
                 to_intent=None, to_field=None):
        self._init_properties(vars())



class GSplineGaincal(gaincalworker.GaincalWorker):
    Inputs = GSplineGaincalInputs
