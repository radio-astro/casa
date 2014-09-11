'''
Created on 10 Sep 2014

@author: sjw
'''
import os

from ..tsyscal import TsyscalPlotRenderer
import pipeline.infrastructure.displays as displays
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.rendererutils as rendererutils

LOG = logging.get_logger(__name__)


class T2_4MDetailsTsysflagspectraRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    '''
    Renders detailed HTML output for the Tsysflagspectra task.
    '''
    def __init__(self, uri='tsysflagspectral.mako',
                 description='Flag spectra in Tsys calibration',
                 always_rerender=False):
        super(T2_4MDetailsTsysflagspectraRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, ctx, context, results):
        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % results.stage_number)

        htmlreports = self.get_htmlreports(context, results)
        
        summary_plots = {}
        subpages = {}
        for result in results:
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

        ctx.update({'summary_plots'   : summary_plots,
                    'summary_subpage' : subpages,
                    'dirname'         : weblog_dir,
                    'htmlreports'     : htmlreports})

        return ctx

    def get_htmlreports(self, context, results):
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
