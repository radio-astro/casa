<%!
rsc_path = ""
import os

columns = {'cleanmask' : ('Clean Mask', 'Clean Mask'),
		   'flux' : ('Primary Beam', 'Primary Beam'),
		   'pbcorimage' : ('Image', 'Image'),
		   'residual' : ('Residual', 'Residual'),
		   'model' : ('Final Model', 'Final Model'),
		   'psf' : ('PSF', 'PSF'),
		   'mom0_fc': ('MOM0_FC', 'MOM0_FC: integrated intensity (moment 0) of line-free channels after continuum subtraction')}

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
        <h2>Clean results for ${field} SpW ${spw} <button class="btn btn-default pull-right" onClick="javascript:window.history.back();">Back</button></h2>
</div>

<div class="row">
<table class="table table-striped">
	<thead>
		<tr>
			<th>Iteration</th>
		    % for colname in colorder:
	        	<th>${columns[colname][0]}</th>
		    % endfor
		</tr>
	</thead>
	<tbody>

		% for i in sorted(plots_dict[field][spw].keys())[::-1]:
		<tr>
		    <!-- iteration row heading -->
		    <td class="vertical-align"><p class="text-center">${i}</p></td>
		    <!-- plots for this iteration, in column order -->
	        % for colname in colorder:
	        <td>
	            <% plot = get_plot(plots_dict, field, spw, i, colname) %>
	            <!-- use bootstrap markup for thumbnails -->
	            % if plot is not None:
	            <div class="thumbnail">
	                <a class="fancybox"
	                   href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
	                   title="Iteration ${i}: ${columns[colname][1]}"
	                   % if colname in ['pbcorimage', 'residual']:
	                   rel="iteration"                   
	                   % endif	
	                   data-thumbnail="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}">
	                   <img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
	                   		title="Iteration ${i}: ${columns[colname][1]}"
	                   		alt="Iteration ${i}: ${columns[colname][1]}"
	                   		class="img-responsive">
	                </a>
	            </div>
	            % endif
	        </td>
	        % endfor <!-- /colname loop-->
	    </tr>
		% endfor <!-- /iteration loop -->

		<tr>
			<td></td>
		    % for colname in ['flux', 'psf', 'model']:
		    	<td>
		            % if colname == 'model':
			            <!-- model plots are associated with the final iteration -->
		                <% 
		                lastiter = sorted(plots_dict[field][spw].keys())[-1]
		                plot = get_plot(plots_dict, field, spw, lastiter, colname)
		                %>
		            % else:
			            <!-- flux and PSF plots are associated with iteration 0 -->
		                <% plot = get_plot(plots_dict, field, spw, 0, colname) %>
		            % endif
		            % if plot is not None:
		                <div class="thumbnail">
		                    <a class="fancybox"
		                       href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
		                       title="${columns[colname][1]}"
		                       data-thumbnail="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}">									   
								<img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
									 title="${columns[colname][1]}"
								     alt="${columns[colname][1]}"]
								     class="img-responsive">
							</a>
							<div class="caption">
								<p class="text-center">${columns[colname][1]}</p>
		               		</div>
		         		</div>
		            % endif
		        </td>
		    % endfor <!-- /colname loop -->			
		</tr>

	</tbody>
</table>
</div>

<%doc>
<div class="row">
    <div class="col-md-11 col-md-offset-1">
    % for colname in ['flux', 'psf', 'model']:
        <div class="col-md-4">
            % if colname == 'model':
	            <!-- model plots are associated with the final iteration -->
                <% 
                lastiter = sorted(plots_dict[field][spw].keys())[-1]
                plot = get_plot(plots_dict, field, spw, lastiter, colname)
                %>
            % else:
	            <!-- flux and PSF plots are associated with iteration 0 -->
                <% plot = get_plot(plots_dict, field, spw, 0, colname) %>
            % endif
            % if plot is not None:
                <div class="thumbnail">
                    <a class="fancybox"
                       href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
                       title="${columns[colname][1]}"
                       data-thumbnail="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}">									   
						<img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
							 title="${columns[colname][1]}"
						     alt="${columns[colname][1]}">
					</a>
					<div class="caption">
						<p class="text-center">${columns[colname][1]}</p>
               		</div>
         		</div>
            % endif
        </div><!-- /div span4 -->
    % endfor <!-- /colname loop -->
	</div>
</div>
</%doc>
