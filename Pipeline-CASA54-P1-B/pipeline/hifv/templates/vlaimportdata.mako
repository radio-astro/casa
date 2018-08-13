<%!
import operator
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils
%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">VLA Import Data</%block>

<!--
<ul class="unstyled">
	<li class="alert alert-info">
		<strong>To-do</strong> Missing Source.xml + no solar system object or cal known to CASA.
	</li>
	<li class="alert alert-info">
		<strong>To-do</strong> Missing BDFs.
	</li>
</ul>
-->

<p>${num_mses} measurement set${'s were' if num_mses != 1 else ' was'}
 registered with the pipeline. The imported data 
are summarized below.</p>

<table class="table table-bordered table-striped table-condensed"
	   summary="Summary of Imported Measurement Sets">
	<caption>Summary of Imported Measurement Sets</caption>
    <thead>
	    <tr>
	        <th scope="col" rowspan="2">Measurement Set</th>
	        <th scope="col" rowspan="2">SchedBlock ID</th>
			<th scope="col" rowspan="2">Src Type</th>
			<th scope="col" rowspan="2">Dst Type</th>
			<th scope="col" colspan="3">Number Imported</th>
			<th scope="col" rowspan="2">Size</th>
		</tr>
		<tr>
			<th>Scans</th>
			<th>Fields</th>
			<th>Flux Densities</th>
		</tr>
	</thead>
	<tbody>
% for importdata_result in result:
	% for ms in importdata_result.mses:
		<tr>
			<td>${ms.basename}</td>
			<td>${ms.schedblock_id}</td>
			<td>${importdata_result.origin[ms.basename]}</td>
			<!-- ScanTables are handled in the template for hsd_importdata, so
				 we can hard-code MS here -->
			<td>MS</td>
			<td>${len(ms.scans)}</td>
			<td>${len(ms.fields)}</td>
			<!-- count the number of measurements added to the setjy result in
				 each importdata_result -->
			<td>${reduce(lambda x, setjy_result: x + len(reduce(list.__add__, [setjy_result.measurements[key] for key in setjy_result.measurements], [])), importdata_result.setjy_results, 0)}</td>
			<td>${str(ms.filesize)}</td>
		</tr>
	% endfor
% endfor
	</tbody>
</table>

% if flux_imported:
<h3>Imported Flux Densities</h3>
<p>The following flux densities were imported into the pipeline context:</p>
<table class="table table-bordered table-striped table-condensed"
	   summary="Flux density results">
	<caption>Flux densities imported from ASDM</caption>
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
% for setjy_result in setjy_results:
	% for field in setjy_result.measurements:
		% for flux in sorted(setjy_result.measurements[field], key=lambda m: m.spw_id):
		<tr>
			<td>${os.path.basename(setjy_result.vis)}</td>
			<td>${field}</td>
			<td>${flux.spw_id}</td>
			<td>${str(flux.I)}</td>
			<td>${str(flux.Q)}</td>
			<td>${str(flux.U)}</td>
			<td>${str(flux.V)}</td>
		</tr>
		% endfor
	% endfor
% endfor
	</tbody>
</table>
% else:
<p>No flux densities were imported.</p>
% endif
