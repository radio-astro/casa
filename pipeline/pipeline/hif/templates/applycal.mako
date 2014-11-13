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

%>
<%inherit file="t2-4m_details-base.html"/>

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
        	this.title = $(this.element).attr('caption');
       	}    
    }); 
	
	$('.caltable_filename').tooltip({
	    'selector': '',
	    'placement': 'left',
	    'container':'body'
	  });

    // fix the thumbnail margins for plots on the n>1 row
    UTILS.fixThumbnailMargins();
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

<%block name="title">Apply calibration tables</%block>

<p>This task applies all calibrations registered with the pipeline to their target measurement sets.<p>


<h2>Applied calibrations</h2>
<table class="table table-bordered table-striped table-condensed"
	   summary="Applied Calibrations">
	<caption>Applied Calibrations</caption>
	<thead>
		<tr>
			<th rowspan="2">Measurement Set</th>
			<th colspan="4">Target</th>
			<th colspan="6">Calibration</th>
		</tr>
		<tr>
			<th>Intent</th>
			<th>Field</th>
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
			<td rowspan="${ca_rowspan}">${space_comma(calapp.calto.intent)}</td>
			<td rowspan="${ca_rowspan}">${space_comma(calapp.calto.field)}</td>
			<td rowspan="${ca_rowspan}">${space_comma(calapp.calto.spw)}</td>
			<td rowspan="${ca_rowspan}">${space_comma(calapp.calto.antenna)}</td>
		% for calfrom in calapp.calfrom:
			<td>${caltypes[calfrom.gaintable]}</td>
			<td>${', '.join([str(i) for i in calfrom.spwmap])}</td>
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



<h2>Flagged data after calibration application</h2>
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
			<th>${ms}</th>
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


% if amp_vs_freq_plots or phase_vs_freq_plots or amp_vs_time_plots or amp_vs_uv_plots or phase_vs_uv_plots or phase_vs_time_plots:
<h2>Plots</h2>

<%self:plot_group plot_dict="${amp_vs_freq_plots}"
				  url_fn="${lambda x: 'amp_vs_freq_%s.html' % sanitise(x)}"
				  data_field="${True}"
				  data_spw="${True}"
				  plot_accessor="${lambda ms_plots: ms_plots.items()}">

	<%def name="title()">
		Calibrated amplitude vs frequency
	</%def>

	<%def name="mouseover(plot)">Click to show amplitude vs frequency for baseband ${plot.parameters['baseband']}</%def>

	<%def name="fancybox_caption(plot)">
		Baseband: ${plot.parameters['baseband']} (spw ${plot.parameters['spw']})<br />
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br />
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
		Baseband ${plot.parameters['baseband']}
	</%def>

	<%def name="caption_text(plot, intent)"> 
		Calibrated amplitude vs frequency for ${intent} calibrator 
		${utils.commafy(utils.safe_split(plot.parameters['field']), quotes=False)}, 
		baseband ${plot.parameters['baseband']}, all antennas and correlations, 
		coloured by antenna.
	</%def>

</%self:plot_group>


<%self:plot_group plot_dict="${phase_vs_freq_plots}"
				  url_fn="${lambda x: 'phase_vs_freq_%s.html' % sanitise(x)}"
				  data_field="${True}"
				  data_spw="${True}"
				  plot_accessor="${lambda ms_plots: ms_plots.items()}">

	<%def name="title()">
		Calibrated phase vs frequency
	</%def>

	<%def name="mouseover(plot)">Click to show phase vs frequency for baseband ${plot.parameters['baseband']}</%def>

	<%def name="fancybox_caption(plot)">
		Baseband: ${plot.parameters['baseband']} (spw ${plot.parameters['spw']})<br />
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br />
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
		Baseband ${plot.parameters['baseband']}
	</%def>

	<%def name="caption_text(plot, intent)">
		Calibrated phase vs frequency for ${intent} calibrator 
		${utils.commafy(utils.safe_split(plot.parameters['field']), quotes=False)}, 
		baseband ${plot.parameters['baseband']}, all antennas and correlations, 
		coloured by antenna.		
	</%def>

</%self:plot_group>


<%self:plot_group plot_dict="${amp_vs_uv_plots}"
				  url_fn="${lambda x: 'amp_vs_uv_%s.html' % sanitise(x)}"
				  data_spw="${True}">

	<%def name="title()">
		Calibrated amplitude vs UV distance
	</%def>

	<%def name="mouseover(plot)">Click to show amplitude vs UV distance for spectral window ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
		Spectral window: ${plot.parameters['spw']}<br />
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br />
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
		Spectral Window ${plot.parameters['spw']}
	</%def>

	<%def name="caption_text(plot, intent)"> 
		Calibrated amplitude vs UV distance for amplitude calibrator ${plot.parameters['field']}
		spw ${plot.parameters['spw']}, all antennas and correlations. Data are coloured by correlation.
	</%def>

</%self:plot_group>


<%self:plot_group plot_dict="${amp_vs_time_plots}"
				  url_fn="${lambda x: 'amp_vs_time_%s.html' % sanitise(x)}"
				  data_spw="${True}">

	<%def name="title()">
		Calibrated amplitude vs time
	</%def>

	<%def name="mouseover(plot)">Click to show amplitude vs time for spectral window ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
		Spectral window: ${plot.parameters['spw']}<br />
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br />
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
		Spectral Window ${plot.parameters['spw']}
	</%def>

	<%def name="caption_text(plot, intent)"> 
		Calibrated amplitude vs time for all fields, spw ${plot.parameters['spw']}, 
		all antennas and correlations. Data are coloured by field.
	</%def>

</%self:plot_group>


<%self:plot_group plot_dict="${phase_vs_time_plots}"
				  url_fn="${lambda x: 'phase_vs_time_%s.html' % sanitise(x)}"
				  data_spw="${True}">

	<%def name="title()">
		Calibrated phase vs time
	</%def>

	<%def name="mouseover(plot)">Click to show phase vs time for spectral window ${plot.parameters['spw']}</%def>

	<%def name="fancybox_caption(plot)">
		Spectral window: ${plot.parameters['spw']}<br />
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br />
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
		Spectral Window ${plot.parameters['spw']}
	</%def>

	<%def name="caption_text(plot, intent)"> 
		Calibrated phase vs time for all fields, spw ${plot.parameters['spw']},
		all antennas and correlations. Data are coloured by field.
	</%def>

</%self:plot_group>

<%self:plot_group plot_dict="${science_amp_vs_freq_plots}"
				  url_fn="${lambda x: 'science_amp_vs_freq_%s.html' % sanitise(x)}"
				  data_spw="${True}"
				  data_field="${True}"
				  plot_accessor="${lambda ms_plots: ms_plots.items()}">

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

	<%def name="mouseover(plot)">Click to show amplitude vs frequency for baseband ${plot.parameters['baseband']}</%def>

	<%def name="fancybox_caption(plot)">
		Baseband: ${plot.parameters['baseband']} (spw ${plot.parameters['spw']})<br />
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br />
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
		Baseband ${plot.parameters['baseband']}
	</%def>

	<%def name="caption_text(plot, source_id)">
		Calibrated amplitude vs frequency for Source #${source_id}
		${utils.commafy(utils.safe_split(plot.parameters['field']), quotes=False)}, 
		baseband ${plot.parameters['baseband']}.
	</%def>

</%self:plot_group>

<%self:plot_group plot_dict="${science_phase_vs_freq_plots}"
				  url_fn="${lambda x: 'science_phase_vs_freq_%s.html' % sanitise(x)}"
				  data_baseband="${True}"
				  data_field="${True}"
				  plot_accessor="${lambda ms_plots: ms_plots.items()}">

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

	<%def name="mouseover(plot)">Click to show phase vs frequency for baseband ${plot.parameters['baseband']}</%def>

	<%def name="fancybox_caption(plot)">
		Baseband: ${plot.parameters['baseband']} (spw ${plot.parameters['spw']})<br />
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br />
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
		Baseband ${plot.parameters['baseband']}
	</%def>

	<%def name="caption_text(plot, source_id)">
		Calibrated phase vs frequency for Source #${source_id}
		${utils.commafy(utils.safe_split(plot.parameters['field']), quotes=False)}, 
		baseband ${plot.parameters['baseband']}.
	</%def>

</%self:plot_group>

<%self:plot_group plot_dict="${science_amp_vs_uv_plots}"
				  url_fn="${lambda x: 'science_amp_vs_uv_%s.html' % sanitise(x)}"
				  data_baseband="${True}"
				  data_field="${True}"
				  plot_accessor="${lambda ms_plots: ms_plots.items()}">

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

	<%def name="mouseover(plot)">Click to show amplitude vs UV distance for baseband ${plot.parameters['baseband']}</%def>

	<%def name="fancybox_caption(plot)">
		Baseband: ${plot.parameters['baseband']} (spw ${plot.parameters['spw']})<br />
		Intents: ${utils.commafy(plot.parameters['intent'], False)}<br />
		Fields: ${cgi.escape(plot.parameters['field'], True)}
	</%def>

	<%def name="caption_title(plot)">
		Baseband ${plot.parameters['baseband']}
	</%def>

	<%def name="caption_text(plot, source_id)">
		Calibrated amplitude vs UV distance for Source #${source_id}
		${utils.commafy(utils.safe_split(plot.parameters['field']), quotes=False)}, 
		baseband ${plot.parameters['baseband']}.
	</%def>

</%self:plot_group>

%endif
