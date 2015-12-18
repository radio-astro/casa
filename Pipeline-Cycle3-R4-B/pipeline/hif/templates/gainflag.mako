<%!
import os.path

def num_lines(report_dir, relpath):
	abspath = os.path.join(report_dir, relpath)
	if os.path.exists(abspath):
		return sum(1 for line in open(abspath) if not line.startswith('#'))
	else:
		return 'N/A'
		
comp_descriptions = {'mediandeviant': 'Flag antennas with deviant median gains.',
                 	 'rmsdeviant'   : 'Flag antennas with deviant RMS gains.'}

%>

<%inherit file="t2-4m_details-base.html"/>

<%block name="title">Flag antennas with deviant gain</%block>

<h2>Flagging steps</h2>
The following section provides summary plots and flagging commands that 
resulted from each flagging metric that the pipeline uses to determine 
antennas with deviant gains.
 
<ul>
% for component in components:
	<li>
	<h3>${component}</h3>
	${comp_descriptions[component]}

    % if htmlreports[component]:
      <h4>Flags</h4>
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
	  % for msname, relpath in htmlreports[component].items():
	    <tr>
			<td>${msname}</td>
			<td><a class="replace-pre" href="${relpath}">${os.path.basename(relpath)}</a></td>
			<td>${num_lines(pcontext.report_dir, relpath)}</td>
		</tr>
	  % endfor
	  </tbody>
      </table>
    %endif
 
    % if component in plots:
	<h4>Plots</h4>
	<ul>
	    % for vis, relpath in plots[component].items():
	        <li>
	               <a class="replace" href="${relpath}">${vis}</a>
	               shows the images used for flagging.
	    % endfor	
	</ul>
    % endif
 
  </li>
% endfor
</ul>
