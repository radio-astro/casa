import os

import pipeline.infrastructure.displays.singledish as displays
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates

LOG = logging.get_logger(__name__)


class T2_4MDetailsSingleDishInspectDataRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='hsd_inspectdata.mako',
                 description='Inspect data',
                 always_rerender=False):
        super(T2_4MDetailsSingleDishInspectDataRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)
        
    def update_mako_context(self, ctx, context, results):
        plot_types = [{'type': 'pointing',
                       'renderer': displays.SDPointingDisplay,
                       'plot_title': 'Telescope Pointing on the Sky'},
                      {'type': 'azel',
                       'renderer': displays.SDAzElDisplay,
                       'plot_title': 'Azimuth/Elevation vs Time'},
                      {'type': 'weather',
                       'renderer': displays.SDWeatherDisplay,
                       'plot_title': 'Weather vs Time'},
                      {'type': 'wvr',
                       'renderer': displays.SDWvrDisplay,
                       'plot_title': 'WVR Reading vs Time'}]
        
        summary_plots = {} 
        subpages = {}
        for _types in plot_types:
            renderer_cls = _types['renderer']
            inputs = renderer_cls.Inputs(context, results[0])
            task = renderer_cls(inputs)
            plots = task.plot()
            plot_group = self._group_by_vis(plots)
            for vis in plot_group.keys():
                if not summary_plots.has_key(vis):
                    summary_plots[vis] = dict([(p['type'], None) for p in plot_types])
                summary_plots[vis][_types['type']] = self._get_summary_plot(_types['type'], 
                                                                            plot_group, 
                                                                            vis)#plot_group[vis][0]
            
            plot_list = {}
            for (name, _plots) in plot_group.items():
                if _types['type'] == 'pointing':
                    renderer = SingleDishPointingPlotsRenderer(context, results, name, _plots,
                                                               _types['plot_title'])
                else:
                    renderer = SingleDishInspectDataPlotsRenderer(context, results, name, _plots,
                                                                  _types['plot_title'])
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())
                plot_list[name] = os.path.basename(renderer.path)
                if not subpages.has_key(name):
                    subpages[name] = {}
                subpages[name][_types['type']] = os.path.basename(renderer.path)
   
        ctx.update({'subplot': subpages,
                    'summary': summary_plots})
    
    def _group_by_vis(self, plots):
        plot_group = {}
        for p in [p for _p in plots for p in _p]:
            key = p.parameters['vis']
            if plot_group.has_key(key):
                plot_group[key].append(p)
            else:
                plot_group[key] = [p]
        return plot_group
    
    def _get_summary_plot(self, plot_type, plot_group, vis):
        if plot_type == 'pointing':
            for plot in plot_group[vis]:
                if plot.parameters['type'] == 'on source pointing':
                    return plot
        return plot_group[vis][0]        


class SingleDishInspectDataPlotsRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, name, plots, plot_title):
        outfile = filenamer.sanitize('%s-%s.html' % (plot_title.lower(), name))
        new_title = '%s for %s' % (plot_title, name)

        super(SingleDishInspectDataPlotsRenderer, self).__init__(
                'hsd_inspectdata_plots.mako', context, result, plots, new_title,
                outfile)


class SingleDishPointingPlotsRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, name, plots, plot_title):
        outfile = filenamer.sanitize('%s-%s.html' % (plot_title.lower(), name))
        new_title = '%s for %s' % (plot_title, name)

        super(SingleDishPointingPlotsRenderer, self).__init__(
                'hsd_pointing_plots.mako', context, result, plots, new_title,
                outfile)
    
    def update_json_dict(self, d, plot):
        d['type'] = plot.parameters['type']
