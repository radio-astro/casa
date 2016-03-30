<%!
rsc_path = ""

import os.path
import pydoc
import numpy as np
import sys

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.displays as displays
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.renderer.logger as logger
import pipeline.infrastructure.renderer.htmlrenderer as hr
import pipeline.infrastructure.renderer.rendererutils as hrutils
import pipeline.infrastructure.renderer.sharedrenderer as sharedrenderer

LOG = infrastructure.get_logger(__name__)
%>
<%inherit file="t2-4m_details-base.html"/>
<%block name="title">Flag T<sub>sys</sub> calibration</%block>

<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>

<script>
$(document).ready(function() {
    // return a function that sets the SPW text field to the given spw
    var createSpwSetter = function(spw) {
        return function() {
            // trigger a change event, otherwise the filters are not changed
            $("#select-spw").select2("val", [spw]).trigger("change");
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


## generate the plots
<%
try:
    r = result[0]
    stage_dir = os.path.join(pcontext.report_dir, 'stage%d' % (r.stage_number))
    plots_dir = stage_dir
    if not os.path.exists(plots_dir):
        os.mkdir(plots_dir)

    plots = []
    for r in result:
        if r.view:
            # display the view first
            # plot() returns the list of Plots it has generated
            LOG.info('Plotting')
            plots.append(displays.ImageDisplay().plot(pcontext, r,
              reportdir=plots_dir))

            # plot Tsys spectra that were flagged
            for flagcmd in r.flagcmds():
                for description in r.descriptions():
                    tsysspectra = r.first(description).children.get(
                      'tsysspectra')
                    if tsysspectra is None:
                        continue

                    for tsys_desc in tsysspectra.descriptions():
                        tsysspectrum = tsysspectra.first(tsys_desc)

                        if not flagcmd.match(tsysspectrum):
                            continue

                        # have found flagged spectrum, now get
                        # associated median spectrum
                        medians = r.last(description).children.get(
                          'tsysmedians')

                        for median_desc in medians.descriptions():
                            median_spectrum = medians.first(median_desc)
                            if median_spectrum.ant is None or \
                              median_spectrum.ant[0]==tsysspectrum.ant[0]:
                                # do the plot
                                plots.append(displays.SliceDisplay().plot(
                                  context=pcontext, results=tsysspectra,
                                  description_to_plot=tsys_desc,
                                  overplot_spectrum=median_spectrum,
                                  reportdir=plots_dir, plotbad=False))
                                break

    # Group the Plots by axes and plot types; each logical grouping will
    # be contained in a PlotGroup
    plot_groups = logger.PlotGroup.create_plot_groups(plots)
    # Write the thumbnail pages for each plot grouping to disk
    for plot_group in plot_groups:
        renderer = sharedrenderer.PlotGroupRenderer(pcontext, r, plot_group)
        plot_group.filename = renderer.filename
        with renderer.get_file() as fileobj:
            fileobj.write(renderer.render())

except Exception, e:
    print 'hif_tsysflag html template exception:', e
    raise e
%>

% if htmlreports:
<h2>Flags</h2>
<table class="table table-bordered table-striped">
	<caption>Report Files</caption>
	<thead>
	    <tr>
	        <th>Flagging Commands</th>
	        <th>Flagging Report</th>
	    </tr>
	</thead>
	<tbody>
	    % for file,reports in htmlreports.items():
	    <tr>
	        <td><a class="replace-pre" href="${reports[0]}">${file}</a></td>
	        <td><a class="replace-pre"
                   href="${reports[1]}">printTsysFlags</a></td>
	    </tr>
	    % endfor
	</tbody>
</table>
% endif
 
% if plot_groups:
<h2>Plots</h2>
<ul>
% for plot_group in plot_groups:
	% if plot_group.title == 'Time vs Antenna1':
		<li>
			<a class="replace" href="${os.path.relpath(os.path.join(dirname, plot_group.filename), pcontext.report_dir)}">${plot_group.title}</a>
               shows the images used for flagging.
        </li>
	% elif 'Tsys vs Channel' in plot_group.title:
		<li>
			<a class="replace" href="${os.path.relpath(os.path.join(dirname, plot_group.filename), pcontext.report_dir)}">${plot_group.title}</a>
               shows the T<sub>sys</sub> spectra flagged in each image.
        </li>
   	% endif
% endfor
</ul>

% if len(summary_plots) > 0:
<h3>T<sub>sys</sub> after flagging</h3>

% for ms in summary_plots:
    <h4><a class="replace"
           href="${os.path.relpath(os.path.join(dirname, summary_subpage[ms]), pcontext.report_dir)}">${ms}</a>
    </h4>
    <ul class="thumbnails">
        % for plot in summary_plots[ms]:
            % if os.path.exists(plot.thumbnail):
            <li class="span3">
                <div class="thumbnail">
                    <a href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
                       class="fancybox"
                       rel="tsys-summary-${ms}">
                        <img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
                             title="T<sub>sys</sub> summary for Spectral Window ${plot.parameters['spw']}"
                             data-thumbnail="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}">
                        </img>
                    </a>

                    <div class="caption">
                    	<h4>
							<a href="${os.path.relpath(os.path.join(dirname, summary_subpage[ms]), pcontext.report_dir)}"
	                       	   class="replace"
	                           data-spw="${plot.parameters['spw']}">
	                           Spectral Window ${plot.parameters['spw']}
	                        </a>
                        </h4>

                        <p>Plot of time-averaged T<sub>sys</sub> for spectral
                            window ${plot.parameters['spw']} (T<sub>sys</sub>
                            window
                        ${plot.parameters['tsys_spw']}), coloured by antenna.
                        </p>
                    </div>
                </div>
            </li>
            % endif
        % endfor
    </ul>
%endfor
%endif

% endif