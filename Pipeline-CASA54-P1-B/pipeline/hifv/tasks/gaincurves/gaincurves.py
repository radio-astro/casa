from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.vdp as vdp
from pipeline.h.heuristics import caltable as caltable_heuristic
from pipeline.infrastructure import casa_tasks
from . import resultobjects

LOG = infrastructure.get_logger(__name__)


class GainCurvesInputs(vdp.StandardInputs):
    @vdp.VisDependentProperty
    def caltable(self):
        namer = caltable_heuristic.GainCurvestable()
        casa_args = self._get_task_args(ignore=('caltable',))
        return namer.calculate(output_dir=self.output_dir, stage=self.context.stage, **casa_args)

    @vdp.VisDependentProperty
    def parameter(self):
        return []

    def __init__(self, context, output_dir=None, vis=None, caltable=None, parameter=None):
        self.context = context
        self.output_dir = output_dir
        self.vis = vis
        self.caltable = caltable
        self.parameter = parameter

    def to_casa_args(self):
        args = super(GainCurvesInputs, self).to_casa_args()
        args['caltype'] = 'gc'
        return args


class GainCurves(basetask.StandardTaskTemplate):
    Inputs = GainCurvesInputs    

    def prepare(self):
        inputs = self.inputs

        gencal_args = inputs.to_casa_args()
        gencal_job = casa_tasks.gencal(**gencal_args)
        self._executor.execute(gencal_job)

        callist = []
        calto = callibrary.CalTo(vis=inputs.vis)
        calfrom = callibrary.CalFrom(gencal_args['caltable'], caltype='gc', interp='', calwt=False)
        calapp = callibrary.CalApplication(calto, calfrom)
        callist.append(calapp)

        return resultobjects.GainCurvesResults(pool=callist)

    def analyse(self, result):

        # double-check that the caltable was actually generated
        on_disk = [ca for ca in result.pool
                   if ca.exists() or self._executor._dry_run]
        result.final[:] = on_disk

        missing = [ca for ca in result.pool
                   if ca not in on_disk and not self._executor._dry_run]
        result.error.clear()
        result.error.update(missing)

        return result
