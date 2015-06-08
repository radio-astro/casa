'''
Created on 11 Sep 2014

@author: sjw
'''
import os

import pipeline.infrastructure.displays.image as image
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates

LOG = logging.get_logger(__name__)


class T2_4MDetailsLowgainFlagRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    '''
    Renders detailed HTML output for the Lowgainflag task.
    '''
    def __init__(self, uri='lowgainflag.mako', 
                 description='Flag antennas with low gain',
                 always_rerender=False):
        super(T2_4MDetailsLowgainFlagRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)
        
    def update_mako_context(self, mako_context, pipeline_context, results):
        htmlreports = self.get_htmlreports(pipeline_context, results)        

        plots = {}
        for result in results:
	    if not result.view:
	        continue
            renderer = TimeVsAntenna1PlotRenderer(pipeline_context, result)
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
            
            vis = os.path.basename(result.inputs['vis'])
            plots[vis] = os.path.relpath(renderer.path, pipeline_context.report_dir)

        mako_context.update({'htmlreports' : htmlreports,
                             'plots'       : plots})

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
    

class TimeVsAntenna1PlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result):
        vis = os.path.basename(result.inputs['vis'])
        title = 'Time vs Antenna1 plots for %s' % vis
        outfile = filenamer.sanitize('time_vs_antenna1-%s.html' % vis)

        stage = 'stage%s' % result.stage_number
        dirname = os.path.join(context.report_dir, stage)

        plotter = image.ImageDisplay()
        plots = plotter.plot(context=context, results=result, reportdir=dirname)
        
        super(TimeVsAntenna1PlotRenderer, self).__init__(
                'generic_x_vs_y_spw_plots.mako', context, 
                result, plots, title, outfile)
