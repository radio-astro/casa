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
	return '<h6 style="margin-top: -11px;">(Spw%s)</h6>' % utils.commafy(spws, quotes=False, multi_prefix='s')

%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">Set model flux</%block>

<h2>Results</h2>
<p>The following flux densities were set in the measurement set model column and recorded in the pipeline context:</p>
<table class="table table-bordered table-striped" summary="Flux density results">
	<caption>Setjy Results</caption>
    <thead>
	    <tr>
	        <th scope="col" rowspan="2">Measurement Set</th>
	        <th scope="col" rowspan="2">Field</th>
	        <th scope="col" rowspan="2">SpW</th>
	        <th scope="col" rowspan="2">Centre Freq.</th>
	        <th scope="col" rowspan="2">Band</th>
	        <th scope="col" colspan="4">Flux Density</th>
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
                  break_rows_by='intent'
                  sort_row_by='baseband,spw'>
	<%def name="title()">
		Model amplitude vs UV distance
	</%def>

	<%def name="preamble()">
		<p>Plots of model amplitude vs UV distance for each Measurement Set.
		One plot is generated per Receiver band, with data shown for all antennas
		and correlations, colored by spw.</p>
	</%def>
	
	<%def name="mouseover(plot)">Click to show model amplitude vs UV distance for receiver bands ${utils.commafy(plot.parameters['receiver'], False)}</%def>

	<%def name="fancybox_caption(plot)">
		Receiver bands: ${utils.commafy(plot.parameters['receiver'], False)} (spw ${plot.parameters['spw']})
	</%def>

	<%def name="caption_title(plot)">
		Receiver bands: ${utils.commafy(plot.parameters['receiver'], False)}
	</%def>

	<%def name="caption_subtitle(plot)">
		${spws_for_baseband(plot)}
	</%def>

	<%def name="caption_text(plot, source_id)">
		Model amplitude vs UV distance for receiver bands
		${utils.commafy(plot.parameters['receiver'], False)} for ${plot.parameters['intent']}
		calibrator.
	</%def>
</%self:plot_group>

