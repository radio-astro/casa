<%!
rsc_path = "../"
import collections
import os
import pipeline.infrastructure.utils as utils

def get_fraction(flagged, total):
   if total == 0 or flagged < 0.0:
       return 'N/A'
   else:
       return '%0.1f%%' % (100.0 * float(flagged) / float(total))

FlagDetailTR = collections.namedtuple("FlagDetailTR", "name spw ant pol nrow totnrow totfrac tsys weather user before postbl prebl postrmean prermean postrms prerms link")

def make_detailed_table(result, stage_dir, fieldname):
   rel_path = os.path.basename(stage_dir)   ### stage#
   rows = []
   for r in result:
       summaries = r.outcome['summary']
       for summary in summaries:
           if summary['field'] != fieldname:
               continue
           html_name = summary['html']
           asdm_name = summary['name']
           ant_name = summary['antenna']
           spw = summary['spw']
           pol = summary['pol']
           nrows = summary['nrow']
           flags = summary['nflags']
           cell_elems = [asdm_name, spw, ant_name, pol, nrows, flags[0]]
           for nflg in flags:
               cell_elems.append(get_fraction(nflg, nrows))
           htext = '<a class="replace-pre" href="%s">details</a>' % (os.path.join(rel_path, html_name),)
           cell_elems.append(htext)
           trow = FlagDetailTR(*cell_elems)
           rows.append(trow)
   if len(rows) == 0:
       return []
   return utils.merge_td_columns(rows, num_to_merge=4)
%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="header" />

<%block name="title">Flag data by Tsys, weather, and statistics of spectra</%block>

<%
try:
   stage_number = result.stage_number
   stage_dir = os.path.join(pcontext.report_dir,'stage%d'%(stage_number))
   if not os.path.exists(stage_dir):
       os.mkdir(stage_dir)

   trim_name = lambda s : s if not s.startswith('"') or not s.endswith('"') else s[1:-1]
   unique_fields = []
   for r in result:
       summaries = r.outcome['summary']
       for summary in summaries:
            if summary['field'] not in unique_fields:
                unique_fields.append(summary['field'])

   flag_types = ['Total', 'Tsys', 'Weather', 'User', 'After calibration']
   fit_flags = ['Baseline RMS', 'Running mean', 'Expected RMS']
except Exception, e:
   print 'hsd_imaging html template exception:', e
   raise e
%>

<!-- short description of what the task does -->
<p>This task flags spectra by several criteria:
<ol>
	<li> eliminate spectra with outlier RMS (Baseline RMS)</li>
	<li> eliminate rapid variation of spectra using deviation from the running mean (Running mean)</li>
	<li> eliminate spectra with remarkably large RMS than expected (Expected RMS)</li>
	<li> eliminate spectra with outlier Tsys value</li>
	<li> by weather (currently disabled)</li>
	<li> by user defined threshold (currently disabled)</li>
</ol>
For 1.-3., the RMSes of spectra before and after baseline fit are obtained using line free channels.
</p>

<h2>Contents</h2>
<ul>
<li><a href="#summarytable">Flag Summary</a></li>
<li><a href="#detailtable">Flag by Reason</a></li>
  <ul>
%for field in unique_fields:
	<li><a href="#${trim_name(field)}">${field}</a></li>
%endfor
  </ul>
</ul>


<H2 id="summarytable" class="jumptarget">Flag Summary</H2>
<table class="table table-bordered table-striped" summary="Flag Summary">
	<caption>flag summary of ON-source target scans per source and spw</caption>
    <thead>
	    <tr>
	        <th scope="col" rowspan="2">Field</th>
	        <th scope="col" rowspan="2">SpW</th>
	        <th scope="col" colspan="3">Flagged Fraction</th>
		</tr>
		<tr>
	        <th scope="col">Before</th>
	        <th scope="col">Additional</th>
	        <th scope="col">Total</th>
	    </tr>
	</thead>
	<tbody>
	% for tr in sumary_table_rows:
		<tr>
		% for td in tr:
			${td}
		% endfor
		</tr>
	%endfor
	</tbody>
</table>


<H2 id="detailtable" class="jumptarget">Flag by Reason</H2>
%for field in unique_fields:
	<H3 id="${trim_name(field)}" class="jumptarget">${field}</H3>
	<table class="table table-bordered table-striped " summary="${field}">
	<thead>
		<tr>
			<th colspan="5">Data Selection</th>
			<th colspan="2">Flagged Total</th>
			%for ftype in flag_types[1:]:
				<th rowspan="2">${ftype}</th>
			%endfor
			%for fflag in fit_flags:
				<th colspan="2">${fflag}</th>
			%endfor
			<th rowspan="2">Plots</th>
		</tr>
		<tr>
			<th>Name</th><th>spw</th><th>Ant.</th><th>Pol</th><th># of rows</th>
			<th>row #</th><th>fraction</th>
			%for fflag in fit_flags:
				<th>post-fit</th><th>pre-fit</th>
			%endfor
		</tr>
		</thead>
		<tbody>
		% for tr in make_detailed_table(result, stage_dir, field):
			<tr>
			% for td in tr:
				${td}
			% endfor
			</tr>
		%endfor <!-- end of table row loop -->
		</tbody>
		</table>
	%endfor <!-- end of per field loop -->
