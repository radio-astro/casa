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
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())

class ApplycalsResults(basetask.Results):
    def __init__(self, applied=[]):
        """
        Construct and return a new ApplycalResults.
        
        The resulting object should be initialized with a list of
        CalibrationTables corresponding to the caltables applied by this task.

        :param applied: caltables applied by this task
        :type applied: list of :class:`~pipeline.domain.caltable.CalibrationTable`
        """
        super(ApplycalResults, self).__init__()
        self.applied = set()
        self.applied.update(applied)
        
    def merge_with_context(self, context):
        """
        See :method:`~pipeline.Results.merge_with_context`
        """
        if not self.applied:
            LOG.error('No results to merge')

        for calapp in self.applied:
            LOG.trace('Marking %s as applied' % calapp.as_applycal())
            context.callibrary.mark_as_applied(calapp.calto, calapp.calfrom)
            
    def __repr__(self):
        for caltable in self.applied:
            s = 'ApplycalResults:\n'
            if type(caltable.gaintable) is types.ListType:
                basenames = [os.path.basename(x) for x in caltable.gaintable]
                s += '\t{name} applied to {vis} spw #{spw}\n'.format(
                    spw=caltable.spw, vis=os.path.basename(caltable.vis),
                    name=','.join(basenames))
            else:
                s += '\t{name} applied to {vis} spw #{spw}\n'.format(
                    name=caltable.gaintable, spw=caltable.spw, 
                    vis=os.path.basename(caltable.vis))
        return s
        
class Applycals(basetask.StandardTaskTemplate):
    Inputs = ApplycalsInputs
    
    def prepare(self):
        
        #Run applycal
        applycal_results = self._do_applycal(self.inputs.context)
   
        inputs = Uncalspw.Inputs(self.inputs.context, bpcaltable='finalBPcal.b', delaycaltable='finaldelay.k')
        task = Uncalspw(inputs)
        result = self._executor.execute(task)
        
        
        return applycal_results
    
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