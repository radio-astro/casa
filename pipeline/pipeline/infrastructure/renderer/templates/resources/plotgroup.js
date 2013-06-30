$(document).ready(function(){
    /**
     * Adds 0 left margin to the first thumbnail on each row that don't get it via CSS rules.
     * Recall the function when the floating of the elements changed.
     */
    function fixThumbnailMargins() {
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

    
    // Show/hide images depending on selected spectral window
    $(".plotfilter button").click(function() {
	var toFilter = [],
	    index;
	
	$(".plotfilter button.active").each(function() {
	    var filterClass = $(this).data("value");
	    toFilter.push(filterClass);
	});

	// active class is added or removed *after* this event handler, so we
	// need to add this button if it's not already active.
	if ($(this).hasClass("active")) {	    
	    index = toFilter.indexOf($(this).data("value"));
	    toFilter.splice(index, 1)
	} else {
	    toFilter.push($(this).data("value"));
	}

	for (index=0; index<toFilter.length; index += 1) {
	    toFilter[index] = toFilter[index].toLowerCase();
	}

	function disjoin(a, b) {
	    return $.grep(a, function($e) { return $.inArray($e, b) == -1; });
	};
	
	$("ul.thumbnails li").each(function() {
	    var imgAttrs = $(this).data("value").toLowerCase().split(" "),
	    	i;
	    // get the intersection of the image attributes and active selectors
	    // lists
	    var disjoint = disjoin(toFilter, imgAttrs);
	    
	    if (disjoint.length === 0) {
		$(this).show();
	    } else {
		if (toFilter.length === 0) {
		    $(this).show();
		} else {
		    $(this).hide();
		}
	    }
	});
	
	fixThumbnailMargins();	
    });

    $("#clearbutton").click(function() {
        // clear the state of all toggle buttons
	$(".plotfilter button.active").each(function() {
	    $(this).click();
	});
    });

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
	    loop : false,
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
    
    fixThumbnailMargins();
});
