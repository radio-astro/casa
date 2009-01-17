import os
from taskinit import *

def flagautocorr(vis=None):
	""" Flag autocorrelations:
	Flags all autocorrelation data in the measurements set.
	Unless these data will be specifically used in the subsequent
	reductions, they should be flagged.

	Keyword arguments:
	vis -- Name of input visibility file (MS)
		default: none. example: vis='ngc5921.ms'

	"""
	casalog.origin('flagautocorr')

	#Python script
	try:
		##Let us start by clearing the state of flagger especially
		##if its a global
		fg.clearflagselection(0)
                if ((type(vis)==str) & (os.path.exists(vis))):
                        fg.open(vis)
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'
		fg.setdata()
		fg.setmanualflags(autocorrelation=True)
		fg.run()
		fg.done()
        

	        #write history
        	ms.open(vis,nomodify=False)
        	ms.writehistory(message='taskname = flagautocorr',origin='flagautocorr')
        	ms.writehistory(message='vis         = "'+str(vis)+'"',origin='flagautocorr')
        	ms.close()
        
	except Exception, instance:
	        print '*** Error ***',instance

