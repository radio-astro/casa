from __future__ import absolute_import
import collections
import json
import os

import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger
from pipeline.infrastructure import casa_tasks

LOG = infrastructure.get_logger(__name__)


class TsysSummaryChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self.caltable = result.final[0].gaintable

    def plot(self):
        science_spws = self.ms.get_spectral_windows(science_windows_only=True)
        plots = [self.get_plot_wrapper(spw) for spw in science_spws]
        return [p for p in plots if p is not None]

    def create_plot(self, tsys_spw):
        figfile = self.get_figfile()

        task_args = {'vis'         : self.ms.name,
                     'caltable'    : self.caltable,
                     'xaxis'       : 'freq',
                     'yaxis'       : 'tsys',
                     'overlay'     : 'antenna,time',
                     'interactive' : False,
                     'spw'         : tsys_spw,
                     'showatm'     : True,
                     'showfdm'     : True,
                     'subplot'     : 11,
                     'figfile'     : figfile}

        task = casa_tasks.plotbandpass(**task_args)
        task.execute(dry_run=False)

    def get_figfile(self):
        return os.path.join(self.context.report_dir, 
                            'stage%s' % self.result.stage_number, 
                            'tsys-%s-summary.png' % self.ms.basename)

    def get_plot_wrapper(self, spw):
        try:
            # Get the Tsys spw for this science spw. Not all science spws
            # have a matching Tsys window. As a result, spwmap may be shorter
            # than the index of the science spw we're trying to plot,
            # raising an IndexError.
            tsys_spw = self.result.final[0].spwmap[spw.id]
        except IndexError:
            return None

        figfile = self.get_figfile()

        # plotbandpass injects spw ID and t00 into every plot filename
        root, ext = os.path.splitext(figfile)
        real_figfile = '%s.spw%0.2d.t00%s' % (root, tsys_spw, ext)
        
        wrapper = logger.Plot(real_figfile,
                              x_axis='freq',
                              y_axis='tsys',
                              parameters={'vis'      : self.ms.basename,
                                          'spw'      : spw.id,
                                          'tsys_spw' : tsys_spw})

        if not os.path.exists(real_figfile):
            LOG.trace('Tsys summary plot for spw %s not found. Creating new '
                      'plot.' % spw.id)
            try:
                self.create_plot(tsys_spw)
            except Exception as ex:
                LOG.error('Could not create Tsys summary plot for spw %s'
                          '' % spw.id)
                LOG.exception(ex)
                return None
            
        return wrapper
    
    
class TsysPerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self.caltable = result.final[0].gaintable

    def plot(self):
        science_spws = self.ms.get_spectral_windows(science_windows_only=True)
        plots = []
        for spw in science_spws:
            for antenna in self.ms.antennas:
                plots.append(self.get_plot_wrapper(spw, antenna))

        return [p for p in plots if p is not None]

    def create_plot(self, tsys_spw, antenna):
        figfile = self.get_figfile()

        task_args = {'vis'         : self.ms.name,
                     'caltable'    : self.caltable,
                     'xaxis'       : 'freq',
                     'yaxis'       : 'tsys',
                     'overlay'     : 'time',
                     'interactive' : False,
                     'spw'         : tsys_spw,
                     'antenna'     : antenna.id,
                     'showatm'     : True,
                     'showfdm'     : True,
                     'subplot'     : 11,
                     'figfile'     : figfile}

        task = casa_tasks.plotbandpass(**task_args)
        task.execute(dry_run=False)

    def get_figfile(self):
        return os.path.join(self.context.report_dir, 
                            'stage%s' % self.result.stage_number, 
                            'tsys-%s.png' % self.ms.basename)

    def get_plot_wrapper(self, spw, antenna):
        # get the Tsys spw for this science spw
        try:
            # Get the Tsys spw for this science spw. Not all science spws
            # have a matching Tsys window. As a result, spwmap may be shorter
            # than the index of the science spw we're trying to plot,
            # raising an IndexError.
            tsys_spw = self.result.final[0].spwmap[spw.id]
        except IndexError:
            return None

        figfile = self.get_figfile()

        # plotbandpass injects antenna name and spw ID into every plot filename
        root, ext = os.path.splitext(figfile)
        real_figfile = '%s.%s.spw%0.2d%s' % (root, antenna.name, tsys_spw, ext)
        
        wrapper = logger.Plot(real_figfile,
                              x_axis='freq',
                              y_axis='tsys',
                              parameters={'vis' : self.ms.basename,
                                          'spw' : spw.id,
                                          'ant' : antenna.name})
        
        if not os.path.exists(real_figfile):
            LOG.trace('Tsys per antenna plot for antenna %s spw %s not found. '
                      'Creating new plot.' % (antenna.name, spw.id))
            try:
                self.create_plot(tsys_spw, antenna)
            except Exception as ex:
                LOG.error('Could not create Tsys plot for antenna %s spw %s '
                          '' % (antenna.id, spw.id))
                LOG.exception(ex)
                return None

        # the Tsys plot may not be created if all data for that antenna are
        # flagged
        if os.path.exists(real_figfile):
            return wrapper            
        return None


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
            spw_id = plot.parameters['spw'] 
            stat = self.get_stat(spw_id, antenna_name)            

            # CAS-7009: weblog tsys histogram filtering fails for multi-spw/bb
            #
            # A single Tsys window could map to multiple science windows. For
            # these windows, only one plot will be generated: that for the
            # corresponding Tsys window. The JSON dictionary uses the plot
            # filename as the key, resulting in science windows that map to a
            # single Tsys window only having one entry in the JSON file (the
            # plot is actually added to the dictionary multiple times, but
            # each addition overwrites that for the previous spectral window).
            # 
            # To work around this, we create a symbolic link to the Tsys plot,
            # inserting the science spw into the link filename. We then
            # calculate the score based on the link, giving separate entries
            # in the JSON dictionary.        
            (root, ext) = os.path.splitext(plot.abspath)
            link_abspath = '%s.scispw%s%s' % (root, spw_id, ext)
            if not os.path.exists(link_abspath):
                os.link(plot.abspath, link_abspath)
            plot.abspath = link_abspath

            # calculate the relative pathnames as seen from the browser
            thumbnail_relpath = os.path.relpath(plot.thumbnail,
                                                self.context.report_dir)
            image_relpath = os.path.relpath(plot.abspath,
                                            self.context.report_dir)

            # all values set on this dictionary will be written to the JSON file
            d[image_relpath] = {'antenna'    : antenna_name,
                                'spw'        : spw_id,
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
