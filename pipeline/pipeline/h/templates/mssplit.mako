<%!
rsc_path = ""
%>

<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">Split a Measurement Set</%block>

<h2>Results</h2>

% if not table_rows:
    <p>No split measurement sets were created.</p>
% else:
    <h4>Split Measurement Sets</h4>

    <p>The following split measurement sets were created</p>

    <table class="table table-bordered table-striped" summary="Split measurement sets">
        <caption>Split measurement sets</caption>
        <thead>
           <tr>
            <th scope="col">Original Measurement Set</th>
            <th scope="col">Split Measurement Set</th>
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
