from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.vdp as vdp
from . import common
from . import gaincalworker

LOG = infrastructure.get_logger(__name__)

class GSplineGaincalInputs(common.VdpCommonGaincalInputs):

    npointaver = vdp.VisDependentProperty(default=3)
    phasewrap = vdp.VisDependentProperty(default=180)
    splinetime = vdp.VisDependentProperty(default=3600)
    gaintype = vdp.VisDependentProperty(default='GSPLINE')

    def __init__(self, context, output_dir=None, vis=None, caltable=None, 
        field=None, spw=None, antenna=None, uvrange=None, intent=None,
        smodel=None, calmode=None,  refant=None, splinetime=None,
        npointaver=None, append=None, phasewrap=None,
        opacity=None, parang=None):

        self.context = context
        self.vis = vis
        self.output_dir = output_dir

        self.field = field
        self.spw = spw
        self.antenna = antenna
        self.uvrange = uvrange
        self.intent = intent

        self.gaintype = gaintype

        self.smodel = smodel
        self.calmode = calmode
        self.refant = refant
        self.splinetime = splinetime
        self.npointaver = npointaver
        self.append = append
        self.phasewrap = phasewrap
        self.opacity = opacity
        self.parang = parang

        self.caltable = caltable


class GSplineGaincal(gaincalworker.GaincalWorker):
    Inputs = GSplineGaincalInputs
