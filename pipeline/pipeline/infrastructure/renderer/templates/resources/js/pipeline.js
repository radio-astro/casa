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

    module.loadFakeframe = function (fakeframe, href, insertPre, callback) {
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
                    var callbackFn = $(this).data("callback");
                    UTILS.loadFakeframe(fakeframe, this.href, false, callbackFn);
                });

                $("a.replace-pre").click(function (evt) {
                    evt.preventDefault();
                    var callbackFn = $(this).data("callback");
                    UTILS.loadFakeframe(fakeframe, this.href, true, callbackFn);
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

                if (callback) {
                	// keep compatibility with scalar callback function
                	if (callback instanceof Array) {
                		for (var i = 0; i < callback.length; i++) {
                		    callback[i]();
                		}
                	} else {
                		callback();
                	}
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

    module.getData = function(scores_dict, key, filter) {
        var scores = [];
        for (var png in scores_dict) {
            var anchor = $("a[href='" + png + "']"),
                score;

            // Generate a histogram for those plots present in this page by checking
            // whether the image is present before adding the score.
            if ($(anchor).is(filter)) {
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
        var filters = [], // filters registered with the pipeline
            scores = {},  // scores dictionary
            refresh,      // refresh()
            that = this;  // reference to this for anonymous functions

        // filter PNGs with our list of filter objects, consolidating multiple
        // refresh request into one event via debounce
        var refresh = UTILS.debounce(function () {
            filter();
            // changing the number of visible PNGs could mess up the margins on
            // row n > 1, so rejig the thumbnail layout on each refresh.
            UTILS.fixThumbnailMargins();
        }, 125);

        this.addFilter = function (filter) {
            // Add filter to our list of filters so we can call it when
            // evaluating each PNG.
            filters.push(filter);

            // Subscribe to 'change' events on the filter, refreshing the filter
            // pipeline state and thus the visibility of each PNG when such an
            // event is published.
            $(filter).on("change", function() {
                refresh();
            });
        };

        this.setScores = function (newScores) {
            scores = newScores;
        };

        var filter = function () {
            // calculate the visibility of each thumbnail by..
            for (var png in scores) {
                // .. getting the score for the thumbnail..
                var score = scores[png];

                // .. finding whether it passes each filter..
                var isVisible = filters.reduce(function (previous, current) {
                    return previous && current.isVisible(score);
                }, true);

                // .. and showing/hiding the thumbnail based on the aggregate
                // filter state.
                var li = $("a[href='" + png + "']").parent().parent();
                if (isVisible) {
                    li.show();
                } else {
                    li.hide();
                }
            }

            // publish a 'change' event, notifying subscribers that the filters
            // have changed
            $(that).trigger("change");
        };
    };


    module.HistogramFilter = function(scoreType, extent) {
        var min = -1e9,      // minimum allowed PNG score
            max = 1e9,       // maximum allowed PNG score
            that = this,     // reference for anonymous functions
        	enabled = false; // whether filter is enabled 
        
        // Set the filter range in response to external input, eg. someone
        // dragging the filter range in the histogram GUI.
        this.setRange = function (newMin, newMax) {
            min = newMin;
            max = newMax;

            // only enable the filter if the filter range differs from the
            // full extent of the histogram
            that.enabled = !((min === extent[0]) && (max === extent[1]));
            
            // notify subscribers (ie. the filter pipeline) that the filter
            // thresholds have changed, and so PNGs should be re-filtered
            // accordingly.
            $(that).trigger("change");
        };

        this.isVisible = function (pngScoreDict) {
        	if (!that.enabled) {
        		return true;
        	}
        	
        	var score = pngScoreDict[scoreType];
            if ((score >= min) && (score <= max)) {
                return true;
            } else {
                return false;
            }
        };
    };


    module.MatchFilter = function(scoreType) {
        var visibleVals = [],
            that = this;

        // Set the filter passthrough values in response to external input, eg.
        // a spectral window being selected in the spw text field.
        this.setVisibleVals = function (vals) {
            visibleVals = vals;
            $(that).trigger("change");
        };

        this.isVisible = function (pngScoreDict) {
            var i,                          // loop variable
                visibleVal,
                valToTest = pngScoreDict[scoreType];

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

    module.xAxisLabels = {
        "Tsys": function (xAxisLabel) {
            xAxisLabel.append("tspan")
                .text("T")
                .append("tspan")
                .attr("baseline-shift", "sub")
                .text("sys");
        },
        "WVR phase RMS": function (xAxisLabel) {
            xAxisLabel.text("Phase RMS ratio");
        },
        "unknown": function (xAxisLabel) {
            xAxisLabel.text("N/A");
        },
        "qa": function (xAxisLabel) {
            xAxisLabel.text("QA Score");
        },
        "K": function (xAxisLabel) {
            xAxisLabel.text("K");
        }
    };

    module.Histogram = function(reference, histogramGetter, xAxisLabeller) {
        // Set the 'constants' for this histogram

            // A formatter for counts.
        var formatCount = d3.format(",.0f"),
            // colors for bars
            color = ["#e5e5e5", "#4086aa"],
            // the getters to call to get histograms for all data + selected data
            getterFns = [histogramGetter.getAllDataHistogram,
                         histogramGetter.getSelectedDataHistogram],
            // CSS ids for all data and selected data
            layerIds = ["allData", "filterData"],
            // tick format, which we can cache to save recreating it
            tickFormat = null,
            // duration for axis realignment. Set to zero for no initial animation. 
            duration = 0,
            // whether to plot the full range (0 to max) or data extent range (min to max)
            plotExtent = true,
            // reference to this function for anonymous functions
            that = this;

        var translate = function(x) {
        	return function(d) {
        		return "translate(" + x(d) + ",0)";
        	};
        }
        
        // histogram of all scores. this never changes so we can generate it at construction time
        var allDataHistogram = histogramGetter.getAllDataHistogram();
        // get the extent (ie. the range) of scores. We cannot derive this from
        // the histogram as the x range has been quantised to bar positions
        var extent = histogramGetter.getExtent();
        // maximum Y value, i.e. the maximum number of counts in any histogram
        // bar
        var yMax = d3.max(allDataHistogram, function(bar) { return bar.y });

        // fieldsets are shrunk by 2.1%, so shrink our width accordingly
        // to remain within the column
        var shrinkFactor = (100-2.127659574468085) / 100;

        // Define the margin object with properties for the four sides
        // (clockwise from the top, as in CSS), then add a bit more to the
        // bottom margin to account for the x axis label
        var xAxisOffset = 20;
        var margin = {top: 10, right: 10, bottom: 20 + xAxisOffset, left: 10};
        // We don't want the height to be responsive.
        var height = 150 - margin.top - margin.bottom;
        // but the width is, so let it be set in the resize function
        var width = shrinkFactor * $(reference).empty().width() - margin.left - margin.right;
        // end Y position for the x axis ticks
        var tickHeight = height + 5;       

        // Lastly, define svg as a G element that translates the origin to the
        // top-left corner of the chart area.
        var svg = d3.select(reference).append("svg")
            .attr("width", width + margin.left + margin.right)
            .attr("height", height + margin.top + margin.bottom);

        var gTransform = svg.append("g")
            .attr("transform", "translate(" + margin.left + "," + margin.top + ")");
        
        // Compute the new X and Y scales. Note that the Y scale never changes
        // on window resize.
        var x = d3.scale.linear()
            .domain(plotExtent === true ? d3.extent(extent) : [0, d3.max(extent)])
            .range([0, width]);
        var y = d3.scale.linear()
            .domain([0, yMax])
            .range([height, 0]);

        // Stash the new scale.
        that.__scale__ = x;                

       	var layer = gTransform.selectAll("g")
            .data(getterFns)
            .enter().append("g")
            .style("fill", function(d, i) { return color[i]; })
            .attr("class", function(d, i) { return layerIds[i]; });

        var bar = layer.selectAll(".bar")
            .data(function(d) { return d(); })
            .enter().append("g")
            .attr("class", "bar");

        var rect = bar.append("rect")
            .attr("x", function(d) { return x(d.x); })
            .attr("width", function(d) { return Math.max(x(d.x + d.dx) - x(d.x) - 2, 0.1); })
            .attr("y", function(d) { return y(d.y); })
            .attr("height", function(d) { return height - y(d.y); });

        var text = bar.append("text")
            .attr("dy", ".75em")
            .attr("y", function(d) { return y(d.y) + 6 })
            .attr("x", function(d) { return x(d.x + d.dx / 2); })
            .attr("text-anchor", "middle")
            .text(function(d) { return formatCount(d.y) });

        var clipPath = gTransform.append("defs").append("clipPath")
            .attr("id", "clip")
            .append("rect")
            .attr("width", width)
            .attr("height", height);

        // draw the x axis horizontal line
        var xAxisLine = gTransform.append("g")
	        .attr("class", "axis")
	        .append("line")
	        .attr("y1", height)
	        .attr("y2", height)
	        .attr("x1", 0)
	        .attr("x2", width-1);
        
        var plot = function() {
            // Compute the tick format.
            var format = tickFormat || x.tickFormat(8);
            // fade in/out time
            var fadeTime = duration / 2;
            var oldBrushExtent = false;
            var tickEnter, tickUpdate, tick = null;
            // Retrieve the old x-scale, if this is an update.
            var oldScale = that.__scale__;
            // Compute the new X scale
            var newScale = d3.scale.linear()
	            .domain(plotExtent === true ? d3.extent(extent) : [0, d3.max(extent)])
        	    .range([0, width]);
            // Stash the new scale.
            that.__scale__ = newScale;                

            // resize brush if selected
            if (brush) {
                oldBrushExtent = brush.extent();
        	};
           
            if (brush) {
            	var rebrush = function() {
            		context.select(".brush").call(brush.extent(oldBrushExtent));
            	};
            	brush.x(newScale);
            	gbrush.transition().duration(fadeTime).style("opacity", 1e-6).each("end", rebrush).transition().duration(fadeTime).style("opacity", 1);
            }

            // Update the tick groups.
            tick = gTransform.selectAll("g.tick")
                .data(newScale.ticks(8), function(d) {
                	return this.textContent || format(d);
                });

            // Initialize the ticks with the old scale
            tickEnter = tick.enter().append("g")
                .attr("class", "tick")
                .attr("transform", translate(oldScale))
                .style("opacity", 1e-6);

            tickEnter.append("line")
                .attr("y1", height)
                .attr("y2", tickHeight);

            tickEnter.append("text")
                .attr("text-anchor", "middle")
                .attr("dy", "1em")
                .attr("y", tickHeight)
                .text(format);

            // Transition the entering ticks to the new scale
            tickEnter.transition()
                .duration(duration)
                .attr("transform", translate(newScale))
                .style("opacity", 1);

            // Transition the updating ticks to the new scale
            tickUpdate = tick.transition()
                .duration(duration)
                .attr("transform", translate(newScale))
                .style("opacity", 1);

            tickUpdate.select("line")
                .attr("y1", height)
                .attr("y2", tickHeight);

            tickUpdate.select("text")
                .attr("y", tickHeight);

            // Transition the exiting ticks to the new scale
            tick.exit().transition()
                .duration(duration)
                .attr("transform", translate(newScale))
                .style("opacity", 1e-6)
                .remove();

            // move the histogram bars and counts to the new position
            rect.transition()
            	.duration(duration)
            	.attr("x", function(d) { return newScale(d.x); })
            	.attr("width", function(d) { return Math.max(newScale(d.x + d.dx) - newScale(d.x) - 2, 0.1); });
            text.transition()
            	.duration(duration)
            	.attr("x", function(d) { return newScale(d.x + d.dx / 2); });
        };
        plot();
        
        var context = gTransform.append("g")
            .attr("transform", "translate(0," + margin.top + ")");

        var brush = d3.svg.brush()
            .x(x)
            .on("brush", brushed);

        // the lightweight object to return, which will hold bare minimum
        // functions for resizing the plots, refreshing the selected data
        // histograms and a hook for informing the filter on brush changes.
        var histObject = {};

        // stub function that is called whenever the brush is updated, i.e.
        // whenever a histogram range is selected via the mouse. This provides
        // a hook for the filters to be updated.
        histObject.onBrush = function() {
            // no-op - to be replaced by real filter
        };

        function brushed() {
            var e = brush.empty() ? x.domain() : brush.extent();
            histObject.onBrush(e[0], e[1]);
        }

        var gbrush = context.append("g")
            .attr("class", "x brush")
            .call(brush)
            .selectAll("rect")
            .attr("y", -17)
            .attr("height", height + 7);

        // text label for the x axis
        var xAxisLabel = gTransform.append("text")
            .attr("transform", "translate(" + (width / 2) + " ," + (height + margin.bottom) + ")")
            .style("text-anchor", "middle")
            .attr("dy", "-1em");

        // call our user-provided function to write the X-axis label
        xAxisLabeller(xAxisLabel);

        // Find and set variables for elements that should be resized. These
        // elements never change, so they are precalculated and do not need to
        // be re-found during the resize itself, making it more efficient.
        var selectedRect = svg.selectAll("g.filterData g rect");
        var selectedText = svg.selectAll("g.filterData g text");

        // Function to resize the histogram. The Y dimensions never change, so
        // we just need to alter dimensions in the X axis. Note that we alter
        // existing elements rather than replotting, i.e., re-writing the DOM,
        // as DOM rewrites are very expensive.
        histObject.resize = function() {
            // Define width and height as the inner dimensions of the chart area.
            // The drawing code needs to reference a responsive elements dimensions

            // look to the fluid column rather than the div, as the div does not resize smaller than the histogram
            width = shrinkFactor * $(reference).parent().parent().width() - margin.left - margin.right;
            svg.attr("width", width + margin.left + margin.right);
            clipPath.attr("width", width);

            that.__scale__.range([0, width]);
            rect.attr("x", function(d) { return that.__scale__(d.x); })
                .attr("width", function(d) { return that.__scale__(d.x + d.dx) - that.__scale__(d.x) - 2; });
            text.attr("x", function(d) { return that.__scale__(d.x + d.dx / 2); });

            xAxisLine.attr("x2", width-1);
            histObject.duration(0).plot();
            xAxisLabel.attr("transform", "translate(" + (width / 2) + " ," + (height + margin.bottom) + ")")
            
            context.select(".brush").call(brush.extent(brush.extent()));
        };

        // Function to refresh the select data histogram. We assume resizing and
        // refreshing never happen at exactly the same time, which means this
        // function only has to change the data attached to the selected data
        // DOM elements and then recalculate the Y positions, heights and scores
        // accordingly. Again, this saves DOM rewrites and so is the efficient
        // way to do it.
        histObject.refreshSelectedData = function() {
            var selectedHistogramData = histogramGetter.getSelectedDataHistogram();
            selectedRect.data(selectedHistogramData)
                .transition()
                .duration(500)
                .delay(function(d, i) { return i * 10; })
                .attr("y", function(d) { return y(d.y); })
                .attr("height", function(d) { return height - y(d.y); });

            selectedText.data(selectedHistogramData)
                .transition()
                .duration(500)
                .delay(function(d, i) { return i * 10; })
                .attr("y", function(d) { return y(d.y) + 6; })
                .text(function(d) { return formatCount(d.y) });
        }

        histObject.duration = function(x) {
            if (!arguments.length) return duration;
            duration = x;
            return histObject;
        };

        histObject.plot = plot;
        
        histObject.tickFormat = function(x) {
            if (!arguments.length) return tickFormat;
            tickFormat = x;
            return histObject;
        };        

        histObject.plotExtent = function(p) {
            if (!arguments.length) return plotExtent;
            plotExtent = p;
            plot();
            return histObject;
        };        
        
        return histObject;
    };

    return module;
}();

ALL_IN_ONE = function() {
    var module = {};

    var getData = function (scores_dict, key, filter) {
        var scores = [];

        for (var png in scores_dict) {
            var anchor = $("a[href='" + png + "']"),
                score;

            if ((filter === undefined) || ($(anchor).is(filter))) {
                score = scores_dict[png][key];
                if (score !== "null") {
                	scores.push(score);
            	}
            }
        }
        return scores;
    };

    var createHistogramGetter = function(scores_dict, key, nBins) {
        var allScores = getData(scores_dict, key);        
        var extent = d3.extent(allScores);
        if ((extent[1] - extent[0]) === 0.0) {
        	extent[0] = Math.min(extent[0], 0.9);
        	extent[1] = Math.max(extent[1], 0.1);
        }
        var histogram = d3.layout.histogram().bins(nBins).range(extent);
        var allDataHistogram = histogram(allScores);
        var that = {};

        that.getSelectedDataHistogram = function() {
            var visibleScores = getData(scores_dict, key, ":visible");
            var visibleHistogram = histogram(visibleScores);
            return visibleHistogram;
        };

        that.getAllDataHistogram = function() {
            return allDataHistogram;
        };

        that.getExtent = function() {
            return extent;
        }

        return that;
    };

    module.easyHistogram = function (pipeline, scores, score_key, element_id,
                                     xAxisLabeller) {
        xAxisLabeller = xAxisLabeller || PLOTS.xAxisLabels["unknown"];
        var histogramGetter = createHistogramGetter(scores, score_key, 20);

        // the histogram filter displays unscored plots (those plots with a
        // score of null) when the histogram filter is disengaged. The 
        // histogram filter needs the histogram extent in order to compare it
        // to the filter range to know when the filter should be disengaged.
        var extent = histogramGetter.getExtent();
        var filter = new FILTERS.HistogramFilter(score_key, extent);
        pipeline.addFilter(filter);

        var histogram = PLOTS.Histogram(element_id, histogramGetter,
                                        xAxisLabeller);
        histogram.onBrush = function (lo, hi) {
            filter.setRange(lo, hi);
        };

        // subscribe to 'change' events from the FilterPipeline, refreshing the
        // selected data histogram on such an event.
        $(pipeline).on("change", function() {
           histogram.refreshSelectedData();
        });

        // refill the histogram to the size of the parent element when the
        // window, and thus the parent span width, changes.
        $(window).resize(UTILS.debounce(function () {
            histogram.resize();
        }, 125));

        return {
            "histogram": histogram,
            "filter": filter
        };
    }

    return module;
}();
