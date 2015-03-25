<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="title">Flux density bootstrapping and spectral index fitting</%block>

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

<p>Make a gain table that includes gain and opacity corrections for final amp cal and for flux density bootstrapping.</p>
<p>Fit the spectral index of calibrators with a power-law and put the fit in the model column.</p>

<%self:plot_group plot_dict="${summary_plots}"
                                  url_fn="${lambda ms:  'noop'}">

        <%def name="title()">
            Fluxboot summary plot
        </%def>

        <%def name="preamble()">


        </%def>
        
        
        <%def name="mouseover(plot)">Summary window </%def>
        
        
        
        <%def name="fancybox_caption(plot)">
          Model calibrator flux densities. Plot of amp vs. freq
        </%def>
        
        
        <%def name="caption_title(plot)">
           Model calibrator
        </%def>
</%self:plot_group>
    



% for ms in summary_plots:
    
<table class="table table-bordered table-striped table-condensed"
	   summary="Spectral Indices">
	<caption>Spectral Indices</caption>
        <thead>
	    <tr>
	        <th scope="col" rowspan="2">Source</th>
	        <th scope="col" rowspan="2">Band</th>
		<th scope="col" rowspan="2">Fitted Spectral Index</th>
		<th scope="col" rowspan="2">SNR</th>
	    </tr>

	</thead>
	<tbody>    
    
    % for row in spindex_results[ms]:
    

		<tr>
		        <td>${row['source']}</td>
			<td>${row['band']}</td>
			<td>${row['spix']}</td>
			<td>${row['SNR']}</td>
		</tr>

    % endfor
	</tbody>
    </table>

    
    
       <table class="table table-bordered table-striped table-condensed"
	   summary="Fitting data with a power law">
	<caption>Fitting data with a power law</caption>
        <thead>
	    <tr>
	        <th scope="col" rowspan="2">Frequency [GHz]</th>
	        <th scope="col" rowspan="2">Data</th>
		<th scope="col" rowspan="2">Error</th>
		<th scope="col" rowspan="2">Fitted Data</th>
	    </tr>

	</thead>
	<tbody>   
  
    % for row in sorted(weblog_results[ms], key=lambda p: p['freq']):
    

	
		<tr>
		        <td>${row['freq']}</td>
			<td>${row['data']}</td>
			<td>${row['error']}</td>
			<td>${row['fitteddata']}</td>
		</tr>

    % endfor
	</tbody>
    </table>
    

%endfor