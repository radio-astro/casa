<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">Final calibration tables</%block>

<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>

<p>Make the final calibration tables.</p>

% for ms in summary_plots:
    <h4>Plots: <br> <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, finaldelay_subpages[ms]), pcontext.report_dir)}">Final delay plots </a> |
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, phasegain_subpages[ms]), pcontext.report_dir)}">BP initial gain phase </a> |
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, bpsolamp_subpages[ms]), pcontext.report_dir)}">BP Amp solution </a> |
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, bpsolphase_subpages[ms]), pcontext.report_dir)}">BP Phase solution </a> |
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, bpsolphaseshort_subpages[ms]), pcontext.report_dir)}">Phase (short) gain solution</a> |
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, finalamptimecal_subpages[ms]), pcontext.report_dir)}">Final amp time cal </a> |
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, finalampfreqcal_subpages[ms]), pcontext.report_dir)}">Final amp freq cal </a> |
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, finalphasegaincal_subpages[ms]), pcontext.report_dir)}">Final phase gain cal </a> 
        
    </h4>
    

%endfor