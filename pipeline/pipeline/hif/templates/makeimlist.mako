<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr

def num_targets (result):
    ntargets = 0
    for r in result:
        ntargets = ntargets + len(r.targets)
    return ntargets

def no_clean_targets(result):
    no_clean_targets = True
    for r in result:
        if r.clean_list_info == {}:
            continue
        no_clean_targets = False
        break
    return no_clean_targets

def get_message(result):
    message = ""
    for r in result:
        message = r.clean_list_info.get('msg', '')
        break
    return message
%>

<%inherit file="t2-4m_details-base.mako"/>

<%block name="header" />

<%block name="title">Make image list<br><small>${result[0].metadata['long description']}</small></%block>

<h2>List of Clean Targets</h2>

%if num_targets(result) <= 0:
    %if no_clean_targets(result):
        <p>There are no clean targets.
    %else:
        <p>${get_message(result)}
    %endif
%else:
    <%
    target = result[0].targets[0]
    %>

    <table class="table table-bordered table-striped">
		<caption>Clean Targets Summary</caption>
		<thead>
	        <tr>
	            <th>field</th>
	            <th>intent</th>
	            <th>spw</th>
	            <th>phasecenter</th>
	            <th>cell</th>
	            <th>imsize</th>
	            <th>imagename</th>
	    %if 'specmode' in target.keys():
	            <th>specmode</th>
	    %endif
	    %if 'start' in target.keys():
	            <th>start</th>
	    %endif
	    %if 'width' in target.keys():
	            <th>width</th>
	    %endif
	    %if 'nbin' in target.keys():
	            <th>nbin</th>
	    %endif
	    %if 'nchan' in target.keys():
	            <th>nchan</th>
	    %endif
	    %if 'restfreq' in target.keys():
	            <th>restfreq (LSRK)</th>
	    %endif
	    %if 'weighting' in target.keys():
	            <th>weighting</th>
	    %endif
	    %if 'robust' in target.keys():
	            <th>robust</th>
	    %endif
	    %if 'noise' in target.keys():
	            <th>noise</th>
	    %endif
	    %if 'npixels' in target.keys():
	            <th>npixels</th>
	    %endif
	    %if 'restoringbeam' in target.keys():
	            <th>restoringbeam</th>
	    %endif
	    %if 'nterms' in target.keys():
	            <th>nterms</th>
	    %endif
	    %if 'uvrange' in target.keys():
	            <th>uvrange</th>
	    %endif
	    %if 'maxthreshiter' in target.keys():
	            <th>maxthreshiter</th>
	    %endif
	        </tr>
		</thead>
		<tbody>
        %for r in result:
	    %for target in r.targets:
	        <tr>
	            <td>${target['field']}</td>
	            <td>${target['intent']}</td>
	            <td>${target['spw']}</td>
	            <td>${target['phasecenter']}</td>
	            <td>${target['cell']}</td>
	            <td>${target['imsize']}</td>
	            <td>${os.path.basename(target['imagename'])}</td>
	        %if 'specmode' in target.keys():
	                <td>${target['specmode']}</td>
	        %endif
	        %if 'start' in target.keys():
	                <td>${target['start']}</td>
	        %endif
	        %if 'width' in target.keys():
	                <td>${target['width']}</td>
	        %endif
	        %if 'nbin' in target.keys():
	                <td>${target['nbin']}</td>
	        %endif
	        %if 'nchan' in target.keys():
	                <td>${target['nchan']}</td>
	        %endif
	        %if 'restfreq' in target.keys():
	                <td>${target['restfreq']}</td>
	        %endif
	        %if 'weighting' in target.keys():
	                <td>${target['weighting']}</td>
	        %endif
	        %if 'robust' in target.keys():
	                <td>${target['robust']}</td>
	        %endif
	        %if 'noise' in target.keys():
	                <td>${target['noise']}</td>
	        %endif
	        %if 'npixels' in target.keys():
	                <td>${target['npixels']}</td>
	        %endif
	        %if 'restoringbeam' in target.keys():
	                <td>${target['restoringbeam']}<td>
	        %endif
	        %if 'uvrange' in target.keys():
	                <td>${target['uvrange']}</td>
	        %endif
	        %if 'maxthreshiter' in target.keys():
	                <td>${target['maxthreshiter']}</td>
	        %endif
			</tr>
	    %endfor
        %endfor
		</tbody>
	</table>
%endif
