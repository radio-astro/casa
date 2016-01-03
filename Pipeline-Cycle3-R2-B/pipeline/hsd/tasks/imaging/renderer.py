import os
import collections
import shutil

import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.displays.singledish as displays

from ..common import renderer as sdsharedrenderer

from . import imaging
from . import scaling

LOG = logging.get_logger(__name__)

class T2_4MDetailsSingleDishImagingRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='hsd_imaging.mako', 
                 description='Image single dish data',
                 always_rerender=False):
        super(T2_4MDetailsSingleDishImagingRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)
        
    def update_mako_context(self, ctx, context, results):            
        plots = []
        #for image_item in result.outcome:
        jyperk = collections.defaultdict(lambda: collections.defaultdict(
                                         lambda: collections.defaultdict(
                                         lambda: collections.defaultdict(lambda: 'N/A (1.0)'))))
        reffile = None
        for r in results:
            if isinstance(r, imaging.SDImagingResults):
                image_item = r.outcome['image']
                antenna_list = r.outcome['file_index']
                imagemode = r.outcome['imagemode']
                spwid = image_item.spwlist
                spw_type = context.observing_run[antenna_list[0]].spectral_window[spwid[0]].type if imagemode.upper()!='AMPCAL' else 'TP'
                task_cls = displays.SDImageDisplayFactory(spw_type)
                inputs = task_cls.Inputs(context,result=r)
                task = task_cls(inputs)
                plots.append(task.plot())
            elif isinstance(r, scaling.IntensityScalingResults) and r.outcome['factors'] is not None:
                LOG.trace('This result should be a IntensityScalingResults object to store jyperk information')
                for st in context.observing_run:
                    ms = st.ms
                    vis = ms.basename
                    ant = st.antenna.name
                    fs = r.outcome['factors'][vis][ant]
                    for (spwid,spw) in st.spectral_window.items():
                        corrs = st.polarization[spw.pol_association[0]].corr_string
                        if fs.has_key(spwid) and spw.is_target and spw.nchan > 1 and spw.nchan != 4:
                            fp = fs[spwid]
                            LOG.info('fp=%s'%(fp))
                            LOG.info('corrs=%s'%(corrs))
                            for corr in corrs:
                                if corr in fp.keys():
                                    ckey = corr
                                elif 'I' in fp.keys() and corr in ['XX', 'YY', 'RR', 'LL']:
                                    ckey = 'I'
                                else:
                                    ckey = None
                                LOG.info('corr=%s ckey=%s'%(corr,ckey))
                                if ckey is not None:
                                    jyperk[vis][ant][spwid][corr] = fp[ckey]
                reffile = r.outcome['reffile']
        reffile_copied = None
        if reffile is not None and os.path.exists(reffile):
            stage_dir = os.path.join(context.report_dir, 'stage%s'%(results.stage_number))
            LOG.debug('copying %s to %s'%(reffile, stage_dir))
            shutil.copy2(reffile, stage_dir)
            reffile_copied = os.path.join(stage_dir, os.path.basename(reffile))
        ctx.update({'jyperk': jyperk,
                    'reffile': reffile_copied})
            
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
                subpage[name] = os.path.basename(renderer.path)
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