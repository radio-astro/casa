var pipeline = pipeline || {};

pipeline.pages = pipeline.pages || function() {
    var module = {};

    module.t1_1 = function() {
        var innerModule = {};

        innerModule.ready = function() {
            var target = $("div#qa_notes");
            var href = $(target).data("href");

            $(target).load(href, function(response, status, xhr) {
                if (status === "error") {
                    // no-op
                }

                if (status === "success") {
                    if (response.length > 0) {

                        // display raw text as preformatted text, but display
                        // HTML content as-is

                        // tests whether the loaded file contains HTML tags
                        var newHtml = $('<div></div>').html(response).text();
                        if (newHtml === response) {
                            $(target).wrapInner("<pre />");
                        }

                        target.prepend('<h2>Quality Assurance notes</h2>');
                    }
                }
            });
        };

        return innerModule;
    }();

    module.t1_4m = function() {
        var innerModule = {};

        innerModule.ready = function() {
            History.Adapter.bind(window, 'statechange', function(event) { // Note: We are using statechange instead of popstate
                var state = History.getState(); // Note: We are using History.getState() instead of event.state
                History.debug("statechange: " + JSON.stringify(state.data));
                pipeline.history.setComponentState(state.data["componentState"]);
            });

            pipeline.appContainer.redirectPreAnchorTarget();

            // load the stage given in the URL
            var logfile = $.url().param("logfile");

            if (logfile) {
                var onSuccess = [function() {
                    pipeline.appContainer.addPreMarkup(logfile);
                    pipeline.appContainer.setTitle(logfile);
                    pipeline.history.pushState(logfile);
                }];
                pipeline.appContainer.load(logfile, onSuccess);
            }
        };

        return innerModule;
    }();

    module.t2_1 = function() {
        var innerModule = {};

        var f = function() {
            // Load MS details into panel is 'ms=x' given in URL
            var anchorId = $.url().param('sidebar').replace(/[^a-zA-Z0-9]/gi, '_');
            var href = $("a#" + anchorId).prop("href");

            var onSuccess = [function() {
                pipeline.sidebar.setActive(anchorId);

                // replace rather than push, as the current state was
                // incomplete. We've now made it whole by loading the stage.
                pipeline.history.replaceState();
            }];

            if (typeof href != 'undefined') {
                pipeline.detailsframe.load(href, onSuccess);
            }
        };

        innerModule.ready = function() {
            History.Adapter.bind(window, 'statechange', function(event) { // Note: We are using statechange instead of popstate
                var state = History.getState(); // Note: We are using History.getState() instead of event.state
                History.debug("statechange: " + JSON.stringify(state.data));
                pipeline.history.setComponentState(state.data["componentState"]);
            });

            $('ul.nav-sidebar li a').click(function(evt) {
                evt.preventDefault();

                pipeline.history.replaceState();

                // load the new page into the fake frame
                var anchorId = $(this).prop("id");
                var href = $(this).prop("href");

                var onSuccess = [function() {
                    pipeline.sidebar.setActive(anchorId);

                    // store the new state
                    pipeline.history.pushState();
                }];

                pipeline.detailsframe.load(href, onSuccess);
            });

            f();
        };

        innerModule.initState = function() {
            var anchor;

            // if the selected MS is in the history, load it, otherwise load the
            // first
            if (!selected) {
                // load content from the first link and set the active MS text.
                anchor = $(module.getSelector()).find('li.menu-item a.replace:first');
            } else {
                $('li.menu-item a.replace').each(function(k,v) {
                    if ($(this).text() === selected) {
                        anchor = $(this);
                    }
                });
            }

            // fallback to first item in case the selector wasn't found
            if (!anchor) {
                anchor = $(module.getSelector()).find('li.menu-item a.replace:first');
            }

            var onSuccess = [function() {
                module.setSelected($(anchor).text());
                pipeline.history.replaceState();
            }];

            pipeline.detailsframe.load($(anchor).prop("href"), onSuccess);
        };

        return innerModule;
    }(); // end t2-1

    module.t2_4m = function() {
        var innerModule = {};

        var f = function() {
            // load the stage given in the URL
            var sidebarParam = $.url().param('sidebar');
            var sidebarAnchor = $("a#" + sidebarParam);
            // .. or fall back to the first URL if nothing was given
            if ((typeof sidebarParam === 'undefined') || (typeof sidebarAnchor === 'undefined')) {
                sidebarAnchor = $('a#sidebar_stage1')
            }

            // load the new page into the fake frame. We do this using the
            // javascript components rather than click() so that we don't push
            // the initial empty history state onto the stack.
            var href = $(sidebarAnchor).prop("href");
            var anchorId = $(sidebarAnchor).prop("id");

            // extract the session target page to load, if supplied
            var msTargetParam = $.url().param('ms');

            // extract the subpage URL, if supplied
            var subpageParam = $.url().param('subpage');

            var onSuccess = [function() {
                pipeline.sidebar.setActive(anchorId);

                pipeline.msselector.setVisible(true);
                // Don't need to check the value of msTargetParam as
                // setSelected will load the first detail page if the anchor
                // cannot be found.
                pipeline.msselector.setSelected(msTargetParam);

                if (typeof subpageParam !== 'undefined') {
                    var dirname = "stage" + pipeline.sidebar.getTaskNumber();
                    var subpagePath = dirname + '/' + subpageParam;
                    pipeline.detailsframe.load(subpagePath);
                }

                // replace rather than push, as the current state was
                // incomplete. We've now made it whole by loading the stage.
                pipeline.history.replaceState();
            }];
            pipeline.fakeframe.load(href, onSuccess);
        };

        var setFancyboxDefaults = function() {
            $.fancybox.defaults.caption = function(instance, item) {
                var caption;

                // first try to get the override caption from the anchor itself
                caption = $(this).data('caption');
                if (caption !== undefined) {
                    return caption;
                }

                // otherwise, duplicate the caption element, converting any anchors
                caption = $(this).siblings('div.caption');
                if (caption === undefined) {
                    // no override or caption element.
                    return '';
                }

                var orig_nodes = $('<div/>').html(caption.clone()).contents();
                orig_nodes.find('a').replaceWith(function(){
                    return $("<span>" + $(this).html() + "</span>");
                });
                return orig_nodes.html();
            };

            $.fancybox.defaults.thumbs.autoStart = true;
            $.fancybox.defaults.thumbs.axis = 'x';

            // Space around image, ignored if zoomed-in or viewport width is smaller than 800px
            $.fancybox.defaults.margin = [44, 0, 22, 0];

            // Should display navigation arrows at the screen edges
            $.fancybox.defaults.arrows = true;

            // Should display infobar (counter and arrows at the top)
            $.fancybox.defaults.infobar = true;

            // Should display toolbar (buttons at the top)
            $.fancybox.defaults.toolbar = true;

            // What buttons should appear in the top right corner.
            // Buttons will be created using templates from `btnTpl` option
            // and they will be placed into toolbar (class="fancybox-toolbar"` element)
            $.fancybox.defaults.buttons = [
                'slideShow',
                'plotCommand',
                'fullScreen',
                // 'thumbs',
                // 'share',
                'download',
                'zoom',
                'close'
            ];

            // Open/close animation type
            // Possible values:
            //   false            - disable
            //   "zoom"           - zoom images from/to thumbnail
            //   "fade"
            //   "zoom-in-out"
            //
            // $.fancybox.defaults.animationEffect = 'zoom';

            // Duration in ms for open/close animation
            $.fancybox.defaults.animationDuration = 250;

            // Transition effect between slides
            //
            // Possible values:
            //   false            - disable
            //   "fade'
            //   "slide'
            //   "circular'
            //   "tube'
            //   "zoom-in-out'
            //   "rotate'
            //
            $.fancybox.defaults.transitionEffect = false;

            // Detect "idle" time in seconds
            $.fancybox.defaults.idleTime = 3;
        };

        innerModule.ready = function() {
            History.Adapter.bind(window, 'statechange', function(event) { // Note: We are using statechange instead of popstate
                var state = History.getState(); // Note: We are using History.getState() instead of event.state
                History.debug("statechange: " + JSON.stringify(state.data));
                pipeline.history.setComponentState(state.data["componentState"]);
            });

            $("ul.nav-sidebar li a").click(function(evt) {
                evt.preventDefault();

                pipeline.history.replaceState();

                // load the new page into the fake frame
                var anchorId = $(this).prop("id");
                var href = $(this).prop("href");

                var msselectorSelected = pipeline.msselector.getSelected();

                var onSuccess = [function() {
                    pipeline.sidebar.setActive(anchorId);

                    pipeline.msselector.setVisible(true);
                    pipeline.msselector.setSelected(msselectorSelected);

                    // store the new state
                    pipeline.history.pushState();
                }];

                pipeline.fakeframe.load(href, onSuccess);
            });

            f();
            setFancyboxDefaults();
        };

        return innerModule;
    }();

    module.t2_4m_details_container = function() {
        var innerModule = {};

        innerModule.ready = function() {
            // keep the dropdown menu open when selecting links
            $(function() {
                $("ul.dropdown-menu").on("click", "[data-stopPropagation]", function(e) {
                    e.stopPropagation();
                });
            });

            $("li.menu-item a.replace").click(function(evt) {
                evt.preventDefault();

                // take a snapshot of the current state so we can restore the page on
                // return.
                pipeline.history.replaceState();

                // load the new page into the fake frame
                var selected = $(this).text();
                pipeline.msselector.setSelected(selected);
                var href = $(this).prop("href");

                // when switching between measurement sets, maintain the same position
                // on the page.
                var currentPosition = pipeline.detailsframe.getScrollTop();
                var onSuccess = [function() {
                    pipeline.detailsframe.setScrollTop(currentPosition);
                }];
                pipeline.detailsframe.load(href, onSuccess);

                // store the new state
                pipeline.history.pushState();
            });
        };

        return innerModule;
    }();

    module.detail_plots = function() {
        var innerModule = {};

        innerModule.ready = function() {
            $('div.thumbnail:visible > a').fancybox({
                selector : 'div.thumbnail:visible > a'
            });
        };

        return innerModule;
    }();

    module.tclean_plots = function() {
        var innerModule = {};

        innerModule.ready = function() {
            // add on-click handler to our thumbnails to launch FancyBox with the
            // relevant thumbnails
            $("button.replace").click(function(evt) {
                evt.preventDefault();

                // take a snapshot of the current state so we can restore the page on
                // return.
                pipeline.history.replaceState();

                // load the new page into the fake frame
                var href = $(this).data("href");

                // when switching between measurement sets, maintain the same position
                // on the page.
                var currentPosition = pipeline.detailsframe.getScrollTop();
                var onSuccess = [function() {
                    pipeline.detailsframe.setScrollTop(currentPosition);
                }];
                pipeline.detailsframe.load(href, onSuccess);

                // store the new state
                pipeline.history.pushState();
            });
        };

        return innerModule;
    }();

    return module;
}();

pipeline.sidebar = pipeline.sidebar || (function() {
    var module = {};
    var activeAnchorId;

    module.getId = function() {
        return "task sidebar";
    };

    module.getState = function() {
        if (!activeAnchorId) {
            return {};
        } else {
            return {
                activeAnchorId: activeAnchorId
            };
        }
    };

    module.setState = function(state) {
        var anchorId = state["activeAnchorId"];
        if (anchorId === activeAnchorId)
            return;

        module.setActive(anchorId);
    };

    module.getTaskNumber = function() {
        return $("a#" + activeAnchorId).data("stagenum");
    };

    module.getTaskName = function() {
        return $("a#" + activeAnchorId).data("taskname");
    };

    module.setActive = function(anchorId) {
        activeAnchorId = anchorId;
        // remove highlight bar from any current active task..
        $("ul.nav-sidebar li.active").removeClass("active");
        // .. and add it to the task that was clicked.
        $("a#" + activeAnchorId).parent().addClass("active");
    };

    module.getQueryUrl = function() {
        if (activeAnchorId) {
            return "?sidebar=" + activeAnchorId;
        } else {
            return "";
        }
    };

    return module;
})();


pipeline.appContainer = pipeline.appContainer || (function() {
    var module = {};
    var loadedHref;
    var title;
    var isPreFormatted;
    var scrollTop;

    module.getSelector = function() {
        return "div#app-container";
    };

    module.getId = function() {
        return "App container";
    };

    module.getState = function() {
        if (!document.URL.match(/t1-4.html$/)) {
            return {}
        } else {
            return {
                page: "t1-4",
                href: loadedHref,
                preformatted: isPreFormatted,
                title: title || document.title,
                scrollTop: module.getScrollTop()
            };
        }
    };

    module.setState = function(state) {
        if (state["page"] === "t1-4") {
            var onSuccess = [];
            if (state["preformatted"] === true) {
                onSuccess.push(function() {
                    module.addPreMarkup(state["title"]);
                });
            }
            if (state["title"] !== "") {
                onSuccess.push(function() {
                    module.setTitle(state["title"]);
                })
            }
            if (state["scrollTop"]) {
                onSuccess.push(function() {
                    module.setScrollTop(state["scrollTop"]);
                });
            }
            var href = state["href"];
            module.load(href, onSuccess);
        }
    };

    module.load = function(href, onSuccess) {
        loadedHref = href;
        isPreFormatted = false;
        title = "Untitled";

        if (onSuccess) {
            if (!(onSuccess instanceof Array)) {
                onSuccess = [onSuccess]
            }
        } else {
            onSuccess = [];
        }

        var container = module.getSelector();
        onSuccess.push(function() {
            module.redirectPreAnchorTarget(container);
        });

        // if no log is being loaded, just load the main content from the
        // HTML, bypassing the javascript parts of the page which would
        // otherwise load repeatedly
        if (href === undefined) {
            href = document.URL + " #mainbody";
        }
        UTILS.loadContent(container, href, onSuccess);
    };

    module.addPreMarkup = function(pageTitle) {
        var target = $(module.getSelector());
        target.wrapInner("<code />");
        target.wrapInner("<pre />");
        target.prepend('<div class="page-header">' +
            '<h1>' + pageTitle +
            '<button class="btn btn-default pull-right" ' +
            'onclick="javascript:window.history.back()">Back</button>' +
            '</h1>' +
            '</div>');
        isPreFormatted = true;
        title = pageTitle;
    };

    module.setTitle = function(newTitle) {
        document.title = newTitle;
        title = newTitle;
    };

    module.redirectPreAnchorTarget = function() {
        var target = module.getSelector();

        $(target).find("a.replace-pre").click(function(evt) {
            evt.preventDefault();

            // take a snapshot of the current state so we can restore the page on
            // return.
            pipeline.history.replaceState();

            var title = $(this).data("title");

            var onSuccess = [function() {
                pipeline.appContainer.addPreMarkup(title);
                pipeline.appContainer.setTitle(title);
                pipeline.history.pushState(title);
            }];

            pipeline.appContainer.load(this.href, onSuccess);
        });
    };

    module.getQueryUrl = function() {
        // Only append to the URL when displaying a log
        if (isPreFormatted) {
            var path = loadedHref.substring(loadedHref.lastIndexOf('/') + 1, loadedHref.length);
            return "?logfile=" + path;
        } else {
            return "";
        }
    };

    module.getScrollTop = function() {
        return $(window).scrollTop();
    };

    module.setScrollTop = function(scrollTop) {
        $(window).scrollTop(scrollTop);
    };

    return module;
})();


pipeline.fakeframe = pipeline.fakeframe || (function() {
    var module = {};
    var loadedHref;

    module.getSelector = function() {
        return "div#fakeframe";
    };

    module.getId = function() {
        return "Main panel";
    };

    module.getState = function() {
        if (!loadedHref) {
            return {};
        } else {
            return { href : loadedHref };
        }
    };

    module.setState = function(state, callbacks) {
        var href = state["href"];
        // if the fakeframe is not used in this page, proceed straight to the
        // callback execution.
        if (href) {
            module.load(href, callbacks);
        } else {
            for (var i=0; i<callbacks.length; i++) {
                callbacks[i]();
            }
        }
    };

    module.load = function(href, callbacks) {
        var target = module.getSelector();
        UTILS.loadContent($(target), href, callbacks);
        loadedHref = href;
    };

    return module;
})();


pipeline.msselector = pipeline.msselector || (function() {
    var module = {};
    var selected = "";

    module.isVisible = function() {
        return $(module.getSelector()).hasClass("visible");
    };

    /* Return true if the selector offers more than one choice */
    holdsMultipleAnchors = function() {
        var numAnchors = $(module.getSelector() + " a.replace").length;
        return (numAnchors > 1);
    };

    module.setVisible = function(visible) {
        if (visible && holdsMultipleAnchors()) {
            $(module.getSelector()).show().addClass("visible");
        } else {
            $(module.getSelector()).hide().removeClass("visible");
        }
    };

    module.getSelector = function() {
        return "nav#ms_selector";
    };

    module.getId = function() {
        return "MS selector";
    };

    module.getState = function() {
        return {
            isVisible : module.isVisible(),
            selected : selected
        };
    };

    module.setState = function(state) {
        module.setVisible(state["isVisible"]);
        // restoring the state of the details frame will load the necessary
        // page, so all the MS selector has to do is update the active
        // selection text
        setSelectedLabel(state["selected"]);
    };

    setSelectedLabel = function(text) {
        // set the 'currently viewing MS123' text
        selected = text;
        var label = "Currently viewing " + selected;
        $("#container-active").text(label);
    };

    getTargetAnchor = function(targetText) {
        var targetAnchor;

        $("li.menu-item a.replace").each(function(k,v) {
            if ($(this).text() === targetText) {
                targetAnchor = $(this);
            }
        });

        // fallback to first item in case the selector wasn't found
        if (!targetAnchor) {
            targetAnchor = $(module.getSelector()).find("li.menu-item a.replace:first");
        }

        return targetAnchor;
    };

    module.getSelected = function() {
        return selected;
    };

    module.setSelected = function(targetText) {
        var targetAnchor = getTargetAnchor(targetText);

        var onSuccess = [function() {
            setSelectedLabel($(targetAnchor).text());
            pipeline.history.replaceState();
        }];

        pipeline.detailsframe.load($(targetAnchor).prop("href"), onSuccess);
    };

    module.getQueryUrl = function() {
        if (selected) {
            return "&ms=" + selected;
        } else {
            return "";
        }
    };

    return module;
})();


pipeline.detailsframe = pipeline.detailsframe || (function() {
    var module = {};
    var loadedHref;
    var title;
    var isPreFormatted;

    module.getSelector = function() {
        return "div#session_container";
    };

    module.getId = function() {
        return "Details frame";
    };

    module.getState = function() {
        return {
            href : loadedHref,
            preformatted : isPreFormatted,
            title : title,
            scrollTop : module.getScrollTop(),
            filters: module.getFilters()
        };
    };

    // get the active filter selections as an associative array
    module.getFilters = function() {
        var d = {};
        var getId = function(o) { return o.id; };

        $("select.select2").each(function() {
            var selected = $(this).select2("data").map(getId);
            d[this.id] = selected;
        });

        return d;
    };

    module.setFilters = function(state) {
        for (var key in state) {
            if (state.hasOwnProperty(key)) {
                $("#" + key).each(function() { $(this).select2("val", state[key]).trigger("change") });
            }
        }
    };

    module.addPreMarkup = function(pageTitle) {
        var target = $(module.getSelector());
        target.wrapInner("<code />");
        target.wrapInner("<pre />");
        target.prepend('<div class="page-header">' +
            '<h1>' + pageTitle +
            '<button class="btn btn-default pull-right" ' +
            'onclick="javascript:window.history.back()">Back</button>' +
            '</h1>' +
            '</div>');
        isPreFormatted = true;
        title = pageTitle;
    };

    module.setState = function(state) {
        var onSuccess = [];
        if (state["preformatted"] === true) {
            onSuccess.push(function() {
                module.addPreMarkup(state["title"]);
            });
        }
        if (state["scrollTop"]) {
            onSuccess.push(function() {
                module.setScrollTop(state["scrollTop"]);
            });
        }
        if (state["filters"]) {
            onSuccess.push(function() {
                module.setFilters(state["filters"]);
            })
        }
        var href = state["href"];
        module.load(href, onSuccess);
    };

    module.load = function(href, onSuccess) {
        if (onSuccess) {
            if (!(onSuccess instanceof Array)) {
                onSuccess = [onSuccess]
            }
        } else {
            onSuccess = [];
        }

        var container = module.getSelector();
        onSuccess.push(function() {
            UTILS.redirectAnchorTarget(container);
            UTILS.redirectPreAnchorTarget(container);
        });
        UTILS.loadContent(container, href, onSuccess);

        loadedHref = href;
        isPreFormatted = false;
        title = "Untitled";
    };

    module.getScrollTop = function() {
        return $(window).scrollTop();
    };

    module.setScrollTop = function(scrollTop) {
        $(window).scrollTop(scrollTop);
    };

    module.getQueryUrl = function() {
        if (loadedHref) {
            var path = loadedHref.substring(loadedHref.lastIndexOf('/') + 1, loadedHref.length);
            return "&subpage=" + path;
        } else {
            return "";
        }
    };

    return module;
})();


pipeline.history = pipeline.history || (function() {
//	window.addEventListener("beforeunload", function(event) {
//	console.log("Replacing state in unload");
//	module.replaceState();
//});

    History.options.debug = true;

    var module = {};
    var manipulatingState = false;

    module.getComponentState = function() {
        var d = {};

        // TODO go through registered components
        d[pipeline.appContainer.getId()] = pipeline.appContainer.getState();
        d[pipeline.sidebar.getId()] = pipeline.sidebar.getState();
        d[pipeline.fakeframe.getId()] = pipeline.fakeframe.getState();
        d[pipeline.msselector.getId()] = pipeline.msselector.getState();
        d[pipeline.detailsframe.getId()] = pipeline.detailsframe.getState();

        return d
    };

    module.setComponentState = function(state) {
        // We need to distinguish between statechange events that come from the
        // browser and statechange events that originate from us. If we didn't,
        // updating the history to reflect the current component state would
        // trigger an event, which would trigger a setComponentState.
        if (manipulatingState) {
            History.debug("Bypassing setComponentState");
            return;
        }

        var callbacks = [function() {
            // these have to happen AFTER the page has been loaded!
            pipeline.msselector.setState(state[pipeline.msselector.getId()]);
            pipeline.detailsframe.setState(state[pipeline.detailsframe.getId()]);
        }];
        pipeline.appContainer.setState(state[pipeline.appContainer.getId()]);
        pipeline.sidebar.setState(state[pipeline.sidebar.getId()]);
        pipeline.fakeframe.setState(state[pipeline.fakeframe.getId()], callbacks);
    };

    module.pushState = function(title) {
        var newState = { componentState : module.getComponentState() };
        var newUrl = getUrl(newState);
        title = title || pipeline.sidebar.getTaskName();
        History.debug("Pushing new state: " + JSON.stringify(newState));
        try {
            manipulatingState = true;
            History.pushState(newState, title, newUrl);
        } finally {
            manipulatingState = false;
        }
    };

    module.replaceState = function(title) {
        var oldState = { componentState : module.getComponentState() };
        var newUrl = getUrl(oldState);
        title = title || pipeline.sidebar.getTaskName();
        History.debug("Updating current state: " + JSON.stringify(oldState));
        try {
            manipulatingState = true;
            History.replaceState(oldState, title, newUrl);
        } finally {
            manipulatingState = false;
        }
    };

    var getUrl = function(state) {
        var loc = window.location;
        var path = loc.pathname.substring(loc.pathname.lastIndexOf('/') + 1, loc.pathname.length);
        return path + pipeline.appContainer.getQueryUrl() + pipeline.sidebar.getQueryUrl() + pipeline.msselector.getQueryUrl() + pipeline.detailsframe.getQueryUrl();
    };

    return module;
})();


UTILS = (function() {
    var module = {};

    /**
     * Adds 0 left margin to the first thumbnail on each row that don't get it via CSS rules.
     * Recall the function when the floating of the elements changed.
     */
    module.fixThumbnailMargins = function() {
        $('ul.thumbnails').each(function() {
            var allThumbnails = $(this).children(),
                visibleThumbnails = $(allThumbnails).filter(":visible"),
                previousOffsetLeft;

            if (visibleThumbnails.length === 0) {
                return;
            }
            previousOffsetLeft = $(visibleThumbnails).first().offset().left;

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
    };

    module.redirectAnchorTarget = function(target) {
        $(target).find("a.replace").click(function(evt) {
            evt.preventDefault();

            // take a snapshot of the current state so we can restore the page on
            // return.
            pipeline.history.replaceState();

            var onSuccess = [function() {
                pipeline.msselector.setVisible(false);
                pipeline.history.pushState();

                // load the desired filter state for the new page from HTML5 local
                // storage
                var state = localStorage.getItem('newFilterState');
                if (state) {
                    var o = JSON.parse(state);
                    pipeline.detailsframe.setFilters(o);
                    localStorage.removeItem('newFilterState');
                }
            }];

            var callbackFn = $(this).data("callback");
            if (callbackFn) {
                if (callbackFn instanceof Array) {
                    // hacky js version of list.extend()
                    onSuccess.push.apply(onSuccess, callbackFn);
                } else {
                    onSuccess.push(callbackFn);
                }
            }

            // extract and save the desired filter state from the element,
            // ready for loading in the secondary page
            var filterState = UTILS.getFilterStateFromElement(this);
            var jsonFilterState = JSON.stringify(filterState);
            localStorage.setItem('newFilterState', jsonFilterState);

            pipeline.detailsframe.load(this.href, onSuccess);
        });
    };

    module.redirectPreAnchorTarget = function(target) {
        $(target).find("a.replace-pre").click(function(evt) {
            evt.preventDefault();

            // take a snapshot of the current state so we can restore the page on
            // return.
            pipeline.history.replaceState();

            var title = $(this).data("title") || $.url(this.href).attr('file');

            var onSuccess = [function() {
                pipeline.msselector.setVisible(false);
                pipeline.detailsframe.addPreMarkup(title);
                pipeline.history.pushState();
            }];

            pipeline.detailsframe.load(this.href, onSuccess);
        });
    };

    module.loadContent = function(target, href, onSuccess) {
        $(target).html("<div class=\"page-header\"><h1><span class=\"glyphicon glyphicon-refresh spinning\" style=\"vertical-align:top\"></span> Loading...</h1></div>");
        $(target).load(href, function(response, status, xhr) {
            if (status == "error") {
                var msg = "Error loading " + href + ":\n";
                $(target).html(msg + xhr.status + " " + xhr.statusText);
            };

            if (status == "success") {
                if (onSuccess) {
                    // keep compatibility with scalar callback function
                    if (onSuccess instanceof Array) {
                        for (var i = 0; i < onSuccess.length; i++) {
                            onSuccess[i]();
                        }
                    } else {
                        onSuccess();
                    }
                }
            }
        });
    };

    module.loadFakeframe = function(target, href) {
        var onSuccess = [function() {
            UTILS.redirectAnchorTarget(target);
            UTILS.redirectPreAnchorTarget(target);
        }];
        UTILS.loadContent(target, href, onSuccess);
    };

    module.getScoresForKey = function(scores_dict, key) {
        var scores = [],
            score = scores_dict[key];

        // value can be a number or a dict mapping correlation
        // axis to score
        if (score !== "null") {
            if (typeof(score) === "number") {
                scores.push(score);
            } else {
                for (var corr_axis in score) {
                    scores.push(score[corr_axis]);
                }
            }
        }

        return scores;
    };

    module.getData = function(scores_dict, key, filter) {
        var scores = [];

        for (var png in scores_dict) {
            var anchor = $("a[href='" + png + "']"),
                score;

            if ((filter === undefined) || ($(anchor).parent().is(filter))) {
                png_scores_dict = scores_dict[png];
                scores = scores.concat(module.getScoresForKey(png_scores_dict, key));
            }
        }
        return scores;
    };

    // A better idom for binding with resize is to debounce
    module.debounce = function(fn, timeout) {
        var timeoutID = -1;
        return function() {
            if (timeoutID > -1) {
                window.clearTimeout(timeoutID);
            }
            timeoutID = window.setTimeout(fn, timeout);
        }
    };

    // Get the desired filter state by reading the data-XXX values from the given element
    module.getFilterStateFromElement = function(v) {
        if (typeof v === 'undefined') {
            return {};
        }

        var state = {};
        var o = $(v);
        var dataKeys = ["vis", "spw", "ant", "pol", "field", "scan", "baseband", "tsys_spw", "intent", "type"];

        for (var i=0; i < dataKeys.length; i++) {
            var key = dataKeys[i];
            var data = o.data(key);
            if (typeof data !== "undefined") {
                // select2 elements are given ID of select-X, where X is the selector type
                state['select-' + key] = data;
            }
        }

        return state;
    };

    return module;
})();

FILTERS = (function() {
    var module = {};

    module.FilterPipeline = function() {
        var filters = [], // filters registered with the pipeline
            scores = {},  // scores dictionary
            refresh,      // refresh()
            that = this;  // reference to this for anonymous functions

        // filter PNGs with our list of filter objects, consolidating multiple
        // refresh request into one event via debounce
        refresh = UTILS.debounce(function() {
            filter();
            // changing the number of visible PNGs could mess up the margins on
            // row n > 1, so rejig the thumbnail layout on each refresh.
            UTILS.fixThumbnailMargins();
        }, 125);

        this.addFilter = function(filter) {
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

        this.setScores = function(newScores) {
            scores = newScores;
        };

        var filter = function() {
            // calculate the visibility of each thumbnail by..
            for (var png in scores) {
                // .. getting the score for the thumbnail..
                var score = scores[png];

                // .. finding whether it passes each filter..
                var isVisible = filters.reduce(function(previous, current) {
                    return previous && current.isVisible(score);
                }, true);

                // .. and showing/hiding the thumbnail based on the aggregate
                // filter state.
                var colDiv = $("a[href='" + png + "']").parent().parent();
                if (isVisible) {
                    colDiv.show();
                } else {
                    colDiv.hide();
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
        this.setRange = function(newMin, newMax) {
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

        this.isVisible = function(pngScoreDict) {
            if (!that.enabled) {
                return true;
            }

            var scores,
                score;

            scores = UTILS.getScoresForKey(pngScoreDict, scoreType);
            for (var i=0; i<scores.length; i++) {
                score = scores[i];
                if ((score >= min) && (score <= max)) {
                    return true;
                }
            }
            return false;
        };
    };


    module.MatchFilter = function(scoreType) {
        var visibleVals = [],
            that = this;

        // Set the filter passthrough values in response to external input, eg.
        // a spectral window being selected in the spw text field.
        this.setVisibleVals = function(vals) {
            visibleVals = vals;
            $(that).trigger("change");
        };

        this.isVisible = function(pngScoreDict) {
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

        $(element).change(function(e) {
            // get the spectral windows that were selected
            var selected = [];
            $(element + ' option:selected').each(function() {
                var t = $(this).text();
                selected.push(t);
            });
            filter.setVisibleVals(selected);
        });

        return filter;
    };

    return module;
})();

PLOTS = function() {
    var module = {};

    module.xAxisLabels = {
        "Tsys": function(xAxisLabel) {
            xAxisLabel.append("tspan")
                .text("T")
                .append("tspan")
                .attr("baseline-shift", "sub")
                .text("sys");
        },
        "WVR phase RMS": function(xAxisLabel) {
            xAxisLabel.text("Phase RMS ratio");
        },
        "unknown": function(xAxisLabel) {
            xAxisLabel.text("N/A");
        },
        "qa": function(xAxisLabel) {
            xAxisLabel.text("QA Score");
        },
        "K": function(xAxisLabel) {
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
        };

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
            width = shrinkFactor * $(reference).parent().width() - margin.left - margin.right;
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

    var createHistogramGetter = function(scores_dict, key, nBins) {
        var allScores = UTILS.getData(scores_dict, key);
        var extent = d3.extent(allScores);
        if ((extent[1] - extent[0]) === 0.0) {
            extent[0] = Math.min(extent[0], 0.9);
            extent[1] = Math.max(extent[1], 0.1);
        }
        var histogram = d3.layout.histogram().bins(nBins).range(extent);
        var allDataHistogram = histogram(allScores);
        var that = {};

        that.getSelectedDataHistogram = function() {
            var visibleScores = UTILS.getData(scores_dict, key, ":visible");
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

    module.easyHistogram = function(pipeline, scores, score_key, element_id,
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
        histogram.onBrush = function(lo, hi) {
            filter.setRange(lo, hi);
        };

        // subscribe to 'change' events from the FilterPipeline, refreshing the
        // selected data histogram on such an event.
        $(pipeline).on("change", function() {
           histogram.refreshSelectedData();
        });

        // refill the histogram to the size of the parent element when the
        // window, and thus the parent span width, changes.
        $(window).resize(UTILS.debounce(function() {
            histogram.resize();
        }, 125));

        return {
            "histogram": histogram,
            "filter": filter
        };
    }

    return module;
}();
