'''
Created on 29 Oct 2014

@author: sjw
'''
import collections
import os

import pipeline.infrastructure
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.displays.gaincal as gaincal_displays

LOG = logging.get_logger(__name__)

GaincalApplication = collections.namedtuple('GaincalApplication', 
                                            'ms gaintable calmode solint intent spw') 


class T2_4MDetailsGaincalRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='gaincal.mako',
                 description='Gain calibration',
                 always_rerender=False):
        super(T2_4MDetailsGaincalRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, ctx, context, results):
        applications = []
        structure_plots = {}
        amp_vs_time_summaries = {}
        phase_vs_time_summaries = {}
        amp_vs_time_details = {}
        phase_vs_time_details = {}
        diagnostic_amp_vs_time_summaries = {}
        diagnostic_phase_vs_time_summaries = {}
        diagnostic_amp_vs_time_details = {}
        diagnostic_phase_vs_time_details = {}
        
        for result in results:
            vis = os.path.basename(result.inputs['vis'])
            ms = context.observing_run.get_ms(vis)

            applications.extend(self.get_gaincal_applications(context, result, ms))

            # generate the phase structure plots
            plotter = gaincal_displays.RMSOffsetVsRefAntDistanceChart(context, result)
            structure_plots[vis] = plotter.plot()

            # result.final calapps contains p solution for solint=int,inf and a
            # solution for solint=inf.
            calapps = result.final

            # generate amp vs time plots
            plotter = gaincal_displays.GaincalAmpVsTimeSummaryChart(context, result, 
                                                                    calapps, 'TARGET')
            amp_vs_time_summaries[vis] = plotter.plot()

            # generate phase vs time plots
            plotter = gaincal_displays.GaincalPhaseVsTimeSummaryChart(context, result,
                                                                      calapps, 'TARGET')
            phase_vs_time_summaries[vis] = plotter.plot()

            # generate diagnostic phase vs time plots for bandpass solution, i.e. 
            # with solint=int
            plotter = gaincal_displays.GaincalPhaseVsTimeSummaryChart(context, result,
                                                                      calapps, 'BANDPASS')
            diagnostic_phase_vs_time_summaries[vis] = plotter.plot()

            # generate diagnostic amp vs time plots for bandpass solution, 
            # pointing to the diagnostic calapps outside result.final
            calapps = result.calampresult.final
            plotter = gaincal_displays.GaincalAmpVsTimeSummaryChart(context, result, 
                                                                    calapps, '')
            diagnostic_amp_vs_time_summaries[vis] = plotter.plot()

            if pipeline.infrastructure.generate_detail_plots(result):
                calapps = result.final

                # phase vs time plots
                plotter = gaincal_displays.GaincalPhaseVsTimeDetailChart(context, result,
                                                                         calapps, 'TARGET')
                phase_vs_time_details[vis] = plotter.plot()
                renderer = GaincalPhaseVsTimePlotRenderer(context, result, 
                                                          phase_vs_time_details[vis])
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())        

                # amp vs time plots
                plotter = gaincal_displays.GaincalAmpVsTimeDetailChart(context, result,
                                                                       calapps, 'TARGET')
                amp_vs_time_details[vis] = plotter.plot()
                renderer = GaincalAmpVsTimePlotRenderer(context, result, 
                                                        amp_vs_time_details[vis])
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())        

                # phase vs time for solint=int
                plotter = gaincal_displays.GaincalPhaseVsTimeDetailChart(context, result,
                                                                         calapps, 'BANDPASS')
                diagnostic_phase_vs_time_details[vis] = plotter.plot()
                renderer = GaincalPhaseVsTimeDiagnosticPlotRenderer(context,
                        result, diagnostic_phase_vs_time_details[vis])
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())        

                # amp vs time plots for solint=int
                calapps = result.calampresult.final
                plotter = gaincal_displays.GaincalAmpVsTimeDetailChart(context, result,
                                                                       calapps, '')
                diagnostic_amp_vs_time_details[vis] = plotter.plot()
                renderer = GaincalAmpVsTimeDiagnosticPlotRenderer(context, 
                        result, diagnostic_amp_vs_time_details[vis])
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())        

            # get the first scan for the PHASE intent(s)
#             first_phase_scan = ms.get_scans(scan_intent='PHASE')[0]
#             scan_id = first_phase_scan.id
#             LOG.trace('Using scan %s for phase structure summary '
#                       'plots' % first_phase_scan.id)
#             structure_summary_plots[vis] = [p for p in structure_plots
#                                             if scan_id in set(p.parameters['scan'].split(','))]
            
        # add the PlotGroups to the Mako context. The Mako template will parse
        # these objects in order to create links to the thumbnail pages we
        # just created
        ctx.update({'applications'        : applications,
                    'structure_plots'     : structure_plots,
                    'amp_vs_time_plots'   : amp_vs_time_summaries,
                    'phase_vs_time_plots' : phase_vs_time_summaries,
                    'diagnostic_amp_vs_time_plots'   : diagnostic_amp_vs_time_summaries,
                    'diagnostic_phase_vs_time_plots' : diagnostic_phase_vs_time_summaries})
    
    def get_gaincal_applications(self, context, result, ms):
        applications = []
        
        calmode_map = {'p':'Phase only',
                       'a':'Amplitude only',
                       'ap':'Phase and amplitude'}
        
        for calapp in result.final:
            solint = calapp.origin.inputs['solint']

            if solint == 'inf':
                solint = 'Infinite'
            
            # Convert solint=int to a real integration time. 
            # solint is spw dependent; science windows usually have the same
            # integration time, though that's not guaranteed.
            if solint == 'int':
                in_secs = ['%0.2fs' % (dt.seconds + dt.microseconds * 1e-6) 
                           for dt in utils.get_intervals(context, calapp)]
                solint = 'Per integration (%s)' % utils.commafy(in_secs, quotes=False, conjunction='or')
            
            gaintable = os.path.basename(calapp.gaintable)
            spw = ', '.join(calapp.spw.split(','))

            to_intent = ', '.join(calapp.intent.split(','))
            if to_intent == '':
                to_intent = 'ALL'

            calmode = calapp.origin.inputs['calmode']
            calmode = calmode_map.get(calmode, calmode)
            a = GaincalApplication(ms.basename, gaintable, solint, calmode,
                                   to_intent, spw)
            applications.append(a)

        return applications


class GaincalPhaseVsTimePlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, plots):
        vis = os.path.basename(result.inputs['vis'])
        title = 'Phase vs time for %s' % vis
        outfile = filenamer.sanitize('phase_vs_time-%s.html' % vis)
        
        super(GaincalPhaseVsTimePlotRenderer, self).__init__(
                'generic_x_vs_y_detail_plots.html', context, 
                result, plots, title, outfile)


# class GaincalPhaseVsTimeDiagnosticPlotRenderer(basetemplates.JsonPlotRenderer):
#     def __init__(self, context, result, plots):
#         vis = os.path.basename(result.inputs['vis'])
#         title = 'Phase vs time for %s' % vis
#         outfile = filenamer.sanitize('diagnostic_phase_vs_time-%s.html' % vis)
#         
#         super(GaincalPhaseVsTimeDiagnosticPlotRenderer, self).__init__(
#                 'diagnostic_phase_vs_time_plots.mako', context, 
#                 result, plots, title, outfile)
# 
#         # The basic json dictionary is set in __init__. Now add the scores.
#         d = json.loads(self.json)
#         score_types = ('PHASE_SCORE_XY', 'PHASE_SCORE_X2X1')
#         # First check if we have scores for this MS
#         table_names = [name for name in self.result.qa.qa_results_dict.iterkeys() if name.find(vis) != -1]
#         if (table_names != []):
#             for key in d.iterkeys():
#                 spw = d[key]['spw']
#                 ant = d[key]['ant']
#                 for score_type in score_types:
#                     average_score = 0.0
#                     num_scores = 0
#                     for table_name in table_names:
#                         spw_id = int(spw)
#                         ant_id_dict = dict((value, key) for (key, value) in self.result.qa.qa_results_dict[table_name]['QASCORES']['ANTENNAS'].iteritems())
#                         ant_id = ant_id_dict[ant]
#                         phase_field_ids = self.result.qa.qa_results_dict[table_name]['PHASE_FIELDS']
#                         if (phase_field_ids != []):
#                             for field_id in phase_field_ids:
#                                 score = self.result.qa.qa_results_dict[table_name]['QASCORES']['SCORES'][field_id][spw_id][ant_id][score_type]
#                                 if (score == 'C/C'):
#                                     average_score += -0.1
#                                 else:
#                                     average_score += score
#                                 num_scores += 1
#                         else:
#                             average_score += 1.0
#                             num_scores += 1
#                     if (num_scores != 0):
#                         average_score /= num_scores
#                     d[key][score_type] = average_score
# 
#         else:
#             # We don't have scores
#             for key in d.iterkeys():
#                 for score_type in score_types:
#                     d[key][score_type] = 1.0
# 
#         self.json = json.dumps(d)

class GaincalPhaseVsTimeDiagnosticPlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, plots):
        vis = os.path.basename(result.inputs['vis'])
        title = 'Phase vs time for %s' % vis
        outfile = filenamer.sanitize('diagnostic_phase_vs_time-%s.html' % vis)

        # collect QA results generated for this vis
        self._qa_data = [v for k, v in result.qa.qa_results_dict.items()
                         if vis in k]
        
        self._score_types = frozenset(['PHASE_SCORE_XY', 'PHASE_SCORE_X2X1'])
                
        super(GaincalPhaseVsTimeDiagnosticPlotRenderer, self).__init__(
                'diagnostic_phase_vs_time_plots.mako', context, result, plots,
                title, outfile)        

    def update_json_dict(self, json_dict, plot):
        ant_name = plot.parameters['ant']
        spw_id = plot.parameters['spw']

        scores_dict = {}        
        for qa_data in self._qa_data:
            antenna_ids = dict((v, k) for (k, v) in qa_data['QASCORES']['ANTENNAS'].items())
            ant_id = antenna_ids[ant_name]

            for score_type in self._score_types:            
                average_score = 0.0
                num_scores = 0
    
                phase_field_ids = qa_data['PHASE_FIELDS']            
                if phase_field_ids:
                    for field_id in phase_field_ids:
                        score = qa_data['QASCORES']['SCORES'][field_id][spw_id][ant_id][score_type]
                        if score == 'C/C':
                            average_score += -0.1
                        else:
                            average_score += score
                        num_scores += 1
                else:
                    average_score += 1.0
                    num_scores += 1
    
                if num_scores != 0:
                    average_score /= num_scores
                scores_dict[score_type] = average_score

        if not scores_dict:
            scores_dict = dict((score_type, 1.0) 
                               for score_type in self._score_types)
        
        json_dict.update(scores_dict)
        plot.scores = scores_dict


class GaincalAmpVsTimePlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, plots):
        vis = os.path.basename(result.inputs['vis'])
        title = 'Amplitude vs time for %s' % vis
        outfile = filenamer.sanitize('amp_vs_time-%s.html' % vis)
        
        super(GaincalAmpVsTimePlotRenderer, self).__init__(
                'generic_x_vs_y_detail_plots.html', context, 
                result, plots, title, outfile)


class GaincalAmpVsTimeDiagnosticPlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, plots):
        vis = os.path.basename(result.inputs['vis'])
        title = 'Amplitude vs time for %s' % vis
        outfile = filenamer.sanitize('diagnostic_amp_vs_time-%s.html' % vis)
        
        super(GaincalAmpVsTimeDiagnosticPlotRenderer, self).__init__(
                'generic_x_vs_y_detail_plots.html', context, 
                result, plots, title, outfile)
