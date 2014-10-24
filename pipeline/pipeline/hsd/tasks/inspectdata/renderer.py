import os
import json

import pipeline.infrastructure.displays.singledish as displays
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates

from ..common import renderer as sdsharedrenderer

LOG = logging.get_logger(__name__)

class T2_4MDetailsSingleDishInspectDataRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, template='hsd_inspectdata.mako',
                 description='Inspect data',
                 always_rerender=False):
        super(T2_4MDetailsSingleDishInspectDataRenderer, self).__init__(template, 
                                                                        description,
                                                                        always_rerender)
        
    def get_display_context(self, context, results):
        ctx = super(T2_4MDetailsSingleDishInspectDataRenderer, self).get_display_context(context, results)
        
        stage_dir = os.path.join(context.report_dir,'stage%d'%(results[0].stage_number))
        if not os.path.exists(stage_dir):
            os.mkdir(stage_dir)
   
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
                plot_list[name] = renderer.filename
                if not subpages.has_key(name):
                    subpages[name] = {}
                subpages[name][_types['type']] = renderer.filename
   
        ctx.update({'subplot': subpages,
                    'summary': summary_plots,
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
    
    def _get_summary_plot(self, plot_type, plot_group, vis):
        if plot_type == 'pointing':
            for plot in plot_group[vis]:
                if plot.parameters['type'] == 'on source pointing':
                    return plot
        return plot_group[vis][0]        

class SingleDishInspectDataPlotsRenderer(sdsharedrenderer.SingleDishGenericPlotsRenderer):
    template = 'sd_inspectdata_plots.html'   

class SingleDishPointingPlotsRenderer(sdsharedrenderer.SingleDishGenericPlotsRenderer):
    template = 'sd_pointing_plots.html'
    
    def __init__(self, context, result, name, plots, plot_title):
        self.context = context
        self.result = result
        self.plots = plots
        self.name = name
        self.plot_title = str(plot_title)

        # all values set on this dictionary will be written to the JSON file
        d = {}
        for plot in plots:
            # calculate the relative pathnames as seen from the browser
            thumbnail_relpath = os.path.relpath(plot.thumbnail,
                                                self.context.report_dir)
            image_relpath = os.path.relpath(plot.abspath,
                                            self.context.report_dir)
            spw_id = plot.parameters['spw']
            ant_id = plot.parameters['ant']
            pol_id = plot.parameters['pol']
            type_str = plot.parameters['type']

            # Javascript JSON parser doesn't like Javascript floating point 
            # constants (NaN, Infinity etc.), so convert them to null. We  
            # do not omit the dictionary entry so that the plot is hidden
            # by the filters.
#             if math.isnan(ratio) or math.isinf(ratio):
#                 ratio = 'null'

            d[image_relpath] = {'spw'       : str(spw_id),
                                'ant'       : ant_id,
                                'pol'       : pol_id,
                                'type'      : type_str,
                                'thumbnail' : thumbnail_relpath}

        self.json = json.dumps(d)
    