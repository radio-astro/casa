<%!
rsc_path = ""
%>

<%inherit file="t2-4m_details-base.html"/>

<%block name="title">Calibrate Antenna Positions</%block>

<h2>Results</h2>

% if not table_rows:
    <p>No antenna positions were corrected.</p>
% else:
    <h4>Antenna Position Offsets</h4>

    <p>The following antenna position x, y, z offsets were used to calibrate the antenna positions </p>

    <table class="table table-bordered table-striped" summary="Antenna Position Offsets">
        <caption>Antenna position offsets per measurement set</caption>
        <thead>
            <tr>
            <th scope="col">Measurement Set</th>
            <th scope="col">Antenna</th>
            <th scope="col">X Offset</th>
            <th scope="col">Y Offset</th>
            <th scope="col">Z Offset</th>
        </tr>
        </thead>
        <tbody>
        % for tr in table_rows:
        <tr>
            % for td in tr:
                ${td}
            % endfor
        </tr>
        % endfor
        </tbody>
    </table>
% endif
