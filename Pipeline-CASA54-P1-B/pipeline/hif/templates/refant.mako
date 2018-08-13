<%!
    import os
%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">Select Reference Antenna</%block>

<p>Each antenna was ranked according to a flagging score and a geometric score, based on the distance between the
    antenna to the centre of the array.</p>

<table class="table table-bordered table-striped"
       summary="Reference Antennas">
    <caption>Reference antenna selection per measurement set. Antennas are listed in order of highest to lowest
        priority.
    </caption>
    <thead>
    <tr>
        <th>Measurement Set</th>
        <th>Reference Antennas (Highest to Lowest)</th>
    </tr>
    </thead>
    <tbody>
        %for tr in refant_table:
            <tr>
                % for td in tr:
                ${td}
                % endfor
            </tr>
        % endfor
    </tbody>
</table>