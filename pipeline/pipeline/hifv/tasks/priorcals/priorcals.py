"""
Example usage:

inputs = pipeline.vla.tasks.priorcals.Priorcals.Inputs(context)
task = pipeline.vla.tasks.priorcals.Priocals(inputs)
result = task.exectue(dry_run=False)
result.accept(context)

"""

from __future__ import absolute_import
import types
import math

import pipeline.infrastructure.casatools as casatools
import numpy


from pipeline.hif.heuristics import fieldnames
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.utils as utils
from . import resultobjects

from pipeline.hifv.tasks.gaincurves import GainCurves
from pipeline.hifv.tasks.opcal import Opcal
from pipeline.hifv.tasks.rqcal import Rqcal
from pipeline.hifv.tasks.swpowcal import Swpowcal
from pipeline.hif.tasks.antpos import Antpos

from pipeline.hifv.tasks.vlautils import VLAUtils

LOG = infrastructure.get_logger(__name__)



class PriorcalsInputs(basetask.StandardInputs):
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())

    def to_casa_args(self):
        raise NotImplementedError

class Priorcals(basetask.StandardTaskTemplate):
    Inputs = PriorcalsInputs

    def prepare(self):
        inputs = self.inputs

        callist = []
        
        gc_result = self._do_gaincurves()
        oc_result = self._do_opcal()
        rq_result = self._do_rqcal()
        #sw_result = self._do_swpowcal()
        antpos_result = self._do_antpos()
        
        try:
            result.merge_withcontext(self.inputs.context)
        except:
            LOG.error('No antenna position corrections.')
            
        return resultobjects.PriorcalsResults(pool=callist, gc_result=gc_result,
                                              oc_result=oc_result, rq_result=rq_result,
                                              antpos_result=antpos_result)

    def analyse(self, results):
	    return results

    def _do_gaincurves(self):
        """Run gaincurves task"""

        inputs = GainCurves.Inputs(self.inputs.context)
        task = GainCurves(inputs)
        return self._executor.execute(task)

    def _do_opcal(self):
        """Run opcal task"""

        inputs = Opcal.Inputs(self.inputs.context)
        task = Opcal(inputs)
        return self._executor.execute(task)

    def _do_rqcal(self):
        """Run requantizer gains task"""

        inputs = Rqcal.Inputs(self.inputs.context)
        task = Rqcal(inputs)
        return self._executor.execute(task)

    def _do_swpowcal(self):
        """Run switched power task"""

        inputs = Swpowcal.Inputs(self.inputs.context)
        task = Swpowcal(inputs)
        return self._executor.execute(task)

    def _do_antpos(self):
        """Run hif_antpos to correct for antenna positions"""

        inputs = Antpos.Inputs(self.inputs.context)
        task = Antpos(inputs)
        return self._executor.execute(task)
