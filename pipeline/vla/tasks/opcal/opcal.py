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
from bisect import bisect_left
import numpy

LOG = infrastructure.get_logger(__name__)


def _find_EVLA_band(frequency, bandlimits=[0.0e6, 150.0e6, 700.0e6, 2.0e9, 4.0e9, 8.0e9, 12.0e9, 18.0e9, 26.5e9, 40.0e9, 56.0e9], BBAND='?4PLSCXUKAQ?'):
    i = bisect_left(bandlimits, frequency)
    return BBAND[i]



def _find_spw(vis):
    """Identify spw information
    """

    with casatools.TableReader(vis+'/SPECTRAL_WINDOW') as table:
        channels = table.getcol('NUM_CHAN')
        originalBBClist = table.getcol('BBC_NO')
        spw_bandwidths = table.getcol('TOTAL_BANDWIDTH')
        reference_frequencies = table.getcol('REF_FREQUENCY')
    
    center_frequencies = map(lambda rf, spwbw: rf + spwbw/2,reference_frequencies, spw_bandwidths)
    
    bands = map(_find_EVLA_band,center_frequencies)
    
    unique_bands = list(numpy.unique(bands))
    
    numSpws = len(channels)

    unique_bands_string = ','.join(unique_bands)
    
    all_spws_list = range(numSpws)
    
    all_spws = ','.join(map(str,all_spws_list))
    
    return all_spws


class OpcalInputs(basetask.StandardInputs):
    def __init__(self, context, output_dir=None, vis=None, caltable=None, caltype=None, parameter=None, spw=None):
	# set the properties to the values given as input arguments
        self._init_properties(vars())
	setattr(self, 'caltype', 'opac')

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
            value = caltable_heuristic.OpCaltable()
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
            'parameter': self.parameter,
            'spw': self.spw}


class Opcal(basetask.StandardTaskTemplate):
    Inputs = OpcalInputs    

    def prepare(self):
        inputs = self.inputs
        
        plotweather_args = {'vis' : inputs.vis, 'seasonal_weight': 1.0}
        
        plotweather_job = casa_tasks.plotweather(**plotweather_args)
        output = self._executor.execute(plotweather_job)
 
        inputs.parameter = output
        
        inputs.spw = _find_spw(inputs.vis)

        gencal_args = inputs.to_casa_args()
        gencal_job = casa_tasks.gencal(**gencal_args)
        self._executor.execute(gencal_job)


        callist = []
        calto = callibrary.CalTo(vis=inputs.vis)
        calfrom = callibrary.CalFrom(gencal_args['caltable'], caltype='opac', interp='', calwt=False)
        calapp = callibrary.CalApplication(calto, calfrom)
        callist.append(calapp)

        return resultobjects.OpcalResults(pool=callist)


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
