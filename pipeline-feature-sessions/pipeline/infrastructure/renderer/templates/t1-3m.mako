<%!
navbar_active='By Topic'
import pipeline.infrastructure.renderer.htmlrenderer as hr
import pipeline.infrastructure.renderer.rendererutils as rendererutils
import pipeline.infrastructure.pipelineqa as pipelineqa
import pipeline.domain.measures as measures
import pipeline.extern.asizeof as asizeof
import pipeline.infrastructure.logging as logging
import math

def get_lowest_scoring_results(topic, context):
	all_results_lists = topic.results_by_type.values()
	if not all_results_lists:
		return ('No tasks in this topic', pipelineqa.QAScore(None, '', ''))
		
	min_scoring_results = []
	for l in all_results_lists:
		no_na = filter(lambda z:z.qa.representative.score is not None, l)
		if no_na:
			min_result_for_task = min(no_na, 
								      key=lambda result: result.qa.representative.score)
			min_scoring_results.append(min_result_for_task)

	if not min_scoring_results:
		return ('No scoring tasks in this topic', pipelineqa.QAScore(None, '', ''))
		
	minresult = min(min_scoring_results, 
				    key=lambda result:result.qa.representative.score)
	anchor = '<a href="t2-4m.html?sidebar=sidebar_stage%s">%s</a>' % (minresult.stage_number, 
												                      hr.get_task_description(minresult, context))
	return (anchor,
			minresult.qa.representative)

tablerow_css_classes = {'QA Error'   : 'danger alert-danger',
						'QA Warning' : 'warning alert-warning',
						'Error'      : 'danger alert-danger',
						'Warning'    : 'warning alert-warning'}

def get_tablerow_class(row):
	return tablerow_css_classes.get(row.type, '')

def flagcolortable(flagpct):
    '''input is value from 0 to 100 percentage
	Returns colorstring
    '''

    colors={'0':'#FFFFFF',
	    '1':'#E0F2F7',
	    '2':'#CEECF5',
	    '3':'#A9E2F3',
	    '4':'#81DAF5',
	    '5':'#58D3F7',
	    '6':'#2ECCFA',
	    '7':'#00BFFF',
	    '8':'#01A9DB',
	    '9':'#0489B1',
	    '10':'#0489B1'}
            
    return colors[str(int(math.floor(flagpct/10.0)))]
    
%>

<%inherit file="base.mako"/>

<%block name="title">Topic Summary</%block>

<style type="text/css">

.table-header-rotated table{
  table-layout: fixed;
  width:100px;
}

.table-header-rotated caption{
  text-align: left;
}

.table-header-rotated tbody {
    display: block;
    overflow-x: auto;
}

.table-header-rotated th.row-header{
  width: auto;
  font-size: 11px;
  
}

.table-header-rotated td{
  height: 20px;
  width: 10px;
  border-top: 1px solid #dddddd;
  border-left: 1px solid #dddddd;
  border-right: 1px solid #dddddd;
  border-bottom: 1px solid #dddddd;
  vertical-align: middle;
  text-align: center;
  font-size: 10px;
}


.table-header-rotated td.rotate-45{
  height: 20px;
  width: 10px;
  border-top: 0px solid #dddddd;
  border-left: 0px solid #dddddd;
  border-right: 1px solid #dddddd;
  border-bottom: 1px solid #dddddd;
  vertical-align: middle;
  text-align: center;
  font-size: 10px;
}
</style>

<h3>Warnings and Errors</h3>
% if tablerows:
<table class="table table-bordered table-striped table-condensed"
	   summary="Messages from tasks">
	<thead>
		<tr>
			<th>Stage</th>
			<th>Task</th>
			<th>Type</th>
			<th>Message</th>
		</tr>
	</thead>
	<tbody>
	% for row in tablerows:
		<tr class="${get_tablerow_class(row)}">
			<td><a href="t2-4m.html?sidebar=sidebar_stage${row.stage}${row.target}">${row.stage}</a></td>
			<td><a href="t2-4m.html?sidebar=sidebar_stage${row.stage}${row.target}">${row.task}</a></td>
			<td>${row.type}</td>
			<td>${row.message}</td>
		</tr>
	% endfor		
	</tbody>
</table>
% else:
<p>No warnings or errors.</p>
% endif

<h3>Tasks by Topic</h3>
<table class="table">
	<thead>
		<tr>
			<th class="col-md-1">Topic</th>
			<th class="col-md-8">Lowest Scoring Task</th>
			<th class="col-md-2">Min Score</th>
			<th class="col-md-1"></th>
		</tr>
	</thead>
	<tbody>
		% for _, topic in registry.get_topics().items():
		<% (task_description, pqascore) = get_lowest_scoring_results(topic, pcontext) %>
			% if not task_description == 'No tasks in this topic':
			<tr>
				<td><a href="${topic.url}">${topic.description}</a></td>
				<td>${task_description}</a><span class="pull-right">${rendererutils.format_shortmsg(pqascore)}</span></td>
				<td><div class="progress" style="margin-bottom:0px;"><div class="progress-bar${rendererutils.get_bar_class(pqascore)}" role="progressbar" style="width:${rendererutils.get_bar_width(pqascore)}%;"><span class="text-center"></span></div></div></td>
				<td><span class="badge${rendererutils.get_badge_class(pqascore)}">${rendererutils.format_score(pqascore)}</span></td>
			</tr>
			% endif
		% endfor			
	</tbody>
</table>


% if flagtables:

    <h3>Flagging Summaries</h3>
    <div class="col-md-6">
    
    % for ms_name in flagtables.keys():
        
        <%
            flagtable = flagtables[ms_name]
        %>
    
         <h4>${ms_name}</h4>

		<%
		    Nplots = 1
		    if pcontext.project_summary.telescope == 'VLA' or pcontext.project_summary.telescope == 'EVLA':
                Nplots = 1
		    endif

		%>
	    % for fieldkey in flagtable.keys()[0:len(flagtable.keys()):Nplots]:
		    <br>
		     <h5>Flagging percentages for  ${fieldkey} </h5>
	        <div style="width: 1700px; height: 250px; overflow: auto;">

		    <table class="table table-header-rotated">
		    <!-- <caption>Flagging percentages for  ${fieldkey} </caption> -->

		    <thead>
		    </thead>
	    
		    <tbody>
		    <tr>
		    <td class="rotate-45"><b>spw</b></td>
		    <%
		    name = flagtable[fieldkey].keys()[0]
		    spw = flagtable[fieldkey][name].keys()[0]
		    antennalist = flagtable[fieldkey][name][spw].keys()
		    %>
		    % for antenna in sorted(antennalist):
		        <td class="rotate-45"><b>${antenna}</b></td>
		    % endfor
		    </tr>
		
		
		    % for name in sorted(flagtable[fieldkey].keys()):
		        <tr>
		        <th class="row-header">${flagtable[fieldkey][name].keys()[0]}</th>
		    
		        % for spw in flagtable[fieldkey][name].keys():
			        % for antenna in sorted(flagtable[fieldkey][name][spw].keys()):
			            <%
				        flagpct = 100.0*flagtable[fieldkey][name][spw][antenna]['flagged']/flagtable[fieldkey][name][spw][antenna]['total']
			            %>
			            <td bgcolor="${flagcolortable(flagpct)}">
						${"{:5.2f}".format(flagpct)}
			            </td>
			        % endfor
		
		        % endfor
		        <tr>
		    % endfor
	    
		    </tbody>
		    </table>

	        </div>
	    % endfor
    
    % endfor

% endif
