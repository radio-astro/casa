import operator

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.hif.tasks.importdata.renderer as super_renderer

LOG = logging.get_logger(__name__)

class T2_4MDetailsSingleDishImportDataRenderer(super_renderer.T2_4MDetailsImportDataRenderer):
    def __init__(self, template='hsd_importdata.mako',
                 description='Register measurement sets with the pipeline', 
                 always_rerender=False):
        super(T2_4MDetailsSingleDishImportDataRenderer, self).__init__(template, description, always_rerender)
        
    def update_mako_context(self, mako_context, pipeline_context, result):
        super(T2_4MDetailsSingleDishImportDataRenderer, self).update_mako_context(mako_context, pipeline_context, result)
        
        
class T2_4MDetailsSingleDishImportDataOldRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, template='hsd_importdataold.mako',
                 description='Register measurement sets with the pipeline', 
                 always_rerender=False):
        super(T2_4MDetailsSingleDishImportDataOldRenderer, self).__init__(template,
                                                                       description,
                                                                       always_rerender)

    def get_display_context(self, context, result):
        super_cls = super(T2_4MDetailsSingleDishImportDataOldRenderer, self)        
        ctx = super_cls.get_display_context(context, result)

        num_mses = reduce(operator.add, [len(r.mses) for r in result])

        ctx.update({'num_mses'      : num_mses})

        return ctx
