<%!
rsc_path = ""
SELECTORS = ['ant', 'spw', 'pol']
%>
<%inherit file="detail_plots_basetemplate.mako"/>

<%self:render_plots plots="${sorted(plots, key=lambda p: p.parameters['ant'])}">
	<%def name="mouseover(plot)">Click to magnify plot for antenna ${plot.parameters['ant']} spw ${plot.parameters['spw']} polarisation ${plot.parameters['pol']}</%def>

	<%def name="fancybox_caption(plot)">
		Antenna: ${plot.parameters['ant']}<br>
		Spectral window: ${plot.parameters['spw']}<br>
		Polarisation: ${plot.parameters['pol']}
	</%def>

	<%def name="caption_text(plot)">
	% if plot.parameters.has_key('vis'):
	    ${'_'.join(plot.parameters['vis'].split('.')[0].split('_')[-2:])}<br>
	% endif
		${plot.parameters['ant']}<br>
		Spw ${plot.parameters['spw']}<br>
		${plot.parameters['pol']}
	</%def>
</%self:render_plots>
