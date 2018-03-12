<%!
rsc_path = ""
import os
import pipeline.infrastructure.utils as utils

# method to output flagging percentages neatly
def percent_flagged(flagsummary):
    flagged = flagsummary.flagged
    total = flagsummary.total

    if total is 0:
        return 'N/A'
    else:
        return '%0.1f%%' % (100.0 * flagged / total)

# method to report number of lines in a file.
def num_lines(report_dir, relpath):
	abspath = os.path.join(report_dir, relpath)
	if os.path.exists(abspath):
		return sum(1 for line in open(abspath) if not line.startswith('#'))
	else:
		return 'N/A'

comp_descriptions = {'nmedian'    : 'Flag T<sub>sys</sub> spectra with high median values.',
                 	 'derivative' : 'Flag T<sub>sys</sub> spectra with high median derivative (ringing).',
                 	 'fieldshape' : 'Flag T<sub>sys</sub> spectra whose shape differs from those associated with BANDPASS data.',
                 	 'edgechans'  : 'Flag edge channels of T<sub>sys</sub> spectra.',
                 	 'birdies'    : 'Flag spikes or birdies in T<sub>sys</sub> spectra.',
                 	 'toomany'    : 'Flag T<sub>sys</sub> spectra with too many flagged timestamps / antennas.'}

std_plot_desc = {'nmedian'    : 'shows the images used to flag',
                 'derivative' : 'shows the images used to flag',
                 'fieldshape' : 'shows the images used to flag',
                 'edgechans'  : 'shows the views used to flag',
                 'birdies'    : 'shows the views used to flag',
                 'toomany'    : 'shows the views used to flag'}

extra_plot_desc = {'nmedian'    : ' shows the spectra flagged in',
     	   		   'derivative' : ' shows the spectra flagged in',
            	   'fieldshape' : ' shows the spectra flagged in'}

%>

<%inherit file="t2-4m_details-base.mako"/>
<%block name="title">Flag T<sub>sys</sub> calibration</%block>

<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>

% if any([msg for msg in task_incomplete_msg.values()]):
  <h2>Error report</h2>
  
  <p>For the following measurement sets, the Tsysflag task ended prematurely with the following error message:</p>
  <ul>
  % for vis, msg in task_incomplete_msg.items():
  	<li>${os.path.basename(vis)} :<br>
  	${msg}</li>
  % endfor
  </ul>
% endif

<h2>Contents</h2>
<ul>
% if updated_refants:
    <li><a href="#refants">Reference antenna update</a></li>
% endif
<li><a href="#plots">T<sub>sys</sub> after flagging</a></li>
<li><a href="#summarytable">Flagged data summary</a></li>
<li>Flag step details</li>
    <ul>
    % for component in components:
      % if htmlreports.get(component) is not None:
        <li><a href="#${component}">${component}</a></li>
      % endif
    % endfor
    </ul>
</ul>

% if updated_refants:
<h2 id="refants" class="jumptarget">Reference Antenna update</h2>

<p>For the measurement set(s) listed below, the reference antenna
    list was updated due to significant flagging (antennas moved to
    end and/or removed). See warnings in task notifications
    for details. Shown below are the updated reference antenna lists,
    only for those measurement sets where it was modified.</p>

<table class="table table-bordered table-striped"
	   summary="Reference Antennas">
	<caption>Updated reference antenna selection per measurement set. Antennas are
	listed in order of highest to lowest priority.</caption>
	<thead>
		<tr>
			<th>Measurement Set</th>
			<th>Reference Antennas (Highest to Lowest)</th>
		</tr>
	</thead>
	<tbody>
%for vis in updated_refants:
		<tr>
			<td>${os.path.basename(vis)}</td>
			## insert spaces in refant list to allow browser to break string
			## if it wants
			<td>${updated_refants[vis].replace(',', ', ')}</td>
		</tr>
%endfor
	</tbody>
</table>
% endif

<%self:plot_group plot_dict="${summary_plots}"
				  url_fn="${lambda x: summary_subpage[x]}"
				  data_tsysspw="${True}"
                  data_vis="${True}"
                  title_id="plots">

	<%def name="title()">
		T<sub>sys</sub> vs frequency after flagging
	</%def>

	<%def name="preamble()">
		<p>Plots of time-averaged T<sub>sys</sub> vs frequency, colored by antenna.</p>
	</%def>

	<%def name="mouseover(plot)">Click to show Tsys vs frequency for Tsys spw ${plot.parameters['tsys_spw']}</%def>

	<%def name="fancybox_caption(plot)">
		T<sub>sys</sub> spw: ${plot.parameters['tsys_spw']}<br/>
		Science spws: ${', '.join([str(i) for i in plot.parameters['spw']])}
	</%def>

	<%def name="caption_title(plot)">
		T<sub>sys</sub> spw ${plot.parameters['tsys_spw']}
	</%def>

	<%def name="caption_text(plot, _)">
		Science spw${utils.commafy(plot.parameters['spw'], quotes=False, multi_prefix='s')}.
	</%def>

</%self:plot_group>

<h2 id="summarytable" class="jumptarget">Flagging steps</h2>
<table class="table table-bordered table-striped">
	<thead>
		<tr>
			<th>Measurement Set</th>
			% for step in components:
			<th>${step}</th>
			% endfor
		</tr>                           
	</thead>
	<tbody>
	% for ms in flags.keys():
		<tr>
			<td>${ms}</td>
			% for step in components:
			% if flags[ms].get(step) is None:
			<td><span class="glyphicon glyphicon-remove"></span></td>
      		% else:
      		<td><span class="glyphicon glyphicon-ok"></span></td>
      		% endif
  			% endfor
		</tr>
	% endfor
	</tbody>
</table>

<h2>Flagged data summary</h2>

% for ms in flags.keys():
<h4>Table: ${ms}</h4>
<table class="table table-bordered table-striped ">
	<caption>Summary of flagged data. Each cell states the amount of data 
		flagged as a fraction of the specified data selection, with the 
		<em>Flagging Step</em> columns giving this information per flagging
		step. Note: for each data selection intent, the flagging statistics
        are calculated for the T<sub>sys</sub> scans (with intent=ATMOSPHERE)
        that cover those fields that also match the data selection intent. A
        value of "N/A" in a row means that no T<sub>sys</sub> scan was acquired
        on the object(s) observed for the corresponding intent.
	</caption>
	<thead>
		<tr>
			<th rowspan="2">Data Selection</th>
			<!-- flags before task is always first agent -->
			<th rowspan="2">flagged before</th>
			<th colspan="${len(components)}">Flagging Step</th>
			<th rowspan="2">flagged after</th>
		</tr>
		<tr>
			% for step in components:
			<th>${step}</th>
			% endfor
		</tr>
	</thead>
	<tbody>
		% for k in ['TOTAL', 'BANDPASS', 'AMPLITUDE', 'PHASE', 'TARGET','ATMOSPHERE']: 
		<tr>
			<th>${k}</th>               
			% for step in ['before'] + components + ['after']:
			% if flags[ms].get(step) is not None:
				##<td>${step} ${k} ${flags[ms][step]['Summary'][k]}</td>
				<td>${percent_flagged(flags[ms][step]['Summary'][k])}</td>
			% else:
				<td>0.0%</td>
			% endif
			% endfor
		</tr>
		% endfor
	</tbody>
</table>

% endfor

<h2>Flag Step Details</h2>
<p>
    The following section provides plots showing the flagging metrics that the
    pipeline uses to determine deviant T<sub>sys</sub> measurements, and the
    flagging commands that resulted from each flagging metric. For certain
    flagging metrics, the pipeline evaluates the metric separately for each
    polarisation. However, if the T<sub>sys</sub> measurement for an antenna is
    found to be deviant in one polarisation, the pipeline will flag the antenna
    for both polarisations.
</p>

% for component in components: 
  % if htmlreports.get(component) is not None:
	<h3 id="${component}" class="jumptarget">${component}</h3>
	${comp_descriptions[component]}

    % if component in stdplots:
	<h4>Plots</h4>
	<ul>
		% for vis, (path, shorttitle) in stdplots[component].items():
		<li><a class="replace" data-vis="${vis}" href="${os.path.relpath(path, pcontext.report_dir)}">${shorttitle}</a> ${std_plot_desc[component]} ${vis}.</li>
		% endfor
	    % if component in extraplots:
			% for vis, (path, shorttitle) in extraplots[component].items():
			<li><a class="replace" data-vis="${vis}" href="${os.path.relpath(path, pcontext.report_dir)}">${shorttitle}</a> ${extra_plot_desc[component]} ${vis}.</li>
			% endfor
		% endif

	</ul>
    % endif

    <h4>Flags</h4>
    <table class="table table-bordered table-striped">
	<thead>
	    <tr>
	    	<th>Table</th>
	        <th>Flagging Commands</th>
	        <th>Number of Statements</th>
	        <th>Flagging Report</th>
	    </tr>
	</thead>
	<tbody>
	    % for file, relpath_reports in htmlreports[component].items():
	    <tr>
	    	<td>${file}</td>
	        <td><a class="replace-pre" href="${relpath_reports[0]}" 
                   data-title="Flagging Commands">Flag commands file</a></td>
            <td>${num_lines(pcontext.report_dir, relpath_reports[0])}</td>
	        <td><a class="replace-pre"
                   href="${relpath_reports[1]}" data-title="Flagging report">
                   Flagging report</a></td>
	    </tr>
	    % endfor
	</tbody>
    </table>
  % endif
% endfor