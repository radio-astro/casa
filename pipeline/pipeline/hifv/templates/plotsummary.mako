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
	spws = plot.parameters['spw'].split(',')
	if not spws:
		return ''
	return '<h6 style="margin-top: -11px;">(Spw%s)</h6>' % utils.commafy(spws, quotes=False, multi_prefix='s')


%>


<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">Plot Summary</%block>

<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>

<script>
$(document).ready(function(){
    // return a function that sets the select text field to the given value
    var createSelectSetter = function(val, element_id) {
    	return function() {
    		var vals = val.toString().split(',');
    		// trigger a change event, otherwise the filters are not changed
            $(element_id).select2("val", vals).trigger("change");
        };
    };

    // create a callback function for each overview plot that will select the
    // appropriate spw once the page has loaded
    $(".thumbnail a").each(function (i, v) {
        var o = $(v);
        var spw = o.data("spw");
        var field = o.data("field");
        var baseband = o.data("baseband");
        var callbacks = [];
        if (typeof field !== 'undefined') {
        	callbacks.push(createSelectSetter(field, "#select-field"));
        }
        if (typeof spw !== 'undefined') {
        	callbacks.push(createSelectSetter(spw, "#select-spw"));
        }
        if (typeof baseband !== 'undefined') {
        	callbacks.push(createSelectSetter(baseband, "#select-baseband"));
        }
	    o.data("callback", callbacks);
    });

    $(".fancybox").fancybox({
        type: 'image',
        prevEffect: 'none',
        nextEffect: 'none',
        loop: false,
        helpers: {
            title: {
                type: 'outside'
            },
            thumbs: {
                width: 50,
                height: 50,
            }
        },
    	beforeShow : function() {
        	this.title = $(this.element).attr('title');
       	}
    });

	$('.caltable_filename').tooltip({
	    'selector': '',
	    'placement': 'left',
	    'container':'body'
	  });
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

%>


<%self:plot_group plot_dict="${summary_plots}"
                  url_fn="${lambda ms: 'noop'}"
                  break_rows_by=''
                  sort_row_by='baseband,spw'>

        <%def name="title()">
            VLA Pipeline Summary Plots
        </%def>

        <%def name="preamble()">
           Phase vs. time for all calibrators, Amp vs. UVwave for all calibrators, as well as a representative selection of fields with intent='TARGET' with Amp vs. UVwave plots.

        </%def>


        <%def name="mouseover(plot)">Summary window </%def>



        <%def name="fancybox_caption(plot)">
          Plot of ${plot.y_axis} vs. ${plot.x_axis}
        </%def>


        <%def name="caption_title(plot)">
           ${plot.parameters['type']}
        </%def>
</%self:plot_group>


% if amp_vs_freq_plots or phase_vs_freq_plots or  amp_vs_time_plots or amp_vs_uv_plots or phase_vs_uv_plots or phase_vs_time_plots:

<%self:plot_group plot_dict="${amp_vs_freq_plots}"
				  url_fn="${lambda x: 'amp_vs_freq_%s.html' % sanitise(x)}"
				  data_field="${True}"
				  data_spw="${True}"
                  break_rows_by=""
                  sort_row_by="intent,field,baseband,spw">

	<%def name="title()">
		Calibrated amplitude vs frequency
	</%def>

	<%def name="preamble()">
		Plots of calibrated amplitude vs frequency for all antennas and
		correlations, coloured by antenna.
	</%def>

	<%def name="mouseover(plot)">Click to show amplitude vs frequency for receiver bands ${utils.commafy(plot.parameters['receiver'], False)}</%def>

	<%def name="fancybox_caption(plot)">
		Receiver bands: ${utils.commafy(plot.parameters['receiver'], False)} (spw ${plot.parameters['spw']})<br>
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br>
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
		Receiver bands: ${utils.commafy(plot.parameters['receiver'], False)}
	</%def>

	<%def name="caption_subtitle(plot)">
		${spws_for_baseband(plot)}
	</%def>

	<%def name="caption_text(plot, intent)">
		${intent.capitalize()} calibrator:
		${utils.commafy(utils.safe_split(plot.parameters['field']), quotes=False)}.
	</%def>

</%self:plot_group>


<%self:plot_group plot_dict="${phase_vs_freq_plots}"
				  url_fn="${lambda x: 'phase_vs_freq_%s.html' % sanitise(x)}"
				  data_field="${True}"
				  data_spw="${True}"
                  break_rows_by=""
                  sort_row_by="intent,field,baseband,spw">

	<%def name="title()">
		Calibrated phase vs frequency
	</%def>

	<%def name="preamble()">
		Plots of calibrated phase vs frequency for all antennas and
		correlations, coloured by antenna.
	</%def>


	<%def name="mouseover(plot)">Click to show phase vs frequency for baseband ${utils.commafy(plot.parameters['receiver'], False)}</%def>

	<%def name="fancybox_caption(plot)">
		Receiver bands: ${utils.commafy(plot.parameters['receiver'], False)} (spw ${plot.parameters['spw']})<br>
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br>
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
		Receiver bands: ${utils.commafy(plot.parameters['receiver'], False)}
	</%def>

	<%def name="caption_subtitle(plot)">
		${spws_for_baseband(plot)}
	</%def>

	<%def name="caption_text(plot, intent)">
		${intent.capitalize()} calibrator:
		${utils.commafy(utils.safe_split(plot.parameters['field']), quotes=False)}.
	</%def>

</%self:plot_group>


<%self:plot_group plot_dict="${amp_vs_uv_plots}"
				  url_fn="${lambda x: 'amp_vs_uv_%s.html' % sanitise(x)}"
				  data_spw="${True}"
                  break_rows_by=""
                  sort_row_by="intent,field,baseband,spw">

	<%def name="title()">
		Calibrated amplitude vs UV distance
	</%def>

	<%def name="preamble()">
		Plots of calibrated amplitude vs UV distance for the amplitude
		calibrator in each measurement set. Data are plotted for all antennas,
		coloured by correlation.
	</%def>

	<%def name="mouseover(plot)">Click to show amplitude vs UV distance for spectral window ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
		Spectral window: ${plot.parameters['spw']}<br>
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br>
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
		Spectral Window ${plot.parameters['spw']}
	</%def>

	<%def name="caption_text(plot, intent)">
		Amplitude calibrator: ${plot.parameters['field']}
	</%def>

</%self:plot_group>


<%self:plot_group plot_dict="${amp_vs_time_plots}"
				  url_fn="${lambda x: 'amp_vs_time_%s.html' % sanitise(x)}"
				  data_spw="${True}"
                  break_rows_by=""
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
		Spectral window: ${plot.parameters['spw']}<br>
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br>
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
		Spectral Window ${plot.parameters['spw']}
	</%def>

</%self:plot_group>


<%self:plot_group plot_dict="${phase_vs_time_plots}"
				  url_fn="${lambda x: 'phase_vs_time_%s.html' % sanitise(x)}"
				  data_spw="${True}"
                  break_rows_by=""
                  sort_row_by="baseband,spw">

	<%def name="title()">
		Calibrated phase vs time
	</%def>

	<%def name="preamble()">
		Plots of calibrated phase vs time for all fields, antennas and
		correlations. Data are coloured by field.
	</%def>

	<%def name="mouseover(plot)">Click to show phase vs time for spectral window ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
		Spectral window: ${plot.parameters['spw']}<br>
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br>
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
		Spectral Window ${plot.parameters['spw']}
	</%def>

</%self:plot_group>

<%self:plot_group plot_dict="${science_amp_vs_freq_plots}"
				  url_fn="${lambda x: 'science_amp_vs_freq_%s.html' % sanitise(x)}"
				  data_spw="${True}"
				  data_field="${True}"
                  break_rows_by=""
                  sort_row_by="baseband,spw">

	<%def name="title()">
		Science target: calibrated amplitude vs frequency
	</%def>

	<%def name="preamble()">
		<p>Calibrated amplitude vs frequency plots for a representative
		science field in each measurement set. The science field displayed
		here is the one with the brightest average amplitude over all spectral
		windows.</p>

		<p>Note: due to a technical problem with visstat, the science field
		displayed here not the brightest field for the source but the first
		field for the source.</p>

		<p>Data are plotted for all antennas and correlations, with different
		spectral windows shown in different colours.</p>
	</%def>

	<%def name="ms_preamble(ms)">
		<p>Plots for ${ms} were created with UV range set to capture the inner
		half of the data (UV max < ${str(uv_max[ms])}).</p>
	</%def>

	<%def name="mouseover(plot)">Click to show amplitude vs frequency for receiver bands ${utils.commafy(plot.parameters['receiver'], False)}</%def>

	<%def name="fancybox_caption(plot)">
		Receiver bands: ${utils.commafy(plot.parameters['receiver'], False)} (spw ${plot.parameters['spw']})<br>
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br>
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
		Receiver bands: ${utils.commafy(plot.parameters['receiver'], False)}
	</%def>

	<%def name="caption_subtitle(plot)">
		${spws_for_baseband(plot)}
	</%def>

	<%def name="caption_text(plot, source_id)">
		Source #${source_id}
		(${utils.commafy(utils.safe_split(plot.parameters['field']), quotes=False)})
	</%def>

</%self:plot_group>

<%self:plot_group plot_dict="${science_phase_vs_freq_plots}"
				  url_fn="${lambda x: 'science_phase_vs_freq_%s.html' % sanitise(x)}"
				  data_baseband="${True}"
				  data_field="${True}"
                  break_rows_by=""
                  sort_row_by="intent,field,baseband,spw">

	<%def name="title()">
		Science target: calibrated phase vs frequency
	</%def>

	<%def name="preamble()">
		<p>Calibrated phase vs frequency plots for a representative science
		field in each measurement set. The science field displayed here is the
		one with the brightest average amplitude over all spectral windows.</p>

		<p>Note: due to a technical problem with visstat, the science field
		displayed here not the brightest field for the source but the first
		field for the source.</p>

		<p>Data are plotted for all antennas and correlations, with different
		spectral windows shown in different colours.</p>
	</%def>

	<%def name="ms_preamble(ms)">
		<p>Plots for ${ms} were created with UV range set to capture the inner
		half of the data (UV max < ${str(uv_max[ms])}).</p>
	</%def>

	<%def name="mouseover(plot)">Click to show phase vs frequency for baseband ${utils.commafy(plot.parameters['receiver'], False)}</%def>

	<%def name="fancybox_caption(plot)">
		Receiver: ${utils.commafy(plot.parameters['receiver'], False)} (spw ${plot.parameters['spw']})<br>
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br>
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
	        Receiver ${utils.commafy(plot.parameters['receiver'], False)}
	</%def>

	<%def name="caption_subtitle(plot)">
		${spws_for_baseband(plot)}
	</%def>

	<%def name="caption_text(plot, source_id)">
		Source #${source_id}
		(${utils.commafy(utils.safe_split(plot.parameters['field']), quotes=False)}).
	</%def>

</%self:plot_group>

<%self:plot_group plot_dict="${science_amp_vs_uv_plots}"
				  url_fn="${lambda x: 'science_amp_vs_uv_%s.html' % sanitise(x)}"
				  data_baseband="${True}"
				  data_field="${True}"
                  break_rows_by=""
                  sort_row_by="intent,field,baseband,spw">

	<%def name="title()">
		Science targets: calibrated amplitude vs UV distance
	</%def>

	<%def name="preamble()">
		<p>Calibrated amplitude vs UV distance plots for a representative
		science field in each measurement set. The science field displayed
		here is the one with the brightest average amplitude over all spectral
		windows.</p>

		<p>Note: due to a technical problem with visstat, the science field
		displayed here not the brightest field for the source but the first
		field for the source.</p>

		<p>Data are plotted for all antennas and correlations, with different
		spectral windows shown in different colours.</p>
	</%def>

	<%def name="mouseover(plot)">Click to show amplitude vs UV distance for receiver bands ${utils.commafy(plot.parameters['receiver'], False)}</%def>

	<%def name="fancybox_caption(plot)">
		Receiver bands: ${utils.commafy(plot.parameters['receiver'], False)} (spw ${plot.parameters['spw']})<br>
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br>
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
	        Receiver bands: ${utils.commafy(plot.parameters['receiver'], False)}
	</%def>

	<%def name="caption_subtitle(plot)">
		${spws_for_baseband(plot)}
	</%def>

	<%def name="caption_text(plot, source_id)">
		Source #${source_id}
		(${utils.commafy(utils.safe_split(plot.parameters['field']), quotes=False)}).
	</%def>

</%self:plot_group>

%endif


%if use_pol_plots:

<%self:plot_group plot_dict="${phase_vs_freq_polarization_plots}"
				  url_fn="${lambda x: 'phase_vs_freq_polarization_%s.html' % sanitise(x)}"
				  data_field="${True}"
				  data_spw="${True}"
                  break_rows_by=""
                  sort_row_by="intent,field,baseband,spw">

	<%def name="title()">
		Calibrated phase vs frequency, intent='POLANGLE, POLLEAKAGE, BANDPASS, PHASE'
	</%def>

	<%def name="preamble()">
		Plots of calibrated phase vs frequency for all antennas and
		correlation='RL,LR', intent=POLANGLE, POLLEAKAGE, BANDPASS and PHASE.
	</%def>


	<%def name="mouseover(plot)">Click to show phase vs frequency for baseband ${utils.commafy(plot.parameters['receiver'], False)}</%def>

	<%def name="fancybox_caption(plot)">
		Receiver bands: ${utils.commafy(plot.parameters['receiver'], False)} (spw ${plot.parameters['spw']})<br>
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br>
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
		Receiver bands: ${utils.commafy(plot.parameters['receiver'], False)}
	</%def>

	<%def name="caption_subtitle(plot)">
		${spws_for_baseband(plot)}
	</%def>

	<%def name="caption_text(plot, intent)">
		${intent.capitalize()} calibrator:
		${utils.commafy(utils.safe_split(plot.parameters['field']), quotes=False)}.
	</%def>

</%self:plot_group>

%endif
