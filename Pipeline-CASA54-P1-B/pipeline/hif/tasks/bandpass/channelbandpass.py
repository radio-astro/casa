from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.vdp as vdp
from . import common
from . import bandpassworker

__all__ = [
    'ChannelBandpass',
    'ChannelBandpassInputs'
]

LOG = infrastructure.get_logger(__name__)


class ChannelBandpassInputs(common.VdpCommonBandpassInputs):
    minsnr = vdp.VisDependentProperty(default=3.0)

    @property
    def bandtype(self):
        return 'B'

    def __init__(self, context, output_dir=None, vis=None, caltable=None, field=None, spw=None, antenna=None,
                 intent=None, solint=None, combine=None, refant=None, minblperant=None, minsnr=None, solnorm=None,
                 fillgaps=None, append=None, scan=None, opacity=None, parang=None):
        # pipeline inputs
        self.context = context
        # vis must be set first, as other properties may depend on it
        self.vis = vis
        self.output_dir = output_dir
        self.caltable = caltable

        # data selection arguments
        self.field = field
        self.spw = spw
        self.antenna = antenna
        self.intent = intent

        # solution parameters
        self.solint = solint
        self.combine = combine
        self.refant = refant
        self.minblperant = minblperant
        self.minsnr = minsnr
        self.solnorm = solnorm
        self.fillgaps = fillgaps
        self.append = append
        self.scan = scan

        # preapply calibrations
        self.opacity = opacity
        self.parang = parang


class ChannelBandpass(bandpassworker.BandpassWorker):
    Inputs = ChannelBandpassInputs
