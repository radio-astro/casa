import collections
import os
import contextlib
import pipeline.infrastructure.renderer.weblog as weblog

import pipeline.infrastructure.displays.vla.semifinalBPdcalsdisplay as semifinalBPdcalsdisplay
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.filenamer as filenamer

#import pipeline.infrastructure.renderer.htmlrenderer as hr

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


class T2_4MDetailssemifinalBPdcalsRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self,uri='semifinalbpdcals.mako', description='Semi-final delay and bandpass calibrations', 
                 always_rerender=False):
        super(T2_4MDetailssemifinalBPdcalsRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)
    
    def get_display_context(self, context, results):
        super_cls = super(T2_4MDetailssemifinalBPdcalsRenderer, self)
        ctx = super_cls.get_display_context(context, results)
        
        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % results.stage_number)
        
        summary_plots = {}
        delay_subpages = {}
        ampgain_subpages = {}
        phasegain_subpages = {}
        bpsolamp_subpages = {}
        bpsolphase_subpages = {}
        
        suffix=''
        
        for result in results:
            
            plotter = semifinalBPdcalsdisplay.semifinalBPdcalsSummaryChart(context, result, suffix=suffix)
            plots = plotter.plot()
            ms = os.path.basename(result.inputs['vis'])
            summary_plots[ms] = plots
            
            # generate testdelay plots and JSON file
            plotter = semifinalBPdcalsdisplay.DelaysPerAntennaChart(context, result, suffix=suffix)
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'semifinalcals_plots.html', 'delays')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                delay_subpages[ms] = renderer.filename
            
                
            # generate phase Gain plots and JSON file
            plotter = semifinalBPdcalsdisplay.semifinalphaseGainPerAntennaChart(context, result, suffix=suffix)
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'semifinalcals_plots.html', 'phasegain')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                phasegain_subpages[ms] = renderer.filename
                
            # generate amp bandpass solution plots and JSON file
            plotter = semifinalBPdcalsdisplay.semifinalbpSolAmpPerAntennaChart(context, result, suffix=suffix)
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'semifinalcals_plots.html', 'bpsolamp')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                bpsolamp_subpages[ms] = renderer.filename
                
            # generate phase bandpass solution plots and JSON file
            plotter = semifinalBPdcalsdisplay.semifinalbpSolPhasePerAntennaChart(context, result, suffix=suffix)
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'semifinalcals_plots.html', 'bpsolphase')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                bpsolphase_subpages[ms] = renderer.filename
        
        ctx.update({'summary_plots'   : summary_plots,
                    'delay_subpages' : delay_subpages,
                    'phasegain_subpages' : phasegain_subpages,
                    'bpsolamp_subpages'  : bpsolamp_subpages,
                    'bpsolphase_subpages' : bpsolphase_subpages,
                    'dirname'         : weblog_dir})
                
        return ctx