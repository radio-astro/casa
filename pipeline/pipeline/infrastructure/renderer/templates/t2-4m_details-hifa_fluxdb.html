<%!
rsc_path = ""
import cgi
import decimal
import os
import string
import types

import pipeline.domain.measures as measures
import pipeline.infrastructure.renderer.htmlrenderer as hr
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
$(document).ready(function() {
    // return a function that sets the SPW text field to the given spw
    var createSpwSetter = function(spw) {
        return function() {
            // trigger a change event, otherwise the filters are not changed
            $("#select-spw").select2("val", [spw]).trigger("change");
        };
    };

    // create a callback function for each overview plot that will select the
    // appropriate spw once the page has loaded
    $(".thumbnail a").each(function (i, v) {
        var o = $(v);
        var spw = o.data("spw");
        o.data("callback", createSpwSetter(spw));
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
});
</script>


<%block name="title">Flux calibration database service</%block>




<h2>Results</h2>


<p>Flux calibration database connection</p>

<p><font color="red">Red columns</font> are from the ms and are used as inputs.</p>
<p><b>Black values</b> are from the DB flux service response.</p>
<p><font color="blue">Blue values</font> are from Todd Hunter's analysisUtils (au).</p>


<table class="table table-bordered table-striped" summary="Flux calibration database">
	<caption>Results from database request</caption>
        <thead>
	    <tr>
	        <th scope="col"><font color="red">Field ID</font></th>
	        <th scope="col"><font color="red">Field Name</font></th>
	        <th scope="col"><font color="red">spw</font></th>
	        <th scope="col"><font color="red">Centre Frequency (Hz)</font></th>
	        <th scope="col"><font color="red">Field Flux I</font></th>
	        <th scope="col">DB Link</th>
	        <th scope="col">DB Source Name</th>
	        <th scope="col">DB/<font color="blue">au</font> Frequency</th>
	        <th scope="col">DB Date</th>
	        <th scope="col">DB/<font color="blue">au</font> Flux Density</th>
	        <th scope="col">DB/<font color="blue">au</font> Flux Density Error/<font color="blue">Uncertainty</font></th>
	        <th scope="col">DB/<font color="blue">au</font> Spectral Index</th>
	        <th scope="col">DB/<font color="blue">au</font> Spectral Index Error/<font color="blue">Uncertainty</font></th>
	        <th scope="col"><font color="blue">au</font> mean Age</th>
	        <th scope="col">DB Error2</th>
	        <th scope="col">DB Error3</th>
	        <th scope="col">DB Error4</th>
	        <th scope="col">DB Warning</th>
	        <th scope="col">DB Notms</th>
	        <th scope="col">DB Verbose</th>
	        
	    </tr>
	</thead>
	<tbody>
%for single_result in result:
    %for row in single_result.fluxtable:
      <tr>
          <td>${row['fieldid']}</td>
          <td>${row['fieldname']}</td>
          <td>${row['spw']}</td>
          <td>${row['frequency']}</td>
          <td>${row['fieldfluxI']}</td>
          <td><a href="${row['url']}">URL</a></td>
          <td>${row['sourcename']}</td>
          <td>${row['dbfrequency']}<br><font color="blue">${row['aufrequency']}</font></td>
          <td>${row['date']}</td>
          <td>${row['fluxdensity']}<br><font color="blue">${row['aufluxDensity']}</font></td>
          <td>${row['fluxdensityerror']}<br><font color="blue">${row['aufluxDensityUncertainty']}</font></td>
          <td>${row['spectralindex']}<br><font color="blue">${row['auspectralIndex']}</font></td>
          <td>${row['spectralindexerror']}<br><font color="blue">${row['auspectralIndexUncertainty']}</font></td>
          <td><font color="blue">${row['aumeanAge']}</font></td>
          <td>${row['error2']}</td>
          <td>${row['error3']}</td>
          <td>${row['error4']}</td>
          <td>${row['warning']}</td>
          <td>${row['notms']}</td>
          <td>${row['verbose']}</td>
      </tr>
    %endfor
%endfor
	</tbody>
</table>






