from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.casatools as casatools
import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.callibrary as callibrary

from pipeline.hifv.tasks.flagging.uncalspw import Uncalspw

import itertools

from pipeline.hif.tasks import applycal


LOG = infrastructure.get_logger(__name__)

class ApplycalsInputs(basetask.StandardInputs):
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())

class ApplycalsResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[]):
        super(ApplycalsResults, self).__init__()

        self.vis = None
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()
        
class Applycals(basetask.StandardTaskTemplate):
    Inputs = ApplycalsInputs
    
    def prepare(self):
        
        applycal_results = self._do_applycal(self.inputs.context)
        
        inputs = Uncalspw.Inputs(self.inputs.context, bpcaltable='finalBPcal.b', delaycaltable='finaldelay.k')
        task = Uncalspw(inputs)
        result = self._executor.execute(task)
        
        
        return ApplycalsResults()
    
    def analyse(self, results):
	return results
    
    def _do_applycal(self, context=None):
        """Run CASA task applycal"""
        
        m = context.observing_run.measurement_sets[0]
        
        tablesToAdd = ['finaldelay.k', 'finalBPcal.b', 'averagephasegain.g', 'finalampgaincal.g', 'finalphasegaincal.g']
        
        for addcaltable in tablesToAdd:
            calto = callibrary.CalTo(self.inputs.vis)
            calfrom = callibrary.CalFrom(gaintable=addcaltable, interp='', calwt=False)
            self.inputs.context.callibrary.add(calto, calfrom)
        
        applycal_inputs = applycal.Applycal.Inputs(self.inputs.context,
            vis = self.inputs.vis,
            field = '',
            spw = '',
            intent = '',
            flagbackup = False,
            calwt = False)
        
        applycal_task = applycal.Applycal(applycal_inputs)
        
        return self._executor.execute(applycal_task, merge=True)