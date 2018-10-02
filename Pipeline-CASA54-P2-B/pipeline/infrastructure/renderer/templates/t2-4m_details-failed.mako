<%!
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.mako"/>
<%block name="header" />

<%block name="title">${hr.get_task_description(result, pcontext, include_stage=False)}</%block>

<p>Sorry, this task encountered an unexpected error during execution and pipeline execution has halted.</p>

<h2>Error message from task execution:</h2>

%for tb in tracebacks:
    <pre>${tb}</pre>
%endfor
