<%!
rsc_path = ""
import pipeline.infrastructure.renderer.rendererutils as rendererutils
%>

<script>
$(document).ready(function() {
	pipeline.pages.t2_4m_details_container.ready();
});
</script>

% if container_urls:
<nav id="ms_selector" class="navbar navbar-fixed-top navbar-inverse col-sm-9 col-sm-offset-3 col-md-10 col-md-offset-2">
	<div class="container-fluid">
        <div class="navbar-header">
            <button type="button" class="navbar-toggle" data-toggle="collapse" data-target=".navbar-ex1-collapse">  
                <span class="sr-only">Toggle navigation</span>
                <span class="icon-bar"></span>
                <span class="icon-bar"></span>
                <span class="icon-bar"></span>
            </button>
        </div>
        <div class="collapse navbar-collapse navbar-ex1-collapse">
            <ul id="detailscontainer-navbar" class="nav navbar-nav">
                <li class="menu-item dropdown">
                    <a href="#" class="dropdown-toggle" data-toggle="dropdown">Measurement Set<b class="caret"></b></a>
                    <ul class="dropdown-menu">
                    	% for session_id, session in container_urls.items():
                        <li class="menu-item dropdown dropdown-submenu">
                            <a href="#" class="dropdown-toggle" data-toggle="dropdown">${session_id}</a>
                            <ul class="dropdown-menu">
                                <li class="menu-item ">
									% for ms_id, (ms_link, ms_result) in session.items():
                                    <a class="replace" href="${ms_link}" id="ms-${ms_id}" data-stopPropagation="true">${ms_id}<span class="text-right">${rendererutils.get_symbol_badge(ms_result)}</span></a>
                                    % endfor
                                </li>
                            </ul>
                        </li>
						% endfor
                    </ul>
                </li>
            </ul>
	    <div>	
		<p id="container-active" class="navbar-text navbar-right navbar-projectcode">Currently viewing ${active_ms}</p>
	</div>
</nav>
% endif

<div id="session_container">
</div>
