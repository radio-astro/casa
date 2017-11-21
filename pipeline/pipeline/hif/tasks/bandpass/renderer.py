"""
Created on 11 Sep 2014

@author: sjw
"""
import collections
import os
import types

import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils

#import pipeline.infrastructure.displays.bandpass as bandpass
from pipeline.h.tasks.common.displays import bandpass as bandpass

LOG = logging.get_logger(__name__)

BandpassApplication = collections.namedtuple('BandpassApplication', 
                                             'ms bandtype solint intent spw gaintable')

PhaseupApplication = collections.namedtuple('PhaseupApplication', 
                                            'ms calmode solint minblperant minsnr flagged phaseupbw') 


class T2_4MDetailsBandpassRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    """
    T2_4MDetailsBandpassRenderer generates the detailed T2_4M-level plots and
    output specific to the bandpass calibration task.
    """
    def __init__(self, uri='bandpass.mako', 
                 description='Bandpass calibration',
                 always_rerender=False):
        super(T2_4MDetailsBandpassRenderer, self).__init__(uri=uri,
                                                           description=description,
                                                           always_rerender=always_rerender)

    """
    Get the Mako context appropriate to the results created by a Bandpass
    task.
    
    This routine triggers the creation of the plots specific to the bandpass
    task, creating thumbnail pages as necessary. The returned dictionary  
    
    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    :param results: the bandpass results to describe
    :type results: 
        :class:`~pipeline.infrastructure.tasks.bandpass.common.BandpassResults`
    :rtype a dictionary that can be passed to the matching bandpass Mako 
        template
    """
    def update_mako_context(self, ctx, context, results):
        stage_dir = os.path.join(context.report_dir, 
                                 'stage%d' % results.stage_number)
        if not os.path.exists(stage_dir):
            os.mkdir(stage_dir)

        # generate the bandpass-specific plots, collecting the Plot objects
        # returned by the plot generator 
        bandpass_table_rows = []
        phaseup_applications = []
        amp_refant = {}
        amp_mode = {}
        amp_details = {}
        amp_vs_time_subpages = {}

        phase_refant = {}
        phase_mode = {}
        phase_details = {}
        phase_vs_time_subpages = {}

        no_cal_results = [r for r in results if r.applies_adopted]
        with_cal_results = [r for r in results if not r.applies_adopted and r.final]

        # we want table entries for all results, but plots only for those with a caltable
        for result in results:
            vis = os.path.basename(result.inputs['vis'])
            ms = context.observing_run.get_ms(vis)
            bandpass_table_rows.extend(self.get_bandpass_table(context, result, ms))
            phaseup_applications.extend(self.get_phaseup_applications(context, result, ms))

        for result in with_cal_results:
            vis = os.path.basename(result.inputs['vis'])
            ms = context.observing_run.get_ms(vis)
            ms_refant = ms.reference_antenna.split(',')[0]

            # need two summary plots: one for the refant, one for the mode
            plotter = bandpass.BandpassAmpVsFreqSummaryChart(context, result)
            summaries = plotter.plot()
            for_refant = [p for p in summaries 
                          if p.parameters['ant'] == ms_refant]
            amp_refant[vis] = for_refant[0] if for_refant else None

            # replace mode with first non-refant plot until we have scores
            LOG.todo('Replace bp summary plot with mode when scores are in place')
            non_refants = [p for p in summaries
                           if p.parameters['ant'] != ms_refant]
            amp_mode[vis] = non_refants[0] if non_refants else None

            # need two summary plots: one for the refant, one for the mode
            plotter = bandpass.BandpassPhaseVsFreqSummaryChart(context, result)
            summaries = plotter.plot()
            for_refant = [p for p in summaries 
                          if p.parameters['ant'] == ms_refant]
            phase_refant[vis] = for_refant[0] if for_refant else None

            # use the same typical antenna as for amp vs frequency
            non_refants = [p for p in summaries
                           if p.parameters['ant'] == amp_mode[vis].parameters['ant']]
            phase_mode[vis] = non_refants[0] if non_refants else None

            # make phase vs freq plots for all data 
            plotter = bandpass.BandpassPhaseVsFreqDetailChart(context, result)
            phase_details[vis] = plotter.plot()
            renderer = BandpassPhaseVsFreqPlotRenderer(context, result, 
                                                       phase_details[vis])            
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())    
                outfile = os.path.basename(renderer.path)
                phase_vs_time_subpages[ms.basename] = outfile

            plotter = bandpass.BandpassAmpVsFreqDetailChart(context, result)
            amp_details[vis] = plotter.plot()            
            renderer = BandpassAmpVsFreqPlotRenderer(context, result, 
                                                     amp_details[vis])            
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())    
                # the filename is sanitised - the MS name is not. We need to
                # map MS to sanitised filename for link construction.
                outfile = os.path.basename(renderer.path)
                amp_vs_time_subpages[ms.basename] = outfile

        # render plots for all EBs in one page
        for d, plotter_cls, subpages in (
                (phase_details, BandpassPhaseVsFreqPlotRenderer, phase_vs_time_subpages),
                (amp_details, BandpassAmpVsFreqPlotRenderer, amp_vs_time_subpages)):
            if d:
                all_plots = list(utils.flatten([v for v in d.values()]))
                renderer = plotter_cls(context, with_cal_results, all_plots)
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())
                    # redirect the subpages to the master page
                    for vis in subpages:
                        subpages[vis] = renderer.path

        bandpass_table_rows = utils.merge_td_columns(bandpass_table_rows)
        adopted_table_rows = make_adopted_table(context, results)

        # add the PlotGroups to the Mako context. The Mako template will parse
        # these objects in order to create links to the thumbnail pages we
        # just created
        ctx.update({'bandpass_table_rows': bandpass_table_rows,
                    'adopted_table': adopted_table_rows,
                    'phaseup_applications': phaseup_applications,
                    'amp_mode': amp_mode,
                    'amp_refant': amp_refant,
                    'phase_mode': phase_mode,
                    'phase_refant': phase_refant,
                    'amp_subpages': amp_vs_time_subpages,
                    'phase_subpages': phase_vs_time_subpages,
                    'dirname': stage_dir})

        return ctx

    def get_phaseup_applications(self, context, result, ms):
        # return early if phase-up was not activated
        if not result.inputs.get('phaseup', False):
            return []
        
        calmode_map = {'p': 'Phase only',
                       'a': 'Amplitude only',
                       'ap': 'Phase and amplitude'}
        
        # identify phaseup from 'preceding' list attached to result
        phaseup_calapps = [] 
        for previous_result in result.preceding:
            for calapp in previous_result:
                l = [cf for cf in calapp.calfrom if cf.caltype == 'gaincal']
                if l and calapp not in phaseup_calapps:
                    phaseup_calapps.append(calapp)
                
        applications = []
        for calapp in phaseup_calapps:
            solint = utils.get_origin_input_arg(calapp, 'solint')

            if solint == 'inf':
                solint = 'Infinite'
            
            # Convert solint=int to a real integration time. 
            # solint is spw dependent; science windows usually have the same
            # integration time, though that's not guaranteed by the MS.
            if solint == 'int':
                in_secs = ['%0.2fs' % (dt.seconds + dt.microseconds * 1e-6) 
                           for dt in utils.get_intervals(context, calapp)]
                solint = 'Per integration (%s)' % utils.commafy(in_secs, quotes=False, conjunction='or')

            assert(len(calapp.origin) is 1)
            origin = calapp.origin[0]
            calmode = origin.inputs.get('calmode', 'N/A')
            calmode = calmode_map.get(calmode, calmode)
            minblperant = origin.inputs.get('minblperant', 'N/A')
            minsnr = origin.inputs.get('minsnr', 'N/A')
            flagged = 'TODO'
            phaseupbw = result.inputs.get('phaseupbw', 'N/A')

            a = PhaseupApplication(ms.basename, calmode, solint, minblperant,
                                   minsnr, flagged, phaseupbw)
            applications.append(a)

        return applications
    
    def get_bandpass_table(self, context, result, ms):
        applications = []
        
        bandtype_map = {'B': 'Channel',
                        'BPOLY': 'Polynomial'}
        
        for calapp in result.final:
            gaintable = os.path.basename(calapp.gaintable)
            to_intent = ', '.join(calapp.intent.split(','))
            if to_intent == '':
                to_intent = 'ALL'

            for origin in calapp.origins:
                spws = origin.inputs['spw'].split(',')
                
                solint = origin.inputs['solint']
    
                if solint == 'inf':
                    solint = 'Infinite'
                
                # Convert solint=int to a real integration time. 
                # solint is spw dependent; science windows usually have the same
                # integration time, though that's not guaranteed by the MS.
                if solint == 'int':
                    in_secs = ['%0.2fs' % (dt.seconds + dt.microseconds * 1e-6) 
                               for dt in utils.get_intervals(context, calapp, set(spws))]
                    solint = 'Per integration (%s)' % utils.commafy(in_secs, quotes=False, conjunction='or')

                # TODO get this from the calapp rather than the top-level 
                # inputs?
                bandtype = origin.inputs['bandtype']
                bandtype = bandtype_map.get(bandtype, bandtype)
                a = BandpassApplication(ms.basename, bandtype, solint, 
                                        to_intent, ', '.join(spws), gaintable)
                applications.append(a)

        return applications


class BaseBandpassPlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, uri, context, results, plots, title, outfile,
                 score_types):
        # wrap singular lists so the code works the same for scalars and vectors
        if not isinstance(results, collections.Iterable):
            results = [results]

        self._ms = {}
        self._num_pols = {}
        self._qa_data = {}
        for r in results:
            vis = os.path.basename(r.inputs['vis'])
            self._ms[vis] = context.observing_run.get_ms(vis)
            caltable = r.final[0].gaintable
            self._num_pols[vis] = utils.get_num_caltable_polarizations(caltable)
            self._qa_data[vis] = r.qa.rawdata

        self._score_types = score_types

        super(BaseBandpassPlotRenderer, self).__init__(
                uri, context, results, plots, title, outfile)

    def update_json_dict(self, json_dict, plot):
        spw = int(plot.parameters['spw'])
        vis = plot.parameters['vis']
        ms = self._ms[vis]
        dd = ms.get_data_description(spw=spw)
        if dd is None:
            return

        antennas = ms.get_antenna(plot.parameters['ant'])
        assert len(antennas) is 1, 'plot antennas != 1'
        antenna = antennas[0]

        num_pols = self._num_pols[vis]
        qa_data = self._qa_data[vis]

        scores_dict = collections.defaultdict(dict)
        for corr_axis in dd.corr_axis:
            pol_id = dd.get_polarization_id(corr_axis)
            # QA dictionary keys are a function of both antenna and feed.
            qa_id = int(antenna.id) * num_pols + pol_id
            qa_str = str(qa_id)
    
            for score_type in self._score_types:            
                if 'QASCORES' in qa_data:
                    try:
                        score = qa_data['QASCORES'][score_type][str(spw)][qa_str]
                    except KeyError:
                        LOG.error('Could not get %s score for %s (%s) spw %s '
                                  'pol %s (id=%s)', score_type, antenna.name,
                                  antenna.id, spw, corr_axis, qa_str)
                        score = None 
                else:
                    score = 1.0
                    
                scores_dict[score_type][corr_axis] = score
        
        json_dict.update(scores_dict)
        plot.scores = scores_dict
            
    # def update_mako_context(self, mako_context):
    #     super(BaseBandpassPlotRenderer, self).update_mako_context(mako_context)
    #     mako_context['vis'] = self._vis


class BandpassAmpVsFreqPlotRenderer(BaseBandpassPlotRenderer):
    def __init__(self, context, results, plots):
        vis = utils.get_vis_from_plots(plots)

        title = 'Calibrated amplitude vs frequency for %s' % vis
        outfile = filenamer.sanitize('amp_vs_freq-%s.html' % vis)
        score_types = frozenset(['AMPLITUDE_SCORE_DD',
                                 'AMPLITUDE_SCORE_FN',
                                 'AMPLITUDE_SCORE_SNR'])

        super(BandpassAmpVsFreqPlotRenderer, self).__init__(
                'bandpass-amp_vs_freq_plots.mako', context, 
                results, plots, title, outfile, score_types)


class BandpassPhaseVsFreqPlotRenderer(BaseBandpassPlotRenderer):
    def __init__(self, context, results, plots):
        vis = utils.get_vis_from_plots(plots)

        title = 'Calibrated phase vs frequency for %s' % vis
        outfile = filenamer.sanitize('phase_vs_freq-%s.html' % vis)
        score_types = frozenset(['PHASE_SCORE_DD',
                                 'PHASE_SCORE_FN',
                                 'PHASE_SCORE_RMS'])
        
        super(BandpassPhaseVsFreqPlotRenderer, self).__init__(
                'bandpass-phase_vs_freq_plots.mako', context, 
                results, plots, title, outfile, score_types)


AdoptedTR = collections.namedtuple('AdoptedTR', 'vis gaintable')


def make_adopted_table(context, results):
    # will hold all the flux stat table rows for the results
    rows = []

    for adopted_result in [r for r in results if r.applies_adopted]:
        assert (len(adopted_result.final) == 1)
        calapp = adopted_result.final[0]
        vis_cell = os.path.basename(calapp.vis)
        gaintable_cell = os.path.basename(calapp.gaintable)
        tr = AdoptedTR(vis_cell, gaintable_cell)
        rows.append(tr)

    return utils.merge_td_columns(rows)
