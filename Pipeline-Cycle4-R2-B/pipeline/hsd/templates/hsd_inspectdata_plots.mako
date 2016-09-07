<%!
rsc_path = ""
SELECTORS = ['ant']
%>
<%inherit file="detail_plots_basetemplate.mako"/>

<%self:render_plots plots="${sorted(plots, key=lambda p: p.parameters['ant'])}">
	<%def name="mouseover(plot)">Click to magnify plot for antenna ${plot.parameters['ant']} </%def>

	<%def name="fancybox_caption(plot)">
		Antenna: ${plot.parameters['ant']}
	</%def>

	<%def name="caption_text(plot)">
		${plot.parameters['ant']}
	</%def>
</%self:render_plots>
