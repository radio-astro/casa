<%!
rsc_path = ""
import cgi
import os.path
import pipeline.infrastructure.casatools as casatools
import pipeline.hif.tasks.tclean.renderer as clean_renderer
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.renderer.rendererutils as rendererutils

columns = {
    'cleanmask' : 'Clean Mask',
    'flux' : 'Primary Beam',
    'image' : 'Image',
    'residual' : 'Residual',
    'model' : 'Final Model',
    'psf' : 'PSF'
}

colorder = ['image', 'residual', 'cleanmask']
%>
<%inherit file="t2-4m_details-base.mako"/>

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

            % for row in image_info:
                %if row.frequency is not None:
                <tr>
                    %if row.nchan is not None:
                        %if row.nchan == 1:
                            <td rowspan="12">${row.field}</td>
                            <td rowspan="12">${row.spw}</td>
                            <td rowspan="12">${row.pol}</td>
                        %else:
                            <td rowspan="11">${row.field}</td>
                            <td rowspan="11">${row.spw}</td>
                            <td rowspan="11">${row.pol}</td>
                        %endif
                    %else:
                        <td rowspan="11">${row.field}</td>
                        <td rowspan="11">${row.spw}</td>
                        <td rowspan="11">${row.pol}</td>
                    %endif
                    <th>${row.frequency_label}</th>
                    <td>${row.frequency}</td>
                    % if row.plot is not None:
                    <%
                    fullsize_relpath = os.path.relpath(row.plot.abspath, pcontext.report_dir)
                    thumbnail_relpath = os.path.relpath(row.plot.thumbnail, pcontext.report_dir)
                    %>
                    %if row.nchan == 1:
                    <td rowspan="11">
                    %else:
                    <td rowspan="10">
                    %endif
                        <a href="${fullsize_relpath}"
                           data-fancybox="clean-summary-images"
                           data-caption="Iteration: ${row.plot.parameters['iter']}<br>Spw: ${row.plot.parameters['spw']}<br>Field: ${cgi.escape(row.field, True)}"
                           title='<div class="pull-left">Iteration: ${row.plot.parameters['iter']}<br>
                                  Spw: ${row.plot.parameters["spw"]}<br>
                                  Field: ${cgi.escape(row.field, True)}</div><div class="pull-right"><a href="${fullsize_relpath}">Full Size</a></div>'>
                          <img class="lazyload"
                               data-src="${thumbnail_relpath}"
                               title="Iteration ${row.plot.parameters['iter']}: image"
                               alt="Iteration ${row.plot.parameters['iter']}: image"
                               class="img-thumbnail img-responsive">
                        </a>
                        <div class="caption">
                            <p>
                                <a class="replace"
                                   href="${os.path.relpath(row.qa_url, pcontext.report_dir)}"
                                   role="button">
                                    View other QA images...
                                </a>
                            </p>
                        </div>
                    </td>
                    % else:
                    <td>No image available</td>
                    % endif
			    </tr>

                <tr>
    				<th>beam</th>
                    <td>${row.beam}</td>
			    </tr>

                <tr>
                    <th>beam p.a.</th>
                    <td>${row.beam_pa}</td>
                </tr>

                <tr>
                    <th>final theoretical sensitivity</th>
                    <td>${row.sensitivity}</td>
                </tr>

                <tr>
                    <th>cleaning threshold</th>
                    <td>${row.cleaning_threshold}</td>
                </tr>

                <tr>
                    <th>clean residual peak / scaled MAD</th>
                    <td>${row.residual_ratio}</td>
                </tr>

                <tr>
                    <th>${row.non_pbcor_label}</th>
                    <td>${row.non_pbcor}</td>
                </tr>

                <tr>
                    <th>pbcor image max / min</th>
                    <td>${row.pbcor}</td>
                </tr>

                <tr>
                    <th>${row.fractional_bw_label}</th>
                    <td>${row.fractional_bw}</td>
                </tr>

                % if row.aggregate_bw_label is not None:
                <tr>
                    <th>${row.aggregate_bw_label}</th>
                    <td>${row.aggregate_bw}</td>
                </tr>
                % endif

                <tr>
                    <th>score</th>
                    <td>${row.score}</td>
                </tr>

                <tr>
                   <th>image file</th>
                   <td colspan="2">${row.image_file}</td>
                </tr>
            %endif
        %endfor
        </tbody>
    </table>
%endif
