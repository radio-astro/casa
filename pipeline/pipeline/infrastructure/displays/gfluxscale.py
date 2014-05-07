from __future__ import absolute_import
import collections
import os
import string
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.renderer.logger as logger
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.casatools as casatools


from pipeline.infrastructure import casa_tasks

from . import common

LOG = infrastructure.get_logger(__name__)


class GFluxscaleSummaryChart(object):
    def __init__(self, context, result, ms):
        self.context = context
        self.ms = ms
        self.result = result
        
    def plot(self):
    
        plots = [self.get_plot_wrapper(label='allant'), self.get_plot_wrapper(ant=self.result.resantenna, label='ant')]
        return [p for p in plots if p is not None]
    
    def create_plot(self, ant='', label='allant'):
        figfile = self.get_figfile(label=label)
        
        #Transfer fields
        #fields = ','.join([self.ms.get_fields(field)[0].name for field in self.result.measurements])
        # or self.result.inputs['transfer']
        
        #Reference fields
        fields = self.result.inputs['reference']
        
        #Get uvdist min/max  for horizontal axis
        uvrange = {}
        
        try:
            with casatools.MSReader(self.result.vis) as msfile:
                uvrange = msfile.range(["uvdist"])
        except:
            LOG.warn("Unable to obatin plotting ranges for gfluxscale uvdist.")
            uvrange["uvdist"] = numpy.array([0,0])
        
        #Get amp min/max   for vertical axis
        try:
            visstat_args = {'vis'          : self.ms.name,
                            'axis'         : 'amp',
                            'datacolumn'   : 'model',
                            'useflags'     : False,
                            'field'        : fields}
                        
            task = casa_tasks.visstat(**visstat_args)
            ampstats = task.execute(dry_run=False)
            ampmax = ampstats['MODEL']['max']
            ampmin = ampstats['MODEL']['min']
            amprange = ampmax - ampmin
            pltmin = ampmin - 0.05 * amprange
            pltmax = ampmax + 0.05 * amprange
        except:
            LOG.warn("Unable to obtain plotting ranges for gfluxscale amps.")
            pltmin = 0
            pltmax = 0
        
        
        task_args = {'vis'             : self.ms.name,
                     'xaxis'           : 'uvdist',
                     'yaxis'           : 'amp',
                     'ydatacolumn'     : 'model',
                     'field'           : fields,
                     'spw'             : '',
                     'coloraxis'       : 'spw',
                     'scan'            : '',
                     'antenna'         : ant,
                     'plotrange'       : [uvrange['uvdist'][0], uvrange['uvdist'][1],pltmin,pltmax],
                     'plotfile'        : figfile,
                     'showgui'         : False}
    
        task = casa_tasks.plotms(**task_args)
        task.execute(dry_run=False)
    
    
    def get_figfile(self, label='all'):
        filesuffix = label + '-gfluxscale.png'
        return os.path.join(self.context.report_dir, 
                            'stage%s' % self.result.stage_number, 
                            ('%s-' + filesuffix) % self.ms.basename)

    def get_plot_wrapper(self, ant='', label='allant'):
        figfile = self.get_figfile(label=label)
        
        wrapper = logger.Plot(figfile,
			    x_axis='uvdist',
			    y_axis='amp',
			    parameters={'vis'      : self.ms.basename,
					'ant'      : ant})
					
	if not os.path.exists(figfile):
            LOG.trace('Gfluxscale plot not found. Creating new plot')
            try:
                self.create_plot(ant=ant, label=label)
            except Exception as ex:
                LOG.error('Could not create gfluxscale plot')
                LOG.exception(ex)
                return None

        if os.path.exists(figfile):
            return wrapper            
        return None