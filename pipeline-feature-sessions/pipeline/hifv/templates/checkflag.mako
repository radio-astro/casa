<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">RFI Flagging</%block>

<p>Flag possible RFI using rflag and tfcrop; checkflagmode=${result[0].inputs['checkflagmode']}</p>

