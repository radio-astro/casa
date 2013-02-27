from __future__ import absolute_import

import pipeline.infrastructure.logging as logging
from . import common
from . import gaincalworker

LOG = logging.get_logger(__name__)


class GSplineGaincalInputs(common.CommonGaincalInputs):
    def __init__(self, context, output_dir=None,
                 # 
                 vis=None, caltable=None, 
                 # data selection arguments
                 field=None, spw=None, antenna=None, intent=None,
                 # solution parameters
                 smodel=None, calmode=None,  refant=None, splinetime=None,
                 npointaver=None, append=None, phasewrap=None,
                 # preapply calibrations
                 gaincurve=None, opacity=None, parang=None,
                 # calibration target
                 to_intent=None, to_field=None):
        self._init_properties(vars())

    @property
    def gaintype(self):
        return 'GSPLINE' 

    @property
    def splinetime(self):
        if self._splinetime is not None:
            return self._splinetime
        return 3600

    @splinetime.setter
    def splinetime(self, value):
        self._splinetime = value

    @property
    def npointaver(self):
        if self._npointaver is not None:
            return self._npointaver
        return 3

    @npointaver.setter
    def npointaver(self, value):
        self._npointaver = value

    @property
    def phasewrap(self):
        if self._phasewrap is not None:
            return self._phasewrap
        return 180

    @phasewrap.setter
    def phasewrap(self, value):
        self._phasewrap = value


class GSplineGaincal(gaincalworker.GaincalWorker):
    Inputs = GSplineGaincalInputs
