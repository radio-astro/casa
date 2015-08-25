<%!
%>

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

<%inherit file="t2-4m_details-base.html"/>
<%block name="header" />

<%block name="title">Find Continuum</%block>

% if not table_rows:
    <p>There are no continuum finding results.
% else:
    <table class="table">
        <thead>
            <tr>
                <th rowspan="2">Field</th>
                <th rowspan="2">Spw</th>
                <th colspan="2">Continuum Frequency Range</th>
                <th rowspan="2">Status</th>
                <th rowspan="2">Average spectrum</th>
            </tr>
            <tr>
                <th>Start</th>
                <th>End</th>
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
