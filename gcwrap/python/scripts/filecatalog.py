import sys
import os
import string
import inspect

from parameter_check import *
from casa_stack_manip import stack_frame_find

def filecatalog():
	""" Open the File Catalog GUI:

	"""
        myf=stack_frame_find( )
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
        	#logpid=os.system('open -a casalogger.app casa.log')
	elif (os.uname()[0]=='Linux'):
        	fcpid=os.spawnlp(os.P_NOWAIT,'casafilecatalog','casafilecatalog')
	else:
        	print 'Unrecognized OS: No filecatalog available'

	myf['fcpid']=fcpid
