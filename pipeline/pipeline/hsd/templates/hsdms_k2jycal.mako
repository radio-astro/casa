<%!
rsc_path = "../"
import os
import collections
%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="header" />

<%block name="title">Generate Kelvin to Jansky conversion caltables</%block>

<%
stage_dir = os.path.join(pcontext.report_dir, 'stage%s'%(result.stage_number))
rowspans_ms = collections.defaultdict(lambda: 0)
rowspans_ant = collections.defaultdict(lambda: collections.defaultdict(lambda: 0))
rowspans_spw = collections.defaultdict(lambda: collections.defaultdict(lambda: collections.defaultdict(lambda: 0)))
for ms in pcontext.observing_run.measurement_sets:
    vis = ms.basename
    for ant in ms.get_antenna():
        ant_name = ant.name
        for spw in ms.get_spectral_windows(science_windows_only=True):
            spwid = spw.id
	    ddid = ms.get_data_description(spw=spwid)
            num_factors = ddid.num_polarizations
            rowspans_ms[vis] += num_factors
            rowspans_ant[vis][ant_name] += num_factors
            rowspans_spw[vis][ant_name][spw.id] += num_factors
%>

<p>This task generates single dish images per source per spectral window. 
It generates an image combined spectral data from whole antenna as well as images per antenna.</p>

<h3>Jy/K Conversion Factor</h3>
The following table lists the Jy/K factor.
% if reffile is not None and len(reffile) > 0 and os.path.exists(os.path.join(stage_dir, os.path.basename(reffile))):
Input file is <a class="replace-pre" href="${os.path.relpath(reffile, pcontext.report_dir)}">${os.path.basename(reffile)}</a>.
% else:
No Jy/K factors file is specified. 
% endif
<table border width="100%">
<tr><th>MS</th><th>Antenna</th><th>Spw</th><th>Pol</th><th>Factor</th></tr>
% for ms in pcontext.observing_run.measurement_sets:
    <% vis_first_row = 1 %>
    <% vis = ms.basename %>
    % for ant in ms.get_antenna():
        <% ant_first_row = 1 %>
        <% ant_name = ant.name %>
        % for spw in ms.get_spectral_windows(science_windows_only=True):
            <% spwid = spw.id %>
            <% jyperk_corr = jyperk[ms.basename][ant_name][spwid] %>
            <% spw_first_row = 1 %>
            <% ddid = ms.get_data_description(spw=spwid) %>
            <% corr_list = map(ddid.get_polarization_label, range(ddid.num_polarizations)) %>
            % for corr in corr_list:
                <% factor = jyperk_corr[corr] %>
                % if vis_first_row == 1:
                    <tr><td rowspan="${rowspans_ms[vis]}">${vis}</td><td rowspan="${rowspans_ant[vis][ant_name]}">${ant_name}</td><td rowspan="${rowspans_spw[vis][ant_name][spwid]}">${spwid}</td><td>${corr}</td><td>${factor}</td></tr>
                    <% vis_first_row = 0 %>
                    <% ant_first_row = 0 %>
                    <% spw_first_row = 0 %>
                % elif ant_first_row == 1:
                    <tr><td rowspan="${rowspans_ant[vis][ant_name]}">${ant_name}</td><td rowspan="${rowspans_spw[vis][ant_name][spwid]}">${spwid}</td><td>${corr}</td><td>${factor}</td></tr>
                    <% ant_first_row = 0 %>
                    <% spw_first_row = 0 %>
                % elif spw_first_row == 1:
                    <tr><td rowspan="${rowspans_spw[vis][ant_name][spwid]}">${spwid}</td><td>${corr}</td><td>${factor}</td></tr>
                    <% spw_first_row = 0 %>
                % else:
                    <tr><td>${corr}</td><td>${factor}</td></tr>
                % endif
            % endfor
        % endfor
    % endfor
% endfor
</table>

