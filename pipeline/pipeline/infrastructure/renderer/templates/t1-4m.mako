<%!
navbar_active='By Task'
import os
import pipeline.domain.measures as measures
import pipeline.infrastructure.renderer.htmlrenderer as hr
import pipeline.infrastructure.renderer.rendererutils as rendererutils
import pipeline.infrastructure.utils as utils
%>
<%inherit file="base.mako"/>

<script>
$(document).ready(function() {
	pipeline.pages.t1_4m.ready();
});
</script>

<div id="mainbody">

<%block name="title">Task Summaries</%block>

<div class="page-header">
	<h1>Task Summaries</h1>
</div>

<table class="table">
	<thead>
		<tr>
			<th class="col-md-8"><span class="glyphicon glyphicon-none"></span> Task</th>
			<th class="col-md-2">QA Score</th>
			<th class="col-md-1"></th>
			<th class="col-md-1">Duration</th>
		</tr>
	</thead>
	<tbody>
		% for i in range(len(results)):
        <%
            result = results[i]
            qascore = scores[result.stage_number]
        %>
		<tr>
            <td>${rendererutils.get_symbol_badge(result)} <a href="t2-4m.html?sidebar=sidebar_stage${result.stage_number}">${hr.get_task_description(result, pcontext)}</a><span
                    class="pull-right">${rendererutils.format_shortmsg(qascore)}</span></td>
            <td>
                <div class="progress" style="margin-bottom:0px;">
                    <div class="progress-bar${rendererutils.get_bar_class(qascore)}"
                         role="progressbar"
                         aria-valuenow="${qascore.score}"
                         aria-valuemin="0"
                         aria-valuemax="1"
                         style="width:${rendererutils.get_bar_width(qascore)}%;">
                        <span class="sr-only">Score = ${qascore.score}</span>
                    </div>
                </div>
            </td>
			<td><span class="badge${rendererutils.get_badge_class(qascore)}">${rendererutils.format_score(qascore)}</span></td>
			<td>${task_duration[i]}</td>
		</tr>
		% endfor
	</tbody>
</table>

<%def name="li_anchor_to_file(relpath, tooltip)">
	<%
	abspath = os.path.join(pcontext.report_dir, relpath)
	file_exists = os.path.exists(abspath)
	if file_exists:
		total_bytes = os.path.getsize(abspath)
		filesize = measures.FileSize(total_bytes, measures.FileSizeUnits.BYTES)
	%>
	% if file_exists:
		<li><a href="${relpath}" data-title="${relpath}" class="replace-pre">View</a>, <a href="t1-4.html?logfile=${relpath}" target="_blank">view in new tab</a> or <a href="${relpath}" download="${relpath}" data-title="Click to download ${tooltip}">download</a> ${relpath} (${str(filesize)})</li>
	% endif
</%def>

% if any(logname in ['casalogs', 'casa_commands', 'pipeline_script', 'pipeline_restore_script'] for logname in pcontext.logs):
<div class="panel panel-default">
	<div class="panel-heading">
		<h3 class="panel-title">CASA logs and scripts</h3>
	</div>
	<div class="panel-body">
		<ul>
		%for log_url in pcontext.logs['casalogs']:
		${li_anchor_to_file(log_url, 'CASA log')}
		%endfor
		${li_anchor_to_file(pcontext.logs['casa_commands'], 'CASA commands log')}
		${li_anchor_to_file(pcontext.logs['pipeline_script'], 'pipeline equivalent script')}
		${li_anchor_to_file(pcontext.logs['pipeline_restore_script'], 'pipeline restore script')}
		</ul>	
	</div>
</div>
%endif 

</div> <!-- end mainbody -->