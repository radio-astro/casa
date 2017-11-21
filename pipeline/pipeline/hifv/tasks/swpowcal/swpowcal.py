from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.vdp as vdp
from pipeline.h.heuristics import caltable as caltable_heuristic
from pipeline.infrastructure import casa_tasks
from . import resultobjects

LOG = infrastructure.get_logger(__name__)


class SwpowcalInputs(vdp.StandardInputs):
    spw = vdp.VisDependentProperty(default='')

    @vdp.VisDependentProperty
    def caltable(self):
        namer = caltable_heuristic.SwpowCaltable()
        casa_args = self._get_task_args(ignore=('caltable',))
        return namer.calculate(output_dir=self.output_dir, stage=self.context.stage, **casa_args)

    @vdp.VisDependentProperty
    def parameter(self):
        return []

    def __init__(self, context, output_dir=None, vis=None, caltable=None, parameter=None, spw=None):
        self.context = context
        self.output_dir = output_dir
        self.vis = vis
        self.spw = spw
        self.parameter = parameter
        self.caltable = caltable

    def to_casa_args(self):
        args = super(SwpowcalInputs, self).to_casa_args()
        args['caltype'] = 'swpow'
        return args


class Swpowcal(basetask.StandardTaskTemplate):
    Inputs = SwpowcalInputs    

    def prepare(self):
        inputs = self.inputs

        with casatools.MSReader(inputs.vis) as ms:
            ms_summary = ms.summary()

        startdate = ms_summary['BeginTime']

        # Note from the original scripted pipeline:
        # Lastly, make switched power table.  This is not used in the
        # pipeline, but may be used for QA and for flagging, especially at
        # S-band for fields near the geostationary satellite belt.  Only
        # relevant for data taken on 24-Feb-2011 or later.
        callist = []
        if startdate >= 55616.6:
            gencal_args = inputs.to_casa_args()

            # CAS-10216
            # spw argument in gencal should be spw='', inputs specified and passed to priorcals is for plotting only
            gencal_args['spw'] = ''
            gencal_job = casa_tasks.gencal(**gencal_args)
            self._executor.execute(gencal_job)

            calto = callibrary.CalTo(vis=inputs.vis)
            calfrom = callibrary.CalFrom(gencal_args['caltable'], caltype='swpow', interp='', calwt=False)
            calapp = callibrary.CalApplication(calto, calfrom)
            callist.append(calapp)

        return resultobjects.SwpowcalResults(pool=callist, spw=inputs.spw)

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
