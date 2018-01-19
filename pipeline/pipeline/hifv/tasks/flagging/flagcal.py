from __future__ import absolute_import
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
from pipeline.infrastructure import casa_tasks

LOG = infrastructure.get_logger(__name__)


class FlagcalResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[]):
        super(FlagcalResults, self).__init__()
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()

    def merge_with_context(self, context):
        """
        See :method:`~pipeline.infrastructure.api.Results.merge_with_context`
        """
        if not self.final:
            LOG.warn('No flagcal results')
            return

    def __repr__(self):
        #return 'FlagcalResults:\n\t{0}'.format(
        #    '\n\t'.join([ms.name for ms in self.mses]))
        return 'FlagcalResults:'


class FlagcalInputs(vdp.StandardInputs):
    caltable = vdp.VisDependentProperty(default='finalampgaincal.g')
    clipminmax = vdp.VisDependentProperty(default=[0.9, 1.1])

    def __init__(self, context, vis=None, caltable=None, clipminmax=None):
        super(FlagcalInputs, self).__init__()
        self.context = context
        self.vis = vis
        self.caltable = caltable
        self.clipminmax = clipminmax


class Flagcal(basetask.StandardTaskTemplate):
    Inputs = FlagcalInputs

    def prepare(self):

        LOG.info("This Flagcal class is running.")

        LOG.info(self.inputs.caltable)
        LOG.info(','.join([str(x) for x in self.inputs.clipminmax]))

        # Check finalcal stage prefixes.
        basevis = os.path.basename(self.inputs.vis)
        caltable = self.inputs.caltable
        if not os.path.exists(caltable):
            caltable = basevis + '.' + caltable

        flagcal_result = self._do_flagdata(caltable=caltable,
                                           clipminmax=self.inputs.clipminmax)

        return flagcal_result

    def analyse(self, results):
        return results

    def _do_flagdata(self, caltable=None, clipminmax=None):
        task_args = {'vis'         : caltable,
                     'mode'        : 'clip',
                     'correlation' : 'ABS_ALL',
                     'datacolumn'  : 'CPARAM',
                     'clipminmax'  : clipminmax,
                     'clipoutside' : True,
                     'action'      : 'apply',
                     'flagbackup'  : False,
                     'savepars'    : False}

        job = casa_tasks.flagdata(**task_args)

        self._executor.execute(job)

        return FlagcalResults([job])
