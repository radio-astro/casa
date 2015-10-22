import collections
import os
import contextlib
import operator
import pipeline.infrastructure.renderer.weblog as weblog

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.filenamer as filenamer

#import pipeline.infrastructure.renderer.htmlrenderer as hr

LOG = logging.get_logger(__name__)


class T2_4MDetailsVLAImportDataRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='vlaimportdata.mako', 
                 description='Register VLA measurement sets with the pipeline', 
                 always_rerender=False):
        super(T2_4MDetailsVLAImportDataRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)
        
    def get_display_context(self, context, result):
        super_cls = super(T2_4MDetailsVLAImportDataRenderer, self)        
        ctx = super_cls.get_display_context(context, result)

        setjy_results = []
        for r in result:
            setjy_results.extend(r.setjy_results)

        measurements = []        
        for r in setjy_results:
            measurements.extend(r.measurements)

        num_mses = reduce(operator.add, [len(r.mses) for r in result])

        ctx.update({'flux_imported' : True if measurements else False,
                    'setjy_results' : setjy_results,
                    'num_mses'      : num_mses})

        return ctx