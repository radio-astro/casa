<%!
rsc_path = ""
import os
%>
<%inherit file="t2-4m_details-base.html"/>
<%block name="header" />

<%block name="title">WVR Calibration and Flagging</%block>

<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>

<script>
$(document).ready(function() {
    // return a function that sets the SPW text field to the given spw
    var createSpwSetter = function(spw) {
        return function() {
            // trigger a change event, otherwise the filters are not changed
            $("#select-spw").select2("val", [spw]).trigger("change");
        };
    };

    // create a callback function for each overview plot that will select the
    // appropriate spw once the page has loaded
    $(".thumbnail a").each(function (i, v) {
        var o = $(v);
        var spw = o.data("spw");
        o.data("callback", createSpwSetter(spw));
    });

    $(".fancybox").fancybox({
        type: 'image',
        prevEffect: 'none',
        nextEffect: 'none',
        loop: false,
        helpers: {
            title: {
                type: 'outside'
            },
            thumbs: {
                width: 50,
                height: 50,
            }
        }
    });
});
</script>

<p>This task checks whether the WVR radiometers are working as intended,
interpolating for antennas that are not. The WVR caltable is only added to 
subsequent pre-applys if it gives a tangible improvement.</p>

<h2>Results</h2>
% if applications:
<h4>Flagging results and WVR application</h4>
<table class="table table-bordered" summary="Flagging Results">
	<caption>Flagging results and applications for WVR</caption>
    <thead>
        <tr>
            <th>Measurement Set</th>
			<th>WVR Table</th>
            <th>Interpolated Antennas</th>
			<th>Applied</th>
        </tr>
    </thead>
	<tbody>
% for application in applications:
		<tr>
			<td>${application.ms}</td>
		  	<td>${application.gaintable}
		  	<td>${application.interpolated}</td>
		  	<td><i class="icon-${'ok' if application.applied else 'remove'}"></i></td>
		</tr>
% endfor		
	</tbody>
</table>

<h4>CASA wvrgcal report</h4>
<table class="table table-bordered table-condensed" summary="Antenna/WVR Information">
	<caption>Antenna/WVR Information captured from CASA wvrgcal task</caption>
    <thead>
        <tr>
            <th>Measurement Set</th>
			<th>Antenna</th>
			<th>WVR?</th>
            <th>Flag?</th>
            <th>RMS</th>
            <th>Disc</th>
		</tr>
    </thead>
	<tbody>
% for ms in wvrinfos:
	% for wvrinfo in wvrinfos[ms]:
		<tr class="${'danger' if wvrinfo.flag else ''}">
			<td>${ms}</td>
		  	<td>${wvrinfo.antenna}</td>
		  	<td><i class="icon-${'ok' if wvrinfo.wvr else 'remove'}"></i></td>
		  	<td><i class="icon-${'ok' if wvrinfo.flag else 'remove'}"></i></td>
		  	<td>${wvrinfo.rms}</td>
		  	<td>${wvrinfo.disc}</td>
		</tr>
	%endfor
% endfor		
	</tbody>
</table>

% else:
<p>This measurement set does not contain any WVR data.</p>
% endif

% if flag_plots or phase_offset_summary_plots or baseline_summary_plots:
<h2>Plots</h2>

<p>The pipeline tests whether application of WVR correction improves the data
by performing a gaincal for a chosen field, usually the bandpass calibrator, 
and comparing the resulting phase corrections evaluated both with and without 
application of WVR correction. Plots based on these data in these evaluation 
caltables are presented below.</p>

% if flag_plots:
	<h3>Flagging plots</h3>

        <p>The following plots show the flagging metric used by the pipeline to
        determine which antennas to flag.</p>

	% for ms in flag_plots:
	    <h4>${ms}</h4>
	    <ul class="thumbnails">
	        % for plot in flag_plots[ms]:
	            % if os.path.exists(plot.thumbnail):
	            <li class="span3">
	                <div class="thumbnail">
	                    <a href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
	                       class="fancybox"
	                       rel="flag-${ms}">
	                        <img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
	                             title="Click to enlarge flag plot for Spectral Window ${plot.parameters['spw']}"
	                             data-thumbnail="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}">
	                        </img>
	                    </a>
	
	                    <div class="caption">
	                        <h4>Spectral Window ${plot.parameters['spw']}</h4>
	
	                        <p>Flagging plot for spectral window 
	                        ${plot.parameters['spw']}, all antennas.
	                        </p>
	                    </div>
	                </div>
	            </li>
	            % endif
	        % endfor
	    </ul>
	%endfor
%endif

% if metric_plots:
<h3>Flagging metric views</h3>

<p>Flagging metric views were calculated for various data 
intents. Click on a link below to show all flagging metric views for that 
measurement set.</p>

<ul>
% for ms, plot in metric_plots.items():
    <li>
        <a href="${os.path.relpath(os.path.join(dirname, 'flagging_metric-%s.html' % ms), pcontext.report_dir)}"
          class="replace">${ms}</a>
    </li>
% endfor        
</ul>

% endif

% if phase_offset_summary_plots:
	<h3>Phase correction with/without WVR</h3>
	
	<p>These plots show the deviation about the scan median phase before and after WVR application. 
	Points are plotted per integration and per correlation.</p>

	<p>Click the summary plots to enlarge, or click the summary title for a gallery of more
	detailed plots for individual antennas.</p> 

	% for ms in phase_offset_summary_plots:
	    <h4><a class="replace"
	           href="${os.path.relpath(os.path.join(dirname, 'phase_offsets-%s.html' % ms), pcontext.report_dir)}">${ms}</a>
	    </h4>
	    <ul class="thumbnails">
	        % for plot in phase_offset_summary_plots[ms]:
	            % if os.path.exists(plot.thumbnail):
	            <li class="span3">
	                <div class="thumbnail">
	                    <a href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
	                       class="fancybox"
	                       rel="offset-${ms}">
	                        <img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
	                             title="Click to show WVR phase offset plots for Spectral Window ${plot.parameters['spw']}"
	                             data-thumbnail="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}">
	                        </img>
	                    </a>
	
	                    <div class="caption">
		                    <h4>
			                    <a href="${os.path.relpath(os.path.join(dirname, 'phase_offsets-%s.html' % ms), pcontext.report_dir)}"
		    	                   class="replace"
		        	               data-spw="${plot.parameters['spw']}">
			                        Spectral Window ${plot.parameters['spw']}
			                    </a>
							</h4>
	
	                        <p>Deviation from scan median phase before and after WVR 
	                        application for spectral window 
	                        ${plot.parameters['spw']}, all antennas.
	                        </p>
	                    </div>
	                </div>
	            </li>
	            % endif
	        % endfor
	    </ul>
	%endfor
%endif

%if baseline_summary_plots:
	<h3>Phase correction vs distance to reference antenna</h3>

	<p>The lower panel of these plots show the median absolute deviation of the gaincal 
	corrections with and without WVR correction applied. The upper panel shows the ratio of
	the RMS deviations about the median for data with WVR correction applied to the RMS
	deviations without WVR correction.</p>

	<p>One plot is generated per scan, with data plotted per correlation and antenna 
	as a function of distance from the reference antenna.</p>

	<p>
	Click the summary plots to enlarge them, or the summary plot title to show a gallery of phase
	offset plots for individual antenna. 
	</p> 

	% for ms in baseline_summary_plots:
	    <h4><a class="replace"
	           href="${os.path.relpath(os.path.join(dirname, 'phase_offsets_vs_baseline-%s.html' % ms), pcontext.report_dir)}">${ms}</a>
	    </h4>
	    <ul class="thumbnails">
	        % for plot in baseline_summary_plots[ms]:
	            % if os.path.exists(plot.thumbnail):
	            <li class="span3">
	                <div class="thumbnail">
	                    <a href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
	                       class="fancybox"
	                       rel="baseline-${ms}">
	                        <img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
	                             title="Click to show WVR phase offset vs baseline plots for Spectral Window ${plot.parameters['spw']}"
	                             data-thumbnail="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}">
	                        </img>
	                    </a>
	
	                    <div class="caption">
							<h4>
			                    <a href="${os.path.relpath(os.path.join(dirname, 'phase_offsets_vs_baseline-%s.html' % ms), pcontext.report_dir)}"
			                       class="replace"
			                       data-spw="${plot.parameters['spw']}">
				                   Spectral Window ${plot.parameters['spw']}
			                    </a>
							</h4>
							
	                        <p>Phase offset (lower) and improvement ratio (upper) 
	                        vs distance to the reference antenna before and after 
	                        WVR application for spectral window 
	                        ${plot.parameters['spw']}, all antennas.
	                        </p>
	                    </div>
	                </div>
	            </li>
	            % endif
	        % endfor
	    </ul>
	%endfor
%endif

%endif