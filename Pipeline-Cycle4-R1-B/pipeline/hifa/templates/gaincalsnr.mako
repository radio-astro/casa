<%!
rsc_path = ""
%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="title">Estimate gaincal SNR</%block>

<h2>Results</h2>

<h4>Gaincal SNR Values</h4>

<p>The gain calibration sensitivities and SNRs are estimated</p>

<table class="table table-bordered table-striped" summary="Gaincal SNR values">
	<caption>Estimate gaincal SNR values</caption>
       <thead>
	    <tr>
	        <th scope="col" rowspan="2">Measurement Set</th>
	        <th scope="col" rowspan="2">Spw</th>
	        <th scope="col" rowspan="2">Sensitivity (mJy)</th>
	        <th scope="col" rowspan="2">SNR</th>
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
