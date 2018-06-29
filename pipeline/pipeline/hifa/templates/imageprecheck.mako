<%!
rsc_path = ""
import os
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">Image Pre-Check</%block>

<%
cqa = casatools.quanta
real_repr_target = result[0].real_repr_target
repr_source = result[0].repr_source
repr_spw = '%s' % (result[0].repr_spw)
reprBW_mode = '%s' % (result[0].reprBW_mode)
reprBW_nbin = '%d' % (result[0].reprBW_nbin)
repr_freq = '%.4f GHz' % (cqa.getvalue(cqa.convert(result[0].repr_target[1], 'GHz')))
repr_bw = '%.4g MHz' % (cqa.getvalue(cqa.convert(result[0].repr_target[2], 'MHz')))
sens_bw = '%.4g MHz' % (cqa.getvalue(cqa.convert(result[0].sensitivity_bandwidth, 'MHz')))
minAR_v = cqa.getvalue(cqa.convert(result[0].minAcceptableAngResolution, 'arcsec'))
maxAR_v = cqa.getvalue(cqa.convert(result[0].maxAcceptableAngResolution, 'arcsec'))
minAR = '%#.3g arcsec' % (minAR_v)
maxAR = '%#.3g arcsec' % (maxAR_v)
sensitivityGoal_v = cqa.getvalue(cqa.convert(result[0].sensitivityGoal, 'mJy'))
sensitivityGoal = '%#.3g mJy' % (sensitivityGoal_v)
robust = '%.1f' % (result[0].hm_robust)
uvtaper = '%s' % (result[0].hm_uvtaper)
single_continuum = result[0].single_continuum
%>
<p>
%if real_repr_target:
<h4>Goals From OT:</h4>
%else:
<h4>Assumed Values (Goal information not available):</h4>
%endif
Representative Target: ${repr_source}
%if not real_repr_target:
(First science target)
%endif
<br>
Representative Frequency: ${repr_freq} (SPW ${repr_spw})
%if not real_repr_target:
(Center of first science spw)
%endif
<br>
Bandwidth for Sensitivity: ${repr_bw}
%if real_repr_target:
    %if reprBW_mode=='nbin' or reprBW_mode=='repr_spw':
(rounded to nearest integer #channels (${reprBW_nbin}), repBW = ${sens_bw})
    %endif
%else:
(repBW=${repr_bw}, channel width of first science spw)
%endif
<br>
Min / Max Acceptable Resolution:
%if minAR_v==0.0 and maxAR_v==0.0:
    Not available
%else:
    ${minAR} / ${maxAR}
%endif
<br>
Goal PI sensitivity:
%if real_repr_target:
    %if sensitivityGoal_v==0.0:
        Not available
    %else:
        ${sensitivityGoal}
    %endif
%else:
    Not available
%endif
<br>
Single Continuum:
%if real_repr_target:
    ${single_continuum}
%else:
    Not available
%endif

<h4>Estimated Synthesized Beam and Sensitivities for the Representative
Target/Frequency</h4>
Estimates are given for three values of the tclean robust weighting
parameter, note that robust=+0.5 is always used for subsequent imaging
stages (values highlighted in green), estimates for robust=-0.5 and +2.0
are informative only. The percent difference between Min/Max acceptable
resolution and the geometric mean of the estimated beams are shown in the
%%Diff minAR / maxAR column. In addition to an estimate for the repBW, an
estimate for the aggregate continuum bandwidth (aggBW) is also given
assuming NO line contamination and NO spectral overlap between spws.
These estimates account for Tsys, the observed uv-coverage, and prior
flagging.
<p>
<b>These estimates should always be considered as the BEST CASE SCENARIO.</b>
The estimates DO NOT account for (1) subsequent science target flagging;
(2) loss of continuum bandwidth due to the hif_findcont process (i.e. removal
of lines and other spectral features from the data used to image the
continuum); (3) Issues that affect the image quality like (a) poor match of
uv-coverage to image complexity; (b) dynamic range effects; (c) calibration
deficiencies (poor phase transfer, residual baseline based effects, residual
antenna position errors etc.).

<table class="table">
    <thead>
        <tr>
            <th>robust</th>
            <th>uvtaper</th>
            <th>Synthesized Beam</th>
            <th>%Diff minAR / maxAR</th>
            <th>cell</th>
            <th>bandwidth</th>
            <th>bwmode</th>
            <th>Effective Sensitivity</th>
        </tr>
    </thead>
    <tbody>
        %for tr in table_rows:
            %if tr.robust==result[0].hm_robust and tr.uvtaper==result[0].hm_uvtaper:
            <tr bgcolor="lightgreen">
            %else:
            <tr>
            %endif
            %for td in tr:
                <td>${td}</td>
            %endfor
            </tr>
        %endfor
    </tbody>
</table>
