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

<%inherit file="t2-4m_details-base.mako"/>
<%block name="header" />

<%block name="title">Check Product Size</%block>

% if not table_rows:
    <p>There are no product size check results.
% else:
    Size mitigation parameters for subsequent hif_makeimlist calls
    <table class="table">
        <thead>
            <tr>
                <th>nbins</th>
                <th>hm_imsize</th>
                <th>hm_cell</th>
                <th>field</th>
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
%endif
