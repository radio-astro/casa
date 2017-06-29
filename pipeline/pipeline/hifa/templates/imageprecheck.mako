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
if not result[0].have_real_repr_target:
    real_repr_target = ' (assumed)'
else:
    real_repr_target = ''
repr_source = result[0].repr_source
repr_spw = '%d' % (result[0].repr_spw)
repr_freq = '%.4f GHz' % (cqa.getvalue(cqa.convert(result[0].repr_target[1], 'GHz')))
repr_bw = '%.4g MHz' % (cqa.getvalue(cqa.convert(result[0].repr_target[2], 'MHz')))
minAR = '%#.2g arcsec' % (cqa.getvalue(cqa.convert(result[0].minAcceptableAngResolution, 'arcsec')))
maxAR = '%#.2g arcsec' % (cqa.getvalue(cqa.convert(result[0].maxAcceptableAngResolution, 'arcsec')))
robust = '%.2f' % (result[0].hm_robust)
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
PI min/max resolutions: ${minAR} / ${maxAR}
<p>
<h4>Heuristics results:</h4>
robust: ${robust}
<br>
uvtaper: ${uvtaper}
<p>
<h4>Beam and sensitivity results for different robust parameters:</h4>
<table class="table">
    <thead>
        <tr>
            <th>robust</th>
            <th>bmin</th>
            <th>bmin/maxAR</th>
            <th>cell</th>
            <th>bandwidth [kHz]</th>
            <th>bwmode</th>
            <th>sensitivity [Jy/beam]</th>
        </tr>
    </thead>
    <tbody>
        % for tr in table_rows:
        <tr>
        % for td in tr:
            ${td}
        % endfor
        </tr>
        %endfor
    </tbody>
</table>
