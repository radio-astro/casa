"""
Created on 24 Oct 2014

@author: sjw
"""
import pipeline.infrastructure
import pipeline.infrastructure.casatools as casatools
from pipeline.h.tasks.applycal.renderer import *
from pipeline.h.tasks.common.displays import applycal as applycal

LOG = logging.get_logger(__name__)

FlagTotal = collections.namedtuple('FlagSummary', 'flagged total')


class T2_4MDetailsSDApplycalRenderer(T2_4MDetailsApplycalRenderer,
                                     basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='applycal.mako', 
                 description='Apply calibrations from context',
                 always_rerender=False):
        super(T2_4MDetailsSDApplycalRenderer, self).__init__(
            uri=uri, description=description, always_rerender=always_rerender)

    def update_mako_context(self, ctx, context, result):
        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % result.stage_number)

        flag_totals = {}
        for r in result:
            if r.inputs['flagsum'] == True:
                flag_totals = utils.dict_merge(flag_totals, self.flags_for_result(r, context))

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

        # CAS-5970: add science target plots to the applycal page
        (science_amp_vs_freq_summary_plots, uv_max) = self.create_single_dish_science_plots(context, result)

        ctx.update({
            'science_amp_vs_freq_plots': science_amp_vs_freq_summary_plots,
            'uv_max': uv_max,
        })
        
    def create_single_dish_science_plots(self, context, results):
        """
        Create plots for the science targets, returning two dictionaries of 
        vis:[Plots].
        MODIFIED for single dish
        """
        amp_vs_freq_summary_plots = collections.defaultdict(dict)
        max_uvs = collections.defaultdict(dict)

        amp_vs_freq_detail_plots = {}

        for result in results:
            vis = os.path.basename(result.inputs['vis'])
            ms = context.observing_run.get_ms(vis)
            max_uvs[vis] = measures.Distance(value=0.0, units=measures.DistanceUnits.METRE)
            
            amp_vs_freq_summary_plots[vis] = []

            # Plot for 1 science field (either 1 science target or for a mosaic 1
            # pointing). The science field that should be chosen is the one with
            # the brightest average amplitude over all spws

            brightest_fields = T2_4MDetailsApplycalRenderer.get_brightest_fields(ms)
            for source_id, brightest_field in brightest_fields.iteritems():
                plots = self.science_plots_for_result(context,
                                                      result,
                                                      applycal.RealVsFrequencySummaryChart,
                                                      [brightest_field.id], None)
                amp_vs_freq_summary_plots[vis].extend(plots)
    
            if pipeline.infrastructure.generate_detail_plots(result):
                fields = set()
                # scans = ms.get_scans(scan_intent='TARGET')
                # for scan in scans:
                #     fields.update([field.id for field in scan.fields])
                with casatools.MSMDReader(vis) as msmd:
                    fields.update(list(msmd.fieldsforintent("OBSERVE_TARGET#ON_SOURCE")))
                
                # Science target detail plots. Note that summary plots go onto the
                # detail pages; we don't create plots per spw or antenna
                plots = self.science_plots_for_result(context,
                                                      result,
                                                      applycal.RealVsFrequencySummaryChart,
                                                      fields, None,
                                                      ApplycalAmpVsFreqSciencePlotRenderer)
                amp_vs_freq_detail_plots[vis] = plots

        for d, plotter_cls in (
                (amp_vs_freq_detail_plots, ApplycalAmpVsFreqSciencePlotRenderer),):
            if d:
                all_plots = list(utils.flatten([v for v in d.values()]))
                renderer = plotter_cls(context, results, all_plots)
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())

        return amp_vs_freq_summary_plots, max_uvs
