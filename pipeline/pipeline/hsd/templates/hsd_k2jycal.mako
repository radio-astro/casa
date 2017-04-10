<%!
rsc_path = "../"
import os

import pipeline.infrastructure.utils as utils
%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="header" />

<%block name="title">Generate Kelvin to Jansky conversion caltables</%block>

<%
stage_dir = os.path.join(pcontext.report_dir, 'stage%s'%(result.stage_number))
%>

<p>This task generates calibtation tables to convert the unit of single dish spectra from Kelvin to Jansky.</p>


<h3>Summay of Jy/K Conversion Factor</h3>
Numbers in histograms show that of MS, antenna, spectral window, and polarization
combination whose conversion factor is in each bin.
% for plot in jyperk_hist:
	% if plot is not None:
    <div class="col-md-3 col-sm-4">
    	% if os.path.exists(plot.thumbnail):
    		<%
            	fullsize_relpath = os.path.relpath(plot.abspath, pcontext.report_dir)
                thumbnail_relpath = os.path.relpath(plot.thumbnail, pcontext.report_dir)
            %>

            <div class="thumbnail">
            	<a href="${fullsize_relpath}"
                	class="fancybox"
                    title='<div class="pull-left">Receiver: ${utils.commafy(plot.parameters['receiver'], quotes=False)}<br>Spw: ${plot.parameters['spw']}<br></div><div class="pull-right"><a href="${fullsize_relpath}">Full Size</a></div>'
                    data-thumbnail="${thumbnail_relpath}">
                	<img src="${thumbnail_relpath}"
                        title="Click to show histrogram of Jy/K factors of spw ${plot.parameters['spw']}"
                      	data-thumbnail="${thumbnail_relpath}">
                </a>

                <div class="caption">
                    <!-- title -->
                    <h4>Spectral Window ${plot.parameters['spw']}</h4>
                    <!-- sub-title -->
	                <h6>${plot.parameters['receiver']}</h6>
                    <!-- description -->
                    <p>Variation of Jy/K factors in spw ${plot.parameters['spw']}</p>
                </div>
            </div>
        % endif
    </div>
    % endif
% endfor
<div class="clearfix"></div><!--  flush plots, break to next row -->

<h3>Jy/K Conversion Factors</h3>
The following table lists the Jy/K factor.
% if reffile is not None and len(reffile) > 0 and os.path.exists(os.path.join(stage_dir, os.path.basename(reffile))):
Input file is <a class="replace-pre" href="${os.path.relpath(reffile, pcontext.report_dir)}">${os.path.basename(reffile)}</a>.
% else:
No Jy/K factors file is specified. 
% endif
<table class="table table-bordered table-striped" summary="Jy/K factors">
    <thead>
	<tr><th>Spw</th><th>MS</th><th>Antenna</th><th>Pol</th><th>Factor</th></tr>
    </thead>
	<tbody>
	% for tr in jyperk_rows:
		<tr>
		% for td in tr:
			${td}
		% endfor
		</tr>
	%endfor
	</tbody>
</table>
<p/>