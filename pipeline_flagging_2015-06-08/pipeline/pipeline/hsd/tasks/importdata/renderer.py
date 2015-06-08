import operator

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates

LOG = logging.get_logger(__name__)

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
