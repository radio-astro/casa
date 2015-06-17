from __future__ import absolute_import
import collections
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.renderer.logger as logger
from . import common
from pipeline.infrastructure import casa_tasks

LOG = infrastructure.get_logger(__name__)


class TsysSummaryChart(object):
    def __init__(self, context, result):
        calto = result.final[0]
        self._vis = calto.vis
        self._vis_basename = os.path.basename(self._vis)
        self._caltable = calto.gaintable

        # Create a spwmap containing just the science spws. Sometimes, not all
        # science spws have a matching Tsys window. As a result, spwmap may be
        # shorter than the index of the science spw we're trying to plot.
        ms = context.observing_run.get_ms(self._vis)
        science_spws = ms.get_spectral_windows(science_windows_only=True)
        science_spw_ids = [spw.id for spw in science_spws]
        
        wrapper = common.CaltableWrapperFactory.from_caltable(self._caltable)
        tsys_in_caltable = set(wrapper.spw) 
        self._spwmap = dict((spw, tsys) for (spw, tsys) in enumerate(calto.spwmap)
                            if spw in science_spw_ids 
                            and tsys in tsys_in_caltable)

        tsysmap = collections.defaultdict(list)
        for sci, tsys in self._spwmap.items():
            tsysmap[tsys].append(sci)
        self._tsysmap = dict((k, sorted(v)) for k, v in tsysmap.items())

        self._figroot = os.path.join(context.report_dir, 
                                     'stage%s' % result.stage_number, 
                                     'tsys-%s-summary.png' % self._vis_basename)

        # plotbandpass injects spw ID into every plot filename
        root, ext = os.path.splitext(self._figroot)
        self._real_figfiles = dict((tsys_spw, '%s.spw%0.2d%s' % (root, tsys_spw, ext))
                                   for tsys_spw in self._tsysmap)


    def create_task(self):
        unique_tsys_spws = set(self._spwmap.values())
        spw_arg = ','.join([str(spw) for spw in unique_tsys_spws])
        
        task_args = {'vis'         : self._vis,
                     'caltable'    : self._caltable,
                     'xaxis'       : 'freq',
                     'yaxis'       : 'tsys',
                     'overlay'     : 'antenna,time',
                     'interactive' : False,
                     'spw'         : spw_arg,
                     'showatm'     : True,
                     'showfdm'     : True,
                     'subplot'     : 11,
                     'figfile'     : self._figroot}

        return casa_tasks.plotbandpass(**task_args)

    def plot(self):
        wrappers = []

        task = self.create_task()
        for tsys_spw, science_spws in self._tsysmap.items():
            wrapper = logger.Plot(self._real_figfiles[tsys_spw],
                                  x_axis='freq',
                                  y_axis='tsys',
                                  parameters={'vis'      : self._vis_basename,
                                              'spw'      : science_spws,
                                              'tsys_spw' : tsys_spw},
                                  command=str(task))
            wrappers.append(wrapper)
        
        if not all([os.path.exists(w.abspath) for w in wrappers]):
            LOG.trace('Tsys summary plots not found. Creating new plots.')
            try:
                task.execute(dry_run=False)
            except Exception as ex:
                LOG.error('Could not create Tsys summary plots')
                LOG.exception(ex)
                return None

        for w in wrappers:
            if not os.path.exists(w.abspath):
                LOG.info('Tsys summary plot not generated for %s spw %s',
                         w.parameters['vis'], w.parameters['spw'])
            
        return wrappers


class TsysPerAntennaChart(common.PlotbandpassDetailBase):
    def __init__(self, context, result, **kwargs):
        super(TsysPerAntennaChart, self).__init__(context, result, 'freq',
                                                   'tsys', overlay='time',
                                                   showatm=True, showfdm=True,
                                                   **kwargs)

        # create a mapping of Tsys windows to science windows
        calto = result.final[0]
        ms = context.observing_run.get_ms(self._vis)
        science_spws = ms.get_spectral_windows(science_windows_only=True)
        science_spw_ids = [spw.id for spw in science_spws]

        wrapper = common.CaltableWrapperFactory.from_caltable(self._caltable)
        tsys_in_caltable = set(wrapper.spw) 

        spwmap = collections.defaultdict(list)
        for science_spw_id, tsys_spw_id in enumerate(calto.spwmap):
            if tsys_spw_id not in tsys_in_caltable:
                continue
            if science_spw_id in science_spw_ids:
                spwmap[tsys_spw_id].append(science_spw_id)                    
        self._tsys_map = dict((tsys_id, ','.join([str(i) for i in science_ids]))
                              for tsys_id, science_ids in spwmap.items())

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
        for tsys_spw_id in self._figfile.keys():
            # some science windows may not have a Tsys window
            science_spws =self._tsys_map.get(tsys_spw_id, 'N/A')
            for antenna_id, figfile in self._figfile[tsys_spw_id].items():
                ant_name = self._antmap[antenna_id]
                if os.path.exists(figfile):                    
                    task = self.create_task(tsys_spw_id, antenna_id)
                    wrapper = logger.Plot(figfile,
                                          x_axis=self._xaxis,
                                          y_axis=self._yaxis,
                                          parameters={'vis'      : self._vis_basename,
                                                      'ant'      : ant_name,
                                                      'spw'      : science_spws,
                                                      'tsys_spw' : tsys_spw_id},
                                          command=str(task))
                    wrappers.append(wrapper)
                else:
                    LOG.trace('No plotbandpass detail plot found for %s spw '
                              '%s antenna %s: %s not found', 
                              self._vis_basename, spw_id, ant_name, figfile)
        return wrappers


def get_chanrange(ms, tsys_spw):
    # CAS-7011: scale Tsys plots to show the inner 90% of channels
    num_tsys_channels = ms.get_spectral_window(tsys_spw).num_channels
    delta = int(round(0.05 * num_tsys_channels)) - 1
    chanrange = '%s~%s' % (delta, num_tsys_channels - delta)
    return chanrange
