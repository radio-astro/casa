<%!
rsc_path = ""
import os
%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="header" />

<%block name="title">Generate Baseline tables and subtract spectral baseline</%block>

<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>

<script>
$(document).ready(function() {
    // return a function that sets the SPW text field to the given spw
    var createSpwSetter = function(spw) {
        return function() {
        	if (typeof spw !== "undefined") {
	            // trigger a change event, otherwise the filters are not changed
	            $("#select-spw").select2("val", [spw]).trigger("change");
        	}
        };
    };

    // create a callback function for each overview plot that will select the
    // appropriate spw once the page has loaded
    $(".thumbnail a").each(function (i, v) {
        var o = $(v);
        var spw = o.data("spw");
        o.data("callback", createSpwSetter(spw));
    });

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
        }
    });
});
</script>


<%
try:
   pass
except Exception, e:
   print 'hsd_baseline html template exception:', e
   raise e
%>

<!-- short description of what the task does -->
<p>This task generates baseline fitting tables and subtracts baseline from spectra. 
Spectral lines are detected by clustering analysis for each spectral window, 
and line free channels are used for baseline fitting. </p>

<p>Spectral data before and after baseline subtraction are shown below. 
The topmost panel in each plot is an integrated spectrum while a set of bottom panels shows a profile map. 
Each panel in the profile map shows an individual spectrum that is located around panel's position. 
Cyan regions shows detected line regions. The line regions in the top panel indicates a property of detected 
cluster while the ones in the profile map are the result of line detection for the corresponding spectra.</p>

<h2>Spectral Data Before Baseline Subtraction</h2>

<p>Red lines indicate the result of baseline fit that is subtracted from the calibrated spectra.</p>

% for field in sparsemap_subpage_before.keys():
    <h4><a class="replace"
           href="${os.path.join(dirname, sparsemap_subpage_before[field])}">${field}</a>
    </h4>
    % for plot in sparsemap_before[field]:
        % if os.path.exists(plot.thumbnail):
	        <div class="col-md-3">
	            <div class="thumbnail">
	                <a href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
	                   class="fancybox"
	                   rel="thumbs">
	                    <img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
	                         title="Sparse Profile Map for Spectral Window ${plot.parameters['spw']} before Baseline Subtraction"
	                         data-thumbnail="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}">
	                </a>
	
	                <div class="caption">
	                    <h4>
	                        <a href="${os.path.join(dirname, sparsemap_subpage_before[field])}"
	                           class="replace"
	                           data-spw="${plot.parameters['spw']}"
	                           data-ant="${plot.parameters['ant']}">
	                           Spectral Window ${plot.parameters['spw']}
	                        </a>
	                    </h4>
	
	                    <p>Profile map for spectral
	                        window ${plot.parameters['spw']} before baseline subtraction.
	                    </p>
	                </div>
	            </div>
	        </div>
        % endif
    % endfor
	<div class="clearfix"></div><!--  flush plots, break to next row -->
%endfor


<h2>Spectral Data After Baseline Subtraction</h2>

<p>Red lines show zero-level. Spectra that are properly subtracted should be located around red lines.</p>

% for field in sparsemap_subpage_after.keys():
    <h4><a class="replace"
           href="${os.path.join(dirname, sparsemap_subpage_after[field])}">${field}</a>
    </h4>
    % for plot in sparsemap_after[field]:
        % if os.path.exists(plot.thumbnail):
	        <div class="col-md-3">
	            <div class="thumbnail">
	                <a href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
	                   class="fancybox"
	                   rel="thumbs">
	                    <img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
	                         title="Sparse Profile Map for Spectral Window ${plot.parameters['spw']} after Baseline Subtraction"
	                         data-thumbnail="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}">
	                </a>
	
	                <div class="caption">
	                    <h4>
	                        <a href="${os.path.join(dirname, sparsemap_subpage_after[field])}"
	                           class="replace"
	                           data-spw="${plot.parameters['spw']}"
	                           data-ant="${plot.parameters['ant']}">
	                           Spectral Window ${plot.parameters['spw']}
	                        </a>
	                    </h4>
	
	                    <p>Profile map for spectral
	                        window ${plot.parameters['spw']} after baseline subtraction.
	                    </p>
	                </div>
	            </div>
	        </div>
        % endif
    % endfor
	<div class="clearfix"></div><!--  flush plots, break to next row -->
%endfor

<h2>Line Detection by Clustering Analysis</h2>

% if len(detail) > 0 or len(cover_only) > 0:

<!-- Link to details page -->
% for plots in detail:
    <h3><a class="replace"
    href="${os.path.join(dirname, plots['html'])}">${plots['title']}</h3>
    
<!--		href="${os.path.relpath(os.path.join(dirname, plots['html']), pcontext.report_dir)}">${plots['title']}-->
    % for plot in plots['cover_plots']:
        % if os.path.exists(plot.thumbnail):
			<div class="col-md-3">
			  	<div class="thumbnail">
                    <a href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
                       class="fancybox"
                       rel="thumbs">
                       <img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
                             title="${plots['title']} for Spectral Window ${plot.parameters['spw']}"
                             data-thumbnail="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}">
                    </a>
                    <div class="caption">
                        <h4>
                            <a href="${os.path.join(dirname, plots['html'])}"
                               class="replace"
                               data-spw="${plot.parameters['spw']}">
                               Spectral Window ${plot.parameters['spw']}
                            </a>
                        </h4>
                        <p>Clustering plot of spectral
                            window ${plot.parameters['spw']}.
                        </p>
                    </div>
                </div>
           	</div>
        % endif
    % endfor
	<div class="clearfix"></div><!--  flush plots, break to next row -->
% endfor

<!-- No details -->
% for plots in cover_only:
    <h3>${plots['title']}</h3>
    % for plot in plots['cover_plots']:
		% if os.path.exists(plot.thumbnail):
			<div class="col-md-3">
			  	<div class="thumbnail">
                    <a href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
                       class="fancybox"
                       rel="thumbs">
                       <img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
                             title="${plots['title']} for Spectral Window ${plot.parameters['spw']}"
                             data-thumbnail="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}">
                    </a>
					<div class="caption">
						<h4>Spectral Window ${plot.parameters['spw']}</h4>
						<p>Clustering plot of spectral window 
						${plot.parameters['spw']}.</p>
					</div>
				</div>
			</div>
        % endif
    % endfor
	<div class="clearfix"></div><!--  flush plots, break to next row -->
% endfor

% else:
<p>No Line detected</p>
% endif

