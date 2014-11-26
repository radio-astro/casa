'''
Created on 11 Sep 2014

@author: sjw
'''
import collections
import os

import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.displays.bandpass as bandpass
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils

LOG = logging.get_logger(__name__)

BandpassApplication = collections.namedtuple('BandpassApplication', 
                                             'ms gaintable bandtype solint intent spw') 
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
                description=description, always_rerender=always_rerender)

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
        applications = []
        phaseup_applications = []
        amp_refant = {}
        amp_mode = {}
        amp_details = {}
        amp_vs_time_subpages = {}

        phase_refant = {}
        phase_mode = {}
        phase_details = {}
        phase_vs_time_subpages = {}

        for result in results:
            vis = os.path.basename(result.inputs['vis'])
            ms = context.observing_run.get_ms(vis)
            applications.extend(self.get_bandpass_applications(context, result, ms))
            phaseup_applications.extend(self.get_phaseup_applications(context, result, ms))
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
            
            non_refants = [p for p in summaries
                           if p.parameters['ant'] != ms_refant]
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

        # add the PlotGroups to the Mako context. The Mako template will parse
        # these objects in order to create links to the thumbnail pages we
        # just created
        ctx.update({'applications'         : applications,
                    'phaseup_applications' : phaseup_applications,
                    'amp_mode'             : amp_mode,
                    'amp_refant'           : amp_refant,
                    'phase_mode'           : phase_mode,
                    'phase_refant'         : phase_refant,
                    'amp_subpages'         : amp_vs_time_subpages,
                    'phase_subpages'       : phase_vs_time_subpages,
                    'dirname'              : stage_dir})

        return ctx

    def get_phaseup_applications(self, context, result, ms):
        # return early if phase-up was not activated
        if result.inputs.get('phaseup', False) != True:
            return []
        
        calmode_map = {'p':'Phase only',
                       'a':'Amplitude only',
                       'ap':'Phase and amplitude'}
        
        # identify phaseup from 'preceding' list attached to result
        phaseup_calapps = [] 
        for previous_result in result.preceding:
            for calapp in previous_result:
                l = [cf for cf in calapp.calfrom if cf.caltype == 'gaincal']
                if l and calapp not in phaseup_calapps:
                    phaseup_calapps.append(calapp)
                
        applications = []
        for calapp in phaseup_calapps:
            solint = calapp.origin.inputs['solint']

            if solint == 'inf':
                solint = 'Infinite'
            
            # Convert solint=int to a real integration time. 
            # solint is spw dependent; science windows usually have the same
            # integration time, though that's not guaranteed by the MS.
            if solint == 'int':
                in_secs = ['%0.2fs' % (dt.seconds + dt.microseconds * 1e-6) 
                           for dt in utils.get_intervals(context, calapp)]
                solint = 'Per integration (%s)' % utils.commafy(in_secs, quotes=False, conjunction='or')
            
            calmode = calapp.origin.inputs.get('calmode', 'N/A')
            calmode = calmode_map.get(calmode, calmode)
            minblperant = calapp.origin.inputs.get('minblperant', 'N/A')
            minsnr = calapp.origin.inputs.get('minsnr', 'N/A')
            flagged = 'TODO'
            phaseupbw = result.inputs.get('phaseupbw', 'N/A')

            a = PhaseupApplication(ms.basename, calmode, solint, minblperant,
                                   minsnr, flagged, phaseupbw)
            applications.append(a)

        return applications
    
    def get_bandpass_applications(self, context, result, ms):
        applications = []
        
        bandtype_map = {'B'    :'Channel',
                        'BPOLY':'Polynomial'}                       
        
        for calapp in result.final:
            solint = calapp.origin.inputs['solint']

            if solint == 'inf':
                solint = 'Infinite'
            
            # Convert solint=int to a real integration time. 
            # solint is spw dependent; science windows usually have the same
            # integration time, though that's not guaranteed by the MS.
            if solint == 'int':
                in_secs = ['%0.2fs' % (dt.seconds + dt.microseconds * 1e-6) 
                           for dt in utils.get_intervals(context, calapp)]
                solint = 'Per integration (%s)' % utils.commafy(in_secs, quotes=False, conjunction='or')
            
            gaintable = os.path.basename(calapp.gaintable)
            spw = ', '.join(calapp.spw.split(','))

            to_intent = ', '.join(calapp.intent.split(','))
            if to_intent == '':
                to_intent = 'ALL'

            # TODO get this from the calapp rather than the top-level inputs?
            bandtype = calapp.origin.inputs['bandtype']
            bandtype = bandtype_map.get(bandtype, bandtype)
            a = BandpassApplication(ms.basename, gaintable, bandtype, solint,
                                    to_intent, spw)
            applications.append(a)

        return applications


class BaseBandpassPlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, uri, context, result, plots, title, outfile,
                 score_types):
        self._vis = os.path.basename(result.inputs['vis'])
        self._ms = context.observing_run.get_ms(self._vis)

        caltable = result.final[0].gaintable
        self._num_pols = utils.get_num_caltable_polarizations(caltable)
        self._qa_data = result.qa.rawdata
        self._score_types = score_types
                
        super(BaseBandpassPlotRenderer, self).__init__(
                uri, context, result, plots, title, outfile)        

    def update_json_dict(self, json_dict, plot):
        spw = int(plot.parameters['spw'])
        dd = self._ms.get_data_description(spw=spw)
        if dd is None:
            return

        antennas = self._ms.get_antenna(plot.parameters['ant'])
        assert len(antennas) is 1, 'plot antennas != 1'
        antenna = antennas[0]

        scores_dict = collections.defaultdict(dict)
        for corr_axis in dd.corr_axis:
            pol_id = dd.get_polarization_id(corr_axis)
            # QA dictionary keys are a function of both antenna and feed.
            qa_id = int(antenna.id) * self._num_pols + pol_id
            qa_str = str(qa_id)
    
            for score_type in self._score_types:            
                if 'QASCORES' in self._qa_data:
                    try:
                        score = self._qa_data['QASCORES'][score_type][str(spw)][qa_str]
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
            
    def update_mako_context(self, mako_context):
        super(BaseBandpassPlotRenderer, self).update_mako_context(mako_context)
        mako_context['vis'] = self._vis


class BandpassAmpVsFreqPlotRenderer(BaseBandpassPlotRenderer):
    def __init__(self, context, result, plots):
        vis = os.path.basename(result.inputs['vis'])
        title = 'Calibrated amplitude vs frequency for %s' % vis
        outfile = filenamer.sanitize('amp_vs_freq-%s.html' % vis)
        score_types = frozenset(['AMPLITUDE_SCORE_DD',
                                 'AMPLITUDE_SCORE_FN',
                                 'AMPLITUDE_SCORE_SNR'])
        
        super(BandpassAmpVsFreqPlotRenderer, self).__init__(
                'bandpass-amp_vs_freq_plots.mako', context, 
                result, plots, title, outfile, score_types)


class BandpassPhaseVsFreqPlotRenderer(BaseBandpassPlotRenderer):
    def __init__(self, context, result, plots):
        vis = os.path.basename(result.inputs['vis'])
        title = 'Calibrated phase vs frequency for %s' % vis
        outfile = filenamer.sanitize('phase_vs_freq-%s.html' % vis)
        score_types = frozenset(['PHASE_SCORE_DD',
                                 'PHASE_SCORE_FN',
                                 'PHASE_SCORE_RMS'])
        
        super(BandpassPhaseVsFreqPlotRenderer, self).__init__(
                'bandpass-phase_vs_freq_plots.mako', context, 
                result, plots, title, outfile, score_types)
