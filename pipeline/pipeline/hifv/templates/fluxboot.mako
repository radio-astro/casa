<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">Flux density bootstrapping and spectral index fitting</%block>

<p>Make a gain table that includes gain and opacity corrections for final amp cal and for flux density bootstrapping.</p>
<p>Fit the spectral index of calibrators with a power-law and put the fit in the model column.</p>

<%self:plot_group plot_dict="${summary_plots}"
                                  url_fn="${lambda ms:  'noop'}">

        <%def name="title()">
            Fluxboot summary plots
        </%def>

        <%def name="preamble()">


        </%def>
        
        
        <%def name="mouseover(plot)">Summary window </%def>
        
        
        
        <%def name="fancybox_caption(plot)">
          ${plot.parameters['figurecaption']}. Plot of amp vs. freq
        </%def>
        
        
        <%def name="caption_title(plot)">
           ${plot.parameters['figurecaption']}
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
		    <th scope="col" rowspan="2">Fitted Curvature</th>
		    <th scope="col" rowspan="2">Fit Order</th>
	    </tr>

	</thead>
	<tbody>    
    
    % for row in spindex_results[ms]:
    

		<tr>
		    <td>${row['source']}</td>
			<td>${row['band']}</td>
			<td>${'{0:.4f}'.format(float(row['spix']))}  +/-  ${'{0:.4f}'.format(float(row['spixerr']))}</td>
			<td>${'{0:.4f}'.format(float(row['curvature']))}  +/-  ${'{0:.4f}'.format(float(row['curvatureerr']))}</td>
            <td>${row['fitorder']}</td>
		</tr>

    % endfor
	</tbody>
    </table>


       <table class="table table-bordered table-striped table-condensed"
	   summary="Fitting data with a power law">
	<caption>Fitting data with a power law</caption>
        <thead>
	    <tr>
	        <th scope="col" rowspan="2">Source</th>
	        <th scope="col" rowspan="2">Frequency [GHz]</th>
	        <th scope="col" rowspan="2">Data</th>
		    <th scope="col" rowspan="2">Error</th>
		    <th scope="col" rowspan="2">Fitted Data</th>
		    <th scope="col" rowspan="2">Residual: Data-Fitted Data</th>
	    </tr>

	</thead>
	<tbody>   
  
     % for sourcekey in sorted(weblog_results[ms].keys()):
        <tr>
		    <td rowspan="${len(weblog_results[ms][sourcekey])}">${sourcekey}</td>
                % for row in sorted(weblog_results[ms][sourcekey], key=lambda p: float(p['freq'])):

		        <td>${row['freq']}</td>
			    <td>${'{0:.4f}'.format(float(row['data']))}</td>
			    <td>${'{0:.6f}'.format(float(row['error']))}</td>
			    <td>${'{0:.4f}'.format(float(row['fitteddata']))}</td>
			    <td>${'{0:.6f}'.format(float(row['data']) - float(row['fitteddata']))}</td>
		</tr>
                % endfor
    % endfor
	</tbody>
    </table>
    

%endfor