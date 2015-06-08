'''
Created on 9 Sep 2014

@author: sjw
'''
import collections
import os

import numpy

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.displays.tsys as displays
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils

LOG = logging.get_logger(__name__)

TsysStat = collections.namedtuple('TsysScore', 'median rms median_max')
TsysMapTR = collections.namedtuple('TsysMapTR', 'vis tsys science')


class T2_4MDetailsTsyscalRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='tsyscal.mako', 
                 description='Calculate Tsys calibration',
                 always_rerender=False):
        super(T2_4MDetailsTsyscalRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, mako_context, pipeline_context, results):
        weblog_dir = os.path.join(pipeline_context.report_dir,
                                  'stage%s' % results.stage_number)

        summary_plots = {}
        subpages = {}
        for result in results:
            plotter = displays.TsysSummaryChart(pipeline_context, result)
            plots = plotter.plot()
            vis = os.path.basename(result.inputs['vis'])
            summary_plots[vis] = plots

            # generate per-antenna plots
            renderer = TsyscalPlotRenderer(pipeline_context, result)
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                # the filename is sanitised - the MS name is not. We need to
                # map MS to sanitised filename for link construction.
                subpages[vis] = renderer.path

        tsysmap = self._get_tsysmap_table_rows(pipeline_context, results)

        mako_context.update({'summary_plots'   : summary_plots,
                             'summary_subpage' : subpages,
                             'tsysmap'         : tsysmap,
                             'dirname'         : weblog_dir})

    def _get_tsysmap_table_rows(self, pipeline_context, results):
        rows = []

        for result in results:
            vis = os.path.basename(result.inputs['vis'])
            calto = result.final[0]
            
            ms = pipeline_context.observing_run.get_ms(vis)
            science_spws = ms.get_spectral_windows(science_windows_only=True)
            science_spw_ids = [spw.id for spw in science_spws]
            
            sci2tsys = dict((spw, tsys) for (spw, tsys) in enumerate(calto.spwmap)
                            if spw in science_spw_ids 
                            and spw not in result.unmappedspws)
    
            tsys2sci = collections.defaultdict(list)
            for sci, tsys in sci2tsys.items():
                tsys2sci[tsys].append(sci)
                
            tsysmap = dict((k, sorted(v)) for k, v in tsys2sci.items())            

            for tsys, sci in tsysmap.items():
                tr = TsysMapTR(vis, tsys, ', '.join([str(w) for w in sci]))
                rows.append(tr)

            if result.unmappedspws:
                tr = TsysMapTR(vis, 'Unmapped', ', '.join([str(w) for w in result.unmappedspws]))
                rows.append(tr)
                
        return utils.merge_td_columns(rows)


class TsyscalPlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result):
        vis = os.path.basename(result.inputs['vis'])
        title = 'T<sub>sys</sub> plots for %s' % vis
        outfile = filenamer.sanitize('tsys-%s.html' % vis)

        plotter = displays.TsysPerAntennaChart(context, result)
        plots = plotter.plot()
        
        self._caltable = result.final[0].gaintable
        self._spwmap = result.final[0].spwmap
        
        super(TsyscalPlotRenderer, self).__init__(
                'tsyscal_plots.mako', context, 
                result, plots, title, outfile)

    def update_json_dict(self, d, plot):
        antenna_name = plot.parameters['ant']
        tsys_spw_id = plot.parameters['tsys_spw'] 
        stat = self.get_stat(tsys_spw_id, antenna_name)            

        d.update({'tsys_spw'   : str(tsys_spw_id),
                  'median'     : stat.median,
                  'median_max' : stat.median_max,
                  'rms'        : stat.rms})                
            
    def get_stat(self, spw, antenna):
        tsys_spw = self._spwmap[spw]
        with casatools.CalAnalysis(self._caltable) as ca:
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
