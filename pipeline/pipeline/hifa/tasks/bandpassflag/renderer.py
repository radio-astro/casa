"""
Created on 28 Apr 2017

@author: Vincent Geers (UKATC)
"""
import collections
import copy
import os

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

        plot_dict = {
            'time_plots': get_plot_dicts(pipeline_context, results, 'time'),
            'uvdist_plots': get_plot_dicts(pipeline_context, results, 'uvdist')
        }
        mako_context.update(plot_dict)


def get_plot_dicts(pipeline_context, results, plot_type):
    """
    Create the bandpass flagging diagnostic plots for the given x axis.

    :param pipeline_context: pipeline context
    :param results: results to create plots for
    :param plot_type: 'time' or 'uvdist'
    :return:
    """
    # plots will be moved to this location
    plot_dest_dir = os.path.join(pipeline_context.report_dir, 'stage%s' % results.stage_number)

    d = collections.OrderedDict()

    for result in results:
        vis = os.path.basename(result.inputs['vis'])
        # ordereddict as raw->before->after ordering is important
        d[vis] = collections.OrderedDict()

        for key in ['raw', 'before', 'after']:
            if key in result.plots:
                plots = result.plots[key][plot_type]
                relocated = relocate_plots(plots, plot_dest_dir)
                # the weblog needs to identify each plot's associated plot type
                for p in relocated:
                    p.parameters['type'] = key
                d[vis][key] = relocated

    return d


def relocate_plots(plots, dest_dir):
    """
    Relocate a list of plots, returning a list of Plot objects that reflect
    the new location.

    :param plots: list of Plot objects
    :param dest_dir: location to move plots to
    :return: list of Plot objects
    """
    # create a copy so that we do not alter the result, which we'd like to
    # keep in its original state
    plot_copies = copy.deepcopy(plots)

    if plot_copies and not os.path.exists(dest_dir):
        os.makedirs(dest_dir)

    # move PNGs to destination directory and modify Plot path to reflect the
    # new location
    for plot in plot_copies:
        src = plot.abspath
        dst = os.path.join(dest_dir, plot.basename)

        # we always need to modify the plot for rerender=True mode
        plot.abspath = dst

        # .. but we should only attempt to move the file if it exists
        if os.path.exists(src):
            os.rename(src, dst)

    return plot_copies
