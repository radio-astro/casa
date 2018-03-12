<%!
import os
%>
<html>
<body>

<div class="page-header">
	<h1>Sky Setup Details<button class="btn btn-default pull-right" onclick="javascript:window.history.back();">Back</button></h1>
</div>

<div class="row">      
% if azel_plot:
	<div class="col-md-6">
		<a href="${os.path.relpath(azel_plot.abspath, pcontext.report_dir)}" data-fancybox>
			<h3>Elevation vs. Azimuth</h3>
		</a>
		<div class="col-md-6">
		  	<div class="thumbnail">
				<a href="${os.path.relpath(azel_plot.abspath, pcontext.report_dir)}"
				   data-fancybox>
					<img src="${os.path.relpath(azel_plot.thumbnail, pcontext.report_dir)}"
						 title="AzEl for ${ms.basename}"
						 alt="AzEl Details for ${ms.basename}" />
			    </a>
			    <div class="caption">
					<h4>Elevation vs. azimuth</h4>
				</div>
			</div>
		</div>
	</div>
% endif

% if el_vs_time_plot:
	<div class="col-md-6">
		<a href="${os.path.relpath(el_vs_time_plot.abspath, pcontext.report_dir)}" data-fancybox>
			<h3>Elevation vs. Time</h3>
		</a>
		<div class="col-md-6">
		  	<div class="thumbnail">
				<a href="${os.path.relpath(el_vs_time_plot.abspath, pcontext.report_dir)}"
				   data-fancybox>
					<img src="${os.path.relpath(el_vs_time_plot.thumbnail, pcontext.report_dir)}"
						 title="ElTime for ${ms.basename}"
						 alt="ElTime Details for ${ms.basename}" />
			    </a>
			    <div class="caption">
					<h4>Elevation vs. time</h4>
				</div>
			</div>
		</div>
	</div>
% endif

</div>

</body>
</html>
