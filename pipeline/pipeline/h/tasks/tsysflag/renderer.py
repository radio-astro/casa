"""
Created on 9 Sep 2014

@author: sjw
"""
import collections
import functools
import os
import shutil

import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.rendererutils as rendererutils
import pipeline.infrastructure.utils as utils
from pipeline.h.tasks.common import calibrationtableaccess as caltableaccess
from pipeline.h.tasks.common.displays import image as image
from pipeline.h.tasks.common.displays import slice as slice_display
from pipeline.h.tasks.common.displays import tsys as tsys
from pipeline.h.tasks.tsyscal import renderer as tsyscalrenderer

LOG = logging.get_logger(__name__)

FlagTotal = collections.namedtuple('FlagSummary', 'flagged total')


class T2_4MDetailsTsysflagRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    """
    Renders detailed HTML output for the Tsysflag task.
    """
    def __init__(self, uri='tsysflag.mako',
                 description='Flag Tsys calibration',
                 always_rerender=False):
        super(T2_4MDetailsTsysflagRenderer, self).__init__(
            uri=uri, description=description, always_rerender=always_rerender)

    def update_mako_context(self, ctx, context, results):
        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % results.stage_number)
        
        # Initialize items that are to be exported to the
        # mako context
        task_incomplete_msg = {}
        flag_totals = collections.defaultdict(dict)
        summary_plots = {}
        subpages = {}
        eb_plots = []
        last_results = []
        updated_refants = {}
        flagcmd_files = {}

        standard_plots = create_plot_detail_page(std_renderer_mapping, context, results)
        extra_plots = create_plot_detail_page(extra_renderer_mapping, context, results)

        # For each result in the results list...
        for result in results:
            
            # If the result is marked as from a Tsysflag task that ended
            # prematurely, then store the reason to be passed to mako context.
            if result.task_incomplete_reason:
                task_incomplete_msg[result.inputs['vis']] = result.task_incomplete_reason
                continue
                
            # Otherwise, continue with generating all necessary reports

            # Create flag totals from the summaries:
            table = os.path.basename(result.inputs['caltable'])

            # summarise flag state on entry
            flag_totals[table]['before'] = self._flags_for_result(result, context, summary='first')

            # summarise flagging by each step
            for component, r in result.components.items():
                if r is not None:
                    flag_totals[table][component] = self._flags_for_result(r, context)
                else:
                    flag_totals[table][component] = None

            # summarise flag state on exit
            flag_totals[table]['after'] = self._flags_for_result(result, context, summary='last')

            # If a manual flagging template file was applied, copy the file to
            # the weblog directory, and store information.
            if 'manual' in result.components and os.path.exists(result.inputs['filetemplate']):
                shutil.copy(result.inputs['filetemplate'], weblog_dir)
                flagcmd_file = os.path.basename(result.inputs['filetemplate'])
                flagcmd_path = os.path.join('stage{}'.format(result.stage_number), flagcmd_file)
                flagcmd_files[os.path.basename(result.components['manual'].table)] = flagcmd_path
            else:
                flagcmd_files[table] = None

            # Generate the summary plots at end of flagging sequence,
            # beware empty sequence
            lastflag = result.components.keys()
            if lastflag:
                lastflag = lastflag[-1]
            lastresult = result.components[lastflag]

            plotter = tsys.TsysSummaryChart(context, lastresult)
            plots = plotter.plot()
            vis = os.path.basename(lastresult.inputs['vis'])
            summary_plots[vis] = plots

            # generate per-antenna plots
            plotter = tsys.TsysPerAntennaChart(context, lastresult)
            per_antenna_plots = plotter.plot()

            renderer = tsyscalrenderer.TsyscalPlotRenderer(context,
                                                           lastresult,
                                                           per_antenna_plots)
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                # the filename is sanitised - the MS name is not. We need to
                # map MS to sanitised filename for link construction.
                subpages[vis] = renderer.path

            eb_plots.extend(per_antenna_plots)
            last_results.append(lastresult)

            # If the reference antenna list was updated, retrieve new refant
            # list.
            if result.refants_to_remove or result.refants_to_demote:
                ms = context.observing_run.get_ms(name=vis)
                updated_refants[vis] = ms.reference_antenna

        # If there were any valid results, then additionally render plots
        # for all EBs in one page
        if last_results:
            renderer = tsyscalrenderer.TsyscalPlotRenderer(context, last_results, eb_plots)
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                # .. and we want the subpage links to go to this master page
                for vis in subpages:
                    subpages[vis] = renderer.path

        # Retrieve the metric_order from the result.
        # NOTE: metric_order is assumed to be the same
        # for all results in a ResultsList.
        components = results[0].metric_order

        # Generate the HTML reports
        htmlreports = self._get_htmlreports(context, results, components)

        # Update the mako context.
        ctx.update({
            'components': components,
            'dirname': weblog_dir,
            'extraplots': extra_plots,
            'flagcmd_files': flagcmd_files,
            'flags': flag_totals,
            'htmlreports': htmlreports,
            'stdplots': standard_plots,
            'summary_plots': summary_plots,
            'summary_subpage': subpages,
            'task_incomplete_msg': task_incomplete_msg,
            'updated_refants': updated_refants
        })

    def _get_htmlreports(self, context, results, components):
        report_dir = context.report_dir
        weblog_dir = os.path.join(report_dir,
                                  'stage%s' % results.stage_number)

        htmlreports = {}

        for component in components:
            # Skip report for manual flagging.
            if component == 'manual':
                continue

            htmlreports[component] = {}

            for msresult in results:
                if not msresult.task_incomplete_reason:
                    flagcmd_abspath = self._write_flagcmd_to_disk(weblog_dir, msresult.components[component], component)
                    report_abspath = self._write_report_to_disk(weblog_dir, msresult.components[component], component)

                    flagcmd_relpath = os.path.relpath(flagcmd_abspath, report_dir)
                    report_relpath = os.path.relpath(report_abspath, report_dir)

                    table_basename = os.path.basename(msresult.components[component].table)
                    htmlreports[component][table_basename] = (flagcmd_relpath, report_relpath)

        return htmlreports

    @staticmethod
    def _write_flagcmd_to_disk(weblog_dir, result, component=''):
        tablename = os.path.basename(result.table)
        filename = os.path.join(
            weblog_dir, '%s%s-flag_commands.txt' % (tablename, component))

        flagcmds = [l.flagcmd for l in result.flagcmds()]
        with open(filename, 'w') as flagfile:
            flagfile.writelines(['# Flag commands for %s\n#\n' % tablename])
            flagfile.writelines(['%s\n' % cmd for cmd in flagcmds])
            if not flagcmds:
                flagfile.writelines(['# No flag commands generated\n'])

        return filename

    @staticmethod
    def _write_report_to_disk(weblog_dir, result, component=''):
        # now write printTsysFlags output to a report file
        tablename = os.path.basename(result.table)
        filename = os.path.join(weblog_dir,
                                '%s%s.report.html' % (tablename, component))
        if os.path.exists(filename):
            return filename
        
        rendererutils.printTsysFlags(result.table, filename)
        return filename

    def _flags_for_result(self, result, context, summary=None):
        name = result.caltable
        tsystable = caltableaccess.CalibrationTableDataFiller.getcal(name)
        ms = context.observing_run.get_ms(name=tsystable.vis) 

        summaries = result.summaries
        if summary == 'first':
            # select only first summary, but keep as list
            summaries = summaries[:1]
        elif summary == 'last':
            # select only last summary, but keep as list
            summaries = summaries[-1:]

        by_intent = self._flags_by_intent(ms, summaries)
        by_spw = self._flags_by_spws(summaries)

        return utils.dict_merge(by_intent, by_spw)

    @staticmethod
    def _flags_by_intent(ms, summaries):
        # create a dictionary of fields per observing intent, eg. 'PHASE':['3C273']
        intent_fields = {}
        for intent in ('BANDPASS', 'PHASE', 'AMPLITUDE', 'TARGET'):
            # use _name from field as we do want the raw name here as used
            # in the summaries dict (not sometimes enclosed in "..."). Better
            # perhaps to fix the summaries dict.
            intent_fields[intent] = [f._name for f in ms.fields
                                     if intent in f.intents]

        # while we're looping, get the total flagged by looking in all scans 
        intent_fields['TOTAL'] = [f._name for f in ms.fields]

        total = collections.defaultdict(dict)

        previous_summary = None
        for summary in summaries:

            for intent, fields in intent_fields.items():
                flagcount = 0
                totalcount = 0
    
                for field in fields:
                    if field in summary['field'].keys():
                        flagcount += int(summary['field'][field]['flagged'])
                        totalcount += int(summary['field'][field]['total'])
        
                    if previous_summary:
                        if field in previous_summary['field'].keys():
                            flagcount -= int(previous_summary['field'][field]['flagged'])

                ft = FlagTotal(flagcount, totalcount)
                # The individual summaries may have been named differently from 
                # each other, but the renderer will expect a single summary, so 
                # consolidate summaries into a single summary named "Summary"
                total['Summary'][intent] = ft
    
            previous_summary = summary
                
        return total 
    
    @staticmethod
    def _flags_by_spws(summaries):
        total = collections.defaultdict(dict)
    
        previous_summary = None
        for summary in summaries:
            tsys_spws = summary['spw'].keys()
    
            flagcount = 0
            totalcount = 0
    
            for spw in tsys_spws:
                try:
                    flagcount += int(summary['spw'][spw]['flagged'])
                    totalcount += int(summary['spw'][spw]['total'])
                except:
                    pass

                if previous_summary:
                    flagcount -= int(previous_summary['spw'][spw]['flagged'])

            ft = FlagTotal(flagcount, totalcount)
            total[summary['name']]['TSYS SPWS'] = ft

            previous_summary = summary
                
        return total


def plot_slice_displays(name_of_metric, context, result_for_metric, reportdir):
    """
    Create Slice plots for the given Tsys metric.

    :param name_of_metric: name of Tsys flagging metric
    :param context: pipeline context
    :param result_for_metric: result for Tsys flagging metric
    :param reportdir: destination directory for plots
    :return: list of Plot objects
    """
    plotter = slice_display.SliceDisplay()
    plots = plotter.plot(context, result_for_metric, reportdir=reportdir, plotbad=False,
                         plot_only_flagged=True, prefix=name_of_metric)

    ensure_vis_in_plot_metadata(plots, result_for_metric.inputs['vis'])

    return plots


def plot_image_displays(name_of_metric, context, result_for_metric, reportdir):
    plotter = image.ImageDisplay()
    plots = plotter.plot(context, result_for_metric, reportdir=reportdir, prefix=name_of_metric)

    ensure_vis_in_plot_metadata(plots, result_for_metric.inputs['vis'])

    return plots


def plot_tsys_spectra(name_of_metric, context, result_for_metric, reportdir):
    plots = []

    # plot Tsys spectra that were flagged
    for flagcmd in result_for_metric.flagcmds():
        for description in result_for_metric.descriptions():
            tsysspectra = result_for_metric.first(description).children.get('tsysspectra')
            if tsysspectra is None:
                continue

            for tsys_desc in tsysspectra.descriptions():
                tsysspectrum = tsysspectra.first(tsys_desc)

                if not flagcmd.match(tsysspectrum):
                    continue

                # have found flagged spectrum, now get the associated median spectrum
                medians = result_for_metric.last(description).children.get('tsysmedians')

                for median_desc in medians.descriptions():
                    median_spectrum = medians.first(median_desc)
                    if median_spectrum.ant is None or median_spectrum.ant[0] == tsysspectrum.ant[0]:
                        # do the plot
                        plotter = slice_display.SliceDisplay()
                        plots.extend(plotter.plot(
                            context=context, results=tsysspectra,
                            description_to_plot=tsys_desc,
                            overplot_spectrum=median_spectrum,
                            reportdir=reportdir, plotbad=False))
                        break

    ensure_vis_in_plot_metadata(plots, result_for_metric.inputs['vis'])
    convert_field_id_to_field_names(plots, context)

    return plots


def ensure_vis_in_plot_metadata(plots, vis):
    """
    Update Plot parameters with the given vis metadata if required.

    :param plots: list of Plots to update
    :param vis: the vis value to set
    :return:
    """
    vis = os.path.basename(vis)
    for plot in plots:
        if 'vis' not in plot.parameters:
            plot.parameters['vis'] = vis


def convert_field_id_to_field_names(plots, context):
    """
    Convert field IDs in the Plot to field names where possible.

    :param plots: list of Plot objects to process
    :param context: pipeline context
    :return: Plot objects with field IDs replaced with field names
    """
    ms_cache = {}
    for plot in plots:
        try:
            vis = plot.parameters['vis']
            if vis in ms_cache:
                ms = ms_cache[vis]
            else:
                ms = context.observing_run.get_ms(vis)
                ms_cache[vis] = ms
            field_name, field_id = [(f.name, f.id) for f in ms.get_fields(int(plot.parameters['field']))][0]
            plot.parameters['field'] = '{!s} (#{!s})'.format(field_name, field_id)
        except:
            continue


def create_plot_detail_page(metric_mapping, context, results_list):
    """
    Create and render any detail plots required for the Tsys result.

    :param metric_mapping: a dict mapping Tsys metric to ComponentRendererMap objects
    :param context: pipeline context
    :param results_list: the Tsysflag results
    :return:
    """
    # find the flagging metrics present in the results list for which we have a mapping
    metrics_to_process = frozenset([metric_name
                                    for ms_result in results_list
                                    for metric_name in ms_result.components
                                    if metric_name in metric_mapping])

    # this will hold the result dictionary - a map of flagging metric to the
    # path of the HTML plot details page for that metric
    d = {}

    # for each metric to process..
    for metric in metrics_to_process:
        # . get the plotting function and Mako template for the metric..
        plot_fn = metric_mapping[metric].plot_fn
        template = metric_mapping[metric].template

        LOG.trace('Generating plots for %s metric', metric)
        # .. then use the plotting function to generate the plots..
        plots = get_plots(plot_fn, context, results_list, metric)
        if not plots:
            LOG.trace('No plots generated for %s metric', metric)
            continue

        # .. and render the Mako template for the thumbnails page
        renderer = PlotDetailRenderer(template, context, results_list, plots, metric)
        with renderer.get_file() as fileobj:
            LOG.trace('Rendering Mako for %s metric to %s', metric, renderer.path)
            fileobj.write(renderer.render())

        # inspect the plots to determine which MSes we generated plots for
        d[metric] = {}
        for vis in frozenset([plot.parameters['vis'] for plot in plots]):
            d[metric][vis] = (renderer.path, renderer.shorttitle)

    return d


def get_plots(plot_fn, context, results_list, name_of_metric):
    """
    Generate detail plots for a Tsys flagging metric, returning a list of Plot
    objects.

    :param plot_fn: plot generating function that returns Plot objects
    :param context: pipeline context
    :param results_list: list of results for Tsys
    :param name_of_metric: name of Tsys metric
    :return: list of Plot objects
    """
    stage = 'stage%s' % results_list.stage_number
    reportdir = os.path.join(context.report_dir, stage)

    plots = []
    for ms_result in results_list:
        # not all MSes may have a result for this metric
        if name_of_metric not in ms_result.components:
            continue

        # we can only generate plots if the metric generated a flagging view
        component_result = ms_result.components[name_of_metric]
        if not component_result.view:
            continue

        component_plots = plot_fn(context, component_result, reportdir)
        plots.extend(component_plots)

    return plots


class PlotDetailRenderer(basetemplates.JsonPlotRenderer):
    """
    Standard Mako renderer used to render all Tsys plot detail pages.
    """
    def __init__(self, template_uri, context, results, plots, component):
        vis = utils.get_vis_from_plots(plots)
        x_axis = plots[0].x_axis
        y_axis = plots[0].y_axis

        outfile = filenamer.sanitize('%s_vs_%s-%s-%s.html' % (y_axis, x_axis, vis, component))

        y_axis = y_axis.replace('Tsys', 'T<sub>sys</sub>')
        title = '%s vs %s for %s' % (y_axis, x_axis, vis)
        self.shorttitle = '%s vs %s' % (y_axis, x_axis)

        super(PlotDetailRenderer, self).__init__(template_uri, context, results, plots, title, outfile)

    def update_json_dict(self, d, plot):
        if 'intent' in plot.parameters:
            d['intent'] = plot.parameters['intent']


# struct to hold a plotting function and the Mako template used to render the
# results of that function
ComponentRendererMap = collections.namedtuple('ComponentRendererMap', 'plot_fn template')


# hold a map of Tsys flagging metric to generator of standard plots
std_renderer_mapping = {
    'birdies': ComponentRendererMap(
        functools.partial(plot_slice_displays, 'birdies'),
        'generic_x_vs_y_spw_ant_plots.mako'),
    'derivative': ComponentRendererMap(
        functools.partial(plot_image_displays, 'derivative'),
        'generic_x_vs_y_per_spw_pol_and_field_plots.mako'),
    'edgechans': ComponentRendererMap(
        functools.partial(plot_slice_displays, 'edgechans'),
        'generic_x_vs_y_spw_intent_plots.mako'),
    'fieldshape': ComponentRendererMap(
        functools.partial(plot_image_displays, 'fieldshape'),
        'generic_x_vs_y_per_spw_pol_and_field_plots.mako'),
    'nmedian': ComponentRendererMap(
        functools.partial(plot_image_displays, 'nmedian'),
        'generic_x_vs_y_per_spw_pol_and_field_plots.mako'),
    'toomany': ComponentRendererMap(
        functools.partial(plot_image_displays, 'toomany'),
        'generic_x_vs_y_per_spw_pol_and_field_plots.mako')
}

# hold a map of Tsys flagging metric to generator of extra plots
extra_renderer_mapping = {
    'nmedian': ComponentRendererMap(
        functools.partial(plot_tsys_spectra, 'nmedian'),
        'generic_x_vs_y_ant_field_spw_pol_plots.mako'),
    'derivative': ComponentRendererMap(
        functools.partial(plot_tsys_spectra, 'derivative'),
        'generic_x_vs_y_ant_field_spw_pol_plots.mako'),
    'fieldshape': ComponentRendererMap(
        functools.partial(plot_tsys_spectra, 'fieldshape'),
        'generic_x_vs_y_ant_field_spw_pol_plots.mako'),
}
