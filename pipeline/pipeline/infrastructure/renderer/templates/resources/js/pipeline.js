UTILS = (function () {
    var module = {};

    module.launchFancybox = function(target) {
        var fullsize = [];
        var thumbnailImg;
        var fullsizeToThumbs = {};
        var index = 0;

        $("ul.thumbnails li:visible div a").each(function () {
            var mainImage = this.href; // Find Image href
            var title = this.title; // Find Image title
            if (mainImage == target) {
                index = fullsize.length;
            }
            fullsize.push({
                href: mainImage,
                title: title
            });
            thumbnailImg = $(this).children("img:first")[0].src;
            fullsizeToThumbs[mainImage] = thumbnailImg;
        });

        $.fancybox(fullsize, {
            type: 'image',
            index: index,
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
                    source: function (current) {
                        var href = current.href;
                        return fullsizeToThumbs[href];
                    }
                }
            }
        });
    };

    /**
     * Adds 0 left margin to the first thumbnail on each row that don't get it via CSS rules.
     * Recall the function when the floating of the elements changed.
     */
    module.fixThumbnailMargins = function () {
        $('ul.thumbnails').each(function () {
            var allThumbnails = $(this).children(),
                visibleThumbnails = $(allThumbnails).filter(":visible"),
                previousOffsetLeft;

            if (visibleThumbnails.length === 0) {
                return;
            }
            previousOffsetLeft = $(visibleThumbnails).first().offset().left;

            $(allThumbnails).removeClass('first-in-row');
            $(visibleThumbnails).first().addClass('first-in-row');
            visibleThumbnails.each(function () {
                var thumbnail = $(this),
                    offsetLeft = $(thumbnail).offset().left;
                if (offsetLeft < previousOffsetLeft) {
                    thumbnail.addClass('first-in-row');
                }
                previousOffsetLeft = offsetLeft;
            });
        });
    };

    module.loadFakeframe = function (fakeframe, href, insertPre) {
        insertPre = insertPre || false;

        $(fakeframe).load(href, function (response, status, xhr) {
            if (status == "error") {
                var msg = "Sorry but there was an error: ";
                $(fakeframe).html(msg + xhr.status + " " + xhr.statusText);
            }

            if (status == "success") {
                // add click listener to all the new replace anchors we just
                // loaded into the document
                $("a.replace").click(function (evt) {
                    evt.preventDefault();
                    var href = evt.target.href;
                    UTILS.loadFakeframe(fakeframe, href, false);
                });

                $("a.replace-pre").click(function (evt) {
                    evt.preventDefault();
                    var href = evt.target.href;
                    UTILS.loadFakeframe(fakeframe, href, true);
                });

                if (insertPre) {
                    $(fakeframe).wrapInner("<pre />");
                    $(fakeframe).prepend('<div class="page-header">' +
                        '<h3>' + $.url(href).attr('file') +
                        '<button class="btn btn-large pull-right" ' +
                        'onclick="javascript:location.reload()">Back</button>' +
                        '</h3>' +
                        '</div>');
                }
            }
        });

        UTILS.calculateAffix();
    };

	// The sidebar should scroll if the tasks stretch off-screen.
	module.calculateAffix = function() {
	    if ($(window).height() < ($("#nav-wrapper").height() + 60)) {
		    // expand the div#content height so that the bottom of the well has
		    // a margin
		    $('#content').height(Math.max($("#content").height(), $("#nav-wrapper").height()+40));

		  	$('#nav-wrapper').affix({
				offset: {
				    top: function () {
					    return Math.abs($("#nav-wrapper").height() - $(window).height() + 100);
					}
				}
			});
	    } else {
			$(window).off('.affix');
			$("#nav-wrapper").removeData('affix').removeClass('affix affix-top affix-bottom');
	    };
	};

    module.getData = function(scores_dict, key) {
        var scores = [];
        for (var png in scores_dict) {
            var ul,
            	score;

            var ul = $("a[href='" + png + "']").parent().parent();

            // Generate a histogram for those plots present in this page by checking
            // whether the image is present before adding the score.
            if (ul.length > 0) {
                score = scores_dict[png][key];
                scores.push(score);
            }
        }
        return scores;
    };

    // A better idom for binding with resize is to debounce
    module.debounce = function (fn, timeout) {
        var timeoutID = -1;
        return function () {
            if (timeoutID > -1) {
                window.clearTimeout(timeoutID);
            }
            timeoutID = window.setTimeout(fn, timeout);
        }
    };

    return module;
})();

FILTERS = (function () {
    var module = {};

    module.FilterPipeline = function() {
        var filters = [];
        var scores = [];
        var that = this;

        this.addFilter = function (filter) {
            filters.push(filter);
            filter.addObserver(that);
        };

        this.setScores = function (newScores) {
            scores = newScores;
        };

        this.filter = function () {
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

        this.refresh = function () {
            this.filter();
            UTILS.fixThumbnailMargins();
//
//            var debounced_draw = debounce(function() {
//        	fixThumbnailMargins();
//            }, 125);
        };
    };


    module.HistogramFilter = function(ntype) {
        var type = ntype || "rms";
        var min = -1e9;
        var max = 1e9;
        var observers = [];

        this.setRange = function (newMin, newMax) {
            min = newMin;
            max = newMax;
            observers.forEach(function (observer) {
                observer.refresh();
            });
        };

        this.isVisible = function (hash) {
            var score = hash[type];
            if ((score >= min) && (score <= max)) {
                return true;
            } else {
                return false;
            }
        };

        this.addObserver = function (observer) {
            observers.push(observer);
        };
    };


    module.MatchFilter = function(key) {
        var matchKey = key;
        var visibleVals = [];
        var observers = [];

        this.setVisibleVals = function (vals) {
            visibleVals = vals;
            observers.forEach(function (observer) {
                observer.refresh();
            });
        };

        this.isVisible = function (hash) {
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

        this.addObserver = function (observer) {
            observers.push(observer);
        };
    };

    // key - the key in the dictionary, eg. antenna, spw, pol
    // element - the name of the element to listen to eg. '#select-spw'
    module.createMatchFilter = function(key, element) {
        var filter = new FILTERS.MatchFilter(key);

        $(element).change(function (e) {
            // get the spectral windows that were selected
            var selected = [];
            $(element + ' option:selected').each(function () {
                var t = $(this).text();
                selected.push(t);
            });
            filter.setVisibleVals(selected);
        });

        return filter;
    };

    return module;
})();

PLOTS = function () {
    var module = {};

    module.Histogram = function(reference, data, numBins) {
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
    };

    return module;
}();

ALL_IN_ONE = function() {
    var module = {};

    module.easyHistogram = function (scores, score_key, element_id) {
        var filter = new FILTERS.HistogramFilter(score_key);

        var data = UTILS.getData(scores, score_key);
        var histogram = PLOTS.Histogram(element_id, data, 20);
        histogram.onBrush = function (lo, hi) {
            filter.setRange(lo, hi);
        };
        histogram.plot();

        // refill the histogram to the size of the parent element when the
        // window, and thus the parent span width, changes.
        $(window).resize(UTILS.debounce(function () {
            histogram.plot();
        }, 125));

        return filter;
    }

    return module;
}();