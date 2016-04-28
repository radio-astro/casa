import os
import collections

import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.displays.singledish.skycal as skycal_display

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
        summary_amp_vs_freq = collections.defaultdict(list)
        details_amp_vs_freq = collections.defaultdict(list)
        summary_amp_vs_time = collections.defaultdict(list)
        details_amp_vs_time = collections.defaultdict(list)
        amp_vs_freq_subpages = {}
        amp_vs_time_subpages = {}
        for result in results:
            if not result.final:
                continue
            
            # get ms domain object
            vis = os.path.basename(result.inputs['vis'])
            ms = context.observing_run.get_ms(vis)
            
            # calibration table summary
            ms_applications = self.get_skycal_applications(context, result, ms)
            applications.extend(ms_applications)
            
            # iterate over CalApplication instances
            final_original = result.final
            
            summaries_freq = []
            details_freq = []
            summaries_time = []
            details_time = []
            for calapp in final_original:
                result.final = [calapp]
                gainfield = calapp.calfrom[0].gainfield
                
                # Amp vs. Freq: summary plots
                summary_plotter = skycal_display.SingleDishSkyCalAmpVsFreqSummaryChart(context, result, gainfield)
                summaries_freq.extend(summary_plotter.plot())
                
                # Amp vs. Freq: detail plots
                detail_plotter = skycal_display.SingleDishSkyCalAmpVsFreqDetailChart(context, result, gainfield)
                details_freq.extend(detail_plotter.plot())
            
                # Amp vs. Time: summary plots
                summary_plotter = skycal_display.SingleDishSkyCalAmpVsTimeSummaryChart(context, result, calapp)
                summaries_time.extend(summary_plotter.plot())
                
                # Amp vs. Time: detail plots
                detail_plotter = skycal_display.SingleDishSkyCalAmpVsTimeDetailChart(context, result, calapp)
                details_time.extend(detail_plotter.plot())
            
            summary_amp_vs_freq[vis].extend(summaries_freq)
            details_amp_vs_freq[vis].extend(details_freq)
            summary_amp_vs_time[vis].extend(summaries_time)
            details_amp_vs_time[vis].extend(details_time)

            result.final = final_original    
            
        for vis, details in details_amp_vs_freq.items():
            renderer = basetemplates.JsonPlotRenderer(uri='generic_x_vs_y_field_spw_ant_detail_plots.mako',
                                                      context=context,
                                                      result=result,
                                                      plots=details,
                                                      title ='Sky Level vs Frequency',
                                                      outfile='%s-sky_level_vs_frequency.html'%(vis))
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
            amp_vs_freq_subpages[vis] = os.path.basename(renderer.path)
            
        LOG.debug('number of items for details_amp_vs_time: {n}'.format(n=len(details_amp_vs_time)))
        for vis, details in details_amp_vs_time.items():
            LOG.debug('vis={vis}, number of plots {n}'.format(vis=vis, n=len(details)))
            renderer = basetemplates.JsonPlotRenderer(uri='generic_x_vs_y_field_spw_ant_detail_plots.mako',
                                                      context=context,
                                                      result=result,
                                                      plots=details,
                                                      title ='Sky Level vs Time',
                                                      outfile='%s-sky_level_vs_time.html'%(vis))
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
            amp_vs_time_subpages[vis] = os.path.basename(renderer.path)
            
        # update Mako context                
        ctx.update({'applications': applications,
                    'summary_amp_vs_freq': summary_amp_vs_freq,
                    'amp_vs_freq_subpages': amp_vs_freq_subpages,
                    'summary_amp_vs_time': summary_amp_vs_time,
                    'amp_vs_time_subpages': amp_vs_time_subpages})
    
    def get_skycal_applications(self, context, result, ms):
        applications = []
        
        calmode_map = {'ps':'Position-switch',
                       'otfraster':'OTF raster edge'}
        
        calapps = result.outcome
        for calapp in calapps:
            caltype = calmode_map[calapp.origin.inputs['calmode']]
            gaintable = os.path.basename(calapp.gaintable)
            spw = calapp.spw.replace(',', ', ')
            intent = calapp.intent.replace(',', ', ')
            antenna = calapp.antenna
            if antenna == '':
                antenna = ', '.join([a.name for a in ms.antennas])
            field = ms.get_fields(calapp.field)[0].name
            
            applications.append({'ms': ms.basename,
                                 'gaintable': gaintable,
                                 'spw': spw,
                                 'intent': intent,
                                 'field': field,
                                 'antenna': antenna,
                                 'caltype': caltype})
        
        return applications
        

