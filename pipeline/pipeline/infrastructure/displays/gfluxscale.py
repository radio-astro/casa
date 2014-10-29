from __future__ import absolute_import
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
from . import setjy as setjy
LOG = infrastructure.get_logger(__name__)


# this can probably be replaced with the setjy version, letting the extra ant
# argument go through as kwargs
# class BasebandSummaryChart(setjy.PlotmsBasebandComposite):
#     """
#     Base class for executing plotms per baseband
#     """
#     def __init__(self, context, result, xaxis, yaxis, intent, ant, **kwargs):
#         LOG.info('%s vs %s plot' % (yaxis, xaxis))
# 
#         # request plots per spw, overlaying all antennas
#         super(BasebandSummaryChart, self).__init__(
#                 context, result, xaxis, yaxis, intent=intent, ant=ant, **kwargs)
                

class GFluxscaleSummaryChart(setjy.BasebandSummaryChart):
    """
    Create an amplitude vs UV distance plot for baseband.
    """
    def __init__(self, context, result, intent='', ydatacolumn='model', ant=''):
    
        #Reference fields
        fields = result.inputs['reference']
    
        #Get uvdist min/max  for horizontal axis
        uvrange = {}
        
        try:
            with casatools.MSReader(result.vis) as msfile:
                uvrange = msfile.range(['uvdist'])
        except:
            LOG.warn("Unable to obtain plotting ranges for gfluxscale uvdist.")
            uvrange["uvdist"] = numpy.array([0,0])
        
        #Get amp min/max   for vertical axis
        #Visstat call removed for 4.2.2 release
        
        '''
        try:
            visstat_args = {'vis'          : result.vis,
                            'axis'         : 'amp',
                            'datacolumn'   : ydatacolumn,
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
        '''
        pltmin = 0
        pltmax = 0
    
        plot_args = {'ydatacolumn' : ydatacolumn,
                     'field'       : fields,
                     'avgtime'     : '',
                     'avgscan'     : False,
                     'avgbaseline' : False,
                     'antenna'     : ant,
                     'plotrange'   : [uvrange['uvdist'][0], uvrange['uvdist'][1],pltmin,pltmax],
                     'coloraxis'   : 'spw',
                     'overwrite'   : True}
        
        super(GFluxscaleSummaryChart, self).__init__(
                context, result, xaxis='uvdist', yaxis='amp', intent=intent, ant=ant,
                **plot_args)

