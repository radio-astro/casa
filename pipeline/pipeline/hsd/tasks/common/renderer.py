import os
import json
import contextlib

import pipeline.infrastructure.renderer.weblog as weblog
import pipeline.infrastructure.filenamer as filenamer

class SingleDishGenericPlotsRenderer(object):
    template = 'sd_generic_plots.html'

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

            # Javascript JSON parser doesn't like Javascript floating point 
            # constants (NaN, Infinity etc.), so convert them to null. We  
            # do not omit the dictionary entry so that the plot is hidden
            # by the filters.
#             if math.isnan(ratio) or math.isinf(ratio):
#                 ratio = 'null'

            d[image_relpath] = {'spw'       : str(spw_id),
                                'ant'       : ant_id,
                                'pol'       : pol_id,
                                'thumbnail' : thumbnail_relpath}

        self.json = json.dumps(d)
         
    def _get_display_context(self):
        return {'pcontext'   : self.context,
                'result'     : self.result,
                'plots'      : self.plots,
                'dirname'    : self.dirname,
                'json'       : self.json,
                'plot_title' : '%s for %s' % (self.plot_title, self.name)}

    @property
    def dirname(self):
        stage = 'stage%s' % self.result.stage_number
        return os.path.join(self.context.report_dir, stage)
    
    @property
    def filename(self):        
        filename = filenamer.sanitize('%s-%s.html' % (self.plot_title.lower(), self.name))
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
    