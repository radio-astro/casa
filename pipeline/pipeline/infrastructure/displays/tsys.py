from __future__ import absolute_import
import collections
import json
import os

import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
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
        self._spwmap = dict((spw, tsys) for (spw, tsys) in enumerate(calto.spwmap)
                            if spw in science_spw_ids)

        self._figroot = os.path.join(context.report_dir, 
                                     'stage%s' % result.stage_number, 
                                     'tsys-%s-summary.png' % self._vis_basename)

        # plotbandpass injects spw ID into every plot filename
        root, ext = os.path.splitext(self._figroot)
        self._real_figfiles = dict((spw, '%s.spw%0.2d%s' % (root, tsys_spw, ext))
                                   for (spw, tsys_spw) in self._spwmap.items())

    def create_plot(self):
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

        task = casa_tasks.plotbandpass(**task_args)
        task.execute(dry_run=False)

    def plot(self):
        wrappers = []
        for science_spw, tsys_spw in self._spwmap.items():         
            wrapper = logger.Plot(self._real_figfiles[science_spw],
                                  x_axis='freq',
                                  y_axis='tsys',
                                  parameters={'vis'      : self._vis_basename,
                                              'spw'      : science_spw,
                                              'tsys_spw' : tsys_spw})
            wrappers.append(wrapper)
            
        if not all([os.path.exists(w.abspath) for w in wrappers]):
            LOG.trace('Tsys summary plots not found. Creating new plots.')
            try:
                self.create_plot()
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

        spwmap = collections.defaultdict(list)
        for science_spw_id, tsys_spw_id in enumerate(calto.spwmap):
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
                self.create_plot(spw_ids, ant_ids)
            except Exception as ex:
                LOG.error('Could not create plotbandpass details plots')
                LOG.exception(ex)
                return None

        wrappers = []
        for tsys_spw_id in self._figfile.keys():
            science_spws = self._tsys_map[tsys_spw_id]
            for antenna_id, figfile in self._figfile[tsys_spw_id].items():
                ant_name = self._antmap[antenna_id]
                if os.path.exists(figfile):                    
                    wrapper = logger.Plot(figfile,
                                          x_axis=self._xaxis,
                                          y_axis=self._yaxis,
                                          parameters={'vis'      : self._vis_basename,
                                                      'ant'      : ant_name,
                                                      'spw'      : science_spws,
                                                      'tsys_spw' : tsys_spw_id})
                    wrappers.append(wrapper)
                else:
                    LOG.trace('No plotbandpass detail plot found for %s spw '
                              '%s antenna %s: %s not found', 
                              self._vis_basename, spw_id, ant_name, figfile)
        return wrappers


TsysStat = collections.namedtuple('TsysScore', 'median rms median_max')

class ScoringTsysPerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.plotter = TsysPerAntennaChart(context, result)
        ms = os.path.basename(result.inputs['vis'])
        
        self.json = {}
        self.json_filename = os.path.join(context.report_dir, 
                                          'stage%s' % result.stage_number, 
                                          'tsys-%s.json' % ms)
        
    def plot(self):
        plots = self.plotter.plot()

        # calculate scores and write them to a JSON file if not done already
        if not os.path.exists(self.json_filename):
            scores = self.get_scores(plots)
            with open(self.json_filename, 'w') as f:
                LOG.trace('Writing Tsys JSON data to %s' % self.json_filename)
                json.dump(scores, f)
            
        return plots
    
    def get_scores(self, plots):
        d = {}
        for plot in plots:
            antenna_name = plot.parameters['ant']
            tsys_spw_id = plot.parameters['tsys_spw'] 
            science_spw_ids = plot.parameters['spw'] 
            stat = self.get_stat(tsys_spw_id, antenna_name)            

            # calculate the relative pathnames as seen from the browser
            thumbnail_relpath = os.path.relpath(plot.thumbnail,
                                                self.context.report_dir)
            image_relpath = os.path.relpath(plot.abspath,
                                            self.context.report_dir)

            # all values set on this dictionary will be written to the JSON file
            d[image_relpath] = {'antenna'    : antenna_name,
                                'tsys_spw'   : str(tsys_spw_id),
                                'spw'        : science_spw_ids,
                                'median'     : stat.median,
                                'median_max' : stat.median_max,
                                'rms'        : stat.rms,
                                'thumbnail'  : thumbnail_relpath}
        return d
            
    def get_stat(self, spw, antenna):    
        caltable = self.result.final[0].gaintable
        tsys_spw = self.result.final[0].spwmap[spw]
        
        with casatools.CalAnalysis(caltable) as ca:
            args = {'spw'     : tsys_spw,
                    'antenna' : antenna,
                    'axis'    : 'TIME',
                    'ap'      : 'AMPLITUDE'}
    
            LOG.trace('Retrieving caltable data for %s spw %s'
                      '' % (antenna, spw))
            ca_result = ca.get(**args)
            return self.get_stat_from_calanalysis(ca_result)

    def get_stat_from_calanalysis(self, ca_result):
        '''
        Calculate the median and RMS for a calanalysis result. The argument
        supplied to this function should be a calanalysis result for ONE
        spectral window and ONE antenna only!
        ''' 
        # get the unique timestamps from the calanalysis result
        times = set([v['time'] for v in ca_result.values()])
        mean_tsyses = []
        for timestamp in sorted(times):
            # get the dictionary for each timestamp, giving one dictionary per
            # feed
            vals = [v for v in ca_result.values() if v['time'] is timestamp]                        
            # get the median Tsys for each feed at this timestamp 
            medians = [numpy.median(v['value']) for v in vals]
            # use the average of the medians per antenna feed as the typical
            # tsys for this antenna at this timestamp
            mean_tsyses.append(numpy.mean(medians))
    
        median = numpy.median(mean_tsyses)
        rms = numpy.std(mean_tsyses)
        median_max = numpy.max(mean_tsyses)

        return TsysStat(median, rms, median_max)


def get_chanrange(ms, tsys_spw):
    # CAS-7011: scale Tsys plots to show the inner 90% of channels
    num_tsys_channels = ms.get_spectral_window(tsys_spw).num_channels
    delta = int(round(0.05 * num_tsys_channels)) - 1
    chanrange = '%s~%s' % (delta, num_tsys_channels - delta)
    return chanrange
