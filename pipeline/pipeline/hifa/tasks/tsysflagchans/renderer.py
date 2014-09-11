'''
Created on 9 Sep 2014

@author: sjw
'''
import os

from ..tsyscal import TsyscalPlotRenderer
import pipeline.infrastructure.displays as displays
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.logger as logger
import pipeline.infrastructure.renderer.rendererutils as rendererutils
import pipeline.infrastructure.renderer.sharedrenderer as sharedrenderer

LOG = logging.get_logger(__name__)


class T2_4MDetailsTsysflagchansRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    '''
    Renders detailed HTML output for the Tsysflagchans task.
    '''
    def __init__(self, uri='tsysflagchans.mako',
                 description='Flag channels of Tsys calibration',
                 always_rerender=False):
        super(T2_4MDetailsTsysflagchansRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, ctx, context, results):
        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % results.stage_number)

        htmlreports = self.get_html_reports(context, results)
        plot_groups = self.get_plots(context, results)

        summary_plots = {}
        subpages = {}
        slplots = []
        for result in results:
            if result.view:
                plotter = displays.SliceDisplay()
                slplots.append(plotter.plot(context, result, weblog_dir, 
                                            plotbad=False,
                                            plot_only_flagged=True))
            
            if result.flagged() is False:
                continue

            plotter = displays.TsysSummaryChart(context, result)
            plots = plotter.plot()
            ms = os.path.basename(result.inputs['vis'])
            summary_plots[ms] = plots

            # generate the per-antenna charts and JSON file
            plotter = displays.ScoringTsysPerAntennaChart(context, result)
            plots = plotter.plot() 
            json_path = plotter.json_filename

            # write the html for each MS to disk
            renderer = TsyscalPlotRenderer(context, result, plots, json_path)
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                # the filename is sanitised - the MS name is not. We need to
                # map MS to sanitised filename for link construction.
                subpages[ms] = renderer.filename


        # add the PlotGroups to the Mako context. The Mako template will parse
        # these objects in order to create links to the thumbnail pages we
        # just created
        ctx.update({'plot_groups'     : plot_groups,
                    'summary_plots'   : summary_plots,
                    'summary_subpage' : subpages,
                    'dirname'         : weblog_dir,
                    'htmlreports'     : htmlreports})

    def get_plots(self, context, results):
        report_dir = context.report_dir
        weblog_dir = os.path.join(report_dir,
                                  'stage%s' % results.stage_number)

        plots = []
        for result in results:
            if result.view:
                plots.append(displays.SliceDisplay().plot(
                    context=context, results=result, reportdir=weblog_dir,
                    plotbad=False, plot_only_flagged=True))

        # Group the Plots by axes and plot types; each logical grouping will
        # be contained in a PlotGroup
        plot_groups = logger.PlotGroup.create_plot_groups(plots)
        # Write the thumbnail pages for each plot grouping to disk
        for plot_group in plot_groups:
            renderer = sharedrenderer.PlotGroupRenderer(context, results, plot_group)
            plot_group.filename = renderer.basename
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                
        return plot_groups

    def get_html_reports(self, context, results):
        report_dir = context.report_dir
        weblog_dir = os.path.join(report_dir,
                                  'stage%s' % results.stage_number)

        htmlreports = {}
        for result in results:
#            if not hasattr(result, 'flagcmdfile'):
#                continue

            flagcmd_abspath = self.write_flagcmd_to_disk(weblog_dir, result)
            report_abspath = self.write_report_to_disk(weblog_dir, result)

            flagcmd_relpath = os.path.relpath(flagcmd_abspath, report_dir)
            report_relpath = os.path.relpath(report_abspath, report_dir)

            table_basename = os.path.basename(result.table)
            htmlreports[table_basename] = (flagcmd_relpath, report_relpath)

        return htmlreports

    def write_flagcmd_to_disk(self, weblog_dir, result):
        tablename = os.path.basename(result.table)
        filename = os.path.join(weblog_dir, '%s.html' % tablename)
        if os.path.exists(filename):
            return filename

        rendererutils.renderflagcmds(result.flagcmds(), filename)
        return filename

    def write_report_to_disk(self, weblog_dir, result):
        # now write printTsysFlags output to a report file
        tablename = os.path.basename(result.table)
        filename = os.path.join(weblog_dir, '%s.report.html' % tablename)
        if os.path.exists(filename):
            return filename

        rendererutils.printTsysFlags(result.table, filename)
        return filename
