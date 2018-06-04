from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.vdp as vdp
from . import common
from . import gaincalworker

LOG = infrastructure.get_logger(__name__)

def hif_gaincal(vis=None, caltable=None, field=None, intent=None, spw=None,
                antenna=None, hm_gaintype=None, calmode=None, solint=None,
                combine=None, refant=None, solnorm=None, minblperant=None,
                minsnr=None, smodel=None, splinetime=None, npointaver=None,
                phasewrap=None,pipelinemode=None, dryrun=None,
                acceptresults=None):
    pass


class KTypeGaincalInputs(common.VdpCommonGaincalInputs):
    append = vdp.VisDependentProperty(default=False)
    combine = vdp.VisDependentProperty(default='')
    gaintype = vdp.VisDependentProperty(default='K', readonly=True)
    minsnr = vdp.VisDependentProperty(default=3)
    parang = vdp.VisDependentProperty(default=False)
    scan = vdp.VisDependentProperty(default='')
    solint = vdp.VisDependentProperty(default='int')
    solnorm = vdp.VisDependentProperty(default=False)

    def __init__(self, context, output_dir=None, vis=None, caltable=None, field=None, intent=None, spw=None,
                 antenna=None, uvrange=None, smodel=None, calmode=None, solint=None, combine=None, refant=None,
                 minblperant=None, minsnr=None, solnorm=None, append=None, scan=None, opacity=None, parang=None):
        super(KTypeGaincalInputs, self).__init__(context, output_dir=output_dir, vis=vis, caltable=caltable,
                                                 intent=intent, field=field, spw=spw, refant=refant, antenna=antenna,
                                                 minblperant=minblperant, opacity=opacity, selectdata=None,
                                                 uvrange=uvrange, calmode=calmode)
        self.scan = scan
        self.smodel = smodel
        self.solint = solint
        self.combine = combine
        self.minsnr = minsnr
        self.solnorm = solnorm
        self.parang = parang
        self.preavg = -1
        self.append = append


class KTypeGaincal(gaincalworker.GaincalWorker):
    Inputs = KTypeGaincalInputs
