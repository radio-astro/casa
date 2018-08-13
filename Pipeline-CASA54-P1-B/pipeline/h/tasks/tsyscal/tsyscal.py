from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.vdp as vdp
from pipeline.h.heuristics import caltable as caltable_heuristic
from pipeline.h.heuristics.tsysspwmap import tsysspwmap
from pipeline.infrastructure import casa_tasks
from pipeline.infrastructure import task_registry
from . import resultobjects

__all__ = [
    'Tsyscal',
    'TsyscalInputs',
]

LOG = infrastructure.get_logger(__name__)


class TsyscalInputs(vdp.StandardInputs):
    """
    TsyscalInputs defines the inputs for the Tsyscal pipeline task.
    """
    chantol = vdp.VisDependentProperty(default=1)

    @vdp.VisDependentProperty
    def caltable(self):
        """
        Get the caltable argument for these inputs.

        If set to a table-naming heuristic, this should give a sensible name
        considering the current CASA task arguments.
        """
        namer = caltable_heuristic.TsysCaltable()
        casa_args = self._get_task_args(ignore=('caltable',))
        return namer.calculate(output_dir=self.output_dir, stage=self.context.stage, **casa_args)

    def __init__(self, context, output_dir=None, vis=None, caltable=None, chantol=None):
        super(TsyscalInputs, self).__init__()

        # pipeline inputs
        self.context = context
        # vis must be set first, as other properties may depend on it
        self.vis = vis
        self.output_dir = output_dir

        # data selection arguments
        self.caltable = caltable

        # solution parameters
        self.chantol = chantol

    # Convert to CASA gencal task arguments.
    def to_casa_args(self):
        return {
            'vis': self.vis,
            'caltable': self.caltable
        }


@task_registry.set_equivalent_casa_task('h_tsyscal')
@task_registry.set_casa_commands_comment('The Tsys calibration and spectral window map is computed.')
class Tsyscal(basetask.StandardTaskTemplate):
    Inputs = TsyscalInputs    

    def prepare(self):
        inputs = self.inputs

        # make a note of the current inputs state before we start fiddling
        # with it. This origin will be attached to the final CalApplication.
        origin = callibrary.CalAppOrigin(task=Tsyscal, 
                                         inputs=inputs.to_casa_args())

        # construct the Tsys cal file
        gencal_args = inputs.to_casa_args()
        gencal_job = casa_tasks.gencal(caltype='tsys', **gencal_args)
        self._executor.execute(gencal_job)

        LOG.todo('tsysspwmap heuristic re-reads measurement set!')
        LOG.todo('tsysspwmap heuristic won\'t handle missing file')

        nospwmap, spwmap = tsysspwmap(
            ms=inputs.ms, tsystable=gencal_args['caltable'],
            tsysChanTol=inputs.chantol)

        callist = []
        calto = callibrary.CalTo(vis=inputs.vis)
        calfrom = callibrary.CalFrom(
            gencal_args['caltable'], caltype='tsys', gainfield='nearest',
            spwmap=spwmap, interp='linear,linear')
        calapp = callibrary.CalApplication(calto, calfrom, origin)
        callist.append(calapp)

        return resultobjects.TsyscalResults(pool=callist, unmappedspws=nospwmap)

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
