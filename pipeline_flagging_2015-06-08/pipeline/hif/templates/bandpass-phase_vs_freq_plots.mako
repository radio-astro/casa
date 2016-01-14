<%!
rsc_path = ""
import collections
SELECTORS = ['vis', 'spw', 'ant']
HISTOGRAM_LABELS = collections.OrderedDict([
	('PHASE_SCORE_DD', 'Derivative')
])
HISTOGRAM_AXES = collections.OrderedDict([
	('PHASE_SCORE_DD', 'PLOTS.xAxisLabels["qa"]')
])

def get_score_text(plot):
	return ', '.join(sorted(['%s=%s' % (k,v) for k,v in plot.scores['PHASE_SCORE_DD'].items()]))
%>
<%inherit file="detail_plots_basetemplate.mako"/>

<%
    multi_vis = len({p.parameters['vis'] for p in plots}) > 1
%>

<%self:render_plots plots="${sorted(plots, key=lambda p: p.parameters['ant'])}">

	<%def name="mouseover(plot)">${plot.parameters['ant']} spw ${plot.parameters['spw']}; ${get_score_text(plot)}</%def>

	<%def name="fancybox_caption(plot)">
        % if multi_vis:
        ${plot.parameters['vis']}<br>
        % endif
		Antenna: ${plot.parameters['ant']}<br>
		Spectral Window: ${plot.parameters['spw']}<br>
		Scores: ${get_score_text(plot)}
	</%def>

	<%def name="caption_text(plot)">
        % if multi_vis:
        ${plot.parameters['vis']}<br>
        % endif
		${plot.parameters['ant']}<br>
		Spw ${plot.parameters['spw']}<br>
	</%def>
</%self:render_plots>

<%doc>
<div class="page-header">
	<h1>Bandpass phase for ${vis}<button class="btn btn-default pull-right" onClick="javascript:window.history.back();">Back</button></h1>
</div>
</%doc>
