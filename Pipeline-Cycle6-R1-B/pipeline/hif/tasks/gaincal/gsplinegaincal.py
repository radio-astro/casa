from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.vdp as vdp
from . import common
from . import gaincalworker

LOG = infrastructure.get_logger(__name__)


class GSplineGaincalInputs(common.VdpCommonGaincalInputs):
    append = vdp.VisDependentProperty(default=False)
    npointaver = vdp.VisDependentProperty(default=3)
    parang = vdp.VisDependentProperty(default=False)
    phasewrap = vdp.VisDependentProperty(default=180)
    preavg = vdp.VisDependentProperty(default=-1)
    splinetime = vdp.VisDependentProperty(default=3600)

    @property
    def gaintype(self):
        return 'GSPLINE'

    def __init__(self, context, output_dir=None, vis=None, caltable=None, field=None, spw=None, antenna=None,
                 uvrange=None, intent=None, smodel=None, calmode=None, refant=None, minblperant=None, splinetime=None,
                 npointaver=None, append=None, phasewrap=None, opacity=None, parang=None, preavg=None):
        super(GSplineGaincalInputs, self).__init__(context, output_dir=output_dir, vis=vis, caltable=caltable,
                                                   intent=intent, field=field, spw=spw, refant=refant, antenna=antenna,
                                                   minblperant=minblperant, opacity=opacity, selectdata=None,
                                                   uvrange=uvrange, calmode=calmode)
        self.smodel = smodel
        self.splinetime = splinetime
        self.npointaver = npointaver
        self.phasewrap = phasewrap
        self.parang = parang
        self.preavg = preavg
        self.append = append


class GSplineGaincal(gaincalworker.GaincalWorker):
    Inputs = GSplineGaincalInputs
