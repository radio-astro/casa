<%!
import cgi
import os
import xml.sax.saxutils as saxutils

import pipeline.domain.measures as measures
import pipeline.extern.asizeof as asizeof
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.htmlrenderer as hr
import pipeline.infrastructure.renderer.rendererutils as rendererutils
import pipeline.infrastructure.utils as utils

def get_qascores(pool, lo, hi):
    with_score = [s for s in pool if s.score not in ('', 'N/A', None)]
    return [s for s in with_score if s.score > lo and s.score <= hi]

def get_notification_trs(result, alerts_info, alerts_success):
    notifications = []
    if result.qa.pool:
        for qascore in get_qascores(result.qa.pool, -0.1, rendererutils.SCORE_THRESHOLD_ERROR):
            n = format_notification('danger alert-danger', 'QA', qascore.longmsg, 'glyphicon glyphicon-remove-sign')
            notifications.append(n)
        for qascore in get_qascores(result.qa.pool, rendererutils.SCORE_THRESHOLD_ERROR, rendererutils.SCORE_THRESHOLD_WARNING):
            n = format_notification('warning alert-warning', 'QA', qascore.longmsg, 'glyphicon glyphicon-exclamation-sign')
            notifications.append(n)
    for logrecord in utils.get_logrecords(result, logging.ERROR):
        n = format_notification('danger alert-danger', 'Error!', logrecord.msg)
        notifications.append(n)
    for logrecord in utils.get_logrecords(result, logging.WARNING):
        n = format_notification('warning alert-warning', 'Warning!', logrecord.msg)
        notifications.append(n)
    if alerts_info is not UNDEFINED:
        for msg in alerts_info:
            n = format_notification('info alert-info', '', msg)
            notifications.append(n)
    if alerts_success is not UNDEFINED:
        for msg in alerts_success:
            n = format_notification('success alert-success', '', msg)
            notifications.append(n)

    return notifications

def format_notification(tr_class, alert, msg, icon_class=None):
    if icon_class:
        icon = '<span class="%s"></span> ' % icon_class
    else:
        icon = ''
    return '<tr class="%s"><td>%s<strong>%s</strong> %s</td></tr>' % (tr_class, icon, alert, msg)

%>

<%def name="plot_group(plot_dict, url_fn, data_spw=False, data_field=False, data_baseband=False, data_tsysspw=False,
                       data_vis=False, title_id=None, plot_accessor=lambda d: [('No Intent', d)], transmission=None,
                       rel_fn=None)">
% if plot_dict:
    % if title_id:
        <h3 id="${title_id}" class="jumptarget">${caller.title()}</h3>
    % else:
        <h3>${caller.title()}</h3>
    % endif

    % if hasattr(caller, 'preamble'):
         ${caller.preamble()}
    % endif

    % for ms, ms_plots in plot_dict.items():
        <%
            relurl = url_fn(ms)
            if relurl:
                subpage_abspath = os.path.join(pcontext.report_dir, dirname, relurl)
                subpage_path = os.path.relpath(subpage_abspath, pcontext.report_dir)
                subpage_exists = os.path.exists(subpage_abspath)
            else:
                subpage_exists = false

            if transmission:
                ms_transmission_plots = transmission.get(ms, {})
            else:
                ms_transmission_plots = {}
        %>

        <h4>
            % if subpage_exists:
            <a class="replace"
               % if data_vis:
               data-vis="${ms}"
               % endif
               href="${subpage_path}">
            % endif
                ${ms}
            % if subpage_exists:
            </a>
            % endif
        </h4>

        % if hasattr(caller, 'ms_preamble'):
            ${caller.ms_preamble(ms)}
        % endif

        <div class="row">
            % for intent, intent_plots in plot_accessor(ms_plots):
                % if intent_plots is not None:
                % for plot in intent_plots:
                <div class="col-md-3 col-sm-4">
                    % if os.path.exists(plot.thumbnail):
                    <%
                        fullsize_relpath = os.path.relpath(plot.abspath, pcontext.report_dir)
                        thumbnail_relpath = os.path.relpath(plot.thumbnail, pcontext.report_dir)

                        if intent in ms_transmission_plots:
                            transmission_plots = [p for p in ms_transmission_plots.get(intent, [])
                                                  if p.parameters['spw'] == plot.parameters['spw']]

                            if intent == 'TARGET':
                                transmission_plots = [p for p in transmission_plots
                                                      if p.parameters['field'] == plot.parameters['field']]

                            if len(transmission_plots) is 1:
                                transmission_plot = transmission_plots[0]
                                if os.path.exists(transmission_plot.thumbnail):
                                    transmission_fullsize_relpath = os.path.relpath(transmission_plot.abspath, pcontext.report_dir)
                                    transmission_thumbnail_relpath = os.path.relpath(transmission_plot.thumbnail, pcontext.report_dir)
                                else:
                                    transmission_plot = False
                        else:
                            transmission_plot = False
                    %>

                    <div class="thumbnail">
                        % if transmission_plot:
                        <a href="${transmission_fullsize_relpath}"
                           class="fancybox"
                           rel="${'transmission-%s-%s'.format(ms, intent)}"
                           % if hasattr(caller, 'fancybox_caption'):
                           title='<div class="pull-left">${caller.fancybox_caption(transmission_plot)}</div><div class="pull-right"><a href="${transmission_fullsize_relpath}">Full Size</a><br>${rendererutils.get_plot_command_markup(pcontext, transmission_plot.command)}</div>'
                           % endif
                           data-thumbnail="${transmission_thumbnail_relpath}">
                            <img src="${transmission_thumbnail_relpath}"
                                 % if hasattr(caller, 'transmission_mouseover'):
                                 title="${caller.transmission_mouseover(transmission_plot)}"
                                 % endif
                                 data-thumbnail="${transmission_thumbnail_relpath}">
                        </a>
                        <br>
                        % endif
                        <a href="${fullsize_relpath}"
                           class="fancybox"
                           % if rel_fn:
                           rel="${rel_fn(plot)}"
                           % elif relurl:
                           rel="${relurl}"
                           % else:
                           rel="${caller.title()}"
                           % endif
                           % if hasattr(caller, 'fancybox_caption'):
                           title='<div class="pull-left">${caller.fancybox_caption(plot)}</div><div class="pull-right"><a href="${fullsize_relpath}">Full Size</a><br>${rendererutils.get_plot_command_markup(pcontext, plot.command)}</div>'
                           % endif
                           data-thumbnail="${thumbnail_relpath}">
                            <img src="${thumbnail_relpath}"
                                 % if hasattr(caller, 'mouseover'):
                                 title="${caller.mouseover(plot)}"
                                 % endif
                                 data-thumbnail="${thumbnail_relpath}">
                        </a>

                        <div class="caption">
                            <h4>
                            % if subpage_exists:
                                <a href="${subpage_path}"
                                % if data_field:
                                   data-field="${cgi.escape(plot.parameters['field'], True)}"
                                % endif
                                % if data_spw:
                                   data-spw="${plot.parameters['spw']}"
                                % endif
                                % if data_tsysspw:
                                   data-tsys_spw="${plot.parameters['tsys_spw']}"
                                % endif
                                % if data_baseband and 'baseband' in plot.parameters:
                                   data-baseband="${plot.parameters['baseband']}"
                                % endif
                                % if data_vis:
                                   data-vis="${plot.parameters['vis']}"
                                % endif
                                   class="replace">
                            % endif
                            ${caller.caption_title(plot)}
                            % if subpage_exists:
                                </a>
                            % endif
                            </h4>
                            % if hasattr(caller, 'caption_subtitle'):
                                <h6>${caller.caption_subtitle(plot)}</h6>
                            % endif

                            % if hasattr(caller, 'caption_text'):
                            <p>${caller.caption_text(plot, intent)}</p>
                            % endif
                        </div>
                    </div>
                    % endif
                </div>
                % endfor
                % endif
            % endfor
        </div><!-- end row -->

    % endfor

% endif
</%def>

<div class="page-header">
    <h1>${hr.get_stage_number(result)}. <%block name="title">Untitled Task</%block><%block name="backbutton"><button class="btn btn-default pull-right" onClick="javascript:window.history.back();">Back</button></%block></h1>
</div>

<%
    notification_trs = get_notification_trs(result, alerts_info, alerts_success)
%>
% if notification_trs:
<table class="table table-bordered">
    <thead>
        <tr>
            <th>Task notifications</th>
        </tr>
    </thead>
    <tbody>
    % for tr in notification_trs:
        ${tr}
    % endfor
    </tbody>
</table>
% endif

${next.body()}

<div class="clearfix"></div>
<div class="panel-group" id="details-accordion" role="tablist" aria-multiselectable="true">

    <%doc>
    Help disabled until the task descriptions from John's presentation are added

    %if taskhelp:

        <div class="accordion-group">
            <div class="accordion-heading">
                <a class="accordion-toggle" data-toggle="collapse" data-parent="#details-accordion" href="#collapseOne">
                    Task Help
                </a>
            </div>
            <div id="collapseOne" class="accordion-body collapse">
                <div class="accordion-inner">
                    ${taskhelp}
                </div>
            </div>
        </div>
    %endif
    </%doc>

    <div class="panel panel-default">
        <div class="panel-heading" role="tab" id="headingThree">
            <h4 class="panel-title">
                <a data-toggle="collapse" data-parent="#details-accordion" href="#collapseThree" aria-expanded="false" aria-controls="collapseThree">
                Pipeline QA
                </a>
            </h4>
        </div>
        <div id="collapseThree" class="panel-collapse collapse" role="tabpanel" aria-labelledby="headingThree">
            <div class="panel-body">
                % if result.qa.pool:
                <table class="table table-bordered" summary="Pipeline QA summary">
                    <caption>Pipeline QA summary for this task.</caption>
                    <thead>
                        <tr>
                            <th>Score</th>
                            <th>Reason</th>
                        </tr>
                    </thead>
                    <tbody>
                    % for qascore in get_qascores(result.qa.pool, -0.1, rendererutils.SCORE_THRESHOLD_ERROR):
                    <tr class="danger alert-danger">
                        <td>${'%0.2f' % qascore.score}</td>
                        <td>${qascore.longmsg}</td>
                    </tr>
                    % endfor
                    % for qascore in get_qascores(result.qa.pool, rendererutils.SCORE_THRESHOLD_ERROR, rendererutils.SCORE_THRESHOLD_WARNING):
                    <tr class="warning alert-warning">
                        <td>${'%0.2f' % qascore.score}</td>
                        <td>${qascore.longmsg}</td>
                    </tr>
                    % endfor
                    % for qascore in get_qascores(result.qa.pool, rendererutils.SCORE_THRESHOLD_WARNING, rendererutils.SCORE_THRESHOLD_SUBOPTIMAL):
                    <tr class="info alert-info">
                        <td>${'%0.2f' % qascore.score}</td>
                        <td>${qascore.longmsg}</td>
                    </tr>
                    % endfor
                    % for qascore in get_qascores(result.qa.pool, rendererutils.SCORE_THRESHOLD_SUBOPTIMAL, 1.0):
                    <tr class="success alert-success">
                        <td>${'%0.2f' % qascore.score}</td>
                        <td>${qascore.longmsg}</td>
                    </tr>
                    % endfor
                </table>
                % else:
                    No pipeline QA for this task.
                % endif
            </div>
        </div>
    </div>

    <div class="panel panel-default">
        <div class="panel-heading" role="tab" id="headingFour">
            <h4 class="panel-title">
                <a data-toggle="collapse" data-parent="#details-accordion" href="#collapseFour" aria-expanded="false" aria-controls="collapseFour">
                Input Parameters
                </a>
            </h4>
        </div>
        <div id="collapseFour" class="panel-collapse collapse" role="tabpanel" aria-labelledby="headingFour">
            <div class="panel-body">
                <dl class="dl-horizontal">
                % for k, v in result.inputs.iteritems():
                    <dt>${str(k)}</dt>
                    <dd>${str(v) if str(v) != '' else '&nbsp;'}</dd>
                % endfor
                </dl>
            </div>
        </div>
    </div>

    <div class="panel panel-default">
        <div class="panel-heading" role="tab" id="headingFive">
            <h4 class="panel-title">
                <a data-toggle="collapse" data-parent="#details-accordion" href="#collapseFive" aria-expanded="false" aria-controls="collapseFive">
                Tasks Execution Statistics
                </a>
            </h4>
        </div>
        <div id="collapseFive" class="panel-collapse collapse" role="tabpanel" aria-labelledby="headingFive">
            <div class="panel-body">
                <dl class="dl-horizontal">
                    <dt>Start time</dt>
                    <dd>${utils.format_datetime(result.timestamps.start, dp=3)}</dd>
                    <dt>End</dt>
                    <dd>${utils.format_datetime(result.timestamps.end, dp=3)}</dd>
                    <dt>Duration</dt>
                    <dd>${utils.format_timedelta(result.timestamps.end - result.timestamps.start, dp=3)}</dd>
                    % if logging.logging_level <= logging.DEBUG:
                        <dt>Context size</dt>
                        <dd>${str(measures.FileSize(asizeof.asizeof(pcontext), measures.FileSizeUnits.BYTES))}</dd>
                    % endif
                </dl>
            </div>
        </div>
    </div>
</div>

<%def name="li_anchor_to_file(relpath)">
    <%
    abspath = os.path.join(pcontext.report_dir, relpath)
    file_exists = os.path.exists(abspath)
    if file_exists:
        total_bytes = os.path.getsize(abspath)
        filesize = measures.FileSize(total_bytes, measures.FileSizeUnits.BYTES)
    %>
    % if file_exists:
        <li><a href="${relpath}" class="replace-pre" data-title="CASA log for stage ${result.stage_number}">View</a> or <a href="${relpath}" download="${relpath}">download</a> ${relpath} (${str(filesize)})</li>
    % endif
</%def>

%if casalog_url:
<div class="panel panel-default">
    <div class="panel-heading">
        <h3 class="panel-title">CASA logs for stage ${result.stage_number}</h3>
    </div>
    <div class="panel-body">
        <ul>${li_anchor_to_file(casalog_url)}</ul>
    </div>
</div>
% endif
