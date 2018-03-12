<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">Targetflag </%block>

<p>Run all calibrated data (including intent='*CALIBRATE*,*TARGET*') through rflag if specified.  If a file with continuum
regions is specified, then rflag will only flag those spw and frequency ranges per the pipeline spectral line heuristics.</p>

% for single_result in result:
    <p><b>Intents through rflag:</b>
    % if single_result.inputs['intents'] == '':
          All intents used
    % else:
          ${single_result.inputs['intents']}
    % endif
    </p>


% endfor







