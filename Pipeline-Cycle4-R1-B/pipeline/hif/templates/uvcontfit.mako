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
                        <th scope="col" colspan="2">Solution Parameters</th>
                        <th scope="col" colspan="2">Applied To</th>
            <th scope="col" rowspan="2">Calibration Table</th>
        </tr>
        <tr>
                        <th>Interval</th>
                        <th>Fit Order</th>
                        <th>Scan Intent</th>
                        <th>Spectral Windows</th>
        </tr>
    </thead>
        <tbody>
% for application in applications:
                <tr>
                        <td>${application.ms}</td>
                        <td>${application.solint}</td>
                        <td>${application.fitorder}</td>
                        <td>${application.intent}</td>
                        <td>${application.spw}</td>
                        <td>${application.gaintable}</td>
                </tr>
% endfor
        </tbody>
</table>

