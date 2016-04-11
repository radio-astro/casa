'''
Created on 24 Oct 2014

@author: sjw
'''
import collections
import itertools
import operator
import os

import pipeline.domain.measures as measures
import pipeline.infrastructure
import pipeline.infrastructure.displays.applycal as applycal
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils
from pipeline.hif.tasks.applycal.renderer import *

LOG = logging.get_logger(__name__)

FlagTotal = collections.namedtuple('FlagSummary', 'flagged total')


class T2_4MDetailsSDApplycalRenderer(T2_4MDetailsApplycalRenderer,
                                     basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='applycal.mako', 
                 description='Apply calibrations from context',
                 always_rerender=False):
        super(T2_4MDetailsSDApplycalRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, ctx, context, result):
        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % result.stage_number)

        flag_totals = {}
        for r in result:
            if r.inputs['flagsum'] == True:
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

        # # these dicts map vis to the hrefs of the detail pages
        # amp_vs_time_subpages = {}

        # amp_vs_time_summary_plots, amp_vs_time_subpages = self.create_plots(
        #     context,
        #     result,
        #     applycal.AmpVsTimeSummaryChart,
        #     ['TARGET']
        # )

        # CAS-5970: add science target plots to the applycal page
        (science_amp_vs_freq_summary_plots, uv_max) = self.create_science_plots(context, result)

        # # these dicts map vis to the list of plots
        # amp_vs_time_detail_plots = {}

        # if pipeline.infrastructure.generate_detail_plots(result):
        #     # detail plots. Don't need the return dictionary, but make sure a
        #     # renderer is passed so the detail page is written to disk

        #     amp_vs_time_detail_plots, amp_vs_time_subpages = self.create_plots(
        #         context,
        #         result,
        #         applycal.AmpVsTimeDetailChart,
        #         ['TARGET'],
        #         ApplycalAmpVsTimePlotRenderer
        #     )

        # # render plots for all EBs in one page
        # for d, plotter_cls, subpages in (
        #         (amp_vs_time_detail_plots, ApplycalAmpVsTimePlotRenderer, amp_vs_time_subpages),):
        #     if d:
        #         all_plots = list(utils.flatten([v for v in d.values()]))
        #         renderer = plotter_cls(context, result, all_plots)
        #         with renderer.get_file() as fileobj:
        #             fileobj.write(renderer.render())
        #             # redirect subpage links to master page
        #             for vis in subpages:
        #                 subpages[vis] = renderer.path

        ctx.update({
#            'amp_vs_time_plots': amp_vs_time_summary_plots,
            'science_amp_vs_freq_plots': science_amp_vs_freq_summary_plots,
            'uv_max': uv_max,
#            'amp_vs_time_subpages': amp_vs_time_subpages,
        })
        
    def create_science_plots(self, context, results):
        """
        Create plots for the science targets, returning two dictionaries of 
        vis:[Plots].
        """
        amp_vs_freq_summary_plots = collections.defaultdict(dict)
        max_uvs = collections.defaultdict(dict)

        amp_vs_freq_detail_plots = {}

        for result in results:
            vis = os.path.basename(result.inputs['vis'])
            ms = context.observing_run.get_ms(vis)
            correlation = ms.get_alma_corrstring()

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
                                                      uv_range, correlation=correlation)
                amp_vs_freq_summary_plots[vis][source_id] = plots
    
            if pipeline.infrastructure.generate_detail_plots(result):
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
                                                  ApplycalAmpVsFreqSciencePlotRenderer,
                                                  correlation=correlation)
                amp_vs_freq_detail_plots[vis] = plots

        for d, plotter_cls in (
                (amp_vs_freq_detail_plots, ApplycalAmpVsFreqSciencePlotRenderer),):
            if d:
                all_plots = list(utils.flatten([v for v in d.values()]))
                renderer = plotter_cls(context, results, all_plots)
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())

        # sort plots by baseband so that the summary plots appear in baseband order
        for plot_dict in (amp_vs_freq_summary_plots,):
            for source_plots in plot_dict.values():
                self.sort_plots_by_baseband(source_plots)

        return (amp_vs_freq_summary_plots, max_uvs)

