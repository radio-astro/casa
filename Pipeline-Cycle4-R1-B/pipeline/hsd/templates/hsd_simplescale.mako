<%!
rsc_path = "../"
import os
%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="header" />

<%block name="title">Apply a scaling factor for non-linearity correction</%block>

<%
try:
    table_data = []
    for r in result:
       table_lists = r.outcome['scantable']
       factor = r.outcome['factor']
       for name_list in table_lists:
           table_data.append((os.path.basename(name_list[1]), factor, os.path.basename(name_list[2])))
except Exception, e:
    print 'hsd_simplescale html template exception:', e
    raise e
%>

<h2>Scaling Summary</h2>
<table class="table table-bordered table-striped">
	<caption>Scaling per scantables</caption>	   
	<thead>
		<tr>
			<th>Input</th>
			<th>Factor</th>
			<th>Output</th>
		</tr>
	</thead>
	<tbody>
	% for row in table_data:
		<tr>
			%for elem in row:
				<td>${elem}</td>
			% endfor
		</tr>
	% endfor
	</tbody>
</table>


