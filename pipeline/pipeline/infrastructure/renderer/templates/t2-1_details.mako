<%!
rsc_path = "./"

import os
%>

<script>
$(document).ready(function(){
	$(".fancybox").fancybox();
});
</script>

<!--  Add image holder library for missing plots -->
<script src="${self.attr.rsc_path}resources/js/holder.js"></script>

<div class="page-header">
<h1>Overview of '${ms.basename}'</h1>
</div>

<div class="row">
	<div class="col-md-6">
		<h3>Observation Execution Time</h3>
	
		<table class="table table-condensed" summary="Data Details">
			<tbody>
				<tr>
					<th>Start Time</th>
					<td>${time_start}</td>
				</tr>
				<tr>
					<th>End Time</th>
					<td>${time_end}</td>
				</tr>
				<tr>
					<th>Total Time on Source</th>
					<td>${time_on_source}</td>
				</tr>
				<tr>
					<th>Total Time on Science Target</th>
					<td>${time_on_science}</td>
				</tr>
			</tbody>
		</table>

		<a class="btn replace-pre"
                 href='${os.path.join(dirname, "listobs.txt")}'>
			listobs output
		</a>
	</div>
		
	<div class="col-md-6">
		<div class="col-md-6">
		  	<div class="thumbnail">
				<a href="${os.path.relpath(intent_vs_time.abspath, pcontext.report_dir)}"
				   class="fancybox">
					<img src="${os.path.relpath(intent_vs_time.thumbnail, pcontext.report_dir)}"
						 title="Intent vs. Time for ${ms.basename}"
						 alt="Intent vs. Time for ${ms.basename}" />
			    </a>
			    <div class="caption">
					<h4>Intent vs Time</h4>
					<p>Track scan intent vs time</p>
				</div>
			</div>
		</div>
	
		<div class="col-md-6">		
		  	<div class="thumbnail">
				<a href="${os.path.relpath(field_vs_time.abspath, pcontext.report_dir)}"
				   class="fancybox">
					<img src="${os.path.relpath(field_vs_time.thumbnail, pcontext.report_dir)}"
						 title="Field vs. Time for ${ms.basename}"
						 alt="Field vs. Time for ${ms.basename}" />
			    </a>
			    <div class="caption">
					<h4>Field vs Time</h4>
					<p>Track observed field vs time</p>
				</div>
			</div>
		</div>
	</div>
		
</div>


<div class="row">
	<div class="col-md-6">
		<a class="replace" href='${os.path.join(dirname, "t2-2-1.html")}'><h3>Spatial Setup</h3></a>
		<table class="table table-condensed" summary="Spatial Setup Summary">
			<tbody>
				<tr>
					<th>Science Targets</th>
					<td>${science_sources}</td>
				</tr>
				<tr>
					<th>Calibrators</th>
					<td>${calibrators}</td>
				</tr>
			</tbody>
		</table>
	</div>
	<div class="col-md-6">
		<a class="replace" href='${os.path.join(dirname, "t2-2-2.html")}'><h3>Spectral Setup</h3></a>
		<table class="table table-condensed" summary="Spectral Setup Summary">
			<tbody>
				<tr>
					<th>All Bands</th>
					<td>${all_bands}</td>
				</tr>
				<tr>
					<th>Science Bands</th>
					<td>${science_bands}</td>
				</tr>
				${vla_basebands}
			</tbody>
		</table>
	</div>
</div>

<div class="row">
	<div class="col-md-6">
		<a class="replace" href='${os.path.join(dirname, "t2-2-3.html")}'><h3>Antenna Setup</h3></a>
		<table class="table table-condensed" summary="Antenna Setup Summary">
			<tbody>
				<tr>
					<th>Min Baseline</th>
					<td>${baseline_min}</td>
				</tr>
				<tr>
					<th>Max Baseline</th>
					<td>${baseline_max}</td>
				</tr>
				<tr>
					<th>Number of Baselines</th>
					<td>${num_baselines}</td>
				</tr>
				<tr>
					<th>Number of Antennas</th>
					<td>${num_antennas}</td>
				</tr>
			</tbody>
		</table>
	</div>
	<div class="col-md-6">
		<a class="replace" href='${os.path.join(dirname, "t2-2-4.html")}'><h3>Sky Setup</h3></a>
		<table class="table table-condensed" summary="Sky Setup Summary">
			<tbody>
				<tr>
					<th>Min Elevation</th>
					<td>${el_min} degrees</td>
				</tr>
				<tr>
					<th>Max Elevation</th>
					<td>${el_max} degrees</td>
				</tr>
			</tbody>
		</table>
	</div>
</div>



<div class="row">        
	<div class="col-md-6">
        % if weather_plot is None or not os.path.exists(weather_plot.thumbnail):
        <h3>Weather</h3>
        <div class="col-md-6">
            <div class="thumbnail">
                <img data-src="holder.js/250x188/text:Not Available">
                <div class="caption">
                    <h4>Weather plot</h4>
                </div>
            </div>
        </div>
        % else:
		<a href="${os.path.relpath(weather_plot.abspath, pcontext.report_dir)}" class="fancybox">
			<h3>Weather</h3>
		</a>
		<div class="col-md-6">
		  	<div class="thumbnail">
				<a href="${os.path.relpath(weather_plot.abspath, pcontext.report_dir)}"
				   class="fancybox">
					<img src="${os.path.relpath(weather_plot.thumbnail, pcontext.report_dir)}"
						 title="Weather Details for ${ms.basename}"
						 alt="Weather Details for ${ms.basename}" />
			    </a>
			    <div class="caption">
					<h4>Weather plot</h4>
				</div>
			</div>
		</div>
        % endif
	</div>

	% if pcontext.project_summary.telescope.lower() in ('alma') and pwv_plot is not None:
	
	<div class="col-md-6">
		<a href="${os.path.relpath(pwv_plot.abspath, pcontext.report_dir)}" class="fancybox">
			<h3>PWV</h3>
		</a>
		<div class="col-md-6">
		  	<div class="thumbnail">
				<a href="${os.path.relpath(pwv_plot.abspath, pcontext.report_dir)}"
				   class="fancybox">
					<img src="${os.path.relpath(pwv_plot.thumbnail, pcontext.report_dir)}"
						 title="PWV Details for ${ms.basename}"
						 alt="PWV Details for ${ms.basename}" />
			    </a>
			    <div class="caption">
					<h4>PWV plot</h4>
				</div>
			</div>
		</div>
	</div>
	
	% endif
	
	<div class="col-md-6">
		<a class="replace" href='${os.path.join(dirname, "t2-2-6.html")}'><h3>Scans</h3></a>
	</div>	
	
	% if pcontext.project_summary.telescope.lower() in ('alma', 'nro') and is_singledish == True:
	
	<div class="col-md-6">
		<a class="replace" href="${os.path.join(dirname, "t2-2-7.html")}">
			<h3>Telescope Pointing</h3>
		</a>
		<div class="col-md-6">
		    <div class="thumbnail">
		        <a href="${os.path.relpath(pointing_plot.abspath, pcontext.report_dir)}"
		            class="fancybox">
		           <img src="${os.path.relpath(pointing_plot.thumbnail, pcontext.report_dir)}"
		               title="Telescope Pointing for ${ms.basename} Field ${pointing_plot.parameters['field']}"
		               alt="Telescope Pointing for ${ms.basename} Field ${pointing_plot.parameters['field']}" />
		        </a>
		        <div class="caption">
		            <h4>Telescope Pointing Plot</h4>
		        </div>
		    </div>
		</div>
	</div>
	
	% endif
</div>

<!-- 
<div class="fancybox-overlay fancybox-overlay-fixed" style="width: auto; height: auto; display: block; ">
	<div class="fancybox-wrap fancybox-desktop fancybox-type-image fancybox-opened" tabindex="-1" style="width: 912px; height: auto; position: absolute; top: 20px; left: 205px; opacity: 1; overflow: visible; ">
		<div class="fancybox-skin" style="padding: 15px; width: auto; height: auto; ">
			<div class="fancybox-outer">
				<div class="fancybox-inner" style="overflow: visible; width: 584px; height: 376px;">
					<img class="fancybox-image" src="sessionSession_default/vla_m81.avg.raw/intent_vs_time.png" alt="" style="width:50%; height: 50%; float:right">
					<img class="fancybox-image" src="sessionSession_default/vla_m81.avg.raw/field_vs_time.png" alt="" style="width:50%; height: 50%; float:right">
					<img class="fancybox-image" src="sessionSession_default/vla_m81.avg.raw/intent_vs_time.png" alt="" style="width:50%; height: 50%;  float:right">
					<img class="fancybox-image" src="sessionSession_default/vla_m81.avg.raw/intent_vs_time.png" alt="" style="width:50%; height: 50%; float:right">
				</div>
				<a title="Previous" class="fancybox-nav fancybox-prev" href="javascript:;"><span></span></a>
				<a title="Next" class="fancybox-nav fancybox-next" href="javascript:;"><span></span></a>
			</div>
			<a title="Close" class="fancybox-item fancybox-close" href="javascript:;"></a>
		</div>
	</div>
</div>
 -->
