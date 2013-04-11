from __future__ import absolute_import
import types

from pipeline.hif.heuristics import caltable as caltable_heuristic
from pipeline.hif.heuristics.tsysspwmap import tsysspwmap
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
from pipeline.infrastructure import casa_tasks
from . import resultobjects

LOG = infrastructure.get_logger(__name__)


class TsyscalInputs(basetask.StandardInputs):
    def __init__(self, context, output_dir=None, vis=None, caltable=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())
        setattr(self, 'caltype', 'tsys')

    @property
    def caltable(self):
        # The value of caltable is ms-dependent, so test for multiple
        # measurement sets and listify the results if necessary 
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('caltable')
        
        # Get the name.
        if callable(self._caltable):
            casa_args = self._get_partial_task_args()
            return self._caltable(output_dir=self.output_dir,
                                  stage=self.context.stage,
                                  **casa_args)
        return self._caltable
        
    @caltable.setter
    def caltable(self, value):
        if value is None:
            value = caltable_heuristic.TsysCaltable()
        self._caltable = value

    # Avoids circular dependency on caltable.
    def _get_partial_task_args(self):
        return {'vis'     : self.vis, 
                'caltype' : self.caltype}

    # Convert to CASA gencal task arguments.
    def to_casa_args(self):
        return {'vis'      : self.vis,
                'caltable' : self.caltable,
                'caltype'  : self.caltype}


class Tsyscal(basetask.StandardTaskTemplate):
    Inputs = TsyscalInputs    

    def prepare(self):
        inputs = self.inputs

        gencal_args = inputs.to_casa_args()
        gencal_job = casa_tasks.gencal(**gencal_args)
        self._executor.execute(gencal_job)

        LOG.warning('TODO: tsysspwmap heuristic re-reads measurement set!')
        LOG.warning("TODO: tsysspwmap heuristic won't handle missing file")
        spwmap = tsysspwmap(vis=inputs.vis, tsystable=gencal_args['caltable'])

        callist = []
        calto = callibrary.CalTo(vis=inputs.vis)
        calfrom = callibrary.CalFrom(gencal_args['caltable'], caltype='tsys',
          gainfield='nearest', spwmap=spwmap, interp='linear,linear')
        calapp = callibrary.CalApplication(calto, calfrom)
        callist.append(calapp)

        return resultobjects.TsyscalResults(pool=callist)


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
