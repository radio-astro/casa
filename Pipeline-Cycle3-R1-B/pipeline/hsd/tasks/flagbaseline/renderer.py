import os
import collections

import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.sharedrenderer as sharedrenderer
import pipeline.infrastructure.renderer.logger as logger
import pipeline.infrastructure.displays.singledish as displays

from ..common import renderer as sdsharedrenderer
from ..baseline import renderer as baselinerenderer

LOG = logging.get_logger(__name__)

def get_stage_number(result_obj):
    if not isinstance(result_obj, collections.Iterable):
        return get_stage_number([result_obj, ])

    if len(result_obj) is 0:
        msg = 'Cannot get stage number for zero-length results list'
        LOG.error(msg)
        return msg

    return result_obj[0].stage_number

class T2_4MDetailsSingleDishFlagBaselineRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, template='hsd_flagbaseline.mako',
                 always_rerender=False):
        super(T2_4MDetailsSingleDishFlagBaselineRenderer, self).__init__(template,
                                                                         always_rerender)
        self.baseline_renderer = baselinerenderer.T2_4MDetailsSingleDishBaselineRenderer(always_rerender=False)        
        
    def get_display_context(self, context, results):
        ctx = super(T2_4MDetailsSingleDishFlagBaselineRenderer, self).get_display_context(context, 
                                                                                          results)
        
        stage_number = get_stage_number(results)
        stage_dir = os.path.join(context.report_dir,'stage%d'%(stage_number))
        if not os.path.exists(stage_dir):
            os.mkdir(stage_dir)
   
        plot_groups_list = []
        flag_html_list = []
        for r in results:

            baseline_result = r.outcome['baseline']
            flag_result = r.outcome['blflag']
           
            plots = []
            inputs = displays.ClusterDisplay.Inputs(context,result=baseline_result)
            task = displays.ClusterDisplay(inputs)
            plots.append(task.plot())
    
            plot_groups = logger.PlotGroup.create_plot_groups(plots)
            for plot_group in plot_groups:
                renderer = sharedrenderer.PlotGroupRenderer(context, results, plot_group)
                plot_group.filename = renderer.basename
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())
            plot_groups_list.append(plot_groups)
        
            rel_path = os.path.basename(stage_dir)   ### stage#

            html_names = []
            summaries = flag_result.outcome['summary']
            for summary in summaries:
                html_names.append(summary['html'])
                flag_html_list.append(html_names)
                
        baseline_context = []
        for r in results:
            baseline_results = basetask.ResultsList()
            baseline_results.append(r.outcome['baseline'])
            baseline_results.stage_number = results.stage_number
            baseline_context.append(self.baseline_renderer.get_display_context(context, baseline_results))
    
        ctx.update({'dirname': stage_dir,
                    'plot_groups_list': plot_groups_list,
                    'flag_html_list': flag_html_list,
                    'baseline_context': baseline_context})
    
        return ctx

class T2_4MDetailsSingleDishPlotFlagBaselineRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, template='hsd_plotflagbaseline.mako',
                 always_rerender=False):
        super(T2_4MDetailsSingleDishPlotFlagBaselineRenderer, self).__init__(template,
                                                                             always_rerender)
    def get_display_context(self, context, results):
        ctx = super(T2_4MDetailsSingleDishPlotFlagBaselineRenderer, self).get_display_context(context, 
                                                                                              results)
        stage_number = get_stage_number(results)
        stage_dir = os.path.join(context.report_dir,'stage%d'%(stage_number))
        if not os.path.exists(stage_dir):
            os.mkdir(stage_dir)

        plots = []
        for r in results:
            inputs = displays.SDBaselineAllDisplay.Inputs(context,result=r)
            task = displays.SDBaselineAllDisplay(inputs)
            plots.extend(task.plot())
            
        plot_group = self._group_by_vis(plots)
        summary_plots = self._summary_plots(plot_group)

        plot_list = {}
        for (name, _plots) in plot_group.items():
            renderer = SingleDishPlotFlagBaselinePlotsRenderer(context, results, name, _plots,
                                                               'Intensity vs Frequency')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
            plot_list[name] = renderer.filename
                
        # default dirname is relative path so replacing it with absolute path 
        ctx.update({'summary_subpage': plot_list,
                    'summary_plots': summary_plots,
                    'dirname': stage_dir})
    
        return ctx

    def _group_by_vis(self, plots):
        plot_group = {}
        for p in [p for _p in plots for p in _p]:
            key = p.parameters['vis']
            if plot_group.has_key(key):
                plot_group[key].append(p)
            else:
                plot_group[key] = [p]
        return plot_group
    
    def _summary_plots(self, plot_group):
        summary_plots = {}
        for (vis, plots) in plot_group.items():
            spw_list = set()
            summary_plots[vis]= []
            for plot in plots:
                spw = plot.parameters['spw']
                if spw not in spw_list:
                    spw_list.add(spw)
                    summary_plots[vis].append(plot)
        return summary_plots





class SingleDishPlotFlagBaselinePlotsRenderer(sdsharedrenderer.SingleDishGenericPlotsRenderer):
    template = 'generic_x_vs_y_detail_plots.html'    
