<%!
import os.path
import pydoc
import sys

import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="task_details_base.html"/>
<%block name="header" />

<%block name="title">Stage ${hr.get_stage_number(result)} Task Details</%block>

<h1>Stage ${hr.get_stage_number(result)}: ${hr.get_task_description(result)}</h1>

<ul>
    <li>Results:</li>
        <ul>
            <li>These are the fluxes held in the context after hif_normflux has averaged 
            separate measurements of the calibrators.
            <table border="1">
                <tr>
                    <td>Measurement Set</td>
                    <td>Field</td>
                    <td>SpW</td>
                    <td>I</td>
                    <td>Q</td>
                    <td>U</td>
                    <td>V</td>
                </tr>

            %for setjy_result in result:
                %for vis, fields in setjy_result.results.items():
                    %for field, field_fluxes in fields.items():
                        <%
                        flux_by_spw = sorted(field_fluxes, key=lambda g: g.spw.id)
                        %>
                        %for flux in flux_by_spw:
                            <tr>
                                <td>${os.path.basename(vis)}</td>
                                <td>${field}</td>
                                <td>${flux.spw.id}</td>
                                <td>${str(flux.I)}</td>
                                <td>${str(flux.Q)}</td>
                                <td>${str(flux.U)}</td>
                                <td>${str(flux.V)}</td>
                            </tr>
                        %endfor
                    %endfor
                %endfor
            %endfor
            </table>
        </ul>

## Write help information to a file and link to it. Probably a better
## way of writing the file but I can't find it.

<%
try:
    r = result[0]
    stage_dir = os.path.join(pcontext.report_dir, 'stage%d' % (r.stage_number))
    filename = os.path.join(stage_dir, 'taskhelp.txt')
    f = open(filename, 'w')
    sys.stdout = f
    pydoc.help('hif_normflux')

except Exception, e:
    print 'html template exception in help information:', e
    raise e

finally:
    sys.stdout = sys.__stdout__
    f.close()
%>

        <li>A listing of the 'help' information for this task can be found
            <a href='taskhelp.txt'>here</a>.
</ul>

<p><table class="one-column-emphasis" summary="Input parameters">
	<caption>Input parameters</caption>
    <colgroup>
    	<col class="oce-first" />
    </colgroup>
	<tbody>
		% for k, v in result.inputs.iteritems():
		<tr>
		  <td>${str(k)}</th>
		  <td>${str(v)}</td>
		</tr>
		% endfor
	</tbody>
</table>

<p><table class="one-column-emphasis" summary="Timing details">
	<caption>Timing details</caption>
    <colgroup>
    	<col class="oce-first" />
    </colgroup>
	<tbody>
		<tr>
		  <td>Start time</th>
		  <td>${result.timestamps.start}</td>
		</tr>
		<tr>
		  <td>End</th>
		  <td>${result.timestamps.end}</td>
		</tr>
		<tr>
		  <td>Duration</th>
		  <td>${str(result.timestamps.end - result.timestamps.start)}</td>
		</tr>
	</tbody>
</table>
