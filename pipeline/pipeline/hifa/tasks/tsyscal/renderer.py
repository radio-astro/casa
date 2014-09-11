'''
Created on 9 Sep 2014

@author: sjw
'''
import contextlib
import os

import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.infrastructure.displays as displays

LOG = logging.get_logger(__name__)


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
            ms = os.path.basename(result.inputs['vis'])
            summary_plots[ms] = plots

            # generate the per-antenna charts and JSON file
            plotter = displays.ScoringTsysPerAntennaChart(pipeline_context, result)
            plots = plotter.plot() 
            json_path = plotter.json_filename

            # write the html for each MS to disk
            renderer = TsyscalPlotRenderer(pipeline_context, result, plots, json_path)
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                # the filename is sanitised - the MS name is not. We need to
                # map MS to sanitised filename for link construction.
                subpages[ms] = renderer.filename

        mako_context.update({'summary_plots'   : summary_plots,
                             'summary_subpage' : subpages,
                             'dirname'         : weblog_dir})


class TsyscalPlotRenderer(object):
    uri = 'tsyscal_plots.mako'

    def __init__(self, context, result, plots, json_path):
        self.context = context
        self.result = result
        self.plots = plots
        self.ms = os.path.basename(self.result.inputs['vis'])

        if os.path.exists(json_path):
            with open(json_path, 'r') as json_file:
                self.json = json_file.readlines()[0]
        else:
            self.json = '{}'
         
    def _get_display_context(self):
        return {'pcontext'   : self.context,
                'result'     : self.result,
                'plots'      : self.plots,
                'dirname'    : self.dirname,
                'json'       : self.json,
                'plot_title' : 'T<sub>sys</sub> plots for %s' % self.ms}

    @property
    def dirname(self):
        stage = 'stage%s' % self.result.stage_number
        return os.path.join(self.context.report_dir, stage)
    
    @property
    def filename(self):        
        filename = filenamer.sanitize('tsys-%s.html' % self.ms)
        return filename
    
    @property
    def path(self):
        return os.path.join(self.dirname, self.filename)
    
    def get_file(self):
        if not os.path.exists(self.dirname):
            os.makedirs(self.dirname)
            
        file_obj = open(self.path, 'w')
        return contextlib.closing(file_obj)
    
    def render(self):
        display_context = self._get_display_context()
        t = weblog.TEMPLATE_LOOKUP.get_template(self.uri)
        return t.render(**display_context)
