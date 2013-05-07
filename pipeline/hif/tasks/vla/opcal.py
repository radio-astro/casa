from __future__ import absolute_import
import os
import types
import sys
from bisect import bisect_left
import numpy

import pipeline.infrastructure.api as api
import pipeline.heuristics as heuristics
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.logging as logging
from pipeline.infrastructure.jobrequest import casa_tasks
import pipeline.infrastructure.casatools as casatools
#from pipeline.extern.analysis_scripts.analysisUtils import plotweather

LOG = logging.get_logger(__name__)

def find_EVLA_band(frequency, bandlimits=[0.0e6, 150.0e6, 700.0e6, 2.0e9, 4.0e9, 8.0e9, 12.0e9, 18.0e9, 26.5e9, 40.0e9, 56.0e9], BBAND='?4PLSCXUKAQ?'):
    i = bisect_left(bandlimits, frequency)
    return BBAND[i]



def find_spw(vis):
    """Identify spw information
    """
    
    casatools.table.open(vis+'/SPECTRAL_WINDOW')
    channels = casatools.table.getcol('NUM_CHAN')
    originalBBClist = casatools.table.getcol('BBC_NO')
    spw_bandwidths=casatools.table.getcol('TOTAL_BANDWIDTH')
    reference_frequencies = casatools.table.getcol('REF_FREQUENCY')
    casatools.table.close()
    
    center_frequencies = map(lambda rf, spwbw: rf + spwbw/2,reference_frequencies, spw_bandwidths)
    
    bands = map(find_EVLA_band,center_frequencies)
    
    unique_bands = list(numpy.unique(bands))
    
    numSpws = len(channels)

    unique_bands_string = ','.join(unique_bands)
    
    all_spws_list = range(numSpws)
    
    all_spws = ','.join(map(str,all_spws_list))
    
    return all_spws


class OpcalResults(api.Results):
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
        s = 'OpcalResults:\n'
        for calapplication in self.final:
            s += '\tBest caltable for spw #{spw} in {vis} is {name}\n'.format(
                spw=calapplication.spw, vis=os.path.basename(calapplication.vis),
                name=calapplication.gaintable)
	return s


class OpcalInputs(basetask.StandardInputs):
    def __init__(self, context, output_dir=None, vis=None, caltable=None, caltype=None, parameter=None, tau=None, spw=None):
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
            value = heuristics.OpCaltable()
        self._caltable = value
    
    @property
    def tau(self):
        return self._tau

    @tau.setter
    def tau(self, value):
        if value is None:
            value = []
        self._tau = value
    
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
        
        #self.tau = casa_tasks.plotweather(vis=self.vis, seasonal_weight=1.0)
        
	return {'vis': self.vis,
	        'caltable': self.caltable,
		'caltype': self.caltype,
                'parameter': self.tau,
                'spw': self.spw}


class Opcal(basetask.StandardTaskTemplate):
    Inputs = OpcalInputs    

    def prepare(self):
        inputs = self.inputs
        
        plotweather_args = {'vis' : inputs.vis, 'seasonal_weight': 1.0}
        
        plotweather_job = casa_tasks.plotweather(**plotweather_args)
        output = self._executor.execute(plotweather_job)
 
        inputs.tau = output
        
        inputs.spw = find_spw(inputs.vis)

        gencal_args = inputs.to_casa_args()
        gencal_job = casa_tasks.gencal(**gencal_args)
        self._executor.execute(gencal_job)

        result = OpcalResults()
        LOG.warning('TODO: opspwmap heuristic re-reads measurement set!')
        LOG.warning("TODO: opspwmap heuristic won't handle missing file")
        #spwmap = heuristics.tsysspwmap(vis=inputs.vis,
	#    tsystable=gencal_args['caltable'])


        callist = []
        calto = callibrary.CalTo(vis=inputs.vis)
        calfrom = callibrary.CalFrom(gencal_args['caltable'], caltype='opac')
        calapp = callibrary.CalApplication(calto, calfrom)
        callist.append(calapp)

        return OpcalResults(pool=callist)


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