"""
Created on 25 Jul 2018

@author: vcg
"""
import collections
import os

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
from pipeline.h.tasks.importdata.renderer import make_repsource_table

LOG = logging.get_logger(__name__)


class T2_4MDetailsRestoreDataRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='restoredata.mako',
                 description='Restore Calibrated Data',
                 always_rerender=False):
        super(T2_4MDetailsRestoreDataRenderer, self).__init__(
            uri=uri, description=description, always_rerender=always_rerender)

    def update_mako_context(self, mako_context, pipeline_context, results):
        weblog_dir = os.path.join(pipeline_context.report_dir, 'stage%s' % results.stage_number)

        # Extract information for representative sources table.
        repsource_table_rows = make_repsource_table(pipeline_context, results)

        # Extract information for flagging summary table.
        flags = _get_flags(pipeline_context, results)
        flags_maxspw = max([len(flags[src][vis]) for src in flags for vis in flags[src]])

        # Update weblog mako context.
        mako_context.update({
            'casa_version': results[0].casa_version_orig,
            'pipeline_version': results[0].pipeline_version_orig,
            'repsource_defined': True if repsource_table_rows else False,
            'repsource_table_rows': repsource_table_rows,
            'flags': flags,
            'flags_maxspw': flags_maxspw,
            'weblog_dir': weblog_dir,
        })


def _get_flags(pipeline_context, results):
    flags = collections.defaultdict(collections.defaultdict)

    # Extract information for each result, session, and vis.
    for r in results:
        for session, res_summaries in r.flagging_summaries.items():
            for vis, session_summaries in res_summaries.items():

                # Load the MS.
                ms = pipeline_context.observing_run.get_ms(name=vis)

                # Get science targets.
                sci_sources = [source.name for source in ms.sources if 'TARGET' in source.intents]

                # Get science spectral windows.
                sci_spws = ms.get_spectral_windows(science_windows_only=True)
                sci_spwids = [str(spw.id) for spw in sci_spws]

                # Extract information for each source in current vis.
                for source, src_summary in session_summaries.items():
                    if source in sci_sources:
                        flags[source][vis] = {}

                        for spw in sci_spwids:
                            flags[source][vis][spw] = src_summary['spw'][spw]

    return flags
