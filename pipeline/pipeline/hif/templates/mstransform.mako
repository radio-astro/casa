<%!
rsc_path = ""
%>

<%inherit file="t2-4m_details-base.html"/>

<%block name="title">Create Target Measurement Sets for Imaging</%block>

<h2>Results</h2>

% if not table_rows:
    <p>No science target measurement sets for imaging were created.</p>
% else:
    <h4>Target Measurement Sets</h4>

    <p>The following science target measurement sets were created for imaging</p>

    <table class="table table-bordered table-striped" summary="Science target measurement sets">
        <caption>Science target measurement sets</caption>
        <thead>
           <tr>
            <th scope="col">Original Measurement Set</th>
            <th scope="col">Target Measurement Set</th>
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
