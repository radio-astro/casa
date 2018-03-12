<%!
rsc_path = ""
import os
import types

import pipeline.infrastructure.renderer.htmlrenderer as hr
import pipeline.infrastructure.logging as logging

agent_description = {
	'before'   : 'Before Task',
	'anos'     : 'ANOS',
	'shadow'   : 'Shadowed Antennas',
	'intents'  : 'Unwanted Intents',
	'online'   : 'Other Online Flags',
	'template' : 'Flagging Template',
	'autocorr' : 'Autocorr',
	'edgespw'  : 'Edge Channels',
	'clip'     : 'Clipping',
	'quack'    : 'Quack',
	'baseband' : 'Baseband'
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
<%inherit file="t2-4m_details-base.mako"/>

<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>
<script>
$(document).ready(function(){
    $("th.rotate").each(function(){ $(this).height($(this).find('span').width() + 8) });
});
</script>

<%
# these functions are defined in template scope so we have access to the flags 
# and agents context objects

def total_for_mses(mses, row):
	flagged = 0
	total = 0
	for ms in mses:
	        #total += flags[ms]['before'][row].total
		if 'edgespw' in flags[ms].keys():
		    total += flags[ms]['edgespw'][row].total
		elif 'clip' in flags[ms].keys():
		    total += flags[ms]['clip'][row].total
		elif 'quack' in flags[ms].keys():
		    total += flags[ms]['quack'][row].total
		elif 'baseband' in flags[ms].keys():
		    total += flags[ms]['baseband'][row].total
		else:
		    total += flags[ms]['before'][row].total
		for agent in flags[ms].keys():
			fs = flags[ms][agent][row]
			if not (agent == 'before' or agent == 'anos' or agent == 'shadow' or agent == 'intents'):
			    flagged += fs.flagged
	if total is 0:
		return 'N/A'
	else: 
		return '%0.2f%%' % (100.0 * flagged / total)
		#return '%0.1f%%' % (total)

def total_for_agent(agent, row, mses=flags.keys()):
	flagged = 0
	total = 0
	for ms in mses:
		if agent in flags[ms]:
			fs = flags[ms][agent][row]
			flagged += fs.flagged
			total += fs.total
		else:
			# agent was not activated for this MS. 
			total += flags[ms]['before'][row].total
	if total is 0:
		return 'N/A'
	else:
		return '%0.2f%%' % (100.0 * flagged / total)

def agent_data(agent, ms):
	if agent not in flags[ms]:
		if agent in ('online', 'template'):
			return '<td></td><td></td>'
		else:
			return '<td></td>' 

	if agent in ('online', 'template'):
		vis_type = type(result.inputs['vis'])
		if vis_type is types.StringType:
			flagfile = os.path.basename(result.inputs['file%s' % agent])			
		elif vis_type is types.ListType:
			for v in result.inputs['vis']:
				if os.path.basename(v) == ms:
					ms_idx = result.inputs['vis'].index(v)
			flagfile = os.path.basename(result.inputs['file%s' % agent][ms_idx])

		relpath = os.path.join('stage%s' % result.stage_number, flagfile)
		abspath = os.path.join(pcontext.report_dir, relpath)
		if os.path.exists(abspath):
			num_lines = sum(1 for line in open(abspath) if not line.startswith('#'))
			return ('<td><a class="replace-pre" href="%s">%s</a></td>'
					'<td>%s</td>' % (relpath, flagfile, num_lines))
		else:
			return '<td>%s</td><td>N/A</td>' % flagfile		
	else:
		return '<td><span class="glyphicon glyphicon-ok"></span></td>'		

def agent_td(agent, ms):
	if agent not in flags[ms]:
		return '<td><span class="glyphicon glyphicon-remove"></span></td>' 
	else:
		return '<td><span class="glyphicon glyphicon-ok"></span></td>'		

%>

<%block name="title">VLA Deterministic Flagging</%block>

<div class="row-fluid">
	<h2>Flagging agents</h2>
	<table class="table table-bordered table-striped">
		<caption>Flagging agent status per measurement set.</caption>	   
		<thead>
			<tr>
				<th>Measurement Set</th>
% for agent in agents[1:]:
				<th>${agent_description[agent]}</th>
% endfor
				<th>Agent Commands</th>
			</tr>				
		</thead>
		<tbody>
% for ms in flags.keys():
			<tr>
				<td>${ms}</td>
	% for agent in agents[1:]:
				${agent_td(agent, ms)}
	% endfor
				<td><a class="replace-pre" href="${os.path.relpath(flagcmds[ms], pcontext.report_dir)}">View</a></td>
			</tr>
% endfor
		</tbody>
	</table>
</div>

<%
flagging_agents = get_template_agents(agents)
mses = [m for m in flags.keys() if 'online' in flags[m] or 'template' in flags[m]]
%>
% if mses:
<div class="row-fluid">
	<h2>Template Files</h2>
	<table class="table table-bordered table-striped">
		<caption>Files used for template flagging steps.</caption>	   
		<thead>
			<tr>
				<th rowspan="2">Measurement Set</th>
	% for agent in flagging_agents:
				${template_agent_header1(agent)}
	% endfor
			</tr>	
			<tr>
	% for agent in flagging_agents:
				${template_agent_header2(agent)}
	% endfor
			</tr>			
		</thead>
		<tbody>
	% for ms in mses:
			<tr>
				<td>${ms}</td>
		% for agent in flagging_agents:
				${agent_data(agent, ms)}
		% endfor
			</tr>
	% endfor
		</tbody>
	</table>
</div>
% endif

<h2>Flagged data summary</h2>
<table class="table table-bordered table-striped "
	   summary="Flagged Data">
	<caption>Summary of flagged data. Each cell states the amount of data 
	flagged as a fraction of the specified data selection, with the 
	<em>Flagging Agent</em> columns giving this information per flagging agent.
	</caption>
	<thead>
		<tr>
			<th rowspan="2">Data Selection (by intent)</th>
			<!-- flags before task is always first agent -->
			<th rowspan="2">${agent_description[agents[0]]}</th>
			<th colspan="3">Flagging Agent (Total Vis)</th>
			<th colspan="${len(agents)-4}">Flagging Agent (Science Vis)</th>
			<th rowspan="2">Total Science</th>
			<th colspan="${len(flags)}">Measurement Set</th>
		</tr>
		<tr>
%for agent in agents[1:]:
			<th>${agent_description[agent]}</th>
%endfor
%for ms in flags.keys():
			<th>${ms}</th>
%endfor
		</tr>
	</thead>
	<tbody>
%for k in ['TOTAL', 'SCIENCE SPWS', 'BANDPASS', 'AMPLITUDE', 'PHASE', 'TARGET']: 
		<tr>
			<th>${total_keys[k]}</th>		
	% for agent in agents:
			<td>${total_for_agent(agent, k)}</td>
	% endfor
			<td>${total_for_mses(flags.keys(), k)}</td>
	% for ms in flags.keys():
			<td>${total_for_mses([ms], k)}</td>
	% endfor		
		</tr>
%endfor
%for ms in flags.keys():
		<tr>
			<th>${ms}</th>
	% for agent in agents:
			<td>${total_for_agent(agent, 'TOTAL', [ms])}</td>
	% endfor
			<td>${total_for_mses([ms], 'TOTAL')}</td>
	% for ms in flags.keys():
			<td></td>
	% endfor
		</tr>
%endfor
	</tbody>
</table>

<%self:plot_group plot_dict="${flagplots}"
				  url_fn="${lambda x: 'junk'}">

	<%def name="title()">
		Flagging reason vs time
	</%def>

	<%def name="preamble()">
		Plots of flagging reason vs time. The reasons for flagging the data are defined in the plot legend.
	</%def>

	<%def name="mouseover(plot)">Click to show flagging reason vs time for ${plot.parameters['vis']}</%def>

	<%def name="fancybox_caption(plot)">
        MS: ${plot.parameters['vis']}
	</%def>

    <%def name="caption_title(plot)"></%def>

</%self:plot_group>
