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
        var filters = [];
        var scores = [];
        var that = this;
        var refreshFns = [];

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

        this.doRefresh = function() {
            for (var i=0; i<refreshFns.length; i++) {
                refreshFns[i]();
            }
        };

        this.refresh = UTILS.debounce(function () {
            that.filter();
            that.doRefresh();
            UTILS.fixThumbnailMargins();
        }, 125);

        this.addRefreshFn = function(fn) {
            refreshFns.push(fn);
        }
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

    module.Histogram = function(reference, histogramGetter) {
        // Set the 'constants' for this histogram
        // A formatter for counts.
        var formatCount = d3.format(",.0f");

        var color = ["#e5e5e5", "#4086aa"];
        var getterFns = [histogramGetter.getAllDataHistogram,
                         histogramGetter.getSelectedDataHistogram];
        var layerIds = ["allDataLayer", "selectedDataLayer"];

        var allDataHistogram = histogramGetter.getAllDataHistogram();
        var extent = histogramGetter.getExtent();
        var yMax = d3.max(allDataHistogram, function(bar) { return bar.y });

        // fieldsets are shrunk by 2.1%, so shrink our width accordingly
        // to remain within the column
        var f = (100-2.127659574468085) / 100;

        // Define the margin object with properties for the four sides
        // (clockwise from the top, as in CSS), then add a bit more to the
        // bottom margin to account for the x axis label
        var xAxisOffset = 20;
        var margin = {top: 10, right: 10, bottom: 20 + xAxisOffset, left: 10};
        // We don't want the height to be responsive.
        var height = 150 - margin.top - margin.bottom;
        // but the width is, so let it be set in the resize function
        var width = f * $(reference).empty().width() - margin.left - margin.right;

        // set X and Y scales. Y scale never changes.
        var x = d3.scale.linear()
            .domain(d3.extent(extent))
            .range([0, width]);
        var xAxis = d3.svg.axis()
            .scale(x)
            .orient("bottom");
        var y = d3.scale.linear()
            .domain([0, yMax])
            .range([height, 0]);

        // Lastly, define svg as a G element that translates the origin to the
        // top-left corner of the chart area.
        var svg = d3.select(reference).append("svg")
            .attr("width", width + margin.left + margin.right)
            .attr("height", height + margin.top + margin.bottom);

        var gTransform = svg.append("g")
            .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

        var that = {};

        that.resize = function() {
            // Define width and height as the inner dimensions of the chart area.
            // The drawing code needs to reference a responsive elements dimensions

            // look to the fluid column rather than the div, as the div does not resize smaller than the histogram
            width = f * $(reference).parent().parent().width() - margin.left - margin.right;
            svg.attr("width", width + margin.left + margin.right);
            clipPath.attr("width", width);

            x = x.range([0, width]);
            rect.attr("x", function(d) { return x(d.x); })
                .attr("width", function(d) { return x(d.x + d.dx) - x(d.x) - 2; });
            text.attr("x", function(d) { return x(d.x + d.dx / 2); });

            xAxis.scale(x);
            gTransform.select("#xaxis").call(xAxis);
            xAxisLabel.attr("transform", "translate(" + (width / 2) + " ," + (height + margin.bottom) + ")")
        };

        var layer = gTransform.selectAll("g")
            .data(getterFns)
            .enter().append("g")
            .style("fill", function(d, i) { return color[i]; })
            .attr("id", function(d, i) { return layerIds[i]; });

        var bar = layer.selectAll(".bar")
            .data(function(d) { return d(); })
            .enter().append("g")
            .attr("class", "bar");

        var rect = bar.append("rect")
            .attr("x", function(d) { return x(d.x); })
            .attr("width", function(d) { return x(d.x + d.dx) - x(d.x) - 2; })
            .attr("y", height)
            .attr("height", 0);

        var text = bar.append("text")
            .attr("dy", ".75em")
            .attr("y", function(d) { return y(d.y) + 6 })
            .attr("x", function(d) { return x(d.x + d.dx / 2); })
            .attr("text-anchor", "middle")
            .text(function(d) { return formatCount(d.y) });

        rect.transition()
            .delay(function(d, i) { return i * 10; })
            .attr("y", function(d) { return y(d.y); })
            .attr("height", function(d) { return height - y(d.y); });

        var selectedRect = svg.selectAll("g#selectedDataLayer g rect");
        var selectedText = svg.selectAll("g#selectedDataLayer g text");

        that.refreshSelectedData = function() {
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

        var clipPath = gTransform.append("defs").append("clipPath")
            .attr("id", "clip")
            .append("rect")
            .attr("width", width)
            .attr("height", height);

        var context = gTransform.append("g")
            .attr("transform", "translate(0," + margin.top + ")");

        var brush = d3.svg.brush()
            .x(x)
            .on("brush", brushed);

        function brushed() {
            var e = brush.empty() ? x.domain() : brush.extent();
            that.onBrush(e[0], e[1]);
        }

        context.append("g")
            .attr("class", "x brush")
            .call(brush)
            .selectAll("rect")
            .attr("y", -17)
            .attr("height", height + 7);

        gTransform.append("g")
            .attr("class", "x axis")
            .attr("transform", "translate(0," + height + ")")
            .attr("id", "xaxis")
            .call(xAxis);

        // text label for the x axis
        var xAxisLabel = gTransform.append("text")
            .attr("transform", "translate(" + (width / 2) + " ," + (height + margin.bottom) + ")")
            .style("text-anchor", "middle")
            .attr("dy", "-1em")
            .append("tspan")
            .text("T")
            .append("tspan")
            .attr("baseline-shift", "sub")
            .text("sys");

        that.onBrush = function() {
            // no-op stub to be replaced by real filter
        };
        return that;
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

            if (filter === undefined) {
                score = scores_dict[png][key];
                scores.push(score);
            } else if ($(anchor).is(filter)) {
                score = scores_dict[png][key];
                scores.push(score);
            }
        }
        return scores;
    }

    module.histogramGetter = function(scores_dict, key, nBins) {
        var allScores = getData(scores_dict, key);
        var extent = d3.extent(allScores);
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
    }

    module.easyHistogram = function (pipeline, scores, score_key, element_id) {
        var histogramGetter = ALL_IN_ONE.histogramGetter(scores, score_key, 20);

        var filter = new FILTERS.HistogramFilter(score_key);
        pipeline.addFilter(filter);

        var histogram = PLOTS.Histogram(element_id, histogramGetter);
        histogram.onBrush = function (lo, hi) {
            filter.setRange(lo, hi);
        };

        pipeline.addRefreshFn(function() {
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