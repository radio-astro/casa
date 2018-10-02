<%!
import os

def sanitise_value(value):
    if value is None:
        return "N/A"
    return value

def percentage_flagged(flag):
    return "{:.1%}".format(flag['flagged'] / flag['total'])
%>

<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">Restore Calibrated Data</%block>

<h3>Previous processing environment</h3>

<table class="table table-bordered table-striped" summary="CASA and Pipeline version used during previous processing run">
    <caption>Information on processing environment during previous processing run.</caption>
    <thead>
        <tr>
            <th scope="col">Description</th>
            <th scope="col">Value</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td>CASA Version</td>
            <td>${sanitise_value(casa_version)}</td>
        </tr>
        <tr>
            <td>Pipeline Version</td>
            <td>${sanitise_value(pipeline_version)}</td>
        </tr>
        <tr>
            <td>Execution mode</td>
            <td>${execution_mode}</td>
        </tr>
    </tbody>
</table>

<h3>Representative Target</h3>

% if repsource_defined:
<p>The following representative target sources and spws are defined</p>
<table class="table table-bordered table-striped table-condensed"
	   summary="Representative target information">
	<caption>Representative target sources. These are imported from the context or derived from the ASDM.</caption>
    <thead>
	    <tr>
	        <th scope="col" rowspan="2">Measurement Set</th>
	        <th scope="col" colspan="5">Representative Source</th>
	    </tr>
	    <tr>
	        <th scope="col">Name</th>
	        <th scope="col">Representative Frequency</th>
	        <th scope="col">Bandwidth for Sensitivity</th>
	        <th scope="col">Spw Id</th>
	        <th scope="col">Chanwidth</th>
	    </tr>
    </thead>
    <tbody>
	% for tr in repsource_table_rows:
		<tr>
		% for td in tr:
			${td}
		% endfor
		</tr>
	%endfor
    </tbody>
</table>
% elif repsource_name_is_none:
    <p>An incomplete representative target source is defined with target name "none". Will try to fall back to existing
        science target sources or calibrators in the imaging tasks.</p>
% else:
    <p>No representative target source is defined. Will try to fall back to existing science target sources in the
        imaging tasks.</p>
% endif

<h3>Flagging Summary</h3>
<table class="table table-bordered table-striped table-condensed" summary="Flagging summary">
	<caption>Summary of flagged data for scans of target sources with intent="TARGET".</caption>
    <thead>
        <tr>
            <th colspan="1">Source</th>
            <th colspan="1">Measurement Set</th>
            <th colspan="${flags_maxspw}">% Flagged data for each science spw</th>
        </tr>
    </thead>
    <tbody>
	% for src in flags:
        <% src_rowspan = len(flags[src]) %>
        <tr>
            <td rowspan="${src_rowspan * 2}">${src}</td>
        % for vis in flags[src]:
            <td rowspan="2">${vis}</td>
            % for spw in sorted(flags[src][vis]):
                <td><strong>${spw}</strong></td>
            % endfor
            % for ind in range(flags_maxspw-len(flags[src][vis])):
                <td></td>
            % endfor
            </tr>
            % for spw in sorted(flags[src][vis]):
                <td>${percentage_flagged(flags[src][vis][spw])}</td>
            % endfor
            % for ind in range(flags_maxspw-len(flags[src][vis])):
                <td></td>
            % endfor
		</tr>
        % endfor
	%endfor
    </tbody>
</table>
