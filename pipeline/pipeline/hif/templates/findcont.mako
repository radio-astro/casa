<%inherit file="t2-4m_details-base.mako"/>
<%block name="header" />

<%block name="title">Find Continuum</%block>

% if not table_rows:
    <p>There are no continuum finding results.
% else:
    <table class="table">
        <thead>
            <tr>
                <th rowspan="2">Field</th>
                <th rowspan="2">Spw</th>
                <th colspan="3">Continuum Frequency Range</th>
                <th rowspan="2">Status</th>
                <th rowspan="2">Average spectrum</th>
            </tr>
            <tr>
                <th>Start</th>
                <th>End</th>
                <th>Frame</th>
            </tr>
        </thead>
        <tbody>
            % for tr in table_rows:
            <tr>
            % for td in tr:
                ${td}
            % endfor
            </tr>
            %endfor
        </tbody>
    </table>
%endif
