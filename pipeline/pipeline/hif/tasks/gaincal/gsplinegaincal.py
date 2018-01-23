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

    parang = vdp.VisDependentProperty(default=False)
    preavg = vdp.VisDependentProperty(default=-1)
    append = vdp.VisDependentProperty(default=False)

    def __init__(self, context, output_dir=None, vis=None, caltable=None, 
        field=None, spw=None, antenna=None, uvrange=None, intent=None,
        smodel=None, calmode=None,  refant=None, splinetime=None,
        npointaver=None, append=None, phasewrap=None,
        opacity=None, parang=None, preavg=None):

        # Standard parameters
        self.context = context
        self.vis = vis
        self.output_dir = output_dir

        # Gaincal inputs parameters
        self.field = field
        self.intent = intent
        self.spw = spw
        self.antenna = antenna
        self.uvrange = uvrange
        self.refant = refant
        self.opacity = opacity
        self.calmode = calmode

        # Task parameters
        self.gaintype = gaintype
        self.smodel = smodel
        self.splinetime = splinetime
        self.npointaver = npointaver
        self.phasewrap = phasewrap
        self.parang = parang
        self.preavg = preavg

        self.append = append
        self.caltable = caltable

class GSplineGaincal(gaincalworker.GaincalWorker):
    Inputs = GSplineGaincalInputs
