<%!
rsc_path = "../"
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="base.mako"/>
<%block name="header" />

<%block name="title">Stage ${hr.get_stage_number(result)} QA Details</%block>

<h1>Stage ${hr.get_stage_number(result)}: ${hr.get_task_description(result, pcontext)}</h1>

<p>This task is missing a specialised QA renderer. Please contact the developers.</p>
