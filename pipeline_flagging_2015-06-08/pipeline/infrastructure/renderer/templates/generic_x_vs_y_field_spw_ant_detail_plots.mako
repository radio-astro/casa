<%!
import cgi
rsc_path = ""
SELECTORS = ['field', 'spw', 'ant']
%>
<%inherit file="detail_plots_basetemplate.mako"/>

<%self:render_plots plots="${sorted(plots, key=lambda p: p.parameters['ant'])}">
	<%def name="mouseover(plot)">Click to magnify plot for ${cgi.escape(plot.parameters['field'], True)} antenna ${plot.parameters['ant']} spw ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
		Field: ${cgi.escape(plot.parameters['field'], True)}<br>
		Spectral window: ${plot.parameters['spw']}<br>
		Antenna: ${plot.parameters['ant']}
	</%def>

	<%def name="caption_text(plot)">
		${cgi.escape(plot.parameters['field'], True)}<br>
		Spw ${plot.parameters['spw']}<br>
		${plot.parameters['ant']}
	</%def>
</%self:render_plots>
