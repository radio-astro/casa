from __future__ import absolute_import

import pipeline.h.tasks.applycal.applycal as applycal
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.sessionutils as sessionutils
import pipeline.infrastructure.vdp as vdp

__all__ = ['IFApplycal',
           'IFApplycalInputs',
           'SessionIFApplycal',
           'SessionIFApplycalInputs']

LOG = infrastructure.get_logger(__name__)


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

    def __init__(self, inputs):
        super(IFApplycal, self).__init__(inputs)


class SessionIFApplycalInputs(IFApplycalInputs):
    # use common implementation for parallel inputs argument
    parallel = sessionutils.parallel_inputs_impl()

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, field=None, spw=None, antenna=None, intent=None,
                 opacity=None, parang=None, applymode=None, flagbackup=None, flagsum=None, flagdetailedsum=None,
                 parallel=None):
        super(SessionIFApplycalInputs, self).__init__(context, output_dir=output_dir, vis=vis, field=field, spw=spw,
                                                      antenna=antenna, intent=intent, opacity=opacity, parang=parang,
                                                      applymode=applymode, flagbackup=flagbackup, flagsum=flagsum,
                                                      flagdetailedsum=flagdetailedsum)
        self.parallel = parallel


class SessionIFApplycal(applycal.SessionApplycal):
    Inputs = SessionIFApplycalInputs
    Task = IFApplycal

    def __init__(self, inputs):
        super(SessionIFApplycal, self).__init__(inputs)
