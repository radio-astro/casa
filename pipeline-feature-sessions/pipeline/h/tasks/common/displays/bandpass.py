from __future__ import absolute_import
import itertools
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.renderer.logger as logger

from . import applycal
from . import common

LOG = infrastructure.get_logger(__name__)


class NullScoreFinder(object):
    def get_score(self, spw, antenna):
        return None


class BandpassDetailChart(common.PlotbandpassDetailBase):
    def __init__(self, context, result, xaxis, yaxis, **kwargs):
        super(BandpassDetailChart, self).__init__(context, result, xaxis, yaxis, **kwargs)

    def plot(self):
        missing = [(spw_id, ant_id)
                   for spw_id in self._figfile.keys()
                   for ant_id in self._antmap.keys()
                   if not os.path.exists(self._figfile[spw_id][ant_id])]
        if missing:
            LOG.trace('Executing new plotbandpass job for missing figures')
            spw_ids = ','.join(set([str(spw_id) for spw_id, _ in missing]))
            ant_ids = ','.join(set([str(ant_id) for _, ant_id in missing]))
            try:
                task = self.create_task(spw_ids, ant_ids)
                task.execute(dry_run=False)
            except Exception as ex:
                LOG.error('Could not create plotbandpass details plots')
                LOG.exception(ex)
                return None

        wrappers = []
        for spw_id in self._figfile.keys():
            for antenna_id, figfile in self._figfile[spw_id].items():
                ant_name = self._antmap[antenna_id]
                if os.path.exists(figfile):
                    task = self.create_task(spw_id, antenna_id)
                    wrapper = logger.Plot(figfile,
                                          x_axis=self._xaxis,
                                          y_axis=self._yaxis,
                                          parameters={'vis' : self._vis_basename,
                                                      'ant' : ant_name,
                                                      'spw' : spw_id},
                                          command=str(task))
                    wrappers.append(wrapper)
                else:
                    LOG.trace('No plotbandpass detail plot found for %s spw '
                              '%s antenna %s: %s not found',
                              self._vis_basename, spw_id, ant_name, figfile)
        return wrappers


class BandpassSummaryChart(common.PlotbandpassDetailBase):
    def __init__(self, context, result, xaxis, yaxis, **kwargs):
        super(BandpassSummaryChart, self).__init__(context, result, xaxis,
                                                  yaxis, overlay='baseband',
                                                  **kwargs)

        # overlaying baseband, so we need to merge the individual spw keys
        # into joint keys, and the filenames into a list as the output could
        # be any of the filenames, depending on whether spws were flagged
        spw_ids = [spw_id for spw_id in self._figfile]
        ant_ids = [ant_ids.keys() for _, ant_ids in self._figfile.items()]
        ant_ids = set(itertools.chain(*ant_ids))

        d = dict((ant_id,
                  [self._figfile[spw_id][ant_id] for spw_id in spw_ids])
                 for ant_id in ant_ids)
        self._figfile = d

    def plot(self):
        missing = [ant_id
                   for ant_id in self._antmap.keys()
                   if not any([os.path.exists(f) for f in self._figfile[ant_id]])]
        if missing:
            LOG.trace('Executing new plotbandpass job for missing figures')
            ant_ids = ','.join([str(ant_id) for ant_id in missing])
            try:
                task = self.create_task('', ant_ids)
                task.execute(dry_run=False)
            except Exception as ex:
                LOG.error('Could not create plotbandpass summary plots')
                LOG.exception(ex)
                return None

        wrappers = []
        for antenna_id, figfiles in self._figfile.items():
            for figfile in figfiles:
                if os.path.exists(figfile):
                    task = self.create_task('', antenna_id)
                    wrapper = logger.Plot(figfile,
                                          x_axis=self._xaxis,
                                          y_axis=self._yaxis,
                                          parameters={'vis' : self._vis_basename,
                                                      'ant' : self._antmap[antenna_id]},
                                          command=str(task))
                    wrappers.append(wrapper)
                    break
            else:
                LOG.trace('No plotbandpass summary plot found for antenna '
                          '%s' % self._antmap[antenna_id])
        return wrappers


class BandpassAmpVsFreqSummaryChart(BandpassSummaryChart):
    """
    Create an amp vs freq plot for each antenna
    """
    def __init__(self, context, result):
        # request plots per spw, overlaying all antennas
        super(BandpassAmpVsFreqSummaryChart, self).__init__(context, result, xaxis='freq', yaxis='amp', showatm=True)


class BandpassPhaseVsFreqSummaryChart(BandpassSummaryChart):
    """
    Create an phase vs freq plot for each antenna
    """
    def __init__(self, context, result):
        # request plots per spw, overlaying all antennas
        super(BandpassPhaseVsFreqSummaryChart, self).__init__(context, result, xaxis='freq', yaxis='phase',
                                                              showatm=True, markersize=6)


class BandpassAmpVsFreqDetailChart(BandpassDetailChart):
    """
    Create an amp vs freq plot for each spw/antenna combination.
    """
    def __init__(self, context, result):
        # request plots per antenna and spw
        super(BandpassAmpVsFreqDetailChart, self).__init__(context, result, xaxis='freq', yaxis='amp', showatm=True)


class BandpassPhaseVsFreqDetailChart(BandpassDetailChart):
    """
    Create an amp vs freq plot for each spw/antenna combination.
    """
    def __init__(self, context, result):
        # request plots per antenna and spw
        super(BandpassPhaseVsFreqDetailChart, self).__init__(context, result, xaxis='freq', yaxis='phase', showatm=True,
                                                             markersize=6)


class BandpassAmpVsUVDetailChart(applycal.SpwSummaryChart):
    def __init__(self, context, output_dir, calto, intent='', ydatacolumn='corrected', **overrides):
        plot_args = {
            'ydatacolumn': ydatacolumn,
            'avgtime': '',
            'avgscan': False,
            'avgbaseline': False,
            'avgchannel': '9000',
            'clearplots': True,
            'coloraxis': 'corr',
            'overwrite': True,
            'plotrange': [0, 0, 0, 0],
            'showgui': False,
        }
        plot_args.update(**overrides)

        super(BandpassAmpVsUVDetailChart, self).__init__(context, output_dir, calto, xaxis='uvdist', yaxis='amp',
                                                         intent=intent, **plot_args)


class BandpassAmpVsTimeDetailChart(applycal.SpwSummaryChart):
    def __init__(self, context, output_dir, calto, intent='', ydatacolumn='corrected', **overrides):
        plot_args = {
            'ydatacolumn': ydatacolumn,
            'avgtime': '',
            'avgscan': False,
            'avgbaseline': False,
            'avgchannel': '9000',
            'clearplots': True,
            'coloraxis': 'corr',
            'overwrite': True,
            'plotrange': [0, 0, 0, 0],
            'showgui': False,
        }
        plot_args.update(**overrides)

        super(BandpassAmpVsUVDetailChart, self).__init__(context, output_dir, calto, xaxis='uvdist', yaxis='amp',
                                                         intent=intent, **plot_args)
