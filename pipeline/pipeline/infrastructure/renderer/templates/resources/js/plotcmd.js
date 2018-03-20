// ==========================================================================
//
// Show CASA plot command
// Enables overlay showing plot command
//
// ==========================================================================
;(function (document, $) {
    'use strict';

	// Collection of methods supported by user browser
	var fn = (function () {

		var fnMap = [
			[
				'requestFullscreen',
				'exitFullscreen',
				'fullscreenElement',
				'fullscreenEnabled',
				'fullscreenchange',
				'fullscreenerror'
			],
			// new WebKit
			[
				'webkitRequestFullscreen',
				'webkitExitFullscreen',
				'webkitFullscreenElement',
				'webkitFullscreenEnabled',
				'webkitfullscreenchange',
				'webkitfullscreenerror'

			],
			// old WebKit (Safari 5.1)
			[
				'webkitRequestFullScreen',
				'webkitCancelFullScreen',
				'webkitCurrentFullScreenElement',
				'webkitCancelFullScreen',
				'webkitfullscreenchange',
				'webkitfullscreenerror'

			],
			[
				'mozRequestFullScreen',
				'mozCancelFullScreen',
				'mozFullScreenElement',
				'mozFullScreenEnabled',
				'mozfullscreenchange',
				'mozfullscreenerror'
			],
			[
				'msRequestFullscreen',
				'msExitFullscreen',
				'msFullscreenElement',
				'msFullscreenEnabled',
				'MSFullscreenChange',
				'MSFullscreenError'
			]
		];

		var val;
		var ret = {};
		var i, j;

		for ( i = 0; i < fnMap.length; i++ ) {
			val = fnMap[ i ];

			if ( val && val[ 1 ] in document ) {
				for ( j = 0; j < val.length; j++ ) {
					ret[ fnMap[ 0 ][ j ] ] = val[ j ];
				}

				return ret;
			}
		}

		return false;
	})();

    $.extend(true, $.fancybox.defaults, {
        btnTpl : {
            plotCommand :
                '<button data-fancybox-plotcmd class="fancybox-button fancybox-button--plotcmd" title="Show plot command">' +
                    '<svg viewBox="0 0 1792 1792">' +
                        '<path d="M649 983l-466 466q-10 10-23 10t-23-10l-50-50q-10-10-10-23t10-23l393-393-393-393q-10-10-10-23t10-23l50-50q10-10 23-10t23 10l466 466q10 10 10 23t-10 23zm1079 457v64q0 14-9 23t-23 9h-960q-14 0-23-9t-9-23v-64q0-14 9-23t23-9h960q14 0 23 9t9 23z"/>' +
                    '</svg>' +
                '</button>'
        }
    });

    var PlotCommand = function( instance ) {
        this.instance = instance;
        this.init();
    };

    $.extend( PlotCommand.prototype, {
        $button  : null,

        init : function() {
            var self = this;
            self.$button = self.instance.$refs.toolbar.find('[data-fancybox-plotcmd]').on('click', function () {
                self.show();
            });
        },

        setButtonState: function() {
            var self = this;
            var instance = $.fancybox.getInstance();
            var current = instance.current;
            var hasCommand = (current.opts.$orig && current.opts.$orig.data("plotcommandtarget") !== undefined);
            var isFullScreen = (instance.FullScreen && instance.FullScreen.isFullscreen());

            if (hasCommand && !isFullScreen) {
                self.$button.show();
            } else {
                self.$button.hide();
            }
        },

        show: function () {
            var self = this;
            var target;

            if (self.instance && self.instance.current) {
                target = self.instance.current.opts.$orig.data("plotcommandtarget");
                $.fancybox.open({
                    src : target,
                    type : 'inline',
                    opts : {
                        smallBtn : false,
                        buttons : false
                    }
                });
            }
        }
    });

    $(document).on({
        'onInit.fb': function (e, instance) {
            if (instance && !instance.PlotCommand) {
                instance.PlotCommand = new PlotCommand(instance);
            }
        },

        'afterLoad.fb': function (e, instance) {
            if (instance && instance.PlotCommand) {
                instance.PlotCommand.setButtonState();
            }
        }
    });

	$(document).on(fn.fullscreenchange, function() {
        var instance = $.fancybox.getInstance();

            if (instance && instance.PlotCommand) {
                instance.PlotCommand.setButtonState();
            }
    });

}( document, window.jQuery || jQuery ));
