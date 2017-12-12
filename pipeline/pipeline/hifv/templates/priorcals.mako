<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.mako"/>

<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>

<%block name="title">Prior calibrations</%block>

<p>Gain curves, opacities, antenna position corrections, requantizer gains, TEC maps, and switched power plots
using the CASA task <b>gencal</b>.</p>

	<h2>Gain Curves</h2>
	Gain curve table written to:
	
	%for single_result in result:
	    <p><b>${os.path.basename(single_result.gc_result.inputs['caltable'])}</b></p>
        %endfor


<%self:plot_group plot_dict="${opacity_plots}"
                                  url_fn="${lambda ms:  'noop'}">

        <%def name="title()">
            Opacities
        </%def>

        <%def name="preamble()">
                Opacities written to:
                
                %for single_result in result:
                        <p><b>${os.path.basename(single_result.oc_result.inputs['caltable'])}</b></p>
                %endfor
                
                <p>
                </p>
        </%def>
        
        
        <%def name="mouseover(plot)">Summary window        </%def>
        
        
        
        <%def name="fancybox_caption(plot)">
          Opacities
        </%def>
        
        
        <%def name="caption_title(plot)">
           Opacities
        </%def>
</%self:plot_group>



% for ms in center_frequencies:
    <table class="table table-bordered table-striped table-condensed"
	   summary="Summary of gencal opacities">
	<caption>Summary of gencal opacities</caption>
        <thead>
	    <tr>
	        <th scope="col" rowspan="2">SPW</th>
	        <th scope="col" rowspan="2">Frequency [GHz]</th>
		<th scope="col" rowspan="2">Opacity [Nepers]</th>
	    </tr>
	</thead>
	<tbody>
	% for i in range(len(center_frequencies[ms])):
		<tr>
		        <td>${spw[ms][i]}</td>
			<td>${center_frequencies[ms][i]/1.e9}</td>
			<td>${opacities[ms][i]}</td>
		</tr>
	% endfor
	</tbody>
    </table>
% endfor

        
<h2>Antenna positions</h2>
        
        %for single_result in result:
            % if single_result.antcorrect == {}:
                <b>No antenna position corrections to apply.</b>
            % else:
                Antenna position corrections written to:
                <p><b>${os.path.basename(single_result.antpos_result.final[0].gaintable)}</b></p>
                <table class="table table-bordered table-striped table-condensed"
	                   summary="Summary of gencal opacities">
	               <caption>Antenna Position Corrections</caption>
                       <thead>
	               <tr>
	                    <th scope="col" rowspan="2">Antenna</th>
	                    <th scope="col" rowspan="2">x</th>
		            <th scope="col" rowspan="2">y</th>
		            <th scope="col" rowspan="2">z</th>
	               </tr>
	               </thead>
	               <tbody>
	               % for key, value in sorted(single_result.antcorrect.iteritems()):
		           <tr>
		               <td>${key}</td>
			       <td>${value[0]}</td>
			       <td>${value[1]}</td>
			       <td>${value[2]}</td>
		           </tr>
	               % endfor
	              </tbody>
                      </table>
            % endif
        %endfor
        
	<h2>Requantizer gains</h2>
	Requantizer gains written to:
	
	%for single_result in result:
	    <p><b>${os.path.basename(single_result.rq_result.inputs['caltable'])}</b></p>
    %endfor


    %if single_result.tecmaps_result:

        <h2>TEC Maps</h2>
	    TEC Caltable written to:

	    %for single_result in result:
	        <p><b>${os.path.basename(single_result.tecmaps_result[0].inputs['caltable'])}</b></p>
        %endfor
        <br>
        TEC Images written to:
        %for single_result in result:
	        <p><b>${single_result.tecmaps_result[0].tec_image}</b></p>
	        <p><b>${single_result.tecmaps_result[0].tec_rms_image}</b></p>
        %endfor

        <img src="${tec_plotfile}">

    %endif

    %if swpowspgain_subpages:

        <h2>Switched Power plots</h2>
        Switched Power table written to:
        %for single_result in result:
	        <p><b>${os.path.basename(single_result.sw_result.inputs['caltable'])}</b></p>
        %endfor
        This table is NOT applied or added to the pipeline context callibrary.

        %for ms in summary_plots.keys():

            <h4>Switched Power Plots:
                <a class="replace" href="${os.path.relpath(os.path.join(dirname, swpowspgain_subpages[ms]), pcontext.report_dir)}">SwPower SPgain plots</a> |
                <a class="replace" href="${os.path.relpath(os.path.join(dirname, swpowtsys_subpages[ms]), pcontext.report_dir)}">SwPower Tsys plots</a>
            </h4>

        %endfor
    %endif