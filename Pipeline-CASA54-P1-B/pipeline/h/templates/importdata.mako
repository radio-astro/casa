<%!
import os
%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">Import Data</%block>

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

<p>Data from ${num_mses} measurement set${'s were' if num_mses != 1 else ' was'}
 registered with the pipeline. The imported data 
${'is' if num_mses == 1 else 'are'} summarised below.</p>

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
			<th scope="col" rowspan="2">flux.csv</th>
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
			<td><a href="${fluxcsv_files[ms.basename]}" class="replace-pre" data-title="flux.csv">View</a> or <a href="${fluxcsv_files[ms.basename]}" download="${fluxcsv_files[ms.basename]}">download</a></td>
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
	<caption>Flux densities imported from the ASDM.  Online flux catalog values are used when available for ALMA.</caption>
    <thead>
	    <tr>
	        <th scope="col" rowspan="2">Measurement Set</th>
	        <th scope="col" rowspan="2">Field</th>
	        <th scope="col" rowspan="2">SpW</th>
	        <th scope="col" colspan="4">Flux Density</th>
	        <th scope="col" rowspan="2">Spix</th>
	        <th scope="col" rowspan="2">Age Of Nearest <p>Monitor Point (days)</th>
	    </tr>
	    <tr>
	        <th scope="col">I</th>
	        <th scope="col">Q</th>
	        <th scope="col">U</th>
	        <th scope="col">V</th>
	    </tr>
	</thead>
	<tbody>
	% for tr in flux_table_rows:
		<tr>
		% for td in tr:
			${td}
		% endfor
		</tr>
	%endfor
	</tbody>
</table>
% else:
<p>No flux densities were imported.</p>
% endif

<h3>Representative Target Information</h3>
% if repsource_defined:
<p>The following representative target sources and spws are defined</p>
<table class="table table-bordered table-striped table-condensed"
	   summary="Representative target information">
	<caption>Representative target sources. These are imported from the context or derived from the ASDM.</caption>
    <thead>
	    <tr>
	        <th scope="col" rowspan="2">Measurement Set</th>
	        <th scope="col" colspan="5">Representative Source</th>
	    </tr>
	    <tr>
	        <th scope="col">Name</th>
	        <th scope="col">Representative Frequency</th>
	        <th scope="col">Bandwidth for Sensitivity</th>
	        <th scope="col">Spw Id</th>
	        <th scope="col">Chanwidth</th>
	    </tr>
    </thead>
    <tbody>
	% for tr in repsource_table_rows:
		<tr>
		% for td in tr:
			${td}
		% endfor
		</tr>
	%endfor
    </tbody>
</table>
% else:
<p>No representative target source is defined</p>
% endif
