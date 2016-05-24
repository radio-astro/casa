"""
Created on 7 Oct 2015

@author: vgeers
"""
import collections
import os

import pipeline.infrastructure.displays as displays
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates

LOG = logging.get_logger(__name__)

class T2_4MDetailsGainflagRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    """
    Renders detailed HTML output for the Gainflag task.
    """
    def __init__(self, uri='gainflag.mako', 
                 description='Flag antennas with gain outliers',
                 always_rerender=False):
        super(T2_4MDetailsGainflagRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, mako_context, pipeline_context, results):
        plots = {}
        for result in results:
            vis = os.path.basename(result.inputs['vis'])
            for component, r in result.components.items():
                if not r.view:
                    continue
                try:
                    renderer = TimeVsAntenna1PlotRenderer(pipeline_context, result, component)
                    with renderer.get_file() as fileobj:
                        fileobj.write(renderer.render())
                    # CAS-8265: multi-ms in time order in all weblog stages
                    # Maintain the time-order of the input results by using an OrderedDict
                    plots.setdefault(component, collections.OrderedDict())[vis] = os.path.relpath(renderer.path, pipeline_context.report_dir)
                except TypeError:
                    continue
                
        htmlreports = self._get_htmlreports(pipeline_context, results)
        
        components = results[0].metric_order
        
        mako_context.update({
            'htmlreports': htmlreports,
            'components': components,
            'plots': plots,
            'agents': ['before', 'after']
        })

    def _get_htmlreports(self, context, results):
        report_dir = context.report_dir
        weblog_dir = os.path.join(report_dir,
                                  'stage%s' % results.stage_number)

        r = results[0]
        components = r.components.keys()

        htmlreports = {}
        
        for component in components:
            # CAS-8265: multi-ms in time order in all weblog stages
            # Maintain the time-order of the input results by using an OrderedDict
            htmlreports[component] = collections.OrderedDict()

            for msresult in results:
                flagcmd_abspath = self._write_flagcmd_to_disk(weblog_dir, msresult.components[component], component)
                flagcmd_relpath = os.path.relpath(flagcmd_abspath, report_dir)
                table_basename = os.path.basename(msresult.components[component].table)
                htmlreports[component][table_basename] = flagcmd_relpath

        return htmlreports

    def _write_flagcmd_to_disk(self, weblog_dir, result, component=None):
        tablename = os.path.basename(result.table)
        if component:
            filename = os.path.join(weblog_dir, '%s%s-flag_commands.txt' % (tablename, component))
        else:
            filename = os.path.join(weblog_dir, '%s-flag_commands.txt' % (tablename))

        flagcmds = [l.flagcmd for l in result.flagcmds()]
        with open(filename, 'w') as flagfile:
            flagfile.writelines(['# Flag commands for %s\n#\n' % tablename])
            flagfile.writelines(['%s\n' % cmd for cmd in flagcmds])
            if not flagcmds:
                flagfile.writelines(['# No flag commands generated\n'])
        
        return filename


class TimeVsAntenna1PlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, component):

        r = result.components[component]

        stage = 'stage%s' % result.stage_number
        dirname = os.path.join(context.report_dir, stage)
        
        plotter = displays.image.ImageDisplay()
        plots = plotter.plot(context, r, reportdir=dirname, prefix=component)
        if not plots:
            raise TypeError('No plots generated for %s component' % component)
        
        vis = os.path.basename(result.inputs['vis'])
        outfile = filenamer.sanitize('time_vs_antenna1-%s-%s.html' % (vis, component))

        title = 'Time vs Antenna1 plots for %s' % vis

        super(TimeVsAntenna1PlotRenderer, self).__init__(
                'generic_x_vs_y_spw_plots.mako', context, 
                result, plots, title, outfile)
