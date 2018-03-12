<%!
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<html>
<body>

<div class="page-header">
	<h1>Spatial Setup Details<button class="btn btn-default pull-right" onclick="javascript:window.history.back();">Back</button></h1>
</div>

<h2>Sources</h2>
<table class="table table-bordered table-striped table-condensed" summary="Sources in ${ms.basename}">
	<caption>Sources in ${ms.basename}</caption>
    <thead>
        <tr>
            <th scope="col" rowspan="2">ID</th>
            <th scope="col" rowspan="2">Source Name</th>
            <th scope="col" colspan="3">Source Position</th>
            <th scope="col" colspan="2">Proper Motion</th>
            <th scope="col" rowspan="2"># Pointings</th>
            <th scope="col" rowspan="2">Intent</th>
        </tr>
        <tr>
            <th scope="col">RA</th>
            <th scope="col">Dec</th>
            <th scope="col">Ref. Frame</th>
            <th scope="col">X</th>
            <th scope="col">Y</th>
        </tr>
    </thead>
	<tbody>
	% for source in ms.sources:
		<% num_pointings = len([f for f in ms.fields if f.source_id == source.id]) %>
		% if num_pointings:
		<tr>
		  <td>${source.id}</td>
		  <td>${source.name}</td>
		  <td>${source.ra}</td>
		  <td>${source.dec}</td>
		  <td>${source.frame}</td>
		  <td>${source.pm_x}</td>
		  <td>${source.pm_y}</td>
		  <td>${num_pointings}</td>
		  <td>${', '.join(sorted([i for i in source.intents]))}</td>
		</tr>
		% endif
	% endfor
	</tbody>
</table>

<h2>Fields</h2>

<table class="table table-bordered table-striped table-condensed" summary="Fields defined in ${ms.basename}">
	<caption>Fields in ${ms.basename}</caption>
    <thead>
        <tr>
            <th scope="col" rowspan="2">Field ID</th>
            <th scope="col" rowspan="2">Field Name</th>
            <th scope="col" colspan="3">Position</th>
            <th scope="col" rowspan="2">Intent</th>
            <th scope="col" rowspan="2">Source Reference</th>
        </tr>
        <tr>
            <th scope="col">RA</th>
            <th scope="col">Dec</th>
            <th scope="col">Ref. Frame</th>
        </tr>
    </thead>
	<tbody>
	% for field in ms.fields:
		<% sources = [s for s in ms.sources if s.id == field.source_id] %>
		<tr>
		  <td>${field.id}</td>		
		  <td>${field.name}</td>
		  <td>${field.ra}</td>
		  <td>${field.dec}</td>
		  <td>${field.frame}</td>
		  <td>${', '.join(sorted([i for i in field.intents]))}</td>
		% if len(sources) == 1:
		  <td>${sources[0].name} (#${field.source_id})</td>
		% else:
		  <td>N/A (#${field.source_id})</td>
		% endif		
		</tr>
	% endfor
	</tbody>
</table>

% if mosaics:
<h2>Mosaic Pointings</h2>	
<ul class="thumbnails">
	% for (source, plot) in mosaics:
	<li>
		<div class="thumbnail">
			<a href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
			   data-fancybox>
				<img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
					 title="Mosaic Pointings for ${source.name} in ${ms.basename}"
					 alt="Mosaic Pointings for ${source.name} in ${ms.basename}" />
			</a>
			<div class="caption">
				<h4>${source.name}</h4>
				<p>Mosaic pointings for Source #${source.id}: ${source.name}.</p>
			</div>				
		</div>
	</li>
	% endfor
</ul>
% endif

</body>
</html>