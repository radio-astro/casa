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

    // Show/hide images depending on selected spectral window
    $("#filter_selector .selector li").click(function() {
        // get the identifier of the button that the user clicked on
        var selected = $(this).attr("id");

        // toggle the state of that button
        $("#filter_selector ul.selector li#" + selected + " a").toggleClass("black")

        // show all thumbnails so that..
        $("ul.thumb li").show()
        $("ul.thumb li").children().attr({ "rel" : "fancybox-thumb" });

        // .. for each filter..
        $("#filter_selector ul.selector").each(function() {
            // .. we can get the ids of all enabled selectors..
            var selectedIds = $(this).find("li:has(a.black)").map(function() {
                var x = this.id;
                return "." + this.id;
            });
            var ids = selectedIds.toArray().join(",");

           if (ids !== "") {
                $("ul.thumb li:not(" + ids + ")").hide();
                $("ul.thumb li:not(" + ids + ")").children().attr({ "rel" : "" });
            };
        });

        return false;
    });

    $("#clearbutton").click(function() {
        // clear the state of all toggle buttons
		$("#filter_selector ul.selector li a").removeClass("black")

		// show all thumbnails and link them together
		$("ul.thumb li").show()
		$("ul.thumb li").children().attr({ "rel" : "fancybox-thumb" });
	});

    function launchFancyboxInParent(target){
    	var basedir = $.url().segment(-2);
    	var thumbs = new Array();
    	var index = 0;
        $("ul.thumb li:visible a").each(function() {        
        	var mainImage = $(this).attr("href"); //Find Image href
            var title = $(this).attr("title"); //Find Image title
            if (mainImage == target) {
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
});
