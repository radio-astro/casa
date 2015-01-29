<%!
rsc_path = ""
import os

columns = {'cleanmask' : 'Clean Mask',
		   'flux' : 'Flux',
		   'image' : 'Image',
		   'residual' : 'Residual',
		   'model' : 'Final Model',
		   'psf' : 'PSF'}

colorder = ['image', 'residual', 'cleanmask']

def get_plot(plots, field, spw, i, colname):
	try:
		return plots[field][spw][i][colname]
	except KeyError:
		return None
%>

<script>
$(document).ready(function() {
	$(".fancybox").fancybox({
        type: 'image',
        prevEffect: 'none',
        nextEffect: 'none',
        loop: false,
        helpers: {
            title: {
                type: 'outside'
            },
            thumbs: {
                width: 50,
                height: 50,
            }
        },
    	beforeShow : function() {
        	this.title = $(this.element).attr('title');
       	}    
    }); 
 });
</script>

<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>

<div class="page-header">
        <h2>Clean results for ${field} SpW ${spw} <button class="btn btn-default pull-right" onClick="javascript:location.reload();">Back</button></h2>
</div>

<!-- column headings -->
<div class="row">
    <!-- iteration column heading -->
    <div class="col-md-2">
        <h4 class="text-center">Iteration</h4>
    </div>
    <!-- headings for columns containing plots -->
    <div class="col-md-10">
    % for colname in colorder:
        <div class="col-md-4">
            <h4 class="text-center">${columns[colname]}</h4>
        </div>
    % endfor
    </div>
</div><!-- /div row-fluid -->

<!-- reverse iterations so final images are shown without scrolling -->
% for i in sorted(plots_dict[field][spw].keys())[::-1]:
<div class="row">
    <!-- iteration row heading -->
    <div class="col-md-2">
        <h4 class="text-center">${i}</h4>
    </div>
    <!-- plots for this iteration, in column order -->
    <div class="col-md-10">
        % for colname in colorder:
        <div class="col-md-4">
            <% plot = get_plot(plots_dict, field, spw, i, colname) %>
            <!-- use bootstrap markup for thumbnails -->
                <!-- span 12 because the parent div has shrunk this
                     container already -->					  
                <div class="col-md-12">
                    % if plot is not None:
                    <div class="thumbnail">
                        <a class="fancybox"
                           href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
                           title="Iteration ${i}: ${columns[colname]}"
                           data-thumbnail="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
                           rel="${colname}">
                           <img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
                           		title="Iteration ${i}: ${columns[colname]}"
                           		alt="Iteration ${i}: ${columns[colname]}">
                        </a>
                    </div>
                    % endif
                </div>
        </div>					
        % endfor <!-- /colname loop-->
    </div>
</div><!-- /div row-fluid -->			
% endfor <!-- /iteration loop -->

<div class="row">
    <div class="col-md-10 col-md-offset-2">
    % for colname in ['flux', 'psf', 'model']:
        <div class="col-md-4">
            <!-- flux and PSF plots are associated with iteration 0 -->
            % if colname == 'model':
                <% 
                lastiter = sorted(plots_dict[field][spw].keys())[-1]
                plot = get_plot(plots_dict, field, spw, lastiter, colname)
                %>
            % else:
                <% plot = get_plot(plots_dict, field, spw, 0, colname) %>
            % endif
                <div class="col-md-12">
                % if plot is not None:
                    <div class="thumbnail">
                        <a class="fancybox"
                           href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
                           title="${columns[colname]}"
                           data-thumbnail="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}">									   
							<img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
	                             title="${columns[colname]}"
    		                     alt="${columns[colname]}">
                        </a>
						<div class="caption">
							<p class="text-center">${columns[colname]}</p>
                     	</div>
               		</div>
                % endif
                </div>
        </div><!-- /div span4 -->
    % endfor <!-- /colname loop -->
	</div>
</div>
