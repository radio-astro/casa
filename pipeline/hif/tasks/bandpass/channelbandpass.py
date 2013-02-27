from __future__ import absolute_import

import pipeline.infrastructure.logging as logging
from . import common
from . import bandpassworker

LOG = logging.get_logger(__name__)


class ChannelBandpassInputs(common.CommonBandpassInputs):
    def __init__(self, context, output_dir=None,
                 #
                 vis=None, caltable=None, 
                 # data selection arguments
                 field=None, spw=None, antenna=None, intent=None,
                 # solution parameters
                 solint=None, combine=None, refant=None, minblperant=None,
                 minsnr=None, solnorm=None, fillgaps=None, append=None,
                 # preapply calibrations
                 gaincurve=None, opacity=None, parang=None,
                 # calibration target
                 to_intent=None, to_field=None):
        self._init_properties(vars())

    @property
    def bandtype(self):
        return 'B'

    @property
    def minsnr(self):
        return self._minsnr

    @minsnr.setter
    def minsnr(self, value):
        if value is None:
            value = 3.0
        self._minsnr = value


class ChannelBandpass(bandpassworker.BandpassWorker):
    Inputs = ChannelBandpassInputs
