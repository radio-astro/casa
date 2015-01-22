<%!
rsc_path = ""

%>

<%inherit file="t2-4m_details-base.html"/>

<%block name="title">Compute Bandpass Smoothing Parameters</%block>

<h2>Results</h2>

<h4>Bandpass Smoothing Parameters</h4>

<p>The bandpass smoothing parameters required to achieve the required SNR are computed and stored in the pipeline context</p>

<div class="row_fluid">
<table class="table table-bordered table-striped" summary="Bandpass smoothing parameters">
	<caption>Recommended Bandpass Smoothing Parameters</caption>
       <thead>
	    <tr>
	        <th scope="col" rowspan="2">Measurement Set</th>
	        <th scope="col" rowspan="2">Spw</th>
	        <th scope="col" colspan="2">Phaseup Solution Parameters</th>
	        <th scope="col" colspan="2">Bandpass Solution Parameters</th>
	    </tr>
	    <tr>
	        <th scope="col">Time Interval(sec, # integrations)</th>
	        <th scope="col"># Points</th>
	        <th scope="col">Frequency Interval (MHz, # channels)</th>
	        <th scope="col"># Points</th>
	    </tr>
	</thead>
	<tbody>
	% for tr in table_rows:
		<tr>
		% for td in tr:
			${td}
		% endfor
		</tr>
	%endfor
	</tbody>
</table>
</div>








