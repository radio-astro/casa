import os
import json

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates

LOG = logging.get_logger(__name__)

class T2_4MDetailsSingleDishMSToScantableRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, template='hsd_mstoscantable.mako',
                 description='Convert MS to Scantable',
                 always_rerender=False):
        super(T2_4MDetailsSingleDishMSToScantableRenderer, self).__init__(template, 
                                                                        description,
                                                                        always_rerender)
        
    def get_display_context(self, context, results):
        ctx = super(T2_4MDetailsSingleDishMSToScantableRenderer, self).get_display_context(context, results)
        
        stage_dir = os.path.join(context.report_dir,'stage%d'%(results[0].stage_number))
        if not os.path.exists(stage_dir):
            os.mkdir(stage_dir)
   
        vismap = {}
        for st in context.observing_run:
            vis = st.ms.basename
            infile = st.basename
            if not vismap.has_key(vis):
                vismap[vis] = []
            vismap[vis].append(infile)
            
        caltablemap = {}
        mappedcaltables = getattr(results[0], 'mappedcaltables', {})
        for (vis, val) in mappedcaltables.items():
            mapped = []
            for (_, v) in val['mapped'].items():
                mapped.extend(v.values())
            caltablemap[val['original']] = mapped
   
        ctx.update({'vismap': vismap,
                    'caltablemap': caltablemap,
                    'dirname': stage_dir})
        
        return ctx
