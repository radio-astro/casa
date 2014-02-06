from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.casatools as casatools
import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.callibrary as callibrary

import itertools

from pipeline.hif.tasks import gaincal
from pipeline.hif.tasks import bandpass
from pipeline.hif.tasks import applycal
from pipeline.hifv.heuristics import getCalFlaggedSoln, getBCalStatistics

LOG = infrastructure.get_logger(__name__)

# CHECKING FLAGGING OF ALL CALIBRATORS
# use rflag mode of flagdata

class CheckflagInputs(basetask.StandardInputs):
    def __init__(self, context, vis=None, checkflagmode=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())
    
    @property
    def checkflagmode(self):
        return self._checkflagmode

    @checkflagmode.setter
    def checkflagmode(self, value):
        if value is None:
            value = None
        self._checkflagmode = value
    
    

class CheckflagResults(basetask.Results):
    def __init__(self, jobs=[]):
        super(CheckflagResults, self).__init__()

        self.jobs=jobs
        
    def __repr__(self):
        s = 'Checkflag (rflag mode) results:\n'
        for job in self.jobs:
            s += '%s performed. Statistics to follow?' % str(job)
        return s 

class Checkflag(basetask.StandardTaskTemplate):
    Inputs = CheckflagInputs
    
    def prepare(self):
        
        LOG.info("Checking RFI flagging of BP and Delay Calibrators")
        
        #Default values
        m = self.inputs.context.observing_run.measurement_sets[0]
        checkflagfields = self.inputs.context.evla['msinfo'][m.name].checkflagfields
        corrstring = self.inputs.context.evla['msinfo'][m.name].corrstring
        testgainscans = self.inputs.context.evla['msinfo'][m.name].testgainscans
        
        method_args = {'field'       : checkflagfields,
                       'correlation' : 'ABS_' + corrstring,
                       'scan'        : testgainscans}
        
        if (self.inputs.checkflagmode == 'semi'):
            calibrator_field_select_string = self.inputs.context.evla['msinfo'][m.name].calibrator_field_select_string
            calibrator_scan_select_string = self.inputs.context.evla['msinfo'][m.name].calibrator_scan_select_string
        
            method_args = {'field'       : calibrator_field_select_string,
                       'correlation' : 'ABS_' + corrstring,
                       'scan'        : calibrator_scan_select_string}
        
        checkflag_result = self._do_checkflag(**method_args)
        
        return checkflag_result
    
    def _do_checkflag(self, field=None, correlation=None, scan=None):
        
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
            
        self._executor.execute(job)
                
        return CheckflagResults([job])
    
    
    
    def analyse(self, results):
	return results
