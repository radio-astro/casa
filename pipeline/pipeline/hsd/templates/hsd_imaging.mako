<%!
rsc_path = "../"
import os
import collections
%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="header" />

<%block name="title">Image single dish data</%block>

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

    // return a function that sets the Antenna text field to the given spw
    var createAntennaSetter = function(ant) {
        return function() {
        	if (typeof ant !== "undefined") {
	            // trigger a change event, otherwise the filters are not changed
	            $("#select-ant").select2("val", [ant]).trigger("change");
        	}
        };
    };

    // return a function that sets the Field text field to the given spw
    var createAntennaSetter = function(field) {
        return function() {
        	if (typeof field !== "undefined") {
	            // trigger a change event, otherwise the filters are not changed
	            $("#select-field").select2("val", [field]).trigger("change");
        	}
        };
    };

    // return a function that sets the Polarization text field to the given spw
    var createPolarizationSetter = function(pol) {
        return function() {
        	if (typeof pol !== "undefined") {
	            // trigger a change event, otherwise the filters are not changed
	            $("#select-pol").select2("val", [pol]).trigger("change");
        	}
        };
    };

    // 
    var createMixedSetter = function(spw, ant, field, pol) {
        return function() {
            // trigger a change event, otherwise the filters are not changed
        	if (typeof spw !== "undefined") {
	            $("#select-spw").select2("val", [spw]).trigger("change");
        	}
        	if (typeof ant !== "undefined") {
            	$("#select-ant").select2("val", [ant]).trigger("change");
        	}
        	if (typeof field !== "undefined") {
            	$("#select-field").select2("val", [field]).trigger("change");
        	}
        	if (typeof pol !== "undefined") {
	            $("#select-pol").select2("val", [pol]).trigger("change");
        	}
        };
    };    

    // create a callback function for each overview plot that will select the
    // appropriate spw once the page has loaded
    $(".thumbnail a").each(function (i, v) {
        var o = $(v);
        var spw = o.data("spw");
        var ant = o.data("ant");
        var field = o.data("field");
        var pol = o.data("pol");
        o.data("callback", createMixedSetter(spw, ant, field, pol));
    });
});
</script>

<%
stage_dir = os.path.join(pcontext.report_dir, 'stage%s'%(result.stage_number))
plots_list = [{'title': 'Channel Map',
               'subpage': channelmap_subpage,
               'plot': channelmap_plots},
              {'title': 'Baseline Rms Map',
               'subpage': rmsmap_subpage,
               'plot': rmsmap_plots},
              {'title': 'Max Intensity Map',
               'subpage': momentmap_subpage,
               'plot': momentmap_plots},
              {'title': 'Integrated Intensity Map',
               'subpage': integratedmap_subpage,
               'plot': integratedmap_plots}]
%>

<p>This task generates single dish images per source per spectral window. 
It generates an image combined spectral data from whole antenna as well as images per antenna.</p>

<h3>Contents</h3>
<ul>
%if len(rms_table) > 0:
    <li><a href="#sensitivity">Image Sensitivity Table</a></li>
%endif
%if sparsemap_subpage != {}:
    <li><a href="#profilemap">Profile Map</a></li>
%endif
% for plots in plots_list:
    % if plots['subpage'] != {}: 
        <li><a href="#${plots['title'].replace(" ", "")}">${plots['title']}</a></li>
    %endif
% endfor
</ul>

%if len(rms_table) > 0:
	<h3 id="sensitivity" class="jumptarget">Image Sensitivity</h3>
	<p>
	RMS of line-free channels. Estimated RMS is listed for representative images.
	</p>
	<table class="table table-bordered table-striped" summary="Image Sentivitity">
		<caption>RMS of line-free channels</caption>
    	<thead>
	    	<tr>
	        	<th>Name</th><th>Representative (Estimate)</th><th>Frequency Ranges</th><th>Channel width [kHz]</th><th>RMS [Jy/beam]</th>
	    	</tr>

  		</thead>
		<tbody>
		% for tr in rms_table:
			<tr>
			% for td in tr:
				${td}
			% endfor
			</tr>
		%endfor
		</tbody>
	</table>
%endif


%if sparsemap_subpage != {}:
<h3 id="profilemap" class="jumptarget">Profile Map</h3>
  % for field in sparsemap_subpage.keys():
    <h4><a class="replace"
           href="${os.path.join(dirname, sparsemap_subpage[field])}"
	   data-field="${field}">
           ${field}
        </a>
    </h4>
    % for plot in sparsemap_plots[field]:
        % if os.path.exists(plot.thumbnail):
	        <div class="col-md-3">
	            <div class="thumbnail">
	                <a href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
	                   title='<div class="pull-left">Profile Map<br>SPW ${plot.parameters['spw']}<br>Source ${field}</div>'
	                   data-fancybox="thumbs">
	                    <img class="lazyload"
                             data-src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
	                         title="Profile map summary for Spectral Window ${plot.parameters['spw']}">
	                </a>
	
	                <div class="caption">
	                    <h4>
	                        <a href="${os.path.join(dirname, sparsemap_subpage[field])}"
	                           class="replace"
	                           data-spw="${plot.parameters['spw']}"
	                           data-ant="${plot.parameters['ant']}"
	                           data-field="${field}">
	                           Spectral Window ${plot.parameters['spw']}
	                        </a>
	                    </h4>
	
	                    <p>Profile map for spectral
	                        window ${plot.parameters['spw']}.
	                    </p>
	                    
	                    <h4>Detailed profile map</h4>
	                    <table border width="100%">
		                    <tr><th>ANTENNA</th><th colspan="${len(profilemap_entries[field].values()[0])}">POL</th></tr>
		                    % for (ant, pols) in profilemap_entries[field].iteritems():
		                        <tr><td>${ant}</td>
		                        <td align="center">
		                        % for pol in pols:
		                            <a href="${os.path.join(dirname, profilemap_subpage[field])}"
		                               class="btn replace"
		                               data-spw="${plot.parameters['spw']}"
		                               data-ant="${ant}"
	                                       data-field="${field}"
		                               data-pol="${pol}">
		                            ${pol}
		                            </a>
		                        % endfor
		                        </td>
		                        </tr>
		                    % endfor
	                    </table>
	                </div>
	            </div>
	        </div>
        % endif
    % endfor
	<div class="clearfix"></div><!--  flush plots, break to next row -->
  %endfor
%endif

% for plots in plots_list:
    % if plots['subpage'] == {}:
        <% continue %>
    % endif
    <h3 id="${plots['title'].replace(" ", "")}" class="jumptarget">${plots['title']}</h3>
    % for field in plots['subpage'].keys():
        <h4><a class="replace"
               href="${os.path.join(dirname, plots['subpage'][field])}"
               data-field="${field}">
               ${field}
            </a>
        </h4>
        % for plot in plots['plot'][field]:
            % if os.path.exists(plot.thumbnail):
	            <div class="col-md-3">
	                <div class="thumbnail">
	                    <a href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
	                       title='<div class="pull-left">${plots['title']}<br>SPW ${plot.parameters['spw']}<br>Source ${field}</div>'
	                       data-fancybox="thumbs">
	                        <img class="lazyload"
                                 data-src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
	                             title="${plots['title']} for Spectral Window ${plot.parameters['spw']}">
	                    </a>
	
	                    <div class="caption">
	                        <h4>
	                            <a href="${os.path.join(dirname, plots['subpage'][field])}"
	                               class="replace"
	                               data-spw="${plot.parameters['spw']}"
	                               data-ant="${plot.parameters['ant']}"
	                               data-field="${field}">
	                               Spectral Window ${plot.parameters['spw']}
	                            </a>
	                        </h4>
	
	                        <p>${plots['title']} for spectral
	                            window ${plot.parameters['spw']}.
	                        </p>
	                    </div>
	                </div>
	            </div>
            % endif
        % endfor
 	    <div class="clearfix"></div><!--  flush plots, break to next row -->
    % endfor
	<div class="clearfix"></div><!--  flush plots, break to next row -->
% endfor

