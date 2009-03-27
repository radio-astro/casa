import sys
import os
import string
import inspect


from parameter_check import *

def filecatalog():
	""" Open the File Catalog GUI:

	"""
        a=inspect.stack()
        stacklevel=0
        for k in range(len(a)):
          if (string.find(a[k][1], 'ipython console') > 0):
                stacklevel=k
                break
        myf=sys._getframe(stacklevel).f_globals
        myf['__last_task']='filecatalog'
        ###
        #Handle globals or user over-ride of arguments
	#Python script
	if (os.uname()[0]=='Darwin'):
        	from Carbon.Launch import LSFindApplicationForInfo
        	from Carbon.CoreFoundation import kCFURLPOSIXPathStyle
        	kLSUnknownCreator = '\x00\x00\x00\x00'
        	fsRef, cfURL = LSFindApplicationForInfo(kLSUnknownCreator, None, "casafilecatalog.app")
        	filecatalog_path = os.path.join(fsRef.as_pathname(), 'Contents', 'MacOS', 'casafilecatalog')
		fcpid=os.spawnvp(os.P_NOWAIT, filecatalog_path, [filecatalog_path])
        	#logpid=os.system('open -a casalogger.app casapy.log')
	elif (os.uname()[0]=='Linux'):
        	fcpid=os.spawnlp(os.P_NOWAIT,'casafilecatalog','casafilecatalog')
	else:
        	print 'Unrecognized OS: No filecatalog available'

	myf['fcpid']=fcpid
