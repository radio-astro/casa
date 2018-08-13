<%!
rsc_path = ""
SELECTORS = ['vis', 'ant', 'spw', 'pol']
%>
<%inherit file="detail_plots_basetemplate.mako"/>

<%
    multi_vis = len({p.parameters['vis'] for p in plots}) > 1
%>

<%self:render_plots plots="${sorted(plots, key=lambda p: p.parameters['spw'])}">
	<%def name="mouseover(plot)">Click to magnify plot for ${plot.parameters['vis']} ${plot.parameters['ant']} spw ${plot.parameters['spw']} ${plot.parameters['pol']} polarisation</%def>

	<%def name="fancybox_caption(plot)">
        % if multi_vis:
        ${plot.parameters['vis']}<br>
        % endif
		Antenna: ${plot.parameters['ant']}<br>
		Spectral Window: ${plot.parameters['spw']}<br>
		Polarisation: ${plot.parameters['pol']}
	</%def>

	<%def name="caption_text(plot)">
        % if multi_vis:
		${plot.parameters['vis']}<br>
        % endif
		${plot.parameters['ant']}<br>
		Spw ${plot.parameters['spw']}<br>
		Pol ${plot.parameters['pol']}
	</%def>
</%self:render_plots>
