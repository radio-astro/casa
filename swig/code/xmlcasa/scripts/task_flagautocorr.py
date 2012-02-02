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

        fglocal = casac.homefinder.find_home_by_name('flaggerHome').create()
        mslocal = casac.homefinder.find_home_by_name('msHome').create()

	#Python script
	try:
                if ((type(vis)==str) & (os.path.exists(vis))):
                        fglocal.open(vis)
                else:
			raise Exception, 'Visibility data set not found - please verify the name'
		fglocal.setdata()
		fglocal.setmanualflags(autocorrelation=True)
		fglocal.run()
        
	        #write history
        	mslocal.open(vis,nomodify=False)
        	mslocal.writehistory(message='taskname = flagautocorr',origin='flagautocorr')
        	mslocal.writehistory(message='vis         = "'+str(vis)+'"',origin='flagautocorr')
        	mslocal.close()
        
	except Exception, instance:
	        print '*** Error ***',instance

