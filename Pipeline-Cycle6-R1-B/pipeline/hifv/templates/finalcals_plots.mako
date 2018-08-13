<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>

<div class="page-header">
	<h1>${plots[0].parameters['type'].title()} plots<button class="btn btn-default pull-right" onClick="javascript:window.history.back();">Back</button></h1>
</div>

% for ms in finaldelay_subpages.keys():
    <h4>Plots: <br> <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, finaldelay_subpages[ms]), pcontext.report_dir)}">Final delay plots </a> |
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, phasegain_subpages[ms]), pcontext.report_dir)}">BP initial gain phase </a> |
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, bpsolamp_subpages[ms]), pcontext.report_dir)}">BP Amp solution </a> |
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, bpsolphase_subpages[ms]), pcontext.report_dir)}">BP Phase solution </a> |
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, bpsolphaseshort_subpages[ms]), pcontext.report_dir)}">Phase (short) gain solution</a> |
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, finalamptimecal_subpages[ms]), pcontext.report_dir)}">Final amp time cal </a> |
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, finalampfreqcal_subpages[ms]), pcontext.report_dir)}">Final amp freq cal </a> |
        <a class="replace"
           href="${os.path.relpath(os.path.join(dirname, finalphasegaincal_subpages[ms]), pcontext.report_dir)}">Final phase gain cal </a> 
        
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
			<p class="text-center">Antenna ${plot.parameters['ant']}</p>
		</div>
	</div>
</div>
% endfor
