<%!
rsc_path = ""
import os

import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="header" />

<%block name="title">Stage ${hr.get_stage_number(result)} Task Details</%block>

<h1>Stage ${hr.get_stage_number(result)}: Fluxscale</h1>

<h2>Results</h2>
<p>The following flux densities were recorded in the pipeline context:</p>
<table class="minimalist" summary="Flux density results">
	<caption>Fluxscale Results</caption>
    <thead>
	    <tr>
	        <th scope="col" rowspan="2">Measurement Set</th>
	        <th scope="col" rowspan="2">Field</th>
	        <th scope="col" rowspan="2">SpW</th>
	        <th scope="col" colspan="4">Flux Density</th>
		</tr>
		<tr>
	        <th scope="col">I</th>
	        <th scope="col">Q</th>
	        <th scope="col">U</th>
	        <th scope="col">V</th>
	    </tr>
	</thead>
	<tbody>
%for single_result in result:
	%for field in single_result.measurements:
		%for flux in sorted(single_result.measurements[field], key=lambda m: m.spw.id):
		<tr>
			<td>${os.path.basename(single_result.vis)}</td>
			<td>${field}</td>
			<td>${flux.spw.id}</td>
			<td>${str(flux.I)}</td>
			<td>${str(flux.Q)}</td>
			<td>${str(flux.U)}</td>
			<td>${str(flux.V)}</td>
		</tr>
		%endfor
	%endfor
%endfor
	</tbody>
</table>
