import os

import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.displays.singledish as displays

from ..common import renderer as sdsharedrenderer

LOG = logging.get_logger(__name__)

class T2_4MDetailsSingleDishCalSkyRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, template='hsd_calsky.mako',
                 description='Generate Sky calibration table', 
                 always_rerender=False):
        super(T2_4MDetailsSingleDishCalSkyRenderer, self).__init__(template,
                                                                   description,
                                                                   always_rerender)
    def get_display_context(self, context, results):
        ctx = super(T2_4MDetailsSingleDishCalSkyRenderer, self).get_display_context(context, results)
        
        stage_dir = os.path.join(context.report_dir,'stage%d'%(results.stage_number))
        if not os.path.exists(stage_dir):
            os.mkdir(stage_dir)
              
        inputs = displays.SDSkyDisplay.Inputs(context,results)
        task = displays.SDSkyDisplay(inputs)
        # plots is list-of-list of plot instances
        plots = task.plot()
        
        # plot_group is a dictionary of (MS names, associating plots) 
        plot_group = self._group_by_vis(plots)
        
        # summary_plots is a dictionary of (MS names, list of typical plots for each spw) 
        # at the moment typical plot is a first plot of each spectral window
        summary_plots = self._summary_plots(plot_group)
        
        plot_list = {}
        for (name, _plots) in plot_group.items():
            #renderer = SingleDishCalSkyPlotsRenderer(context, results, name, _plots)
            renderer = sdsharedrenderer.SingleDishGenericPlotsRenderer(context, results, name, _plots,
                                                      'Sky Level vs Frequency')
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

