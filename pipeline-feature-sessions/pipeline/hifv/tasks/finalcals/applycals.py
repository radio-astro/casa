from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp

import pipeline.hif.tasks.applycal.ifapplycal as ifapplycal

LOG = infrastructure.get_logger(__name__)


class ApplycalsInputs(ifapplycal.IFApplycalInputs):
    """
    ApplycalInputs defines the inputs for the Applycal pipeline task.
    """
    field = vdp.VisDependentProperty(default='')
    flagbackup = vdp.VisDependentProperty(default=False)
    intent = vdp.VisDependentProperty(default='')
    parang = vdp.VisDependentProperty(default=True)
    spw = vdp.VisDependentProperty(default='')

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, field=None, spw=None, antenna=None, intent=None,
                 opacity=None, parang=None, applymode=None, flagbackup=None, flagsum=None, flagdetailedsum=None):
        super(ApplycalsInputs, self).__init__(context, output_dir=output_dir, vis=vis, field=field, spw=spw,
                                              antenna=antenna, intent=intent, opacity=opacity, parang=parang,
                                              applymode=applymode, flagbackup=flagbackup, flagsum=flagsum,
                                              flagdetailedsum=flagdetailedsum)


class Applycals(ifapplycal.IFApplycal):
    Inputs = ApplycalsInputs

    def modify_task_args(self, task_args):
        task_args = super(Applycals, self).modify_task_args(task_args)

        # Eliminate target data selection for VLA applycal jobs
        del task_args['field']
        del task_args['intent']
        del task_args['spw']

        # Force all calwts to be False. Could do this with a simple '= False'
        # but do it as an array to keep casa_commands the same
        task_args['calwt'] = [False] * len(task_args['calwt'])

        return task_args

    def process_flagsummary(self, flagsummary):
        ms = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        return limit_fields(flagsummary, ms)


def limit_fields(flagsummary, ms):
    calibrator_fields = ms.get_fields(intent='AMPLITUDE,PHASE,BANDPASS')

    target_fields = ms.get_fields(intent='TARGET')
    plot_stride = len(target_fields) / 30 + 1
    targetfields = target_fields[::plot_stride]

    fields_to_plot = calibrator_fields + targetfields

    return {k: v for k, v in flagsummary.iteritems() if k in fields_to_plot}
