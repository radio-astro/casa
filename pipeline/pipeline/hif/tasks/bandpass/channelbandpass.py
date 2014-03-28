from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure as infrastructure
from . import common
from . import bandpassworker

LOG = infrastructure.get_logger(__name__)



class ChannelBandpassInputs(common.CommonBandpassInputs):
    minsnr = basetask.property_with_default('minsnr', 3.0)

    @property
    def bandtype(self):
        return 'B'

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None,
                 #
                 vis=None, caltable=None, 
                 # data selection arguments
                 field=None, spw=None, antenna=None, intent=None,
                 # solution parameters
                 solint=None, combine=None, refant=None, minblperant=None,
                 minsnr=None, solnorm=None, fillgaps=None, append=None, scan=None,
                 # preapply calibrations
                 opacity=None, parang=None,
                 # calibration target
                 to_intent=None, to_field=None):
        self._init_properties(vars())


class ChannelBandpass(bandpassworker.BandpassWorker):
    Inputs = ChannelBandpassInputs
