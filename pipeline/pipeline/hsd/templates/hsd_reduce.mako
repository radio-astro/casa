<%!
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="task_details_base.html"/>

<h1>Stage ${hr.get_stage_number(result)}: ${hr.get_task_description(result)}</h1>

<ul>
<li>
% if result[0].success:
The task completed successfully.
% else:
The task failed.
% endif
</li>
<li>
<a href="${result[0].url}">Go to single dish results</a>
</li>
</ul>

<table class="one-column-emphasis" summary="Input parameters">
	<caption>Input parameters</caption>
    <colgroup>
    	<col class="oce-first" />
    </colgroup>
	<tbody>
		% for k, v in result.inputs.iteritems():
		<tr>
		  <td>${str(k)}</th>
		  <td>${str(v)}</td>
		</tr>
		% endfor
	</tbody>
</table>

<table class="one-column-emphasis" summary="Input parameters">
	<caption>Timing details</caption>
    <colgroup>
    	<col class="oce-first" />
    </colgroup>
	<tbody>
		<tr>
		  <td>Start time</th>
		  <td>${result.timestamps.start}</td>
		</tr>
		<tr>
		  <td>End</th>
		  <td>${result.timestamps.end}</td>
		</tr>
		<tr>
		  <td>Duration</th>
		  <td>${str(result.timestamps.end - result.timestamps.start)}</td>
		</tr>
	</tbody>
</table>
