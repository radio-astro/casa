<%!
import os.path

def num_lines(report_dir, relpath):
	abspath = os.path.join(report_dir, relpath)
	if os.path.exists(abspath):
		return sum(1 for line in open(abspath) if not line.startswith('#'))
	else:
		return 'N/A'
		
comp_descriptions = {'mediandeviant' : 'Flag antennas with deviant median gains.',
                 	 'rmsdeviant'    : 'Flag antennas with deviant RMS gains.',
                 	 'nrmsdeviant'   : 'Flag antennas with deviant normalised RMS gains.'}

%>

<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">Flag antennas with deviant gain</%block>

<h2>Flagging steps</h2>
The following section provides summary plots and flagging commands that 
resulted from each flagging metric that the pipeline uses to determine 
antennas with deviant gains.
 
% for component in components:
	<h3>${component}</h3>
	${comp_descriptions[component]}

##     % if component in plots:
## 	<h4>Plots</h4>
## 	<ul>
## 	    % for vis, relpath in plots[component].items():
## 	        <li>
## 	               <a class="replace" href="${relpath}" data-vis="${vis}">${vis}</a>
## 	               shows the images used for flagging.
## 	    % endfor
## 	</ul>
##     % endif
 
    % if htmlreports[component]:
      <h4>Flags</h4>
      <table class="table table-bordered table-striped">
      <caption>Report Files</caption>
	  <thead>
	    <tr>
	        <th>Measurement Set</th>
	        <th>Flagging Commands</th>
	        <th>Number of Statements</th>
	        <th><abbr title="Shows the data views used by the pipeline to flag data">Flagging View</abbr></th>
	    </tr>
	  </thead>
	  <tbody>
	  % for msname, relpath in htmlreports[component].items():
	    <tr>
			<td>${msname}</td>
			<td><a class="replace-pre" href="${relpath}">${os.path.basename(relpath)}</a></td>
			<td>${num_lines(pcontext.report_dir, relpath)}</td>
            % if component in plots and msname in plots[component]:
                <td><a class="replace" data-vis="${msname}" href="${plots[component][msname]}">Display</a></td>
            % else:
                <td>N/A</td>
            % endif
		</tr>
	  % endfor
	  </tbody>
      </table>
    %endif
 
% endfor
