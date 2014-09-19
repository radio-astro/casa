<%!
rsc_path = ""
import os

def format_spwmap (phaseup_spwmap):
    if not phaseup_spwmap:
        return ''
    else:
        return ','.join([str(spwid) for spwid in phaseup_spwmap]).replace(',', ', ')
%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="title">Map Narrow to Wide Spectral Windows</%block>

<p>Map narrow to wide spectral windows and compute a per spectral window phase offsets caltable</p>

<div class="row-fluid">
    <table class="table table-bordered table-striped" summary="Narrow to wide spw mapping results">
	<caption>Narrow to wide spw map per measurement set.</caption>
        <thead>
	    <tr>
	        <th>Measurement Set</th>
	        <th>Spectral Window Map</th>
	    </tr>
	</thead>
	<tbody>
            %for r in result:
		<tr>
			<td>${os.path.basename(r.vis)}</td>
			<td>${format_spwmap(r.phaseup_spwmap)}</td>
		</tr>
            %endfor
	</tbody>
    </table>
</div>
