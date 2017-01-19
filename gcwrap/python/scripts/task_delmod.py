import os
from taskinit import *
from parallel.parallel_task_helper import ParallelTaskHelper

_ms = mstool( )
_cb = cbtool( )

def delmod(vis=None,otf=None,field=None,scr=None):

        casalog.origin('delmod')

        # Do the trivial parallelization
        if ParallelTaskHelper.isParallelMS(vis):
                helper = ParallelTaskHelper('delmod', locals())
                helper.go()
                return


	#Python script
	try:

		# only if vis exists...
		if ((type(vis)==str) & (os.path.exists(vis))):
			# ... and we are asked to do something...
			# open without adding anything!
			_cb.open(vis,addcorr=False,addmodel=False)
			_cb.delmod(otf=otf,field=field,scr=scr)
			_cb.close()
		else:
                        raise Exception, 'Visibility data set not found - please verify the name'

        	#write history
        	_ms.open(vis,nomodify=False)
        	_ms.writehistory(message='taskname = delmod',origin='delmod')
        	_ms.writehistory(message='vis         = "'+str(vis)+'"',origin='delmod')
		_ms.writehistory(message='otf         = "'+str(otf)+'"',origin='delmod')
		_ms.writehistory(message='scr         = "'+str(scr)+'"',origin='delmod')
		_ms.close()

	except Exception, instance:
		print '*** Error ***',instance
