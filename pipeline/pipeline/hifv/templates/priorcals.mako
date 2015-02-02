<%!
rsc_path = ""
%>
<%inherit file="t2-4m_details-base.html"/>

<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>

<%block name="title">Prior calibrations</%block>

<p>Gain curves, opacities, antenna position corrections, and requantizer gains
using the CASA task <b>gencal</b>.</p>

	<h2>Gain Curves</h2>
	Gain curve table written to:
	
	%for single_result in result:
	    <p><b>${single_result.gc_result[0].inputs['caltable']}</b></p>
        %endfor


<%self:plot_group plot_dict="${opacity_plots}"
                                  url_fn="${lambda ms:  'noop'}">

        <%def name="title()">
            Opacities
        </%def>

        <%def name="preamble()">
                Opacities written to:
                
                %for single_result in result:
                        <p><b>${single_result.oc_result[0].inputs['caltable']}</b></p>
                %endfor
                
                <p>Some description of the opacity plots goes here.
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
        
	<h2>Requantizer gains</h2>
	Requantizer gains written to:
	
	%for single_result in result:
	    <p><b>${single_result.rq_result[0].inputs['caltable']}</b></p>
        %endfor
