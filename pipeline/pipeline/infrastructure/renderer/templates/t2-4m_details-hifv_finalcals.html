<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="title">Final calibration tables</%block>

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

<p>Make the final calibration tables.</p>

% for ms in summary_plots:
    <h4>Plots: <br> <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, finaldelay_subpages[ms]), pcontext.report_dir)}">Final delay plots</a> |
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, phasegain_subpages[ms]), pcontext.report_dir)}">Gain phase </a> |
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, bpsolamp_subpages[ms]), pcontext.report_dir)}">BP Amp solution </a> |
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, bpsolphase_subpages[ms]), pcontext.report_dir)}">BP Phase solution </a> |
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, bpsolphaseshort_subpages[ms]), pcontext.report_dir)}">BP Phase (short) solution </a> |
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, finalamptimecal_subpages[ms]), pcontext.report_dir)}">Final amp time cal </a> |
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, finalampfreqcal_subpages[ms]), pcontext.report_dir)}">Final amp freq cal </a> |
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, finalphasegaincal_subpages[ms]), pcontext.report_dir)}">Final phase gain cal </a> 
        
    </h4>
    

%endfor