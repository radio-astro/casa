from __future__ import absolute_import
import os
import types

from pipeline.hif.heuristics import caltable as caltable_heuristic
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
from pipeline.infrastructure import casa_tasks
from . import resultobjects
import pipeline.infrastructure.casatools as casatools
from bisect import bisect_left
import numpy
from pipeline.hifv.tasks.vlautils import VLAUtils

LOG = infrastructure.get_logger(__name__)



def _find_spw(vis, bands, context):
    """Identify spw information
    """
    
    vlainputs = VLAUtils.Inputs(context)
    vlautils = VLAUtils(vlainputs)

    with casatools.TableReader(vis+'/SPECTRAL_WINDOW') as table:
        channels = table.getcol('NUM_CHAN')
        originalBBClist = table.getcol('BBC_NO')
        spw_bandwidths = table.getcol('TOTAL_BANDWIDTH')
        reference_frequencies = table.getcol('REF_FREQUENCY')
    
    center_frequencies = map(lambda rf, spwbw: rf + spwbw/2,reference_frequencies, spw_bandwidths)
    
    if (bands == []):
        bands = map(vlautils.find_EVLA_band,center_frequencies)
    
    unique_bands = list(numpy.unique(bands))
    
    numSpws = len(channels)

    unique_bands_string = ','.join(unique_bands)
    
    all_spws_list = range(numSpws)
    
    all_spws = ','.join(map(str,all_spws_list))
    
    return all_spws, center_frequencies


class OpcalInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
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
        
        context = self.inputs.context
        
        m = context.observing_run.measurement_sets[0]
        spw2band = context.evla['msinfo'][m.name].spw2band
        bands = spw2band.values()
        
        with casatools.MSReader(inputs.vis) as ms:
            ms_summary = ms.summary()
        
        startdate = ms_summary['BeginTime']
        
        if (((startdate >= 55918.80) and (startdate <= 55938.98)) or ((startdate >= 56253.6) and (startdate <= 56271.6))):
            LOG.info("Weather station broken during this period, using 100%")
            LOG.info("seasonal model for calculating the zenith opacity")
            seasonal_weight=1.0            
        else:
            LOG.info("Using seasonal_weight of 0.5")
            seasonal_weight=0.5
            
        plotweather_args = {'vis' : inputs.vis, 'seasonal_weight': seasonal_weight, 'doPlot' : True}
        plotweather_job = casa_tasks.plotweather(**plotweather_args)
        opacities = self._executor.execute(plotweather_job)
 
        inputs.parameter = opacities
        
        inputs.spw, center_frequencies = _find_spw(inputs.vis, bands, context)

        gencal_args = inputs.to_casa_args()
        gencal_job = casa_tasks.gencal(**gencal_args)
        self._executor.execute(gencal_job)


        callist = []
        calto = callibrary.CalTo(vis=inputs.vis)
        calfrom = callibrary.CalFrom(gencal_args['caltable'], caltype='opac', interp='', calwt=False)
        calapp = callibrary.CalApplication(calto, calfrom)
        callist.append(calapp)

        return resultobjects.OpcalResults(pool=callist, opacities=opacities, spw=inputs.spw, center_frequencies=center_frequencies, seasonal_weight=seasonal_weight)


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
