from __future__ import absolute_import
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
from . import setjy as setjy
LOG = infrastructure.get_logger(__name__)


class GFluxscaleSummaryChart(setjy.BasebandSummaryChart):
    """
    Create an amplitude vs UV distance plot for baseband.
    """
    def __init__(self, context, output_dir, calto, intent='', ydatacolumn='model', ant='', uvrange='', **overrides):
        # Get uvdist min/max  for horizontal axis
        uvrangeplot = {}
        
        try:
            with casatools.MSReader(calto.vis) as msfile:
                # Numpy arrays must be converted to standard Python primitives, otherwise
                # the contained values cause problems with the underlying SWIG wrappers.
                uvrangeplot = msfile.range(['uvdist'])['uvdist'].tolist()
        except:
            LOG.warn("Unable to obtain plotting ranges for gfluxscale uvdist.")
            uvrangeplot = [0, 0]
        
        # Get amp min/max   for vertical axis
        # Visstat call removed for 4.2.2 release
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
        
        plot_args = {
            'ydatacolumn': ydatacolumn,
            'field': calto.field,
            'avgtime': '',
            'avgscan': False,
            'avgbaseline': False,
            'avgchannel': '9000',
            'antenna': ant,
            'uvrange': uvrange,  # Specified in hifa_gfluxscale task inputs
            'plotrange': [uvrangeplot[0], uvrangeplot[1], pltmin, pltmax],
            'coloraxis': 'spw',
            'overwrite': True
        }
        plot_args.update(**overrides)

        super(GFluxscaleSummaryChart, self).__init__(context, output_dir, calto, xaxis='uvdist', yaxis='amp',
                                                     intent=intent, ant=ant, **plot_args)
