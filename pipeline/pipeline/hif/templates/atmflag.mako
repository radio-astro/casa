<%!
rsc_path = ""

import os.path
import pydoc
import numpy as np
import sys

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.displays as displays
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.renderer.logger as logger
import pipeline.infrastructure.renderer.htmlrenderer as hr
import pipeline.infrastructure.renderer.rendererutils as hrutils

LOG = infrastructure.get_logger(__name__)
%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">Flag on atmospheric transmission</%block>

## generate the plots
<%
try:
    r = result[0]
    stage_dir = os.path.join(pcontext.report_dir, 'stage%d' % (r.stage_number))
    plots_dir = stage_dir
    if not os.path.exists(plots_dir):
        os.mkdir(plots_dir)

    plots = []
    htmlreports = {}
    for r in result:
        if hasattr(r, 'flagcmdfile'):
            reason = r.reason

            flagcmdhtml = os.path.join(plots_dir, '%s.atm.flagcmd.html' %
              os.path.basename(r.vis))
            hrutils.renderflagcmds(r.flagcmdfile, flagcmdhtml, reason)
            htmlreports[os.path.basename(r.vis)] = os.path.basename(flagcmdhtml)

        if r.view:
            # display the view first
            # plot() returns the list of Plots it has generated
            LOG.info('Plotting')
            plots.append(displays.SliceDisplay().plot(context=pcontext,
              results=r, reportdir=plots_dir, plotbad=True,
              plot_only_flagged=False))

    # Group the Plots by axes and plot types; each logical grouping will
    # be contained in a PlotGroup
    plot_groups = logger.PlotGroup.create_plot_groups(plots)
    # Write the thumbnail pages for each plot grouping to disk
    for plot_group in plot_groups:
        renderer = sharedrenderer.PlotGroupRenderer(pcontext, r, plot_group)
        plot_group.filename = renderer.basename
        with renderer.get_file() as fileobj:
            fileobj.write(renderer.render())

except Exception, e:
    print 'hif_atmflag html template exception:', e
    raise e
%>
 
% if plot_groups:
<h2>Plots</h2>
<ul>
    % for plot_group in plot_groups:
        <li><a class="replace" href="${os.path.join(dirname, plot_group.filename)}">${plot_group.title}</a>
        shows the Tsys spectra flagged in each image.</li>
    % endfor
</ul>
% endif

% if htmlreports:
<h2>Flags</h2>
<table class="table table-bordered">
    <tr>
        <td>Flagcmds</td>
    </tr>
    % for file,report in htmlreports.items():
    <tr>
        <td><a class="replace" href="${os.path.join(dirname, report)}">${file}</a></td>
    </tr>
    % endfor
</table>
% endif
