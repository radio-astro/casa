<%!
import cgi
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
	<h1>Sky Setup Details<button class="btn btn-default pull-right" onclick="javascript:window.history.back();">Back</button></h1>
</div>

<div class="row">      
% if azel_plot:
	<div class="col-md-4">
		<a href="${os.path.relpath(azel_plot.abspath, pcontext.report_dir)}" data-fancybox>
			<h3>Elevation vs. Azimuth</h3>
		</a>
        <div class="col-md-12">
		  	<div class="thumbnail">
				<a href="${os.path.relpath(azel_plot.abspath, pcontext.report_dir)}"
				   data-fancybox>
					<img class="lazyload"
                         data-src="${os.path.relpath(azel_plot.thumbnail, pcontext.report_dir)}"
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
	<div class="col-md-4">
		<a href="${os.path.relpath(el_vs_time_plot.abspath, pcontext.report_dir)}" data-fancybox>
			<h3>Elevation vs. Time</h3>
		</a>
        <div class="col-md-12">
		  	<div class="thumbnail">
				<a href="${os.path.relpath(el_vs_time_plot.abspath, pcontext.report_dir)}"
				   data-fancybox>
					<img class="lazyload"
                         data-src="${os.path.relpath(el_vs_time_plot.thumbnail, pcontext.report_dir)}"
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

%if plot_uv is not None:
	<div class="col-md-4">
        <a href="${os.path.relpath(plot_uv.abspath, pcontext.report_dir)}" data-fancybox>
            <h3>UV coverage</h3>
        </a>
        <div class="col-md-12">
            <div class="thumbnail">
                <a href="${os.path.relpath(plot_uv.abspath, pcontext.report_dir)}" data-fancybox>
                    <img class="lazyload"
                         data-src="${os.path.relpath(plot_uv.thumbnail, pcontext.report_dir)}"
                         title="UV coverage for ${ms.basename}, TARGET field ${cgi.escape(plot_uv.parameters['field_name'], True)} (#${plot_uv.parameters['field']}), spw ${plot_uv.parameters['spw']}."
                         alt="UV coverage for ${ms.basename}, ${cgi.escape(plot_uv.parameters['field_name'], True)} (#${plot_uv.parameters['field']}), spw ${plot_uv.parameters['spw']}"/>
                </a>
                <div class="caption">
                    <h4>UV Coverage</h4>
                    <p>UV coverage plot for TARGET field ${cgi.escape(plot_uv.parameters['field_name'], True)}
                        (#${plot_uv.parameters['field']}), spw ${plot_uv.parameters['spw']}.</p>
                </div>
            </div>
        </div>
    </div>
%endif

</div>

</body>
</html>
