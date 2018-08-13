<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">Make RMS Uncertainty Images</%block>

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
});
</script>

<p>RMS Images are meant to represent the root-mean-square deviation from the mean (rmsd)
   appropriate to measure the noise level in a Gaussian distribution.
</p>

<table class="table">
        <tr>
            <td><strong>maximum</strong></td>
            <td>${'%#.4e' % (plotter.result.max)} Jy/beam</td>
        </tr>
        <tr>
            <td><strong>minimum</strong></td>
            <td>${'%#.4e' % (plotter.result.min)} Jy/beam</td>
        </tr>
        <tr>
            <td><strong>mean</strong></td>
            <td>${'%#.4e' % (plotter.result.mean)} Jy/beam</td>
        </tr>
        <tr>
            <td><strong>median</strong></td>
            <td>${'%#.4e' % (plotter.result.median)} Jy/beam</td>
        </tr>
        <tr>
            <td><strong>sigma</strong></td>
            <td>${'%#.4e' % (plotter.result.sigma)} Jy/beam</td>
        </tr>
        <tr>
            <td><strong>MAD rms</strong></td>
            <td>${'%#.4e' % (plotter.result.MADrms)}  Jy/beam</td>
        </tr>
</table>

<%self:plot_group plot_dict="${rmsplots}"
                                  url_fn="${lambda ms:  'noop'}">

        <%def name="title()">
            RMS images
        </%def>

        <%def name="preamble()">


        </%def>


        <%def name="mouseover(plot)">RMS plot </%def>



        <%def name="fancybox_caption(plot)">
          RMS plot
        </%def>


        <%def name="caption_title(plot)">
           ${plot.basename}
        </%def>
</%self:plot_group>

