"""
Created on 28 Apr 2017

@author: Vincent Geers (UKATC)
"""

import collections

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
from pipeline.hif.tasks.correctedampflag.renderer import T2_4MDetailsCorrectedampflagRenderer
from pipeline.hifa.tasks.bandpass.renderer import T2_4MDetailsBandpassRenderer
from pipeline.infrastructure import basetask

LOG = logging.get_logger(__name__)

FlagTotal = collections.namedtuple('FlagSummary', 'flagged total')


class T2_4MDetailsBandpassflagRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    """
    Renders detailed HTML output for the Bandpassflag task.
    """
    # FIXME: set rerender to false before final commit
    def __init__(self, uri='bandpassflag.mako',
                 description='Phase-up bandpass calibration + flagging',
                 always_rerender=True):
        super(T2_4MDetailsBandpassflagRenderer, self).__init__(
            uri=uri, description=description, always_rerender=always_rerender)

        # Attach bandpass renderer.
        self.bprenderer = T2_4MDetailsBandpassRenderer(
            uri=uri, description=description, always_rerender=always_rerender)

        # Attach correctedampflag renderer.
        self.cafrenderer = T2_4MDetailsCorrectedampflagRenderer(
            uri=uri, description=description, always_rerender=always_rerender)

    def update_mako_context(self, mako_context, pipeline_context, results):

        #
        # Get flagging reports, summaries
        #
        cafresults = basetask.ResultsList()
        for result in results:
            cafresults.append(result.cafresult)
        cafresults.stage_number = results.stage_number
        self.cafrenderer.update_mako_context(
            mako_context, pipeline_context, cafresults)

        #
        # Render the bandpass results
        #

        # Collect bandpass task results, and run those through
        # standard hifa bandpass mako context update.
        bpresults = basetask.ResultsList()
        for result in results:
            bpresults.append(result.bpresult)
        bpresults.stage_number = results.stage_number
        self.bprenderer.update_mako_context(
            mako_context, pipeline_context, bpresults)
