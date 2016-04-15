<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="title">Reweight visibilities</%block>

<p>Calculate data weights based on st. dev. within each spw.</p>

% for single_result in result:

    % if single_result.inputs['contfile'] != '':
        <p><b>Spectral Exclusion File used: </b> ${single_result.inputs['contfile']} </p>
    % endif

% endfor
