from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
from pipeline.infrastructure import casa_tasks
from pipeline.infrastructure import task_registry

LOG = infrastructure.get_logger(__name__)


class LinfeedpolcalResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[]):
        super(LinfeedpolcalResults, self).__init__()
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()

    def merge_with_context(self, context):
        """
        See :method:`~pipeline.infrastructure.api.Results.merge_with_context`
        """
        if not self.final:
            LOG.warn('No linfeedpolcal results')
            return

    def __repr__(self):
        #return 'LinfeedpolcalResults:\n\t{0}'.format(
        #    '\n\t'.join([ms.name for ms in self.mses]))
        return 'LinfeedpolcalResults:'


class LinfeedpolcalInputs(vdp.StandardInputs):
    def __init__(self, context, vis=None):
        self.context = context
        self.vis = vis


@task_registry.set_equivalent_casa_task('hif_linfeedpolcal')
class Linfeedpolcal(basetask.StandardTaskTemplate):
    Inputs = LinfeedpolcalInputs

    def prepare(self):

        LOG.info("This Linfeedpolcal class is running.")

        return LinfeedpolcalResults()

    def analyse(self, results):
        return results

    def _do_somethinglinfeedpolcal(self):

        task = casa_tasks.linfeedpolcalcal(vis=self.inputs.vis, caltable='tempcal.linfeedpolcal')

        return self._executor.execute(task)

