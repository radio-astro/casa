<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">Semi-final delay and bandpass calibrations</%block>

<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>

<p>Perform semi-final delay and bandpass calibrations, as the spectral index
of the bandpass calibrator has not yet been determined.</p>

% for ms in summary_plots:
    <h4>Plots:  <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, delay_subpages[ms]), pcontext.report_dir)}">Delay plots </a>|
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, phasegain_subpages[ms]), pcontext.report_dir)}">Gain phase </a>|
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, bpsolamp_subpages[ms]), pcontext.report_dir)}">BP Amp solution </a>|
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, bpsolphase_subpages[ms]), pcontext.report_dir)}">BP Phase solution </a>
    </h4>
%endfor



<br>



<%self:plot_group plot_dict="${summary_plots}"
                                  url_fn="${lambda ms:  'noop'}">

        <%def name="title()">
            semiFinalBPdcals summary plot
        </%def>

        <%def name="preamble()">


        </%def>
        
        
        <%def name="mouseover(plot)">Summary window </%def>
        
        
        
        <%def name="fancybox_caption(plot)">
          Semi-final calibrated bandpass
        </%def>
        
        
        <%def name="caption_title(plot)">
           Semi-final calibrated bandpass
        </%def>
</%self:plot_group>
