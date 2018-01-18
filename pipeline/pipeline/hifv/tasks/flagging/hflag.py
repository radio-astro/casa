from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.vdp as vdp

from pipeline.hifv.tasks.testBPdcals import testBPdcals
from pipeline.hifv.tasks.flagging.flagbaddeformatters import FlagBadDeformatters
from pipeline.hifv.tasks.flagging.checkflag import Checkflag
from pipeline.hifv.tasks.flagging.uncalspw import Uncalspw
from pipeline.hifv.tasks.semiFinalBPdcals import semiFinalBPdcals


LOG = infrastructure.get_logger(__name__)


class HeuristicflagInputs(vdp.StandardInputs):
    def __init__(self, context, vis=None):
        super(HeuristicflagInputs, self).__init__()
        self.context = context
        self.vis = vis


class HeuristicflagResults(basetask.Results):
    def __init__(self, jobs=[]):
        super(HeuristicflagResults, self).__init__()

        self.jobs=jobs
        
    def __repr__(self):
        s = 'Heuristic flagging results:\n'
        for job in self.jobs:
            s += '%s performed. Statistics to follow?' % str(job)
        return s 


class Heuristicflag(basetask.StandardTaskTemplate):
    Inputs = HeuristicflagInputs
    
    def prepare(self):
        
        #Default values
        m = self.inputs.context.observing_run.measurement_sets[0]
        corrstring = self.inputs.context.evla['msinfo'][m.name].corrstring
        
        testBPdcals_result = self._do_testBPdcals()
        
        flagbaddef_result = self._do_FlagBadDeformatters()
        
        uncalspw_result = self._do_Uncalspw(bpcaltable='testBPcal.b', delaycaltable='testdelay.k')
        
        checkflag_result = self._do_Checkflag()
        
        sbpd_result = self._do_semiFinalBPdcals()
        
        checkflag_result = self._do_Checkflag(checkflagmode='semi')
        
        sbpd_result = self._do_semiFinalBPdcals()
        
        uncalspw_result = self._do_Uncalspw(bpcaltable='BPcal.b', delaycaltable='delay.k')
        
        return HeuristicflagResults([])
    
    def analyse(self, results):
        return results
    
    def _do_testBPdcals(self):
        
        inputs = testBPdcals.Inputs(self.inputs.context)
        task = testBPdcals(inputs)
        
        result = self._executor.execute(task, True)
            
        return result
    
    def _do_FlagBadDeformatters(self):
        
        inputs = FlagBadDeformatters.Inputs(self.inputs.context)
        task = FlagBadDeformatters(inputs)
        
        result = self._executor.execute(task, True)
            
        return result
    
    def _do_Uncalspw(self, bpcaltable=None, delaycaltable=None):
        
        inputs = Uncalspw.Inputs(self.inputs.context, bpcaltable=bpcaltable, delaycaltable=delaycaltable)
        task = Uncalspw(inputs)
        
        result = self._executor.execute(task, True)
        
        return result
        
    def _do_Checkflag(self, checkflagmode=None):
        
        inputs = Checkflag.Inputs(self.inputs.context, checkflagmode=checkflagmode)
        task = Checkflag(inputs)
        
        result = self._executor.execute(task, True)
     
        return result
    
    def _do_semiFinalBPdcals(self):
    
        inputs = semiFinalBPdcals.Inputs(self.inputs.context)
        task = semiFinalBPdcals(inputs)
        
        result = self._executor.execute(task, True)
        
        return result
