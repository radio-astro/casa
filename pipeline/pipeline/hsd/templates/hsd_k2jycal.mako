<%!
rsc_path = "../"
import os
%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="header" />

<%block name="title">Generate Kelvin to Jansky conversion caltables</%block>

<%
stage_dir = os.path.join(pcontext.report_dir, 'stage%s'%(result.stage_number))
%>

<p>This task generates single dish images per source per spectral window. 
It generates an image combined spectral data from whole antenna as well as images per antenna.</p>

<h3>Jy/K Conversion Factor</h3>
The following table lists the Jy/K factor.
% if reffile is not None and len(reffile) > 0 and os.path.exists(os.path.join(stage_dir, os.path.basename(reffile))):
Input file is <a class="replace-pre" href="${os.path.relpath(reffile, pcontext.report_dir)}">${os.path.basename(reffile)}</a>.
% else:
No Jy/K factors file is specified. 
% endif
<table class="table table-bordered table-striped" summary="Jy/K factors">
    <thead>
	<tr><th>MS</th><th>Spw</th><th>Antenna</th><th>Pol</th><th>Factor</th></tr>
    </thead>
	<tbody>
	% for tr in jyperk_rows:
		<tr>
		% for td in tr:
			${td}
		% endfor
		</tr>
	%endfor
	</tbody>
</table>
<p/>