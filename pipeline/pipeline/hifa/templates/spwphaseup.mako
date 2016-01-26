<%!
rsc_path = ""
import os

def choose_spwmap(combine_spwmap, phaseup_spwmap):
    if combine_spwmap:
        return combine_spwmap
    else:
        return phaseup_spwmap

def format_spwmap(phaseup_spwmap):
    if not phaseup_spwmap:
        return ''
    else:
        return ', '.join([str(spwid) for spwid in phaseup_spwmap])
%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="title">Compute Spw Phaseup Map and Offsets</%block>

<p>Compute the phaseup spw map and spectral window phase offsets caltable</p>

<table class="table table-bordered table-striped" summary="Narrow to wide spw mapping results">
	<caption>Phaseup spw map per measurement set.</caption>
        <thead>
	    <tr>
	        <th>Measurement Set</th>
	        <th>Phaseup Spw Map</th>
	    </tr>
	</thead>
	<tbody>
    % for r in result:
		<tr>
			<td>${os.path.basename(r.vis)}</td>
			<td>${format_spwmap(choose_spwmap(r.combine_spwmap, r.phaseup_spwmap))}</td>
		</tr>
    % endfor
	</tbody>
</table>
