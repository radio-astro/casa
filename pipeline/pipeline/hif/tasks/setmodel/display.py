from __future__ import absolute_import

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.utils as utils
#from . import applycal
from pipeline.h.tasks.common.displays import applycal as applycal

LOG = infrastructure.get_logger(__name__)

# TODO
# create an adapter object so that Setjy and calibration tasks can use the
# same PlotmsLeaf and composite objects

class BasebandComposite(applycal.BasebandComposite):
    """
    Create a PlotLeaf for each baseband in the applied calibration.
    """
    def __init__(self, context, output_dir, calto, xaxis, yaxis, ant='', field='', intent='', uvrange='', **kwargs):
        super(BasebandComposite, self).__init__(context, output_dir, calto, xaxis, yaxis, ant=ant, field=field,
                                                intent=intent, uvrange=uvrange, **kwargs)
        

class PlotmsBasebandComposite(BasebandComposite):
    leaf_class = applycal.PlotmsLeaf

    def plot(self):
        # merge separate spw jobs into one job using plotms iterator
        jobs_and_wrappers = super(PlotmsBasebandComposite, self).plot()
        successful_wrappers = utils.plotms_iterate(jobs_and_wrappers)
        return successful_wrappers
        

class BasebandSummaryChart(PlotmsBasebandComposite):
    """
    Base class for executing plotms per baseband
    """
    def __init__(self, context, output_dir, calto, xaxis, yaxis, intent, **kwargs):
        LOG.info('%s vs %s plot' % (yaxis, xaxis))

        # request plots per spw, overlaying all antennas
        super(BasebandSummaryChart, self).__init__(context, output_dir, calto, xaxis, yaxis, intent=intent, **kwargs)


class AmpVsUVSummaryChart(BasebandSummaryChart):
    """
    Create an amplitude vs UV distance plot for each baseband.
    """
    def __init__(self, context, output_dir, calto, intent='', ydatacolumn='model', **overrides):
        plot_args = {
            'ydatacolumn': ydatacolumn,
            'avgtime': '',
            'avgscan': False,
            'avgbaseline': False,
            'avgchannel': '9000',
            'plotrange': [0, 0, 0, 0],
            'coloraxis': 'spw',
            'overwrite': True
        }
        plot_args.update(**overrides)

        super(AmpVsUVSummaryChart, self).__init__(context, output_dir, calto, xaxis='uvdist', yaxis='amp',
                                                  intent=intent, **plot_args)
