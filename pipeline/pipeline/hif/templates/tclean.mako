<%!
rsc_path = ""
import cgi
import os.path
import pipeline.infrastructure.casatools as casatools
import pipeline.hif.tasks.tclean.renderer as clean_renderer
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.renderer.rendererutils as rendererutils

columns = {'cleanmask' : 'Clean Mask',
           'flux' : 'Primary Beam',
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
<%inherit file="t2-4m_details-base.mako"/>

<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>
<script>
$(document).ready(function(){
    $(".fancybox").fancybox({
        type: 'image',
        prevEffect: 'none',
        nextEffect: 'none',
        loop: false,
        helpers: {
            title: {
                type: 'outside'
            },
            thumbs: {
                width: 50,
                height: 50,
            }
        },
    	beforeShow : function() {
        	this.title = $(this.element).attr('title');
       	},
    });
});
</script>

<%block name="header" />

<%block name="title">
<%
try:
    long_description = '<br><small>{!s}'.format(result.metadata['long description'])
except:
    long_description = ''
%>Tclean/MakeImages${long_description}</%block>


<h2>Image Details</h2>

%if not len(result[0].targets):
    %if result[0].clean_list_info == {}:
        <p>There are no clean targets.
    %else:
        <p>${result[0].clean_list_info.get('msg', '')}
    %endif
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
                fields = sorted(set([k[0] for k in info_dict]))
                spws = []
                for k in info_dict.keys():
                    try:
                        spws.append(int(k[1]))
                    except:
                        spws.append(k[1])
                spws = sorted(set(spws))
                pols = sorted(set([k[2] for k in info_dict]))
                %>
                % for field in fields:
                    % for spw in spws:
                        % for pol in pols:
                            %if info_dict.get((field,str(spw),pol,'frequency')) is not None:
                            <tr>
                                %if info_dict.get((field,str(spw),pol,'nchan')) is not None:
                                    %if info_dict[(field,str(spw),pol,'nchan')] == 1:
                                        <td rowspan="12">${field}</td>
                                        <td rowspan="12">${spw}</td>
                                        <td rowspan="12">${pol}</td>
                                    %else:
                                        <td rowspan="11">${field}</td>
                                        <td rowspan="11">${spw}</td>
                                        <td rowspan="11">${pol}</td>
                                    %endif
                                %else:
                                    <td rowspan="11">${field}</td>
                                    <td rowspan="11">${spw}</td>
                                    <td rowspan="11">${pol}</td>
                                %endif
                                %if info_dict.get((field,str(spw),pol,'nchan')) is not None:
                                    %if info_dict[(field,str(spw),pol,'nchan')] == 1:
								<th>center frequency of image</th>
                                    %else:
								<th>center frequency of cube</th>
                                    %endif
                                %else:
								<th>center frequency</th>
                                %endif
								<td>${casatools.quanta.tos(info_dict[(field,str(spw),pol,'frequency')], 4)} (LSRK)</td>
                                <% 
                                try:
                                    final_iter = sorted(plots_dict[field][str(spw)].keys())[-1]
                                    plot = get_plot(plots_dict, field, str(spw), final_iter, 'image') 
                                except:
                                    plot = None
                                %>
                                % if plot is not None:
                                    <%
                                    renderer = clean_renderer.TCleanPlotsRenderer(pcontext, result, plots_dict, field, str(spw), pol)
                                    with renderer.get_file() as fileobj:
                                        fileobj.write(renderer.render())

                                    fullsize_relpath = os.path.relpath(plot.abspath, pcontext.report_dir)
                                    thumbnail_relpath = os.path.relpath(plot.thumbnail, pcontext.report_dir)
                                    %>
                                    %if info_dict.get((field,str(spw),pol,'nchan')) is not None:
                                        %if info_dict[(field,str(spw),pol,'nchan')] == 1:
                                            <td rowspan="11">
                                        %else:
                                            <td rowspan="10">
                                        %endif
                                    %else:
                                        <td rowspan="10">
                                    %endif
										<a class="fancybox"
                                           href="${fullsize_relpath}"
                                           rel="clean-summary-images"
                                           title='<div class="pull-left">Iteration: ${final_iter}<br>
		                                          Spw: ${plot.parameters['spw']}<br>
		                                          Field: ${cgi.escape(field, True)}</div><div class="pull-right"><a href="${fullsize_relpath}">Full Size</a></div>'
                                           data-thumbnail="${thumbnail_relpath}">
										  <img src="${thumbnail_relpath}"
										       title="Iteration ${final_iter}: image"
										       alt="Iteration ${final_iter}: image"
										       class="img-thumbnail img-responsive"
                                               data-thumbnail="${thumbnail_relpath}">
										</a>
                                        <div class="caption">
                                            <p>
                                                <a class="replace"
                                                   href="${os.path.relpath(renderer.path, pcontext.report_dir)}"
                                                   role="button">
                                                    View other QA images...
                                                </a>
                                            </p>
                                        </div>
                                </td>
                                %else:
                                <td>No image available</td>
                                %endif
			    </tr>
                            <tr>
				<th>beam</th>
                                %if info_dict.get((field,str(spw),pol,'beam major')) is not None:
                                    <td>${'%#.3g x %#.3g %s' % (info_dict[(field,str(spw),pol,'beam major')]['value'], info_dict[(field,str(spw),pol,'beam minor')]['value'], info_dict[(field,str(spw),pol,'beam major')]['unit'])}</td>
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
                                <th>final theoretical sensitivity</th>
                                %if info_dict.get((field,str(spw),pol,'sensitivity')) is not None:
                                            <td>${'%.2g %s' % (info_dict[(field,str(spw),pol,'sensitivity')],
                                                info_dict[(field,str(spw),pol,'brightness unit')])}
                                    %if info_dict.get((field,str(spw),pol,'min sensitivity')) is not None and info_dict.get((field,str(spw),pol,'max sensitivity')) is not None:
                                            <br>${'min: %.2g %s (field: %s)' % (info_dict[(field,str(spw),pol,'min sensitivity')], info_dict[(field,str(spw),pol,'brightness unit')], info_dict[(field,str(spw),pol,'min field id')])}
                                            <br>${'max: %.2g %s (field: %s)' % (info_dict[(field,str(spw),pol,'max sensitivity')], info_dict[(field,str(spw),pol,'brightness unit')], info_dict[(field,str(spw),pol,'max field id')])}
                                    %endif
                                            </td>
                                %else:
                                            <td>-</td>
                                %endif
                            </tr>
                            <tr>
                                <th>cleaning threshold</th>
                                %if info_dict.get((field,str(spw),pol,'threshold')) is not None:
                                            <td>${'%.2g %s' % (casatools.quanta.convert(info_dict[(field,str(spw),pol,'threshold')], info_dict[(field,str(spw),pol,'brightness unit')])['value'],
                                                info_dict[(field,str(spw),pol,'brightness unit')])}
                                    %if info_dict.get((field,str(spw),pol,'dirty DR')) is not None:
                                            <br>${'Dirty DR: %.2g' % (info_dict[(field,str(spw),pol,'dirty DR')])}
                                        %if info_dict[(field,str(spw),pol,'maxEDR used')]:
                                            <br>${'DR correction: %.2g' % (info_dict[(field,str(spw),pol,'DR correction factor')])}
                                        %else:
                                            <br>${'DR correction: %.2g' % (info_dict[(field,str(spw),pol,'DR correction factor')])}
                                        %endif
                                    %else:
                                            <br>No DR information
                                    %endif
                                            </td>
                                %else:
                                            <td>-</td>
                                %endif
                            </tr>
                            <tr>
                                <th>clean residual peak / scaled MAD</th>
                                %if info_dict.get((field,str(spw),pol,'residual peak/rms')) is not None:
                                            <td>${'%.2f' % info_dict.get((field,str(spw),pol,'residual peak/rms'))}</td>
                                %else:
                                            <td>-</td>
                                %endif
                            </tr>
                            <tr>
                                %if info_dict.get((field,str(spw),pol,'nchan')) is not None:
                                    %if info_dict[(field,str(spw),pol,'nchan')] > 1:
                                        <th>non-pbcor image rms / rmsmin / rmsmax</th>
                                        %if info_dict.get((field,str(spw),pol,'masked rms')) is not None:
                                                    <td>${'%#.2g / %#.2g / %#.2g %s' % (info_dict[(field,str(spw),pol,'masked rms')],
                                                        info_dict[(field,str(spw),pol,'masked rms min')],
                                                        info_dict[(field,str(spw),pol,'masked rms max')],
                                                        info_dict[(field,str(spw),pol,'brightness unit')])}</td>
                                        %else:
                                                    <td>-</td>
                                        %endif
                                    %else:
                                        <th>non-pbcor image rms</th>
                                        %if info_dict.get((field,str(spw),pol,'masked rms')) is not None:
                                                    <td>${'%#.2g %s' % (info_dict[(field,str(spw),pol,'masked rms')],
                                                        info_dict[(field,str(spw),pol,'brightness unit')])}</td>
                                        %else:
                                                    <td>-</td>
                                        %endif
                                    %endif
                                %else:
                                        <th>No rms information</th>
                                            <td>-</td>
                                %endif
                            </tr>
                            <tr>
                                <th>pbcor image max / min </th>
                                %if info_dict.get((field,str(spw),pol,'non-masked max')) is not None and info_dict.get((field,str(spw),pol,'non-masked min')) is not None:
                                            <td>${'%#.3g / %#.3g %s' % (info_dict[(field,str(spw),pol,'non-masked max')], info_dict[(field,str(spw),pol,'non-masked min')],
                                                info_dict[(field,str(spw),pol,'brightness unit')])}</td>
                                %else:
                                            <td>-</td>
                                %endif
                            </tr>
                            <tr>
                                %if info_dict.get((field,str(spw),pol,'nchan')) is not None:
                                    %if info_dict[(field,str(spw),pol,'nchan')] > 1:
                                        <th>channels</th>
                                            <td>${'%d x %s' % (info_dict[(field,str(spw),pol,'nchan')],
                                                info_dict[(field,str(spw),pol,'width')])} (LSRK)</td>
                                    %else:
                                        <th>fractional bandwidth / nterms</th>
                                            <td>${'%s / %s' % (info_dict[(field,str(spw),pol,'fractional bandwidth')],
                                                info_dict[(field,str(spw),pol,'nterms')])}</td>
                                    %endif
                                %else:
                                        <th>No channel / width information</th>
                                            <td>-</td>
                                %endif
                            </tr>
                                %if info_dict.get((field,str(spw),pol,'nchan')) is not None:
                                    %if info_dict[(field,str(spw),pol,'nchan')] == 1:
                                        <tr>
                                        <th>aggregate bandwidth</th>
                                        %if info_dict.get((field,str(spw),pol,'aggregate bandwidth')) is not None:
                                            <td>${info_dict[(field,str(spw),pol,'aggregate bandwidth')]}</td>
                                            </tr>
                                        %else:
                                            <td>-</td>
                                            </tr>
                                        %endif
                                    %endif
                                %else:
                                        <tr>
                                        <th>No bandwidth information</th>
                                        <td>-</td>
                                        </tr>
                                %endif
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
                               <td colspan="2">${info_dict[(field,str(spw),pol,'image name')].replace('.pbcor','')}</td>
                            </tr>
                            %endif
                        %endfor
                    %endfor
                %endfor
                </tbody>
        </table>
%endif
