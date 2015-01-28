<%!
rsc_path = ""
SELECTORS = ['ant', 'spw']
%>
<%inherit file="detail_plots_basetemplate.mako"/>

<%self:render_plots plots="${sorted(plots, key=lambda p: p.parameters['ant'])}">
	<%def name="mouseover(plot)">Click to magnify plot for ${plot.parameters['ant']} spw ${plot.parameters['spw']} </%def>

	<%def name="fancybox_caption(plot)">
		Antenna: ${plot.parameters['ant']}<br>
		Spectral window: ${plot.parameters['spw']}
	</%def>

	<%def name="caption_text(plot)">
		<span class="text-center">${plot.parameters['ant']}</span><br>		
		<span class="text-center">Spw ${plot.parameters['spw']}</span>
	</%def>
</%self:render_plots>
