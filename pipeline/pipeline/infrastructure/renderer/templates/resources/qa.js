$(document).ready(function(){
	//Larger thumbnail preview  
	$("ul.thumb li").hover(function() {
	    $(this).css({'z-index' : '10'});
	    $(this).find('img').addClass("hover").stop()
	        .animate({
	            marginTop: '-104px', 
	            marginLeft: '-135px', 
	            top: '50%', 
	            left: '50%', 
	            width: '250px', /* max = 280 cover */ 
	            height: '188px',
	            padding: '5px' 
	        }, 200);
	    
	    } , function() {
	    $(this).css({'z-index' : '0'});
	    $(this).find('img').removeClass("hover").stop()
	        .animate({
	            marginTop: '0', /* Set alignment back to default */
	            marginLeft: '0',
	            top: '0', 
	            left: '0', 
	            width: '100px', /* Set width back to default */
	            height: '75px',  /* Set height back to default */
	            padding: '5px'
	        }, 400);
	});
 
    //Swap Image on Click
    $("ul.thumb li a").click(function() {        
        var mainImage = $(this).attr("href"); //Find Image Name
        $("#main_view img").attr({ src: mainImage });
        $("#main_view a").attr({ href: mainImage });
        return false;        
    });

    function launchFancyboxInParent(target){
    	var basedir = $.url().segment(-2);
    	var thumbs = new Array();
    	var index = 0;
        $("ul.thumb li:visible a").each(function() {        
        	var mainImage = $(this).attr("href"); //Find Image href
            var title = $(this).attr("title"); //Find Image title
            var basename = $.url(mainImage).segment(-1);
            if (basename == target) {
            	index = thumbs.length;
            } 
            thumbs.push({
            	href  : basedir + '/' + mainImage,
            	title : title                     
            });
        });
        parent.postMessage({'command' : 'fancybox',
        					'thumbs'  : thumbs,
        					'index'   : index      }, '*');
        return false;
    }
    
	$("ul.thumb li a[rel=fancybox-thumb]").click(function(evt) {
		evt.preventDefault();
		var href = evt.target.parentElement.href;
		var target = $.url(href).segment(-1);
		launchFancyboxInParent(target);
        return false;
	});    
	
    var score_type = 'total';
    var scores = [];
    for (var png in scorehash) {    	
    	/*
    	 * Generate a histogram for those plots present in this page by checking
    	 * whether the image is present before adding the score.
    	 */
        var ul = $("a[href='qa/" + png + "']").parent();
        if (ul.length > 0) {
            var score = scorehash[png][score_type];
            scores.push(score);
        }
    }
    
    function filter(threshold_lo, threshold_hi) {
        for (var png in scorehash) {
            var score = scorehash[png][score_type];
            var ul = $("a[href='qa/" + png + "']").parent();
            if ((score >= threshold_lo) && (score <= threshold_hi)) {
                ul.show();
            } else {
                ul.hide();
            };
        };
    };

	// A formatter for counts.
	var formatCount = d3.format(",.0f");
	
	var margin = {top: 10, right: 30, bottom: 30, left: 30},
	    width = 660 - margin.left - margin.right,
	    height = 100 - margin.top - margin.bottom;
	
	var x = d3.scale.linear()
	    .domain([0, 1])
	    .range([0, width]);
	
	// Generate a histogram using twenty uniformly-spaced bins.
	var data = d3.layout.histogram()
	    .bins(x.ticks(20))
	    (scores);
	
	var y = d3.scale.linear()
	    .domain([0, d3.max(data, function(d) { return d.y; })])
	    .range([height, 0]);
	
	var xAxis = d3.svg.axis()
	    .scale(x)
	    .orient("bottom");
	
	var svg = d3.select("#scores_histogram").append("svg")
	    .attr("width", width + margin.left + margin.right)
	    .attr("height", height + margin.top + margin.bottom)
	    .append("g")
	    .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

	var bar = svg.selectAll(".bar")
	    .data(data)
	    .enter().append("g")
	    .attr("class", "bar")
	    .attr("transform", function(d) { return "translate(" + x(d.x) + "," + y(d.y) + ")"; });
	
	bar.append("rect")
	    .attr("x", 1)
	    .attr("width", x(data[0].dx) - 1)
	    .attr("height", function(d) { return height - y(d.y); });
	
	bar.append("text")
	    .attr("dy", ".75em")
	    .attr("y", 6)
	    .attr("x", x(data[0].dx) / 2)
	    .attr("text-anchor", "middle")
	    .text(function(d) { return formatCount(d.y); });

	svg.append("defs").append("clipPath")
	    .attr("id", "clip")
	    .append("rect")
	    .attr("width", width)
	    .attr("height", height);
	
	var context = svg.append("g")
	    .attr("transform", "translate(0," + margin.top + ")");
	//    .attr("transform", "translate(" + margin.left + "," + margin.top + ")");
	
	var brush = d3.svg.brush()
	    .x(x)
	    .on("brush", brushed);
	
	function brushed() {
	  var extent = brush.empty() ? x.domain() : brush.extent();
	  filter(extent[0], extent[1]);
	}
	
	context.append("g")
	      .attr("class", "x brush")
	      .call(brush)
	      .selectAll("rect")
	      .attr("y", -17)
	      .attr("height", height + 7);
		
	svg.append("g")
	    .attr("class", "x axis")
	    .attr("transform", "translate(0," + height + ")")
	    .call(xAxis);

});


/*
 * TODO: make magnify-on-hover use something similar to the code below, removing
 * hard-coded sizes
 * 
	$('ul.thumb li').hover(
	    function() {
	        var img = $(this).find('img'),
	            w = img.data('width'),
	            h = img.data('height'),
	            position = img.data('position');
	        img.addClass('hover').stop()
	            .animate({
	                opacity: '1',
	                width: 2*w,
	                height: 2*h,
	                top: position.top - 0.5*h,
	                left: position.left - 0.5*w, 
	            }, 200);
	    }, function() {
	        var img = $(this).find('img'),
	            w = img.data('width'),
	            h = img.data('height'),
	            position = img.data('position');
	        img.removeClass('hover').stop()
	            .animate({
	                opacity: '0',
	                width: w,
	                height: h,
	                top: position.top,
	                left: position.left,
	            }, 400);
	    });
*/
