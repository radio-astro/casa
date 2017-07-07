<%!
rsc_path = ""
import os
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">Image Pre-Check</%block>

<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>

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
        },
    });
});
</script>

<%
cqa = casatools.quanta
if not result[0].real_repr_target:
    real_repr_target = ' (assumed)'
else:
    real_repr_target = ''
repr_source = result[0].repr_source
repr_spw = '%s' % (result[0].repr_spw)
repr_freq = '%.4f GHz' % (cqa.getvalue(cqa.convert(result[0].repr_target[1], 'GHz')))
repr_bw = '%.4g MHz' % (cqa.getvalue(cqa.convert(result[0].repr_target[2], 'MHz')))
minAR_v = cqa.getvalue(cqa.convert(result[0].minAcceptableAngResolution, 'arcsec'))
maxAR_v = cqa.getvalue(cqa.convert(result[0].maxAcceptableAngResolution, 'arcsec'))
minAR = '%#.2g arcsec' % (minAR_v)
maxAR = '%#.2g arcsec' % (maxAR_v)
robust = '%.1f' % (result[0].hm_robust)
uvtaper = '%s' % (result[0].hm_uvtaper)
%>
<p>
<h4>Representative target${real_repr_target}:</h4>
${repr_source}, SPW ${repr_spw}
<br>
PI Frequency: ${repr_freq}
<br>
PI Bandwidth: ${repr_bw}
<br>
PI min/max resolutions:
%if minAR_v==0.0 and maxAR_v==0.0:
    Not available
%else:
    ${minAR} / ${maxAR}
%endif
<p>
<h4>Heuristics results:</h4>
robust: ${robust}
<br>
uvtaper: ${uvtaper}
<p>
<h4>Beam and sensitivity results for different robust and uvtaper parameters:</h4>
<table class="table">
    <thead>
        <tr>
            <th>robust</th>
            <th>uvtaper</th>
            <th>beam</th>
            <th>bmin/maxAR</th>
            <th>cell</th>
            <th>bandwidth</th>
            <th>bwmode</th>
            <th>sensitivity</th>
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
