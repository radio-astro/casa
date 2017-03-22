<%!
rsc_path = ""
SELECTORS = ['vis', 'spw', 'scan']
%>
<%inherit file="detail_plots_basetemplate.mako"/>

<%
    multi_vis = len({p.parameters['vis'] for p in plots}) > 1
%>

<%self:render_plots plots="${sorted(plots, key=lambda p: p.parameters['spw'])}">
	<%def name="mouseover(plot)">Click to magnify plot for spw ${plot.parameters['spw']} scan ${plot.parameters['scan']}</%def>

	<%def name="fancybox_caption(plot)">
        % if multi_vis:
        ${plot.parameters['vis']}<br>
        % endif
		Spectral window: ${plot.parameters['spw']}<br>
		Scan: ${plot.parameters['scan']}
	</%def>

	<%def name="caption_text(plot)">
        % if multi_vis:
		${plot.parameters['vis']}<br>
        % endif
		Spw ${plot.parameters['spw']}<br>
		Scan ${plot.parameters['scan']}
	</%def>
</%self:render_plots>
