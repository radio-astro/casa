from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
from . import channelbandpass
from . import polynomialbandpass

LOG = infrastructure.get_logger(__name__)


# class BandpassModeInputs(basetask.ModeInputs):
#     _modes = {'channel' : channelbandpass.ChannelBandpass,
#               'poly'    : polynomialbandpass.PolynomialBandpass}
#
#     def __init__(self, context, mode='channel', **parameters):
#         super(BandpassModeInputs, self).__init__(context, mode, **parameters)
#
#
# class BandpassMode(basetask.ModeTask):
#     Inputs = BandpassModeInputs


class BandpassModeInputs(vdp.ModeInputs):
    _modes = {
        'channel': channelbandpass.ChannelBandpass,
        # 'poly': polynomialbandpass.PolynomialBandpass
    }

    def __init__(self, context, mode=None, **parameters):
        # Why is the default set here rather than as a default
        # argument? Because the default argument is used when the
        # argument is not supplied. A super class could specify None,
        # in which case we would lose the default.
        if mode is None:
            mode = 'channel'

        # remove attempts to set what we know to be read-only parameters
        for read_only_attr in ['bandtype']:
            if read_only_attr in parameters:
                del parameters[read_only_attr]

        super(BandpassModeInputs, self).__init__(context, mode, **parameters)


class BandpassMode(basetask.ModeTask):
    Inputs = BandpassModeInputs
