$(document).ready(function() {
    /**
     * Adds 0 left margin to the first thumbnail on each row that don't get it via CSS rules.
     * Recall the function when the floating of the elements changed.
     */
    function fixThumbnailMargins() {
	console.log("Fixing margins");
        $('ul.thumbnails').each(function() {
            var allThumbnails = $(this).children(),
            	visibleThumbnails = $(allThumbnails).filter(":visible"),
                previousOffsetLeft;
            
            if (visibleThumbnails.length === 0) {
        	return;
            }            
            previousOffsetLength = $(visibleThumbnails).first().offset().left;
            
            $(allThumbnails).removeClass('first-in-row');
            $(visibleThumbnails).first().addClass('first-in-row');
            visibleThumbnails.each(function() {
                var thumbnail = $(this),
                    offsetLeft = $(thumbnail).offset().left;
                if (offsetLeft < previousOffsetLeft) {
                    thumbnail.addClass('first-in-row');
                }
                previousOffsetLeft = offsetLeft;
            });
        });
    }
    fixThumbnailMargins();

    function FilterPipeline() {
        var filters = [];
        var scores = [];
        var that = this;

        this.addFilter = function(filter) {
            filters.push(filter);
            filter.addObserver(that);
        };

        this.setScores = function(newScores) {
            scores = newScores;
        };

        this.filter = function() {
            for (var png in scores) {
                var isVisible = true;
                var score = scores[png];
                for (var i in filters) {
                    var f = filters[i];
                    isVisible = (isVisible && f.isVisible(score));
                }

                var li = $("a[href='" + png + "']").parent().parent();
                if (isVisible) {
                    li.show();
                } else {
                    li.hide();
                }
            }
        };

        this.refresh = function() {
            this.filter();
            fixThumbnailMargins();
//
//            var debounced_draw = debounce(function() {
//        	fixThumbnailMargins();
//            }, 125);
        };
    }


    function HistogramFilter(ntype) {
        var type = ntype || "rms";
        var min = -1e9;
        var max = 1e9;
        var observers = [];

        this.setRange = function(newMin, newMax) {
            min = newMin;
            max = newMax;
            observers.forEach(function(observer) {
                observer.refresh();
            });
        };

        this.isVisible = function(hash) {
            var score = hash[type];
            if ((score >= min) && (score <= max)) {
                return true;
            } else {
                return false;
            }
        };

        this.addObserver = function(observer) {
            observers.push(observer);
        };
    }

    
    function MatchFilter(key) {
        var matchKey = key;
        var visibleVals = [];
        var observers = [];

        this.setVisibleVals = function(vals) {
            visibleVals = vals;
            observers.forEach(function(observer) {
                observer.refresh();
            });
        };

        this.isVisible = function(hash) {
            var i,
            	visibleVal,
            	valToTest = hash[matchKey];

            if (visibleVals.length === 0) {
        	return true;
            }
            
            for (i = 0; i < visibleVals.length; i += 1) {
                visibleVal = visibleVals[i];
                if (String(visibleVal) === String(valToTest)) {
                    return true;
                }
            }
            return false;
        };

        this.addObserver = function(observer) {
            observers.push(observer);
        };
    }


    function Histogram(reference, data, numBins) {
        // Define the margin object with properties for the four sides
        // (clockwise from the top, as in CSS), then add a bit more to the 
        // bottom margin to account for the x axis label
        var xAxisOffset = 20;
        var margin = {top: 10, right: 10, bottom: 20 + xAxisOffset, left: 10};

        // default is twenty uniformly-spaced bins
        var numBins = numBins || 20;
        
        // bin data into n bins
        var binned = d3.layout.histogram()
                .bins(numBins)
                (data);

        var plot = function() {
            // Define width and height as the inner dimensions of the chart area.
            // The drawing code needs to reference a responsive elements dimensions
            var width = $(reference).empty().width() - margin.left - margin.right;
            // We don't want the height to be responsive.
            var height = 150 - margin.top - margin.bottom;

            var x = d3.scale.linear()
                    .domain(d3.extent(data))
                    .range([0, width]);

            var y = d3.scale.linear()
                    .domain([0, d3.max(binned, function(d) {
                	return d.y;
                    })])
                    .range([height, 0]);

            var xAxis = d3.svg.axis()
                    .scale(x)
                    .orient("bottom");

            var yAxis = d3.svg.axis()
                    .scale(y)
                    .orient("right");

            // A formatter for counts.
            var formatCount = d3.format(",.0f");

            // Lastly, define svg as a G element that translates the origin to the
            // top-left corner of the chart area.
            var svg = d3.select(reference).append("svg")
                    .attr("width", width + margin.left + margin.right)
                    .attr("height", height + margin.top + margin.bottom)
                    .append("g")
                    .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

            var bar = svg.selectAll(".bar")
                    .data(binned)
                    .enter().append("g")
                    .attr("class", "bar")
                    .attr("transform", function(d) {
                return "translate(0," + y(d.y) + ")";
            });

            bar.append("rect")
                    .attr("x", function(d) { return x(d.x); })
                    .attr("width", function(d) { return x(d.x + d.dx) - x(d.x) - 2; })
                    .attr("height", function(d) { return height - y(d.y); });

            bar.append("text")
                    .attr("dy", ".75em")
                    .attr("y", 6)
                    .attr("x", function(d) {
                return x(d.x + d.dx / 2);
            })
                    .attr("text-anchor", "middle")
                    .text(function(d) {
                return formatCount(d.y);
            });

            svg.append("defs").append("clipPath")
                    .attr("id", "clip")
                    .append("rect")
                    .attr("width", width)
                    .attr("height", height);

            var context = svg.append("g")
                    .attr("transform", "translate(0," + margin.top + ")");

            var brush = d3.svg.brush()
                    .x(x)
                    .on("brush", brushed);

            function brushed() {
                var extent = brush.empty() ? x.domain() : brush.extent();
                that.onBrush(extent[0], extent[1]);
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

            // text label for the x axis
            svg.append("text")
                    .attr("transform", "translate(" + (width / 2) + " ," + (height + margin.bottom) + ")")
                    .style("text-anchor", "middle")
                    .attr("dy", "-1em")
                    .append("tspan")
                    .text("T")
                    .append("tspan")
                    .attr("baseline-shift", "sub")
                    .text("sys");
        };

        var onBrush = function() {
            // no-op stub to be replaced by real filter 
        };

        var that = {};
        that.plot = plot;
        that.onBrush = onBrush;
        return that;
    }

    var filterPipeline = new FilterPipeline();
    filterPipeline.setScores(scorehash);

    var spwFilter = new MatchFilter('spw');
    filterPipeline.addFilter(spwFilter);

    var antFilter = new MatchFilter('antenna');
    filterPipeline.addFilter(antFilter);

    
    $('.select2').select2();
    $('#select-spw').change(function(e) {
        // get the spectral windows that were selected 
        var spws = [];
        $('#select-spw option:selected').each(function() {
            var spw = $(this).text();
            spws.push(spw);
        });
        spwFilter.setVisibleVals(spws);
    });
    $('#select-ant').change(function(e) {
        // get the spectral windows that were selected 
        var selected = [];
        $('#select-ant option:selected').each(function() {
            var t = $(this).text();
            selected.push(t);
        });
        antFilter.setVisibleVals(selected);
    });

    function getData(key) {
        var scores = [];
        for (var png in scorehash) {
            var ul,
            	score;
    		
            var ul = $("a[href='" + png + "']").parent().parent();
            
            // Generate a histogram for those plots present in this page by checking
            // whether the image is present before adding the score.             
            if (ul.length > 0) {
                score = scorehash[png][key];
                scores.push(score);
            }
        }
        return scores;
    }

    var histogramTsysMedianFilter = new HistogramFilter("median");
    filterPipeline.addFilter(histogramTsysMedianFilter);

    var dataTsysMedian = getData("median");
    var histogramTsysMedian = Histogram("#histogram-tsysmedian", dataTsysMedian, 20);
    histogramTsysMedian.onBrush = function(lo, hi) {
        histogramTsysMedianFilter.setRange(lo, hi);
    };
    histogramTsysMedian.plot();

    var histogramTsysMedianMaxFilter = new HistogramFilter("median_max");
    filterPipeline.addFilter(histogramTsysMedianMaxFilter);
    
    var dataTsysMedianMax = getData("median_max");
    var histogramTsysMedianMax = Histogram("#histogram-tsysmedianmax", dataTsysMedianMax, 20);
    histogramTsysMedianMax.onBrush = function(lo, hi) {
        histogramTsysMedianMaxFilter.setRange(lo, hi);
    };
    histogramTsysMedianMax.plot();

    var histogramTsysRmsFilter = new HistogramFilter("rms");
    filterPipeline.addFilter(histogramTsysRmsFilter);

    var dataTsysRms = getData("rms");    
    var histogramTsysRms = Histogram("#histogram-tsysrms", dataTsysRms, 20);
    histogramTsysRms.onBrush = function(lo, hi) {
        histogramTsysRmsFilter.setRange(lo, hi);
    };
    histogramTsysRms.plot();

    // A better idom for binding with resize is to debounce
    var debounce = function(fn, timeout) {
	var timeoutID = -1;
	return function() {
	    if (timeoutID > -1) {
		window.clearTimeout(timeoutID);
	    }
	    timeoutID = window.setTimeout(fn, timeout);
	}
    };

    var debounced_draw = debounce(function() {
	histogramTsysMedian.plot();
	histogramTsysMedianMax.plot();
	histogramTsysRms.plot();
    }, 125);

    $(window).resize(debounced_draw);

//    $(".fancybox").fancybox();

    $("ul.thumbnails li div a").click(function(evt) {
	evt.preventDefault();
	var target = this.href;
	launchFancyboxInParent(target);
	return false;
    });

    function launchFancyboxInParent(target){
	var fullsize = [];
	var thumbnail;
	var thumbnailImg;
	var fullsizeToThumbs = {};
	var index = 0;
	

	$("ul.thumbnails li:visible div a").each(function() {        
	    var mainImage = this.href; // Find Image href
	    var title = this.title; // Find Image title
	    if (mainImage == target) {
        	index = fullsize.length;
	    }
	    fullsize.push({
        	href  : mainImage,
        	title : title                     
	    });
	    thumbnailImg = $(this).children("img:first")[0].src;
	    fullsizeToThumbs[mainImage] = thumbnailImg;
	});

	$.fancybox(fullsize, {
	    type    : 'image',
	    index   : index,
	    prevEffect : 'none',
	    nextEffect : 'none',
	    helpers	: {
		title	: {
		    type: 'outside'
		},
		thumbs	: {
		    width	: 50,
		    height	: 50,
		    source  : function(current) {
			var href = current.href;
			return fullsizeToThumbs[href];
	            }
		}
	    }
	});
    }

});
