<%!
rsc_path = ""
import os
import pipeline.infrastructure.renderer.htmlrenderer as hr
%>
<%inherit file="t2-4m_details-base.mako"/>

<%block name="title">RFI Flagging</%block>

<p>Flag possible RFI using rflag and tfcrop; checkflagmode=${result[0].inputs['checkflagmode']}</p>


% if result[0].inputs['checkflagmode'] in ('bpd','allcals', 'bpd-vlass', 'allcals-vlass'):

<%self:plot_group plot_dict="${summary_plots}"
                                  url_fn="${lambda ms:  'noop'}">

        <%def name="title()">
            Checkflag summary plot
        </%def>

        <%def name="preamble()">


        </%def>


        <%def name="mouseover(plot)">Summary window</%def>



        <%def name="fancybox_caption(plot)">
          Calibrated bandpass after flagging
        </%def>


        <%def name="caption_title(plot)">
           Calibrated bandpass after flagging
        </%def>
</%self:plot_group>

%endif