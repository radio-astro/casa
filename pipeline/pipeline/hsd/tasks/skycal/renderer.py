import os

import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.displays.bandpass as bandpass

from ..common import renderer as sdsharedrenderer

LOG = logging.get_logger(__name__)

class T2_4MDetailsSingleDishSkyCalRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='skycal.mako',
                 description='Single-Dish Sky Calibration', 
                 always_rerender=False):
        super(T2_4MDetailsSingleDishSkyCalRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)
        
    def update_mako_context(self, ctx, context, results):
        stage_dir = os.path.join(context.report_dir, 
                                 'stage%d' % results.stage_number)
        if not os.path.exists(stage_dir):
            os.mkdir(stage_dir)

        applications = []
        summary_amp = {}
        amp_vs_freq_subpages = {}
        for result in results:
            if not result.final:
                continue
            
            # get ms domain object
            vis = os.path.basename(result.inputs['vis'])
            ms = context.observing_run.get_ms(vis)
            
            # calibration table summary
            ms_applications = self.get_skycal_applications(context, result, ms)
            applications.extend(ms_applications)
            
            # summary plots
            summary_plotter = bandpass.BandpassAmpVsFreqSummaryChart(context, result)
            summaries = summary_plotter.plot()
            summary_amp[vis] = summaries 
            
            # detail plots
            detail_plotter = bandpass.BandpassAmpVsFreqDetailChart(context, result)
            details = detail_plotter.plot()
            print details
            
            #renderer = sdsharedrenderer.SingleDishGenericPlotsRenderer(context, results, vis, details,
            #                                                           'Sky Level vs Frequency')
            renderer = basetemplates.JsonPlotRenderer(uri='generic_x_vs_y_spw_ant_plots.mako',
                                                      context=context,
                                                      result=result,
                                                      plots=details,
                                                      title ='Sky Level vs Frequency',
                                                      outfile='%s-sky_level_vs_frequency.html'%(vis))
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
            amp_vs_freq_subpages[vis] = os.path.basename(renderer.path)
            
        # update Mako context                
        ctx.update({'applications': applications,
                    'summary_amp': summary_amp,
                    'amp_subpages': amp_vs_freq_subpages})
    
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
        

