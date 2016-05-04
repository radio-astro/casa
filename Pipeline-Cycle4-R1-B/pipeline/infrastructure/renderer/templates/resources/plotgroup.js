$(document).ready(function () {
    // Show/hide images depending on selected spectral window
    $(".plotfilter button").click(function () {
        var toFilter = {},
            clicked = this,
            parentIndex;

        // create a list of the active buttons for each button group
        $(".plotfilter").each(function (index) {
            var activeValues = [];
            $(this).children(".active").each(function () {
                activeValues.push($(this).data("value"));
            });

            toFilter[index] = activeValues;

            if ($.contains(this, clicked)) {
                parentIndex = index;
            }
        });

        // active class is added or removed *after* this event handler has
        // completed, so we need to add this button if it's not already active
        // or remove it if it's just been deselected
        if ($(this).hasClass("active")) {
            toFilter[parentIndex].splice(
                toFilter[parentIndex].indexOf($(this).data("value"))
                , 1)
        } else {
            toFilter[parentIndex].push($(this).data("value"));
        }

        $.each(toFilter, function (key, value) {
            $.each(value, function (index, selector) {
                toFilter[key][index] = selector.toLowerCase();
            });
        });

        function disjoin(a, b) {
            return $.grep(a, function ($e) {
                return $.inArray($e, b) == -1;
            });
        };

        $("ul.thumbnails li").each(function () {
            var imgAttrs = $(this).data("value").toLowerCase().split(" ");
            var isVisible = true;

            $.each(toFilter, function (index, activeSelectors) {
                var disjoint = disjoin(activeSelectors, imgAttrs);

                isVisible = isVisible && (activeSelectors.length > disjoint.length
                                          || activeSelectors.length === 0);
            });

            if (isVisible) {
                $(this).show();
            } else {
                $(this).hide();
            }
        });

        UTILS.fixThumbnailMargins();
    });

    $("#clearbutton").click(function () {
        // clear the state of all toggle buttons
        $(".plotfilter button.active").each(function () {
            $(this).click();
        });
    });

    $("ul.thumbnails li div a").click(function (evt) {
        evt.preventDefault();
        var target = this.href;
        UTILS.launchFancybox(target);
        return false;
    });

    UTILS.fixThumbnailMargins();
});
