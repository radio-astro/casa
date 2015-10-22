<%!
rsc_path = ""

import os
import pydoc
import numpy as np
import sys

import pipeline.infrastructure.displays as displays
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.renderer.logger as logger
import pipeline.infrastructure.renderer.htmlrenderer as hr
import pipeline.infrastructure.renderer.sharedrenderer as sharedrenderer

%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="title">Stage ${hr.get_stage_number(result)} Task Details</%block>

<h1>Stage ${hr.get_stage_number(result)}: ${hr.get_task_description(result)}</h1>

## generate the plots
<%
qa_results = False
try:
    r = result[0]
    stage_dir = os.path.join(pcontext.report_dir, 'stage%d' % (r.stage_number))
    plots_dir = stage_dir
    if not os.path.exists(plots_dir):
        os.mkdir(plots_dir)

    plots = []
    for r in result:
        if r.qa_wvr.view:
            qa_results = True

            # display the view first
            # plot() returns the list of Plots it has generated, so we just
            # need to add each one to the HTML logger with logger.addPlot()
            plots.append(displays.ImageDisplay().plot(pcontext, r.qa_wvr,
              reportdir=plots_dir, change='WVR'))

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
    print 'hif_wvrgcal html template exception:', e
    raise e
%>
 
<ul>

% if not qa_results:
    <li>No QA results were calculated.
% else:
    <li>QA Results:</li>
    <ul>
        <li>Overall QA score.
            <table border="1">
                <tr>
                    <td>Vis</td>
                    <td>Score</td>
                </tr>
            % for r in result:
                <tr>
                    <td>${r.qa_wvr.vis}</td>
                    <td>${r.qa_wvr.overall_score}</td>
                </tr>
            % endfor
            </table>
        </li>
    </ul>

    <li>QA Plots:</li>
    <ul>
    % if plot_groups:
        % for plot_group in plot_groups: 
            <li><a class="replace" href="${os.path.join(dirname, plot_group.filename)}"}>${plot_group.title}</a></li>
        % endfor
    % endif
    </ul>

% endif

</ul>
