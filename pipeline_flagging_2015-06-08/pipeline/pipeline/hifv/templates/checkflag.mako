<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="title">RFI Flagging</%block>

<p>Flag possible RFI on bandpass calibrator using rflag.</p>

