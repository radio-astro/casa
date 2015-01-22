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


%>
<%inherit file="t2-4m_details-base.html"/>

<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>
<script>
$(document).ready(function(){
    // return a function that sets the SPW text field to the given spw
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
		if (typeof field === 'undefined') {
	        o.data("callback", createSelectSetter(spw, "#select-spw"));
		} else {
	        o.data("callback", [createSelectSetter(spw, "#select-spw"),
	                            createSelectSetter(field, "#select-field")]);
		}
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
        }
    });
	
    // fix the thumbnail margins for plots on the n>1 row
    UTILS.fixThumbnailMargins();
});
</script>



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
				  plot_accessor="${lambda ms_plots: ms_plots.items()}">
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
		Baseband: ${plot.parameters['baseband']}		
	</%def>

	<%def name="caption_title(plot)">
		Baseband ${plot.parameters['baseband']}
	</%def>

	<%def name="caption_text(plot, source_id)">
		Model amplitude vs UV distance in baseband
		${plot.parameters['baseband']} for ${plot.parameters['intent']}
		calibrator.
	</%def>



</%self:plot_group>

