<%!
import os
import types

agent_description = {
	'before'   : 'Before Task',
	'after'    : 'Flagged by Task'
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


<%
# these functions are defined in template scope so we have access to the flags 
# and agents context objects

def total_for_mses(mses, row):
	flagged = 0
	total = 0
	for ms in mses:
		total += flags[ms]['before'][row].total
		for agent in flags[ms].keys():
			fs = flags[ms][agent][row]
			flagged += fs.flagged
	if total is 0:
		return 'N/A'
	else:
		return '%0.1f%%' % (100.0 * flagged / total)

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
		return '%0.1f%%' % (100.0 * flagged / total)

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
		return '<td><i class="icon-ok"></i></td>'		

def num_lines(relpath):
	abspath = os.path.join(pcontext.report_dir, relpath)
	if os.path.exists(abspath):
		return sum(1 for line in open(abspath) if not line.startswith('#'))
	else:
		return 'N/A'
		
%>


<%inherit file="t2-4m_details-base.html"/>
<%block name="header" />

<%block name="title">Flag raw channels</%block>

% if plots:
<h2>Plots</h2>
<ul>
    % for vis, relpath in plots.items():
        <li>
               <a class="replace" href="${relpath}">${vis}</a>
               shows the images used for flagging.
    % endfor
</ul>
% endif

% if htmlreports:
<div class="row-fluid">
<h2>Flags</h2>
	<table class="table table-bordered table-striped">
		<caption>Report Files</caption>
		<thead>
			<tr>
				<th>Measurement Set</th>
				<th>Flagging Commands</th>
				<th>Number of Statements</th>
			</tr>
		</thead>
		<tbody>
		% for msname, relpath in htmlreports.items():
			<tr>
				<td>${msname}</td>
				<td><a class="replace-pre" href="${relpath}">${os.path.basename(relpath)}</a></td>
				<td>${num_lines(relpath)}</td>
	        </tr>
		% endfor
		</tbody>
	</table>
</div>

<div class="row-fluid">
	<h2>Flagged data summary</h2>
	<table class="table table-bordered table-striped "
		   summary="Flagged Data">
		<caption>Summary of flagged data.</caption>
		<thead>
			<tr>
				<th rowspan="2">Data Selection</th>
				<!-- flags before task is always first agent -->
				<th rowspan="2">${agent_description[agents[0]]}</th>
				<th rowspan="2">${agent_description[agents[1]]}</th>
				<th rowspan="2">Total</th>
				<th colspan="${len(flags)}">Measurement Set</th>
			</tr>
			<tr>
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
</div>

% endif

