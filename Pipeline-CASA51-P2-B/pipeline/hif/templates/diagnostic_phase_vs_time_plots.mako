<%!
rsc_path = ""
import collections

SELECTORS = ['vis', 'spw', 'ant']

HISTOGRAM_LABELS = collections.OrderedDict([
	('PHASE_SCORE_XY', 'X-Y phase deviation'),
	('PHASE_SCORE_X2X1', 'X2-X1 phase deviation')
])
HISTOGRAM_AXES = collections.OrderedDict([
	('PHASE_SCORE_XY', 'PLOTS.xAxisLabels["qa"]'),
	('PHASE_SCORE_X2X1', 'PLOTS.xAxisLabels["qa"]')
])

def get_score_text(plot):
	score_map = {'PHASE_SCORE_XY'   : 'X-Y',
				 'PHASE_SCORE_X2X1' : 'X2-X1'}
	return ', '.join(sorted(['%s=%s' % (score_map[k],v) for k,v in plot.scores.items()]))
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
