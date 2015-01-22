import collections
import os
import contextlib
import pipeline.infrastructure.renderer.weblog as weblog

import pipeline.infrastructure.displays.vla.finalcalsdisplay as finalcalsdisplay
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
        
        
        


class T2_4MDetailsfinalcalsRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self,uri='finalcals.mako', description='Final Calibration Tables', 
                 always_rerender=False):
        super(T2_4MDetailsfinalcalsRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)
    
    def get_display_context(self, context, results):
        super_cls = super(T2_4MDetailsfinalcalsRenderer, self)
        ctx = super_cls.get_display_context(context, results)
        
        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % results.stage_number)
        
        summary_plots = {}
        finaldelay_subpages = {}
        phasegain_subpages = {}
        bpsolamp_subpages = {}
        bpsolphase_subpages = {}
        bpsolphaseshort_subpages = {}
        finalamptimecal_subpages = {}
        finalampfreqcal_subpages = {}
        finalphasegaincal_subpages = {}
        
        for result in results:
            
            plotter = finalcalsdisplay.finalcalsSummaryChart(context, result)
            plots = plotter.plot()
            ms = os.path.basename(result.inputs['vis'])
            summary_plots[ms] = plots
            
            # generate testdelay plots and JSON file
            plotter = finalcalsdisplay.finalDelaysPerAntennaChart(context, result)
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'finalcals_plots.html', 'finaldelays')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                finaldelay_subpages[ms] = renderer.filename
            
                
            # generate phase Gain plots and JSON file
            plotter = finalcalsdisplay.finalphaseGainPerAntennaChart(context, result)
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'finalcals_plots.html', 'phasegain')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                phasegain_subpages[ms] = renderer.filename
                
            # generate amp bandpass solution plots and JSON file
            plotter = finalcalsdisplay.finalbpSolAmpPerAntennaChart(context, result)
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'finalcals_plots.html', 'bpsolamp')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                bpsolamp_subpages[ms] = renderer.filename
                
            # generate phase bandpass solution plots and JSON file
            plotter = finalcalsdisplay.finalbpSolPhasePerAntennaChart(context, result)
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'finalcals_plots.html', 'bpsolphase')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                bpsolphase_subpages[ms] = renderer.filename
                
            # generate phase short bandpass solution plots and JSON file
            plotter = finalcalsdisplay.finalbpSolPhaseShortPerAntennaChart(context, result)
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'finalcals_plots.html', 'bpsolphaseshort')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                bpsolphaseshort_subpages[ms] = renderer.filename
            
            # generate final amp time cal solution plots and JSON file
            plotter = finalcalsdisplay.finalAmpTimeCalPerAntennaChart(context, result)
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'finalcals_plots.html', 'finalamptimecal')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                finalamptimecal_subpages[ms] = renderer.filename
                
            # generate final amp freq cal solution plots and JSON file
            plotter = finalcalsdisplay.finalAmpFreqCalPerAntennaChart(context, result)
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'finalcals_plots.html', 'finalampfreqcal')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                finalampfreqcal_subpages[ms] = renderer.filename
        
            # generate final phase gain cal solution plots and JSON file
            plotter = finalcalsdisplay.finalPhaseGainCalPerAntennaChart(context, result)
            plots = plotter.plot() 
            json_path = plotter.json_filename
            
             # write the html for each MS to disk
            renderer = VLASubPlotRenderer(context, result, plots, json_path, 'finalcals_plots.html', 'finalphasegaincal')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                finalphasegaincal_subpages[ms] = renderer.filename
        
        ctx.update({'summary_plots'   : summary_plots,
                    'finaldelay_subpages' : finaldelay_subpages,
                    'phasegain_subpages' : phasegain_subpages,
                    'bpsolamp_subpages'  : bpsolamp_subpages,
                    'bpsolphase_subpages' : bpsolphase_subpages,
                    'bpsolphaseshort_subpages' : bpsolphaseshort_subpages,
                    'finalamptimecal_subpages' : finalamptimecal_subpages,
                    'finalampfreqcal_subpages' : finalampfreqcal_subpages,
                    'finalphasegaincal_subpages' : finalphasegaincal_subpages,
                    'dirname'         : weblog_dir})
                
        return ctx