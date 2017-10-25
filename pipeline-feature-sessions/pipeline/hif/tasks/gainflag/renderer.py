"""
Created on 7 Oct 2015

@author: vgeers
"""
import os

#import pipeline.infrastructure.displays.image as image
import pipeline.h.tasks.common.displays.image as image
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils

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
        stage = 'stage%s' % results.stage_number
        dirname = os.path.join(pipeline_context.report_dir, stage)

        component_plots = {}
        for result in results:
            vis = os.path.basename(result.inputs['vis'])

            for component, r in result.components.items():
                if not r.view:
                    continue

                plotter = image.ImageDisplay()
                plots = plotter.plot(pipeline_context, r, reportdir=dirname, prefix=component)

                if plots:
                    component_plots.setdefault(component, {})[vis] = plots

        htmlreports = self._get_htmlreports(pipeline_context, results)

        components = results[0].metric_order

        # render plots for all EBs in one page
        plot_subpages = {}
        for component, vis_plots in component_plots.items():
            all_plots = list(utils.flatten([v for v in vis_plots.values()]))
            renderer = TimeVsAntenna1PlotRenderer(pipeline_context, results, all_plots, component)
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                subpage_path = os.path.relpath(renderer.path, pipeline_context.report_dir)
                for vis in vis_plots:
                    plot_subpages.setdefault(component, {})[vis] = subpage_path

        mako_context.update({
            'htmlreports': htmlreports,
            'components': components,
            'plots': plot_subpages,
            'agents': ('before', 'after')
        })

    def _get_htmlreports(self, context, results):
        report_dir = context.report_dir
        weblog_dir = os.path.join(report_dir,
                                  'stage%s' % results.stage_number)

        r = results[0]
        components = r.components.keys()

        htmlreports = {}
        
        for component in components:
            htmlreports[component] = {}

            for msresult in results:
                flagcmd_abspath = self._write_flagcmd_to_disk(weblog_dir, 
                  msresult.components[component], component)
                flagcmd_relpath = os.path.relpath(flagcmd_abspath, report_dir)
                table_basename = os.path.basename(
                  msresult.components[component].table)
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
    def __init__(self, context, result, plots, component):
        vis = utils.get_vis_from_plots(plots)

        title = 'Time vs Antenna1 plots for %s' % vis
        outfile = filenamer.sanitize('time_vs_antenna1-%s-%s.html' % (vis, component))

        super(TimeVsAntenna1PlotRenderer, self).__init__(
                'generic_x_vs_y_spw_plots.mako', context, 
                result, plots, title, outfile)
