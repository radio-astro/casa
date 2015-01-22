from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.utils as utils
from . import applycal

LOG = infrastructure.get_logger(__name__)

# TODO
# create an adapter object so that Setjy and calibration tasks can use the
# same PlotmsLeaf and composite objects

class BasebandComposite(applycal.BasebandComposite):
    """
    Create a PlotLeaf for each baseband in the applied calibration.
    """
    def __init__(self, context, result, xaxis, yaxis, ant='', field='',
                 intent='', **kwargs):
        spws=[]
        for fieldname in result.measurements.keys():
            for fluxm in result.measurements[fieldname]:
                spws.append(fluxm.spw_id)
        
        spwlist = ','.join([str(i) for i in spws])

        # create a fake CalTo object so we can use the applycal class
        calto = callibrary.CalTo(result.inputs['vis'], spw=spwlist)

        super(BasebandComposite, self).__init__(context, result, calto, xaxis, 
                yaxis, ant=ant, field=field, intent=intent, **kwargs)
        

class PlotmsBasebandComposite(BasebandComposite):
    leaf_class = applycal.PlotmsLeaf

    def plot(self):
        # merge separate spw jobs into one job using plotms iterator
        jobs_and_wrappers = super(PlotmsBasebandComposite, self).plot()        
        successful_wrappers = utils.plotms_iterate(jobs_and_wrappers, 'spw')
        return successful_wrappers
        

class BasebandSummaryChart(PlotmsBasebandComposite):
    """
    Base class for executing plotms per baseband
    """
    def __init__(self, context, result, xaxis, yaxis, intent, **kwargs):
        LOG.info('%s vs %s plot' % (yaxis, xaxis))

        # request plots per spw, overlaying all antennas
        super(BasebandSummaryChart, self).__init__(
                context, result, xaxis, yaxis, intent=intent, **kwargs)
                

class AmpVsUVSummaryChart(BasebandSummaryChart):
    """
    Create an amplitude vs UV distance plot for each baseband.
    """
    def __init__(self, context, result, intent='', ydatacolumn='model'):
        plot_args = {'ydatacolumn' : ydatacolumn,
                     'avgtime'     : '',
                     'avgscan'     : False,
                     'avgbaseline' : False,
                     'plotrange'   : [0, 0, 0, 0],
                     'coloraxis'   : 'spw',
                     'overwrite'   : True}
        
        super(AmpVsUVSummaryChart, self).__init__(
                context, result, xaxis='uvdist', yaxis='amp', intent=intent, 
                **plot_args)
