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

#Utility from the EVLA scripted pipeline
def uniq(inlist):
   uniques = []
   for item in inlist:
      if item not in uniques:
         uniques.append(item)
   return uniques


class UncalspwInputs(basetask.StandardInputs):
    def __init__(self, context, vis=None, 
                 # table selection arguments
                 delaycaltable=None, bpcaltable=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())
    
    @property
    def delaycaltable(self):
        return self._delaycaltable

    @delaycaltable.setter
    def delaycaltable(self, value):
        if value is None:
            value = None
        self._delaycaltable = value
    
    @property
    def bpcaltable(self):
        return self._bpcaltable

    @bpcaltable.setter
    def bpcaltable(self, value):
        if value is None:
            value = None
        self._bpcaltable = value


class UncalspwResults(basetask.Results):
    def __init__(self, jobs=[], flagspw1='', flagspw1b='', flagspw2=''):
        super(UncalspwResults, self).__init__()

        self.jobs = jobs
        self.flagspw1  = flagspw1
        self.flagspw1b = flagspw1b
        self.flagspw2  = flagspw2
    
    def merge_with_context(self, context):
        m = context.observing_run.measurement_sets[0]
        context.evla['msinfo'][m.name].flagspw1  = self.flagspw1
        context.evla['msinfo'][m.name].flagspw1b = self.flagspw1b
        context.evla['msinfo'][m.name].flagspw2  = self.flagspw2
        
    def __repr__(self):
        s = 'Uncalspw results:\n'
        for job in self.jobs:
            s += '%s performed. Statistics to follow?' % str(job)
        return s 

class Uncalspw(basetask.StandardTaskTemplate):
    Inputs = UncalspwInputs
    
    def prepare(self):
        
        method_args = {'delaycaltable' : self.inputs.delaycaltable,
                       'bpcaltable' : self.inputs.bpcaltable}
        
        LOG.info("Uncalspw using: " + self.inputs.delaycaltable + " " + self.inputs.bpcaltable)
        
        uncalspw_result = self._do_uncalspw(**method_args)
        
        return uncalspw_result
        
    def _do_uncalspw(self, delaycaltable=None, bpcaltable=None):
        
        print "uncalspw"
        
        m = self.inputs.context.observing_run.measurement_sets[0]
        context = self.inputs.context
        myscans = context.evla['msinfo'][m.name].scandict
        
        myspw = []
        for idd in myscans['DataDescription'].keys():
            ispw = myscans['DataDescription'][idd]['spw']
            if myspw.count(ispw)<1:
                myspw.append(ispw)
        
        calflagresult = getCalFlaggedSoln(delaycaltable)
        goodspw = []
        for ispw in calflagresult['spw'].keys():
            tot = 0.0
            flagd = 0.0
            for ipol in calflagresult['spw'][ispw].keys():
                tot += calflagresult['spw'][ispw][ipol]['total']
                flagd += calflagresult['spw'][ispw][ipol]['flagged']
            if tot>0:
                fract = flagd/tot
                if fract<1.0:
                    goodspw.append(ispw)
        
        flagspwlist = []
        flagspw = ''
        for ispw in myspw:
            if goodspw.count(ispw)<1:
                flagspwlist.append(ispw)
                if flagspw=='':
                    flagspw = str(ispw)
                else:
                    flagspw += ','+str(ispw)
        
        calflagresult = getCalFlaggedSoln(bpcaltable)
        goodspw = []
        for ispw in calflagresult['spw'].keys():
            tot = 0.0
            flagd = 0.0
            for ipol in calflagresult['spw'][ispw].keys():
                tot += calflagresult['spw'][ispw][ipol]['total']
                flagd += calflagresult['spw'][ispw][ipol]['flagged']
            if tot>0:
                fract = flagd/tot
                if fract<1.0:
                    goodspw.append(ispw)
        
        for ispw in myspw:
            if goodspw.count(ispw)<1:
                flagspwlist.append(ispw)
                if flagspw=='':
                    flagspw = str(ispw)
                else:
                    flagspw += ','+str(ispw)
        
        #From scripted uncalspws1.py
        if (delaycaltable == 'testdelay.k' and bpcaltable == 'testBPcal.b'):
            flagspw1 = ','.join(["%s" % ii for ii in uniq(flagspwlist)])
            
            print "uncalspws1"
        
            if (flagspw1 == ''):
                LOG.info("All spws have calibration")
                return UncalspwResults(jobs=[],flagspw1=flagspw1)
            else:
                LOG.info("No calibration found for spw(s) "+flagspw1+", flagging these spws in the ms")
                spw = flagspw1
                
                task_args = {'vis'        : self.inputs.vis,
                            'action'      : 'apply',                     
                            'spw'         : spw,
                            'savepars'    : True,
                            'flagbackup'  : True}
                
                job = casa_tasks.flagdata(**task_args)
                
                self._executor.execute(job)
                    
                return UncalspwResults(jobs=[job], flagspw1=flagspw1)
        
        #From scripted uncalspws1b.py
        if (delaycaltable == 'delay.k' and bpcaltable == 'BPcal.b'):
            flagspw1b = ','.join(["%s" % ii for ii in uniq(flagspwlist)])
            flagspw1 = context.evla['msinfo'][m.name].flagspw1
            
            if (flagspw1b == ''):
                LOG.info("All spws have calibration")
                return UncalspwResults(jobs=[], flagspw1=flagspw1, flagspw1b=flagspw1b)
            elif (flagspw1b==flagspw1):
                LOG.info("No calibration found for spw(s) "+flagspw1b+", already flagged")
                return UncalspwResults(jobs=[], flagspw1=flagspw1, flagspw1b=flagspw1b)
            else:
                LOG.info("No calibration found for spw(s) "+flagspw1b+", flagging these spws in the ms")
                spw=flagspw1b
                
                task_args = {'vis'        : self.inputs.vis,
                            'action'      : 'apply',                     
                            'spw'         : spw,
                            'savepars'    : True,
                            'flagbackup'  : True}
                
                job = casa_tasks.flagdata(**task_args)
                
                self._executor.execute(job)
                    
                return UncalspwResults(jobs=[job], flagspw1=flagspw1, flagspw1b=flagspw1b)
        
        #From scripted uncalspws2.py
        if (delaycaltable == 'finaldelay.k' and bpcaltable == 'finalBPcal.b'):
            flagspw2 = ','.join(["%s" % ii for ii in uniq(flagspwlist)])
            flagspw1 = context.evla['msinfo'][m.name].flagspw1
            flagspw1b = context.evla['msinfo'][m.name].flagspw1b
            
            if (flagspw2 == ''):
                LOG.info("All spws have calibration")
                return UncalspwResults(jobs=[], flagspw1=flagspw1, flagspw1b=flagspw1b, flagspw2=flagspw2)
            elif (flagspw2==flagspw1b):
                LOG.info("No calibration found for spw(s) "+flagspw2+", already flagged")
                return UncalspwResults(jobs=[], flagspw1=flagspw1, flagspw1b=flagspw1b, flagspw2=flagspw2)
            else:
                LOG.info("No calibration found for spw(s) "+flagspw2+", flagging these spws in the ms")
                spw=flagspw2
                
                task_args = {'vis'        : self.inputs.vis,
                            'action'      : 'apply',                     
                            'spw'         : spw,
                            'savepars'    : True,
                            'flagbackup'  : True}
                
                job = casa_tasks.flagdata(**task_args)
                
                self._executor.execute(job)
                    
                return UncalspwResults(jobs=[job], flagspw1=flagspw1, flagspw1b=flagspw1b, flagspw2=flagspw2)

        
    def analyse(self, results):
	return results
