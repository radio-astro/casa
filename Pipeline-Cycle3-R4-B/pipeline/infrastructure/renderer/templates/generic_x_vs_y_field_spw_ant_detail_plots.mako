<%!
import cgi
rsc_path = ""
SELECTORS = ['vis', 'field', 'spw', 'ant']
%>
<%inherit file="detail_plots_basetemplate.mako"/>

<%
    multi_vis = len({p.parameters['vis'] for p in plots}) > 1
%>

<%self:render_plots plots="${sorted(plots, key=lambda p: p.parameters['ant'])}">
	<%def name="mouseover(plot)">Click to magnify plot for ${cgi.escape(plot.parameters['field'], True)} antenna ${plot.parameters['ant']} spw ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
        % if multi_vis:
        ${plot.parameters['vis']}<br>
        % endif
		Field: ${cgi.escape(plot.parameters['field'], True)}<br>
		Spectral window: ${plot.parameters['spw']}<br>
		Antenna: ${plot.parameters['ant']}
	</%def>

	<%def name="caption_text(plot)">
        % if multi_vis:
		${plot.parameters['vis']}<br>
        % endif
		${cgi.escape(plot.parameters['field'], True)}<br>
		Spw ${plot.parameters['spw']}<br>
		${plot.parameters['ant']}
	</%def>
</%self:render_plots>
