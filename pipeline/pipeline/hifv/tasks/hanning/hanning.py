from __future__ import absolute_import

import os
import shutil

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.casatools as casatools
from pipeline.infrastructure import task_registry

LOG = infrastructure.get_logger(__name__)


class HanningInputs(vdp.StandardInputs):
    def __init__(self, context, vis=None):
        super(HanningInputs, self).__init__()
        self.context = context
        self.vis = vis


class HanningResults(basetask.Results):
    def __init__(self, final=None, pool=None, preceding=None):

        if final is None:
            final = []
        if pool is None:
            pool = []
        if preceding is None:
            preceding = []

        super(HanningResults, self).__init__()

        self.vis = None
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()

    def merge_with_context(self, context):    
        m = context.observing_run.measurement_sets[0]


@task_registry.set_equivalent_casa_task('hifv_hanning')
class Hanning(basetask.StandardTaskTemplate):
    Inputs = HanningInputs
    
    def prepare(self):

        if self._checkpreaveraged():
            if not self._executor._dry_run:
                try:
                    self._do_hanningsmooth()
                    LOG.info("Removing original VIS " + self.inputs.vis)
                    shutil.rmtree(self.inputs.vis)
                    LOG.info("Renaming temphanning.ms to " + self.inputs.vis)
                    os.rename('temphanning.ms', self.inputs.vis)
                except Exception as ex:
                    LOG.warn('Problem encountered with hanning smoothing. ' + str(ex))
        else:
            LOG.warn("Data in this MS are pre-averaged.  CASA task hanningsmooth() was not executed.")

        return HanningResults()
    
    def analyse(self, results):
        return results
    
    def _do_hanningsmooth(self):

        task = casa_tasks.hanningsmooth(vis=self.inputs.vis,
                                        datacolumn='data',
                                        outputvis='temphanning.ms')

        return self._executor.execute(task)

    def _checkpreaveraged(self):

        with casatools.TableReader(self.inputs.vis + '/SPECTRAL_WINDOW') as table:
            effective_bw = table.getcol('EFFECTIVE_BW')
            resolution = table.getcol('RESOLUTION')

        return not(resolution[0][0] < effective_bw[0][0])

