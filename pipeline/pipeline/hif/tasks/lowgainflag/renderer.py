"""
Created on 11 Sep 2014

@author: sjw
"""
import os

import pipeline.h.tasks.common.displays.image as image
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils

LOG = logging.get_logger(__name__)


class T2_4MDetailsLowgainFlagRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    """
    Renders detailed HTML output for the Lowgainflag task.
    """
    def __init__(self, uri='lowgainflag.mako', 
                 description='Flag antennas with low gain',
                 always_rerender=False):
        super(T2_4MDetailsLowgainFlagRenderer, self).__init__(
            uri=uri, description=description, always_rerender=always_rerender)
        
    def update_mako_context(self, mako_context, pipeline_context, results):
        htmlreports = self.get_htmlreports(pipeline_context, results)        

        # Create plots of flagging views.
        stage = 'stage%s' % results.stage_number
        dirname = os.path.join(pipeline_context.report_dir, stage)
        plots = {}
        for result in (r for r in results if r.view):
            vis = os.path.basename(result.inputs['vis'])
            plotter = image.ImageDisplay()
            plots[vis] = plotter.plot(
                context=pipeline_context, results=result, reportdir=dirname)

        plots_path = None
        if plots:
            all_plots = list(utils.flatten([v for v in plots.values()]))
            renderer = TimeVsAntenna1PlotRenderer(pipeline_context, results,
                                                  all_plots)
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                plots_path = os.path.relpath(renderer.path,
                                             pipeline_context.report_dir)

        # Check for updated reference antenna lists.
        updated_refants = {}
        for result in results:
            vis = result.vis
            # If the reference antenna list was updated, retrieve new refant
            # list.
            if result.refants_to_demote:
                ms = pipeline_context.observing_run.get_ms(name=vis)
                updated_refants[vis] = ms.reference_antenna

        mako_context.update({
            'htmlreports': htmlreports,
            'plots_path': plots_path,
            'updated_refants': updated_refants
        })

    def get_htmlreports(self, context, results):
        report_dir = context.report_dir
        weblog_dir = os.path.join(report_dir,
                                  'stage%s' % results.stage_number)

        htmlreports = {}
        for result in results:
            flagcmd_abspath = self._write_flagcmd_to_disk(weblog_dir, result)
            flagcmd_relpath = os.path.relpath(flagcmd_abspath, report_dir)
            table_basename = os.path.basename(result.table)
            htmlreports[table_basename] = flagcmd_relpath

        return htmlreports

    @staticmethod
    def _write_flagcmd_to_disk(weblog_dir, result):
        tablename = os.path.basename(result.table)
        filename = os.path.join(weblog_dir, '%s-flag_commands.txt' % tablename)
        flagcmds = [l.flagcmd for l in result.flagcmds()]
        with open(filename, 'w') as flagfile:
            flagfile.writelines(['# Flag commands for %s\n#\n' % tablename])
            flagfile.writelines(['%s\n' % cmd for cmd in flagcmds])
            if not flagcmds:
                flagfile.writelines(['# No flag commands generated\n'])
        return filename
    

class TimeVsAntenna1PlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, plots):
        vis = utils.get_vis_from_plots(plots)

        title = 'Time vs Antenna1 plots for %s' % vis
        outfile = filenamer.sanitize('time_vs_antenna1-%s.html' % vis)

        super(TimeVsAntenna1PlotRenderer, self).__init__(
                'generic_x_vs_y_spw_plots.mako', context, 
                result, plots, title, outfile)
