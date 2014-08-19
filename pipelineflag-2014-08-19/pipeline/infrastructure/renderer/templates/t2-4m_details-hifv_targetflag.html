<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="title">Targetflag (all data through rflag)</%block>

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

<p>Run all calibrated data (including target) through rflag.</p>

% for ms in summary_plots:
    
    <ul class="thumbnails">
        % for plot in summary_plots[ms]:
            % if os.path.exists(plot.thumbnail):
            <li class="span3">
                <div class="thumbnail">
                    <a href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
                       class="fancybox"
                       rel="targetflag-summary-${ms}">
                        <img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
                             title="Model calibrator flux densities"
                             data-thumbnail="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}">
                        </img>
                    </a>

                    <div class="caption">
                    	<h4>${plot.parameters['type']}
                        </h4>

                        <p>Plot of ${plot.y_axis} vs. ${plot.x_axis}
                        </p>
                    </div>
                </div>
            </li>
            % endif
        % endfor
    </ul>

%endfor