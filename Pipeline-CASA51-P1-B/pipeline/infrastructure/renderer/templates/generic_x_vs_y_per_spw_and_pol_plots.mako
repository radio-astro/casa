<%!
rsc_path = ""
SELECTORS = ['spw', 'pol']
%>
<%inherit file="detail_plots_basetemplate.mako"/>

<%self:render_plots plots="${sorted(plots, key=lambda p: p.parameters['spw'])}">
	<%def name="mouseover(plot)">Click to magnify plot for spw ${plot.parameters['spw']} polarisation ${plot.parameters['pol']}</%def>

	<%def name="fancybox_caption(plot)">
		Spectral window: ${plot.parameters['spw']}<br>
		Polarisation: ${plot.parameters['pol']}
	</%def>

	<%def name="caption_text(plot)">
		<span class="text-center">Spw ${plot.parameters['spw']}</span><br>
		<span class="text-center">${plot.parameters['pol']}</span>		
	</%def>
</%self:render_plots>
