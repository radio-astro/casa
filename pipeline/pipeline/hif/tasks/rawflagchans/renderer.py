"""
Created on 24 Nov 2014

@author: sjw
"""
import os

import pipeline.infrastructure.displays.image as image
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils
from ..common import flagging_renderer_utils as flagutils

LOG = logging.get_logger(__name__)


class T2_4MDetailsRawflagchansRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='rawflagchans.mako', 
                 description='Flag channels in raw data',
                 always_rerender=False):
        super(T2_4MDetailsRawflagchansRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, mako_context, pipeline_context, results):
        htmlreports = self._get_htmlreports(pipeline_context, results)

        plots = {}
        flag_totals = {}

        stage = 'stage%s' % results.stage_number
        dirname = os.path.join(pipeline_context.report_dir, stage)

        for result in (r for r in results if r.view):
            vis = os.path.basename(result.inputs['vis'])
            plotter = image.ImageDisplay()
            plots[vis] = plotter.plot(context=pipeline_context, results=result, reportdir=dirname)

            flags_for_result = flagutils.flags_for_result(result, pipeline_context)
            flag_totals = utils.dict_merge(flag_totals, flags_for_result)

        # render plots for all EBs in one page
        plots_path = None
        if plots:
            all_plots = list(utils.flatten([v for v in plots.values()]))
            renderer = BaselineVsChannelsPlotRenderer(pipeline_context, results, all_plots)
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                plots_path = os.path.relpath(renderer.path, pipeline_context.report_dir)

        mako_context.update({
            'htmlreports': htmlreports,
            'flags': flag_totals,
            'agents': ('before', 'after'),
            'plots_path': plots_path
        })

    def _get_htmlreports(self, context, results):
        report_dir = context.report_dir
        weblog_dir = os.path.join(report_dir, 'stage%s' % results.stage_number)

        htmlreports = {}
        for result in results:
            flagcmd_abspath = self._write_flagcmd_to_disk(weblog_dir, result)
            flagcmd_relpath = os.path.relpath(flagcmd_abspath, report_dir)
            table_basename = os.path.basename(result.table)
            htmlreports[table_basename] = flagcmd_relpath

        return htmlreports

    def _write_flagcmd_to_disk(self, weblog_dir, result):
        tablename = os.path.basename(result.table)
        filename = os.path.join(weblog_dir, '%s-flag_commands.txt' % tablename)
        flagcmds = [l.flagcmd for l in result.flagcmds()]
        with open(filename, 'w') as flagfile:
            flagfile.writelines(['# Flag commands for %s\n#\n' % tablename])
            flagfile.writelines(['%s\n' % cmd for cmd in flagcmds])
            if not flagcmds:
                flagfile.writelines(['# No flag commands generated\n'])
                
        return filename


class BaselineVsChannelsPlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, plots):
        vis = utils.get_vis_from_plots(plots)

        title = 'Baseline vs channels for %s' % vis
        outfile = filenamer.sanitize('baseline_vs_channels-%s.html' % vis)

        super(BaselineVsChannelsPlotRenderer, self).__init__(
                'generic_x_vs_y_spw_pol_plots.mako', context,
                result, plots, title, outfile)
