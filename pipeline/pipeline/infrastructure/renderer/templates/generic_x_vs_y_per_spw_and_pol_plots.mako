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
	var scores_dict = $('#scores').data('scores');	

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
    filterPipeline.addFilter(FILTERS.createMatchFilter('pol', '#select-pol'));

    // add on-click handler to our thumbnails to launch FancyBox with the
    // relevant thumbnails
    $("ul.thumbnails li div a").click(function (evt) {
        evt.preventDefault();
        var target = this.href;
        UTILS.launchFancybox(target);
        return false;
    });

    // fix the thumbnail margins for plots on the n>1 row
    UTILS.fixThumbnailMargins();
});
</script>

<div data-scores="${json}" id="scores"></div>

<div class="page-header">
	<h1>${plot_title}<button class="btn btn-large pull-right" onClick="javascript:location.reload();">Back</button></h1>
</div>

<div class="column-fluid">
	<div class="row-fluid">
		<div class="column-fluid span6">
			<div>
			<fieldset>
				<legend>Spectral Window Filter</legend>
				<select id="select-spw" class="select2" multiple style="width:100%" placeholder="Show all spectral windows">
					% for spw in sorted(list(set([int(p.parameters['spw']) for p in plots]))):
					<option>${spw}</option>
					% endfor
		       	</select>
			</fieldset>
			</div>
		</div>		
	
		<div class="column-fluid span6">
			<div>
			<fieldset>
				<legend>Polarisation Filter</legend>
				<select id="select-pol" class="select2" multiple style="width:100%" placeholder="Show all polarisations">
					% for pol in sorted(list(set([p.parameters['pol'] for p in plots]))):
					<option>${pol}</option>
					% endfor
		       	</select>
			</fieldset>
			</div>
		</div>		
	</div>
</div>

<br>

<div class="column-fluid">
	<ul class="thumbnails">
	% for plot in sorted(plots, key=lambda p: p.parameters['spw']):
		<li class="span2">
			<div class="thumbnail">
				<a class="fancybox"
				   href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
				   title="Spw ${plot.parameters['spw']} ${plot.parameters['pol']}"
				   data-thumbnail="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}">
					<img   src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
						 title="Spw ${plot.parameters['spw']} ${plot.parameters['pol']}"
						   alt="">
					</img>
				</a>
					<p class="text-center">Spw ${plot.parameters['spw']}
					${plot.parameters['pol']}</p>
			</div>
		</li>
	% endfor
	</ul>
</div>
