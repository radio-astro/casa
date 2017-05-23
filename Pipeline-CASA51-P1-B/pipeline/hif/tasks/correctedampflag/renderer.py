"""
Created on 31 Jan 2017

@author: Vincent Geers (UKATC)
"""
import collections
import os

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils

LOG = logging.get_logger(__name__)

FlagTotal = collections.namedtuple('FlagSummary', 'flagged total')


class T2_4MDetailsCorrectedampflagRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    """
    Renders detailed HTML output for the Correctedampflag task.
    """
    def __init__(self, uri='correctedampflag.mako',
                 description='Flag corrected - model amplitudes for calibrator.',
                 always_rerender=False):
        super(T2_4MDetailsCorrectedampflagRenderer, self).__init__(
            uri=uri, description=description, always_rerender=always_rerender)

    def update_mako_context(self, mako_context, pipeline_context, results):

        # Generate HTML reports, including flag commands.
        htmlreports = self._get_htmlreports(pipeline_context, results)

        # Generate flagging totals.
        flag_totals = self._get_flag_totals(pipeline_context, results)

        # Update the mako context.
        mako_context.update(
            {'htmlreports': htmlreports,
             'flags': flag_totals,
             })

    def _get_flag_totals(self, context, results):

        # Initialize items that are to be exported to the
        # mako context.
        flag_totals = collections.defaultdict(dict)

        # For each result in the results list...
        for result in results:

            # Get flagging state before flagging.
            flag_totals[result.vis]['before'] = self._flags_for_result(
                result, context, summary='first')

            # Get flagging state after flagging.
            flag_totals[result.vis]['after'] = self._flags_for_result(
                result, context, summary='last')

        return flag_totals

    def _get_htmlreports(self, context, results):
        report_dir = context.report_dir
        weblog_dir = os.path.join(report_dir, 'stage%s' % results.stage_number)

        htmlreports = {}
        for result in results:
            flagcmd_abspath = self._write_flagcmd_to_disk(weblog_dir, result)
            flagcmd_relpath = os.path.relpath(flagcmd_abspath, report_dir)
            table_basename = os.path.basename(result.vis)
            htmlreports[table_basename] = flagcmd_relpath

        return htmlreports

    def _write_flagcmd_to_disk(self, weblog_dir, result):
        tablename = os.path.basename(result.vis)
        filename = os.path.join(weblog_dir, '%s-flag_commands.txt' % tablename)
        flagcmds = [l.flagcmd for l in result.flagcmds()]
        with open(filename, 'w') as flagfile:
            flagfile.writelines(['# Flag commands for %s\n#\n' % tablename])
            flagfile.writelines(['%s\n' % cmd for cmd in flagcmds])
            if not flagcmds:
                flagfile.writelines(['# No flag commands generated\n'])

        return filename

    def _flags_for_result(self, result, context, summary=None):
        ms = context.observing_run.get_ms(result.vis)
        summaries = result.summaries
        if summary == 'first':
            # select only first summary, but keep as list
            summaries = summaries[:1]
        elif summary == 'last':
            # select only last summary, but keep as list
            summaries = summaries[-1:]

        by_intent = self._flags_by_intent(ms, summaries)
        by_spw = self._flags_by_spws(summaries)

        return utils.dict_merge(by_intent, by_spw)

    def _flags_by_intent(self, ms, summaries):
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
                # The individual summaries may have been named differently from
                # each other, but the renderer will expect a single summary, so
                # consolidate summaries into a single summary named "Summary"
                total['Summary'][intent] = ft

            previous_summary = summary

        return total

    def _flags_by_spws(self, summaries):
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

                if previous_summary:
                    flagcount -= int(previous_summary['spw'][spw]['flagged'])

            ft = FlagTotal(flagcount, totalcount)
            total[summary['name']]['TSYS SPWS'] = ft

            previous_summary = summary

        return total
