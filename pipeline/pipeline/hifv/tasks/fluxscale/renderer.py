import collections
import os
import contextlib
import pipeline.infrastructure.renderer.weblog as weblog

import pipeline.infrastructure.displays.vla.testgainsdisplay as testgainsdisplay
import pipeline.infrastructure.displays.vla.fluxbootdisplay as fluxbootdisplay
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.filenamer as filenamer

LOG = logging.get_logger(__name__)


class VLASubPlotRenderer(object):
    #template = 'testdelays_plots.html'
    
    def __init__(self, context, result, plots, json_path, template, filename_prefix):
        self.context = context
        self.result = result
        self.plots = plots
        self.ms = os.path.basename(self.result.inputs['vis'])
        self.template = template
        self.filename_prefix=filename_prefix

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
                'json'       : self.json}

    @property
    def dirname(self):
        stage = 'stage%s' % self.result.stage_number
        return os.path.join(self.context.report_dir, stage)
    
    @property
    def filename(self):        
        filename = filenamer.sanitize(self.filename_prefix + '-%s.html' % self.ms)
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
        t = weblog.TEMPLATE_LOOKUP.get_template(self.template)
        return t.render(**display_context)






class T2_4MDetailsSolintRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self,uri='solint.mako', description='Determine solint and Test gain calibrations', 
                 always_rerender=True):
        super(T2_4MDetailsSolintRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)
    
    def get_display_context(self, context, results):
        super_cls = super(T2_4MDetailsSolintRenderer, self)
        ctx = super_cls.get_display_context(context, results)
        
        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % results.stage_number)
        
        summary_plots = {}
        testgainsamp_subpages = {}
        testgainsphase_subpages = {}
        
        longsolint = {}
        gain_solint2 = {}
        
        shortsol2 = {}
        short_solint = {}
        new_gain_solint1 = {}
        
        
        for result in results:
            
            plotter = testgainsdisplay.testgainsSummaryChart(context, result)
            plots = plotter.plot()
            ms = os.path.basename(result.inputs['vis'])
            summary_plots[ms] = plots
            
            # generate testdelay plots and JSON file
            plotter = testgainsdisplay.testgainsPerAntennaChart(context, result, 'amp')
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'testgains_plots.html', 'amp')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                testgainsamp_subpages[ms] = renderer.filename
            
            # generate amp Gain plots and JSON file
            plotter = testgainsdisplay.testgainsPerAntennaChart(context, result, 'phase')
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'testgains_plots.html', 'phase')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                testgainsphase_subpages[ms] = renderer.filename
           
            #String type
            new_gain_solint1[ms] = result.new_gain_solint1
            longsolint[ms]       = result.longsolint
            

        ctx.update({'summary_plots'   : summary_plots,
                    'testgainsamp_subpages' : testgainsamp_subpages,
                    'testgainsphase_subpages'   : testgainsphase_subpages,
                    'new_gain_solint1'           : new_gain_solint1,
                    'longsolint'                : longsolint,
                    'dirname'         : weblog_dir})
                
        return ctx
        

class T2_4MDetailsfluxbootRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self,uri='fluxboot.mako', description='Determine solint and Test gain calibrations', 
                 always_rerender=True):
        super(T2_4MDetailsfluxbootRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)
    
    def get_display_context(self, context, results):
        super_cls = super(T2_4MDetailsfluxbootRenderer, self)
        ctx = super_cls.get_display_context(context, results)
        
        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % results.stage_number)
        
        summary_plots = {}
        weblog_results = {}
        spindex_results = {}

        for result in results:
            
            plotter = fluxbootdisplay.fluxbootSummaryChart(context, result)
            plots = plotter.plot()
            ms = os.path.basename(result.inputs['vis'])
            summary_plots[ms] = plots
            weblog_results[ms] = result.weblog_results
            spindex_results[ms] = result.spindex_results
            
        ctx.update({'summary_plots'   : summary_plots,
                    'weblog_results'  : weblog_results,
                    'spindex_results' : spindex_results,
                    'dirname'         : weblog_dir})
                
        return ctx