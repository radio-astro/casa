<%!
rsc_path = ""
import os
%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="header" />

<%block name="title">Single-Dish Sky Calibration</%block>

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

<p>This task generates a sky calibration table, a collection of OFF spectra for single dish data calibration.</p>

<h2>Results</h2>
<table class="table table-bordered" summary="Application Results">
	<caption>Applied calibrations and parameters used for caltable generation</caption>
    <thead>
        <tr>
            <th scope="col" rowspan="2">Measurement Set</th>
			<th scope="col" colspan="1">Solution Parameters</th>
			<th scope="col" colspan="4">Applied To</th>
            <th scope="col" rowspan="2">Calibration Table</th>
		</tr>
		<tr>
			<th>Type</th>
			<th>Field</th>
			<th>Scan Intent</th>
			<th>Antennas</th>
			<th>Spectral Windows</th>
        </tr>
    </thead>
	<tbody>
% for application in applications:
		<tr>
			<td>${application['ms']}</td>
		  	<td>${application['caltype']}</td>
		  	<td>${application['field']}</td>
		  	<td>${application['intent']}</td>
		  	<td>${application['antenna']}</td>
		  	<td>${application['spw']}</td>
		  	<td>${application['gaintable']}</td>
		</tr>
% endfor		
	</tbody>
</table>

