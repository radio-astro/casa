<%!
rsc_path = ""
SELECTORS = ['vis', 'spw']
%>
<%inherit file="detail_plots_basetemplate.mako"/>

<%
    multi_vis = len({p.parameters['vis'] for p in plots}) > 1
%>

<%self:render_plots plots="${sorted(plots, key=lambda p: p.parameters['spw'])}">
	<%def name="mouseover(plot)">Click to magnify plot for ${plot.parameters['vis']} spw ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
        % if multi_vis:
        ${plot.parameters['vis']}<br>
        % endif
		Spectral window: ${plot.parameters['spw']}
	</%def>

	<%def name="caption_text(plot)">
        % if multi_vis:
        ${plot.parameters['vis']}<br>
        % endif
		<span class="text-center">Spw ${plot.parameters['spw']}</span>
	</%def>
</%self:render_plots>
