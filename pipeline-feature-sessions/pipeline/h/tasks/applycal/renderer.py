"""
Created on 24 Oct 2014

@author: sjw
"""
import collections
import itertools
import operator
import os

import pipeline.domain.measures as measures
import pipeline.infrastructure
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils

#import pipeline.infrastructure.displays.applycal as applycal
from pipeline.h.tasks.common.displays import applycal as applycal
#import pipeline.infrastructure.displays.plotatmosphere as plotatmosphere
from pipeline.h.tasks.common.displays import plotatmosphere as plotatmosphere

LOG = logging.get_logger(__name__)

FlagTotal = collections.namedtuple('FlagSummary', 'flagged total')


class T2_4MDetailsApplycalRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='applycal.mako', 
                 description='Apply calibrations from context',
                 always_rerender=False):
        super(T2_4MDetailsApplycalRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, ctx, context, result):
        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % result.stage_number)

        flag_totals = {}
        for r in result:
            if r.inputs['flagsum'] is True:
                flag_totals = utils.dict_merge(flag_totals,
                                           self.flags_for_result(r, context))

        calapps = {}
        for r in result:
            calapps = utils.dict_merge(calapps,
                                       self.calapps_for_result(r))

        caltypes = {}
        for r in result:
            caltypes = utils.dict_merge(caltypes,
                                        self.caltypes_for_result(r))

        filesizes = {}
        for r in result:
            vis = r.inputs['vis']
            ms = context.observing_run.get_ms(vis)
            filesizes[ms.basename] = ms._calc_filesize()

        # return all agents so we get ticks and crosses against each one
        agents = ['before', 'applycal']
        
        ctx.update({
            'flags': flag_totals,
            'calapps': calapps,
            'caltypes': caltypes,
            'agents': agents,
            'dirname': weblog_dir,
            'filesizes': filesizes
        })

        transmission_plots = self.create_transmission_plots(context, result)

        # these dicts map vis to the hrefs of the detail pages
        amp_vs_freq_subpages = {}
        phase_vs_freq_subpages = {}
        amp_vs_uv_subpages = {}

        amp_vs_time_summary_plots, amp_vs_time_subpages = self.create_plots(
            context,
            result,
            applycal.AmpVsTimeSummaryChart,
            ['PHASE', 'BANDPASS', 'AMPLITUDE', 'CHECK', 'TARGET']
        )

        phase_vs_time_summary_plots, phase_vs_time_subpages = self.create_plots(
            context,
            result,
            applycal.PhaseVsTimeSummaryChart,
            ['PHASE', 'BANDPASS', 'AMPLITUDE', 'CHECK', 'TARGET']
        )

        amp_vs_freq_summary_plots = utils.OrderedDefaultdict(list)
        for intents in [['PHASE'], ['BANDPASS'], ['CHECK'], ['AMPLITUDE']]:
            # it doesn't matter that the subpages dict is repeatedly redefined.
            # The only purpose of the returned dict is to map the vis to a
            # non-existing page, which will disable the link.
            plots, amp_vs_freq_subpages = self.create_plots(
                context,
                result,
                applycal.AmpVsFrequencySummaryChart,
                intents
            )

            for vis, vis_plots in plots.items():
                amp_vs_freq_summary_plots[vis].extend(vis_plots)

        phase_vs_freq_summary_plots = utils.OrderedDefaultdict(list)
        for intents in [['PHASE'], ['BANDPASS'], ['CHECK']]:
            plots, phase_vs_freq_subpages = self.create_plots(
                context,
                result,
                applycal.PhaseVsFrequencyPerSpwSummaryChart,
                intents
            )

            for vis, vis_plots in plots.items():
                phase_vs_freq_summary_plots[vis].extend(vis_plots)

        # CAS-7659: Add plots of all calibrator calibrated amp vs uvdist to
        # the WebLog applycal page
        amp_vs_uv_summary_plots = utils.OrderedDefaultdict(list)
        for intents in [['AMPLITUDE'], ['PHASE'], ['BANDPASS'], ['CHECK']]:
            plots, amp_vs_uv_subpages = self.create_plots(
                context,
                result,
                applycal.AmpVsUVSummaryChart,
                intents
            )

            for vis, vis_plots in plots.items():
                amp_vs_uv_summary_plots[vis].extend(vis_plots)

        # Phase vs UV distance plots are not required
        # phase_vs_uv_summary_plots, _ = self.create_plots(
        #     context,
        #     result,
        #     applycal.PhaseVsUVSummaryChart,
        #     ['AMPLITUDE']
        # )

        # CAS-5970: add science target plots to the applycal page
        (science_amp_vs_freq_summary_plots,
         science_phase_vs_freq_summary_plots,
         science_amp_vs_uv_summary_plots,
         uv_max) = self.create_science_plots(context, result)

        corrected_ratio_to_antenna1_plots = utils.OrderedDefaultdict(list)
        corrected_ratio_to_uv_dist_plots = {}
        for r in result:
            vis = os.path.basename(r.inputs['vis'])
            uvrange_dist = uv_max.get(vis, None)
            in_m = str(uvrange_dist.to_units(measures.DistanceUnits.METRE))
            uvrange = '0~%sm' % in_m

            # CAS-9229: Add amp / model vs antenna id plots for other calibrators
            for intents, uv_cutoff in [(['AMPLITUDE'], uvrange),
                                       (['PHASE'], ''),
                                       (['BANDPASS'], ''),
                                       (['CHECK'], '')]:
                p, _ = self.create_plots(
                    context,
                    [r],
                    applycal.CorrectedToModelRatioVsAntenna1SummaryChart,
                    intents,
                    uvrange=uv_cutoff
                )

                for vis, vis_plots in p.items():
                    corrected_ratio_to_antenna1_plots[vis].extend(vis_plots)

            p, _ = self.create_plots(
                context,
                [r],
                applycal.CorrectedToModelRatioVsUVDistanceSummaryChart,
                ['AMPLITUDE'],
                uvrange=uvrange,
                plotrange=[0, float(in_m), 0, 0]
            )
            corrected_ratio_to_uv_dist_plots[vis] = p[vis]

        # these dicts map vis to the list of plots
        amp_vs_freq_detail_plots = {}
        phase_vs_freq_detail_plots = {}
        phase_vs_time_detail_plots = {}

        # CAS-9154 Add per-antenna amplitude vs time plots for applycal stage
        #
        # Compromise to generate some antenna-specific plots to allow
        # bad antennas to be identified while keeping the overall number of
        # plots relatively unchanged.
        #
        # Note that 'TARGET' has been removed from the intents list
        amp_vs_time_detail_plots, amp_vs_time_subpages = self.create_plots(
            context,
            result,
            applycal.CAS9154AmpVsTimeDetailChart,
            ['AMPLITUDE', 'PHASE', 'BANDPASS', 'CHECK'],
            ApplycalAmpVsTimePlotRenderer,
            avgchannel='9000'
        )

        # CAS-9216: Add per-antenna amplitude vs UV distance plots for
        # applycal stage
        amp_vs_uv_detail_plots, amp_vs_uv_subpages = self.create_plots(
            context,
            result,
            applycal.CAS9216AmpVsUVDetailChart,
            ['AMPLITUDE', 'PHASE', 'BANDPASS', 'CHECK'],
            ApplycalAmpVsUVPlotRenderer,
        )

        if pipeline.infrastructure.generate_detail_plots(result):
            # detail plots. Don't need the return dictionary, but make sure a
            # renderer is passed so the detail page is written to disk
            amp_vs_freq_detail_plots, amp_vs_freq_subpages = self.create_plots(
                context,
                result,
                applycal.AmpVsFrequencyDetailChart,
                ['BANDPASS', 'PHASE', 'CHECK', 'AMPLITUDE'],
                ApplycalAmpVsFreqPlotRenderer
            )

            phase_vs_freq_detail_plots, phase_vs_freq_subpages = self.create_plots(
                context,
                result,
                applycal.PhaseVsFrequencyDetailChart,
                ['BANDPASS', 'PHASE', 'CHECK'],
                ApplycalPhaseVsFreqPlotRenderer
            )

            # phase_vs_uv_detail_plots, phase_vs_uv_subpages = self.create_plots(
            #     context,
            #     result,
            #     applycal.PhaseVsUVDetailChart,
            #     ['AMPLITUDE'],
            #     ApplycalPhaseVsUVPlotRenderer
            # )

            phase_vs_time_detail_plots, phase_vs_time_subpages = self.create_plots(
                context,
                result,
                applycal.PhaseVsTimeDetailChart,
                ['AMPLITUDE', 'PHASE', 'BANDPASS', 'CHECK', 'TARGET'],
                ApplycalPhaseVsTimePlotRenderer
            )

        # render plots for all EBs in one page
        for d, plotter_cls, subpages in (
                (amp_vs_freq_detail_plots, ApplycalAmpVsFreqPlotRenderer, amp_vs_freq_subpages),
                (phase_vs_freq_detail_plots, ApplycalPhaseVsFreqPlotRenderer, phase_vs_freq_subpages),
                (amp_vs_uv_detail_plots, ApplycalAmpVsUVPlotRenderer, amp_vs_uv_subpages),
                # (phase_vs_uv_detail_plots, ApplycalPhaseVsUVPlotRenderer, phase_vs_uv_subpages),
                (amp_vs_time_detail_plots, ApplycalAmpVsTimePlotRenderer, amp_vs_time_subpages),
                (phase_vs_time_detail_plots, ApplycalPhaseVsTimePlotRenderer, phase_vs_time_subpages)):
            if d:
                all_plots = list(utils.flatten([v for v in d.values()]))
                renderer = plotter_cls(context, result, all_plots)
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())
                    # redirect subpage links to master page
                    for vis in subpages:
                        subpages[vis] = renderer.path

        ctx.update({
            'amp_vs_freq_plots': amp_vs_freq_summary_plots,
            'phase_vs_freq_plots': phase_vs_freq_summary_plots,
            'amp_vs_time_plots': amp_vs_time_summary_plots,
            'amp_vs_uv_plots': amp_vs_uv_summary_plots,
            'phase_vs_time_plots': phase_vs_time_summary_plots,
            'corrected_to_antenna1_plots': corrected_ratio_to_antenna1_plots,
            'corrected_to_model_vs_uvdist_plots' : corrected_ratio_to_uv_dist_plots,
            'science_amp_vs_freq_plots': science_amp_vs_freq_summary_plots,
            'science_phase_vs_freq_plots': science_phase_vs_freq_summary_plots,
            'science_amp_vs_uv_plots': science_amp_vs_uv_summary_plots,
            'uv_max': uv_max,
            'amp_vs_freq_subpages': amp_vs_freq_subpages,
            'phase_vs_freq_subpages': phase_vs_freq_subpages,
            'amp_vs_time_subpages': amp_vs_time_subpages,
            'amp_vs_uv_subpages': amp_vs_uv_subpages,
            'phase_vs_time_subpages': phase_vs_time_subpages,
            'transmission_plots': transmission_plots
        })

    def create_transmission_plots(self, context, results):
        d = {}
        for intents in [['PHASE'], ['BANDPASS'], ['CHECK'], ['AMPLITUDE']]:
            plots, _ = self.create_plots(
                context,
                results,
                plotatmosphere.TransmissionSummaryChart,
                intents
            )

            key = utils.commafy(intents, quotes=False)
            for vis, vis_plots in plots.items():
                d.setdefault(vis, {})[key] = vis_plots

        for result in results:
            vis = os.path.basename(result.inputs['vis'])
            ms = context.observing_run.get_ms(vis)

            d.setdefault(vis, {})['TARGET'] = []

            brightest_fields = T2_4MDetailsApplycalRenderer.get_brightest_fields(ms)
            for source_id, brightest_field in brightest_fields.items():
                self.result = self.science_plots_for_result(context, result, plotatmosphere.TransmissionSummaryChart,
                                                            [brightest_field.id])
                plots = self.result
                d[vis]['TARGET'].extend(plots)

        return d

    def create_science_plots(self, context, results):
        """
        Create plots for the science targets, returning two dictionaries of 
        vis:[Plots].
        """
        amp_vs_freq_summary_plots = collections.defaultdict(dict)
        phase_vs_freq_summary_plots = collections.defaultdict(dict)
        amp_vs_uv_summary_plots = collections.defaultdict(dict)
        max_uvs = collections.defaultdict(dict)

        amp_vs_freq_detail_plots = {}
        phase_vs_freq_detail_plots = {}
        amp_vs_uv_detail_plots = {}

        for result in results:
            vis = os.path.basename(result.inputs['vis'])
            ms = context.observing_run.get_ms(vis)

            amp_vs_freq_summary_plots[vis] = []
            phase_vs_freq_summary_plots[vis] = []
            amp_vs_uv_summary_plots[vis] = []

            # Plot for 1 science field (either 1 science target or for a mosaic 1
            # pointing). The science field that should be chosen is the one with
            # the brightest average amplitude over all spws

            # Ideally, the uvmax of the spectrum (plots 1 and 2)
            # would be set by the appearance of plot 3; that is, if there is
            # no obvious drop in amplitude with uvdist, then use all the data.
            # A simpler compromise would be to use a uvrange that captures the
            # inner half the data.
            baselines = sorted(ms.antenna_array.baselines,
                               key=operator.attrgetter('length'))
            # take index as midpoint + 1 so we include the midpoint in the
            # constraint
            half_baselines = baselines[0:(len(baselines)//2)+1]
            uv_max = half_baselines[-1].length.to_units(measures.DistanceUnits.METRE)
            uv_range = '<%s' % uv_max
            LOG.debug('Setting UV range to %s for %s', uv_range, vis)
            max_uvs[vis] = half_baselines[-1].length

            brightest_fields = T2_4MDetailsApplycalRenderer.get_brightest_fields(ms)        
            for source_id, brightest_field in brightest_fields.items():
                plots = self.science_plots_for_result(context,
                                                      result, 
                                                      applycal.AmpVsFrequencySummaryChart,
                                                      [brightest_field.id],
                                                      uv_range)
                amp_vs_freq_summary_plots[vis].extend(plots)
    
                plots = self.science_plots_for_result(context, 
                                                      result,
                                                      applycal.PhaseVsFrequencyPerSpwSummaryChart,
                                                      [brightest_field.id],
                                                      uv_range)
                phase_vs_freq_summary_plots[vis].extend(plots)
    
                plots = self.science_plots_for_result(context, 
                                                      result, 
                                                      applycal.AmpVsUVSummaryChart,
                                                      [brightest_field.id])
                amp_vs_uv_summary_plots[vis].extend(plots)

            if pipeline.infrastructure.generate_detail_plots(results):
                scans = ms.get_scans(scan_intent='TARGET')
                fields = set()
                for scan in scans:
                    fields.update([field.id for field in scan.fields])
                
                # Science target detail plots. Note that summary plots go onto the
                # detail pages; we don't create plots per spw or antenna
                plots = self.science_plots_for_result(context,
                                                  result,
                                                  applycal.AmpVsFrequencySummaryChart,
                                                  fields,
                                                  uv_range,
                                                  ApplycalAmpVsFreqSciencePlotRenderer)
                amp_vs_freq_detail_plots[vis] = plots

                plots = self.science_plots_for_result(context,
                                              result, 
                                              applycal.PhaseVsFrequencyPerBasebandSummaryChart,
                                              fields,
                                              uv_range,
                                              ApplycalPhaseVsFreqSciencePlotRenderer)
                phase_vs_freq_detail_plots[vis] = plots

                plots = self.science_plots_for_result(context,
                                              result, 
                                              applycal.AmpVsUVSummaryChart,
                                              fields,
                                              renderer_cls=ApplycalAmpVsUVSciencePlotRenderer)
                amp_vs_uv_detail_plots[vis] = plots

        for d, plotter_cls in (
                (amp_vs_freq_detail_plots, ApplycalAmpVsFreqSciencePlotRenderer),
                (phase_vs_freq_detail_plots, ApplycalPhaseVsFreqSciencePlotRenderer),
                (amp_vs_uv_detail_plots, ApplycalAmpVsUVSciencePlotRenderer)):
            if d:
                all_plots = list(utils.flatten([v for v in d.values()]))
                renderer = plotter_cls(context, results, all_plots)
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())

        return (amp_vs_freq_summary_plots, phase_vs_freq_summary_plots,
                amp_vs_uv_summary_plots, max_uvs)

    def science_plots_for_result(self, context, result, plotter_cls, fields, uvrange=None, renderer_cls=None):
        if plotter_cls is plotatmosphere.TransmissionSummaryChart:
            # remove warning message about coloraxis
            overrides = {}
        else:
            overrides = {'coloraxis': 'spw'}

        if uvrange is not None:
            overrides['uvrange'] = uvrange
            # CAS-9395: ALMA pipeline weblog plot of calibrated amp vs.
            # frequency with avgantenna=True and a uvrange upper limit leads
            # to misleading results and wrong conclusions
            overrides['avgantenna'] = False

        plots = []
        plot_output_dir = os.path.join(context.report_dir, 'stage%s' % result.stage_number)
        calto, _ = _get_data_selection_for_plot(context, result, ['TARGET'])

        for field in fields:
            # override field when plotting amp/phase vs frequency, as otherwise
            # the field is resolved to a list of all field IDs  
            overrides['field'] = field

            plotter = plotter_cls(context, plot_output_dir, calto, 'TARGET', **overrides)
            plots.extend(plotter.plot())

        for plot in plots:
            plot.parameters['intent'] = ['TARGET']

        if renderer_cls is not None:
            renderer = renderer_cls(context, result, plots)
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())        

        return plots

    @staticmethod
    def get_brightest_fields(ms, intent='TARGET'):
        """
        
        """
        # get IDs for all science spectral windows
        spw_ids = set()
        for scan in ms.get_scans(scan_intent=intent):
            scan_spw_ids = set([dd.spw.id for dd in scan.data_descriptions])
            spw_ids.update(scan_spw_ids)

        if intent == 'TARGET':
            science_ids = set([spw.id for spw in ms.get_spectral_windows()])
            spw_ids = spw_ids.intersection(science_ids)

        result = collections.OrderedDict()

        by_source_id = lambda field: field.source.id
        fields_by_source_id = sorted(ms.get_fields(intent=intent), 
                                     key=by_source_id)
        for source_id, source_fields in itertools.groupby(fields_by_source_id,
                                                          by_source_id):
            fields = list(source_fields)
            
            # give the sole science target name if there's only one science target
            # in this ms.
            if len(fields) is 1:
                LOG.info('Only one %s target for Source #%s. Bypassing '
                         'brightest target selection.', intent, source_id)
                result[source_id] = fields[0]
                continue
            
            field_ids = set([(f.id, f.name) for f in fields])
    
            # Switch to second field
            #field = fields[0]
            field = fields[1]
            LOG.warning('Bypassing brightest field selection due to problem '
                        'with visstat. Using Field #%s (%s) for Source #%s'
                        '', field.id, field.name, source_id)
            result[source_id] = field
            continue
    
            # holds the mapping of field name to mean flux 
            average_flux = {}
        
            # defines the parameters for the visstat job
            job_params = {
                'vis': ms.name,
                'axis': 'amp',
                'datacolumn': 'corrected',
                'spw': ','.join(map(str, spw_ids)),
            }

            # solve circular import problem by importing at run-time
            from pipeline.infrastructure import casa_tasks
        
            LOG.info('Calculating which %s field has the highest mean flux '
                     'for Source #%s', intent, source_id)
            # run visstat for each scan selection for the target
            for field_id, field_name in field_ids:
                job_params['field'] = str(field_id)
                job = casa_tasks.visstat(**job_params)
                LOG.debug('Calculating statistics for %r (#%s)', field_name, field_id)
                result = job.execute(dry_run=False)
                
                average_flux[(field_id, field_name)] = float(result['CORRECTED']['mean'])
                
            LOG.debug('Mean flux for %s targets:', intent)
            for (field_id, field_name), v in average_flux.items():
                LOG.debug('\t%r (%s): %s', field_name, field_id, v)
        
            # find the ID of the field with the highest average flux
            sorted_by_flux = sorted(average_flux.iteritems(), 
                                    key=operator.itemgetter(1),
                                    reverse=True)
            (brightest_id, brightest_name), highest_flux = sorted_by_flux[0]
            
            LOG.info('%s field %r (%s) has highest mean flux (%s)', intent,
                     brightest_name, brightest_id, highest_flux)
            result[source_id] = brightest_id

        return result

    def create_plots(self, context, results, plotter_cls, intents, renderer_cls=None, **kwargs):
        """
        Create plots and return (dictionary of vis:[Plots], dict of vis:subpage URL).
        """
        d = {}
        subpages = {}

        for result in results:
            plots, href = self.plots_for_result(context, result, plotter_cls, intents, renderer_cls, **kwargs)
            d = utils.dict_merge(d, plots)

            vis = os.path.basename(result.inputs['vis'])
            subpages[vis] = href

        return d, subpages

    def plots_for_result(self, context, result, plotter_cls, intents, renderer_cls=None, **kwargs):
        vis = os.path.basename(result.inputs['vis'])
        output_dir = os.path.join(context.report_dir, 'stage%s' % result.stage_number)
        calto, str_intents = _get_data_selection_for_plot(context, result, intents)

        plotter = plotter_cls(context, output_dir, calto, str_intents, **kwargs)
        plots = plotter.plot()
        for plot in plots:
            plot.parameters['intent'] = intents

        d = {vis: plots}

        path = None
        if renderer_cls is not None:
            renderer = renderer_cls(context, result, plots)
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                path = renderer.path

        return d, path

    def calapps_for_result(self, result):
        calapps = collections.defaultdict(list)
        for calapp in result.applied:
            vis = os.path.basename(calapp.vis)
            calapps[vis].append(calapp)
        return calapps

    def caltypes_for_result(self, result):
        type_map = {
            'bandpass': 'Bandpass',
            'gaincal': 'Gain',
            'tsys': 'T<sub>sys</sub>',
            'wvr': 'WVR',
            'ps': 'Sky',
        }
        
        d = {}
        for calapp in result.applied:
            for calfrom in calapp.calfrom:
                caltype = type_map.get(calfrom.caltype, calfrom.caltype)

                if calfrom.caltype == 'gaincal':
                    # try heuristics to detect phase-only and amp-only 
                    # solutions 
                    caltype += self.get_gain_solution_type(calfrom.gaintable)
                    
                d[calfrom.gaintable] = caltype

        return d
                
    def get_gain_solution_type(self, gaintable):
        # CAS-9835: hif_applycal() "type" descriptions are misleading /
        # incomplete in weblog table
        #
        # quick hack: match filenamer-generated file names
        #
        # TODO find a way to attach the originating task to the callibrary entries
        if gaintable.endswith('.gacal.tbl'):
            return ' (amplitude only)'
        if gaintable.endswith('.gpcal.tbl'):
            return ' (phase only)'
        if gaintable.endswith('.gcal.tbl'):
            return ''

        # resort to inspecting caltable values to infer what its type is

        # solve circular import problem by importing at run-time
        from pipeline.infrastructure import casa_tasks

        # get stats on amp solution of gaintable 
        calstat_job = casa_tasks.calstat(caltable=gaintable, axis='amp', 
                                         datacolumn='CPARAM', useflags=True)
        calstat_result = calstat_job.execute(dry_run=False)        
        stats = calstat_result['CPARAM']

        # amp solutions of unity imply phase-only was requested
        tol = 1e-3
        no_amp_soln = all([utils.approx_equal(stats['sum'], stats['npts'], tol),
                           utils.approx_equal(stats['min'], 1, tol),
                           utils.approx_equal(stats['max'], 1, tol)])

        # same again for phase solution
        calstat_job = casa_tasks.calstat(caltable=gaintable, axis='phase', 
                                         datacolumn='CPARAM', useflags=True)
        calstat_result = calstat_job.execute(dry_run=False)        
        stats = calstat_result['CPARAM']

        # phase solutions ~ 0 implies amp-only solution
        tol = 1e-5
        no_phase_soln = all([utils.approx_equal(stats['sum'], 0, tol),
                             utils.approx_equal(stats['min'], 0, tol),
                             utils.approx_equal(stats['max'], 0, tol)])

        if no_phase_soln and not no_amp_soln:
            return ' (amplitude only)'
        if no_amp_soln and not no_phase_soln:
            return ' (phase only)'
        return ''

    def flags_for_result(self, result, context):
        ms = context.observing_run.get_ms(result.inputs['vis'])
        summaries = result.summaries

        by_intent = self.flags_by_intent(ms, summaries)
        by_spw = self.flags_by_science_spws(ms, summaries)

        return {ms.basename : utils.dict_merge(by_intent, by_spw)}

    def flags_by_intent(self, ms, summaries):
        # create a dictionary of scans per observing intent, eg. 'PHASE':[1,2,7]
        intent_scans = {}
        for intent in ('BANDPASS', 'PHASE', 'AMPLITUDE', 'CHECK', 'TARGET'):
            # convert IDs to strings as they're used as summary dictionary keys
            intent_scans[intent] = [str(s.id) for s in ms.scans
                                    if intent in s.intents]

        # while we're looping, get the total flagged by looking in all scans 
        intent_scans['TOTAL'] = [str(s.id) for s in ms.scans]

        total = collections.defaultdict(dict)

        previous_summary = None
        for summary in summaries:

            for intent, scan_ids in intent_scans.items():
                flagcount = 0
                totalcount = 0
    
                for i in scan_ids:
                    # workaround for KeyError exception when summary 
                    # dictionary doesn't contain the scan
                    if i not in summary['scan']:
                        continue

                    flagcount += int(summary['scan'][i]['flagged'])
                    totalcount += int(summary['scan'][i]['total'])
        
                    if previous_summary:
                        flagcount -= int(previous_summary['scan'][i]['flagged'])
    
                ft = FlagTotal(flagcount, totalcount)
                total[summary['name']][intent] = ft
                
            previous_summary = summary
                
        return total 
    
    def flags_by_science_spws(self, ms, summaries):
        science_spws = ms.get_spectral_windows(science_windows_only=True)
    
        total = collections.defaultdict(dict)
    
        previous_summary = None
        for summary in summaries:
    
            flagcount = 0
            totalcount = 0
    
            for spw in science_spws:
                spw_id = str(spw.id)
                flagcount += int(summary['spw'][spw_id]['flagged'])
                totalcount += int(summary['spw'][spw_id]['total'])
        
                if previous_summary:
                    flagcount -= int(previous_summary['spw'][spw_id]['flagged'])

            ft = FlagTotal(flagcount, totalcount)
            total[summary['name']]['SCIENCE SPWS'] = ft
                
            previous_summary = summary
                
        return total


class ApplycalAmpVsFreqPlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, plots):
        vis = utils.get_vis_from_plots(plots)

        title = 'Calibrated amplitude vs frequency for %s' % vis
        outfile = filenamer.sanitize('amp_vs_freq-%s.html' % vis)
        
        super(ApplycalAmpVsFreqPlotRenderer, self).__init__(
                'generic_x_vs_y_field_spw_ant_detail_plots.mako', context, 
                result, plots, title, outfile)


class ApplycalPhaseVsFreqPlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, plots):
        vis = utils.get_vis_from_plots(plots)

        title = 'Calibrated phase vs frequency for %s' % vis
        outfile = filenamer.sanitize('phase_vs_freq-%s.html' % vis)
        
        super(ApplycalPhaseVsFreqPlotRenderer, self).__init__(
                'generic_x_vs_y_field_spw_ant_detail_plots.mako', context, 
                result, plots, title, outfile)


class ApplycalAmpVsFreqSciencePlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, plots):
        vis = utils.get_vis_from_plots(plots)

        title = 'Calibrated amplitude vs frequency for %s' % vis
        outfile = filenamer.sanitize('science_amp_vs_freq-%s.html' % vis)
        
        super(ApplycalAmpVsFreqSciencePlotRenderer, self).__init__(
                'generic_x_vs_y_spw_field_detail_plots.mako', context,
                result, plots, title, outfile)


class ApplycalPhaseVsFreqSciencePlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, plots):
        vis = utils.get_vis_from_plots(plots)

        title = 'Calibrated phase vs frequency for %s' % vis
        outfile = filenamer.sanitize('science_phase_vs_freq-%s.html' % vis)
        
        super(ApplycalPhaseVsFreqSciencePlotRenderer, self).__init__(
                'generic_x_vs_y_field_baseband_detail_plots.mako', context,
                result, plots, title, outfile)


class ApplycalAmpVsUVSciencePlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, plots):
        vis = utils.get_vis_from_plots(plots)

        title = 'Calibrated amplitude vs UV distance for %s' % vis
        outfile = filenamer.sanitize('science_amp_vs_uv-%s.html' % vis)
        
        super(ApplycalAmpVsUVSciencePlotRenderer, self).__init__(
                'generic_x_vs_y_spw_field_detail_plots.mako', context,
                result, plots, title, outfile)


class ApplycalAmpVsUVPlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, plots):
        vis = utils.get_vis_from_plots(plots)

        title = 'Calibrated amplitude vs UV distance for %s' % vis
        outfile = filenamer.sanitize('amp_vs_uv-%s.html' % vis)
        
        super(ApplycalAmpVsUVPlotRenderer, self).__init__(
                'generic_x_vs_y_field_spw_ant_detail_plots.mako', context,
                result, plots, title, outfile)


class ApplycalPhaseVsUVPlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, plots):
        vis = utils.get_vis_from_plots(plots)

        title = 'Calibrated phase vs UV distance for %s' % vis
        outfile = filenamer.sanitize('phase_vs_uv-%s.html' % vis)
        
        super(ApplycalPhaseVsUVPlotRenderer, self).__init__(
                'generic_x_vs_y_spw_ant_plots.mako', context, 
                result, plots, title, outfile)


class ApplycalAmpVsTimePlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, plots):
        vis = utils.get_vis_from_plots(plots)

        title = 'Calibrated amplitude vs times for %s' % vis
        outfile = filenamer.sanitize('amp_vs_time-%s.html' % vis)
        
        super(ApplycalAmpVsTimePlotRenderer, self).__init__(
                'generic_x_vs_y_spw_ant_plots.mako', context,
                result, plots, title, outfile)


class ApplycalPhaseVsTimePlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, plots):
        vis = utils.get_vis_from_plots(plots)

        title = 'Calibrated phase vs times for %s' % vis
        outfile = filenamer.sanitize('phase_vs_time-%s.html' % vis)
        
        super(ApplycalPhaseVsTimePlotRenderer, self).__init__(
                'generic_x_vs_y_field_spw_ant_detail_plots.mako', context, 
                result, plots, title, outfile)


def _get_data_selection_for_plot(context, result, intent):
    """
    Inspect a result, returning a CalTo that matches the data selection of the
    applied calibration.

    Background: we don't want to create plots for an entire MS, only the data
    selection of interest. Rather than calculate and explicitly pass in the
    data selection of interest, this function calculates the data selection of
    interest by inspecting the results and extracting the data selection that 
    the calibration is applied to.

    :param context: pipeline Context
    :param result: a Result with an .applied property containing CalApplications
    :param intent: pipeline intent
    :return: 
    """
    spw = _get_calapp_arg(result, 'spw')
    field = _get_calapp_arg(result, 'field')
    antenna = _get_calapp_arg(result, 'antenna')
    intent = ','.join(intent).upper()

    vis = {calapp.vis for calapp in result.applied}
    assert (len(vis) is 1)
    vis = vis.pop()

    wanted = set(intent.split(','))
    fields_with_intent = set()
    for f in context.observing_run.get_ms(vis).get_fields(field):
        intersection = f.intents.intersection(wanted)
        if not intersection:
            continue
        fields_with_intent.add(f.name)
    field = ','.join(fields_with_intent)

    calto = callibrary.CalTo(vis, field, spw, antenna, intent)

    return calto, intent


def _get_calapp_arg(result, arg):
    s = set()
    for calapp in result.applied:
        s.update(utils.safe_split(getattr(calapp, arg)))
    return ','.join(s)
