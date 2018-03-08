from __future__ import absolute_import
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
from pipeline.infrastructure import casa_tasks, task_registry
from recipes import tec_maps

LOG = infrastructure.get_logger(__name__)


class ${taskname.capitalize()}Results(basetask.Results):
    def __init__(self):
        super(${taskname.capitalize()}Results, self).__init__()
        self.pipeline_casa_task = '${taskname.capitalize()}'

    def merge_with_context(self, context):
        """
        See :method:`~pipeline.infrastructure.api.Results.merge_with_context`
        """
        return

    def __repr__(self):
        #return '${taskname.capitalize()}Results:\n\t{0}'.format(
        #    '\n\t'.join([ms.name for ms in self.mses]))
        return '${taskname.capitalize()}Results:'


class ${taskname.capitalize()}Inputs(vdp.StandardInputs):
    def __init__(self, context, vis=None):
        self.context = context
        self.vis = vis

@task_registry.set_equivalent_casa_task('${package}_${taskname.lower()}')
@task_registry.set_casa_commands_comment('Add your task description for inclusion in casa_commands.log')
class ${taskname.capitalize()}(basetask.StandardTaskTemplate):
    Inputs = ${taskname.capitalize()}Inputs

    def prepare(self):

        LOG.info("This ${taskname.capitalize()} class is running.")

        return ${taskname.capitalize()}Results()

    def analyse(self, results):
        return results

    def _do_something${taskname.lower()}(self):

        task = casa_tasks.${taskname.lower()}cal(vis=self.inputs.vis, caltable='tempcal.${taskname.lower()}')

        return self._executor.execute(task)

    def _do_tec_maps(self):

        tec_maps.create(vis=self.vis, doplot=True, imname='iono')
        # gencal_job = casa_tasks.gencal(**gencal_args)
        gencal_job = casa_tasks.gencal(vis=self.vis, caltable='tec.cal', caltype='tecim', infile='iono.IGS_TEC.im')
        self._executor.execute(gencal_job)

