from __future__ import absolute_import
import os
import types

import pipeline.infrastructure.api as api
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.logging as logging
from pipeline.infrastructure.jobrequest import casa_tasks

from pipeline.hif.heuristics.tsysspwmap import tsysspwmap as tsysspwmap
from pipeline.hif.heuristics import caltable as tcaltable

LOG = logging.get_logger(__name__)


class TsyscalResults(api.Results):
    def __init__(self, final=[], pool=[], preceding=[]):
        self.vis = None
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()

    def merge_with_context(self, context):
        if not self.final:
            LOG.error('No results to merge')
            return

        for calapp in self.final:
            LOG.debug('Adding calibration to callibrary:\n'
                      '%s\n%s' % (calapp.calto, calapp.calfrom))
            context.callibrary.add(calapp.calto, calapp.calfrom)

    def __repr__(self):

	# Format the Tsyscal results.
        s = 'TsyscalResults:\n'
        for calapplication in self.final:
            s += '\tBest caltable for spw #{spw} in {vis} is {name}\n'.format(
                spw=calapplication.spw, vis=os.path.basename(calapplication.vis),
                name=calapplication.gaintable)
	return s


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
            value = tcaltable.TsysCaltable()
        self._caltable = value

    # Avoids circular dependency on caltable.
    # NOT SURE WHY THIS IS NECCESARY.
    def _get_partial_task_args(self):
	return {'vis': self.vis, 'caltype': self.caltype}

    # Convert to CASA gencal task arguments.
    def to_casa_args(self):
	return {'vis': self.vis,
	        'caltable': self.caltable,
		'caltype': self.caltype}


class Tsyscal(basetask.StandardTaskTemplate):
    Inputs = TsyscalInputs    

    def prepare(self):
        inputs = self.inputs

        gencal_args = inputs.to_casa_args()
        gencal_job = casa_tasks.gencal(**gencal_args)
        self._executor.execute(gencal_job)

        result = TsyscalResults()
        LOG.warning('TODO: tsysspwmap heuristic re-reads measurement set!')
        LOG.warning("TODO: tsysspwmap heuristic won't handle missing file")
        #spwmap = tsysspwmap.tsysspwmap(vis=inputs.vis,
        spwmap = tsysspwmap(vis=inputs.vis, tsystable=gencal_args['caltable'])

        # assuming this task handles a single ms
	#fields = inputs.ms.get_fields()
	#calapplist = []
	#for f in fields:
	    #LOG.warning('TODO: Replace loop over fields with nearest option')
	    #LOG.warning('    Field %s  id %s' % (f.name, f.id))
            #calto = callibrary.CalTo(vis=inputs.vis, field=f.id)

            ## careful now! Calling inputs.caltable mid-task will remove the
            ## newly-created caltable, so we must look at the task arguments
            ## instead
	    ## Note not sure about the frequency spline interpolation
            #calfrom = callibrary.CalFrom(gencal_args['caltable'],
	        ##caltype='tsys', gainfield='nearest', spwmap=spwmap,
	        #caltype='tsys', gainfield=str(f.id), spwmap=spwmap,
	    	#interp='nearest,spline')
            #calapplist.append (callibrary.CalApplication(calto, calfrom))

        #return TsyscalResults(pool=calapplist)

        callist = []
        calto = callibrary.CalTo(vis=inputs.vis)
        calfrom = callibrary.CalFrom(gencal_args['caltable'], caltype='tsys',
            gainfield='nearest', spwmap=spwmap, interp='linear,linear')
        calapp = callibrary.CalApplication(calto, calfrom)
        callist.append(calapp)

        return TsyscalResults(pool=callist)


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
