<%doc>
Base template for detail plots.

Override SELECTORS with the plot parameters that selectors should be created
for, e.g.

    SELECTORS = ['spw', 'ant'] 

Use HISTOGRAM_LABELS and HISTOGRAM_AXES to automatically create histograms for
your plots. The key for each tuple in HISTOGRAM_LABELS and HISTOGRAM_AXES
should be the plot parameter containing the score. The corresponding values in
HISTOGRAM_LABELS and HISTOGRAM_AXES should be the label for that score and the
javascript defining the X Axis label to use for that score, respectively. For
example:

	HISTOGRAM_LABELS = collections.OrderedDict([
		('ratio', 'Ratio of phase RMS before/after WVR application')
	])	
	HISTOGRAM_AXES = collections.OrderedDict([
		('ratio', 'PLOTS.xAxisLabels["WVR phase RMS"]')
	])

This will create histograms using the value held in plot.parameters['ratio'],
assigning an x-axis label of 'WVR Phase RMS'. You may want to use OrderedDicts
so that the plot panels are ordered consistently.
</%doc>

<%!
rsc_path = ""
import os

SELECTORS = []
HISTOGRAM_LABELS = {}
HISTOGRAM_AXES = {}

SELECT2_LABEL = {'spw' : 'Spectral window filter',
				 'ant' : 'Antenna filter',
				 'pol' : 'Polarisation filter',
				 'field' : 'Field filter',
				 'scan' : 'Scan filter',
				 'baseband' : 'Baseband filter',
				 'tsys_spw' : 'T<sub>sys</sub> window filter',
				 'intent' : 'Observing intent filter',
				 'type' : 'Type filter'}
				 
SELECT2_PLACEHOLDER = {'spw' : 'Show all spectral windows',
				       'ant' : 'Show all antennas',
				 	   'pol' : 'Show all polarisations',
				       'field' : 'Show all fields',
				       'scan' : 'Show all scans',
				       'baseband' : 'Show all basebands',
				       'tsys_spw' : 'Show all Tsys windows',
				       'intent' : 'Show all observing intents',
				       'type' : 'Show all types'}

def get_options(selector, plots):
	return format_options([p.parameters[selector] for p in plots])

def format_options(options):
	# remove any duplicates
	no_dups = set(options)
	# sort options
	sorted_options = sorted(list(no_dups))
	# return HTML element for each option
	return ['<option>%s</option>' % option for option in sorted_options]
%>

<link href="${self.attr.rsc_path}resources/css/select2.css" rel="stylesheet"/>
<link href="${self.attr.rsc_path}resources/css/select2-bootstrap.css" rel="stylesheet"/>
<script src="${self.attr.rsc_path}resources/js/select2.min.js"></script>

<!-- include required files and styles for histograms -->
<script src="${self.attr.rsc_path}resources/js/d3.min.js"></script>
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
% for selector in self.attr.SELECTORS:
    filterPipeline.addFilter(FILTERS.createMatchFilter('${selector}', '#select-${selector}'));
% endfor

% if self.attr.HISTOGRAM_AXES:
	var charts = [
	% for (key, axis) in self.attr.HISTOGRAM_AXES.items():	
				  ALL_IN_ONE.easyHistogram(filterPipeline, scores_dict, "${key}", "#histogram-${key}", ${axis}),
	% endfor
	             ];

	// link histogram ranges to the range checkbox
	var rangeCheckbox = $("input#rangeCheckbox");
	rangeCheckbox.click(function() {
		var state = rangeCheckbox.prop("checked");
		charts.forEach(function(chart) {
			chart.histogram.duration(1000).plotExtent(state);
		});
	});
% endif

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

<div data-scores="${json}" id="scores"></div>

<%def name="render_selectors(selectors)">
	% if len(selectors) > 0:
		<div class="row">
		% for selector in selectors:
			<div class="col-md-${str(12/len(selectors))}">
				<div class="form-group">
					<label for="select-${selector}" class="control-label">${SELECT2_LABEL[selector]}</label>
					<select id="select-${selector}" class="form-control select2" multiple data-placeholder="${SELECT2_PLACEHOLDER[selector]}">
						% for option in get_options(selector, plots):
						<option>${option}</option>
						% endfor
				    </select>			
				</div>
			</div>
		% endfor
		</div>
	% endif
</%def>

<%def name="render_plots(plots)">
	% for plot in plots:
		% if os.path.exists(plot.thumbnail):
			<div class="col-md-2 col-sm-3">
			    <%
			    	fullsize_relpath = os.path.relpath(plot.abspath, pcontext.report_dir)
			    	thumbnail_relpath = os.path.relpath(plot.thumbnail, pcontext.report_dir)
			    %>
				<div class="thumbnail">
					<a href="${fullsize_relpath}"
					   class="fancybox"
					   % if hasattr(caller, 'fancybox_caption'):
					   title="${caller.fancybox_caption(plot)}"
					   % endif
					   data-thumbnail="${thumbnail_relpath}">
						<img src="${thumbnail_relpath}"
							 % if hasattr(caller, 'mouseover'):
					   		 title="${caller.mouseover(plot)}"
					   		 % endif
					   		 data-thumbnail="${thumbnail_relpath}">
					</a>
			
					<div class="caption">
					% if hasattr(caller, 'caption_title'):
						<h4>${caller.caption_title(plot)}</h4>
					% endif
					% if hasattr(caller, 'caption_subtitle'):
						<h6>${caller.caption_subtitle(plot)}</h6>								
					% endif
					% if hasattr(caller, 'caption_text'):		
					    ${caller.caption_text(plot)}
					% endif
					</div>
				</div>		
			</div>
		% endif
	% endfor
</%def>

<div class="page-header">
	<h1><%block name="title">${plot_title}</%block><button class="btn btn-default pull-right" onClick="javascript:location.reload();">Back</button></h1>
</div>

% if self.attr.HISTOGRAM_AXES:
	<div class="row">
		<div class="col-md-12">
			<div class="form-group">
				<label for="rangeCheckbox" class="control-label">Clip histogram range to match data range</label>
				<input type="checkbox" id="rangeCheckbox" checked></input>
			</div>
		</div>
	</div>
	
	<div class="row">
		% for histogram_id, label in self.attr.HISTOGRAM_LABELS.items():
		<div class="col-md-${12/len(self.attr.HISTOGRAM_LABELS)}">
			<div class="panel panel-default">
				<div class="panel-heading">
					<h5 class="panel-title">${label}</h5>
				</div>
				<div class="panel-body">
					<div id="histogram-${histogram_id}"></div>
				</div>
			</div>
		</div>
		% endfor
	</div>
% endif


<%self:render_selectors selectors="${self.attr.SELECTORS}">
</%self:render_selectors>

<br>

${next.body()}