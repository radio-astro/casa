from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.logging as logging
from . import channelbandpass
from . import polynomialbandpass

LOG = logging.get_logger(__name__)


class BandpassModeInputs(basetask.ModeInputs):
    _modes = {'channel' : channelbandpass.ChannelBandpass,
              'poly'    : polynomialbandpass.PolynomialBandpass}

    def __init__(self, context, mode='channel', **parameters):        
        super(BandpassModeInputs, self).__init__(context, mode, **parameters)


class BandpassMode(basetask.ModeTask):
    Inputs = BandpassModeInputs
