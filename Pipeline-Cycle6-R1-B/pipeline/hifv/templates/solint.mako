<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">Solution Interval and test gain calibrations</%block>

<p>Determine the solution interval for a scan-average equivalent and do test gain calibrations to establish a short solution interval.</p>

% for ms in summary_plots:

    <ul>
        <li>The long solution interval is: <b> ${longsolint[ms]}s</b>.</li>
        <li>The short solution interval used is: <b>${new_gain_solint1[ms]}</b>.</li>
    </ul>

    <h4>Plots:  <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, testgainsamp_subpages[ms]), pcontext.report_dir)}">Testgains amp plots </a> | 
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, testgainsphase_subpages[ms]), pcontext.report_dir)}">Testgains phase plots</a>
    </h4>
    


%endfor