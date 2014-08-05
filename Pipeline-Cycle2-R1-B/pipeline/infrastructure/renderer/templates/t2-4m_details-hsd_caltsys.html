<%!
rsc_path = "../"
import os

import itertools
import pipeline.infrastructure.renderer.htmlrenderer as hr
import pipeline.infrastructure.renderer.logger as logger
import pipeline.infrastructure.displays as displays
%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="header" />

<%block name="title">${hr._get_task_description_for_class(result[0].task)}</%block>

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

    // return a function that sets the ANT text field to the given antenna
    var createAntennaSetter = function(ant) {
        return function() {
            // trigger a change event, otherwise the filters are not changed
            $("#select-ant").select2("val", [ant]).trigger("change");
        };
    };

    // 
    var createMixedSetter = function(spw, ant) {
        return function() {
            // trigger a change event, otherwise the filters are not changed
            $("#select-spw").select2("val", [spw]).trigger("change");
            $("#select-ant").select2("val", [ant]).trigger("change");
        };
    };    

    // create a callback function for each overview plot that will select the
    // appropriate spw once the page has loaded
    $(".thumbnail a").each(function (i, v) {
        var o = $(v);
        var spw = o.data("spw");
        var ant = o.data("ant");
        o.data("callback", createMixedSetter(spw, ant));
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

<p>This task generates a Tsys calibration table.</p>

<h3>Tsys vs Frequency</h3>
% for ms in summary.keys():
    <h4><a class="replace"
           href="${os.path.relpath(os.path.join(dirname, subpage[ms]), pcontext.report_dir)}">${ms}</a>
    </h4>
    
    <ul class="thumbnails">
        % for _summary in summary[ms]:
            % if os.path.exists(_summary.thumbnail):
                <li class="span3">
                    <div class="thumbnail">
                        <a href="${os.path.relpath(_summary.abspath, pcontext.report_dir)}"
                           class="fancybox"
                           rel="thumbs">
                            <img src="${os.path.relpath(_summary.thumbnail, pcontext.report_dir)}"
                                 title="Tsys vs Frequency"
                                 data-thumbnail="${os.path.relpath(_summary.thumbnail, pcontext.report_dir)}">
                            </img>
                        </a>

                        <div class="caption">
                            <h4>
                                <a href="${os.path.relpath(os.path.join(dirname, subpage[ms]), pcontext.report_dir)}"
                                   class="replace"
				   data-ant="${_summary.parameters['ant']}">
                                   Antenna ${_summary.parameters['ant']}
                                </a>
                            </h4>
                        </div>
                    </div>
                </li>
            % endif
        % endfor
    </ul>
%endfor

