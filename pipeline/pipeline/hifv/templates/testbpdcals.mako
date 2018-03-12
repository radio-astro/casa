<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">Initial test calibrations</%block>

<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>

<p>Initial test calibrations using bandpass and delay calibrators</p>

% for ms in summary_plots:
    <h4>Plots:  <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, testdelay_subpages[ms]), pcontext.report_dir)}">Test delay plots </a>|
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, ampgain_subpages[ms]), pcontext.report_dir)}">Gain Amplitude </a>|
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, phasegain_subpages[ms]), pcontext.report_dir)}">Gain Phase </a>|
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, bpsolamp_subpages[ms]), pcontext.report_dir)}">BP Amp solution </a>|
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, bpsolphase_subpages[ms]), pcontext.report_dir)}">BP Phase solution </a>
    </h4>
%endfor


<%self:plot_group plot_dict="${summary_plots}"
                                  url_fn="${lambda ms:  'noop'}">

        <%def name="title()">
            testBPdcals summary plot
        </%def>

        <%def name="preamble()">


        </%def>
        
        
        <%def name="mouseover(plot)">Summary window </%def>
        
        
        
        <%def name="fancybox_caption(plot)">
          Initial calibrated bandpass
        </%def>
        
        
        <%def name="caption_title(plot)">
           Initial calibrated bandpass
        </%def>
</%self:plot_group>




