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
            <td><strong>sigma</strong></td>
            <td>${'%#.4e' % (plotter.result.sigma)}</td>
        </tr>
</table>

<%self:plot_group plot_dict="${pbcorplots}"
                                  url_fn="${lambda ms:  'noop'}">

        <%def name="title()">
            Primary Beam Corrected TT0 images
        </%def>

        <%def name="preamble()">


        </%def>


        <%def name="mouseover(plot)">primary beam corrected tt0 plot </%def>



        <%def name="fancybox_caption(plot)">
          primary beam corrected tt0 plot
        </%def>


        <%def name="caption_title(plot)">
           ${plot.basename}
        </%def>
</%self:plot_group>
