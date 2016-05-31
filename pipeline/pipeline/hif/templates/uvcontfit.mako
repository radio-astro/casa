<%!
rsc_path = ""
import os

%>
<%inherit file="t2-4m_details-base.html"/>

<%block name="title">Compute UV Continuum Subtraction Coefficients</%block>

<p>This task computes the UV continuum model that that will be subtracted from the
science target data prior to cube imaging</p>

<h2>Results</h2>

<table class="table table-bordered" summary="Application Results">
        <caption>Applied calibrations and parameters used for caltable generation</caption>
    <thead>
        <tr>
            <th scope="col" rowspan="2">Measurement Set</th>
                        <th scope="col" colspan="3">Solution Parameters</th>
                        <th scope="col" colspan="2">Applied To</th>
            <th scope="col" rowspan="2">Calibration Table</th>
        </tr>
        <tr>
                        <th>Frequency Ranges (TOPO)</th>
                        <th>Interval</th>
                        <th>Fit Order</th>
                        <th>Source Intent</th>
                        <th>Spectral Window</th>
        </tr>
    </thead>
        <tbody>
                % for tr in table_rows:
                <tr>
                    % for td in tr:
                        ${td}
                    % endfor
                </tr>
                % endfor
        </tbody>
</table>

