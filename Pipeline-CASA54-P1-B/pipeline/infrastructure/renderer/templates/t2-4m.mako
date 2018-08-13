<%!
navbar_active='By Task'
import pipeline.infrastructure.renderer.htmlrenderer as hr
import pipeline.infrastructure.renderer.qaadapter as qa
import pipeline.infrastructure.renderer.rendererutils as rendererutils
%>
<%inherit file="base.mako"/>

<script>
$(document).ready(function() {
	pipeline.pages.t2_4m.ready();
});
</script>
 
<%block name="title">Task Details</%block>
 
<div class="row">
	<div class="col-sm-3 col-md-2 sidebar hidden-print">
		<strong>Tasks in execution order</strong>
		<ul class="nav nav-sidebar">
			% for result in results:
			<li>
				<a id="sidebar_stage${result.stage_number}"
				   data-stagenum="${result.stage_number}"
				   data-taskname="${hr.get_task_name(result)}"
				   href="stage${result.stage_number}/t2-4m_details-container.html">
					${hr.get_task_name(result)}
					<span class="pull-right">
						${rendererutils.get_symbol_badge(result)}						
					</span>
				</a>
			</li>
			% endfor
		</ul>
	</div><!-- end sidebar -->		

	<div class="col-sm-9 col-sm-offset-3 col-md-10 col-md-offset-2 main" 
	     id="fakeframe">
	</div><!-- end div col-md-9 -->
</div><!-- end row -->
