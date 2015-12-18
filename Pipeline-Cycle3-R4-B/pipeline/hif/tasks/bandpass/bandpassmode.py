from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure as infrastructure
from . import channelbandpass
from . import polynomialbandpass

LOG = infrastructure.get_logger(__name__)



class BandpassModeInputs(basetask.ModeInputs):
    _modes = {'channel' : channelbandpass.ChannelBandpass,
              'poly'    : polynomialbandpass.PolynomialBandpass}

    def __init__(self, context, mode='channel', **parameters):        
        super(BandpassModeInputs, self).__init__(context, mode, **parameters)


class BandpassMode(basetask.ModeTask):
    Inputs = BandpassModeInputs
