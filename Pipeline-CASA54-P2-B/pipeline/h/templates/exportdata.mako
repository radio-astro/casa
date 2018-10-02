<%!
import os
%>

<%inherit file="t2-4m_details-base.mako"/>


<%block name="title">Data Products for Export</%block>

<h2>Results</h2>

<h4>Summary Documents, Scripts, and Logs</h4>

<p>Pipeline processing request, scripts, and logs</p>

<table class="table table-bordered table-striped" summary="Processing documents, scripts, and logs">
    <caption>Pipeline processing documents, scripts, and logs</caption>
    <thead>
	<tr>
	    <th scope="col">Description</th>
	    <th scope="col">File name</th>
	</tr>
    </thead>
    <tbody>
%for r in result:
      <tr>
          <td>Pipeline processing request</td>
          <td>${r.pprequest}</td>
      </tr>
      <tr>
          <td>Pipeline web log</td>
          <td>${r.weblog}</td>
      </tr>
      <tr>
          <td>Pipeline processing script</td>
          <td>${r.pipescript}</td>
      </tr>
      <tr>
          <td>Pipeline restore script</td>
          <td>${r.restorescript}</td>
      </tr>
      <tr>
          <td>CASA commands log</td>
          <td>${r.commandslog}</td>
      </tr>
%endfor
    </tbody>
</table>

%if result[0].inputs['exportmses']:

<h4>Final Measurement Sets</h4>

<p>Per ASDM, compressed tar file of final measurement set </p>

<table class="table table-bordered table-striped" summary="Per ASDM Measurement Set Tarfile">
    <caption>Final measurement set</caption>
    <thead>
	<tr>
	    <th scope="col">Measurement Set</th>
	    <th scope="col">Session</th>
	    <th scope="col">Final Tarfile</th>
	</tr>
    </thead>
%else:
<h4>Calibration Instructions and Final Flags</h4>

<p>Per ASDM, text file of applycal instructions and compressed tar file of final flags </p>

<table class="table table-bordered table-striped" summary="Per ASDM calibration and flagging">
    <caption>Applycal instructions and final flags</caption>
    <thead>
	<tr>
	    <th scope="col">Measurement Set</th>
	    <th scope="col">Session</th>
	    <th scope="col">Applycal Instructions</th>
	    <th scope="col">Final Flags</th>
	</tr>
    </thead>
%endif

    <tbody>

%if result[0].inputs['exportmses']:
%for r in result:
    %for session in r.sessiondict:
        %for vis in r.sessiondict[session][0]:
      <tr>
          <td>${vis}</td>
          <td>${session}</td>
          <td>${r.visdict[vis]}</td>
      </tr>
        %endfor
    %endfor
%endfor
%else:
%for r in result:
    %for session in r.sessiondict:
        %for vis in r.sessiondict[session][0]:
      <tr>
          <td>${vis}</td>
          <td>${session}</td>
          <td>${r.visdict[vis][1]}</td>
          <td>${r.visdict[vis][0]}</td>
      </tr>
        %endfor
    %endfor
%endfor
% endif
    </tbody>
</table>

% if not result[0].inputs['exportmses']:

<h4>Calibration Tables</h4>

<p>Per observing session, compressed tar file of the final calibration tables</p>

<table class="table table-bordered table-striped" summary="Per session calibration tables">
    <caption>Final calibration tables</caption>
    <thead>
	<tr>
	    <th scope="col">Session</th>
	    <th scope="col">Calibration Tables</th>
	</tr>
   </thead>
   <tbody>
%for r in result:
    %for session in r.sessiondict:
      <tr>
          <td>${session}</td>
          <td>${r.sessiondict[session][1]}</td>
      </tr>
    %endfor
%endfor
   </tbody>
</table>
% endif

<h4>Calibrator Source Images</h4>

<p>FITS files of all the calibrator source images</p>

<table class="table table-bordered table-striped" summary="Calibrator source images">
    <caption>Final calibrator images</caption>
    <thead>
	<tr>
	    <th scope="col">Source name</th>
	    <th scope="col">Source type</th>
	    <th scope="col">Spw</th>
	    <th scope="col">FITS file</th>
	</tr>
   </thead>
   <tbody>
%for r in result:
    %for calimage in r.calimages[0]:
        %for fitsfile in calimage['fitsfiles']:
          <tr>
              <td>${calimage['sourcename']}</td>
              <td>${calimage['sourcetype']}</td>
              <td>${calimage['spwlist']}</td>
              <td>${os.path.basename(fitsfile)}</td>
          </tr>
        %endfor
    %endfor
%endfor
   </tbody>
</table>

<h4>Auxiliary Calibrator Source Images</h4>

<p>FITS files of all the auxiliary calibrator source images</p>

<table class="table table-bordered table-striped" summary="Auxiliary calibrator source images">
    <thead>
	<tr>
	    <th scope="col">Source name</th>
	    <th scope="col">Source type</th>
	    <th scope="col">Spw</th>
	    <th scope="col">FITS file</th>
	</tr>
   </thead>
   <tbody>
%for r in result:
    %for calimage in r.calimages[0]:
        %for fitsfile in calimage['auxfitsfiles']:
          <tr>
              <td>${calimage['sourcename']}</td>
              <td>${calimage['sourcetype']}</td>
              <td>${calimage['spwlist']}</td>
              <td>${os.path.basename(fitsfile)}</td>
          </tr>
        %endfor
    %endfor
%endfor
   </tbody>
</table>

<h4>Target Source Images</h4>

<p>FITS files of all the target source images</p>

<table class="table table-bordered table-striped" summary="Target source images">
    <thead>
	<tr>
	    <th scope="col">Source name</th>
	    <th scope="col">Source type</th>
	    <th scope="col">Spw</th>
	    <th scope="col">FITS file</th>
	</tr>
   </thead>
   <tbody>
%for r in result:
    %for targetimage in r.targetimages[0]:
        %for fitsfile in targetimage['fitsfiles']:
          <tr>
              <td>${targetimage['sourcename']}</td>
              <td>${targetimage['sourcetype']}</td>
              <td>${targetimage['spwlist']}</td>
              <td>${os.path.basename(fitsfile)}</td>
          </tr>
        %endfor
    %endfor
%endfor
   </tbody>
</table>

<h4>Auxiliary Target Source Images</h4>

<p>FITS files of all the auxiliary target source images</p>

<table class="table table-bordered table-striped" summary="Auxiliary Target source images">
    <thead>
	<tr>
	    <th scope="col">Source name</th>
	    <th scope="col">Source type</th>
	    <th scope="col">Spw</th>
	    <th scope="col">FITS file</th>
	</tr>
   </thead>
   <tbody>
%for r in result:
    %for targetimage in r.targetimages[0]:
        %for fitsfile in targetimage['auxfitsfiles']:
          <tr>
              <td>${targetimage['sourcename']}</td>
              <td>${targetimage['sourcetype']}</td>
              <td>${targetimage['spwlist']}</td>
              <td>${os.path.basename(fitsfile)}</td>
          </tr>
        %endfor
    %endfor
%endfor
   </tbody>
</table>
