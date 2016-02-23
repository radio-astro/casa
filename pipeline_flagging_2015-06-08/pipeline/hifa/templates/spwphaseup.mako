<%!
rsc_path = ""
import os

def format_spwmap(spwmap):
    if not spwmap:
        return ''
    else:
        return ', '.join([str(spwid) for spwid in spwmap])
%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="title">Compute Spw Phaseup Map and Offsets</%block>

<p>This task computes the spectral window map that will be used to apply the time gaincal phase solutions
and the caltable containing per spw phase offsets</p>

<h2>Results</h2>
<table class="table table-bordered table-striped" summary="Narrow to wide spw mapping results">
	<caption>Phase solution spw map per measurement set.</caption>
        <thead>
	    <tr>
	        <th>Measurement Set</th>
	        <th>Phaseup Spw Map</th>
	    </tr>
	</thead>
	<tbody>
    % for spwmap in spwmaps:
		<tr>
			<td>${os.path.basename(spwmap.ms)}</td>
			<td>${format_spwmap(spwmap.spwmap)}</td>
		</tr>
    % endfor
	</tbody>
</table>

<table class="table table-bordered" summary="Application Results">
        <caption>Applied calibrations and parameters used for caltable generation</caption>
    <thead>
        <tr>
            <th scope="col" rowspan="2">Measurement Set</th>
                        <th scope="col" colspan="2">Solution Parameters</th>
                        <th scope="col" colspan="2">Applied To</th>
            <th scope="col" rowspan="2">Calibration Table</th>
                </tr>
                <tr>
                        <th>Type</th>
            <th>Interval</th>
                        <th>Scan Intent</th>
                        <th>Spectral Windows</th>
        </tr>
    </thead>
        <tbody>
% for application in applications:
                <tr>
                        <td>${application.ms}</td>
                        <td>${application.solint}</td>
                        <td>${application.calmode}</td>
                        <td>${application.intent}</td>
                        <td>${application.spw}</td>
                        <td>${application.gaintable}</td>
                </tr>
% endfor
        </tbody>
</table>

