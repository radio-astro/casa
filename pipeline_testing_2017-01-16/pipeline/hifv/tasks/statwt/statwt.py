from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure as infrastructure
from pipeline.hifv.heuristics import cont_file_to_CASA

LOG = infrastructure.get_logger(__name__)

# CALCULATE DATA WEIGHTS BASED ON ST. DEV. WITHIN EACH SPW
# use statwt


class StatwtInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())



class StatwtResults(basetask.Results):
    def __init__(self, jobs=[]):
        super(StatwtResults, self).__init__()

        self.jobs=jobs
        
    def __repr__(self):
        s = 'Statwt results:\n'
        for job in self.jobs:
            s += '%s performed. ' % str(job)
        return s 


class Statwt(basetask.StandardTaskTemplate):
    Inputs = StatwtInputs
    
    def prepare(self):

        statwt_result = self._do_statwt()
        
        return StatwtResults([statwt_result])
    
    def analyse(self, results):
        return results
    
    def _do_statwt(self):

        fielddict = cont_file_to_CASA()

        if fielddict != {}: LOG.info('cont.dat file present.  Using VLA Spectral Line Heuristics for task statwt.')

        if (fielddict == {}):
            task_args = {'vis'          : self.inputs.vis,
                         'dorms'        : False,
                         'fitspw'       : '',
                         'fitcorr'      : '',
                         'combine'      : '',
                         'minsamp'      : 2,
                         'field'        : '',
                         'spw'          : '',
                         'datacolumn'   : 'corrected'}
                     
            job = casa_tasks.statwt(**task_args)
            
            return self._executor.execute(job)

        if (fielddict != {}):


            for field in fielddict.keys():
                task_args = {'vis'          : self.inputs.vis,
                         'dorms'        : False,
                         'fitspw'       : fielddict[field],
                         'fitcorr'      : '',
                         'combine'      : '',
                         'minsamp'      : 2,
                         'field'        : field,
                         'spw'          : '',
                         'datacolumn'   : 'corrected'}

                job = casa_tasks.statwt(**task_args)

                statwt_result = self._executor.execute(job)

            return statwt_result
