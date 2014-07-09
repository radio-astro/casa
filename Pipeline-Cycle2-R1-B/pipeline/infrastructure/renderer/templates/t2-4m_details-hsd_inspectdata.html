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

    // return a function that sets the Antenna text field to the given spw
    var createAntennaSetter = function(ant) {
        return function() {
            // trigger a change event, otherwise the filters are not changed
            $("#select-ant").select2("val", [ant]).trigger("change");
        };
    };

    // return a function that sets the Polarization text field to the given spw
    var createPolarizationSetter = function(pol) {
        return function() {
            // trigger a change event, otherwise the filters are not changed
            $("#select-pol").select2("val", [pol]).trigger("change");
        };
    };

    // 
    var createMixedSetter = function(spw, ant, pol) {
        return function() {
            // trigger a change event, otherwise the filters are not changed
            $("#select-spw").select2("val", [spw]).trigger("change");
            $("#select-ant").select2("val", [ant]).trigger("change");
            $("#select-pol").select2("val", [pol]).trigger("change");
        };
    };    

    // create a callback function for each overview plot that will select the
    // appropriate spw once the page has loaded
    $(".thumbnail a").each(function (i, v) {
        var o = $(v);
        var spw = o.data("spw");
        var ant = o.data("ant");
        var pol = o.data("pol");
        o.data("callback", createMixedSetter(spw, ant, pol));
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

<%
plot_types = ['pointing', 'azel', 'weather', 'wvr']
plot_titles = {'pointing': 'Telescope Pointing', 
               'azel': 'Az/El Plots', 
               'weather': 'Weather Plots', 
               'wvr': 'WVR Plots'}
plot_desc = {'pointing': 'Telescope pointing on the sky', 
             'azel': 'Azimuth/Elevation vs Time', 
             'weather': 'Weather vs Time', 
             'wvr': 'WVR Reading vs Time'}
             
def get_spwmap(tsys_strategy):
    spwmap = {}
    for l in tsys_strategy:
        if spwmap.has_key(l[0]):
            spwmap[l[0]].append(l[1])
        else:
            spwmap[l[0]] = [l[1]]
    return spwmap
    
spwmap = {}
for st in pcontext.observing_run:
    spwmap[st.basename] = get_spwmap(st.calibration_strategy['tsys_strategy'])
%>

<p>This task inspects registered datasets all together.</p>

% for ms in summary.keys():
    <h4>Overview plots for ${ms}</h4>
    
    <ul class="thumbnails">
        % for _type in plot_types:
            % if os.path.exists(summary[ms][_type].thumbnail):
            <li class="span3">
                <div class="thumbnail">
                    <a href="${os.path.relpath(summary[ms][_type].abspath, pcontext.report_dir)}"
                       class="fancybox"
                       rel="thumbs">
                        <img src="${os.path.relpath(summary[ms][_type].thumbnail, pcontext.report_dir)}"
                             title="${plot_titles[_type]}"
                             data-thumbnail="${os.path.relpath(summary[ms][_type].thumbnail, pcontext.report_dir)}">
                        </img>
                    </a>

                    <div class="caption">
                        <h4>
                            <a href="${os.path.relpath(os.path.join(dirname, subplot[ms][_type]), pcontext.report_dir)}"
                               class="replace">
                               ${plot_titles[_type]}
                            </a>
                        </h4>

                        <p>${plot_desc[_type]}</p>
                    </div>
                </div>
            </li>
            % endif
        % endfor
    </ul>
%endfor

<h4>Summary of Reduction Group</h4>
<p>Reduction group is a set of data that will be processed together at the following stages such as 
baseline subtraction and imaging. Grouping was performed based on frequency coverage and number of 
channels of spectral window.</p>

<table class="table table-bordered table-striped table-condensed"
       summary="Summary of Reduction Group">
    <caption>Summary of Reduction Group</caption>
    <thead>
        <tr>
            <th scope="col" rowspan="2">Group ID</th>
            <th scope="col" colspan="2">Frequency Range</th>
            <th scope="col" rowspan="2">Measurement Set</th>
            <th scope="col" rowspan="2">Scantable</th>
            <th scope="col" rowspan="2">Antenna</th>
            <th scope="col" rowspan="2">Spectral Window</th>
            <th scope="col" rowspan="2">Num Chan</th>
        </tr>
        <tr>
            <th>Min [MHz]</th>
            <th>Max [MHz]</th>
        </tr>
    </thead>
    <tbody>
    % for (group_id, group_desc) in pcontext.observing_run.reduction_group.items():
        <tr>
            <td rowspan="${len(group_desc)}">${group_id}</td>
            % for f in group_desc.frequency_range:
                <td rowspan="${len(group_desc)}">${'%7.1f'%(f/1.e6)}</td>
            % endfor
            % for m in group_desc:
                <td>${pcontext.observing_run[m.antenna].ms.basename}</td>
                <td>${pcontext.observing_run[m.antenna].basename}</td>
                <td>${pcontext.observing_run[m.antenna].antenna.name}</td>
                <td>${m.spw}</td>
                <td>${pcontext.observing_run[m.antenna].spectral_window[m.spw].nchan}</td>
                % if len(group_desc) > 1:
                    </tr><tr>
                % endif
            % endfor
        </tr>
    % endfor
    </tbody>
</table>

<h4>Calibration Strategy</h4>
<p>Below is a summary of sky calibration mode and spectral window mapping for T<sub>sys</sub> calibration.</p>
<table class="table table-bordered table-striped table-condensed"
       summary="Summary of Calibration Strategy">
    <caption>Summary of Calibration Strategy</caption>
    <thead>
        <tr>
            <th scope="col" rowspan="2">Scantable</th>
            <th scope="col" rowspan="2">Antenna</th>
            <th scope="col" rowspan="2">Sky Calibration Mode</th>
            <th scope="col" colspan="2">T<sub>sys</sub> Spw Map</th>
        </tr>
        <tr>
            <th>T<sub>sys</sub></th>
            <th>Target</th>
        </tr>
    </thead>
    <tbody>
    % for st in pcontext.observing_run:
        <tr>
            <td rowspan="${len(spwmap[st.basename])}">${st.basename}</td>
            <td rowspan="${len(spwmap[st.basename])}">${st.antenna.name}</td>
            <td rowspan="${len(spwmap[st.basename])}">${st.calibration_strategy['calmode']}</td>
            % for (k,v) in spwmap[st.basename].items():
                <td>${k}</td>
                <td>${','.join(map(str, v))}</td>
                % if len(spwmap[st.basename]) > 1:
                    </tr><tr>
                % endif
            % endfor
        </tr>
    % endfor
    </tbody>
</table>
