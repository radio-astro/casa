'''
Created on 24 Nov 2014

@author: sjw
'''
import os

import pipeline.infrastructure.displays.image as image
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils
from ..common import flagging_renderer_utils as flagutils

LOG = logging.get_logger(__name__)


class T2_4MDetailsFlagcorrectedRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='flagcorrected.mako', 
                 description='Flag corrected data',
                 always_rerender=False):
        super(T2_4MDetailsFlagcorrectedRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, mako_context, pipeline_context, results):
        htmlreports = self._get_htmlreports(pipeline_context, results)

        plots = {}
        flag_totals = {}
        for result in results:
            if not result.view:
                continue
            
            if result.metric == 'antenna':
                renderer = TimeVsAntennaPlotRenderer(pipeline_context, result)
            elif result.metric == 'baseline':
                renderer = BaselineVsTimePlotRenderer(pipeline_context, result)
            
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())

            vis = os.path.basename(result.inputs['vis'])
            plots[vis] = os.path.relpath(renderer.path, pipeline_context.report_dir)
                
            flags_for_result = flagutils.flags_for_result(result, 
                                                          pipeline_context)
            flag_totals = utils.dict_merge(flag_totals, 
                                           flags_for_result)

        mako_context.update({'htmlreports' : htmlreports,
                             'plots'       : plots,
                             'flags'       : flag_totals,
                             'agents'      : ['before', 'after']})        

    def _get_htmlreports(self, context, results):
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


class BaselineVsTimePlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result):
        vis = os.path.basename(result.inputs['vis'])
        title = 'Baseline vs Time plots for %s' % vis
        outfile = filenamer.sanitize('baseline_vs_time-%s.html' % vis)

        stage = 'stage%s' % result.stage_number
        dirname = os.path.join(context.report_dir, stage)

        plotter = image.ImageDisplay()
        plots = plotter.plot(context=context, results=result, reportdir=dirname)
        
        super(BaselineVsTimePlotRenderer, self).__init__(
                'generic_x_vs_y_per_spw_and_pol_plots.mako', context, 
                result, plots, title, outfile)


class TimeVsAntennaPlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result):

        vis = os.path.basename(result.inputs['vis'])
        title = 'Time vs Antenna plots for %s' % vis
        outfile = filenamer.sanitize('time_vs_ant-%s.html' % vis)

        stage = 'stage%s' % result.stage_number
        dirname = os.path.join(context.report_dir, stage)

        plotter = image.ImageDisplay()
        plots = plotter.plot(context, results=result, reportdir=dirname)
        
        super(TimeVsAntennaPlotRenderer, self).__init__(
                'generic_x_vs_y_per_spw_and_pol_plots.mako', context, 
                result, plots, title, outfile)
