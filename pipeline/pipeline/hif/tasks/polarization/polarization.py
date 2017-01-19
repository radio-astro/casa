from __future__ import absolute_import
import os


import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.hif.tasks.gaincal as gaincal

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


class PolarizationInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())


class Polarization(basetask.StandardTaskTemplate):
    Inputs = PolarizationInputs

    def prepare(self):

        LOG.info("This Polarization class is running.")



        return PolarizationResults()

    def analyse(self, results):
        return results

    def do_gaincal(self, caltable, RefAntOutput):
        inputs = self.inputs

        #Similar inputs to linpolcal.py
        task_inputs = gaincal.GTypeGaincal.Inputs(inputs.context,
                                                  output_dir=inputs.output_dir,
                                                  vis=inputs.vis,
                                                  caltable=caltable,
                                                  field=inputs.field,
                                                  intent='',
                                                  scan='',
                                                  spw=inputs.spw,
                                                  solint='int',
                                                  gaintype='KCROSS',
                                                  refant=RefAntOutput[0].lower(),
                                                  smodel=[1, 0, 1, 0],
                                                  to_intent='PHASE,TARGET,AMPLITUDE,BANDPASS',
                                                  to_field=None)

        gaincal_task = gaincal.GTypeGaincal(task_inputs)
        result = self._executor.execute(gaincal_task, merge=True)

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
                     'gaintable': GainTables,
                     'poltype': poltype,
                     'gainfield': ['', '', '', '0', '0', ''],
                     'spwmap': [[], spwmapK, [], [], [], []],
                     'parang': True}

        task = casa_tasks.polcal(**task_args)

        return self._executor.execute(task)


