<%!
rsc_path = ""
SELECTORS = ['vis', 'spw', 'pol', 'field']
%>
<%inherit file="detail_plots_basetemplate.mako"/>

<%
    multi_vis = len({p.parameters['vis'] for p in plots}) > 1
%>

<%self:render_plots plots="${sorted(plots, key=lambda p: p.parameters['spw'])}">
    % if plot.parameters.get('field', None) not in (None, 'None', ''):
	<%def name="mouseover(plot)">Click to magnify plot for ${plot.parameters['vis']} spw ${plot.parameters['spw']} polarisation ${plot.parameters['pol']} field ${plot.parameters['field']}</%def>
    % else:
	<%def name="mouseover(plot)">Click to magnify plot for ${plot.parameters['vis']} spw ${plot.parameters['spw']} polarisation ${plot.parameters['pol']}</%def>
    % endif

	<%def name="fancybox_caption(plot)">
        % if multi_vis:
		${plot.parameters['vis']}<br>
        % endif
        Spectral window: ${plot.parameters['spw']}<br>
		Polarisation: ${plot.parameters['pol']}<br>
		% if plot.parameters.get('field', None) not in (None, 'None', ''):
        Field: ${plot.parameters['field']}
        % endif
	</%def>

	<%def name="caption_text(plot)">
        % if multi_vis:
		${plot.parameters['vis']}<br>
        % endif
        Spw ${plot.parameters['spw']}<br>
		${plot.parameters['pol']}<br>
		% if plot.parameters.get('field', None) not in (None, 'None', ''):
		${plot.parameters['field']}
        % endif
	</%def>
</%self:render_plots>
