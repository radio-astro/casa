<%!
import os
%>
<div class="page-header">
	<h1>Telescope Pointing Details for ${ms.basename}<button class="btn btn-default pull-right" onclick="javascript:window.history.back();">Back</button></h1>
</div>

<% 
def antenna_name(plot):
	return plot.parameters['antenna']
	
def intent(plot):
	return plot.parameters['intent'].capitalize()
	
def caption_string(plot):
    if plot.parameters['intent'].capitalize() == 'Target':
        return 'raster scan on source'
    else:
        return 'raster scan including reference'

# TODO: multi-source support
field_name = ms.get_fields(intent='TARGET')[0].name

def get_field_name(plot):
    field_attr = plot.parameters['field']
    if len(field_attr) == '':
        return field_name
    else:
        return field_attr.replace('"','')
%>

<div class="row">      
% if target_pointing is not None and len(target_pointing) > 0:
	% for plots in zip(target_pointing, whole_pointing):
		% for plot in plots:
			<div class="col-md-6">
				<a href="${os.path.relpath(plot.abspath, pcontext.report_dir)}" class="fancybox">
					<h3>Antenna ${antenna_name(plot)} Field ${get_field_name(plot)}</h3>
				</a>
				<div class="col-md-6">
				  	<div class="thumbnail">
						<a href="${os.path.relpath(plot.abspath, pcontext.report_dir)}"
						   class="fancybox">
							<img src="${os.path.relpath(plot.thumbnail, pcontext.report_dir)}"
								 title="Telescope pointing for antenna ${antenna_name(plot)}"
								 alt="Telescope pointing for antenna ${antenna_name(plot)}" />
					    </a>
				    	<div class="caption">
							<h4>${caption_string(plot)}</h4>
						</div>
					</div>
				</div>
			</div>
		% endfor
	% endfor
% endif
</div>










