import os
import json
import contextlib

import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.displays.singledish as displays

LOG = logging.get_logger(__name__)

class T2_4MDetailsSingleDishBaselineRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    # Renderer class for stage summary
    def __init__(self, template='hsd_baseline.mako',
                 description='Generate Baseline tables and subtract spectral baseline',
                 always_rerender=False):
        super(T2_4MDetailsSingleDishBaselineRenderer, self).__init__(template,
                                                                     description,
                                                                   always_rerender)
        
    def get_display_context(self, context, results):
        ctx = super(T2_4MDetailsSingleDishBaselineRenderer, self).get_display_context(context, results)
        
        stage_dir = os.path.join(context.report_dir,'stage%d'%(results.stage_number))
        if not os.path.exists(stage_dir):
            os.mkdir(stage_dir)
        plots = []
        for r in results:
            inputs = displays.ClusterDisplay.Inputs(context,result=r)
            task = displays.ClusterDisplay(inputs)
            plots.append(task.plot())

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
                group_desc['html'] = renderer.filename
                group_desc['cover_plots'] = self._get_a_plot_per_spw(_plots)
                plot_detail.append(group_desc)
            else:
                group_desc['cover_plots'] = _plots
                plot_cover.append(group_desc)
                
        ctx.update({'detail': plot_detail,
                    'cover_only': plot_cover})
        
        return ctx
    
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
    
class SingleDishClusterPlotsRenderer(object):
    # take a look at WvrgcalflagPhaseOffsetVsBaselinePlotRenderer when we have
    # scores and histograms to generate. there should be a common base class. 
    template = 'sd_cluster_plots.html'
    
    def __init__(self, context, result, xytitle, plots):
        self.context = context
        self.result = result
        self.plots = plots
        self.xy_title = "Clustering: %s" % xytitle
        self.json = json.dumps(self._get_plot_info(plots))

    def _get_plot_info(self, plots): 
        d = {}
        for plot in plots:
            # calculate the relative pathnames as seen from the browser
            thumbnail_relpath = os.path.relpath(plot.thumbnail,
                                                self.context.report_dir)
            image_relpath = os.path.relpath(plot.abspath,
                                            self.context.report_dir)
            d[image_relpath] = {'thumbnail': thumbnail_relpath}
            for key, val in plot.parameters.items():
                d[image_relpath][key] = val
        return d

    def _get_display_context(self):
        return {'pcontext'   : self.context,
                'result'     : self.result,
                'plots'      : self.plots,
                'dirname'    : self.dirname,
                'json'       : self.json,
                'plot_title' : self.xy_title}

    @property
    def dirname(self):
        stage = 'stage%s' % self.result.stage_number
        return os.path.join(self.context.report_dir, stage)
    
    @property
    def filename(self):        
        filename = filenamer.sanitize('%s.html' % (self.xy_title.lower().replace(" ", "_")))
        return filename
    
    @property
    def path(self):
        return os.path.join(self.dirname, self.filename)
    
    def get_file(self):
        if not os.path.exists(self.dirname):
            os.makedirs(self.dirname)
            
        file_obj = open(self.path, 'w')
        return contextlib.closing(file_obj)
    
    def render(self):
        display_context = self._get_display_context()
        t = weblog.TEMPLATE_LOOKUP.get_template(self.template)
        return t.render(**display_context)
