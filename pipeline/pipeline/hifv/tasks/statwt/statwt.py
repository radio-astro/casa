from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
from pipeline.hifv.heuristics import cont_file_to_CASA
from pipeline.infrastructure import casa_tasks
from pipeline.infrastructure import task_registry

LOG = infrastructure.get_logger(__name__)

# CALCULATE DATA WEIGHTS BASED ON ST. DEV. WITHIN EACH SPW
# use statwt


class StatwtInputs(vdp.StandardInputs):
    def __init__(self, context, vis=None):
        super(StatwtInputs, self).__init__()
        self.context = context
        self.vis = vis


class StatwtResults(basetask.Results):
    def __init__(self, jobs=None, flag_summaries=[]):

        if jobs is None:
            jobs = []

        super(StatwtResults, self).__init__()
        self.jobs = jobs
        self.summaries = flag_summaries
        
    def __repr__(self):
        s = 'Statwt results:\n'
        for job in self.jobs:
            s += '%s performed. ' % str(job)
        return s 


@task_registry.set_equivalent_casa_task('hifv_statwt')
class Statwt(basetask.StandardTaskTemplate):
    Inputs = StatwtInputs
    
    def prepare(self):

        flag_summaries = []
        # flag statistics before task
        flag_summaries.append(self._do_flagsummary('before'))
        # actual statwt operation
        statwt_result = self._do_statwt()
        # flag statistics after task
        flag_summaries.append(self._do_flagsummary('statwt'))
        
        return StatwtResults(jobs=[statwt_result], flag_summaries=flag_summaries)
    
    def analyse(self, results):
        return results
    
    def _do_statwt(self):

        fielddict = cont_file_to_CASA()

        if fielddict != {}:
            LOG.info('cont.dat file present.  Using VLA Spectral Line Heuristics for task statwt.')

        task_args = {'vis': self.inputs.vis,
                     'fitspw': '',
                     'fitcorr': '',
                     'combine': '',
                     'minsamp': 10,
                     'field': '',
                     'spw': '',
                     'datacolumn': 'corrected'}

        if fielddict == {}:
            job = casa_tasks.statwt(**task_args)
            return self._executor.execute(job)

        # cont.dat file present and need to execute by field and fitspw
        if fielddict != {}:
            for field in fielddict.keys():
                task_args['fitspw'] = fielddict[field]
                task_args['field'] = field
                job = casa_tasks.statwt(**task_args)

                statwt_result = self._executor.execute(job)

            return statwt_result

    def _do_flagsummary(self, name):
        job = casa_tasks.flagdata(name=name, vis = self.inputs.vis, mode='summary')
        return self._executor.execute(job)