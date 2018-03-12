<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>

<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">Circular Feed Polarization Calibration</%block>

<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>

<ul>
%for single_result in result:

    %if single_result.final:
        Polarization Cal Tables written to disk and added to the pipeline callibrary:
        %for calapp in single_result.final:
            %for calfrom in calapp.calfrom:
                <li>${os.path.basename(calfrom.gaintable)}</li>
            %endfor
        %endfor
    %else:
        No polarization intents present - polarization calibration not performed.
    %endif

%endfor
</ul>


<ul>
%for single_result in result:

    %if single_result.final:
        ${single_result.calstrategy}
        <li>Using standard calibrator ${single_result.caldictionary['fluxcal']}:
            Field name = ${single_result.caldictionary['fluxcalfieldname']},
            Field ID = ${str(single_result.caldictionary['fluxcalfieldid'])}</li>
        <li>POLANGLE Field = ${single_result.caldictionary['polanglefield']}</li>
        <li>POLLEAKAGE Field = ${single_result.caldictionary['polleakagefield']}</li>
    %endif

%endfor
</ul>




<hr>

%for single_result in result:

    %if single_result.final:

    <%self:plot_group plot_dict="${polarization_plotcal_plots}"
                                  url_fn="${lambda ms:  'noop'}">

        <%def name="title()">
            Polarization Plotcal Plots
        </%def>

        <%def name="preamble()">
           Plots resulting from polarization calibration.
        </%def>


        <%def name="mouseover(plot)">Summary window </%def>



        <%def name="fancybox_caption(plot)">
          ${plot.parameters['caption']}
        </%def>


        <%def name="caption_title(plot)">
           ${plot.parameters['caption']}
        </%def>

    </%self:plot_group>

    %else:
        <br>
    %endif

%endfor




%if ampfreq_subpages:

        <h2>D-term Amplitude vs. Frequency Plots</h2>
        Cal table used:
        %for single_result in result:
	        <p><b>${os.path.basename(single_result.final[1].gaintable)}</b></p>
        %endfor

        %for ms in polarization_plotcal_plots.keys():

            <h4>
            <a class="replace" href="${os.path.relpath(os.path.join(dirname, ampfreq_subpages[ms]), pcontext.report_dir)}">Amplitude vs. Frequency (per antenna plots)</a>
            </h4>

        %endfor
%endif





