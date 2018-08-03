<%!
rsc_path = ""
import cgi
import decimal
import os
import string
import types

import pipeline.domain.measures as measures
import pipeline.infrastructure.renderer.htmlrenderer as hr
import pipeline.infrastructure.renderer.rendererutils as rendererutils
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.utils as utils

agent_description = {
	'before'   : 'Before',
	'applycal' : 'After',
}

total_keys = {
	'TOTAL'        : 'All Data',
	'SCIENCE SPWS' : 'Science Spectral Windows',
	'BANDPASS'     : 'Bandpass',
	'AMPLITUDE'    : 'Flux',
	'PHASE'        : 'Phase',
	'TARGET'       : 'Target'
}

def template_agent_header1(agent):
	span = 'col' if agent in ('online','template') else 'row'
	return '<th %sspan=2>%s</th>' % (span, agent_description[agent])

def template_agent_header2(agent):
	if agent in ('online', 'template'):
		return '<th>File</th><th>Number of Statements</th>'
	else:
		return ''		

def get_template_agents(agents):
	return [a for a in agents if a in ('online', 'template')]

def sanitise(url):
	return filenamer.sanitize(url)

def spws_for_baseband(plot):
	spws = plot.parameters['spw'].split(',')
	if not spws:
		return ''
	return '<h6 style="margin-top: -11px;">Spw%s</h6>' % utils.commafy(spws, quotes=False, multi_prefix='s')

def rx_for_plot(plot):
	rx = plot.parameters['receiver']
	if not rx:
		return ''
	rx_string = utils.commafy(rx, quotes=False)
	# Don't need receiver prefix for ALMA bands
	if 'ALMA' not in rx_string:
		prefix = 'Receiver bands: ' if len(rx) > 1 else 'Receiver band: '
	else:
		prefix = ''
	return '<h6 style="margin-top: -11px;">%s%s</h6>' % (prefix, rx_string)
%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">Phased-up fluxscale</%block>

<h2>Contents</h2>
<ul>
    <li>Tables:</li>
    <ul>
        %if adopted_table:
            <li><a href="#adopted">Measurement sets using adopted flux calibrations</a></li>
        %endif
        <li><a href="#antennas">Antennas used for flux scaling</a></li>
        <li><a href="#computed">Computed flux densities</a></li>
    </ul>
    <li>Plots:</li>
    <ul>
        %if flux_plots:
            <li><a href="#flux_vs_freq_comparison">Derived flux density vs catalogue flux density</a></li>
        %endif
        %if ampuv_allant_plots:
            <li><a href="#model">Flux calibrator model comparison</a></li>
        %endif
    </ul>
</ul>

<h2>Results</h2>

% if adopted_table:
<h3 id="adopted">Measurement sets using adopted flux calibrations</h3>
    <p>Measurement sets without flux calibrator scans adopt the appropriate flux calibrations from other measurement
        sets within the session. This is possible when the measurement sets with- and without-flux calibrator scans
        independently observe a common set of fields, such as a common phase calibrator. In these instances, the adopted
        flux calibration for a field is equal to the mean flux calibration for that field as derived from the
        measurement sets with a flux calibration. The precise values used for these data are listed in the 'Computed
        Flux Densities' section.</p>

    <p>The following measurement sets use a flux calibration adopted from other measurement sets.</p>

    <table class="table table-bordered table-striped"
           summary="Measurement sets with flux calibration adopted from other data">
        <caption>Measurement sets using a flux calibration adopted from other data</caption>
        <thead>
        <tr>
            <th scope="col">Measurement Set</th>
            <th scope="col">Fields using adopted flux calibration</th>
        </tr>
        </thead>
        <tbody>
            % for tr in adopted_table:
                <tr>
                    % for td in tr:
                ${td}
                    % endfor
                </tr>
            % endfor
        </tbody>
    </table>
% endif

<h3 id="antennas">Antennas Used for Flux Scaling</h3>

<p>The following antennas were used for flux scaling, entries for unresolved flux calibrators are blank</p>

<table class="table table-bordered table-striped" summary="Flux Scaling Antennas">
	<caption>Antennas for Flux Calibration</caption>
	<thead>
	    <tr>
	        <th scope="col">Measurement Set</th>
	        <th scope="col">UV Range</th>
	        <th scope="col">Antennas</th>
	    </tr>
	</thead>
	<tbody>
% for single_result in [r for r in result if not r.applies_adopted]:
		<tr>
			<td>${os.path.basename(single_result.vis)}</td>
                	<td>${single_result.uvrange}</td>
                	<td>${single_result.resantenna.replace(',', ', ').replace('&', '')}</td>
		</tr>
% endfor
	</tbody>
</table>


<h3 id="computed">Computed Flux Densities</h3>

<p>The following flux densities were set in the measurement set model column and recorded in the pipeline context:</p>

<table class="table table-bordered table-striped" summary="Flux density results">
	<caption>Phased-up Fluxscale Results</caption>
    <thead>
	    <tr>
	        <th scope="col" rowspan="3">Measurement Set</th>
	        <th scope="col" rowspan="3">Field</th>
	        <th scope="col" rowspan="3">Spw</th>
	        <th scope="col" rowspan="3">Frequency Bandwidth (TOPO)</th>
	        <th scope="col" colspan="4">Derived Flux Density</th>
	        <th scope="col" rowspan="3">Flux Ratio (Derived / Catalog)</th>
	        <th scope="col" rowspan="3">Spix</th>
		</tr>
		<tr>
	        <th scope="col" colspan="4">Catalog Flux Density</th>
	        </tr>
		<tr>
	        <th scope="col">I</th>
	        <th scope="col">Q</th>
	        <th scope="col">U</th>
	        <th scope="col">V</th>
	        </tr>
	</thead>
	<tbody>
	% for tr in table_rows:
		<tr>
		% for td in tr:
			${td}
		% endfor
		</tr>
	%endfor
	</tbody>
</table>


<%self:plot_group plot_dict="${flux_plots}"
                  url_fn="${lambda ms: None}"
				  title_id="flux_vs_freq_comparison">

	<%def name="title()">
		Derived flux density vs catalogue flux density
	</%def>

	<%def name="preamble()">
    <p>These plots show amplitude vs frequency for the non-AMPLITUDE calibrators in each measurement set, comparing the
        pipeline-derived flux density <i>S</i><sub>derived</sub> to the catalogue flux
        density<i>S</i><sub>catalogue</sub> reported by analysisUtils, online source catalogues, and/or recorded in the
        ASDM. In these plots, <i>S</i><sub>catalogue</sub> is extrapolated using the spectral index to cover the
        frequency range of the spectral windows.</p>

    <p>QA metrics are calculated by comparing the flux density ratio
        <i>K</i><sub>spw</sub>=<i>S</i><sub>derived</sub>/<i>S</i><sub>catalogue</sub> for each spectral window to the
        ratio for the highest SNR spectral window. This metric evaluates how consistent the relative flux calibration is
        from spectral window to spectral window for each calibrator; it does not evaluate whether the absolute flux
        calibration is reasonable as compared to the catalogue measurements. All QA scores based on this metric are
        included in the Pipeline QA section at the bottom of this page.</p>
    </%def>

	<%def name="mouseover(plot)">Click to show amplitude vs frequency for ${plot.parameters['vis']} ${utils.dequote(plot.parameters['field'])}</%def>

	<%def name="fancybox_caption(plot)">
		${plot.parameters['vis']}<br>
		Field: ${utils.dequote(plot.parameters['field'])}<br>
        Intents: ${utils.commafy(plot.parameters['intent'], False)}
	</%def>

    <%def name="caption_title(plot)">
		${plot.parameters['field']}<br>
	</%def>

    <%def name="caption_text(plot, ptype)">
        Intents: ${utils.commafy(plot.parameters['intent'], False)}
	</%def>
</%self:plot_group>


%if ampuv_allant_plots:
    <h3 id="model">Flux Calibrator Model Comparison</h3>
    Antenna selection used for flux transfer to the secondary calibrators.

	%for ms in ampuv_allant_plots:
	    <h4>${ms}</h4>
		% for intent in ampuv_allant_plots[ms]:
			<div class="row">
		        % for i, plot in enumerate(ampuv_allant_plots[ms][intent]):
		        	<!--  Select on antenna -->
		            <%
		              antplot = ampuv_ant_plots[ms][intent][i]
		            %>
		            <div class="col-md-3">
			            % if os.path.exists(plot.thumbnail):
			                <div class="thumbnail">
			                    <a href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
								   title='<div>Baseband ${plot.parameters["baseband"]} (spw ${plot.parameters["spw"]}).<br>
			                              Receiver bands: ${utils.commafy(plot.parameters["receiver"], False)}.<br>
			                              ${"All antennas." if plot.parameters.get("ant","") == "" else "Antennas: "+str(plot.parameters["ant"])+"."}<br>
			                              Flux calibrator fields: ${plot.parameters["field"]}.</div>'
			                       data-fancybox="amp_vs_uv-${ms}">
			                        <img class="lazyload"
                                         data-src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
			                             title="Click to show amplitude vs UV plot for Baseband ${plot.parameters['baseband']}">
			                    </a>
			                    <div class="caption">
									<h4>Baseband ${plot.parameters['baseband']}</h4>
									${rx_for_plot(plot)}
									${spws_for_baseband(plot)}

								    <p>Amp vs. uvdist for
								    <%
									antlist = plot.parameters.get('ant','').split(',')
									antdisp = ' '.join([','.join(antlist[i:i+4])+'<br>' for i in range(0,len(antlist),4)])
								    %>
								    ${'all antennas.' if plot.parameters.get('ant','') == '' else 'antennas: '+antdisp}
								    Color coded by spw.<br> Flux calibrator fields: ${plot.parameters['field']}.
								    </p>
								</div>
							</div>
			            % endif

			            % if os.path.exists(antplot.thumbnail):
			                <div class="thumbnail">
			                    <a href="${os.path.relpath(antplot.abspath, pcontext.report_dir)}"
								   title='<div class="pull-left">Baseband ${antplot.parameters["baseband"]} (spw ${antplot.parameters["spw"]}).<br>
			                              Receiver bands: ${utils.commafy(antplot.parameters["receiver"], False)}.<br>
			                              ${"All antennas." if antplot.parameters.get("ant","") == "" else "Antennas: "+str(antplot.parameters["ant"])+"."}<br>
			                              Flux calibrator fields: ${antplot.parameters["field"]}.</div>'
			                       data-fancybox="amp_vs_uv-${ms}">
			                        <img class="lazyload"
                                         data-src="${os.path.relpath(antplot.thumbnail, pcontext.report_dir)}"
			                             title="Click to show amplitude vs UV plot for Baseband ${antplot.parameters['baseband']}">
			                    </a>
			                    <div class="caption">
									<h4>Baseband ${antplot.parameters['baseband']}</h4>
									${rx_for_plot(antplot)}
									${spws_for_baseband(antplot)}

								    <p>Selection for
								    <%
									antlist = antplot.parameters.get('ant','').split(',')
									antdisp = ' '.join([','.join(antlist[i:i+4])+'<br>' for i in range(0,len(antlist),4)])
								    %>
								    ${' all antennas.' if antplot.parameters.get('ant','') == '' else ' antennas: '+antdisp}
								    </p>
								</div>
			                </div>
		            	% endif
		            </div>
		        % endfor
			</div>
		% endfor
	%endfor
%endif
