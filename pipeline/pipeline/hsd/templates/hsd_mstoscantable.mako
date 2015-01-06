<%!
rsc_path = "../"
import os
%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="header" />

<%block name="title">Convert MS to Scantable</%block>

<%
%>

<p>This task converts registered measurement sets to single dish specific data format, scantables. 
During conversion, the data are separated by antenna. Optionally, interferometric Tsys caltables 
are converted to single dish specific Tsys caltables.</p>

<div class="row-fluid">
	<h2>Conversion Summary</h2>
	<table class="table table-bordered table-striped">
		<caption>MS conversion</caption>	   
		<thead>
			<tr>
				<th>Measurement Set</th>
				<th>Scantables</th>
			</tr>
		</thead>
		<tbody>
% for (vis,infiles) in vismap.items():
			<tr>
				<td rowspan="${len(infiles)}">${os.path.basename(vis)}</td>
				<td>${os.path.basename(infiles[0])}</td>
			</tr>
	% for infile in infiles[1:]:
			<tr>
				<td>${os.path.basename(infile)}</td>
			</tr>
	% endfor
% endfor
		</tbody>
	</table>

% if len(caltablemap) > 0:
	<table class="table table-bordered table-striped">
		<caption>Caltable conversion</caption>	   
		<thead>
			<tr>
				<th>Original Caltable</th>
				<th>Converted Caltable</th>
			</tr>
		</thead>
		<tbody>
	% for (orig,mapped) in caltablemap.items():
			<tr>
				<td rowspan="${len(mapped)}">${os.path.basename(orig)}</td>
				<td>${os.path.basename(mapped.values()[0])}</td>
			</tr>
		% for m in mapped.values()[1:]:
			<tr>
				<td>${os.path.basename(m)}</td>
			</tr>
		% endfor
	% endfor
		</tbody>
	</table>
% else:
No Tsys caltables.
% endif
</div>


