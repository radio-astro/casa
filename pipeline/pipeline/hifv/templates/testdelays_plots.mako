<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>

<link href="${self.attr.rsc_path}resources/css/select2.css" rel="stylesheet"/>
<link href="${self.attr.rsc_path}resources/css/select2-bootstrap.css" rel="stylesheet"/>
<script src="${self.attr.rsc_path}resources/js/select2.min.js"></script>

<!-- include required files and styles for histograms -->
<script src="${self.attr.rsc_path}resources/js/d3.v3.min.js"></script>
<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>

<script>
$(document).ready(function () {
    // push JSON directly into page, avoiding XHR cross-site domain problems
	var json='${json}';
	var scores_dict = JSON && JSON.parse(json) || $.parseJSON(json);

    // activate the input fields for spw, antenna, etc.
    $('.select2').select2();

    // create a new filter pipeline, and tell it to filter based on the scores
    // in the scores_dict JSON dictionary that is set in the template.
    var filterPipeline = new FILTERS.FilterPipeline();
    filterPipeline.setScores(scores_dict);

    // create filters that listen for events on the .select2 input fields we just
    // created, and add them to the filter pipeline, filtering on the appropriate
    // dictionary key
    filterPipeline.addFilter(FILTERS.createMatchFilter('spw', '#select-spw'));
    filterPipeline.addFilter(FILTERS.createMatchFilter('antenna', '#select-ant'));

    // get the X-axis label for Tsys
    var xAxis = PLOTS.xAxisLabels["K"];

    // create histograms and histogram filters for the three distributions we want
    // to highlight: RMS, average median, and maximum median reached.
    var charts = [ALL_IN_ONE.easyHistogram(filterPipeline, scores_dict, "median", "#histogram-tsysmedian", xAxis),
    			  ALL_IN_ONE.easyHistogram(filterPipeline, scores_dict, "median_max", "#histogram-tsysmedianmax", xAxis),
				  ALL_IN_ONE.easyHistogram(filterPipeline, scores_dict, "rms", "#histogram-tsysrms", xAxis)]

    // link histogram ranges to the range checkbox
    var rangeCheckbox = $("input#rangeCheckbox");
	rangeCheckbox.click(function() {
		var state = rangeCheckbox.prop("checked");
		charts.forEach(function(chart) {
			chart.histogram.duration(1000).plotExtent(state);
		});
	});
    
    // add on-click handler to our thumbnails to launch FancyBox with the
    // relevant thumbnails
    $("div.thumbnail a").click(function (evt) {
        evt.preventDefault();
        var target = this.href;
        UTILS.launchFancybox(target);
        return false;
    });
});
</script>

<div class="page-header">
	<h1>Test Delay Plots<button class="btn btn-default pull-right" onClick="javascript:window.history.back();">Back</button></h1>
</div>

<br>
% for ms in testdelay_subpages.keys():
    <h4>Plots:  <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, testdelay_subpages[ms]), pcontext.report_dir)}">Test delay plots </a>|
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, ampgain_subpages[ms]), pcontext.report_dir)}">Gain Amplitude </a>|
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, phasegain_subpages[ms]), pcontext.report_dir)}">Gain Phase </a>|
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, bpsolamp_subpages[ms]), pcontext.report_dir)}">BP Amp solution </a>|
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, bpsolphase_subpages[ms]), pcontext.report_dir)}">BP Phase solution </a>
    </h4>
%endfor

<br>

% for plot in sorted(plots, key=lambda p: p.parameters['ant']):
<div class="col-md-2 col-sm-3">
		<div class="thumbnail">
			<a class="fancybox"
				   href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
				   title="Antenna ${plot.parameters['ant']} "
				   data-thumbnail="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}">
					<img   src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
						 title="Antenna ${plot.parameters['ant']} "
						   alt="">
			</a>
			<div class="caption">
				<span class="text-center">Antenna ${plot.parameters['ant']}</span>
			</div>
	</div>
</div>
% endfor
