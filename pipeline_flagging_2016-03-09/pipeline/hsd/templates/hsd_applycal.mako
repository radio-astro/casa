<%!
import os
import string
import types

agent_description = {
	'before'   : 'Before',
	'applycal' : 'After',
}

total_keys = {
	'TOTAL'        : 'All Data',
	'SCIENCE SPWS' : 'Science Spectral Windows',
	'BANDPASS'     : 'Bandpass',
	'AMPLITUDE'    : 'Flux',
	'PHASE'        : 'Phase',
	'TARGET'       : 'Target'
}

def template_agent_header1(agent):
	span = 'col' if agent in ('online','template') else 'row'
	return '<th %sspan=2>%s</th>' % (span, agent_description[agent])

def template_agent_header2(agent):
	if agent in ('online', 'template'):
		return '<th>File</th><th>Number of Statements</th>'
	else:
		return ''		

def get_template_agents(agents):
	return [a for a in agents if a in ('online', 'template')]


%>
<%inherit file="t2-4m_details-base.html"/>

<script>
$(document).ready(function(){
	$(".fancybox").fancybox();
	$('.caltable_popover').popover({
		  template: '<div class="popover gaintable_popover"><div class="arrow"></div><div class="popover-inner"><h3 class="popover-title"></h3><div class="popover-content"><p></p></div></div></div>'
	});
});
</script>

<%
def space_comma(s):
	return ', '.join(string.split(s, ','))
	
def ifmap_to_spwmap(num_spw, ifmap):
    spwmap = []
    for i in xrange(num_spw):
        j = i
        for (k,v) in ifmap.items():
            if i in v:
                j = k
        spwmap.append(j)
    return spwmap
%>


<%block name="title">Apply calibration tables</%block>

<%
from pipeline.infrastructure.casatools import TableReader, quanta
tsysdict = {}
for st in pcontext.observing_run:
    prefix = st.basename.replace('.asap','')
    science_spws = [m[1] for m in st.calibration_strategy['tsys_strategy']]
    tsysdict[st.basename] = {}
    for spwid in science_spws:
        tsystable = '.'.join([prefix, 'spw%s'%(spwid), 'tsyscal.tbl'])
        tsysdict[st.basename][spwid] = {}
        pol = st.spectral_window[spwid].pol_association[0]
        for (polid, corr) in zip(st.polarization[pol].polno, st.polarization[pol].corr_string):
            with TableReader(tsystable) as tb:
                tsel = None
                try:
                    tsel = tb.query('POLNO==%s'%(polid), sortlist='TIME')
                    tsys = tsel.getcol('TSYS')
                    time = tsel.getcol('TIME')
                    if len(time) > 0:
                        tsys_list = tsys[0]
                        time_list = map(lambda t: quanta.time(quanta.quantity(t,'d'), form=['fits'])[0], time)
                    else:
                        tsys_list = ['N/A']
                        time_list = ['-']
                    tsysdict[st.basename][spwid][corr] = dict(zip(time_list, tsys_list))
                finally:
                    if tsel is not None:
                        tsel.close()
%>

<p>This task applies all calibrations registered with the pipeline to their target scantables.<p>

<h2>Applied calibrations</h2>
<table class="table table-bordered table-striped table-condensed"
	   summary="Applied Calibrations">
	<caption>Applied Calibrations</caption>
	<thead>
		<tr>
            <th rowspan="2">Scantable</th>
			<th colspan="4">Target</th>
			<th colspan="6">Calibration</th>
		</tr>
		<tr>
			<th>Intent</th>
			<th>Field</th>
			<th>Spw</th>
			<th>Antenna</th>
			<th>Type</th>
			<th>spwmap</th>
			<th>interp</th>
			<th>table</th>
		</tr>
	</thead>
	<tbody>
% for vis in calapps:
	% for calapp in calapps[vis]:
		<% ca_rowspan = len(calapp.calfrom) %>
		<% num_spw = len(pcontext.observing_run.get_scantable(os.path.basename(calapp.infile)).spectral_window) %>
		<tr>
			<td rowspan="${ca_rowspan}">${vis}</td>
			<td rowspan="${ca_rowspan}">${space_comma(calapp.calto.intent)}</td>
			<td rowspan="${ca_rowspan}">${space_comma(calapp.calto.field)}</td>
			<td rowspan="${ca_rowspan}">${space_comma(calapp.calto.spw)}</td>
			<td rowspan="${ca_rowspan}">${space_comma(calapp.calto.antenna)}</td>
		% for calfrom in calapp.calfrom:
			<td>${caltypes[calfrom.gaintable]}</td>
			<td>${', '.join([str(i) for i in ifmap_to_spwmap(num_spw, calfrom.spwmap)])}</td>
			<td>${space_comma(calfrom.interp if len(calfrom.interp) > 0 else 'linear')}</td>
			<td><a href="#" class="btn btn-small caltable_popover" data-toggle="popover" data-placement="top" data-content="${os.path.basename(calfrom.gaintable)}" title="" data-original-title="">View...</a></td>
		</tr>
		% endfor
	% endfor
% endfor		
	</tbody>
</table>

<h2>Applied Mean Tsys Values</h2>

<p>Applied Tsys value is averaged within science spectral window. The table below lists mean Tsys value for each measurement. 
The value actually applied is interpolated in time.</p>

<table class="table table-bordered table-striped table-condensed">
    <caption>Applied Mean Tsys Values</caption>
    <thread>
        <tr>
            <th>MS</th>
            <th>Spw</th>
            <th>Antenna</th>
            <th>Polarization</th>
            <th>Time</th>
            <th>Tsys [K]</th>
        </tr>
    </thread>
    <tbody>
    % for st in pcontext.observing_run:
        % for (spwid, d) in tsysdict[st.basename].items():
            % for (corr, dd) in d.items():
                <% is_first_entry = True %>
                % for (t, v) in dd.items():
                    <tr>
                        % if is_first_entry:
                            <td rowspan="${len(dd)}">${st.ms.basename}</td>
                            <td rowspan="${len(dd)}">${spwid}</td>
                            <td rowspan="${len(dd)}">${st.antenna.name}</td>
                            <td rowspan="${len(dd)}">${corr}</td>
                            <% is_first_entry = False %>
                        % endif
                        <td>${t}</td>
                        <td>${v}</td>
                    </tr>
                % endfor
            % endfor
        % endfor
    % endfor
    </tbody>
</table>