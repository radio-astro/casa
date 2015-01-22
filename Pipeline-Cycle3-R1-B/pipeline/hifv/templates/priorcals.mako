<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.html"/>

<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>


<%block name="title">Prior calibrations</%block>

<p>Gain curves, opacities, antenna position corrections, and requantizer gains
using the CASA task <b>gencal</b>.</p>

<div class="row-fluid">
	<h2>Gain Curves</h2>
	Gain curve table written to:
	
	%for single_result in result:
	    <p><b>${single_result.gc_result[0].inputs['caltable']}</b></p>
        %endfor
</div>

<div class="row-fluid">
	<h2>Opacities</h2>
	Opacities written to:
	
	%for single_result in result:
	    <p><b>${single_result.oc_result[0].inputs['caltable']}</b></p>
        %endfor
        
% for ms in opacity_plots:
    
    <ul class="thumbnails">
        % for plot in opacity_plots[ms]:
            % if os.path.exists(plot.thumbnail):
            <li class="span3">
                <div class="thumbnail">
                    <a href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
                       class="fancybox"
                       rel="${ms}.plotweather.png">
                        <img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
                             title="testBPdcals summary for Spectral Window ${plot.parameters['spw']}"
                             data-thumbnail="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}">
                        </img>
                    </a>

                    <div class="caption">
                    	<h4>Opacities
                        </h4>
                    </div>
                </div>
            </li>
            % endif
        % endfor
    </ul>

%endfor


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

        
</div>

<div class="row-fluid">
	<h2>Antenna positions</h2>
	Antenna position corrections written to:
	
	%for single_result in result:
	    <p><b>${single_result.antpos_result[0].inputs['caltable']}</b></p>
        %endfor
        
        %for single_result in result:
            % if single_result.antpos_result[0].inputs['offsets'] == []:
                <b>No antenna position corrections to apply.</b>
            % else:
                <b>${single_result.antpos_result[0].inputs['offsets']}</b>
            % endif
        %endfor
        
</div>

<div class="row-fluid">
	<h2>Requantizer gains</h2>
	Requantizer gains written to:
	
	%for single_result in result:
	    <p><b>${single_result.rq_result[0].inputs['caltable']}</b></p>
        %endfor
</div>