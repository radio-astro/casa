<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="title">Flag bad deformatters</%block>

<p>Identify and flag basebands with bad deformatters or RFI based on bandpass (BP) table amps and phases.</p>

       
        % for single_result in result:
            <h3>BP Table Amps</h3>
            <table class="table table-bordered table-striped table-condensed"
	       summary="Deformatter Flagging Amp">
	    <caption></caption>
	    <thead>
		<tr>
			<th>Antenna</th>
			<th>SPWs</th>
		</tr>
	    </thead>
	    <tbody>
	    
	    % if single_result.result_amp == []:
	        <tr>
	        <td>None</td>
	        <td>None</td>
	        </tr>
	    % else:
	        % for key, listvalues in single_result.amp_collection.iteritems():
	            <tr>
	            <td>${key}</td>
	            <td>${','.join(listvalues)}</td>
	            </tr>
	        % endfor
	    % endif
	        
	    </tbody>
            </table>
            <br>
            
            <h3>BP Table Phases</h3>
            <table class="table table-bordered table-striped table-condensed"
	       summary="Deformatter Flagging Phase">
	    <caption></caption>
	    <thead>
		<tr>
			<th>Antenna</th>
			<th>SPWs</th>
		</tr>
	    </thead>
	    <tbody>
	    
	    % if single_result.result_phase == []:
	        <tr>
	        <td>None</td>
	        <td>None</td>
	        </tr>
	    % else:
	        % for key, listvalues in single_result.phase_collection.iteritems():
	            <tr>
	            <td>${key}</td>
	            <td>${','.join(listvalues)}</td>
	            </tr>
	        % endfor
	    % endif
	        
	    </tbody>
            </table>
            
        % endfor
	
	