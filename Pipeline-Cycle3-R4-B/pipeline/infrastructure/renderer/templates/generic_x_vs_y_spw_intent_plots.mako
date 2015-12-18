<%!
rsc_path = ""
SELECTORS = ['spw', 'intent']
%>
<%inherit file="detail_plots_basetemplate.mako"/>

<%self:render_plots plots="${sorted(plots, key=lambda p: p.parameters['spw'])}">
	<%def name="mouseover(plot)">Click to magnify plot for Spw ${plot.parameters['spw']} ${plot.parameters['intent']}</%def>

	<%def name="fancybox_caption(plot)">
		Spectral window: ${plot.parameters['spw']}<br>
		Intent: ${plot.parameters['intent']}
	</%def>

	<%def name="caption_text(plot)">
		<span class="text-center">Spw ${plot.parameters['spw']}</span><br>
		<span class="text-center">${plot.parameters['intent']}</span>
	</%def>
</%self:render_plots>
