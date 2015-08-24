<%!
import os.path
import pipeline.infrastructure.casatools as casatools
import pipeline.hif.tasks.clean.renderer as clean_renderer
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.renderer.rendererutils as rendererutils

columns = {'cleanmask' : 'Clean Mask',
           'image' : 'Image',
           'residual' : 'Residual',
           'model' : 'Final Model',
           'psf' : 'PSF'}

colorder = ['image', 'residual', 'cleanmask']

def get_plot(plots, field, spw, i, colname):
	try:
		return plots[field][spw][i][colname]
	except KeyError:
		return None
%>

<%inherit file="t2-4m_details-base.html"/>
<%block name="header" />

<%block name="title">Find Continuum</%block>

<h2>Find Continuum Details</h2>

%if not len(result[0].result_cont_ranges):
    <p>There are no continuum finding results.
%else:
    <table class="table table-striped">
                <thead>
                <tr>
                    <th>Field</th>
                    <th>Spw</th>
                    <th>Continuum frequency ranges</th>
                    <th>Status</th>
                    <th>Average spectrum</th>
                </tr>
                </thead>
                <tbody>

                <%
                ## get sorted key lists so that table entries are ordered
                fields = sorted(set(result[0].result_cont_ranges.keys()))
                spws = []
                for k in result[0].result_cont_ranges[fields[0]].keys():
                    try:
                        spws.append(int(k))
                    except:
                        spws.append(k)
                spws = sorted(set(spws))
                %>
                % for field in fields:
                    % for spw in spws:
                        <tr>
                            <td>${field}</td>
                            <td>${spw}</td>
                            %if result[0].result_cont_ranges[field][str(spw)]['cont_ranges'] != '':
                                <td>${'</br>'.join(['%s~%sGHz' % (c[0], c[1]) for c in result[0].result_cont_ranges[field][str(spw)]['cont_ranges']])}</td>
                            %else:
                                <td>-</td>
                            %endif
                            <td>${result[0].result_cont_ranges[field][str(spw)]['status']}</td>
                            <td>${result[0].result_cont_ranges[field][str(spw)]['plotfile']}</td>
                            <!--
                            <% 
                            try:
                                final_iter = sorted(plots_dict[field][str(spw)].keys())[-1]
                                plot = get_plot(plots_dict, field, str(spw), final_iter, 'image') 
                            except:
                                plot = None
                            %>
                            % if plot is not None:
                                <%
                                renderer = clean_renderer.CleanPlotsRenderer(pcontext, result, plots_dict, field, str(spw), pol)
                                with renderer.get_file() as fileobj:
                                    fileobj.write(renderer.render())
                                %>
                            <td rowspan="7">
										<a class="replace" href="${os.path.relpath(renderer.path, pcontext.report_dir)}">
										  <img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
										       title="Iteration ${final_iter}: image"
										       alt="Iteration ${final_iter}: image"
										       class="img-responsive">
										</a>
                            </td>
                            %else:
                            <td>No plot available</td>
                            %endif
                            -->
			</tr>
                    %endfor
                %endfor
                </tbody>
        </table>
%endif
