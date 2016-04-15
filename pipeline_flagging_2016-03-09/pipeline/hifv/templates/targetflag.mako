<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="title">Targetflag </%block>

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

<p>Run all calibrated data (including intent='*CALIBRATE*,*TARGET*') through rflag if specified.  If a file with continuum
regions is specified, then rflag will only flag those spw and frequency ranges per the pipeline spectral line heuristics.</p>

% for single_result in result:
    <p><b>Intents through rflag:</b>
    % if single_result.inputs['intents'] == '':
          All intents used
    % else:
          ${single_result.inputs['intents']}
    % endif
    </p>

    % if single_result.inputs['contfile'] != '':
        <p><b>Spectral Exclusion File (with intent='*TARGET*'): </b> ${single_result.inputs['contfile']} </p>
    % endif

% endfor







