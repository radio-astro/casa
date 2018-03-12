<%!
rsc_path=''
import os
import pipeline.infrastructure.utils as utils

%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">T<sub>sys</sub> Calibration</%block>

<p>This task generates a T<sub>sys</sub> calibration table, mapping each
science spectral window to the T<sub>sys</sub> window that overlaps in 
frequency.</p>

<h2>T<sub>sys</sub> window mapping</h2>
<table class="table table-bordered table-striped">
	<caption>Mapping of T<sub>sys</sub> window to science window</caption>
	<thead>
		<tr>
			<th>Measurement Set</th>
			<th>T<sub>sys</sub> window</th>
			<th>Science windows</th>
		</tr>
	</thead>
	<tbody>
	% for tr in tsysmap:
		<tr>
		% for td in tr:
			${td}
		% endfor
		</tr>
	% endfor
	</tbody>
</table>


<h2>Plots</h2>

<%self:plot_group plot_dict="${summary_plots}"
				  url_fn="${lambda x: summary_subpage[x]}"
				  data_tsysspw="${True}"
                  data_vis="${True}">

	<%def name="title()">
		T<sub>sys</sub> vs frequency
	</%def>

	<%def name="preamble()">
		<p>Plots of time-averaged T<sub>sys</sub> vs frequency, colored by antenna.</p>
	</%def>

	<%def name="mouseover(plot)">Click to show Tsys vs frequency for Tsys spw ${plot.parameters['tsys_spw']}</%def>

	<%def name="fancybox_caption(plot)">T<sub>sys</sub> spw: ${plot.parameters['tsys_spw']}<br/>Science spws: ${', '.join([str(i) for i in plot.parameters['spw']])}</%def>

	<%def name="caption_title(plot)">
		T<sub>sys</sub> spw ${plot.parameters['tsys_spw']}
	</%def>

	<%def name="caption_text(plot, _)">
		Science spw${utils.commafy(plot.parameters['spw'], quotes=False, multi_prefix='s')}.
	</%def>

</%self:plot_group>

