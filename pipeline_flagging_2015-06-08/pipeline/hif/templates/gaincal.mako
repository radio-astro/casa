<%!
rsc_path = ""
import os
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="header" />

<%block name="title">Gain Calibration</%block>

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
                height: 50
            }
        },
    	beforeShow : function() {
        	this.title = $(this.element).attr('title');
       	}    
    }); 
 });
</script>

<p>This task creates gain solutions for each measurement set.</p>

<ul>
    <li>Plots</li>
    <ul>
    % if phase_vs_time_plots:
        <li><a href="#phase_vs_time_plots">Phase vs time</a></li>
    % endif
    % if structure_plots:
        <li><a href="#structure_plots">Phase structure</a></li>
    % endif
    % if amp_vs_time_plots:
        <li><a href="#amp_vs_time_plots">Amplitude vs time</a></li>
    % endif
    </ul>
    <li>Diagnostic plots</li>
    <ul>
    % if diagnostic_phase_vs_time_plots:
        <li><a href="#diagnostic_phase_vs_time_plots">Phase vs time</a></li>
    % endif
    % if diagnostic_amp_vs_time_plots:
        <li><a href="#diagnostic_amp_vs_time_plots">Amplitude vs time</a></li>
    % endif
    </ul>
</ul>

<h2>Results</h2>
<table class="table table-bordered" summary="Application Results">
	<caption>Applied calibrations and parameters used for caltable generation</caption>
    <thead>
        <tr>
            <th scope="col" rowspan="2">Measurement Set</th>
			<th scope="col" colspan="2">Solution Parameters</th>
			<th scope="col" colspan="2">Applied To</th>
            <th scope="col" rowspan="2">Calibration Table</th>
		</tr>
		<tr>
			<th>Type</th>
            <th>Interval</th>
			<th>Scan Intent</th>
			<th>Spectral Windows</th>
        </tr>
    </thead>
	<tbody>
% for application in applications:
		<tr>
			<td>${application.ms}</td>
		  	<td>${application.solint}</td>
		  	<td>${application.calmode}</td>
		  	<td>${application.intent}</td>
		  	<td>${application.spw}</td>
		  	<td>${application.gaintable}</td>
		</tr>
% endfor		
	</tbody>
</table>

% if structure_plots or phase_vs_time_plots or amp_vs_time_plots:
<h2>Plots</h2>

<%self:plot_group plot_dict="${phase_vs_time_plots}"
				  url_fn="${lambda x: phase_vs_time_subpages[x]}"
				  data_spw="${True}"
                  data_vis="${True}"
                  title_id="phase_vs_time_plots">

	<%def name="title()">
		Phase vs time
	</%def>

	<%def name="preamble()">
		<p>Plots show the phase correction to be applied to the target source. 
		A plot is shown for each spectral window, with phase correction data points
		plotted per antenna and correlation as a function of time.</p>

		<p>Click the summary plots to enlarge them, or the spectral window heading to
		see detailed plots per spectral window and antenna.</p> 
	</%def>

	<%def name="mouseover(plot)">Click to show phase vs time for spectral window ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">Spectral window ${plot.parameters['spw']}</%def>

	<%def name="caption_title(plot)">
		Spectral window ${plot.parameters['spw']}
	</%def>

	<%def name="caption_text(plot, intent)"> 
		Phase vs time for spectral window ${plot.parameters['spw']}, 
		all antennas and correlations.
	</%def>

</%self:plot_group>


<%self:plot_group plot_dict="${structure_plots}"
				  url_fn="${lambda x: 'baseline-%s.html' % filenamer.sanitize(x)}"
                  title_id="structure_plots">

	<%def name="title()">
		Phase structure: phase RMS vs distance to reference antenna
	</%def>

	<%def name="preamble()">
		<p>Plots are generated per spectral window, with phase RMS data points per antenna and
		correlation as a function of distance from the reference antenna. The phase RMS is calculated
		as the RMS of the phase correction measured over all scans with phase observing intent.</p>
	
		<p>Click the summary plots to enlarge them.</p> 
	</%def>

	<%def name="mouseover(plot)">Click to show phase RMS vs distance plots for spectral window ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">Spectral window ${plot.parameters['spw']}</%def>

	<%def name="caption_title(plot)">
		Spectral window ${plot.parameters['spw']}
	</%def>

	<%def name="caption_text(plot, intent)"> 
		RMS phase vs distance to reference antenna for spectral
		window ${plot.parameters['spw']}, all antennas.
	</%def>

</%self:plot_group>


<%self:plot_group plot_dict="${amp_vs_time_plots}"
				  url_fn="${lambda x: amp_vs_time_subpages[x]}"
				  data_spw="${True}"
                  data_vis="${True}"
                  title_id="amp_vs_time_plots">

	<%def name="title()">
		Amplitude vs time
	</%def>

	<%def name="preamble()">
		<p>Plots show the amplitude calibration to be applied to the target source. 
		A plot is shown for each spectral window, with amplitude correction data 
		points per antenna and correlation as a function of time.</p>
	
		<p>Click the summary plots to enlarge them, or the spectral window heading to
		see detailed plots per spectral window and antenna.</p> 
	</%def>

	<%def name="mouseover(plot)">Click to show amplitude vs time for spectral window ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">Spectral window ${plot.parameters['spw']}</%def>

	<%def name="caption_title(plot)">
		Spectral window ${plot.parameters['spw']}
	</%def>

	<%def name="caption_text(plot, intent)"> 
		Amplitude vs time for spectral window ${plot.parameters['spw']}, 
		all antennas and correlations.
	</%def>

</%self:plot_group>

%endif

% if diagnostic_phase_vs_time_plots or diagnostic_amp_vs_time_plots:
<h2>Diagnostic plots</h2>

<%self:plot_group plot_dict="${diagnostic_phase_vs_time_plots}"
				  url_fn="${lambda x: diagnostic_phase_vs_time_subpages[x]}"
				  data_spw="${True}"
                  data_vis="${True}"
                  title_id="diagnostic_phase_vs_time_plots">

	<%def name="title()">
		Phase vs time for solint='int'
	</%def>

	<%def name="preamble()">
		<p>Plots show the phase correction calculated using solint='int'. This
		correction is not applied to the target. 
		A plot is shown for each spectral window, with phase correction data points
		plotted per antenna and correlation as a function of time.</p>

		<p>Click the summary plots to enlarge them, or the spectral window heading to
		see detailed plots per spectral window and antenna.</p> 
	</%def>

	<%def name="mouseover(plot)">Click to show phase vs time for spectral window ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">Spectral window ${plot.parameters['spw']}</%def>

	<%def name="caption_title(plot)">
		Spectral window ${plot.parameters['spw']}
	</%def>

	<%def name="caption_text(plot, intent)"> 
		Phase vs time for spectral window ${plot.parameters['spw']}, 
		all antennas and correlations.
	</%def>

</%self:plot_group>


<%self:plot_group plot_dict="${diagnostic_amp_vs_time_plots}"
				  url_fn="${lambda x: diagnostic_amp_vs_time_subpages[x]}"
				  data_spw="${True}"
                  data_vis="${True}"
                  title_id="diagnostic_amp_vs_time_plots">

	<%def name="title()">
		Amplitude vs time for solint='int'
	</%def>

	<%def name="preamble()">
		<p>Plots show the amplitude correction calculated using solint='int'. This
		correction is not applied to the target. 
		A plot is shown for each spectral window, with phase correction data points
		plotted per antenna and correlation as a function of time.</p>

		<p>Click the summary plots to enlarge them, or the spectral window heading to
		see detailed plots per spectral window and antenna.</p> 
	</%def>

	<%def name="mouseover(plot)">Click to show amplitude vs time for spectral window ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">Spectral window ${plot.parameters['spw']}</%def>

	<%def name="caption_title(plot)">
		Spectral window ${plot.parameters['spw']}
	</%def>

	<%def name="caption_text(plot, intent)"> 
		Amplitude vs time for spectral window ${plot.parameters['spw']}, 
		all antennas and correlations.
	</%def>

</%self:plot_group>

% endif