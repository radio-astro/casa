<%!
rsc_path = ""
import os
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.renderer.htmlrenderer as hr

def get_mapping(ms, spwid):
    if ms.combine_spwmap:
        return 'of <strong>combined</strong> solution '
    else:
        return ' for spectral window <strong>' + str(spwid) + '</strong>'

def get_mapped_window(ms, spwid):
    if ms.combine_spwmap:
        return 'the <strong>combined</strong> solution '
    elif ms.phaseup_spwmap:
        return 'spectral window <strong>' + str(ms.phaseup_spwmap[spwid]) + '</strong>'
    else:
        return spwid

def get_mapped_scispws(ms):
    if ms.combine_spwmap:
        spws = [str(spw.id) for spw in ms.get_spectral_windows()]
        return 'the following spectral windows ' + ','.join(spws) + ' have been combined' 
    elif ms.phaseup_spwmap:
        spws = [str(spw.id) for spw in ms.get_spectral_windows() if spw.id != ms.phaseup_spwmap[spw.id]]
        return 'the following spectral windows '  + ','.join(spws) + ' have been remapped' 
    else:
        return 'no spectral windows have been combined or remapped'

%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="header" />

<%block name="title">Gain Calibration</%block>

<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>

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
    % if diagnostic_phaseoffset_vs_time_plots:
        <li><a href="#diagnostic_phaseoffset_vs_time_plots">Phase offsets vs time</a></li>
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
		  	<td>${application.calmode}</td>
		  	<td>${application.solint}</td>
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
                  sort_row_by="spw"
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
		Phase vs time ${get_mapping(pcontext.observing_run.get_ms(name=plot.parameters['vis']), plot.parameters['spw'])},
                all antennas and correlations.</%def>

</%self:plot_group>


<%self:plot_group plot_dict="${structure_plots}"
				  url_fn="${lambda x: 'baseline-%s.html' % filenamer.sanitize(x)}"
                  sort_row_by="spw"
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

    <%def name="ms_preamble(ms)">
        <p><strong>Note that ${get_mapped_scispws(pcontext.observing_run.get_ms(name=ms))}</strong></p>
    </%def>

	<%def name="mouseover(plot)">Click to show phase RMS vs distance plots for spectral window ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">Spectral window ${plot.parameters['spw']}</%def>

	<%def name="caption_title(plot)">
		Spectral window ${plot.parameters['spw']}
	</%def>

    <%def name="caption_text(plot, intent)">
        RMS phase vs distance to reference antenna ${get_mapping(pcontext.observing_run.get_ms(name=plot.parameters['vis']), plot.parameters['spw'])}, all antennas.
    </%def>

</%self:plot_group>


<%self:plot_group plot_dict="${amp_vs_time_plots}"
				  url_fn="${lambda x: amp_vs_time_subpages[x]}"
				  data_spw="${True}"
                  data_vis="${True}"
                  sort_row_by="spw"
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

% if diagnostic_phase_vs_time_plots or diagnostic_phaseoffset_vs_time_plots or diagnostic_amp_vs_time_plots:
<h2>Diagnostic plots</h2>

<%self:plot_group plot_dict="${diagnostic_phase_vs_time_plots}"
				  url_fn="${lambda x: diagnostic_phase_vs_time_subpages[x]}"
				  data_spw="${True}"
                  data_vis="${True}"
                  sort_row_by="spw"
                  title_id="diagnostic_phase_vs_time_plots">

	<%def name="title()">
		Phase vs time
	</%def>

	<%def name="preamble()">
		<p>These diagnostic plots show the phase solution for a calibration
            generated using a short solution interval. This calibration is not applied
            to the target. One plot is shown for each non-combined spectral
            window, with phase correction plotted per antenna and
            correlation as a function of time.</p>

		<p>Click the summary plots to enlarge them, or the spectral window
            heading to see detailed plots per spectral window and antenna.</p>
	</%def>

    <%def name="ms_preamble(ms)">
        <p>Plots show the diagnostic phase calibration for ${ms} calculated
            using solint='${diagnostic_solints[ms]['phase']}'.</p>
    </%def>

	<%def name="mouseover(plot)">Click to show phase vs time for spectral window ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">Spectral window ${plot.parameters['spw']}</%def>

	<%def name="caption_title(plot)">
		Spectral window ${plot.parameters['spw']}
	</%def>

	<%def name="caption_text(plot, intent)"> 
		Phase vs time ${get_mapping(pcontext.observing_run.get_ms(name=plot.parameters['vis']), plot.parameters['spw'])},
                all antennas and correlations.
	</%def>

</%self:plot_group>

<%self:plot_group plot_dict="${diagnostic_phaseoffset_vs_time_plots}"
				  url_fn="${lambda x: diagnostic_phaseoffset_vs_time_subpages[x]}"
				  data_spw="${True}"
                  data_vis="${True}"
                  sort_row_by="spw"
                  title_id="diagnostic_phaseoffset_vs_time_plots">

	<%def name="title()">
		Phase offsets vs time
	</%def>

	<%def name="preamble()">
		<p>These diagnostic plots show the phase offsets as a function of time. The
            phase offsets are computed by preapplying the previous phase only solutions to the
            data and computing a new phase solution. The new phase solutions should scatter
            around zero. The new solutions are not applied to the target. One plot is shown for
            each spectral window, with phase offset plotted per antenna and correlation as a
            function of time.</p>
                
		<p>Click the summary plots to enlarge them, or the spectral window
            heading to see detailed plots per spectral window and antenna.</p>
	</%def>

        <%def name="ms_preamble(ms)">
                <p>Plots show the diagnostic phase offsets for ${ms} calculated
            using solint='inf'.</p>

            <p><strong>Note that ${get_mapped_scispws(pcontext.observing_run.get_ms(name=ms))}</strong></p>
        </%def>

	<%def name="mouseover(plot)">Click to show phase offset vs time for spectral window ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">Spectral window ${plot.parameters['spw']}</%def>

	<%def name="caption_title(plot)">
		Spectral window ${plot.parameters['spw']}
	</%def>

	<%def name="caption_text(plot, intent)"> 
		Phase offset vs time for spectral window <strong>${plot.parameters['spw']}</strong>, which has been 
		mapped to ${get_mapped_window(pcontext.observing_run.get_ms(name=plot.parameters['vis']), plot.parameters['spw'])},
                all antennas and correlations.
	</%def>

</%self:plot_group>


<%self:plot_group plot_dict="${diagnostic_amp_vs_time_plots}"
				  url_fn="${lambda x: diagnostic_amp_vs_time_subpages[x]}"
				  data_spw="${True}"
                  data_vis="${True}"
                  sort_row_by="spw"
                  title_id="diagnostic_amp_vs_time_plots">

	<%def name="title()">
		Amplitude vs time
	</%def>

	<%def name="preamble()">
		<p>These diagnostic plots show the amplitude solution for a calibration
            generated using a short solution interval. This calibration is not applied
            to the target. One plot is shown for each non-combined spectral
            window, with amplitude correction plotted per antenna and
            correlation as a function of time.</p>

		<p>Click the summary plots to enlarge them, or the spectral window
            heading to see detailed plots per spectral window and antenna.</p>
	</%def>

    <%def name="ms_preamble(ms)">
        <p>Plots show the diagnostic amplitude calibration for ${ms} calculated
            using solint='${diagnostic_solints[ms]['amp']}'.</p>
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
