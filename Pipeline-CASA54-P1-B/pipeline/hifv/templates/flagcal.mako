<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">Flagcal</%block>

% for single_result in result:
    <p>Caltable <b>${single_result.inputs['caltable']}</b> flagged with clipminmax=[${','.join([str(x) for x in single_result.inputs['clipminmax']])}].

    </p>


% endfor

