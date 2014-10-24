import os

import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.displays.singledish as displays

from ..common import renderer as sdsharedrenderer

LOG = logging.get_logger(__name__)

class T2_4MDetailsSingleDishCalTsysRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, template='hsd_caltsys.mako', 
                 description='Generate Tsys calibration table',
                 always_rerender=False):
        super(T2_4MDetailsSingleDishCalTsysRenderer, self).__init__(template,
                                                                    description,
                                                                    always_rerender)
        
    def get_display_context(self, context, results):
        ctx = super(T2_4MDetailsSingleDishCalTsysRenderer, self).get_display_context(context, results)
        
        stage_dir = os.path.join(context.report_dir,'stage%d'%(results.stage_number))
        if not os.path.exists(stage_dir):
            os.mkdir(stage_dir)

        inputs = displays.SDTsysDisplay.Inputs(context,results)
        task = displays.SDTsysDisplay(inputs)
        plots = task.plot()
        plots_per_type = self._plots_per_type(plots, None)
        summary_plots = {}
        plot_list = {}
        plot_group = self._group_by_vis(plots_per_type)
        for (name, _plots) in plot_group.items():
            summary_plots[name] = _plots['summary']
            individual_plots = _plots['individual']
            renderer = sdsharedrenderer.SingleDishGenericPlotsRenderer(context, results, name, individual_plots,
                                                      'Tsys vs Frequency')
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
            plot_list[name] = renderer.filename

        ctx.update({'subpage': plot_list,
                    'summary': summary_plots,
                    'dirname': stage_dir})

        return ctx

    def _group_by_vis(self, plots):
        plot_group = {}
        for p in plots[0]:
            key = p.parameters['vis']
            if plot_group.has_key(key):
                plot_group[key]['summary'].append(p)
            else:
                plot_group[key] = {'summary': [p],
                                   'individual': []}
        for p in plots[1]:
            plot_group[p.parameters['vis']]['individual'].append(p)
        return plot_group

        
        
    def _plots_per_type(self, plots, xaxis_list):
        plot_group = [[], []] #[summary plots, individual plots]
        for _plots in plots:
            for p in _plots:
                spw = p.parameters['spw']
                if spw == 'all':
                    plot_group[0].append(p)
                else:
                    plot_group[1].append(p)
        return plot_group
