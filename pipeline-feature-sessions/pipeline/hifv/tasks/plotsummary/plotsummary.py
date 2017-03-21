from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure as infrastructure

from pipeline.hif.tasks import applycal

LOG = infrastructure.get_logger(__name__)


class PlotSummaryInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())


class PlotSummaryResults(basetask.Results):
    def __init__(self, final=None, pool=None, preceding=None):
        if final is None:
            final = []
        if pool is None:
            pool = []
        if preceding is None:
            preceding = []

        super(PlotSummaryResults, self).__init__()

        self.vis = None
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()


class PlotSummary(basetask.StandardTaskTemplate):
    Inputs = PlotSummaryInputs

    def prepare(self):

        result = self._do_plotsummary()

        return result

    def analyse(self, results):
        return results

    def _do_plotsummary(self):

        applycal_inputs = applycal.IFApplycal.Inputs(self.inputs.context,
                                                     vis=self.inputs.vis,
                                                     field='',
                                                     spw='',
                                                     intent='',
                                                     flagbackup=False,
                                                     flagsum=False, flagdetailedsum=False)

        applycal_task = applycal.IFApplycal(applycal_inputs)

        result = applycal_task.execute(dry_run=True)

        return result