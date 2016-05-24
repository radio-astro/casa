"""
Created on 29 Oct 2014

@author: sjw
"""
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
        structure_plots = collections.OrderedDict()
        amp_vs_time_summaries = collections.OrderedDict()
        phase_vs_time_summaries = collections.OrderedDict()
        amp_vs_time_details = collections.OrderedDict()
        phase_vs_time_details = collections.OrderedDict()
        diagnostic_amp_vs_time_summaries = collections.OrderedDict()
        diagnostic_phase_vs_time_summaries = collections.OrderedDict()
        diagnostic_amp_vs_time_details = collections.OrderedDict()
        diagnostic_phase_vs_time_details = collections.OrderedDict()
        amp_vs_time_subpages = collections.OrderedDict()
        phase_vs_time_subpages = collections.OrderedDict()
        diagnostic_amp_vs_time_subpages = collections.OrderedDict()
        diagnostic_phase_vs_time_subpages = collections.OrderedDict()
        diagnostic_solints = collections.OrderedDict()

        for result in results:
            vis = os.path.basename(result.inputs['vis'])
            ms = context.observing_run.get_ms(vis)

            ms_applications = self.get_gaincal_applications(context, result, ms)
            applications.extend(ms_applications)

            if vis not in diagnostic_solints:
                diagnostic_solints[vis] = {}
            try:
                diag_phase = [a for a in ms_applications
                              if a.calmode == 'Phase only'
                              and 'TARGET' not in a.intent][0]
                solint = 'int' if 'Per integration' in diag_phase.solint else diag_phase.solint
                diagnostic_solints[vis]['phase'] = solint
            except IndexError:
                diagnostic_solints[vis]['phase'] = 'N/A'

            try:
                diag_solint = result.calampresult.final[0].origin.inputs['solint']
                diagnostic_solints[vis]['amp'] = diag_solint
            except IndexError:
                diagnostic_solints[vis]['amp'] = 'N/A'

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
                    phase_vs_time_subpages[vis] = renderer.path

                # amp vs time plots
                plotter = gaincal_displays.GaincalAmpVsTimeDetailChart(context, result,
                                                                       calapps, 'TARGET')
                amp_vs_time_details[vis] = plotter.plot()
                renderer = GaincalAmpVsTimePlotRenderer(context, result, 
                                                        amp_vs_time_details[vis])
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())        
                    amp_vs_time_subpages[vis] = renderer.path

                # phase vs time for solint=int
                plotter = gaincal_displays.GaincalPhaseVsTimeDetailChart(context, result,
                                                                         calapps, 'BANDPASS')
                diagnostic_phase_vs_time_details[vis] = plotter.plot()
                renderer = GaincalPhaseVsTimeDiagnosticPlotRenderer(context,
                        result, diagnostic_phase_vs_time_details[vis])
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())        
                    diagnostic_phase_vs_time_subpages[vis] = renderer.path

                # amp vs time plots for solint=int
                calapps = result.calampresult.final
                plotter = gaincal_displays.GaincalAmpVsTimeDetailChart(context, result,
                                                                       calapps, '')
                diagnostic_amp_vs_time_details[vis] = plotter.plot()
                renderer = GaincalAmpVsTimeDiagnosticPlotRenderer(context, 
                        result, diagnostic_amp_vs_time_details[vis])
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())        
                    diagnostic_amp_vs_time_subpages[vis] = renderer.path


            # get the first scan for the PHASE intent(s)
#             first_phase_scan = ms.get_scans(scan_intent='PHASE')[0]
#             scan_id = first_phase_scan.id
#             LOG.trace('Using scan %s for phase structure summary '
#                       'plots' % first_phase_scan.id)
#             structure_summary_plots[vis] = [p for p in structure_plots
#                                             if scan_id in set(p.parameters['scan'].split(','))]

        # render plots for all EBs in one page
        for d, plotter_cls, subpages in (
                (amp_vs_time_details, GaincalAmpVsTimePlotRenderer, amp_vs_time_subpages),
                (phase_vs_time_details, GaincalPhaseVsTimePlotRenderer, phase_vs_time_subpages),
                (diagnostic_amp_vs_time_details, GaincalAmpVsTimeDiagnosticPlotRenderer, diagnostic_amp_vs_time_subpages),
                (diagnostic_phase_vs_time_details, GaincalPhaseVsTimeDiagnosticPlotRenderer, diagnostic_phase_vs_time_subpages)):
            if d:
                all_plots = list(utils.flatten([v for v in d.values()]))
                renderer = plotter_cls(context, results, all_plots)
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())
                    # redirect subpage links to master page
                    for vis in subpages:
                        subpages[vis] = renderer.path

        # add the PlotGroups to the Mako context. The Mako template will parse
        # these objects in order to create links to the thumbnail pages we
        # just created
        ctx.update({
            'applications': applications,
            'structure_plots': structure_plots,
            'amp_vs_time_plots': amp_vs_time_summaries,
            'phase_vs_time_plots': phase_vs_time_summaries,
            'diagnostic_amp_vs_time_plots': diagnostic_amp_vs_time_summaries,
            'diagnostic_phase_vs_time_plots': diagnostic_phase_vs_time_summaries,
            'amp_vs_time_subpages': amp_vs_time_subpages,
            'phase_vs_time_subpages': phase_vs_time_subpages,
            'diagnostic_amp_vs_time_subpages': diagnostic_amp_vs_time_subpages,
            'diagnostic_phase_vs_time_subpages': diagnostic_phase_vs_time_subpages,
            'diagnostic_solints': diagnostic_solints
        })

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
            a = GaincalApplication(ms.basename, gaintable, calmode, solint,
                                   to_intent, spw)
            applications.append(a)

        return applications


class GaincalPhaseVsTimePlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, plots):
        vis = utils.get_vis_from_plots(plots)

        title = 'Phase vs time for %s' % vis
        outfile = filenamer.sanitize('phase_vs_time-%s.html' % vis)

        super(GaincalPhaseVsTimePlotRenderer, self).__init__(
                'generic_x_vs_y_spw_ant_plots.mako', context, result, plots, title, outfile)


class GaincalPhaseVsTimeDiagnosticPlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, results, plots):
        vis = utils.get_vis_from_plots(plots)

        title = 'Phase vs time for %s' % vis
        outfile = filenamer.sanitize('diagnostic_phase_vs_time-%s.html' % vis)

        if not isinstance(results, list):
            results = [results]

        # collect QA results generated for this vis
        self._qa_data = {}
        for result in results:
            b = os.path.basename(result.inputs['vis'])
            self._qa_data[b] = [v for k, v in result.qa.qa_results_dict.items()
                                if b in k]

        self._score_types = frozenset(['PHASE_SCORE_XY', 'PHASE_SCORE_X2X1'])
                
        super(GaincalPhaseVsTimeDiagnosticPlotRenderer, self).__init__(
                'diagnostic_phase_vs_time_plots.mako', context, results, plots,
                title, outfile)        

    def update_json_dict(self, json_dict, plot):
        ant_name = plot.parameters['ant']
        spw_id = plot.parameters['spw']

        scores_dict = {}
        for qa_data in self._qa_data[plot.parameters['vis']]:
            antenna_ids = dict((v, k) for (k, v) in qa_data['QASCORES']['ANTENNAS'].items())
            ant_id = antenna_ids[ant_name]

            for score_type in self._score_types:            
                average_score = 0.0
                num_scores = 0
    
                phase_field_ids = set(qa_data['PHASE_FIELDS'])
                if phase_field_ids:
                    # not all PHASE fields have scores, eg. uid://A002/X6a533e/X834.
                    # Avoid KeyErrors by only retrieving scores for those
                    # with scores.
                    fields_with_scores = set(qa_data['QASCORES']['SCORES'].keys())
                    for field_id in phase_field_ids.intersection(fields_with_scores):
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
        vis = utils.get_vis_from_plots(plots)

        title = 'Amplitude vs time for %s' % vis
        outfile = filenamer.sanitize('amp_vs_time-%s.html' % vis)
        
        super(GaincalAmpVsTimePlotRenderer, self).__init__(
                'generic_x_vs_y_spw_ant_plots.mako', context, 
                result, plots, title, outfile)


class GaincalAmpVsTimeDiagnosticPlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, plots):
        vis = utils.get_vis_from_plots(plots)

        title = 'Amplitude vs time for %s' % vis
        outfile = filenamer.sanitize('diagnostic_amp_vs_time-%s.html' % vis)
        
        super(GaincalAmpVsTimeDiagnosticPlotRenderer, self).__init__(
                'generic_x_vs_y_spw_ant_plots.mako', context, 
                result, plots, title, outfile)
