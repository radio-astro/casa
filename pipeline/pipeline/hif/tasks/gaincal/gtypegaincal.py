from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.vdp as vdp
from . import common
from . import gaincalworker

LOG = infrastructure.get_logger(__name__)


class GTypeGaincalInputs(common.VdpCommonGaincalInputs):
    # Why is gaintype not read-only? Well, some tasks use GtypeGaincal class
    # but change the mode to 'T', 'KCROSS', etc. I suspect this is misusing
    # GTypeGaincal; there should either be dedicated classes introduced for
    # these non-G tasks or we should rename this class to something more
    # generic.
    gaintype = vdp.VisDependentProperty(default='G')

    append = vdp.VisDependentProperty(default=False)
    combine = vdp.VisDependentProperty(default='')
    minsnr = vdp.VisDependentProperty(default=3)
    parang = vdp.VisDependentProperty(default=False)
    preavg = vdp.VisDependentProperty(default=-1)
    scan = vdp.VisDependentProperty(default='')
    solint = vdp.VisDependentProperty(default='inf')
    solnorm = vdp.VisDependentProperty(default=False)

    def __init__(self, context, output_dir=None, vis=None, caltable=None, field=None, spw=None, antenna=None,
                 uvrange=None, intent=None, smodel=None, calmode=None, solint=None, combine=None, refant=None,
                 minblperant=None, minsnr=None, solnorm=None, append=None, scan=None, preavg=None, opacity=None,
                 parang=None, gaintype=None):
        super(GTypeGaincalInputs, self).__init__(context, output_dir=output_dir, vis=vis, caltable=caltable,
                                                 intent=intent, field=field, spw=spw, refant=refant, antenna=antenna,
                                                 minblperant=minblperant, opacity=opacity, selectdata=None,
                                                 uvrange=uvrange, calmode=calmode)
        self.append = append
        self.combine = combine
        self.gaintype = gaintype
        self.minsnr = minsnr
        self.parang = parang
        self.preavg = preavg
        self.scan = scan
        self.smodel = smodel
        self.solint = solint
        self.solnorm = solnorm


class GTypeGaincal(gaincalworker.GaincalWorker):
    Inputs = GTypeGaincalInputs
