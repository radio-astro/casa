<%!
import os
%>
<%inherit file="t2-4m_details-base.mako"/>
<%namespace name="importdata" file="importdata.mako"/>

<%block name="title">${importdata.title()}</%block>

<%
def get_spwmap(ms):
    dotsysspwmap = ms.calibration_strategy['tsys']
    tsys_strategy = ms.calibration_strategy['tsys_strategy']
    spwmap = {}
    if dotsysspwmap == True:
        for l in tsys_strategy:
            if l[0] in spwmap:
                spwmap[l[0]].append(l[1])
            else:
                spwmap[l[0]] = [l[1]]
        spwmap_values = [i for l in spwmap.values() for i in l]
        for spw in ms.get_spectral_windows(science_windows_only=True):
            if spw.id not in spwmap_values:
                spwmap[spw.id] = [spw.id]
    else:
        for spw in ms.get_spectral_windows(science_windows_only=True):
            spwmap[spw.id] = [spw.id]
    return spwmap
  
spwmap = {}
for ms in pcontext.observing_run.measurement_sets:
    spwmap[ms.basename] = get_spwmap(ms)
    
fieldmap = {}
for ms in pcontext.observing_run.measurement_sets:
    map_as_name = dict([(ms.fields[i].name,ms.fields[j].name) for (i,j) in ms.calibration_strategy['field_strategy'].iteritems()])
    fieldmap[ms.basename] = map_as_name
    
contents = {}
for vis, _spwmap in spwmap.iteritems():
    _fieldmap = fieldmap[vis]
    _spwkeys = _spwmap.keys()
    _spwkeys.sort()
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
baseline subtraction and imaging. Grouping is performed based on field and spectral window properties 
(frequency coverage and number of channels).</p>

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
	% for tr in reduction_group_rows:
		<tr>
		% for td in tr:
			${td}
		% endfor
		</tr>
	%endfor
	</tbody>
</table>

<h4>Calibration Strategy</h4>
<p>Summary of sky calibration mode, spectral window mapping for T<sub>sys</sub> calibration, 
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
            <td rowspan="${num_content}">${', '.join(map(lambda x: x.name, ms.antennas))}</td>
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
