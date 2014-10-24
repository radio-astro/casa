import os

import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.displays.singledish as displays

from ..common import renderer as sdsharedrenderer

LOG = logging.get_logger(__name__)

class T2_4MDetailsSingleDishImagingRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, template='hsd_imaging.mako', 
                 description='Image single dish data',
                 always_rerender=False):
        super(T2_4MDetailsSingleDishImagingRenderer, self).__init__(template,
                                                                    description,
                                                                    always_rerender)
        
    def get_display_context(self, context, results):
        ctx = super(T2_4MDetailsSingleDishImagingRenderer, self).get_display_context(context, results)
        
        stage_dir = os.path.join(context.report_dir,'stage%d'%(results.stage_number))
        if not os.path.exists(stage_dir):
            os.mkdir(stage_dir)
            
        plots = []
        #for image_item in result.outcome:
        for r in results:
            image_item = r.outcome['image']
            antenna_list = r.outcome['file_index']
            spwid = image_item.spwlist
            spw_type = context.observing_run[antenna_list[0]].spectral_window[spwid[0]].type
            task_cls = displays.SDImageDisplayFactory(spw_type)
            inputs = task_cls.Inputs(context,result=r)
            task = task_cls(inputs)
            plots.append(task.plot())
            
        map_types = {'sparsemap': {'type': 'sd_sparse_map',
                                   'plot_title': 'Sparse Profile Map'},
                     'profilemap': {'type': 'sd_spectral_map',
                                    'plot_title': 'Detailed Profile Map'},
                     'channelmap': {'type': 'channel_map',
                                    'plot_title': 'Channel Map'},
                     'rmsmap': {'type': 'rms_map',
                                'plot_title': 'Baseline RMS Map'},
                     'integratedmap': {'type': 'sd_integrated_map',
                                       'plot_title': 'Integrated Intensity Map'}}
        for (key, value) in map_types.items():
            plot_list = self._plots_per_field_with_type(plots, value['type'])
            summary = self._summary_plots(plot_list)
            subpage = {}
            for (name, _plots) in plot_list.items():
                #renderer = value['renderer'](context, results, name, _plots)
                renderer = sdsharedrenderer.SingleDishGenericPlotsRenderer(context, results, name, _plots, 
                                                          value['plot_title'])
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())
                subpage[name] = renderer.filename
            ctx.update({'%s_subpage'%(key): subpage,
                        '%s_plots'%(key): summary})
            if key == 'sparsemap':
                profilemap_entries = {}
                for (field, _plots) in plot_list.items():
                    _ap = {}
                    for p in _plots:
                        ant = p.parameters['ant']
                        pol = p.parameters['pol']
                        if not _ap.has_key(ant):
                            _ap[ant] = [pol]
                        elif pol not in _ap[ant]:
                            _ap[ant].append(pol)
                    profilemap_entries[field] = _ap
                ctx.update({'profilemap_entries': profilemap_entries})

        ctx.update({'dirname': stage_dir})

        return ctx
    
    def _plots_per_field_with_type(self, plots, type_string):
        plot_group = {}
        for p in [p for _p in plots for p in _p]:
            if p.parameters['type'] == type_string:
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
                if plot.parameters['ant'] == 'COMBINED':
                    idx = spw_list.index(spw)
                    summary_plots[field_name][idx] = plot
        return summary_plots
