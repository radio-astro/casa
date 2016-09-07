import os

import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.displays.singledish as displays

from ..common import renderer as sdsharedrenderer

LOG = logging.get_logger(__name__)


class T2_4MDetailsSingleDishBaselineRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    # Renderer class for stage summary
    def __init__(self, template='hsd_baseline.mako',
                 description='Generate Baseline tables and subtract spectral baseline',
                 always_rerender=False):
        super(T2_4MDetailsSingleDishBaselineRenderer, self).__init__(template,
                                                                     description,
                                                                   always_rerender)
        
    def update_mako_context(self, ctx, context, results):
        plots = []
        sparsemap_plots = []
        for r in results:
            inputs = displays.ClusterDisplay.Inputs(context,result=r)
            task = displays.ClusterDisplay(inputs)
            plots.append(task.plot())
            sparsemap_plots.extend(r.outcome['plots'])

        plot_group = self._group_by_axes(plots)
        plot_detail = [] # keys are 'title', 'html', 'cover_plots'
        plot_cover = [] # keys are 'title', 'cover_plots'
        # Render stage details pages
        details_title = ["R.A. vs Dec."]
        for (name, _plots) in plot_group.items():
            group_desc = {'title': name}
            if name in details_title:
                renderer = SingleDishClusterPlotsRenderer(context, results, name, _plots)
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())
                group_desc['html'] = os.path.basename(renderer.path)
                group_desc['cover_plots'] = self._get_a_plot_per_spw(_plots)
                plot_detail.append(group_desc)
            else:
                group_desc['cover_plots'] = _plots
                plot_cover.append(group_desc)
                
        ctx.update({'detail': plot_detail,
                    'cover_only': plot_cover})
                
        # profile map before and after baseline subtracton
        maptype_list = ['before', 'after']
        for maptype in maptype_list:
            plot_list = self._plots_per_field_with_type(sparsemap_plots, maptype)
            summary = self._summary_plots(plot_list)
            subpage = {}
            for (name, _plots) in plot_list.items():
                plot_title = 'Sparse Profile Map %s Baseline Subtraction'%(maptype.lower())
                renderer = sdsharedrenderer.SingleDishGenericPlotsRenderer(context, results, name, _plots, 
                                                          plot_title)
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())
                subpage[name] = os.path.basename(renderer.path)
            ctx.update({'sparsemap_subpage_%s'%(maptype.lower()): subpage,
                        'sparsemap_%s'%(maptype.lower()): summary})
            
    def _group_by_axes(self, plots):
        plot_group = {}
        for p in [p for _p in plots for p in _p]:
            key = "%s vs %s" % (p.x_axis, p.y_axis)
            if plot_group.has_key(key): plot_group[key].append(p)
            else: plot_group[key] = [p]
        return plot_group
    
    def _get_a_plot_per_spw(self, plots):
        known_spw = []
        plot_list = []
        for p in plots:
            if p.parameters['type'] == 'clustering_final' and p.parameters['spw'] not in known_spw:
                known_spw.append(p.parameters['spw'])
                plot_list.append(p)
        return plot_list
    
    def _plots_per_field_with_type(self, plots, type_string):
        plot_group = {}
        for p in plots:
            if p.parameters['type'].find(type_string) != -1:
                key = p.field
                if plot_group.has_key(key):
                    plot_group[key].append(p)
                else:
                    plot_group[key] = [p]
        return plot_group

    def _summary_plots(self, plot_group):
        summary_plots = {}
        for (field_name, plots) in plot_group.items():
            spw_list = []
            summary_plots[field_name]= []
            for plot in plots:
                spw = plot.parameters['spw']
                if spw not in spw_list:
                    spw_list.append(spw)
                    summary_plots[field_name].append(plot)
        return summary_plots
    
class SingleDishClusterPlotsRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, xytitle, plots):
        outfile = filenamer.sanitize('%s.html' % (xytitle.lower().replace(" ", "_")))
        new_title = "Clustering: %s" % xytitle

        super(SingleDishClusterPlotsRenderer, self).__init__(
                'hsd_cluster_plots.mako', context, result, plots, new_title,
                outfile)
        
    def update_json_dict(self, d, plot):
        d['type'] = plot.parameters['type']
