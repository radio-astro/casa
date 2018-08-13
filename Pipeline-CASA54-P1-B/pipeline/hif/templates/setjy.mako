<%!
rsc_path = ""
import cgi
import os
import string
import types

import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils

agent_description = {
	'before'   : 'Before',
	'applycal' : 'After',
}

total_keys = {
	'TOTAL'        : 'All Data',
	'SCIENCE SPWS' : 'Science Spectral Windows',
	'BANDPASS'     : 'Bandpass',
	'AMPLITUDE'    : 'Flux',
	'PHASE'        : 'Phase',
	'TARGET'       : 'Target'
}

def template_agent_header1(agent):
	span = 'col' if agent in ('online','template') else 'row'
	return '<th %sspan=2>%s</th>' % (span, agent_description[agent])

def template_agent_header2(agent):
	if agent in ('online', 'template'):
		return '<th>File</th><th>Number of Statements</th>'
	else:
		return ''		

def get_template_agents(agents):
	return [a for a in agents if a in ('online', 'template')]

def sanitise(url):
	return filenamer.sanitize(url)

def spws_for_baseband(plot):
	spws = plot.parameters['spw'].split(',')
	if not spws:
		return ''
	return '<h6 style="margin-top: -11px;">Spw%s</h6>' % utils.commafy(spws, quotes=False, multi_prefix='s')

def rx_for_plot(plot):
	rx = plot.parameters['receiver']
	if not rx:
		return ''
	rx_string = utils.commafy(rx, quotes=False)
	# Don't need receiver prefix for ALMA bands
	if 'ALMA' not in rx_string:
		prefix = 'Receiver bands: ' if len(rx) > 1 else 'Receiver band: '
	else:
		prefix = ''
	return '<h6 style="margin-top: -11px;">%s%s</h6>' % (prefix, rx_string)


%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">Set model flux</%block>

<h2>Results</h2>
<p>The following flux densities were set in the measurement set model column and recorded in the pipeline context.
Only the spectral index of the bandpass calibrator is set here and its flux density will be set later.</p>
<table class="table table-bordered table-striped" summary="Flux density results">
	<caption>Setjy Results</caption>
    <thead>
	    <tr>
	        <th scope="col" rowspan="2">Measurement Set</th>
	        <th scope="col" rowspan="2">Field</th>
	        <th scope="col" rowspan="2">SpW</th>
	        <th scope="col" rowspan="2">Centre Freq</th>
	        <th scope="col" rowspan="2">Band</th>
	        <th scope="col" colspan="4">Flux Density</th>
	        <th scope="col" rowspan="2">Spix</th>
	        <th scope="col" rowspan="2">flux.csv</th>
		</tr>
		<tr>
	        <th scope="col">I</th>
	        <th scope="col">Q</th>
	        <th scope="col">U</th>
	        <th scope="col">V</th>
	    </tr>
	</thead>
	<tbody>
	% for tr in table_rows:
		<tr>
		% for td in tr:
			${td}
		% endfor
		</tr>
	%endfor
	</tbody>
</table>


<%self:plot_group plot_dict="${amp_vs_uv_plots}"
				  url_fn="${lambda ms: 'noop'}"
				  break_rows_by=""
                  sort_row_by="baseband">
	<%def name="title()">
		Model amplitude vs UV distance
	</%def>

	<%def name="preamble()">
		<p>Plots of model amplitude vs UV distance for each Measurement Set.
		One plot is generated per baseband, with data shown for all antennas
		and correlations, colored by spw.</p>
	</%def>
	
	<%def name="mouseover(plot)">Click to show model amplitude vs UV distance for baseband ${plot.parameters['baseband']}</%def>

	<%def name="fancybox_caption(plot)">
		Receiver: ${utils.commafy(plot.parameters['receiver'], quotes=False)}<br>
		Baseband: ${plot.parameters['baseband']} (spw ${plot.parameters['spw']})<br>
	</%def>

	<%def name="caption_title(plot)">
		Baseband: ${plot.parameters['baseband']}
	</%def>

	<%def name="caption_subtitle(plot)">
		${rx_for_plot(plot)}
 		${spws_for_baseband(plot)}
 	</%def>

	<%def name="caption_text(plot, source_id)">
		Model amplitude vs UV distance in baseband
		${plot.parameters['baseband']} for ${plot.parameters['intent']}
		calibrator.
	</%def>
</%self:plot_group>

