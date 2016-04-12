<%!
import os
%>
<%inherit file="t2-4m_details-base.html"/>
<%namespace name="importdata" file="importdata.mako"/>

<%block name="title">${importdata.title()}</%block>

<%
def get_spwmap(tsys_strategy):
    spwmap = {}
    for l in tsys_strategy:
        if spwmap.has_key(l[0]):
            spwmap[l[0]].append(l[1])
        else:
            spwmap[l[0]] = [l[1]]
    return spwmap
  
spwmap = {}
for ms in pcontext.observing_run.measurement_sets:
    spwmap[ms.basename] = get_spwmap(ms.calibration_strategy['tsys_strategy'])
    
fieldmap = {}
for ms in pcontext.observing_run.measurement_sets:
    map_as_name = dict([(ms.fields[i].name,ms.fields[j].name) for (i,j) in ms.calibration_strategy['field_strategy'].items()])
    fieldmap[ms.basename] = map_as_name
    
contents = {}
for vis, _spwmap in spwmap.items():
    _fieldmap = fieldmap[vis]
    _spwkeys = _spwmap.keys()
    _fieldkeys = _fieldmap.keys()
    l = max(len(_spwkeys), len(_fieldkeys))
    _contents = []
    for i in xrange(l):
        items = ['', '', '', '']
        if i < len(_spwkeys):
            key = _spwkeys[i]
            items[0] = key
            items[1] = ','.join(map(str, _spwmap[key]))
        if i < len(_fieldkeys):
            key = _fieldkeys[i]
            items[2] = _fieldmap[key].strip('"')
            items[3] = key.strip('"')
        _contents.append(items)
    contents[vis] = _contents
%>

${importdata.body()}

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
            <th scope="col" rowspan="2">Field</th>
            <th scope="col" rowspan="2">Measurement Set</th>
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
    % for (group_id, group_desc) in pcontext.observing_run.ms_reduction_group.items():
        <tr>
            <td rowspan="${len(group_desc)}">${group_id}</td>
            % for f in group_desc.frequency_range:
                <td rowspan="${len(group_desc)}">${'%7.1f'%(f/1.e6)}</td>
            % endfor
            <td rowspan="${len(group_desc)}">${group_desc.field_name}</td>
            % for m in group_desc:
                <td>${m.ms.basename}</td>
                <td>${m.ms.antennas[m.antenna].name}</td>
                <td>${m.ms.spectral_windows[m.spw].id}</td>
                <td>${m.ms.spectral_windows[m.spw].num_channels}</td>
                % if len(group_desc) > 1:
                    </tr><tr>
                % endif
            % endfor
        </tr>
    % endfor
    </tbody>
</table>

<h4>Calibration Strategy</h4>
<p>Below is a summary of sky calibration mode, spectral window mapping for T<sub>sys</sub> calibration, 
and mapping information on reference and target fields.</p>
<table class="table table-bordered table-striped table-condensed"
       summary="Summary of Calibration Strategy">
    <caption>Summary of Calibration Strategy</caption>
    <thead>
        <tr>
            <th scope="col" rowspan="2">MS</th>
            <th scope="col" rowspan="2">Antenna</th>
            <th scope="col" rowspan="2">Sky Calibration Mode</th>
            <th scope="col" colspan="2">T<sub>sys</sub> Spw Map</th>
            <th scope="col" colspan="2">Field Map</th>
        </tr>
        <tr>
            <th>T<sub>sys</sub></th>
            <th>Target</th>
            <th>Reference</th>
            <th>Target</th>
        </tr>
    </thead>
    <tbody>
    % for ms in pcontext.observing_run.measurement_sets:
        <%
            content = contents[ms.basename]
            num_content = len(content)
        %>
        <tr>
            <td rowspan="${num_content}">${ms.basename}</td>
            <td rowspan="${num_content}">${','.join(map(lambda x: x.name, ms.antennas))}</td>
            <td rowspan="${num_content}">${ms.calibration_strategy['calmode']}</td>
            % for items in content:
                % for item in items:
                    <td>${item}</td>
                % endfor
                % if num_content > 1:
                    </tr><tr>
                % endif
            % endfor
        </tr>
    % endfor
    </tbody>
</table>
