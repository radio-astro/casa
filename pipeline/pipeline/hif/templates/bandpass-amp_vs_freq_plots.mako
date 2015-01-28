<%!
rsc_path = ""
import collections
SELECTORS = ['spw', 'ant']
HISTOGRAM_LABELS = collections.OrderedDict([
	('AMPLITUDE_SCORE_SNR', 'SNR (Error Function)')
])
HISTOGRAM_AXES = collections.OrderedDict([
	('AMPLITUDE_SCORE_SNR', 'PLOTS.xAxisLabels["qa"]')
])
def get_score_text(plot):
	return ', '.join(sorted(['%s=%s' % (k,v) for k,v in plot.scores['AMPLITUDE_SCORE_SNR'].items()]))
%>
<%inherit file="detail_plots_basetemplate.mako"/>

<%self:render_plots plots="${sorted(plots, key=lambda p: p.parameters['ant'])}">
	<%def name="mouseover(plot)">${plot.parameters['ant']} spw ${plot.parameters['spw']}; ${get_score_text(plot)}</%def>

	<%def name="fancybox_caption(plot)">
		Antenna: ${plot.parameters['ant']}<br>
		Spectral Window: ${plot.parameters['spw']}<br>
		Scores: ${get_score_text(plot)}
	</%def>

	<%def name="caption_text(plot)">
		${plot.parameters['ant']}<br>
		Spw ${plot.parameters['spw']}<br>
	</%def>
</%self:render_plots>

<%doc>
<div class="page-header">
	<h1>Bandpass amplitude for ${vis}<button class="btn btn-default pull-right" onClick="javascript:location.reload();">Back</button></h1>
</div>
</%doc>