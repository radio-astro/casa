from __future__ import absolute_import
import os
import types

from pipeline.hif.heuristics import caltable as caltable_heuristic
from pipeline.hif.heuristics.tsysspwmap import tsysspwmap
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
from pipeline.infrastructure import casa_tasks
from . import resultobjects
import pipeline.infrastructure.casatools as casatools

import pipeline.infrastructure.casatools as casatools
import numpy

LOG = infrastructure.get_logger(__name__)



class SwpowcalInputs(basetask.StandardInputs):
    def __init__(self, context, output_dir=None, vis=None, caltable=None, caltype=None, parameter=[]):
	# set the properties to the values given as input arguments
        self._init_properties(vars())
	setattr(self, 'caltype', 'swpow')

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
            value = caltable_heuristic.SwpowCaltable()
        self._caltable = value
    
    @property
    def parameter(self):
        return self._parameter

    @parameter.setter
    def parameter(self, value):
        if value is None:
            value = []
        self._parameter = value
    
    @property
    def spw(self):
        return self._spw
    
    @spw.setter
    def spw(self, value):
        if value is None:
            value = ''
        self._spw = value

    # Avoids circular dependency on caltable.
    # NOT SURE WHY THIS IS NECCESARY.
    def _get_partial_task_args(self):
	return {'vis': self.vis, 'caltype': self.caltype}

    # Convert to CASA gencal task arguments.
    def to_casa_args(self):
        
	return {'vis': self.vis,
	        'caltable': self.caltable,
		    'caltype': self.caltype,
            'parameter': self.parameter}


class Swpowcal(basetask.StandardTaskTemplate):
    Inputs = SwpowcalInputs    

    def prepare(self):
        inputs = self.inputs

        with casatools.MSReader(inputs.vis) as ms:
            ms_summary = ms.summary()

        startdate = ms_summary['BeginTime']

        #Note from the original scripted pipeline:
        # Lastly, make switched power table.  This is not used in the
        # pipeline, but may be used for QA and for flagging, especially at
        # S-band for fields near the geostationary satellite belt.  Only
        # relevant for data taken on 24-Feb-2011 or later.
        if startdate >= 55616.6:
            gencal_args = inputs.to_casa_args()
            gencal_job = casa_tasks.gencal(**gencal_args)
            self._executor.execute(gencal_job)

            callist = []
            calto = callibrary.CalTo(vis=inputs.vis)
            calfrom = callibrary.CalFrom(gencal_args['caltable'], caltype='swpow', interp='', calwt=False)
            calapp = callibrary.CalApplication(calto, calfrom)
            callist.append(calapp)

        return resultobjects.SwpowcalResults(pool=callist)


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
