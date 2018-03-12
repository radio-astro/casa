<%!
import pipeline.infrastructure.renderer.htmlrenderer as hr
import pipeline.domain.measures as measures
%>
<html>
<body>

<div class="page-header">
	<h1>Spectral Setup Details<button class="btn btn-default pull-right" onclick="javascript:window.history.back();">Back</button></h1>
</div>

<div id="tabbable">
	<ul class="nav nav-tabs">
		<li class="active"><a href="#tabs-science" data-toggle="tab">Science Windows</a></li>
		<li><a href="#tabs-all" data-toggle="tab">All Windows</a></li>
	</ul>
	<div class="tab-content">
		<div class="tab-pane active" id="tabs-science">		
			<h2>Science Windows</h2>
			<table class="table table-bordered table-striped table-condensed" summary="Science Spectral Windows in ${ms.basename}">
				<caption>Spectral Windows with Science Intent in ${ms.basename}</caption>
			    <thead>
			        <tr>
			            <th scope="col" rowspan="2">ID</th>
			            <%
						spwtypelabel='<th scope="col" rowspan="2">Type</th>'
						if 'VLA' in pcontext.project_summary.telescope:
					                spwtypelabel=''
						endif
						%>
						${spwtypelabel}
			            <th scope="col" colspan="3">Frequency ${'(%s)' % (ms.get_spectral_windows()[0].frame)}</th>
			            <th scope="col" rowspan="2">Bandwidth ${'(%s)' % (ms.get_spectral_windows()[0].frame)}</th>
			            <th scope="col" rowspan="2">Transitions</th>
			            <th scope="col" colspan="3">Channels ${'(%s)' % (ms.get_spectral_windows()[0].frame)}</th>
			            <th scope="col" rowspan="2">Correlator Axis</th>
			            <th scope="col" rowspan="2">Band</th>
						<%
						basebandlabel=''
						if pcontext.project_summary.telescope != 'ALMA':
                                        basebandlabel = '<th scope="col" rowspan="2">Baseband</th>'
						endif
						%>
						${basebandlabel}
			        </tr>
			        <tr>
			        	<th>Start</th>
			        	<th>Centre</th>
			        	<th>End</th>
			        	<th>Number</th>
			        	<th>Frequency Width</th>
			        	<th>Velocity Width</th>
			        </tr>
			    </thead>
				<tbody>
					% for spw in ms.get_spectral_windows(science_windows_only=True):
					<tr>
					  <td>${spw.id}</td>
			            <%
						spwtypeentry='<td>'+str(spw.type)+'</td>'
						if 'VLA' in pcontext.project_summary.telescope:
					                spwtypeentry=''
						endif
						%>
						${spwtypeentry}
					  <td>${str(spw.min_frequency)}</td>
					  <td>${str(spw.centre_frequency)}</td>
					  <td>${str(spw.max_frequency)}</td>
					  <td>${str(spw.bandwidth)}</td>
					  <td>${', '.join(spw.transitions)}</td>
					  <td>${spw.num_channels}</td>
					  <td>${spw.channels[0].getWidth()}</td>
					  <td>${str(measures.LinearVelocity(299792458 * spw.channels[0].getWidth().to_units(measures.FrequencyUnits.HERTZ) / spw.centre_frequency.to_units(measures.FrequencyUnits.HERTZ), measures.LinearVelocityUnits.METRES_PER_SECOND))}</td>
					  <td>${', '.join(sorted(ms.get_data_description(spw=spw).corr_axis))}</td>
					  <td>${spw.band}</td>
					  <%
						basebanditem=''
						if pcontext.project_summary.telescope != 'ALMA':
						    try:
						        basebandstring = spw.name.split('#')[1]
						        basebanditem = '<td>'+basebandstring+'</td>'
						    except:
						        basebanditem = '<td></td>'
						endif
					  %>
					  ${basebanditem}
					</tr>
					% endfor
				</tbody>
			</table>
		</div>
	
		<div class="tab-pane" id="tabs-all">
			<h2>All Windows</h2>
			<table class="table table-bordered table-striped table-condensed" summary="Spectral Windows in ${ms.basename}">
				<caption>All Spectral Windows in ${ms.basename}</caption>
			    <thead>
			        <tr>
			            <th scope="col" rowspan="2">ID</th>
			            <%
						spwtypelabel='<th scope="col" rowspan="2">Type</th>'
						if 'VLA' in pcontext.project_summary.telescope:
						            spwtypelabel=''
						endif
						%>
						${spwtypelabel}
			            <th scope="col" colspan="3">Frequency ${'(%s)' % (ms.get_spectral_windows()[0].frame)}</th>
			            <th scope="col" rowspan="2">Bandwidth ${'(%s)' % (ms.get_spectral_windows()[0].frame)}</th>
			            <th scope="col" rowspan="2">Transitions</th>
			            <th scope="col" colspan="3">Channels ${'(%s)' % (ms.get_spectral_windows()[0].frame)}</th>
			            <th scope="col" rowspan="2">Correlator Axis</th>
			            <th scope="col" rowspan="2">Band</th>
						<%
						     basebandlabel=''
						     if pcontext.project_summary.telescope != 'ALMA':
                                 basebandlabel = '<th scope="col" rowspan="2">Baseband</th>'
						     endif
						%>
						${basebandlabel}
			            <th scope="col" rowspan="2">Intents</th>
			        </tr>
			        <tr>
			        	<th>Start</th>
			        	<th>Centre</th>
			        	<th>End</th>
			        	<th>Number</th>
			        	<th>Frequency Width</th>
			        	<th>Velocity Width</th>
			        </tr>
			    </thead>
				<tbody>
					% for spw in ms.get_spectral_windows(science_windows_only=False):
					<tr>
						<td>${spw.id}</td>
			            <%
						spwtypeentry='<td>'+str(spw.type)+'</td>'
						if 'VLA' in pcontext.project_summary.telescope:
					                spwtypeentry=''
						endif
						%>
						${spwtypeentry}
						<td>${str(spw.min_frequency)}</td>
						<td>${str(spw.centre_frequency)}</td>
						<td>${str(spw.max_frequency)}</td>
						<td>${str(spw.bandwidth)}</td>
					        <td>${','.join(spw.transitions)}</td>
						<td>${spw.num_channels}</td>
						<td>${spw.channels[0].getWidth()}</td>
						<td>${str(measures.LinearVelocity(299792458 * spw.channels[0].getWidth().to_units(measures.FrequencyUnits.HERTZ) / spw.centre_frequency.to_units(measures.FrequencyUnits.HERTZ), measures.LinearVelocityUnits.METRES_PER_SECOND))}</td>
						<%
							dd = ms.get_data_description(spw=spw)
							if dd is None:
								polarizations = ""
							else:
								polarizations = ', '.join(sorted(dd.corr_axis))
						%>
						<td>${polarizations}</td>
						<td>${spw.band}</td>
						<%
						    basebanditem=''
						    if pcontext.project_summary.telescope != 'ALMA':
						        try:
						            basebandstring = spw.name.split('#')[1]
						            basebanditem = '<td>'+basebandstring+'</td>'
						        except:
						            basebanditem = '<td></td>'
						    endif
					     %>
					     ${basebanditem}
				  		<td>${', '.join(sorted([i for i in spw.intents]))}</td>
					</tr>
					% endfor
				</tbody>
			</table>
		</div>
	</div>
</div>

</body>
</html>