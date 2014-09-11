'''
Created on 9 Sep 2014

@author: sjw
'''
import collections
import os

from ..tsyscal import TsyscalPlotRenderer
import pipeline.hif.tasks.common.calibrationtableaccess as caltableaccess
import pipeline.infrastructure.displays as displays
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.renderer.rendererutils as rendererutils
import pipeline.infrastructure.utils as utils

LOG = logging.get_logger(__name__)

FlagTotal = collections.namedtuple('FlagSummary', 'flagged total')


class T2_4MDetailsTsysflagRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    '''
    Renders detailed HTML output for the Tsysflag task.
    '''
    def __init__(self, uri='tsysflag.mako',
                 description='Flag Tsys calibration',
                 always_rerender=False):
        super(T2_4MDetailsTsysflagRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)

    def update_mako_context(self, ctx, context, results):
        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % results.stage_number)

        flag_totals = {}
        components = ['nmedian', 'derivative', 'edgechans', 'fieldshape', 'birdies']
        for msresult in results:
            table = os.path.basename(msresult.inputs['caltable'])
            flag_totals[table] = {}

            # summarise flag state on entry
            flag_totals[table]['before'] = self.flags_for_result(msresult, context,
              summary='first')

            # summarise flagging by each step
            for component in components:
                if component not in msresult.components.keys():
                    flag_totals[table][component] = None
                else:
                    flag_totals[table][component] = self.flags_for_result(
                      msresult.components[component], context)

            # summarise flag state on exit
            flag_totals[table]['after'] = self.flags_for_result(msresult, context,
              summary='last')

        htmlreports = self.get_htmlreports(context, results)
        
        summary_plots = {}
        subpages = {}
        for msresult in results:
            # summary plots at end of flagging sequence, beware empty
            # sequence
            lastflag = msresult.components.keys()
            if lastflag:
                lastflag = lastflag[-1]
            lastresult = msresult.components[lastflag]

            plotter = displays.TsysSummaryChart(context, lastresult)
            plots = plotter.plot()
            ms = os.path.basename(lastresult.inputs['vis'])
            summary_plots[ms] = plots

            # generate the per-antenna charts and JSON file
            plotter = displays.ScoringTsysPerAntennaChart(context, lastresult)
            plots = plotter.plot() 
            json_path = plotter.json_filename

            # write the html for each MS to disk
            renderer = TsyscalPlotRenderer(context, lastresult, plots, json_path)
            with renderer.get_file() as fileobj:
                fileobj.write(renderer.render())
                # the filename is sanitised - the MS name is not. We need to
                # map MS to sanitised filename for link construction.
                subpages[ms] = renderer.filename

        ctx.update({'flags'           : flag_totals,
                    'components'      : components,
                    'summary_plots'   : summary_plots,
                    'summary_subpage' : subpages,
                    'dirname'         : weblog_dir,
                    'htmlreports'     : htmlreports})

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
        if summary=='first':
            summaries = summaries[:1]
        elif summary=='last':
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
                    print spw, summary['spw'].keys()

                if previous_summary:
                    flagcount -= int(previous_summary['spw'][spw]['flagged'])

            ft = FlagTotal(flagcount, totalcount)
            total[summary['name']]['TSYS SPWS'] = ft

            previous_summary = summary
                
        return total
