from __future__ import absolute_import
import os


import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks

LOG = infrastructure.get_logger(__name__)


class CircfeedpolcalResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[]):
        super(CircfeedpolcalResults, self).__init__()
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()

    def merge_with_context(self, context):
        """
        See :method:`~pipeline.infrastructure.api.Results.merge_with_context`
        """
        if not self.final:
            LOG.warn('No circfeedpolcal results')
            return

    def __repr__(self):
        #return 'CircfeedpolcalResults:\n\t{0}'.format(
        #    '\n\t'.join([ms.name for ms in self.mses]))
        return 'CircfeedpolcalResults:'


class CircfeedpolcalInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())


class Circfeedpolcal(basetask.StandardTaskTemplate):
    Inputs = CircfeedpolcalInputs

    def prepare(self):

        LOG.info("This Circfeedpolcal class is running.")

        return CircfeedpolcalResults()

    def analyse(self, results):
        return results

    def _do_somethingcircfeedpolcal(self):

        task = casa_tasks.circfeedpolcalcal(vis=self.inputs.vis, caltable='tempcal.circfeedpolcal')

        return self._executor.execute(task)

