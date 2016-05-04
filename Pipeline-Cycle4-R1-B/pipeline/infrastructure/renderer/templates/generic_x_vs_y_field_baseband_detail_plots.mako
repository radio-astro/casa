<%!
import cgi
rsc_path = ""
SELECTORS = ['vis', 'field', 'baseband']
%>
<%inherit file="detail_plots_basetemplate.mako"/>

<%
    multi_vis = len({p.parameters['vis'] for p in plots}) > 1
%>

<%self:render_plots plots="${sorted(plots, key=lambda p: p.parameters['baseband'])}">
	<%def name="mouseover(plot)">Click to magnify plot for ${cgi.escape(plot.parameters['field'], True)} baseband ${plot.parameters['baseband']}</%def>

	<%def name="fancybox_caption(plot)">
        % if multi_vis:
        ${plot.parameters['vis']}<br>
        % endif
		Field: ${cgi.escape(plot.parameters['field'], True)}<br>
		Baseband: ${plot.parameters['baseband']}
	</%def>

	<%def name="caption_text(plot)">
        % if multi_vis:
		${plot.parameters['vis']}<br>
        % endif
		${cgi.escape(plot.parameters['field'], True)}<br>
		Baseband ${plot.parameters['baseband']}
	</%def>
</%self:render_plots>
