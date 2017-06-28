import os
import collections

import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.displays.singledish as displays
import pipeline.infrastructure.utils as utils

from ..common import renderer as sdsharedrenderer

from . import imaging

LOG = logging.get_logger(__name__)

ImageRMSTR = collections.namedtuple('ImageRMSTR', 'name estimate range width rms')

class T2_4MDetailsSingleDishImagingRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='hsd_imaging.mako', 
                 description='Image single dish data',
                 always_rerender=False):
        super(T2_4MDetailsSingleDishImagingRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)
        
    def update_mako_context(self, ctx, context, results):
        plots = []
        image_rms = []
        for r in results:
            if isinstance(r, imaging.SDImagingResultItem):
                image_item = r.outcome['image']
                msid_list = r.outcome['file_index']
                imagemode = r.outcome['imagemode']
                spwid = image_item.spwlist
                ref_ms = context.observing_run.measurement_sets[msid_list[0]]
                ref_spw = spwid[0]
                spw_type =  'TP' if imagemode.upper()=='AMPCAL' else ref_ms.spectral_windows[ref_spw].type 
                task_cls = displays.SDImageDisplayFactory(spw_type)
                inputs = task_cls.Inputs(context,result=r)
                task = task_cls(inputs)
                plots.append(task.plot())
                # RMS of combined image
                if r.outcome.has_key('image_sensitivity'):
                    rms_info = r.outcome['image_sensitivity']
                    icon = '<span class="glyphicon glyphicon-ok"></span>' if rms_info['representative'] else ''
                    tr = ImageRMSTR(image_item.imagename, icon, rms_info['frequency_range'], 
                                    rms_info['channel_width']*1.e-3, rms_info['rms'])
                    image_rms.append(tr)
   
        rms_table = utils.merge_td_columns(image_rms, num_to_merge=0)

                            
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
        for (key, value) in map_types.iteritems():
            plot_list = self._plots_per_field_with_type(plots, value['type'])
            summary = self._summary_plots(plot_list)
            subpage = {}
            for (name, _plots) in plot_list.iteritems():
                #renderer = value['renderer'](context, results, name, _plots)
                renderer = sdsharedrenderer.SingleDishGenericPlotsRenderer(context, results, name, _plots, 
                                                          value['plot_title'])
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())
                subpage[name] = os.path.basename(renderer.path)
            ctx.update({'%s_subpage'%(key): subpage,
                        '%s_plots'%(key): summary})
            if key == 'sparsemap':
                profilemap_entries = {}
                for (field, _plots) in plot_list.iteritems():
                    _ap = {}
                    for p in _plots:
                        ant = p.parameters['ant']
                        pol = p.parameters['pol']
                        if not _ap.has_key(ant):
                            _ap[ant] = [pol]
                        elif pol not in _ap[ant]:
                            _ap[ant].append(pol)
                    profilemap_entries[field] = _ap
                ctx.update({'profilemap_entries': profilemap_entries, 'rms_table': rms_table})
    
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
        for (field_name, plots) in plot_group.iteritems():
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
