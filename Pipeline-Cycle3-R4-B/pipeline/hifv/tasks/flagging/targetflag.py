from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure as infrastructure


LOG = infrastructure.get_logger(__name__)

# CHECKING FLAGGING OF ALL CALIBRATORS
# use rflag mode of flagdata


class TargetflagInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())
    

class TargetflagResults(basetask.Results):
    def __init__(self, jobs=[]):
        super(TargetflagResults, self).__init__()

        self.jobs=jobs
        
    def __repr__(self):
        s = 'Targetflag (rflag mode) results:\n'
        for job in self.jobs:
            s += '%s performed. Statistics to follow?' % str(job)
        return s 


class Targetflag(basetask.StandardTaskTemplate):
    Inputs = TargetflagInputs
    
    def prepare(self):
        
        # Default values
        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        # corrstring = self.inputs.context.evla['msinfo'][m.name].corrstring
        corrstring = m.get_vla_corrstring()
        
        method_args = {'field'       : '',
                       'correlation' : 'ABS_' + corrstring,
                       'scan'        : ''}

        rflag_result = self._do_rflag(**method_args)
        
        return TargetflagResults([rflag_result])
    
    def analyse(self, results):
        return results
    
    def _do_rflag(self, field=None, correlation=None, scan=None):
        
        task_args = {'vis'          : self.inputs.vis,
                     'mode'         : 'rflag',
                     'field'        : field,
                     'correlation'  : correlation,
                     'scan'         : scan,
                     'ntime'        : 'scan',
                     'combinescans' : False,
                     'datacolumn'   : 'corrected',
                     'winsize'      : 3,
                     'timedevscale' : 4.0,
                     'freqdevscale' : 4.0,
                     'action'       : 'apply',
                     'display'      : '',
                     'extendflags'  : False,
                     'flagbackup'   : False,
                     'savepars'     : True}
                     
        job = casa_tasks.flagdata(**task_args)
            
        return self._executor.execute(job)
