import os

import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.displays.singledish as displays

from ..common import renderer as sdsharedrenderer

LOG = logging.get_logger(__name__)

class T2_4MDetailsSingleDishCalSkyRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='skycal.mako',
                 description='Single-Dish Sky Calibration', 
                 always_rerender=False):
        super(T2_4MDetailsSingleDishCalSkyRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)
        
    def update_mako_context(self, ctx, context, results):
        applications = []
        for result in results:
            vis = os.path.basename(result.inputs['vis'])
            ms = context.observing_run.get_ms(vis)
            
            ms_applications = self.get_skycal_applications(context, result, ms)
            applications.extend(ms_applications)
                
        # default dirname is relative path so replacing it with absolute path 
        ctx.update({'applications': applications})
    
    def get_skycal_applications(self, context, result, ms):
        applications = []
        
        calmode_map = {'ps':'Position-switch',
                       'otfraster':'OTF raster edge'}
        
        calapp = result.outcome
        caltype = calmode_map[calapp.origin.inputs['calmode']]
        gaintable = os.path.basename(calapp.gaintable)
        spw = calapp.spw.replace(',', ', ')
        intent = calapp.intent.replace(',', ', ')
        antenna = calapp.antenna
        if antenna == '':
            antenna = ', '.join([a.name for a in ms.antennas])
        field = calapp.field.strip('"')
        
        applications.append({'ms': ms.basename,
                             'gaintable': gaintable,
                             'spw': spw,
                             'intent': intent,
                             'field': field,
                             'antenna': antenna,
                             'caltype': caltype})
        
        return applications
        

