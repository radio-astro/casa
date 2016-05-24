"""
Created on 10 Sep 2014

@author: sjw
"""
import collections
import os

import pipeline.infrastructure.displays.image as image
import pipeline.infrastructure.displays.wvr as wvr
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils

LOG = logging.get_logger(__name__)

WvrApplication = collections.namedtuple('WvrApplication', 
                                        'ms gaintable interpolated applied') 


class T2_4MDetailsWvrgcalflagRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    """
    T2_4MDetailsWvrgcalflagRenderer generates the detailed T2_4M-level plots
    and output specific to the wvrgcalflag task.
    """
    def __init__(self, uri='wvrgcalflag.mako',
                 description='Calculate and flag WVR calibration',
                 always_rerender=False):
        super(T2_4MDetailsWvrgcalflagRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    """
    Update the Mako context appropriate to the results created by a Wvrgcalflag
    task.
    
    :param context: the pipeline Context
    :type context: :class:`~pipeline.infrastructure.launcher.Context`
    :param results: the bandpass results to describe
    :type results: 
        :class:`~pipeline.infrastructure.tasks.wvrgcalflag.resultobjects.WvrgcalflagResults`
    :rtype a dictionary that can be passed to the matching Mako template
    """
    def update_mako_context(self, ctx, context, results):
        plots_dir = os.path.join(context.report_dir, 
                                 'stage%d' % results.stage_number)
        if not os.path.exists(plots_dir):
            os.mkdir(plots_dir)

        applications = []
        # CAS-8265: multi-ms in time order in all weblog stages
        # Maintain the time-order of the input results by using an OrderedDict
        flag_plots = collections.OrderedDict()
        metric_plots = collections.OrderedDict()
        phase_offset_summary_plots = collections.OrderedDict()
        phase_offset_subpages = collections.OrderedDict()
        phase_offset_detail_plots = collections.OrderedDict()
        baseline_summary_plots = collections.OrderedDict()
        baseline_summary_subpages = collections.OrderedDict()
        baseline_detail_plots = collections.OrderedDict()
        wvrinfos = collections.OrderedDict()
        ms_non12 = []
        for result in results:
            vis = os.path.basename(result.inputs['vis'])
            ms = context.observing_run.get_ms(vis)

            # if there's no WVR data, the pool will be empty
            if not result.pool:
                # check if this MS is all 7m data. 
                if all([a for a in ms.antennas if a.diameter != 12.0]):
                    ms_non12.append(os.path.basename(vis))
                continue

            applications.extend(self.get_wvr_applications(result))
            try:
                wvrinfos[vis] = result.wvr_infos
            except:
                pass

            # collect flagging metric plots from result
            if result.qa_wvr.view:
                plotter = image.ImageDisplay()
                plots = plotter.plot(context, result.qa_wvr, reportdir=plots_dir, 
                                     prefix='qa', change='WVR')
                plots = sorted(plots, key=lambda p: int(p.parameters['spw']))
                
                # render flagging metric plots to their own HTML page
                renderer = WvrcalflagMetricPlotsRenderer(context, result, plots)
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())        

                metric_plots[vis] = plots[0]

            if result.view:
                flag_plotter = image.ImageDisplay()
                plots = flag_plotter.plot(context, result, reportdir=plots_dir, 
                                          prefix='flag', change='Flagging')
                # sort plots by spw
                flag_plots[vis] = sorted(plots, 
                                         key=lambda p: int(p.parameters['spw']))

            if result.nowvr_result:
                # generate the phase offset summary plots
                phase_offset_summary_plotter = wvr.WVRPhaseOffsetSummaryPlot(context, result)
                phase_offset_summary_plots[vis] = phase_offset_summary_plotter.plot()
                
                # generate the per-antenna phase offset plots
                phase_offset_plotter = wvr.WVRPhaseOffsetPlot(context, result)
                phase_offset_detail_plots[vis] = phase_offset_plotter.plot()
                # write the html for each MS to disk
                renderer = WvrgcalflagPhaseOffsetPlotRenderer(context, result, 
                                                              phase_offset_detail_plots[vis])
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())        
                    # the filename is sanitised - the MS name is not. We need to
                    # map MS to sanitised filename for link construction.
                    phase_offset_subpages[vis] = renderer.path

                baseline_plotter = wvr.WVRPhaseVsBaselineChart(context, result)
                baseline_detail_plots[vis] = baseline_plotter.plot()
                # write the html for each MS to disk
                renderer = WvrgcalflagPhaseOffsetVsBaselinePlotRenderer(context, result, 
                                                                        baseline_detail_plots[vis])
                
                # get the first scan for the QA intent(s)
                qa_intent = set(result.inputs['qa_intent'].split(','))
                qa_scan = sorted([scan.id for scan in ms.scans 
                                   if not qa_intent.isdisjoint(scan.intents)])[0]                               
                # scan parameter on plot is comma-separated string 
                qa_scan = str(qa_scan)            
                LOG.trace('Using scan %s for phase vs baseline summary '
                          'plots' % qa_scan)
                baseline_summary_plots[vis] = [p for p in baseline_detail_plots[vis]
                                               if qa_scan in set(p.parameters['scan'].split(','))]
                
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())        
                    # the filename is sanitised - the MS name is not. We need to
                    # map MS to sanitised filename for link construction.
                    baseline_summary_subpages[vis] = renderer.path

        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % results.stage_number)

        # render plots for all EBs in one page
        for d, plotter_cls, subpages in (
                (phase_offset_detail_plots, WvrgcalflagPhaseOffsetPlotRenderer, phase_offset_subpages),
                (baseline_detail_plots, WvrgcalflagPhaseOffsetVsBaselinePlotRenderer, baseline_summary_subpages)):
            if d:
                all_plots = list(utils.flatten([v for v in d.values()]))
                renderer = plotter_cls(context, results, all_plots)
                with renderer.get_file() as fileobj:
                    fileobj.write(renderer.render())
                    # redirect the subpages to the master page
                    for vis in subpages:
                        subpages[vis] = renderer.path

        # add the PlotGroups to the Mako context. The Mako template will parse
        # these objects in order to create links to the thumbnail pages we
        # just created
        ctx.update({
            'applications': applications,
            'wvrinfos': wvrinfos,
            'flag_plots': flag_plots,
            'metric_plots': metric_plots,
            'phase_offset_summary_plots': phase_offset_summary_plots,
            'phase_offset_subpages': phase_offset_subpages,
            'baseline_summary_plots': baseline_summary_plots,
            'baseline_summary_subpages': baseline_summary_subpages,
            'dirname': weblog_dir
        })

        # tell the user not to panic for non-12m dishes missing WVR 
        if ms_non12:
            msg = ('WVR data are not expected for %s, which %s not observed '
                   'using 12m antennas.'
                   '' % (utils.commafy(ms_non12, quotes=False, conjunction='or'),
                         'was' if len(ms_non12) is 1 else 'were'))
            ctx['alerts_info'] = [msg,]

        # Phase vs time for the overview plot should be for the widest window
        # at the highest frequency
#         spws = sorted(ms.spectral_windows, 
#                       key=operator.attrgetter('bandwidth', 'centre_frequency'))
#         overview_spw = spws[-1]

    def get_wvr_applications(self, result):
        applications = []

        interpolated = utils.commafy(result.wvrflag, False)

        # define a closure that adds a wvrapplication for each calapplication
        # unless the applications list already contains one for that 
        # ms/caltable combination
        def collect(calapps, accept):
            for calapp in calapps:
                ms = os.path.basename(calapp.vis)
                gaintable = os.path.basename(calapp.gaintable)
    
                a = WvrApplication(ms, gaintable, interpolated, accept)            
    
                if not any([r for r in applications 
                            if r.ms == ms and r.gaintable == gaintable]):
                    applications.append(a)

        collect(result.final, True)
        collect(result.pool, False)

        return applications


class WvrcalflagMetricPlotsRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, plots):
        vis = os.path.basename(result.inputs['vis'])
        title = 'WVR flagging metric view for %s' % vis
        outfile = filenamer.sanitize('flagging_metric-%s.html' % vis)

        super(WvrcalflagMetricPlotsRenderer, self).__init__(
                'generic_x_vs_y_spw_intent_plots.mako', 
                context, result, plots, title, outfile)

    def update_json_dict(self, d, plot):
        d['intent'] = plot.parameters['intent']


class WvrgcalflagPhaseOffsetPlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, plots):
        vis = utils.get_vis_from_plots(plots)

        title = 'WVR Phase Offset Plots for %s' % vis
        outfile = filenamer.sanitize('phase_offsets-%s.html' % vis)

        # HACK! ant parameter is a list, but for these plots we know there's
        # only one plot.
        for plot in [p for p in plots if isinstance(p.parameters['ant'], list)]:
            plot.parameters['ant'] = plot.parameters['ant'][0]

        super(WvrgcalflagPhaseOffsetPlotRenderer, self).__init__(
                'wvrgcalflag_phase_offset_plots.mako', 
                context, result, plots, title, outfile)
         
    def update_json_dict(self, d, plot):
        if plot.qa_score <= 0.0:
            d['ratio'] = 0.0
        else:
            d['ratio'] = 1.0 / plot.qa_score
        plot.score = d['ratio']


class WvrgcalflagPhaseOffsetVsBaselinePlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, plots):
        vis = utils.get_vis_from_plots(plots)

        title = 'Phase offset vs average baseline for %s' % vis
        outfile = filenamer.sanitize('phase_offsets_vs_baseline-%s.html' % vis)
        
        super(WvrgcalflagPhaseOffsetVsBaselinePlotRenderer, self).__init__(
                'generic_x_vs_y_spw_scan_plots.mako', context, 
                result, plots, title, outfile)
