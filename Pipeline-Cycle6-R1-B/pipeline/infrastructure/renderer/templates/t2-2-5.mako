<%!
import os
%>
<html>
<head>
    <script>
        lazyload();
    </script>
</head>
<body>

<div class="page-header">
	<h1>Weather Details<button class="btn btn-default pull-right" onclick="javascript:window.history.back();">Back</button></h1>
</div>

% if weather_plot is not None and os.path.exists(weather_plot.abspath):
<ul class="thumbnails">
	<li class="span3">
	  	<div class="thumbnail">
			<a href="${os.path.relpath(weather_plot.abspath, pcontext.report_dir)}"
			   data-fancybox>
				<img class="lazyload"
                     data-src="${os.path.relpath(weather_plot.thumbnail, pcontext.report_dir)}"
					 title="Weather Details for ${ms.basename}"
					 alt="Weather Details for ${ms.basename}" />
		    </a>
		    <div class="caption">
				<h4>Weather</h4>
				<p>Weather during execution of ${ms.basename}.</p>
			</div>
		</div>
	</li>
</ul>
% else:
<p class="alert alert-error">No weather plot available.</p>
% endif

</body>
</html>
