<%!
rsc_path = ""
import cgi
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
    'apriorical': 'Apriori calibrations applied',
    'before': 'Calibrated data before flagging',
    'after': 'Calibrated data after flagging'
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

<%block name="title">Phased-up Fluxscale Calibration and Flagging</%block>

<p>
    This task computes the flagging heuristics on the phase calibrator and flux
    calibrator by calling hif_correctedampflag which looks for outlier
    visibility points by statistically examining the scalar difference of
    corrected amplitudes minus model amplitudes, and flags those outliers. The
    philosophy is that only outlier data points that have remained outliers
    after calibration will be flagged. The heuristic works equally well on
    resolved calibrators and point sources because it is not performing a
    vector difference, and thus is not sensitive to nulls in the flux density
    vs. uvdistance domain. Note that the phase of the data is not assessed.
</p>
<p>
    In further detail, the workflow is as follows: an a priori calibration is
    applied using pre-existing caltables in the calibration state, a
    preliminary phase and amplitude gaincal solution is solved and applied, the
    flagging heuristics are run, and any outliers are flagged. Plots are
    generated at three points in this workflow: after a priori calibration,
    after preliminary phase and amplitude calibration but before flagging
    heuristics are run, and after flagging heuristics have been run and
    applied. If no points were flagged, the "after" plots are not generated or
    displayed. The score for this stage is the standard data flagging score,
    which depends on the fraction of data flagged.
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
		% for k in ['TOTAL', 'BANDPASS', 'AMPLITUDE', 'PHASE', 'TARGET']:
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

<%self:plot_group plot_dict="${time_plots}"
				  url_fn="${lambda x: 'junk'}"
                  rel_fn="${lambda plot: 'amp_vs_time_%s_%s_%s_%s' % (plot.parameters['vis'], plot.parameters['intent'], plot.parameters['field'], plot.parameters['spw'])}"
				  title_id="amp_vs_time"
                  break_rows_by="intent,field,type_idx"
                  sort_row_by="spw">

	<%def name="title()">
		Amplitude vs time
	</%def>

	<%def name="preamble()">
		<p>These plots show amplitude vs time for three cases: 1, the data with apriori calibrations applied;
            2, the calibrated data before application of any flags; and 3, where flagging was applied,
            the calibrated data after application of flags.</p>

		<p>Data are plotted for all antennas and correlations, with different
		correlations shown in different colours.</p>
	</%def>

	<%def name="mouseover(plot)">Click to show amplitude vs time for spw ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
		${plot_type(plot)}<br>
		${plot.parameters['vis']}<br>
		Spw ${plot.parameters['spw']}<br>
		Intents: ${utils.commafy([plot.parameters['intent']], False)}<br>
        Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

    <%def name="caption_title(plot)">
		Spectral Window ${plot.parameters['spw']}<br>
	</%def>

	<%def name="caption_subtitle(plot)">
		Intents: ${utils.commafy([plot.parameters['intent']], False)}<br>
        Fields: ${utils.commafy(utils.safe_split(plot.parameters['field']), quotes=False)}
	</%def>

    <%def name="caption_text(plot, ptype)">
		${plot_type(plot)}.
	</%def>

</%self:plot_group>


<%self:plot_group plot_dict="${uvdist_plots}"
				  url_fn="${lambda x: 'junk'}"
                  rel_fn="${lambda plot: 'amp_vs_uvdist_%s_%s_%s_%s' % (plot.parameters['vis'], plot.parameters['intent'], plot.parameters['field'], plot.parameters['spw'])}"
				  title_id="amp_vs_uvdist"
                  break_rows_by="intent,field,type_idx"
                  sort_row_by="spw">

	<%def name="title()">
		Amplitude vs UV distance
	</%def>

	<%def name="preamble()">
		<p>These plots show amplitude vs UV distance for three cases: 1, the data with apriori calibrations applied;
            2, the calibrated data before application of any flags; and 3, where flagging was applied,
            the calibrated data after application of flags.</p>

		<p>Data are plotted for all antennas and correlations, with different
		correlations shown in different colours.</p>
	</%def>

	<%def name="mouseover(plot)">Click to show amplitude vs UV distance for ${plot.parameters['field']} (${plot.parameters['intent']}) spw ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
		${plot_type(plot)}<br>
		${plot.parameters['vis']}<br>
		Spw ${plot.parameters['spw']}<br>
		Intents: ${utils.commafy([plot.parameters['intent']], False)}<br>
        Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

    <%def name="caption_title(plot)">
		Spectral Window ${plot.parameters['spw']}<br>
	</%def>

	<%def name="caption_subtitle(plot)">
		Intents: ${utils.commafy([plot.parameters['intent']], False)}<br>
        Field: ${utils.commafy(utils.safe_split(plot.parameters['field']), quotes=False)}
	</%def>

    <%def name="caption_text(plot, ptype)">
		${plot_type(plot)}.
	</%def>

</%self:plot_group>
