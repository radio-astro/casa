<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="title">Solution Interval and test gain calibrations</%block>

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

<p>Determine the solution interval for a scan-average equivalent and do test gain calibrations to establish a short solution interval.</p>

% for ms in summary_plots:

    <ul>
        <li>The long solution interval is: <b> ${longsolint[ms]}s</b>.</li>
        <li>The short solution interval used is: <b>${new_gain_solint1[ms]}</b>.</li>
    </ul>

    <h4>Plots:  <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, testgainsamp_subpages[ms]), pcontext.report_dir)}">Testgains amp plots</a>|
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, testgainsphase_subpages[ms]), pcontext.report_dir)}">Testgains phase plots</a>
    </h4>
    


%endfor