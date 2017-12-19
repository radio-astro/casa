from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.vdp as vdp
from . import common
from . import gaincalworker

LOG = infrastructure.get_logger(__name__)

class KTypeGaincalInputs(common.VdpCommonGaincalInputs):
    combine = vdp.VisDependentProperty(default='')
    minsnr  = vdp.VisDependentProperty(default=3)
    solint  = vdp.VisDependentProperty(default='int')
    solnorm = vdp.VisDependentProperty(default=False)
    gaintype = vdp.VisDependentProperty(default='K')

    def __init__(self, context, output_dir=None, vis=None, caltable=None,
                 field=None, spw=None, antenna=None, uvrange=None, intent=None,
                 smodel=None, gaintype=None, calmode=None, solint=None,
                 combine=None, refant=None, minblperant=None, minsnr=None,
                 solnorm=None, append=None, scan=None,
                 opacity=None, parang=None):

        self.context = context
        self.vis = vis
        self.output_dir = output_dir

        self.field = field
        self.spw = spw
        self.antenna = antenna
        self.uvrange = uvrange
        self.intent = intent
        self.scan = scan

        self.gaintype = gaintype

        self.smodel = smodel
        self.calmode = calmode
        self.solint = solint
        self.combine = combine
        self.refant = refant
        self.minblperant = minblperant
        self.minsnr = minsnr
        self.solnorm = solnorm
        self.append = append
        self.scan = scan
        self.opacity = opacity
        self.parang = parang

        self.caltable = caltable


class KTypeGaincal(gaincalworker.GaincalWorker):
    Inputs = KTypeGaincalInputs
