<%!
import os.path

def num_lines(report_dir, relpath):
	abspath = os.path.join(report_dir, relpath)
	if os.path.exists(abspath):
		return sum(1 for line in open(abspath) if not line.startswith('#'))
	else:
		return 'N/A'
%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="title">Flag antennas with low gain</%block>
 
% if plots:
<h2>Plots</h2>
<ul>
    % for vis, relpath in plots.items():
        <li>
               <a class="replace" href="${relpath}">${vis}</a>
               shows the images used for flagging.
    % endfor
</ul>
% endif

% if htmlreports:
<h2>Flags</h2>
<table class="table table-bordered table-striped">
	<caption>Report Files</caption>
	<thead>
		<tr>
			<th>Measurement Set</th>
			<th>Flagging Commands</th>
			<th>Number of Statements</th>
		</tr>
	</thead>
	<tbody>
		% for msname, relpath in htmlreports.items():
		<tr>
			<td>${msname}</td>
			<td><a class="replace-pre" href="${relpath}">${os.path.basename(relpath)}</a></td>
			<td>${num_lines(pcontext.report_dir, relpath)}</td>
		</tr>
		% endfor
	</tbody>
</table>
% endif
