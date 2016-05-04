<%!
rsc_path = ""
import os
%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="header" />

<%block name="title">Single-Dish Sky Calibration</%block>

<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>

<script>
$(document).ready(function() {
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

<p>This task generates a sky calibration table, a collection of OFF spectra for single dish data calibration.</p>

<h2>Results</h2>
<table class="table table-bordered" summary="Application Results">
	<caption>Applied calibrations and parameters used for caltable generation</caption>
    <thead>
        <tr>
            <th scope="col" rowspan="2">Measurement Set</th>
			<th scope="col" colspan="1">Solution Parameters</th>
			<th scope="col" colspan="4">Applied To</th>
            <th scope="col" rowspan="2">Calibration Table</th>
		</tr>
		<tr>
			<th>Type</th>
			<th>Field</th>
			<th>Scan Intent</th>
			<th>Antennas</th>
			<th>Spectral Windows</th>
        </tr>
    </thead>
	<tbody>
% for application in applications:
		<tr>
			<td>${application['ms']}</td>
		  	<td>${application['caltype']}</td>
		  	<td>${application['field']}</td>
		  	<td>${application['intent']}</td>
		  	<td>${application['antenna']}</td>
		  	<td>${application['spw']}</td>
		  	<td>${application['gaintable']}</td>
		</tr>
% endfor		
	</tbody>
</table>

% if len(reference_coords) > 0:
    <h2>Reference Coordinates</h2>
    <table class="table table-bordered" summary="Reference Coordinates">
	    <caption>Reference coordinates for position-switch calibration</caption>
        <thead>
            <tr>
                <th scope="col">Measurement Set</th>
			    <th scope="col">Field</th>
			    <th scope="col">Reference Coordinate</th>
		    </tr>
        </thead>
	    <tbody>
	    % for vis, coords in reference_coords.items():
	        <% isfirst = True %>
	        % for field, coord in coords.items():
	            <tr>
	                % if isfirst:
	                    <% isfirst = False %>
                        <td rowspan=${len(coords)}>${vis}</td>
                    % endif
	                <td>${field}</td>
	                <td>${coord}</td>
	            </tr>
	        % endfor
	    % endfor
	    </tbody>
    </table>
% endif

<h2>Amp vs. Frequency Plots</h2>
% for ms in pcontext.observing_run.measurement_sets:
    <% 
        vis = ms.basename 
        subpage = os.path.join(dirname, amp_vs_freq_subpages[vis])
    %>
    <h4><a class="replace" href="${subpage}">${vis}</a></h4>
    % for plot in summary_amp_vs_freq[vis]:
        % if os.path.exists(plot.thumbnail):
            <% 
                img_path = os.path.relpath(plot.abspath, pcontext.report_dir)
                thumbnail_path = os.path.relpath(plot.thumbnail, pcontext.report_dir)
                ant = plot.parameters['ant']
                field = plot.parameters['field']
            %>
 	        <div class="col-md-3">
	            <div class="thumbnail">
	                <a href="${img_path}" class="fancybox" rel="thumbs">
	                    <img src="${thumbnail_path}"
	                         title="Sky level summary for Field ${field}, Antenna ${ant}"
	                         data-thumbnail="${thumbnail_path}">
	                </a>
	                <div class="caption">
	                    <h4>
	                        <a href="${subpage}" class="replace" data-field="${field}" data-ant="${ant}">
	                           Field ${field} Antenna ${ant}
	                        </a>
	                    </h4>
	
	                    <p>Plot of sky level vs frequency for field ${field}, antenna ${ant}.</p>
	                </div>
	            </div>
	        </div>
        % endif
    % endfor
	<div class="clearfix"></div><!--  flush plots, break to next row -->
% endfor

<h2>Amp vs. Time Plots</h2>
% for ms in pcontext.observing_run.measurement_sets:
    <% 
        vis = ms.basename 
        subpage = os.path.join(dirname, amp_vs_time_subpages[vis])
    %>
    <h4><a class="replace" href="${subpage}">${vis}</a></h4>
    % for plot in summary_amp_vs_time[vis]:
        % if os.path.exists(plot.thumbnail):
            <% 
                img_path = os.path.relpath(plot.abspath, pcontext.report_dir)
                thumbnail_path = os.path.relpath(plot.thumbnail, pcontext.report_dir)
                ant = plot.parameters['ant']
                field = plot.parameters['field']
            %>
 	        <div class="col-md-3">
	            <div class="thumbnail">
	                <a href="${img_path}" class="fancybox" rel="thumbs">
	                    <img src="${thumbnail_path}"
	                         title="Sky level summary for Field ${field}, Antenna ${ant}"
	                         data-thumbnail="${thumbnail_path}">
	                </a>
	                <div class="caption">
	                    <h4>
	                        <a href="${subpage}" class="replace" data-field="${field}" data-ant="${ant}">
	                           Field ${field} Antenna ${ant}
	                        </a>
	                    </h4>
	
	                    <p>Plot of sky level vs time for field ${field}, antenna ${ant}.</p>
	                </div>
	            </div>
	        </div>
        % endif
    % endfor
	<div class="clearfix"></div><!--  flush plots, break to next row -->
% endfor
