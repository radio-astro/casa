from __future__ import absolute_import

import pipeline.infrastructure.logging as logging
from . import common
from . import gaincalworker

LOG = logging.get_logger(__name__)


class GTypeGaincalInputs(common.CommonGaincalInputs):
    def __init__(self, context, output_dir=None,
                 # 
                 vis=None, caltable=None, 
                 # data selection arguments
                 field=None, spw=None, antenna=None, intent=None,
                 # solution parameters
                 smodel=None, calmode=None, solint=None, combine=None, 
                 refant=None, minblperant=None, minsnr=None, solnorm=None, 
                 append=None,
                 # preapply calibrations
                 gaincurve=None, opacity=None, parang=None,
                 # calibration target
                 to_intent=None, to_field=None):
        self._init_properties(vars())

    @property
    def gaintype (self):
        return 'G'

    @property
    def solnorm(self):
        return self._solnorm

    @solnorm.setter
    def solnorm(self, value):
        if value is None:
            value = False
        self._solnorm = value

    @property
    def solint(self):
        return self._solint

    @solint.setter
    def solint(self, value):
        if value is None:
            value = 'inf'
        self._solint = value

    @property
    def combine(self):
        return self._combine

    @combine.setter
    def combine(self, value):
        if value is None:
            value = ''
        self._combine = value

    @property
    def minsnr(self):
        return self._minsnr

    @minsnr.setter
    def minsnr(self, value):
        if value is None:
            value = 3
        self._minsnr = value


class GTypeGaincal(gaincalworker.GaincalWorker):
    Inputs = GTypeGaincalInputs
