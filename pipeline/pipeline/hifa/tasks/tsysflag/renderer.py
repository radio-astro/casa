'''
Created on 9 Sep 2014

@author: sjw
'''
import collections
import os

from ..tsyscal import renderer as tsyscalrenderer
import pipeline.hif.tasks.common.calibrationtableaccess as caltableaccess
import pipeline.infrastructure.displays as displays
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.rendererutils as rendererutils
import pipeline.infrastructure.utils as utils

LOG = logging.get_logger(__name__)

FlagTotal = collections.namedtuple('FlagSummary', 'flagged total')

std_templates = {'nmedian': 'generic_x_vs_y_per_spw_and_pol_plots.mako',
                 'derivative': 'generic_x_vs_y_per_spw_and_pol_plots.mako',
                 'edgechans': 'generic_x_vs_y_spw_intent_plots.mako',
                 'fieldshape': 'generic_x_vs_y_spw_intent_plots.mako',
                 'birdies': 'generic_x_vs_y_spw_ant_plots.mako'}

extra_templates = {'nmedian': 'generic_x_vs_y_per_spw_and_pol_plots.mako',
                   'derivative': 'generic_x_vs_y_per_spw_and_pol_plots.mako',
                   'fieldshape': 'generic_x_vs_y_spw_intent_plots.mako'}


class T2_4MDetailsTsysflagRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    '''
    Renders detailed HTML output for the Tsysflag task.
    '''
    def __init__(self, uri='tsysflag.mako',
                 description='Flag Tsys calibration',
                 always_rerender=False):
        super(T2_4MDetailsTsysflagRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def _do_standard_plots(self, context, result, component):
        if component in ('nmedian','derivative','fieldshape'):
            renderer = ImageDisplayPlotRenderer(context, result, component)
        elif component in ('edgechans', 'birdies'):
            renderer = SliceDisplayPlotRenderer(context, result, component)
        else:
            raise NotImplementedError('Unhandled Tsys flagging component: %s', component)

        with renderer.get_file() as fileobj:
            fileobj.write(renderer.render())
            
        return renderer

    def _do_extra_plots(self, context, result, component):
        if component in ('nmedian','derivative','fieldshape'):
            renderer = TsysSpectraPlotRenderer(context, result, component)
        else:
            raise NotImplementedError('Unhandled Tsys flagging component: %s', component)

        with renderer.get_file() as fileobj:
            fileobj.write(renderer.render())
            
        return renderer

    def update_mako_context(self, ctx, context, results):
        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % results.stage_number)

        stdplots = collections.defaultdict(dict)
        extraplots = collections.defaultdict(dict)
        for result in results:
            vis = os.path.basename(result.inputs['vis'])
            for component, r in result.components.items():
                if not r.view:
                    continue
                try:
                    renderer = self._do_standard_plots(context, result, component)
                    stdplots[component][vis] = renderer
                except TypeError:
                    continue

                try:
                    renderer = self._do_extra_plots(context, result, component)
                    extraplots[component][vis] = renderer
                except (TypeError, NotImplementedError):
                    continue

        flag_totals = collections.defaultdict(dict)
        for result in results:
            table = os.path.basename(result.inputs['caltable'])

            # summarise flag state on entry
            flag_totals[table]['before'] = self.flags_for_result(result, 
                    context, summary='first')

            # summarise flagging by each step
            for component, r in result.components.items():
                flag_totals[table][component] = self.flags_for_result(r, 
                                                                      context)

            # summarise flag state on exit
            flag_totals[table]['after'] = self.flags_for_result(result, 
                    context, summary='last')

        htmlreports = self.get_htmlreports(context, results)
        
        summary_plots = {}
        subpages = {}
        eb_plots = []
        last_results = []
        for result in results:
            # summary plots at end of flagging sequence, beware empty sequence
            lastflag = result.components.keys()
            if lastflag:
                lastflag = lastflag[-1]
            lastresult = result.components[lastflag]

            plotter = displays.TsysSummaryChart(context, lastresult)
            plots = plotter.plot()
            vis = os.path.basename(lastresult.inputs['vis'])
            summary_plots[vis] = plots

            # generate per-antenna plots
            plotter = displays.TsysPerAntennaChart(context, lastresult)
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

        # additionally render plots for all EBs in one page
        renderer = tsyscalrenderer.TsyscalPlotRenderer(context, last_results,
                                                       eb_plots)
        with renderer.get_file() as fileobj:
            fileobj.write(renderer.render())
            # .. and we want the subpage links to go to this master page
            for vis in subpages:
                subpages[vis] = renderer.path

        components = ['nmedian', 'derivative', 'edgechans', 'fieldshape', 'birdies']

        ctx.update({'flags': flag_totals,
                    'components': components,
                    'summary_plots': summary_plots,
                    'summary_subpage': subpages,
                    'dirname': weblog_dir,
                    'stdplots': stdplots,
                    'extraplots': extraplots,
                    'htmlreports': htmlreports})
        
    def get_htmlreports(self, context, results):
        report_dir = context.report_dir
        weblog_dir = os.path.join(report_dir,
                                  'stage%s' % results.stage_number)

        r = results[0]
        components = r.components.keys()

        htmlreports = {}

        for component in components:
            htmlreports[component] = {}

            for msresult in results:
                flagcmd_abspath = self.write_flagcmd_to_disk(weblog_dir, 
                  msresult.components[component], component)
                report_abspath = self.write_report_to_disk(weblog_dir, 
                  msresult.components[component], component)

                flagcmd_relpath = os.path.relpath(flagcmd_abspath, report_dir)
                report_relpath = os.path.relpath(report_abspath, report_dir)

                table_basename = os.path.basename(
                  msresult.components[component].table)
                htmlreports[component][table_basename] = \
                  (flagcmd_relpath, report_relpath)

        return htmlreports

    def write_flagcmd_to_disk(self, weblog_dir, result, component=None):
        tablename = os.path.basename(result.table)
        if component:
            filename = os.path.join(weblog_dir, '%s%s.html' % (tablename, component))
        else:
            filename = os.path.join(weblog_dir, '%s.html' % (tablename))

        rendererutils.renderflagcmds(result.flagcmds(), filename)
        return filename

    def write_report_to_disk(self, weblog_dir, result, component=None):
        # now write printTsysFlags output to a report file
        tablename = os.path.basename(result.table)
        if component:
            filename = os.path.join(weblog_dir, '%s%s.report.html' % (tablename, 
              component))
        else:
            filename = os.path.join(weblog_dir, '%s.report.html' % (tablename))
        if os.path.exists(filename):
            return filename
        
        rendererutils.printTsysFlags(result.table, filename)
        return filename

    def flags_for_result(self, result, context, summary=None):
        name = result.inputs['caltable']
        tsystable = caltableaccess.CalibrationTableDataFiller.getcal(name)
        ms = context.observing_run.get_ms(name=tsystable.vis) 

        summaries = result.summaries
        if summary == 'first':
            summaries = summaries[:1]
        elif summary == 'last':
            summaries = summaries[-1:]

        by_intent = self.flags_by_intent(ms, summaries)
        by_spw = self.flags_by_spws(ms, summaries)

        return utils.dict_merge(by_intent, by_spw)

    def flags_by_intent(self, ms, summaries):
        # create a dictionary of fields per observing intent, eg. 'PHASE':['3C273']
        intent_fields = {}
        for intent in ('BANDPASS', 'PHASE', 'AMPLITUDE', 'TARGET', 'ATMOSPHERE'):
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
                total[summary['name']][intent] = ft
    
            previous_summary = summary
                
        return total 
    
    def flags_by_spws(self, ms, summaries):
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
#                     print spw, summary['spw'].keys()

                if previous_summary:
                    flagcount -= int(previous_summary['spw'][spw]['flagged'])

            ft = FlagTotal(flagcount, totalcount)
            total[summary['name']]['TSYS SPWS'] = ft

            previous_summary = summary
                
        return total


class TimeVsAntennaPlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, component):
        r = result.components[component]

        vis = os.path.basename(result.inputs['vis'])
        self._vis = vis
        title = 'Time vs Antenna plots for %s' % vis
        outfile = filenamer.sanitize('%s-time_vs_ant-%s.html' % (vis, component))

        stage = 'stage%s' % result.stage_number
        dirname = os.path.join(context.report_dir, stage)

        plotter = displays.image.ImageDisplay()
        plots = plotter.plot(context, r, reportdir=dirname, prefix=component)
        
        super(TimeVsAntennaPlotRenderer, self).__init__(
                'generic_x_vs_y_per_spw_and_pol_plots.mako', context, 
                result, plots, title, outfile)

    def update_json_dict(self, d, plot):
        if 'vis' not in plot.parameters:
            plot.parameters['vis'] = self._vis
            d['vis'] = self._vis


class ImageDisplayPlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, component):
        r = result.components[component]

        stage = 'stage%s' % result.stage_number
        dirname = os.path.join(context.report_dir, stage)

        plotter = displays.image.ImageDisplay()
        plots = plotter.plot(context, r, reportdir=dirname, prefix=component)
        if not plots:
            raise TypeError('No plots generated for %s component' % component)

        x_axis = plots[0].x_axis
        y_axis = plots[0].y_axis

        vis = os.path.basename(result.inputs['vis'])
        self._vis = vis
        outfile = '%s-%s_vs_%s-%s.html' % (vis, y_axis, x_axis, component)
        outfile = filenamer.sanitize(outfile)

        y_axis = y_axis.replace('Tsys', 'T<sub>sys</sub>')
        title = '%s vs %s plots for %s' % (y_axis, x_axis, vis)
        self.shorttitle = '%s vs %s' % (y_axis, x_axis)

        super(ImageDisplayPlotRenderer, self).__init__(
                'generic_x_vs_y_per_spw_and_pol_plots.mako', context, 
                result, plots, title, outfile)

    def update_json_dict(self, d, plot):
        if 'vis' not in plot.parameters:
            plot.parameters['vis'] = self._vis
            d['vis'] = self._vis


class SliceDisplayPlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, component):
        r = result.components[component]

        stage = 'stage%s' % result.stage_number
        dirname = os.path.join(context.report_dir, stage)

        plotter = displays.slice.SliceDisplay()
        plots = plotter.plot(context, r, reportdir=dirname, plotbad=False,
                             plot_only_flagged=True, prefix=component)
        if not plots:
            raise TypeError('No plots generated for %s component' % component)

        x_axis = plots[0].x_axis
        y_axis = plots[0].y_axis

        vis = os.path.basename(result.inputs['vis'])
        self._vis = vis
        outfile = '%s-%s_vs_%s-%s.html' % (vis, y_axis, x_axis, component)
        outfile = filenamer.sanitize(outfile)

        y_axis = y_axis.replace('Tsys', 'T<sub>sys</sub>')
        title = '%s vs %s plots for %s' % (y_axis, x_axis, vis)
        self.shorttitle = '%s vs %s' % (y_axis, x_axis)

        template = std_templates[component]
        super(SliceDisplayPlotRenderer, self).__init__(
                template, context, result, plots, title, outfile)

    def update_json_dict(self, d, plot):
        if 'intent' in plot.parameters:
            d['intent'] = plot.parameters['intent']
        if 'vis' not in plot.parameters:
            plot.parameters['vis'] = self._vis
            d['vis'] = self._vis


class TsysSpectraPlotRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, component):
        stage = 'stage%s' % result.stage_number
        dirname = os.path.join(context.report_dir, stage)

        plots = []
        r = result.components[component]
        # plot Tsys spectra that were flagged
        for flagcmd in r.flagcmds():
            for description in r.descriptions():
                tsysspectra = r.first(description).children.get('tsysspectra')
                if tsysspectra is None:
                    continue
        
                for tsys_desc in tsysspectra.descriptions():
                    tsysspectrum = tsysspectra.first(tsys_desc)
        
                    if not flagcmd.match(tsysspectrum):
                        continue
        
                    # have found flagged spectrum, now get
                    # associated median spectrum
                    medians = r.last(description).children.get('tsysmedians')
        
                    for median_desc in medians.descriptions():
                        median_spectrum = medians.first(median_desc)
                        if median_spectrum.ant is None or \
                          median_spectrum.ant[0]==tsysspectrum.ant[0]:
                            # do the plot
                            plots.extend(displays.SliceDisplay().plot(
                              context=context, results=tsysspectra,
                              description_to_plot=tsys_desc,
                              overplot_spectrum=median_spectrum,
                              reportdir=dirname, plotbad=False))
                            break

        if not plots:
            raise TypeError('No spectra found for %s component' % component)

        x_axis = plots[0].x_axis
        y_axis = plots[0].y_axis

        vis = os.path.basename(result.inputs['vis'])
        self._vis = vis
        outfile = '%s-%s_vs_%s-%s.html' % (vis, y_axis, x_axis, component)
        outfile = filenamer.sanitize(outfile)
        
        y_axis = y_axis.replace('Tsys', 'T<sub>sys</sub>')
        title = '%s vs %s plots for %s' % (y_axis, x_axis, vis)
        self.shorttitle = '%s vs %s' % (y_axis, x_axis)

        template = extra_templates[component]
        super(TsysSpectraPlotRenderer, self).__init__(
                template, context, result, plots, title, outfile)
        
    def update_json_dict(self, d, plot):
        if 'intent' in plot.parameters:
            d['intent'] = plot.parameters['intent']
        if 'vis' not in plot.parameters:
            plot.parameters['vis'] = self._vis
            d['vis'] = self._vis
