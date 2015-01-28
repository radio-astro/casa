<%!
import cgi
rsc_path = ""
SELECTORS = ['field', 'baseband']
%>
<%inherit file="detail_plots_basetemplate.mako"/>

<%self:render_plots plots="${sorted(plots, key=lambda p: p.parameters['baseband'])}">
	<%def name="mouseover(plot)">Click to magnify plot for ${cgi.escape(plot.parameters['field'], True)} baseband ${plot.parameters['baseband']}</%def>

	<%def name="fancybox_caption(plot)">
		Field: ${cgi.escape(plot.parameters['field'], True)}<br>
		Baseband: ${plot.parameters['baseband']}
	</%def>

	<%def name="caption_text(plot)">
		<span class="text-center">${cgi.escape(plot.parameters['field'], True)}</span><br>
		<span class="text-center">Baseband ${plot.parameters['baseband']}</span>
	</%def>
</%self:render_plots>
