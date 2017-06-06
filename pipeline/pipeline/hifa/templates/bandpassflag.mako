<%!
rsc_path = ""
import os
import pipeline.infrastructure.utils as utils

# method to output flagging percentages neatly
def percent_flagged(flagsummary):
    flagged = flagsummary.flagged
    total = flagsummary.total

    if total is 0:
        return 'N/A'
    else:
        return '%0.1f%%' % (100.0 * flagged / total)

_types = {
    'raw': 'Raw data',
    'before': 'Corrected data before flagging',
    'after': 'Corrected data after flagging'
}

def plot_type(plot):
    return _types[plot.parameters['type']]

%>

<%
# these functions are defined in template scope.
def num_lines(relpath):
	abspath = os.path.join(pcontext.report_dir, relpath)
	if os.path.exists(abspath):
		return sum(1 for line in open(abspath) if not line.startswith('#'))
	else:
		return 'N/A'
%>

<%inherit file="t2-4m_details-base.mako"/>
<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>
<script>
$(document).ready(function(){
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
        },
    	beforeShow : function() {
        	this.title = $(this.element).attr('title');
       	},
    });
});
</script>

<%block name="header" />

<%block name="title">Bandpass Calibration and Flagging</%block>

<p>
    This task creates bandpass solutions for each measurement set, and
    flags outliers based on a comparison of the calibrated (corrected)
    amplitudes with the model amplitudes for the bandpass calibrator source.
</p>

% if htmlreports:
    <h2>Flagging</h2>
    <table class="table table-bordered table-striped">
        <caption>Report Files</caption>
        <thead>
            <tr>
                <th>Measurement Set</th>
                <th>Flagging Commands</th>
                <th>Number of Statements</th>
            </tr>
        </thead>
        <tbody>
        % for msname, relpath in htmlreports.items():
            <tr>
                <td>${msname}</td>
                <td><a class="replace-pre" href="${relpath}">${os.path.basename(relpath)}</a></td>
                <td>${num_lines(relpath)}</td>
            </tr>
        % endfor
        </tbody>
    </table>
% endif


<h2>Flagged data summary</h2>

% for ms in flags.keys():
<h4>Measurement Set: ${os.path.basename(ms)}</h4>
<table class="table table-bordered table-striped ">
	<caption>Summary of flagged data. Each cell states the amount of data
		flagged as a fraction of the specified data selection.
	</caption>
	<thead>
		<tr>
			<th rowspan="2">Data Selection</th>
			<!-- flags before task is always first agent -->
			<th rowspan="2">flagged before</th>
			<th rowspan="2">flagged after</th>
		</tr>
	</thead>
	<tbody>
		% for k in ['TOTAL', 'BANDPASS', 'AMPLITUDE', 'PHASE', 'TARGET','ATMOSPHERE']:
		<tr>
			<th>${k}</th>
			% for step in ['before','after']:
			% if flags[ms].get(step) is not None:
				<td>${percent_flagged(flags[ms][step]['Summary'][k])}</td>
			% else:
				<td>0.0%</td>
			% endif
			% endfor
		</tr>
		% endfor
	</tbody>
</table>

% endfor


<h2>Bandpass results</h2>

% if phaseup_applications:
    <h4>Phase-up on bandpass calibrator</h4>
    <table class="table table-bordered" summary="Application Results">
        <caption>Applied calibrations and parameters used for phase-up calibration</caption>
        <thead>
            <tr>
                <th scope="col" rowspan="2">Measurement Set</th>
                <th scope="col" colspan="5">Phase-up Solution Parameters</th>
            </tr>
            <tr>
                <th>Type</th>
                <th>Interval</th>
                <th>Min Baselines per Antenna</th>
                <th>Min SNR</th>
                <th>Phase-up Bandwidth</th>
            </tr>
        </thead>
        <tbody>
    % for application in phaseup_applications:
            <tr>
                <td>${application.ms}</td>
                <td>${application.calmode}</td>
                <td>${application.solint}</td>
                <td>${application.minblperant}</td>
                <td>${application.minsnr}</td>
                <td>${application.phaseupbw}</td>
            </tr>
    % endfor
        </tbody>
    </table>
% endif

<h4>Bandpass calibration</h4>

<table class="table table-bordered" summary="Application Results">
	<caption>Parameters used for bandpass calibration</caption>
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
	% for tr in bandpass_table_rows:
		<tr>
		% for td in tr:
			${td}
		% endfor
		</tr>
	%endfor
	</tbody>
</table>

% if amp_refant or amp_mode:
    <h2>Plots</h2>

    <p>Plots show the bandpass correction applied to the target source.
    The first two plots show amplitude vs frequency; one for the reference antenna
    and one for a typical antenna, identified the antenna with mode score.
    The third plot shows phase vs frequency for the typical antenna.
    </p>

    <p>Click the summary plots to enlarge them, or the plot title to see
    see detailed plots per spectral window and antenna.</p>

    % for ms in amp_refant:

        <h4>${ms}</h4>
        % if amp_refant[ms] or amp_mode[ms]:
        <div class="col-md-8">
            <div class="thumbnail">
                <div class="caption">
                    <%
                        plot = amp_refant[ms]
                    %>
                    <h4>Amplitude vs frequency
                        (<a class="replace"
                            data-vis="${ms}"
                            href="${os.path.relpath(os.path.join(dirname, amp_subpages[ms]), pcontext.report_dir)}">show ${ms}</a>)
                    </h4>
                    <p>The plots below show amplitude vs frequency for the
                    bandpass correction, overlayed for all spectral windows
                    and correlations. Click on the link above to show
                    show detailed plots for all antennas, or on the links
                    below to show plots with specific antennas preselected.</p>
                </div>
                <div class="row">
                    <div class="col-md-6">
                        % if plot is None or not os.path.exists(plot.thumbnail):
                        <a href="${os.path.relpath(os.path.join(dirname, amp_subpages[ms]), pcontext.report_dir)}"
                           class="fancybox"
                           rel="plots-${ms}">
                            <img data-src="holder.js/255x188/text:Not Available">
                        </a>
                        <div class="caption">
                            <h5>Reference antenna
                                (<a href="${os.path.relpath(os.path.join(dirname, amp_subpages[ms]), pcontext.report_dir)}"
                                    class="replace"
                                    data-vis="${ms}">
                                    show all detail plots</a>)
                            </h5>
                            <p>The amplitude vs frequency plot for the reference antenna
                            is not available.</p>
                        </div>
                        % else:
                        <a href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
                           class="fancybox"
                           rel="plots-${ms}">
                            <img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
                                 title="Click to show amplitude vs time plot"
                                 data-thumbnail="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}">
                        </a>
                        <div class="caption">
                            <h5>
                                Reference antenna (${plot.parameters['ant']})
                                (<a href="${os.path.relpath(os.path.join(dirname, amp_subpages[ms]), pcontext.report_dir)}"
                                   class="replace"
                                   data-ant="${plot.parameters['ant']}"
                                   data-vis="${plot.parameters['vis']}">
                                   show ${plot.parameters['ant']}</a>)
                            </h5>
                            <p>Amplitude vs frequency for the reference antenna
                            (${plot.parameters['ant']}). Click the link above to show
                            detailed plots for ${plot.parameters['ant']}.</p>
                        </div>
                        % endif
                    </div>

                    <%
                        plot = amp_mode[ms]
                    %>
                    <div class="col-md-6">
                        % if plot is None or not os.path.exists(plot.thumbnail):
                        <a href="${os.path.relpath(os.path.join(dirname, amp_subpages[ms]), pcontext.report_dir)}"
                           class="fancybox"
                           rel="plots-${ms}">
                            <img data-src="holder.js/255x188/text:Not Available">
                        </a>
                        <div class="caption">
                            <h5>Typical antenna
                                (<a href="${os.path.relpath(os.path.join(dirname, amp_subpages[ms]), pcontext.report_dir)}"
                                    class="replace"
                                    data-vis="${ms}">
                                    show all detail plots</a>)
                            </h5>
                            <p>The amplitude vs frequency plot for a typical antenna
                            is not available.</p>
                        </div>
                        % else:
                        <a href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
                           class="fancybox"
                           rel="plots-${ms}">
                            <img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
                                 title="Click to show amplitude vs time plot"
                                 data-thumbnail="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}">
                        </a>
                        <div class="caption">
                            <h5>Typical antenna (${plot.parameters['ant']})
                                (<a href="${os.path.relpath(os.path.join(dirname, amp_subpages[ms]), pcontext.report_dir)}"
                                   class="replace"
                                   data-ant="${plot.parameters['ant']}"
                                   data-vis="${plot.parameters['vis']}">
                                   show ${plot.parameters['ant']}
                                </a>)
                            </h5>
                            <p>Amplitude vs frequency for a typical antenna
                            (${plot.parameters['ant']}). Click the link above to show
                            detailed plots for ${plot.parameters['ant']}.</p>
                            <p>NB. random antenna until scores are working</p>
                        </div>
                        % endif
                    </div>
                </div>
            </div>
        </div>
        % endif

        % if phase_mode[ms]:
        <div class="col-md-4">
            <div class="thumbnail">
                <div class="caption">
                    <%
                        plot = phase_mode[ms]
                    %>
                    <h4>Phase vs frequency
                        (<a class="replace"
                            data-vis="${ms}"
                            href="${os.path.relpath(os.path.join(dirname, phase_subpages[ms]), pcontext.report_dir)}">show ${ms}</a>)
                    </h4>
                    <p>The plot below shows phase vs frequency for the
                    bandpass correction, overlayed for all spectral windows
                    and correlations. Click on the link above to show
                    show phase vs frequency plots for all antennas, or on the
                    link for just the typical antenna.</p>
                </div>
                <div class="row">
                    <div class="col-md-12">
                        % if plot is None or not os.path.exists(plot.thumbnail):
                        <a href="${os.path.relpath(os.path.join(dirname, phase_subpages[ms]), pcontext.report_dir)}"
                           class="fancybox"
                           rel="plots-${ms}">
                            <img data-src="holder.js/255x188/text:Not Available">
                        </a>
                        <div class="caption">
                            <h5>Typical antenna
                                (<a href="${os.path.relpath(os.path.join(dirname, phase_subpages[ms]), pcontext.report_dir)}"
                                    class="replace"
                                    data-vis="${ms}">
                                    show all detail plots</a>)
                            </h5>
                            <p>The phase vs frequency plot is not available.</p>
                        </div>
                        % else:
                        <a href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
                              class="fancybox"
                              rel="plots-${ms}">
                            <img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
                                    title="Click to show phase vs frequency plot"
                                    data-thumbnail="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}">
                        </a>
                        <div class="caption">
                            <h5>Typical antenna (${plot.parameters['ant']})
                                (<a href="${os.path.relpath(os.path.join(dirname, phase_subpages[ms]), pcontext.report_dir)}"
                                   class="replace"
                                   data-ant="${plot.parameters['ant']}"
                                   data-vis="${plot.parameters['vis']}">
                                   show ${plot.parameters['ant']}
                                </a>)
                            </h5>
                            <p>Phase vs frequency for a typical antenna
                            (${plot.parameters['ant']}). Click the link above to show
                            detailed plots for ${plot.parameters['ant']}.</p>
                        </div>
                        % endif
                    </div>
                </div>
            </div>
        </div>
        % endif

    % endfor
% endif


<%self:plot_group plot_dict="${time_plots}"
				  url_fn="${lambda x: 'junk'}"
                  rel_fn="${lambda plot: 'amp_vs_time_%s_%s' % (plot.parameters['vis'], plot.parameters['spw'])}"
				  plot_accessor="${lambda ms_plots: ms_plots.items()}"
				  title_id="amp_vs_time">

	<%def name="title()">
		Amplitude vs time
	</%def>

	<%def name="preamble()">
		<p>These plots show amplitude vs time for three cases: 1, the raw, uncorrected data; 2, the corrected
            data column before application of any flags; and 3, where flagging was applied, the corrected data
            after application of flags.</p>

		<p>Data are plotted for all antennas and correlations, with different
		correlations shown in different colours.</p>
	</%def>

	<%def name="mouseover(plot)">Click to show amplitude vs time for spw ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
		${plot_type(plot)}<br>
		${plot.parameters['vis']}<br>
		Spw ${plot.parameters['spw']}<br>
		Intents: ${utils.commafy([plot.parameters['intent']], False)}
	</%def>

    <%def name="caption_title(plot)">
		Spectral Window ${plot.parameters['spw']}<br>
	</%def>

	<%def name="caption_subtitle(plot)">
		Intents: ${utils.commafy([plot.parameters['intent']], False)}
	</%def>

    <%def name="caption_text(plot, ptype)">
		${plot_type(plot)}.
	</%def>

</%self:plot_group>


<%self:plot_group plot_dict="${uvdist_plots}"
				  url_fn="${lambda x: 'junk'}"
                  rel_fn="${lambda plot: 'amp_vs_uvdist_%s_%s' % (plot.parameters['vis'], plot.parameters['spw'])}"
				  plot_accessor="${lambda ms_plots: ms_plots.items()}"
				  title_id="amp_vs_uvdist">

	<%def name="title()">
		Amplitude vs UV distance
	</%def>

	<%def name="preamble()">
		<p>These plots show amplitude vs UV distance for three cases: 1, the raw, uncorrected data; 2, the corrected
            data column before application of any flags; and 3, where flagging was applied, the corrected data
            after application of flags.</p>

		<p>Data are plotted for all antennas and correlations, with different
		correlations shown in different colours.</p>
	</%def>

	<%def name="mouseover(plot)">Click to show amplitude vs UV distance for spw ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
		${plot_type(plot)}<br>
		${plot.parameters['vis']}<br>
		Spw ${plot.parameters['spw']}<br>
		Intents: ${utils.commafy([plot.parameters['intent']], False)}
	</%def>

    <%def name="caption_title(plot)">
		Spectral Window ${plot.parameters['spw']}<br>
	</%def>

	<%def name="caption_subtitle(plot)">
		Intents: ${utils.commafy([plot.parameters['intent']], False)}
	</%def>

    <%def name="caption_text(plot, ptype)">
		${plot_type(plot)}.
	</%def>

</%self:plot_group>
