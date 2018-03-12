<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>

<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>

<div class="page-header">
	<h1>Semi-finalcals ${plots[0].parameters['type'].title()} Plots<button class="btn btn-default pull-right" onClick="javascript:window.history.back();">Back</button></h1>
</div>

% for ms in delay_subpages.keys():
    <h4>Plots:  <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, delay_subpages[ms]), pcontext.report_dir)}">Delay plots </a>|
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, phasegain_subpages[ms]), pcontext.report_dir)}">Gain phase </a>|
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, bpsolamp_subpages[ms]), pcontext.report_dir)}">BP Amp solution </a>|
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, bpsolphase_subpages[ms]), pcontext.report_dir)}">BP Phase solution </a>
    </h4>
%endfor

<br>


% for plot in sorted(plots, key=lambda p: p.parameters['ant']):
<div class="col-md-2 col-sm-3">
	<div class="thumbnail">
		<a data-fancybox="allplots"
		   href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
		   title="Antenna ${plot.parameters['ant']} ">
			<img   src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
				 title="Antenna ${plot.parameters['ant']} "
				   alt="">
		</a>
		<div class="caption">
			<span class="text-center">Antenna ${plot.parameters['ant']}</span>
		</div>
	</div>
</div>
% endfor