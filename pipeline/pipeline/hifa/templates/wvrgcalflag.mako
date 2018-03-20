<%!
rsc_path = ""
import os
import pipeline.infrastructure.filenamer as filenamer
%>
<%inherit file="t2-4m_details-base.mako"/>
<%block name="header" />

<%block name="title">WVR Calibration and Flagging</%block>

<p>This task checks whether the WVR radiometers are working as intended,
interpolating for antennas that are not. The WVR caltable is only added to 
subsequent pre-applys if it gives a tangible improvement.</p>

<h2>Results</h2>

%if not applications:
<p>This measurement set either does not contain any antennas with WVR data,
or in the case of mixed-array does not contain enough antennas with WVR data
(as set by the threshold parameter in inputs). WVR calibration and flagging has
been skipped.</p>
%endif

% if flag_plots or phase_offset_summary_plots or baseline_summary_plots:
<h2>Plots</h2>

<p>The pipeline tests whether application of WVR correction improves the data
by performing a gaincal for a chosen field, usually the bandpass calibrator,
and comparing the resulting phase corrections evaluated both with and without
application of WVR correction. Plots based on these data in these evaluation
caltables are presented below.</p>

<%self:plot_group plot_dict="${flag_plots}"
				  url_fn="${lambda x: flag_subpages[x]}">
                  data_vis="${True}"
                  data_spw="${True}">

	<%def name="title()">
		Flagging metric view(s)
	</%def>

	<%def name="preamble()">
		<p>The following plots show the flagging metric used by the pipeline
		to determine which antennas' WVR corrections to flag. The RMS phase
		during observation of the bandpass calibrator is calculated without
		WVR corrections applied, and with WVR corrections applied, and the
		metric is the ratio of those two RMS values. If the WVR measurements
		are corrupted, or the wvrgcal task itself flags the WVR data on a
		given antenna, then the pipeline will not calculate a metric here.</p>
	</%def>

	<%def name="mouseover(plot)">Click to enlarge flag plot for spw ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
		Spectral window: ${plot.parameters['spw']}
	</%def>

	<%def name="caption_title(plot)">
		Spectral window ${plot.parameters['spw']}
	</%def>
</%self:plot_group>

<%self:plot_group plot_dict="${phase_offset_summary_plots}"
				  url_fn="${lambda x: phase_offset_subpages[x]}"
				  data_spw="${True}"
                  data_vis="${True}">

	<%def name="title()">
		Phase correction with/without WVR
	</%def>

	<%def name="preamble()">
        % if metric_plots:

            <p>The following set of plots show the improvement in the rms phase
            after applying the WVR corrections. These plots are calculated for
            various data intents after both the pipeline and wvrgcal task have
            selected antennas whose WVR correction needs flagging. The
            correction applied to those antennas in these plots is the
            correction interpolated from neighboring antennas. Sometimes
            antennas can have sufficiently corrupted data that the metric can
            still not be calculated, but those antennas will in most cases be
            flagged for the rest of the calibration process. Click on a link
            below to show all flagging metric views for that measurement set.
            </p>

            <ul>
            % for ms, subpage in metric_subpages.items():
                <li>
                    <a href="${os.path.relpath(os.path.join(dirname, subpage), pcontext.report_dir)}"
                       data-vis="${ms}"
                       class="replace">
                        ${ms}
                    </a>
                </li>
            % endfor
            </ul>
            % endif

		<p>The next set of plots show the deviation about the scan median phase
        before and after WVR application. Points are plotted per integration
        and per correlation.</p>

		<p>Click the summary plots to enlarge, or click the summary title for a
        gallery of more detailed plots for individual antennas.</p>
	</%def>

	<%def name="mouseover(plot)">Click to show WVR phase offset plots for spw ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
		Spectral window: ${plot.parameters['spw']}
	</%def>

	<%def name="caption_title(plot)">
		Spectral window ${plot.parameters['spw']}
	</%def>
</%self:plot_group>

<%self:plot_group plot_dict="${baseline_summary_plots}"
				  url_fn="${lambda x: baseline_summary_subpages[x]}"
				  data_spw="${True}"
                  data_vis="${True}">

	<%def name="title()">
		Phase correction vs distance to reference antenna
	</%def>

	<%def name="preamble()">
		<p>Plots show the phase offset (lower panel) and improvement ratio
		(upper panel) vs distance to the reference antenna before and after
        WVR application.</p>

		<p>The lower panel of these plots show the median absolute deviation of
        the gaincal corrections with and without WVR correction applied. The
        upper panel shows the ratio of the RMS deviations about the median for
        data with WVR correction applied to the RMS deviations without WVR
        correction. One plot is generated per scan, with points plotted per
		correlation and antenna as a function of distance from the reference
        antenna.</p>

		<p>Click the summary plots to enlarge them, or the summary plot title
        to show a gallery of phase offset plots for individual antenna.</p>
	</%def>

	<%def name="mouseover(plot)">Click to show WVR phase offset vs baseline plots for spw ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
		Spectral window: ${plot.parameters['spw']}
	</%def>

	<%def name="caption_title(plot)">
		Spectral window ${plot.parameters['spw']}
	</%def>
</%self:plot_group>

%endif

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
		  	<td><span class="glyphicon glyphicon-${'ok' if application.applied else 'remove'}"></span></td>
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
            <th>Interpolated Correction</th>
            <th>RMS</th>
            <th>Disc</th>
		</tr>
    </thead>
	<tbody>
% for ms in wvrinfos:
	% for wvrinfo in wvrinfos[ms]:
		<tr class="${'danger alert-danger' if wvrinfo.flag else ''}">
			<td>${ms}</td>
		  	<td>${wvrinfo.antenna}</td>
		  	<td><span class="glyphicon glyphicon-${'ok' if wvrinfo.wvr else 'remove'}"></span></td>
		  	<td><span class="glyphicon glyphicon-${'ok' if wvrinfo.flag else 'remove'}"></span></td>
		  	<td>${wvrinfo.rms}</td>
		  	<td>${wvrinfo.disc}</td>
		</tr>
	%endfor
% endfor		
	</tbody>
</table>
% endif
