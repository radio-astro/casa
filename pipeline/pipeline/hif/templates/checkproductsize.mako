<%inherit file="t2-4m_details-base.mako"/>
<%block name="header" />

<%block name="title">Check Product Size</%block>

% if not table_rows:
    <p>There are no product size check results.
% else:
    <%
    allowed_maxcubesize = '%.3g GB' % (result[0].allowed_maxcubesize)
    allowed_maxcubelimit = '%.3g GB' % (result[0].allowed_maxcubelimit)
    original_maxcubesize = '%.3g GB' % (result[0].original_maxcubesize)
    mitigated_maxcubesize = '%.3g GB' % (result[0].mitigated_maxcubesize)
    allowed_productsize = '%.3g GB' % (result[0].allowed_productsize)
    original_productsize = '%.3g GB' % (result[0].original_productsize)
    cube_mitigated_productsize = '%.3g GB' % (result[0].cube_mitigated_productsize)
    mitigated_productsize = '%.3g GB' % (result[0].mitigated_productsize)
    %>
    <p>
    Allowed maximum cube size: ${allowed_maxcubesize}
    <br>
    Allowed cube size limit: ${allowed_maxcubelimit}
    <br>
    Predicted maximum cube size: ${original_maxcubesize}
    <br>
    Mitigated maximum cube size: ${mitigated_maxcubesize}
    <br>
    Allowed product size: ${allowed_productsize}
    <br>
    Initial predicted product size: ${original_productsize}
    <br>
    Predicted product size after cube size mitigation: ${cube_mitigated_productsize}
    <br>
    Mitigated product size: ${mitigated_productsize}
    <p>
    Size mitigation parameters for subsequent hif_makeimlist calls
    <table class="table">
        <thead>
            <tr>
                <th>nbins</th>
                <th>hm_imsize</th>
                <th>hm_cell</th>
                <th>field</th>
                <th>spw (cube)</th>
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
