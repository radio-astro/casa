<%!
rsc_path = ""
SELECTORS = ['spw', 'scan']
%>
<%inherit file="detail_plots_basetemplate.mako"/>

<%self:render_plots plots="${sorted(plots, key=lambda p: p.parameters['spw'])}">
	<%def name="mouseover(plot)">Click to magnify plot for spw ${plot.parameters['spw']} scan ${plot.parameters['scan']}</%def>

	<%def name="fancybox_caption(plot)">
		Spectral window: ${plot.parameters['spw']}<br>
		Scan: ${plot.parameters['scan']}
	</%def>

	<%def name="caption_text(plot)">
		<span class="text-center">Spw ${plot.parameters['spw']}</span><br>
		<span class="text-center">Scan ${plot.parameters['scan']}</span>		
	</%def>
</%self:render_plots>
