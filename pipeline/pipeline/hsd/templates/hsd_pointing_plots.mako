<%!
rsc_path = ""
SELECTORS = ['ant', 'type']
%>
<%inherit file="detail_plots_basetemplate.mako"/>

<%self:render_plots plots="${sorted(plots, key=lambda p: p.parameters['ant'])}">
	<%def name="mouseover(plot)">Click to magnify plot for antenna ${plot.parameters['ant']} ${plot.parameters['type']}</%def>

	<%def name="fancybox_caption(plot)">
		Type: ${plot.parameters['type']}<br>
		Antenna: ${plot.parameters['ant']}
	</%def>

	<%def name="caption_text(plot)">
		<span class="text-center">${plot.parameters['ant']}</span><br>
		<span class="text-center">${plot.parameters['type']}</span>		
	</%def>
</%self:render_plots>
