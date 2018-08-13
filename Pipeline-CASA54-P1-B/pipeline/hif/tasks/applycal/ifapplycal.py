from __future__ import absolute_import

import pipeline.h.tasks.applycal.applycal as applycal
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.sessionutils as sessionutils
import pipeline.infrastructure.vdp as vdp
from pipeline.infrastructure import task_registry

__all__ = ['IFApplycal',
           'IFApplycalInputs',
           'HpcIFApplycal',
           'HpcIFApplycalInputs']

LOG = infrastructure.get_logger(__name__)


class IFApplycalInputs(applycal.ApplycalInputs):
    flagdetailedsum = vdp.VisDependentProperty(default=True)

    def __init__(self, context, output_dir=None, vis=None, field=None, spw=None, antenna=None, intent=None, parang=None,
                 applymode=None, flagbackup=None, flagsum=None, flagdetailedsum=None):
        super(IFApplycalInputs, self).__init__(context, output_dir=output_dir, vis=vis, field=field, spw=spw,
                                               antenna=antenna, intent=intent, parang=parang, applymode=applymode,
                                               flagbackup=flagbackup, flagsum=flagsum, flagdetailedsum=flagdetailedsum)


@task_registry.set_equivalent_casa_task('hif_applycal')
@task_registry.set_casa_commands_comment('Calibrations are applied to the data. Final flagging summaries are computed')
class IFApplycal(applycal.Applycal):
    Inputs = IFApplycalInputs

    def __init__(self, inputs):
        super(IFApplycal, self).__init__(inputs)


class HpcIFApplycalInputs(IFApplycalInputs):
    # use common implementation for parallel inputs argument
    parallel = sessionutils.parallel_inputs_impl()

    def __init__(self, context, output_dir=None, vis=None, field=None, spw=None, antenna=None, intent=None, parang=None,
                 applymode=None, flagbackup=None, flagsum=None, flagdetailedsum=None, parallel=None):
        super(HpcIFApplycalInputs, self).__init__(context, output_dir=output_dir, vis=vis, field=field, spw=spw,
                                                  antenna=antenna, intent=intent, parang=parang, applymode=applymode,
                                                  flagbackup=flagbackup, flagsum=flagsum,
                                                  flagdetailedsum=flagdetailedsum)
        self.parallel = parallel


@task_registry.set_equivalent_casa_task('hpc_hif_applycal')
class HpcIFApplycal(applycal.HpcApplycal):
    Inputs = HpcIFApplycalInputs
    Task = IFApplycal

    def __init__(self, inputs):
        super(HpcIFApplycal, self).__init__(inputs)
