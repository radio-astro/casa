from __future__ import absolute_import

import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure as infrastructure
import shutil
import os


LOG = infrastructure.get_logger(__name__)


class HanningInputs(basetask.StandardInputs):
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())


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


class Hanning(basetask.StandardTaskTemplate):
    Inputs = HanningInputs
    
    def prepare(self):

        if not self._executor._dry_run:
            try:
                self._do_hanningsmooth()
                LOG.info("Removing original VIS " + self.inputs.vis)
                shutil.rmtree(self.inputs.vis)
                LOG.info("Renaming temphanning.ms to " + self.inputs.vis)
                os.rename('temphanning.ms', self.inputs.vis)
            except:
                LOG.warn('Problem encountered with hanning smoothing.')

        return HanningResults()
    
    def analyse(self, results):
        return results
    
    def _do_hanningsmooth(self):

        task = casa_tasks.hanningsmooth(vis=self.inputs.vis,
                                        datacolumn='data',
                                        outputvis='temphanning.ms')

        return self._executor.execute(task)
