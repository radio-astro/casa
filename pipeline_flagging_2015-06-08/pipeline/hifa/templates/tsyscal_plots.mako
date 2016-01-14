<%!
rsc_path = ""
import collections
SELECTORS = ['vis', 'tsys_spw', 'spw', 'ant']
HISTOGRAM_LABELS = collections.OrderedDict([
	('median', 'Average of Median T<sub>sys</sub> over time'),
	('median_max', 'Maximum of Median T<sub>sys</sub> over time'),
	('rms', 'RMS deviation from Average Median T<sub>sys</sub>')
])

# set all X-axis labels to Kelvin
HISTOGRAM_AXES = collections.OrderedDict([
	('median', 'PLOTS.xAxisLabels["K"]'),
	('median_max', 'PLOTS.xAxisLabels["K"]'),
	('rms', 'PLOTS.xAxisLabels["K"]')
])
%>
<%inherit file="detail_plots_basetemplate.mako"/>

<%
    multi_vis = len({p.parameters['vis'] for p in plots}) > 1
%>

<%self:render_plots plots="${sorted(plots, key=lambda p: p.parameters['ant'])}">
	<%def name="mouseover(plot)">Click to magnify plot for ${plot.parameters['ant']} Tsys spw ${plot.parameters['tsys_spw']}</%def>

	<%def name="fancybox_caption(plot)">
        % if multi_vis:
        ${plot.parameters['vis']}
        % endif
		${plot.parameters['ant']}<br>
		T<sub>sys</sub> spw ${plot.parameters['tsys_spw']}<br>
		Science spw ${plot.parameters['spw']}
	</%def>

	<%def name="caption_text(plot)">
        % if multi_vis:
		<span class="text-center">${plot.parameters['vis']}</span><br>
        % endif
		<span class="text-center">${plot.parameters['ant']}</span><br>
		<span class="text-center">T<sub>sys</sub> spw ${plot.parameters['tsys_spw']}</span><br>
		<span class="text-center">Science spw ${plot.parameters['spw']}</span>
	</%def>
</%self:render_plots>

