<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>

% if plots:

<div class="page-header">
	<h1>Switched Power ${plots[0].parameters['type'].title()} Plots<button class="btn btn-default pull-right" onClick="javascript:window.history.back();">Back</button></h1>
</div>

% for ms in swpowspgain_subpages.keys():

    <h4>Plots:  <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, swpowspgain_subpages[ms]), pcontext.report_dir)}">SwPow SPgain plots </a> |
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, swpowtsys_subpages[ms]), pcontext.report_dir)}">SwPow Tsys plots</a>
    </h4>



%endfor

<br>

% for plot in sorted(plots, key=lambda p: p.parameters['ant']):
<div class="col-md-2 col-sm-3">
	<div class="thumbnail">
		<a data-fancybox="allplots"
		   href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
		   title="Antenna ${plot.parameters['ant']}">
			<img   src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
				 title="Antenna ${plot.parameters['ant']}"
				   alt="">
		</a>
		<div class="caption">
			<span class="text-center">Antenna ${plot.parameters['ant']}</span>
		</div>
	</div>
</div>
% endfor

%endif