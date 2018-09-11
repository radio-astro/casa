import os
import collections

import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils
import pipeline.domain.measures as measures
import pipeline.infrastructure.filenamer as filenamer

from ..common import renderer as sdsharedrenderer
from ..common import utils as sdutils
from ..common import compress

from . import imaging
from . import display

LOG = logging.get_logger(__name__)

ImageRMSTR = collections.namedtuple('ImageRMSTR', 'name estimate range width rms')


class T2_4MDetailsSingleDishImagingRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='hsd_imaging.mako', 
                 description='Image single dish data',
                 always_rerender=False):
        super(T2_4MDetailsSingleDishImagingRenderer, self).__init__(
            uri=uri, description=description, always_rerender=always_rerender)

    def update_mako_context(self, ctx, context, results):
        cqa = casatools.quanta
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
                spw_type = 'TP' if imagemode.upper() == 'AMPCAL' else ref_ms.spectral_windows[ref_spw].type
                task_cls = display.SDImageDisplayFactory(spw_type)
                inputs = task_cls.Inputs(context, result=r)
                task = task_cls(inputs)
                plots.append(task.plot())
                # RMS of combined image
                if r.sensitivity_info is not None:
                    rms_info = r.sensitivity_info
                    sensitivity = rms_info.sensitivity
                    icon = '<span class="glyphicon glyphicon-ok"></span>' if rms_info.representative else ''
                    tr = ImageRMSTR(image_item.imagename, icon, rms_info.frequency_range, 
                                    cqa.getvalue(cqa.convert(sensitivity['bandwidth'], 'kHz'))[0],
                                    cqa.getvalue(sensitivity['sensitivity'])[0])
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
                     'momentmap': {'type': 'sd_moment_map',
                                   'plot_title': 'Maximum Intensity Map'},
                     'integratedmap': {'type': 'sd_integrated_map',
                                       'plot_title': 'Integrated Intensity Map'}}

        for (key, value) in map_types.iteritems():
            plot_list = self._plots_per_field_with_type(plots, value['type'])
            LOG.debug('plot_list=%s'%((plot_list)))
            
            # plot_list can be empty
            # typical case is spectral map for NRO
            if len(plot_list) == 0:
                ctx.update({'%s_subpage' % key: None,
                            '%s_plots' % key: None})
                continue
            
            flattened = []
            for inner in plot_list.values():
                for plot in inner:
                    flattened.append(plot)
            LOG.debug('flattened=%s'%((flattened)));
            #summary = self._summary_plots(plot_list)
            if key == 'channelmap':
                summary = self._summary_plots_channelmap(context, plot_list)
            else:
                summary = self._summary_plots(plot_list)
            subpage = {}
            plot_title = value['plot_title']
            LOG.debug('plot_title=%s'%(plot_title));
            renderer = basetemplates.JsonPlotRenderer('generic_x_vs_y_ant_field_spw_pol_plots.mako',
                                                      context,
                                                      results,
                                                      flattened,
                                                      plot_title,
                                                      filenamer.sanitize('%s.html' % (plot_title.lower())))
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
            for (name, _plots) in plot_list.iteritems():
                subpage[name] = os.path.basename(renderer.path)
            ctx.update({'%s_subpage' % key: subpage,
                        '%s_plots' % key: summary})
            if key == 'sparsemap':
                profilemap_entries = {}
                for (field, _plots) in plot_list.iteritems():
                    _ap = {}
                    for p in _plots:
                        ant = p.parameters['ant']
                        pol = p.parameters['pol']
                        field = p.parameters['field']
                        if ant not in _ap:
                            _ap[ant] = [pol]
                        elif pol not in _ap[ant]:
                            _ap[ant].append(pol)
                    profilemap_entries[field] = _ap
                ctx.update({'profilemap_entries': profilemap_entries, 'rms_table': rms_table})
    
    @staticmethod
    def _plots_per_field_with_type(plots, type_string):
        plot_group = {}
        for p in [p for _p in plots for p in _p]:
            if p.parameters['type'] == type_string:
                key = p.field
                if key in plot_group:
                    plot_group[key].append(p)
                else:
                    plot_group[key] = [p]
        return plot_group
        
    @staticmethod
    def _summary_plots(plot_group):
        summary_plots = {}
        for (field_name, plots) in plot_group.iteritems():
            spw_list = []
            summary_plots[field_name] = []
            for plot in plots:
                spw = plot.parameters['spw']
                if spw not in spw_list:
                    spw_list.append(spw)
                    summary_plots[field_name].append(plot)
                if plot.parameters['ant'] == 'COMBINED':
                    idx = spw_list.index(spw)
                    summary_plots[field_name][idx] = plot
        return summary_plots
    
    @staticmethod
    def _summary_plots_channelmap(context, plot_group):
        # take first ms as representative one
        ms = context.observing_run.measurement_sets[0]
        source_names = [filenamer.sanitize(s.name) for s in ms.sources]
        
        summary_plots = {}
        for (field_name, plots) in plot_group.items():
            spw_list = []
            summary_plots[field_name] = []
            best_plot = {}
            min_separation = {}
            
            for plot in plots:
                if plot.parameters['ant'] != 'COMBINED':
                    continue
                
                spw_id = plot.parameters['spw']
                if spw_id not in spw_list:
                    spw_list.append(spw_id)
                source_name = plot.field
                source_index = source_names.index(source_name)
                source_id = ms.sources[source_index].id
                # center frequency
                spw = ms.get_spectral_window(spw_id)
                cf = spw.centre_frequency
                center_freq = float(cf.convert_to(measures.FrequencyUnits.HERTZ).value)
                # first item of rest frequencies
                rest_frequency = sdutils.get_restfrequency(ms.name, spw_id, source_id)
                if rest_frequency is None:
                    # center frequency of the spw (TOPO)
                    # the result may be wrong due to the difference of frequency reference
                    LOG.debug('rest frequency is not available for {} spw {}. Using center frequency instead.'.format(source_name, spw_id))
                    rest_frequency = center_freq
                
                # line window in LSRK frequency
                line_window = plot.parameters['line']
                if line_window[0] > line_window[1]:
                    tmp = line_window[0]
                    line_window[1] = line_window[0]
                    line_window[0] = tmp
                line_center = sum(line_window) / 2
                LOG.debug('line_center = {}'.format(line_center))
                    
                penalty = center_freq
                if line_window[0] <= rest_frequency and rest_frequency <= line_window[1]:
                    separation = abs(line_center - rest_frequency)
                    LOG.debug('line window brackets rest frequency')
                    LOG.debug('FIELD {} SPW {} rest frequency {} separation {}'.format(field_name, spw_id, rest_frequency, separation))
                else:
                    # add penalty term to the separation
                    separation = penalty + abs(line_center - rest_frequency)
                    LOG.debug('FIELD {} SPW {} rest frequency {} separation {} (w/o penalty {})'.format(field_name, 
                                                                                                        spw_id, 
                                                                                                        rest_frequency, 
                                                                                                        separation,
                                                                                                        separation - penalty))

                if spw_id not in best_plot or separation < min_separation[spw_id]:
                    LOG.debug('updating best_plot for SPW {} (min_separation {} separation {})'.format(spw_id,
                                                                                                       min_separation.get(spw_id, None),
                                                                                                       separation))
                    best_plot[spw_id] = plot
                    min_separation[spw_id] = separation

            LOG.debug('FIELD {}'.format(field_name))
            LOG.debug('spw_list {}'.format(spw_list))
            for spw_id in spw_list:
                summary_plots[field_name].append(best_plot[spw_id])
                
        return summary_plots
                
                
