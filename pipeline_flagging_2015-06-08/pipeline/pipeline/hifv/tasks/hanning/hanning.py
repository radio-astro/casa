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




LOG = infrastructure.get_logger(__name__)

class HanningInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())


class HanningResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[]):
        super(HanningResults, self).__init__()

        self.vis = None
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()

        
    def merge_with_context(self, context):    
        m = context.observing_run.measurement_sets[0]


class Hanning(basetask.StandardTaskTemplate):
    Inputs = HanningInputs
    
    def prepare(self):
        
        result = self._do_hanningsmooth()
        
        return HanningResults()
    
    def analyse(self, results):
	return results
    
    def _do_hanningsmooth(self):

        task = casa_tasks.hanningsmooth(vis=self.inputs.vis, 
                                        datacolumn='data',
                                        outputvis='')

        self._executor.execute(task)
