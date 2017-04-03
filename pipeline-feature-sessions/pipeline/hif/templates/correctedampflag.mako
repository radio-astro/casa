<%!
import os
%>

<%
# these functions are defined in template scope.
def num_lines(relpath):
	abspath = os.path.join(pcontext.report_dir, relpath)
	if os.path.exists(abspath):
		return sum(1 for line in open(abspath) if not line.startswith('#'))
	else:
		return 'N/A'
%>

<%inherit file="t2-4m_details-base.mako"/>
<%block name="header" />

<%block name="title">Flag corrected-model amplitudes for calibrator</%block>

This task identifies baselines and antennas with a significant fraction of
 outlier integrations, based on a comparison of the calibrated (corrected)
 amplitudes with the model amplitudes for one or more specified calibrator
 sources.

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
			<td>${num_lines(relpath)}</td>
        </tr>
	% endfor
	</tbody>
</table>

% endif
