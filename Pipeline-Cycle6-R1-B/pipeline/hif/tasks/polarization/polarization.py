from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
from pipeline.infrastructure import casa_tasks
from pipeline.infrastructure import task_registry

LOG = infrastructure.get_logger(__name__)


class PolarizationResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[]):
        super(PolarizationResults, self).__init__()
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()

    def merge_with_context(self, context):
        """
        See :method:`~pipeline.infrastructure.api.Results.merge_with_context`
        """
        if not self.final:
            LOG.warn('No polarization results')
            return

    def __repr__(self):
        #return 'PolarizationResults:\n\t{0}'.format(
        #    '\n\t'.join([ms.name for ms in self.mses]))
        return 'PolarizationResults:'


class PolarizationInputs(vdp.StandardInputs):
    def __init__(self, context, vis=None):
        super(PolarizationInputs, self).__init__()
        self.context = context
        self.vis = vis


@task_registry.set_equivalent_casa_task('hif_polarization')
class Polarization(basetask.StandardTaskTemplate):
    Inputs = PolarizationInputs

    def prepare(self):

        LOG.info("This Polarization class is running.")



        return PolarizationResults()

    def analyse(self, results):
        return results

    def do_gaincal(self, caltable):
        inputs = self.inputs


        result = True

        return result

    def do_polcal(self, caltable, poltype, RefAntOutput):

        '''
        From Chris Hales script

        polcal(vis='vlass3C48.ms', caltable='vlass3C48.D_1', field='0', refant='ea20', poltype='Df+QU',
               gaintable=['vlass3C48.antpos', 'vlass3C48.K', 'vlass3C48.B', 'vlass3C48.Gp', 'vlass3C48.Ga',
                          'vlass3C48.Kx_1'], gainfield=['', '', '', '0', '0', ''], spwmap=[[], spwmapK, [], [], [], []])
        polcal(vis='vlass3C286.ms', caltable='vlass3C286.D_1', field='1', refant='ea20', poltype='Df+QU',
               gaintable=['vlass3C286.antpos', 'vlass3C286.K', 'vlass3C286.B', 'vlass3C286.Gp', 'vlass3C286.Ga',
                          'vlass3C286.Kx_1'], gainfield=['', '', '', '1', '1', ''],
               spwmap=[[], spwmapK, [], [], [], []])
        '''

        spwmapK = [0, 0, 0]
        spwmapGinit = [1, 1, 1]

        task_args = {'vis': self.inputs.vis,
                     'caltable': caltable,
                     'field': '0',
                     'refant': RefAntOutput[0].lower(),
                     'gaintable': [],
                     'poltype': poltype,
                     'gainfield': ['', '', '', '0', '0', ''],
                     'spwmap': [[], spwmapK, [], [], [], []],
                     'parang': True}

        task = casa_tasks.polcal(**task_args)

        return self._executor.execute(task)


