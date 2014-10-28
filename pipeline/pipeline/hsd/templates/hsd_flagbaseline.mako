<%!
rsc_path = "../"
import os

import itertools
import pipeline.infrastructure.renderer.htmlrenderer as hr
import pipeline.infrastructure.renderer.logger as logger
import pipeline.infrastructure.displays as displays
%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="header" />

<%block name="title">${hr._get_task_description_for_class(result[0].task)}</%block>

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


<%
def get_fraction(flagged, total):
   if total is 0:
       return 'N/A'
   else:
       return '%0.1f%%' % (100.0 * float(flagged) / float(total))

try:
   #stage_number = result.stage_number
   #stage_dir = os.path.join(pcontext.report_dir,'stage%d'%(stage_number))
   #if not os.path.exists(stage_dir):
   #    os.mkdir(stage_dir)

   #rel_path = os.path.basename(stage_dir)   ### stage#

   # for baseline summary
   detail = baseline_context[-1]['detail']
   cover_only = baseline_context[-1]['cover_only']

   # for flag summary
   html_names = []
   asdm_names = []
   ant_names = []
   spw = []
   pol = []
   nrows = []
   flags = []
   flag_result = result[-1].outcome['blflag']
   summaries = flag_result.outcome['summary']
   for summary in summaries:
       html_names.append(summary['html'])
       asdm_names.append(summary['name'])
       ant_names.append(summary['antenna'])
       spw.append(summary['spw'])
       pol.append(summary['pol'])
       nrows.append(summary['nrow'])
       flags.append(summary['nflags'])

   flag_types = ['Total', 'Tsys', 'Weather', 'User']
   fit_flags = ['Baseline RMS', 'Running mean', 'Expected RMS']
except Exception, e:
   print 'hsd_imaging html template exception:', e
   raise e
%>

<p>This task is kind of "supertask" that executes baseline subtraction and flagging repeatedly inside the task. 
Number of iteration is specified by the task inputs "iteration".  
So far, final baseline and flag results are shown below.</p>

<h2>Final Iteration</h2>
<h3>Line Detection by Clustering Analysis</h3>

% if len(detail) > 0 or len(cover_only) > 0:

<!-- Link to details page -->
% for plots in detail:
    <h3><a class="replace"
    href="${os.path.join(dirname, plots['html'])}">${plots['title']}
<!--        href="${os.path.relpath(os.path.join(dirname, plots['html']), pcontext.report_dir)}">${plots['title']}-->
    </h3>
    <ul class="thumbnails">
        % for plot in plots['cover_plots']:
            % if os.path.exists(plot.thumbnail):
                <li class="span3">
                    <div class="thumbnail">
                        <a href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
                           class="fancybox"
                           rel="thumbs">
                           <img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
                                 title="${plots['title']} for Spectral Window ${plot.parameters['spw']}"
                                 data-thumbnail="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}">
                           </img>
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
                </li>
            % endif
        % endfor
    </ul>
% endfor

<!-- No details -->
% for plots in cover_only:
    <h3>${plots['title']}</h3>
    <ul class="thumbnails">
        % for plot in plots['cover_plots']:
            % if os.path.exists(plot.thumbnail):
                <li class="span3">
                    <div class="thumbnail">
                        <a href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
                           class="fancybox"
                           rel="thumbs">
                           <img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
                                 title="${plots['title']} for Spectral Window ${plot.parameters['spw']}"
                                 data-thumbnail="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}">
                           </img>
                        </a>
    
                        <div class="caption">
                            <h4>Spectral Window ${plot.parameters['spw']}</h4>
                            <p>Clustering plot of spectral window 
                            ${plot.parameters['spw']}.
                            </p>
                        </div>
                    </div>
                </li>
            % endif
        % endfor
    </ul>
% endfor

% else:
<p>No Line detected</p>
% endif


<h3>Flag Summaries</h3>

% if html_names:
<table class="table table-bordered table-striped " summary="Flagged Data">
<thead>
    <tr>
    <th rowspan="2">
    <th colspan="5">Data Selection</th>
    <th colspan="2">Flagged Total</th>
    %for ftype in flag_types[1:]:
    <th rowspan="2">${ftype}</th>
    %endfor
    %for fflag in fit_flags:
    <th colspan="2">${fflag}</th>
    %endfor
    </tr>
    <tr>
    <th>Name</th><th>Ant.</th><th>spw</th><th>Pol</th><th># of rows</th>
    <th>row #</th><th>fraction</th>
    %for fflag in fit_flags:
    <th>post-fit</th><th>pre-fit</th>
    %endfor
    </tr>
    </thead>
    <tbody>
    %for idx in range(len(html_names)):
    <tr>
        <th><a class="replace-pre" href="${os.path.relpath(os.path.join(dirname, html_names[idx]), pcontext.report_dir)}">details</a></th>
        <td>${asdm_names[idx]}</td><td>${ant_names[idx]}</td><td>${spw[idx]}</td><td>${pol[idx]}</td><td>${nrows[idx]}</td>
        <td>${flags[idx][0]}</td>
        %for nflg in flags[idx]:
        <td>${get_fraction(nflg, nrows[idx])}</td>
        %endfor
    </tr>
    %endfor
    </tbody>
    </table>
% else:
No Flag Summaries
% endif
