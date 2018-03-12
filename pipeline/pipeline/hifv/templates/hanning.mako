<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">Hanning Smoothing</%block>

<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>

<p>Hanning Smoothing</p>

