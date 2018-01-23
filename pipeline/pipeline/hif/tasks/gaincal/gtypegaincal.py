from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.vdp as vdp
from . import common
from . import gaincalworker

LOG = infrastructure.get_logger(__name__)


class GTypeGaincalInputs(common.VdpCommonGaincalInputs):

    scan = vdp.VisDependentProperty(default='')

    gaintype = vdp.VisDependentProperty(default='G')
    preavg = vdp.VisDependentProperty(default=-1)

    combine = vdp.VisDependentProperty(default='')
    minsnr = vdp.VisDependentProperty(default=3)
    solint = vdp.VisDependentProperty(default='inf')
    solnorm = vdp.VisDependentProperty(default=False)
    parang = vdp.VisDependentProperty(default=False)

    append = vdp.VisDependentProperty(default=False)

    def __init__(self, context, output_dir=None, vis=None, caltable=None, 
        field=None, spw=None, antenna=None, uvrange=None, intent=None,
        smodel=None, gaintype=None, calmode=None, solint=None,
	combine=None, refant=None, minblperant=None, minsnr=None,
	solnorm=None, append=None, scan=None, preavg=None,
        opacity=None, parang=None):

        # Standard parameters
        self.context = context
        self.vis = vis
        self.output_dir = output_dir

        # Gaincal inputs parameters
        self.field = field
        self.spw = spw
        self.antenna = antenna
        self.uvrange = uvrange
        self.intent = intent
        self.calmode = calmode
        self.refant = refant
        self.minblperant = minblperant
        self.opacity = opacity

        # Task parameters
        self.scan = scan
        self.gaintype = gaintype
        self.smodel = smodel
        self.solint = solint
        self.combine = combine
        self.minsnr = minsnr
        self.solnorm = solnorm
        self.preavg = preavg
        self.parang = parang
        self.append = append

        # Caltable parameters (caltable from inputs)
        self.caltable = caltable


class GTypeGaincal(gaincalworker.GaincalWorker):
    Inputs = GTypeGaincalInputs
