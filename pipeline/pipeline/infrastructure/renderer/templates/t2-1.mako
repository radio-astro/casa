<%!
navbar_active='Observation Summary'
import re
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="base.mako"/>

<script>
$(document).ready(function() {
	pipeline.pages.t2_1.ready();
});
</script>

<%block name="title">Session Data Details</%block>

<div class="row">
	<div class="col-sm-3 col-md-2 sidebar hidden-print">
		% for session in sessions:
		<strong>Session: ${session.name}</strong>
		<ul class="nav nav-sidebar">
			% for ms in session.mses:
			<li>
				<a id="sidebar_${re.sub('[^a-zA-Z0-9]', '_', ms.basename)}"
				   href="session${session.name}/${ms.basename}/t2-1_details.html">
					${ms.basename}
				</a>
			</li>
			% endfor
		</ul>
		% endfor
	</div><!-- /.span3 -->		

	<%
	first_session_name = sessions[0].name
	first_ms_name = sessions[0].mses[0].basename
	%>
	<div class="col-sm-9 col-sm-offset-3 col-md-10 col-md-offset-2 main" 
	     id="session_container"
	     data-src="session${first_session_name}/${first_ms_name}/t2-1_details.html">
	</div>
</div>
