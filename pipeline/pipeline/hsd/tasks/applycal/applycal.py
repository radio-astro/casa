from __future__ import absolute_import
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from .. import common

from pipeline.h.tasks.applycal.applycal import ApplycalInputs, Applycal

LOG = infrastructure.get_logger(__name__)


class SDApplycalInputs(ApplycalInputs,basetask.StandardInputs,
                         basetask.OnTheFlyCalibrationMixin):
    """
    ApplycalInputs defines the inputs for the Applycal pipeline task.
    """
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None,
                 # 
                 vis=None, 
                 # data selection arguments
                 field=None, spw=None, antenna=None, intent=None,
                 # preapply calibrations
                 opacity=None, parang=None, applymode=None, calwt=None,
                 flagbackup=None, flagsum=None, flagdetailedsum=None):
        self._init_properties(vars())

    flagdetailedsum = basetask.property_with_default('flagdetailedsum', True)

    @property
    def intent(self):
        return self._intent

    @intent.setter
    def intent(self, value):
        if value is None:
            value = 'TARGET'
        self._intent = value.replace('*', '')

    @property
    def applymode(self):
        return self._applymode

    @applymode.setter
    def applymode(self, value):
        if value is None:
            value = 'calflagstrict'
        elif value == '':
            value = 'calflagstrict'
        self._applymode = value

class SDApplycal(Applycal, common.SingleDishTask):#basetask.StandardTaskTemplate):
    """
    Applycal executes CASA applycal tasks for the current context state,
    applying calibrations registered with the pipeline context to the target
    measurement set.
    
    Applying the results from this task to the context marks the referred
    tables as applied. As a result, they will not be included in future
    on-the-fly calibration arguments.
    """
    Inputs = SDApplycalInputs
    ### Note this is a temporary workaround ###
    antenna_to_apply = '*&&&'
    
    def _get_flagsum_arg(self, vis):
        # CAS-8813 flag fraction should be based on target instead of total
        task_args = super(SDApplycal, self)._get_flagsum_arg(vis=vis)
        task_args.update({'intent': 'OBSERVE_TARGET#ON_SOURCE'})
        return task_args
        
    def _tweak_flagkwargs(self, template):
        # CAS-8813 flag fraction should be based on target instead of total
        flagkwargs = []
        for t in template:
            flagkwargs.append(t + " intent='OBSERVE_TARGET#ON_SOURCE'")
        return flagkwargs
        
    def prepare(self):
        # execute Applycal
        results = super(SDApplycal, self).prepare()
        # Update Tsys in datatable
        context = self.inputs.context
        #datatable = DataTable(name=context.observing_run.ms_datatable_name, readonly=False)
        datatable = self.datatable_instance
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
    
