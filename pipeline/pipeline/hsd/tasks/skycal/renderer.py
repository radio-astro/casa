import os
import collections

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils
from pipeline.domain.datatable import DataTableImpl as DataTable

from . import skycal as skycal_task
from . import display as skycal_display

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
        summary_elev_diff = collections.defaultdict(list)
        details_elev_diff = collections.defaultdict(list)
        amp_vs_freq_subpages = {}
        amp_vs_time_subpages = {}
        elev_diff_subpages = {}
        reference_coords = collections.defaultdict(dict)
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
            summaries_elev = []
            details_elev = []
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
                
                # reference coordinates
                LOG.debug('calapp=%s'%(calapp))
                calmode = utils.get_origin_input_arg(calapp, 'calmode')
                LOG.debug('calmode=\'%s\''%(calmode))
                field_domain = ms.get_fields(gainfield)[0]
                if calmode == 'ps':
                    reference_coord = self._get_reference_coord(context, ms, field_domain)
                    reference_coords[vis][field_domain.name] = reference_coord
                    
                # Compute elevation difference
                eldiff = skycal_task.compute_elevation_difference(context, result)
                                
                # Elevation difference: summary plots
                summaries_elev.extend(skycal_display.plot_elevation_difference(context, result, eldiff, perantenna=False))
                
                # Elevation difference: detail plots
                details_elev.extend(skycal_display.plot_elevation_difference(context, result, eldiff, perantenna=True))
                
            
            summary_amp_vs_freq[vis].extend(summaries_freq)
            details_amp_vs_freq[vis].extend(details_freq)
            summary_amp_vs_time[vis].extend(summaries_time)
            details_amp_vs_time[vis].extend(details_time)
            summary_elev_diff[vis].extend(summaries_elev)
            details_elev_diff[vis].extend(details_elev)

            result.final = final_original    
            
            
        # Sky Level vs Frequency
        flattened = [plot for inner in details_amp_vs_freq.values() for plot in inner]
        renderer = basetemplates.JsonPlotRenderer(uri='hsd_generic_x_vs_y_ant_field_spw_plots.mako',
                                                  context=context,
                                                  result=result,
                                                  plots=flattened,
                                                  title ='Sky Level vs Frequency',
                                                  outfile='sky_level_vs_frequency.html')
        with renderer.get_file() as fileobj:
            fileobj.write(renderer.render())
        for vis in details_amp_vs_freq.keys():
            amp_vs_freq_subpages[vis] = os.path.basename(renderer.path)        
            
        # Sky Level vs Time
        flattened = [plot for inner in details_amp_vs_time.values() for plot in inner]
        renderer = basetemplates.JsonPlotRenderer(uri='hsd_generic_x_vs_y_ant_field_spw_plots.mako',
                                                  context=context,
                                                  result=result,
                                                  plots=flattened,
                                                  title ='Sky Level vs Time',
                                                  outfile='sky_level_vs_time.html')
        with renderer.get_file() as fileobj:
            fileobj.write(renderer.render())
        for vis in details_amp_vs_time.keys():
            amp_vs_time_subpages[vis] = os.path.basename(renderer.path)
            
        # Elevation difference
        flattened = [plot for inner in details_elev_diff.values() for plot in inner]
        renderer = basetemplates.JsonPlotRenderer(uri='hsd_generic_x_vs_y_ant_field_spw_plots.mako',
                                                context=context,
                                                result=result,
                                                plots=flattened,
                                                title='Elevation Difference vs. Time',
                                                outfile='elevation_difference_vs_time.html')
        with renderer.get_file() as fileobj:
            fileobj.write(renderer.render())
        for vis in details_elev_diff.keys():
            elev_diff_subpages[vis] = os.path.basename(renderer.path)
           
        LOG.debug('reference_coords=%s'%(reference_coords))    
        
        # update Mako context                
        ctx.update({'applications': applications,
                    'summary_amp_vs_freq': summary_amp_vs_freq,
                    'amp_vs_freq_subpages': amp_vs_freq_subpages,
                    'summary_amp_vs_time': summary_amp_vs_time,
                    'amp_vs_time_subpages': amp_vs_time_subpages,
                    'summary_elev_diff': summary_elev_diff,
                    'elev_diff_subpages': elev_diff_subpages,
                    'reference_coords': reference_coords})
    
    def get_skycal_applications(self, context, result, ms):
        applications = []
        
        calmode_map = {'ps':'Position-switch',
                       'otfraster':'OTF raster edge'}
        
        calapps = result.outcome
        for calapp in calapps:
            caltype = calmode_map[utils.get_origin_input_arg(calapp, 'calmode')]
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
        

    def _get_reference_coord(self, context, ms, field):
        LOG.debug('_get_reference_coord({ms}, {field})'.format(ms=ms.basename, field=field.name))
        spws = ms.get_spectral_windows(science_windows_only=True)
        dd = ms.get_data_description(spw=spws[0].id)
        data_desc_id = dd.id
        reference_states = [state for state in ms.states if 'REFERENCE' in state.intents]
        state_ids = [state.id for state in reference_states]
        field_id = field.id
        with casatools.TableReader(ms.name) as tb:
            t = tb.query('ANTENNA1==ANTENNA2 && FIELD_ID=={field} && DATA_DESC_ID={ddid} && STATE_ID IN {states}'.format(field=field_id, ddid=data_desc_id, states=state_ids))
            rownumbers = t.rownumbers()
            antenna_ids = t.getcol('ANTENNA1')
            times = t.getcol('TIME')
            t.close()
            timeref = tb.getcolkeyword('TIME', 'MEASINFO')['Ref']
            timeunit = tb.getcolkeyword('TIME', 'QuantumUnits')[0]
        with casatools.MSMDReader(ms.name) as msmd:
            pointing_direction = msmd.pointingdirection(rownumbers[0])
            antenna_position = msmd.antennaposition(antenna_ids[0])
        qa = casatools.quanta
        me = casatools.measures
        epoch = me.epoch(rf=timeref, v0=qa.quantity(times[0], timeunit))
        
        LOG.debug('pointing_direction=%s'%(pointing_direction))
        
        direction = pointing_direction['antenna1']['pointingdirection']
        me.doframe(antenna_position)
        me.doframe(epoch)
        # 2018/04/18 TN
        # CAS-10874 single dish pipeline should use a direction reference
        # taken from input MS
        datatable_name = os.path.join(context.observing_run.ms_datatable_name, ms.basename)
        datatable = DataTable()
        datatable.importdata(datatable_name, minimal=False, readonly=True)
        outref = datatable.direction_ref
        
        converted= me.measure(direction, rf=outref)
        LOG.debug('converted direction=%s'%(converted))
        coord = '{ref} {ra} {dec}'.format(ref=outref,
                                          ra=qa.formxxx(converted['m0'], format='hms'),
                                          dec=qa.formxxx(converted['m1'], format='dms'))
        
        return coord
