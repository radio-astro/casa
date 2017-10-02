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
from pipeline.hifa.tasks.gfluxscaleflag.renderer import get_plot_dicts

LOG = logging.get_logger(__name__)

FlagTotal = collections.namedtuple('FlagSummary', 'flagged total')


class T2_4MDetailsBandpassflagRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    """
    Renders detailed HTML output for the Bandpassflag task.
    """
    def __init__(self, uri='bandpassflag.mako',
                 description='Phase-up bandpass calibration and flagging',
                 always_rerender=False):
        super(T2_4MDetailsBandpassflagRenderer, self).__init__(uri=uri, description=description,
                                                               always_rerender=always_rerender)

        # Attach bandpass renderer.
        self.bprenderer = T2_4MDetailsBandpassRenderer(uri=uri, description=description,
                                                       always_rerender=always_rerender)

        # Attach correctedampflag renderer.
        self.cafrenderer = T2_4MDetailsCorrectedampflagRenderer(uri=uri, description=description,
                                                                always_rerender=always_rerender)

    def update_mako_context(self, mako_context, pipeline_context, results):

        # Initialize items that are to be exported to the
        # mako context
        updated_refants = {}

        #
        # Get flagging reports, summaries
        #
        cafresults = basetask.ResultsList()
        for result in results:
            cafresults.append(result.cafresult)
        cafresults.stage_number = results.stage_number
        self.cafrenderer.update_mako_context(mako_context, pipeline_context, cafresults)

        #
        # Render the bandpass results
        #

        # Collect bandpass task results, and run those through
        # standard hifa bandpass mako context update.
        bpresults = basetask.ResultsList()
        for result in results:
            bpresults.append(result.bpresult)
        bpresults.stage_number = results.stage_number
        self.bprenderer.update_mako_context(mako_context, pipeline_context, bpresults)

        #
        # Get diagnostic plots.
        #
        time_plots = get_plot_dicts(pipeline_context, results, 'time')
        uvdist_plots = get_plot_dicts(pipeline_context, results, 'uvdist')

        #
        # Check for updated reference antenna lists.
        #
        for result in results:
            vis = result.vis
            # If the reference antenna list was updated, retrieve new refant
            # list.
            if result.refants_to_remove or result.refants_to_demote:
                ms = pipeline_context.observing_run.get_ms(name=vis)
                updated_refants[vis] = ms.reference_antenna

        # Update the mako context.
        mako_context.update({
            'time_plots': time_plots,
            'uvdist_plots': uvdist_plots,
            'updated_refants': updated_refants
        })
