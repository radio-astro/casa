'''
Created on 5 Sep 2014

@author: sjw
'''
import operator

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates

LOG = logging.get_logger(__name__)


class T2_4MDetailsImportDataRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='importdata.mako', 
                 description='Register measurement sets with the pipeline', 
                 always_rerender=False):
        super(T2_4MDetailsImportDataRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, mako_context, pipeline_context, result):
        setjy_results = []
        for r in result:
            setjy_results.extend(r.setjy_results)

        measurements = []        
        for r in setjy_results:
            measurements.extend(r.measurements)

        num_mses = reduce(operator.add, [len(r.mses) for r in result])

        mako_context.update({'flux_imported' : True if measurements else False,
                             'setjy_results' : setjy_results,
                             'num_mses'      : num_mses})
