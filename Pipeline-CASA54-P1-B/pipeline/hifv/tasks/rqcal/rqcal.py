from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.vdp as vdp
from pipeline.h.heuristics import caltable as caltable_heuristic
from pipeline.infrastructure import casa_tasks
from . import resultobjects

LOG = infrastructure.get_logger(__name__)


class RqcalInputs(vdp.StandardInputs):
    @vdp.VisDependentProperty
    def caltable(self):
        namer = caltable_heuristic.RqCaltable()
        casa_args = self._get_task_args(ignore=('caltable',))
        return namer.calculate(output_dir=self.output_dir, stage=self.context.stage, **casa_args)

    @vdp.VisDependentProperty
    def parameter(self):
        return []

    def __init__(self, context, output_dir=None, vis=None, caltable=None, parameter=None):
        self.context = context
        self.output_dir = output_dir
        self.vis = vis
        self.parameter = parameter
        self.caltable = caltable

    def to_casa_args(self):
        args = super(RqcalInputs, self).to_casa_args()
        args['caltype'] = 'rq'
        return args


class Rqcal(basetask.StandardTaskTemplate):
    Inputs = RqcalInputs    

    def prepare(self):
        inputs = self.inputs

        #with casatools.MSReader(inputs.vis) as ms:
        #    ms_summary = ms.summary()

        #startdate = ms_summary['BeginTime']
        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        startdate = m.start_time['m0']['value']

        # Note from original scripted pipeline:
        # Apply switched power calibration (when commissioned); for now, just
        # requantizer gains (needs casa4.1!), and only for data with
        # sensible switched power tables (Feb 24, 2011)
        callist = []
        if startdate >= 55616.6:
            gencal_args = inputs.to_casa_args()
            gencal_job = casa_tasks.gencal(**gencal_args)
            self._executor.execute(gencal_job)

            calto = callibrary.CalTo(vis=inputs.vis)
            calfrom = callibrary.CalFrom(gencal_args['caltable'], caltype='rq', interp='', calwt=False)
            calapp = callibrary.CalApplication(calto, calfrom)
            callist.append(calapp)

        return resultobjects.RqcalResults(pool=callist)

    def analyse(self, result):
        # With no best caltable to find, our task is simply to set the one
        # caltable as the best result

        # double-check that the caltable was actually generated
        on_disk = [ca for ca in result.pool
                   if ca.exists() or self._executor._dry_run]
        result.final[:] = on_disk

        missing = [ca for ca in result.pool
                   if ca not in on_disk and not self._executor._dry_run]
        result.error.clear()
        result.error.update(missing)

        return result
