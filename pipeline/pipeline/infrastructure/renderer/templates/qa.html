<%!
rsc_path = "../"
css_file = "qa.css"
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="base.html"/>
<%block name="head">
	${parent.head()}

	<!-- Add jQueryUI accordion -->
	<script src="http://d3js.org/d3.v2.min.js?2.10.0"></script>
	<script src="${self.attr.rsc_path}resources/qa.js"></script>

	<!-- push JSON directly into page, avoiding XHR cross-site domain problems -->
	<script> 
		var json='${json}';
		scorehash = JSON && JSON.parse(json) || $.parseJSON(json);
	</script>
</%block>
<%block name="header" />

<%block name="title">Stage ${hr.get_stage_number(result)} ${plot_group.title} plots</%block>

<input id="back" type="button" name="back" value="Back" onClick="javascript:history.back();">
<h1>QA ${plot_group.title} Plots</h1>

<div id="scores_histogram">
    <h3>QA Total Score Histogram</h3>
    <div class="chart"></div>
</div>
<p>Click and drag in the chart above to filter by score.</p>

<div class="thumbnails">
	<ul class="thumb">
% for plot in sorted(plot_group.plots, key=lambda p: p.qa_score['total']):
		<li class="${plot.css_class}">
			<a rel="fancybox-thumb" href="${os.path.relpath(plot.abspath, os.path.commonprefix([plot.abspath,dirname]))}" title="${plot.parameters['ant']} spw ${plot.parameters['spw']}: QA total: ${plot.qa_score['total']}">
				<img   src="${os.path.relpath(plot.thumbnail, dirname)}"
					 title="${plot.parameters['ant']} spw ${plot.parameters['spw']}: QA total: ${plot.qa_score['total']}"
					   alt="${plot.parameters['ant']} spw ${plot.parameters['spw']}: QA total: ${plot.qa_score['total']}">
				</img>
			</a>
		</li>
% endfor
	</ul>
</div>
