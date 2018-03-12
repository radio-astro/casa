<%!
import pipeline.infrastructure.renderer.htmlrenderer as hr
import pipeline.infrastructure.utils as utils
%>
<html>
<body>

<div class="page-header">
	<h1>Scan Details<button class="btn btn-default pull-right" onclick="javascript:window.history.back();">Back</button></h1>
</div>

<div id="tabbable">
	<ul class="nav nav-tabs">
		<li class="active"><a href="#tabs-science" data-toggle="tab">Science Scans</a></li>
		<li><a href="#tabs-all" data-toggle="tab">All Scans</a></li>
	</ul>
	<div class="tab-content">
		<div class="tab-pane active" id="tabs-science">
			<h2>Science Scans</h2>
			<table class="table table-bordered table-condensed table-striped" summary="Science Scans in ${ms.basename}">
				<caption>Scans with Science Intent in ${ms.basename}</caption>
			    <thead>
			        <tr>
			            <th scope="col" rowspan="2">ID</th>
			            <th scope="col" colspan="3">Time</th>
			            <th scope="col" rowspan="2">Spws</th>
			            <th scope="col" rowspan="2">Fields</th>
			            <th scope="col" rowspan="2">Intents</th>
			        </tr>
					<tr>
			            <th scope="col">Start</th>
			            <th scope="col">End</th>			
			            <th scope="col">Duration</th>			
					</tr>
			    </thead>
				<tbody>
					<% sciencerows = [r for r in tablerows if 'TARGET' in r.intents] %>
					% for row in sciencerows:
					<tr>
					  <td>${row.id}</td>
					  <td>${row.time_start}</td>
					  <td>${row.time_end}</td>
					  <td>${row.duration}</td>
					  <td>${row.spws}</td>
					  <td>${row.fields}</td>
					  <td>${utils.commafy(row.intents)}</td>
					</tr>
					% endfor
				</tbody>
			</table>
		</div>
		<div class="tab-pane" id="tabs-all">
			<h2>All Scans</h2>
			<table class="table table-bordered table-striped table-condensed" summary="All Scans in ${ms.basename}">
				<caption>All Scans in ${ms.basename}</caption>
			    <thead>
			        <tr>
			            <th scope="col" rowspan="2">ID</th>
			            <th scope="col" colspan="3">Time</th>
			            <th scope="col" rowspan="2">Spws</th>
			            <th scope="col" rowspan="2">Fields</th>
			            <th scope="col" rowspan="2">Intents</th>
			        </tr>
					<tr>
			            <th scope="col">Start</th>
			            <th scope="col">End</th>			
			            <th scope="col">Duration</th>			
					</tr>
			    </thead>
				<tbody>
					% for row in tablerows:
					<tr>
					  <td>${row.id}</td>
					  <td>${row.time_start}</td>
					  <td>${row.time_end}</td>
					  <td>${row.duration}</td>
					  <td>${row.spws}</td>
					  <td>${row.fields}</td>
					  <td>${utils.commafy(row.intents)}</td>
					</tr>
					% endfor
				</tbody>
			</table>
		</div>
	</div>

</div>

</body>
</html>
