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
import numpy as np


LOG = infrastructure.get_logger(__name__)

class SolintInputs(basetask.StandardInputs):
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())

class SolintResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[], longsolint=None, gain_solint2=None):
        super(SolintResults, self).__init__()

        self.vis = None
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()
        self.longsolint = longsolint
        self.gain_solint2 = gain_solint2
        
    def merge_with_context(self, context):    
        m = context.observing_run.measurement_sets[0]
        context.evla['msinfo'][m.name].gain_solint2 = self.gain_solint2
        context.evla['msinfo'][m.name].longsolint = self.longsolint

class Solint(basetask.StandardTaskTemplate):
    Inputs = SolintInputs
    
    def prepare(self):
    
        calMs = 'calibrators.ms'
        split_result = self._do_split(calMs)
        
        (longsolint, gain_solint2) = self._do_determine_solint(calMs)
        
        return SolintResults(longsolint=longsolint, gain_solint2=gain_solint2)
    
    def analyse(self, results):
	return results
    
    def _do_split(self, calMs):
        
        m = self.inputs.context.observing_run.measurement_sets[0]
        channels = self.inputs.context.evla['msinfo'][m.name].channels
        calibrator_scan_select_string = self.inputs.context.evla['msinfo'][m.name].calibrator_scan_select_string
    
        LOG.info("Splitting out calibrators into calibrators.ms")
    
        task_args = {'vis'          : m.name,
                     'outputvis'    : calMs,
                     'datacolumn'   : 'corrected',
                     'field'        : '',
                     'spw'          : '',
                     'width'        : int(max(channels)),
                     'antenna'      : '',
                     'timebin'      : '0s',
                     'timerange'    : '',
                     'scan'         : calibrator_scan_select_string,
                     'intent'       : '',
                     'array'        : '',
                     'uvrange'      : '',
                     'correlation'  : '',
                     'observation'  : '',
                     'keepflags'    : False}
        
        job = casa_tasks.split(**task_args)
            
        return self._executor.execute(job)
    
    def _do_determine_solint(self, calMs):
        
        durations = []
        old_spws = []
        old_field = ''
        
        with casatools.MSReader(calMs) as ms:
            scan_summary = ms.getscansummary()    
            
            m = self.inputs.context.observing_run.measurement_sets[0]
            phase_scan_list = self.inputs.context.evla['msinfo'][m.name].phase_scan_list
            
            print phase_scan_list
            
            for kk in range(len(phase_scan_list)):
                ii = phase_scan_list[kk]
                
                try:
                    end_time = scan_summary[str(ii)]['0']['EndTime']
                    begin_time = scan_summary[str(ii)]['0']['BeginTime']
                    new_spws = scan_summary[str(ii)]['0']['SpwIds']
                    new_field = scan_summary[str(ii)]['0']['FieldId']
                    
                    #print end_time, begin_time, new_spws, new_field
                    
                    if ((kk > 0) and (phase_scan_list[kk-1] == ii-1) and (set(new_spws) == set(old_spws)) and (new_field == old_field)):
                        # if contiguous scans, just increase the time on the previous one
                        print "End time, old begin time", end_time, old_begin_time
                        durations[-1] = 86400*(end_time - old_begin_time)
                        #print "first durations: ", durations
                    else:
                        print "End time, begin time", end_time, begin_time
                        durations.append(86400*(end_time - begin_time))
                        old_begin_time = begin_time
                        print "append durations, old, begin:", durations, old_begin_time, begin_time
                    LOG.info("Scan "+str(ii)+" has "+str(durations[-1])+"s on source")
                    old_spws = new_spws
                    old_field = new_field
                
                except KeyError:
                    LOG.warn("WARNING: scan "+str(ii)+" is completely flagged and missing from calibrators.ms")
              
              
        longsolint = (np.max(durations))*1.01
        gain_solint2=str(longsolint)+'s'
                   
        return (longsolint, gain_solint2)

