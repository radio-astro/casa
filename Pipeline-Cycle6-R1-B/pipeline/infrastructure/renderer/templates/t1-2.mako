<%!
navbar_active='Observation Summary'

import datetime
import pipeline.domain.measures as measures
import pipeline.infrastructure.utils as utils
%>
<%inherit file="base.mako"/>

<%block name="title">Observation Summary</%block>

<div class="page-header">
<h1>Measurement Sets</h1>
</div>

<table class="table table-bordered table-striped table-condensed" 
	   summary="Measurement Set Summaries">
	<caption>Measurement Set Summaries</caption>
    <thead>
        <tr>
            <th scope="col" rowspan="2">Measurement Set</th>
            <th scope="col" rowspan="2">Receivers</th>
            <th scope="col" rowspan="2">Num Antennas</th>
            <th scope="col" colspan="3">Time (UTC)</th>
			<!-- break heading divider for subcolumns -->
            <th scope="col" colspan="3">Baseline Length</th>
            <th scope="col" rowspan="2">Size</th>
        </tr>
        <tr>
            <th scope="col">Start</th>
            <th scope="col">End</th>
            <th scope="col">On Source</th>
            <th scope="col">Min</th>
            <th scope="col">Max</th>
            <th scope="col">RMS</th>
        </tr>
    </thead>
	<tbody>
		% for row in ms_summary_rows:
		<tr>
		  <td><a href="${row.href}">${row.ms}</a></td>
		  <td>${utils.commafy(row.receivers)}</td>
		  <td>${row.num_antennas}</td>
		  <td>${row.time_start}</td>
		  <td>${row.time_end}</td>
		  <td>${row.time_on_source}</td>
		  <td>${str(row.baseline_min)}</td>
		  <td>${str(row.baseline_max)}</td>
		  <td>${str(row.baseline_rms)}</td>
		  <td>${str(row.filesize)}</td>
		</tr>
		% endfor
	</tbody>
</table>

<p>Show more information on a measurement set by clicking on its name.</p>

<!-- 
<a href="t2-1.html">Details per ASDM</a>
 -->