<%!
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.mako"/>
<%block name="header" />

<%block name="title">${hr.get_task_description(result, pcontext, include_stage=False)}</%block>

<p>Sorry, this task does not have a results template.</p>
