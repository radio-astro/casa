<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">Primary beam corrected tt0 images</%block>

<script src="${self.attr.rsc_path}resources/js/pipeline.js"></script>

<script>
$(document).ready(function() {
    // return a function that sets the SPW text field to the given spw
    var createSpwSetter = function(spw) {
        return function() {
            // trigger a change event, otherwise the filters are not changed
            $("#select-spw").select2("val", [spw]).trigger("change");
        };
    };

    // create a callback function for each overview plot that will select the
    // appropriate spw once the page has loaded
    $(".thumbnail a").each(function (i, v) {
        var o = $(v);
        var spw = o.data("spw");
        o.data("callback", createSpwSetter(spw));
    });

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
        }
    });
});
</script>

<p>Primary beam corrected images</p>

<%
    # restored stats
    pbcor_min = plotter.result.pbcor_stats.get('min')[0]
    pbcor_max = plotter.result.pbcor_stats.get('max')[0]
    pbcor_sigma = plotter.result.pbcor_stats.get('sigma')[0]
    pbcor_madRMS = plotter.result.pbcor_stats.get('medabsdevmed')[0] * 1.4826  # see CAS-9631 
    pbcor_unit = 'Jy/beam'

    # residual of pb corrected image stats
    residual_min = plotter.result.residual_stats.get('min')[0]
    residual_max = plotter.result.residual_stats.get('max')[0]
    residual_sigma = plotter.result.residual_stats.get('sigma')[0]
    residual_madRMS = plotter.result.residual_stats.get('medabsdevmed')[0] * 1.4826  # see CAS-9631 
    residual_unit = 'Jy/beam'
%>

<table class="table">
    <tr>
        <th></th>
        <th>restored</th>
        <th>residual</th>
    </tr>
    <tr>
        <td><strong>maximum</strong></td>
        <td>${'{:.4e}'.format(pbcor_max)} ${pbcor_unit}</td>
        <td>${'{:.4e}'.format(residual_max)} ${residual_unit}</td>
    </tr>
    <tr>
        <td><strong>minimum</strong></td>
        <td>${'{:.4e}'.format(pbcor_min)} ${pbcor_unit}</td>
        <td>${'{:.4e}'.format(residual_min)} ${residual_unit}</td>
    </tr>
    <tr>
        <td><strong>sigma</strong></td>
        <td>${'{:.4e}'.format(pbcor_sigma)}</td>
        <td>${'{:.4e}'.format(residual_sigma)}</td>
    </tr>
    <tr>
        <td><strong>MAD rms</strong></td>
        <td>${'{:.4e}'.format(pbcor_madRMS)} ${pbcor_unit}</td>
        <td>${'{:.4e}'.format(residual_madRMS)} ${residual_unit}</td>
    </tr>
</table>

<%self:plot_group plot_dict="${pbcorplots}"
                                  url_fn="${lambda ms:  'noop'}">

        <%def name="title()">
            Primary Beam Corrected TT0 images
        </%def>

        <%def name="preamble()">


        </%def>


        <%def name="mouseover(plot)">${plot.basename}</%def>



        <%def name="fancybox_caption(plot)">
          primary beam corrected tt0 plot
        </%def>


        <%def name="caption_title(plot)">
           ${plot.basename}
        </%def>
</%self:plot_group>
