<%!
rsc_path = ""
import os
import pipeline.infrastructure.utils as utils

# method to output flagging percentages neatly
def percent_flagged(flagsummary):
    flagged = flagsummary.flagged
    total = flagsummary.total

    if total is 0:
        return 'N/A'
    else:
        return '%0.1f%%' % (100.0 * flagged / total)
%>

<%
# these functions are defined in template scope.
def num_lines(relpath):
	abspath = os.path.join(pcontext.report_dir, relpath)
	if os.path.exists(abspath):
		return sum(1 for line in open(abspath) if not line.startswith('#'))
	else:
		return 'N/A'
%>

<%inherit file="t2-4m_details-base.mako"/>
<%block name="header" />

<%block name="title">Flag corrected-model amplitudes for calibrator</%block>

This task identifies baselines and antennas with a significant fraction of
 outlier integrations, based on a comparison of the calibrated (corrected)
 amplitudes with the model amplitudes for one or more specified calibrator
 sources.

% if htmlreports:
    <h2>Flags</h2>
    <table class="table table-bordered table-striped">
        <caption>Report Files</caption>
        <thead>
            <tr>
                <th>Measurement Set</th>
                <th>Flagging Commands</th>
                <th>Number of Statements</th>
            </tr>
        </thead>
        <tbody>
        % for msname, relpath in htmlreports.items():
            <tr>
                <td>${msname}</td>
                <td><a class="replace-pre" href="${relpath}">${os.path.basename(relpath)}</a></td>
                <td>${num_lines(relpath)}</td>
            </tr>
        % endfor
        </tbody>
    </table>

% endif

% for ms in flags.keys():
    <h4>Measurement Set: ${ms}</h4>
    <table class="table table-bordered table-striped ">
        <caption>Summary of flagged data. Each cell states the amount of data
            flagged as a fraction of the specified data selection.
        </caption>
        <thead>
            <tr>
                <th rowspan="2">Data Selection</th>
                <!-- flags before task is always first agent -->
                <th rowspan="2">flagged before</th>
                <th rowspan="2">flagged after</th>
            </tr>
        </thead>
        <tbody>
            % for k in ['TOTAL', 'BANDPASS', 'AMPLITUDE', 'PHASE', 'TARGET','ATMOSPHERE']:
            <tr>
                <th>${k}</th>
                % for step in ['before','after']:
                % if flags[ms].get(step) is not None:
                    <td>${percent_flagged(flags[ms][step]['Summary'][k])}</td>
                % else:
                    <td>0.0%</td>
                % endif
                % endfor
            </tr>
            % endfor
        </tbody>
    </table>
% endfor
