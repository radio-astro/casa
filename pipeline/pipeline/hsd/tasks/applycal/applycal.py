from __future__ import absolute_import

import os

import pipeline.infrastructure as infrastructure
from pipeline.domain.datatable import DataTableImpl as DataTable
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
import pipeline.infrastructure.sessionutils as sessionutils
from pipeline.h.tasks.applycal.applycal import Applycal, ApplycalInputs
from pipeline.infrastructure import task_registry

LOG = infrastructure.get_logger(__name__)


class SDApplycalInputs(ApplycalInputs):
    """
    ApplycalInputs defines the inputs for the Applycal pipeline task.
    """
    flagdetailedsum = vdp.VisDependentProperty(default=True)
    intent = vdp.VisDependentProperty(default='TARGET')

    def __init__(self, context, output_dir=None, vis=None, field=None, spw=None, antenna=None, intent=None, parang=None,
                 applymode=None, flagbackup=None, flagsum=None, flagdetailedsum=None):
        super(SDApplycalInputs, self).__init__(context, output_dir=output_dir, vis=vis, field=field, spw=spw,
                                               antenna=antenna, intent=intent, parang=parang, applymode=applymode,
                                               flagbackup=flagbackup, flagsum=flagsum, flagdetailedsum=flagdetailedsum)


@task_registry.set_equivalent_casa_task('hsd_applycal')
@task_registry.set_casa_commands_comment('Calibrations are applied to the data. Final flagging summaries are computed')
class SDApplycal(Applycal):
    """
    Applycal executes CASA applycal tasks for the current context state,
    applying calibrations registered with the pipeline context to the target
    measurement set.
    
    Applying the results from this task to the context marks the referred
    tables as applied. As a result, they will not be included in future
    on-the-fly calibration arguments.
    """
    Inputs = SDApplycalInputs

    def modify_task_args(self, task_args):
        # override template fn in Applycal with our SD-specific antenna
        # selection arguments
        task_args['antenna'] = '*&&&'
        return task_args

    def _get_flagsum_arg(self, args):
        # CAS-8813 flag fraction should be based on target instead of total
        task_args = super(SDApplycal, self)._get_flagsum_arg(args)
        task_args['intent'] = 'OBSERVE_TARGET#ON_SOURCE'
        return task_args
        
    def _tweak_flagkwargs(self, template):
        # CAS-8813 flag fraction should be based on target instead of total
        # use of ' rather than " is required to prevent escaping of flagcmds
        return [row + " intent='OBSERVE_TARGET#ON_SOURCE'" for row in template]

    def prepare(self):
        # execute Applycal
        results = super(SDApplycal, self).prepare()

        # Update Tsys in datatable
        context = self.inputs.context
        # this task uses _handle_multiple_vis framework 
        msobj = self.inputs.ms
        datatable_name = os.path.join(context.observing_run.ms_datatable_name, msobj.basename)
        datatable = DataTable()
        datatable.importdata(name=datatable_name, readonly=False)
        datatable._update_flag(msobj.name)
        for calapp in results.applied:
            filename = os.path.join(context.output_dir, calapp.vis)
            fieldids = [fieldobj.id for fieldobj in msobj.get_fields(name=calapp.field)]
            for _calfrom in calapp.calfrom:
                if _calfrom.caltype == 'tsys':
                    LOG.info('Updating Tsys for {0}'.format(os.path.join(calapp.vis)))
                    tsystable = _calfrom.gaintable
                    spwmap = _calfrom.spwmap
                    gainfield = _calfrom.gainfield
                    datatable._update_tsys(context, filename, tsystable, spwmap, fieldids, gainfield)

        # here, full export is necessary
        datatable.exportdata(minimal=False)

        return results
    

### Tier-0 parallelization
class HpcSDApplycalInputs(SDApplycalInputs):
    # use common implementation for parallel inputs argument
    parallel = sessionutils.parallel_inputs_impl()

    def __init__(self, context, output_dir=None, vis=None, field=None, spw=None, antenna=None, intent=None, parang=None,
                 applymode=None, flagbackup=None, flagsum=None, flagdetailedsum=None, parallel=None):
        super(HpcSDApplycalInputs, self).__init__(context, output_dir=output_dir, vis=vis, field=field, spw=spw,
                                               antenna=antenna, intent=intent, parang=parang, applymode=applymode,
                                               flagbackup=flagbackup, flagsum=flagsum, flagdetailedsum=flagdetailedsum)
        self.parallel = parallel
        

class HpcSDApplycal(sessionutils.ParallelTemplate):
    Inputs = HpcSDApplycalInputs
    Task = SDApplycal
    
    @basetask.result_finaliser
    def get_result_for_exception(self, vis, exception):
        LOG.error('Error operating target flag for {!s}'.format(os.path.basename(vis)))
        LOG.error('{0}({1})'.format(exception.__class__.__name__, exception.message))
        import traceback
        tb = traceback.format_exc()
        if tb.startswith('None'):
            tb = '{0}({1})'.format(exception.__class__.__name__, exception.message)
        return basetask.FailedTaskResults(self, exception, tb)

