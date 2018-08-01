<%!
rsc_path = ""
import cgi
import os
import string
import types

import pipeline.infrastructure.renderer.htmlrenderer as hr
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils

agent_description = {
	'before'   : 'Before',
	'applycal' : 'Additional',
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
	spws = format_range(plot.parameters['spw']).split(',')
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

<script>
$(document).ready(function(){
	$('.caltable_filename').tooltip({
	    'selector': '',
	    'placement': 'left',
	    'container':'body'
	  });

    $("th.rotate").each(function(){ $(this).height($(this).find('span').width() + 8) });
});
</script>

<%
# these functions are defined in template scope so we have access to the flags 
# and agents context objects

def total_for_mses(mses, row):
	flagged = 0
	total = 0
	for ms in mses:
		total += flags[ms]['before'][row].total
		for agent in flags[ms].keys():
			fs = flags[ms][agent][row]
			flagged += fs.flagged
	if total is 0:
		return 'N/A'
	else:
		return '%0.1f%%' % (100.0 * flagged / total)

def total_for_agent(agent, row, mses=flags.keys()):
	flagged = 0
	total = 0
	for ms in mses:
		if agent in flags[ms]:
			fs = flags[ms][agent][row]
			flagged += fs.flagged
			total += fs.total
		else:
			# agent was not activated for this MS. 
			total += flags[ms]['before'][row].total
	if total is 0:
		return 'N/A'
	else:
		return '%0.1f%%' % (100.0 * flagged / total)

def space_comma(s):
	return ', '.join(string.split(s, ','))

def format_range(ranges):
    #convert a ranges string (e.g., '0~2') to a string of comma separated numbers (e.g., '0,1,2')
    return str(',').join(map(str, utils.range_to_list(ranges)))
    
def format_spwmap(spwmap, scispws):
    if not spwmap:
        return ''
    else:
        spwmap_strings=[]
        for ind, spwid in enumerate(spwmap):
        	if ind in scispws:
        		spwmap_strings.append("<strong>{0}</strong>".format(spwid))
        	else:
        		spwmap_strings.append(str(spwid))
        
        return ', '.join(spwmap_strings)
%>

<%block name="title">Apply calibration tables</%block>

<p>This task applies all calibrations registered with the pipeline to their target measurement sets.<p>

<h2>Contents</h2>
<ul>
<li><a href="#appliedcal">Applied calibrations</a></li>
<li><a href="#flaggeddata">Flagged data after calibration application</a></li>
<li><a href="#plots">Plots</a></li>
  <ul>
%if amp_vs_freq_plots:
  <li><a href="#calampvsfreq">Calibrated amplitude vs frequency</a></li>
%endif
%if phase_vs_freq_plots:
  <li><a href="#calphasevsfreq">Calibrated phase vs frequency</a></li>
%endif
%if amp_vs_uv_plots:
  <li><a href="#calampvsuvdist">Calibrated amplitude vs UV distance</a></li>
%endif
%if amp_vs_time_plots:
  <li><a href="#calampvstime">Calibrated amplitude vs time</a></li>
%endif
%if phase_vs_time_plots:
  <li><a href="#calphasevstime">Calibrated phase vs time</a></li>
%endif
%if corrected_to_antenna1_plots:
  <li><a href="#corrampvsant">(Corrected amplitude / model) vs antenna</a></li>
%endif
%if corrected_to_model_vs_uvdist_plots:
  <li><a href="#corrampvsuvdist">(Corrected amplitude / model) vs UV distance</a></li>
%endif
%if science_amp_vs_freq_plots:
  <li><a href="#scicalampvsfreq">Science target: calibrated amplitude vs frequency</a></li>
%endif
%if science_amp_vs_uv_plots:
  <li><a href="#scicalampvsuvdist">Science target: calibrated amplitude vs UV distance</a></li>
%endif
%if uv_plots:
  <li><a href="#uvcoverage">UV coverage</a></li>
%endif
  </ul>
</ul>

<h2 id="appliedcal" class="jumptarget">Applied calibrations</h2>
<p>The <i>Fields</i> column lists fields within the measurement set containing any of the intents listed in the
    <i>Intents</i> column. If a field name is ambiguous and does not uniquely identify a field, e.g., when a field is
    observed with multiple intents, then the unambiguous field ID is listed instead of the field name. The order of
    entries in the <i>Fields</i> and <i>Intents</i> columns has no significance.</p>
<table class="table table-bordered table-striped table-condensed"
	   summary="Applied Calibrations">
	<caption>Applied Calibrations</caption>
	<thead>
		<tr>
			<th colspan="2">Measurement Set</th>
			<th colspan="4">Target</th>
			<th colspan="6">Calibration</th>
		</tr>
		<tr>
		    <th>Name</th>
		    <th>Final Size</th>
			<th>Intent</th>
			<th>Fields</th>
			<th>Spw</th>
			<th>Antenna</th>
			<th>Type</th>
			<th>spwmap</th>
			<th>gainfield</th>
			<th>interp</th>
			<th>calwt</th>
			<th>table</th>
		</tr>
	</thead>
	<tbody>
% for vis in calapps:
	% for calapp in calapps[vis]:
		<% ca_rowspan = len(calapp.calfrom) %>
		<tr>
			<td rowspan="${ca_rowspan}">${vis}</td>
			<td rowspan="${ca_rowspan}">${filesizes[vis]}</td>
			<td rowspan="${ca_rowspan}">${space_comma(calapp.calto.intent)}</td>
			<td rowspan="${ca_rowspan}">${space_comma(calapp.calto.field)}</td>
			<td rowspan="${ca_rowspan}">${space_comma(format_range(calapp.calto.spw))}</td>
			<td rowspan="${ca_rowspan}">${space_comma(calapp.calto.antenna)}</td>
		% for calfrom in calapp.calfrom:
			<td>${caltypes[calfrom.gaintable]}</td>
			<td>${format_spwmap(calfrom.spwmap, utils.range_to_list(calapp.calto.spw))}</td>
			<td>${space_comma(calfrom.gainfield)}</td>
			<td>${space_comma(calfrom.interp)}</td>
			<td>${calfrom.calwt}</td>
			<td><a class="caltable_filename" data-toggle="tooltip" data-placement="left" title data-original-title="${os.path.basename(calfrom.gaintable)}">Filename</a></td>
		</tr>
		% endfor
	% endfor
% endfor		
	</tbody>
</table>



<h2 id="flaggeddata" class="jumptarget">Flagged data after calibration application</h2>
<table class="table table-bordered table-striped "
	   summary="Flagged Data">
	<caption>Summary of measurement set flagging status after application
	of (potentially flagged) calibration tables. Each cell gives the
	amount of data flagged as a fraction of the specified data selection.
	</caption>
	<thead>
		<tr>
			<th rowspan="2">Data Selection</th>
			<!-- flags before task is always first agent -->
			<th colspan="${len(agents)+1}">% Flagged Data</th>
			<th colspan="${len(flags)}">Measurement Set</th>
		</tr>
		<tr>
%for agent in agents:
			<th>${agent_description[agent]}</th>
%endfor
			<th>Total</th>
%for ms in flags.keys():
			<th class="rotate"><div><span>${ms}</span></div></th>
%endfor
		</tr>
	</thead>
	<tbody>
%for k in ['TOTAL', 'SCIENCE SPWS', 'BANDPASS', 'AMPLITUDE', 'PHASE', 'TARGET']: 
		<tr>
			<th>${total_keys[k]}</th>		
	% for agent in agents:
			<td>${total_for_agent(agent, k)}</td>
	% endfor
			<td>${total_for_mses(flags.keys(), k)}</td>
	% for ms in flags.keys():
			<td>${total_for_mses([ms], k)}</td>
	% endfor		
		</tr>
%endfor
%for ms in flags.keys():
		<tr>
			<th>${ms}</th>
	% for agent in agents:
			<td>${total_for_agent(agent, 'TOTAL', [ms])}</td>
	% endfor
			<td>${total_for_mses([ms], 'TOTAL')}</td>
	% for ms in flags.keys():
			<td></td>
	% endfor
		</tr>
%endfor
	</tbody>
</table>

% if amp_vs_freq_plots or phase_vs_freq_plots or amp_vs_time_plots or amp_vs_uv_plots or phase_vs_time_plots or science_amp_vs_freq_plots or uv_plots:
<h2 id="plots" class="jumptarget">Plots</h2>

<%self:plot_group plot_dict="${amp_vs_freq_plots}"
				  url_fn="${lambda x: amp_vs_freq_subpages[x]}"
				  data_field="${True}"
				  data_spw="${True}"
                  data_vis="${True}"
				  title_id="calampvsfreq"
                  break_rows_by="intent,field"
                  sort_row_by="baseband,spw">

	<%def name="title()">
		Calibrated amplitude vs frequency
	</%def>

	<%def name="preamble()">
		Plots of calibrated amplitude vs frequency for all antennas and
		correlations, coloured by antenna. The atmospheric transmission
        for each spectral window is overlayed on each plot in pink.
	</%def>

	<%def name="mouseover(plot)">Click to show amplitude vs frequency for spw ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
		Receiver: ${utils.commafy(plot.parameters['receiver'], quotes=False)}<br>
		Spw: ${plot.parameters['spw']}<br>
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br>
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
		Spw ${plot.parameters['spw']}<br>
	</%def>

	<%def name="caption_subtitle(plot)">
		${rx_for_plot(plot)}
	</%def>

	<%def name="caption_text(plot, intent)">
		${'%s %s' % (intent.capitalize(), 'source' if intent.upper() == 'CHECK' else 'calibrator')}:
		${utils.commafy(utils.safe_split(plot.parameters['field']), quotes=False)}.
	</%def>

</%self:plot_group>


<%self:plot_group plot_dict="${phase_vs_freq_plots}"
				  url_fn="${lambda x: phase_vs_freq_subpages[x]}"
				  data_field="${True}"
				  data_spw="${True}"
				  data_vis="${True}"
				  title_id="calphasevsfreq"
                  break_rows_by="intent,field"
                  sort_row_by="baseband,spw">

	<%def name="title()">
		Calibrated phase vs frequency
	</%def>

	<%def name="preamble()">
		Plots of calibrated phase vs frequency for all antennas and
		correlations, coloured by antenna.
	</%def>


	<%def name="mouseover(plot)">Click to show phase vs frequency for spw ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
		Receiver: ${utils.commafy(plot.parameters['receiver'], quotes=False)}<br>
		Spectral window: ${plot.parameters['spw']})<br>
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br>
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
		Spectral Window ${plot.parameters['spw']}
	</%def>

	<%def name="caption_subtitle(plot)">
		${rx_for_plot(plot)}
	</%def>

	<%def name="caption_text(plot, intent)">
		${'%s %s' % (intent.capitalize(), 'source' if intent.upper() == 'CHECK' else 'calibrator')}:
		${utils.commafy(utils.safe_split(plot.parameters['field']), quotes=False)}.
	</%def>

</%self:plot_group>


<%self:plot_group plot_dict="${amp_vs_uv_plots}"
				  url_fn="${lambda x: amp_vs_uv_subpages[x]}"
                  data_field="${True}"
				  data_spw="${True}"
				  data_vis="${True}"
				  title_id="calampvsuvdist"
                  break_rows_by="intent,field"
                  sort_row_by="baseband,spw">

	<%def name="title()">
		Calibrated amplitude vs UV distance
	</%def>

	<%def name="preamble()">
		Plots of calibrated amplitude vs UV distance for the calibrators in
        each measurement set. Data are plotted for all antennas, coloured by
        correlation.
	</%def>

	<%def name="mouseover(plot)">Click to show amplitude vs UV distance for spectral window ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
		Receiver: ${utils.commafy(plot.parameters['receiver'], quotes=False)}<br>
		Spectral window: ${plot.parameters['spw']}<br>
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br>
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
		Spectral Window ${plot.parameters['spw']}
	</%def>

	<%def name="caption_subtitle(plot)">
		${rx_for_plot(plot)}
	</%def>

	<%def name="caption_text(plot, intent)">
        ${utils.commafy(plot.parameters['intent'], False).capitalize()} ${'source' if intent.upper() == 'CHECK' else 'calibrator'}:
        ${plot.parameters['field']}
	</%def>

</%self:plot_group>


<%self:plot_group plot_dict="${amp_vs_time_plots}"
				  url_fn="${lambda x: amp_vs_time_subpages[x]}"
				  data_vis="${True}"
				  data_spw="${True}"
				  title_id="calampvstime"
                  break_rows_by="intent,field"
                  sort_row_by="baseband,spw">

	<%def name="title()">
		Calibrated amplitude vs time
	</%def>

	<%def name="preamble()">
		Plots of calibrated amplitude vs time for all fields, antennas and
		correlations. Data are coloured by field.
	</%def>

	<%def name="mouseover(plot)">Click to show amplitude vs time for spectral window ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
		Receiver: ${utils.commafy(plot.parameters['receiver'], quotes=False)}<br>
		Spectral window: ${plot.parameters['spw']}<br>
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br>
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
		Spectral Window ${plot.parameters['spw']}
	</%def>

	<%def name="caption_subtitle(plot)">
		${rx_for_plot(plot)}
	</%def>

</%self:plot_group>


<%self:plot_group plot_dict="${phase_vs_time_plots}"
				  url_fn="${lambda x: phase_vs_time_subpages[x]}"
				  data_vis="${True}"
                  data_spw="${True}"
                  title_id="calphasevstime"
                  break_rows_by="intent,field"
                  sort_row_by="baseband,spw">

	<%def name="title()">
		Calibrated phase vs time
	</%def>

	<%def name="preamble()">
		Plots of calibrated phase vs time for all antennas and correlations.
        Data are coloured by field.
	</%def>

	<%def name="mouseover(plot)">Click to show phase vs time for spectral window ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
		Receiver: ${utils.commafy(plot.parameters['receiver'], quotes=False)}<br>
		Spectral window: ${plot.parameters['spw']}<br>
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br>
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
		Spectral Window ${plot.parameters['spw']}
	</%def>

	<%def name="caption_subtitle(plot)">
		${rx_for_plot(plot)}
	</%def>

</%self:plot_group>


<%self:plot_group plot_dict="${corrected_to_antenna1_plots}"
				  url_fn="${lambda x: 'junk'}"
				  title_id="corrampvsant"
                  break_rows_by="intent,field"
                  sort_row_by="baseband,spw">

	<%def name="title()">
        (Corrected amplitude / model) vs antenna
	</%def>

	<%def name="preamble()">
		Plots of the ratio of the corrected amplitude to the model column
        value versus antenna ID. Data are coloured by antenna and are shown
        for all antennas and correlations.
	</%def>

    <%def name="ms_preamble(ms)">
	% if uv_max[ms].value > 0.0:
		<p>Plots for AMPLITUDE calibration intent were created with UV
		range set to capture the inner half of the data
		(UV max < ${str(uv_max[ms])}). Plots for other intents have no UV
		range restriction.</p>
	% endif
	</%def>

	<%def name="mouseover(plot)">Click to show the ratio of the corrected amplitude to model column for spw ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
		Spectral window: ${plot.parameters['spw']}<br>
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br>
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
		Spectral Window ${plot.parameters['spw']}<br>
	</%def>

	<%def name="caption_subtitle(plot)">
		${rx_for_plot(plot)}
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br>
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

</%self:plot_group>


<%self:plot_group plot_dict="${corrected_to_model_vs_uvdist_plots}"
				  url_fn="${lambda x: 'junk'}"
				  title_id="corrampvsuvdist"
                  break_rows_by="intent,field"
                  sort_row_by="baseband,spw">

	<%def name="title()">
		(Corrected amplitude / model) vs UV distance
	</%def>

	<%def name="preamble()">
		Plots of the ratio of the corrected amplitude to the model column
        value versus UV distance. Data are coloured by antenna and are
        shown for all antennas and correlations.
	</%def>

    <%def name="ms_preamble(ms)">
	% if uv_max[ms].value > 0.0:
		<p>Plots for ${ms} were created with UV range set to capture the inner
		half of the data (UV max < ${str(uv_max[ms])}).</p>
	% endif
	</%def>

	<%def name="mouseover(plot)">Click to show the ratio of the corrected amplitude to model column for spw ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
		Spectral window: ${plot.parameters['spw']}<br>
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br>
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
		Spectral Window ${plot.parameters['spw']}<br>
	</%def>

	<%def name="caption_subtitle(plot)">
		${rx_for_plot(plot)}
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br>
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

</%self:plot_group>


<%self:plot_group plot_dict="${science_amp_vs_freq_plots}"
				  url_fn="${lambda x: 'science_amp_vs_freq-all_data.html'}"
				  data_spw="${True}"
				  data_field="${True}"
                  data_vis="${True}"
				  title_id="scicalampvsfreq"
                  break_rows_by="intent,field"
                  sort_row_by="baseband,spw">

	<%def name="title()">
		Science target: calibrated amplitude vs frequency
	</%def>

	<%def name="preamble()">
	% if uv_max[ms].value > 0.0:
		<p>Calibrated amplitude vs frequency plots for a representative
		science field in each measurement set. The science field displayed
		here is the first field for the source. The atmospheric transmission
        for each spectral window is overlayed on each plot in pink.</p>
    % else: #Single dish (source = field, so far)
		<p>Calibrated amplitude vs frequency plots of each source in each 
		measurement set. The atmospheric transmission for each spectral window is
        overlayed on each plot in pink.</p>
	% endif
		
		<p>Data are plotted for all antennas and correlations, with different
		spectral windows shown in different colours.</p>
	</%def>

	<%def name="mouseover(plot)">Click to show amplitude vs frequency for spw ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
		Receiver: ${utils.commafy(plot.parameters['receiver'], quotes=False)}<br>
		Spw: ${plot.parameters['spw']}<br>
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br>
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
		Spw ${plot.parameters['spw']}
	</%def>

	<%def name="caption_subtitle(plot)">
		${rx_for_plot(plot)}
	</%def>

	<%def name="caption_text(plot, source_id)">
		Source #${source_id}
		(${utils.commafy(utils.safe_split(plot.parameters['field']), quotes=False)})
	</%def>

</%self:plot_group>

<%self:plot_group plot_dict="${science_amp_vs_uv_plots}"
				  url_fn="${lambda x: 'science_amp_vs_uv-all_data.html'}"
				  data_spw="${True}"
				  data_field="${True}"
                  data_vis="${True}"
				  title_id="scicalampvsuvdist"
                  break_rows_by="intent,field"
                  sort_row_by="baseband,spw">

	<%def name="title()">
		Science target: calibrated amplitude vs UV distance
	</%def>

	<%def name="preamble()">
		<p>Calibrated amplitude vs UV distance plots for a representative
		science field in each measurement set. The science field displayed 
		here is the first field for the source.</p>

		<p>Data are plotted for all antennas and correlations, with different
		spectral windows shown in different colours.</p>
	</%def>

	<%def name="mouseover(plot)">Click to show amplitude vs UV distance for spw ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
		Receiver: ${utils.commafy(plot.parameters['receiver'], quotes=False)}<br>
		Spw: ${plot.parameters['spw']}<br>
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br>
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
		Spw ${plot.parameters['spw']}
	</%def>

	<%def name="caption_subtitle(plot)">
		${rx_for_plot(plot)}
 	</%def>

	<%def name="caption_text(plot, source_id)">
		Source #${source_id}
		(${utils.commafy(utils.safe_split(plot.parameters['field']), quotes=False)}). 
	</%def>

</%self:plot_group>

<%self:plot_group plot_dict="${uv_plots}"
				  url_fn="${lambda ms: 'noop'}"
				  title_id="uvcoverage"
				  break_rows_by=""
                  sort_row_by="">
	<%def name="title()">
		UV coverage
	</%def>

	<%def name="preamble()">
		<p>Plots of UV coverage for each Measurement Set.</p>
	</%def>

	<%def name="mouseover(plot)">Click to show UV coverage for ${plot.parameters['vis']}, TARGET field ${cgi.escape(plot.parameters['field_name'], True)} (#${plot.parameters['field']}), spw ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
        Vis: ${plot.parameters['vis']}<br>
        Field: ${cgi.escape(plot.parameters['field_name'], True)} (#${plot.parameters['field']})<br>
        Spw: ${cgi.escape(plot.parameters['spw'])}
	</%def>

	<%def name="caption_title(plot)">
	</%def>

	<%def name="caption_subtitle(plot)">
 	</%def>

	<%def name="caption_text(plot, _)">
		UV coverage plot for TARGET field ${cgi.escape(plot.parameters['field_name'], True)}
        (#${plot.parameters['field']}), spw ${plot.parameters['spw']}
	</%def>
</%self:plot_group>

%endif

