from __future__ import absolute_import
import os

from pipeline.domain import DataTable
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp

from pipeline.h.tasks.applycal.applycal import Applycal, ApplycalInputs

LOG = infrastructure.get_logger(__name__)


class SDMSApplycalInputs(ApplycalInputs):
    """
    ApplycalInputs defines the inputs for the Applycal pipeline task.
    """
    flagdetailedsum = basetask.property_with_default('flagdetailedsum', True)
    intent = vdp.VisDependentProperty(default='TARGET')

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, field=None, spw=None, antenna=None, intent=None,
                 opacity=None, parang=None, applymode=None, flagbackup=None, flagsum=None, flagdetailedsum=None):
        super(SDMSApplycalInputs, self).__init__(context, output_dir=output_dir, vis=vis, field=field, spw=spw,
                                                 antenna=antenna, intent=intent, opacity=opacity, parang=parang,
                                                 applymode=applymode, flagbackup=flagbackup, flagsum=flagsum,
                                                 flagdetailedsum=flagdetailedsum)


class SDMSApplycal(Applycal):
    """
    Applycal executes CASA applycal tasks for the current context state,
    applying calibrations registered with the pipeline context to the target
    measurement set.
    
    Applying the results from this task to the context marks the referred
    tables as applied. As a result, they will not be included in future
    on-the-fly calibration arguments.
    """
    Inputs = SDMSApplycalInputs

    def modify_task_args(self, task_args):
        # override template fn in Applycal with our SD-specific antenna
        # selection arguments
        task_args['antenna'] = '*&&&'
        return task_args

    def _get_flagsum_arg(self, args):
        # CAS-8813 flag fraction should be based on target instead of total
        task_args = super(SDMSApplycal, self)._get_flagsum_arg(args)
        task_args['intent'] = 'OBSERVE_TARGET#ON_SOURCE'
        return task_args
        
    def _tweak_flagkwargs(self, template):
        # CAS-8813 flag fraction should be based on target instead of total
        return [row + ' intent="OBSERVE_TARGET#ON_SOURCE"' for row in template]

    def prepare(self):
        # execute Applycal
        results = super(SDMSApplycal, self).prepare()

        # Update Tsys in datatable
        context = self.inputs.context
        datatable = DataTable(name=context.observing_run.ms_datatable_name, readonly=False)
        # this task uses _handle_multiple_vis framework 
        msobj = self.inputs.ms
        datatable._update_flag(context, msobj.name)
        for calapp in results.applied:
            filename = os.path.join(context.output_dir, calapp.vis)
            fieldids = [fieldobj.id for fieldobj in msobj.get_fields(name=calapp.field)]
            for _calfrom in calapp.calfrom:
                if _calfrom.caltype == 'tsys':
                    tsystable = _calfrom.gaintable
                    spwmap = _calfrom.spwmap
                    gainfield= _calfrom.gainfield
                    datatable._update_tsys(context, filename, tsystable, spwmap, fieldids, gainfield)

        # here, full export is necessary
        datatable.exportdata(minimal=False)

        return results
