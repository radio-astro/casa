<%!
import cgi
rsc_path = ""
SELECTORS = ['spw', 'field']
%>
<%inherit file="detail_plots_basetemplate.mako"/>

<%self:render_plots plots="${sorted(plots, key=lambda p: cgi.escape(p.parameters['field'], True))}">
	<%def name="mouseover(plot)">Click to magnify plot for ${cgi.escape(plot.parameters['field'], True)} spw ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
		Field: ${cgi.escape(plot.parameters['field'], True)}<br>
		Spectral window: ${plot.parameters['spw']}
	</%def>

	<%def name="caption_text(plot)">
		<span class="text-center">${cgi.escape(plot.parameters['field'], True)}</span><br>		
		<span class="text-center">Spw ${plot.parameters['spw']}</span>
	</%def>
</%self:render_plots>
