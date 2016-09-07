<%!
rsc_path = ""
SELECTORS = ['spw', 'type']
%>
<%inherit file="detail_plots_basetemplate.mako"/>

<%self:render_plots plots="${sorted(plots, key=lambda p: p.parameters['spw'])}">
	<%def name="mouseover(plot)">Click to magnify plot for spw ${plot.parameters['spw']} ${plot.parameters['type']}</%def>

	<%def name="fancybox_caption(plot)">
		Type: ${plot.parameters['type']}<br>
		Spectral Window: ${plot.parameters['spw']}
	</%def>

	<%def name="caption_text(plot)">
		<span class="text-center">${plot.parameters['type']}</span><br>	
		<span class="text-center">Spw ${plot.parameters['spw']}</span>
	</%def>
</%self:render_plots>
