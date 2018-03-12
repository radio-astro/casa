<%!
rsc_path = "../../"
import os.path
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<html>
<head>
    <script>
        lazyload();
    </script>
</head>
<body>

<div class="page-header">
	<h1>Antenna Setup Details<button class="btn btn-default pull-right" onclick="javascript:window.history.back();">Back</button></h1>
</div>

<div class="tabbable">
	<ul class="nav nav-tabs">
		<li class="active"><a href="#tabs-offsets" data-toggle="tab">Antennas</a></li>
		<li><a href="#tabs-baselines" data-toggle="tab">Baselines</a></li>
	</ul>
	<div class="tab-content">
		<div class="tab-pane active" id="tabs-offsets">
		%if plot_ants is not None or plot_ants_plog is not None:
			<div class="row">
				<h3>Antenna Positions</h3>	
				%if plot_ants is not None:
					<div class="col-md-3">
					  	<div class="thumbnail">
							<a href="${os.path.relpath(plot_ants.abspath, pcontext.report_dir)}"
							   data-fancybox>
								<img class="lazyload"
                                     data-src="${os.path.relpath(plot_ants.thumbnail, pcontext.report_dir)}"
									 title="Antenna Latitude vs. Antenna Longitude for ${ms.basename}"
	 							     alt="Antenna Latitude vs. Antenna Longitude for ${ms.basename}"/>
						    </a>
						    <div class="caption">
								<h4>Antenna Position</h4>
								<p>Plot antenna latitude vs antenna longitude</p>
							</div>
						</div>
					</div>
				%endif
				%if plot_ants_plog is not None:
					<div class="col-md-3">
					  	<div class="thumbnail">
							<a href="${os.path.relpath(plot_ants_plog.abspath, pcontext.report_dir)}"
							   data-fancybox>
								<img class="lazyload"
                                     data-src="${os.path.relpath(plot_ants_plog.thumbnail, pcontext.report_dir)}"
									 title="Antenna Positions (polar-logarithmic) for ${ms.basename}"
	 								 alt="Antenna Positions (polar-logarithmic) for ${ms.basename}"/>
						    </a>
						    <div class="caption">
								<h4>Antenna Position</h4>
								<p>Polar-logarithmic plot of antenna positions.</p>
							</div>
						</div>
					</div>
				%endif
			</div>
		%endif
			<div class="row">
				<h3>Antenna Details</h3>
				<div>
					<table class="table table-bordered table-striped table-condensed" summary="Antenna Details for ${ms.basename}">
						<caption>Antenna Details and Offsets for ${ms.basename}</caption>
					    <thead>
					        <tr>
					            <th scope="col" rowspan="2">ID</th>
					            <th scope="col" rowspan="2">Name</th>
					            <th scope="col" rowspan="2">Pad</th>
					            <th scope="col" rowspan="2">Diameter</th>
					            <th scope="col" colspan="2">Offset from Array Centre</th>
					        </tr>
							<tr>
					            <th scope="col">Longitude</th>
					            <th scope="col">Latitude</th>			
							</tr>
					    </thead>
						<tbody>
							% for antenna in ms.antennas:
							<%
								(offset_x, offset_y) = ms.antenna_array.get_offset(antenna)
							%>
							<tr>
							  <td>${antenna.id}</td>
							  <td>${antenna.name}</td>
							  <td>${antenna.station}</td>
							  <td>${str(antenna.diameter)}</td>
							  <td>${str(offset_x)}</td>
							  <td>${str(offset_y)}</td>
							</tr>
							% endfor
						</tbody>
					</table>
				</div>
			</div>
		</div>
		<div class="tab-pane" id="tabs-baselines">		
			<h3>Baseline Summary</h3>
	
			<%
			baselines = sorted(ms.antenna_array.baselines, key=lambda b: b.length)
			num_baselines = len(ms.antenna_array.baselines)
			min_baseline = ms.antenna_array.min_baseline
			max_baseline = ms.antenna_array.max_baseline
			%>
			
			<table class="table table-bordered table-striped table-condensed" summary="Antenna and Baseline Summary">
				<caption>Summary of Baselines in ${ms.basename}</caption>
			    <thead>
			        <tr>
			            <th scope="col" colspan="2">Minimum Baseline</th>
			            <th scope="col" colspan="2">Maximum Baseline</th>
			        </tr>
					<tr>
			            <th scope="col">Length</th>
			            <th scope="col">Antenna Pair</th>			
			            <th scope="col">Length</th>
			            <th scope="col">Antenna Pair</th>			
					</tr>
			    </thead>
				<tbody>
					<tr>
					  <td>${str(min_baseline.length)}</td>
					  <td>${min_baseline.antenna1.name} - ${min_baseline.antenna2.name}</td>
					  <td>${str(max_baseline.length)}</td>
					  <td>${max_baseline.antenna1.name} - ${max_baseline.antenna2.name}</td>
					</tr>
				</tbody>
			</table>
			
			<h3>Baseline Details</h3>
			<p>In the table below, the 'percentile' column defines in which percentile the baseline length falls, where the longest baseline is 100%.</p>
	
			<table class="table table-bordered table-striped table-condensed" summary="Antenna Details for ${ms.basename}">
				<caption>
					Baselines for ${ms.basename}
				</caption>
				<thead>
					<tr>
						<th scope="col" rowspan="1">Antenna 1</th>
						<th scope="col" rowspan="1">Antenna 2</th>
						<th scope="col" rowspan="1">Baseline Length</th>
						<th scope="col" rowspan="1">Percentile (%)</th>
					</tr>
				</thead>
				<tbody>
					% for (i, baseline) in enumerate(baselines, start=1):
					<tr>
						<td>${baseline.antenna1.name}</td>
						<td>${baseline.antenna2.name}</td>
						<td>${str(baseline.length)}</td>
						<td>${round(100.0*i/num_baselines, 1)}</td>
					</tr>
					% endfor
				</tbody>
			</table>
		</div>
	</div>
</div>

</body>
</html>
