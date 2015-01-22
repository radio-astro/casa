<%!
rsc_path = ""

import os
%>
<%inherit file="t2-4m_details-base.html"/>
<%block name="header" />

<%block name="title">Flag T<sub>sys</sub> channels</%block>

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
 
% if htmlreports:
<h2>Flags</h2>
<table class="table table-bordered table-striped">
        <caption>Report Files</caption>
        <thead>
            <tr>
                <th>Flagging Commands</th>
                <th>Flagging Report</th>
            </tr>
        </thead>
        <tbody>
            % for file,reports in htmlreports.items():
            <tr>
                <td><a class="replace-pre"
                       href="${reports[0]}">${file}</a></td>
                <td><a class="replace-pre"
                       href="${reports[1]}">printTsysFlags</a></td>
            </tr>
            % endfor
        </tbody>
</table>
% endif
 
% if plot_groups:
<h2>Plots</h2>
<ul>
    % for plot_group in plot_groups:
        <li>
               <a class="replace" href="${os.path.relpath(os.path.join(dirname, plot_group.filename), pcontext.report_dir)}">${plot_group.title}</a>
               shows the images used for flagging.
    % endfor
</ul>

<h3>T<sub>sys</sub> after flagging</h3>

% for ms in summary_plots:
    <h4><a class="replace"
           href="${os.path.relpath(os.path.join(dirname, summary_subpage[ms]), pcontext.report_dir)}">${ms}</a>
    </h4>
    <ul class="thumbnails">
        % for plot in summary_plots[ms]:
            % if os.path.exists(plot.thumbnail):
            <li class="span3">
                <div class="thumbnail">
                    <a href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
                       class="fancybox"
                       rel="tsys-summary-${ms}">
                        <img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
                             title="T<sub>sys</sub> summary for Spectral Window ${plot.parameters['spw']}"
                             data-thumbnail="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}">
                        </img>
                    </a>

                    <div class="caption">
                    	<h4>
							<a href="${os.path.relpath(os.path.join(dirname, summary_subpage[ms]), pcontext.report_dir)}"
	                       	   class="replace"
	                           data-spw="${plot.parameters['spw']}">
	                           Spectral Window ${plot.parameters['spw']}
	                        </a>
                        </h4>

                        <p>Plot of time-averaged T<sub>sys</sub> for spectral
                            window ${plot.parameters['spw']} (T<sub>sys</sub>
                            window
                        ${plot.parameters['tsys_spw']}), coloured by antenna.
                        </p>
                    </div>
                </div>
            </li>
            % endif
        % endfor
    </ul>
%endfor

% endif
