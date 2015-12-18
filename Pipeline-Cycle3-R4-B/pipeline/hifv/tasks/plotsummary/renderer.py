import collections
import os

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.displays.vla.plotsummarydisplay as plotsummarydisplay


LOG = logging.get_logger(__name__)






class T2_4MDetailsplotsummaryRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='plotsummary.mako',
                 description='VLA Plot Summary', always_rerender=False):
        super(T2_4MDetailsplotsummaryRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)
    
    def get_display_context(self, context, results):
        super_cls = super(T2_4MDetailsplotsummaryRenderer, self)
        ctx = super_cls.get_display_context(context, results)
        
        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % results.stage_number)
        
        summary_plots = {}

        for result in results:
            
            plotter = plotsummarydisplay.plotsummarySummaryChart(context, result)
            plots = plotter.plot()
            ms = os.path.basename(result.inputs['vis'])
            summary_plots[ms] = plots
            
        ctx.update({'summary_plots'   : summary_plots,
                    'dirname'         : weblog_dir})
                
        return ctx