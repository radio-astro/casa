<%!
rsc_path = ""
css_file = "css/pipeline.css"
navbar_active=''

import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta http-equiv="X-UA-Compatible" content="IE=edge">
<%block name="head">

<!-- Add jQuery library -->
<script src="${self.attr.rsc_path}resources/js/jquery-2.1.3.min.js"></script>

<!-- bootstrap 3 -->
<!-- 
<link href="${self.attr.rsc_path}resources/css/bootstrap.min.css" rel="stylesheet">
 -->
<link href="${self.attr.rsc_path}resources/css/pipeline.min.css" rel="stylesheet">
<script src="${self.attr.rsc_path}resources/js/bootstrap.js"></script>
<!-- for now, adopt the bootstrap 2 theme  -->


<!-- Add mousewheel plugin (this is optional) -->
<script src="${self.attr.rsc_path}resources/fancybox/lib/jquery.mousewheel-3.0.6.pack.js"></script>

<link rel="stylesheet" href="${self.attr.rsc_path}resources/fancybox/source/jquery.fancybox.css" type="text/css" media="screen" />
<script src="${self.attr.rsc_path}resources/fancybox/source/jquery.fancybox.pack.js"></script>

<link rel="stylesheet" href="${self.attr.rsc_path}resources/fancybox/source/helpers/jquery.fancybox-buttons.css" type="text/css" media="screen" />
<script src="${self.attr.rsc_path}resources/fancybox/source/helpers/jquery.fancybox-buttons.js"></script>
<script src="${self.attr.rsc_path}resources/fancybox/source/helpers/jquery.fancybox-media.js"></script>

<link rel="stylesheet" href="${self.attr.rsc_path}resources/fancybox/source/helpers/jquery.fancybox-thumbs.css" type="text/css" media="screen" />
<script src="${self.attr.rsc_path}resources/fancybox/source/helpers/jquery.fancybox-thumbs.js"></script>

<link rel="stylesheet" href="${self.attr.rsc_path}resources/${self.attr.css_file}" type="text/css" media="screen" />
 
<!--  Add purl-JS URL parsing extension -->
<script src="${self.attr.rsc_path}resources/js/purl.js"></script>

<!--  add FontAwesome -->
<link rel="stylesheet" href="${self.attr.rsc_path}resources/css/font-awesome.min.css">

<!--  Add image holder library for missing plots -->
<script src="${self.attr.rsc_path}resources/js/holder.js"></script>

<!-- add History.js for HTML5 history manipulation -->
<script src="${self.attr.rsc_path}resources/js/history.js"></script>
<script src="${self.attr.rsc_path}resources/js/history.adapter.jquery.js"></script>

</%block>

<title>
% if pcontext.project_summary.proposal_code != '':
${pcontext.project_summary.proposal_code} -
% endif
<%block name="title">Untitled Page</%block></title>
</head>
<body>

<%block name="header">

<nav class="navbar navbar-default navbar-fixed-top hidden-print">
    <div class="container-fluid">
	    <div class="navbar-header">
			<button type="button" class="navbar-toggle collapsed" data-toggle="collapse" data-target="#navbar-collapse-1">
				<span class="sr-only">Toggle navigation</span>
				<span class="icon-bar"></span>
				<span class="icon-bar"></span>
				<span class="icon-bar"></span>
			</button>
	    	<a class="navbar-brand" href="#">
	    		<img alt="${pcontext.project_summary.telescope}" 
	    		     src="${self.attr.rsc_path}resources/img/${pcontext.project_summary.telescope.lower()}logo.png"/>
	    	</a>
	    </div>
        <div class="collapse navbar-collapse" id="navbar-collapse-1">
            <ul class="nav navbar-nav navbar-left">
                <li class="${'active' if self.attr.navbar_active == 'Home' else ''}">
                	<a href="t1-1.html"><span class="glyphicon glyphicon-home"></span> Home</a>
                </li>
                % if pcontext.logtype != 'GOUS':
                <li class="${'active' if self.attr.navbar_active == 'By Topic' else ''}">
                	<a href="t1-3.html">By Topic</a>
                </li>
                % endif
                <li class="${'active' if self.attr.navbar_active == 'By Task' else ''}">
                	<a href="t1-4.html">By Task</a>
                </li>
            </ul>
            % if pcontext.project_summary.proposal_code != '':
			<p class="navbar-text navbar-right navbar-projectcode">${pcontext.project_summary.proposal_code}</p>
	        % else:
			<p class="navbar-text navbar-right navbar-projectcode">Project Code N/A</p>
			% endif
        </div>
    </div>
</nav>
</%block>

<div id="app-container" class="container-fluid">
	${next.body()}
	
	<div class="footer">
		<%block name="footer"></%block>
	</div>
</div>

</body>
</html>
