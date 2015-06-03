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

<%block name="title">Tclean/MakeImages</%block>

<h2>Image Details</h2>

%if not len(result[0].targets):
    <p>There are no clean results.
%else:
    <table class="table table-striped">
                <thead>
                <tr>
                    <th>Field</th>
                    <th>Spw</th>
                    <th>Pol</th>
                    <th colspan="2">Image details</th>
                    <th>Image result</th>
                </tr>
                </thead>
                <tbody>

                <%
                ## get sorted key lists so that table entries are ordered
                fields = sorted(set([k[0] for k in info_dict.keys()]))
                spws = sorted(set([int(k[1]) for k in info_dict.keys()]))
                pols = sorted(set([k[2] for k in info_dict.keys()]))
                %>
                % for field in fields:
                    % for spw in spws:
                        % for pol in pols:
                            %if info_dict.get((field,str(spw),pol,'frequency')) is not None:
                            <tr>
                                <td rowspan="8">${field}</td>
                                <td rowspan="8">${spw}</td>
                                <td rowspan="8">${pol}</td>
								<th>frequency</th>
								<td>${casatools.quanta.tos(info_dict[(field,str(spw),pol,'frequency')], 4)}</td>
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
                                <td>No image available</td>
                                %endif
							</tr>
                            <tr>
								<th>beam</th>
                                %if info_dict.get((field,str(spw),pol,'beam major')) is not None:
                                    <%
                                    beam_major = casatools.quanta.tos(info_dict[(field,str(spw),pol,'beam major')],2)
                                    beam_major = beam_major.replace('arcsec', '')
                                    %>
                                <td>${beam_major}x${
                                            casatools.quanta.tos(info_dict[(field,str(spw),pol,'beam minor')],2)}</td>
                                %else:
                                <td>-</td>
                                %endif
							</tr>
                            <tr>
                                            <th>beam p.a.</th> 
                                %if info_dict.get((field,str(spw),pol,'beam pa')) is not None:
                                            <td>${casatools.quanta.tos(info_dict[(field,str(spw),pol,'beam pa')],1)}</td>
                                %else:
                                            <td>-</td>
                                %endif
                            </tr>
                            <tr>
                                            <th>image maximum </th>
                                %if info_dict.get((field,str(spw),pol,'max')) is not None:
                                            <td>${'%.2e %s' % (info_dict[(field,str(spw),pol,'max')],
                                                info_dict[(field,str(spw),pol,'brightness unit')])}</td>
                                %else:
                                            <td>-</td>
                                %endif
                            </tr>
                            <tr>
                                <th>residual rms</th>
                                %if info_dict.get((field,str(spw),pol,'residual rms')) is not None:
                                            <td>${'%.2e %s' % (info_dict[(field,str(spw),pol,'residual rms')],
                                                info_dict[(field,str(spw),pol,'brightness unit')])}</td>
                                %else:
                                            <td>-</td>
                                %endif
                            </tr>
                            <tr>
                                        <th>channels</th>
                                %if info_dict.get((field,str(spw),pol,'nchan')) is not None:
                                            <td>${'%d x %s' % (info_dict[(field,str(spw),pol,'nchan')],
                                                info_dict[(field,str(spw),pol,'width')])}</td>
                                %else:
                                            <td>-</td>
                                %endif
                            </tr>
                            <tr>
                                        <th>score</th>
                                %if info_dict.get((field,str(spw),pol,'score')) is not None:
                                            <td><span class="badge ${rendererutils.get_badge_class(info_dict[(field,str(spw),pol,'score')])}">${'%0.2f' % (info_dict[(field,str(spw),pol,'score')].score)}</span></td>
                                %else:
                                            <td>-</td>
                                %endif
                            </tr>
                            <tr>
                               <th>image file</th>
                               <td colspan="2">${info_dict[(field,str(spw),pol,'image name')]}</td>
                            </tr>
                            %endif
                        %endfor
                    %endfor
                %endfor
                </tbody>
        </table>
%endif
