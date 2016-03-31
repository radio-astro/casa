from __future__ import absolute_import

import os
import types

from pipeline.hif.heuristics import caltable as caltable_heuristic

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
from . import jyperkreader

from pipeline.hifa.tasks.tsyscal import resultobjects

LOG = infrastructure.get_logger(__name__)

class SDK2JyCalInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, infiles=None, caltable=None,
                 reffile=None):
        vis=infiles
        # set the properties to the values given as input arguments
        self._init_properties(vars())

    @property
    def caltable(self):
        # The value of caltable is ms-dependent, so test for multiple
        # measurement sets and listify the results if necessary 
        if type(self.infiles) is types.ListType:
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
            value = caltable_heuristic.GaincalCaltable()
        self._caltable = value

    @property
    def caltype(self):
        return 'amp'

    @property
    def reffile(self):
        return self._reffile
    
    @reffile.setter
    def reffile(self, value):
        if value is None:
            value =  'jyperk.csv'
        self._reffile = value

    # Avoids circular dependency on caltable.
    def _get_partial_task_args(self):
        return {'vis'     : self.infiles,
                'caltype' : self.caltype}

    # Convert to CASA gencal task arguments.
    def to_casa_args(self):
        return {'vis'      : self.infiles,
                'caltable' : self.caltable,
                'caltype'  : self.caltype}


class SDK2JyCal(basetask.StandardTaskTemplate):
    Inputs = SDK2JyCalInputs    

    def prepare(self):
        inputs = self.inputs

        if self.inputs.reffile is None or not os.path.exists(self.inputs.reffile):
            LOG.error('No scaling factors available')
            return resultobjects.TsyscalResults(pool=[])
        # read scaling factor list
        #factors_list = read_scaling_factor(reffile)
        reffile = os.path.abspath(os.path.expandvars(os.path.expanduser(inputs.reffile)))
        factors_list = jyperkreader.read(inputs.context, reffile)
        LOG.debug('factors_list=%s'%(factors_list))
        
        # # generate scaling factor dictionary
        # factors = rearrange_factors_list(factors_list)                


        # # make a note of the current inputs state before we start fiddling
        # # with it. This origin will be attached to the final CalApplication.
        # origin = callibrary.CalAppOrigin(task=SDK2JyCal, 
        #                                  inputs=inputs.to_casa_args())

        # # construct the Tsys cal file
        # gencal_args = inputs.to_casa_args()
        # gencal_job = casa_tasks.gencal(**gencal_args)
        # self._executor.execute(gencal_job)

        callist = []
        # calto = callibrary.CalTo(vis=inputs.infiles)
        # calfrom = callibrary.CalFrom(gencal_args['caltable'], caltype='amp',
        #   gainfield='', spwmap=None, interp='nearest,nearest')
        # calapp = callibrary.CalApplication(calto, calfrom, origin)
        # callist.append(calapp)

        return resultobjects.TsyscalResults(pool=callist)

    def analyse(self, result):
        # # With no best caltable to find, our task is simply to set the one
        # # caltable as the best result

        # # double-check that the caltable was actually generated
        # on_disk = [ca for ca in result.pool
        #            if ca.exists() or self._executor._dry_run]
        # result.final[:] = on_disk

        # missing = [ca for ca in result.pool
        #            if ca not in on_disk and not self._executor._dry_run]
        # result.error.clear()
        # result.error.update(missing)

        return result
