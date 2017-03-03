from __future__ import absolute_import

import pipeline.h.tasks.applycal.applycal as applycal
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp

LOG = infrastructure.get_logger(__name__)

__all__ = ['IFApplycal',
           'IFApplycalInputs']


class IFApplycalInputs(applycal.ApplycalInputs):
    flagdetailedsum = vdp.VisDependentProperty(default=True)

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, field=None, spw=None, antenna=None, intent=None,
                 opacity=None, parang=None, applymode=None, flagbackup=None, flagsum=None, flagdetailedsum=None):
        super(IFApplycalInputs, self).__init__(context, output_dir=output_dir, vis=vis, field=field, spw=spw,
                                               antenna=antenna, intent=intent, opacity=opacity, parang=parang,
                                               applymode=applymode, flagbackup=flagbackup, flagsum=flagsum,
                                               flagdetailedsum=flagdetailedsum)


class IFApplycal(applycal.Applycal):
    Inputs = IFApplycalInputs
