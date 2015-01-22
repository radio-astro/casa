// listen for FancyBox events from child iframes 
function receiveMessage(event) {
	cmd = event.data['command'];
	if (cmd != 'fancybox') 
		return false;
	thumbs = event.data['thumbs'];
	index = event.data['index'];
	$.fancybox(thumbs,
		{
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
					height	: 50
				}
			}
		}
	);
}
window.addEventListener('message', receiveMessage, false);
