import os
from taskinit import *
from parallel.parallel_task_helper import ParallelTaskHelper

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
			cb.open(vis,addcorr=False,addmodel=False)
			cb.delmod(otf=otf,field=field,scr=scr)
			cb.close()
		else:
                        raise Exception, 'Visibility data set not found - please verify the name'

        	#write history
        	ms.open(vis,nomodify=False)
        	ms.writehistory(message='taskname = delmod',origin='delmod')
        	ms.writehistory(message='vis         = "'+str(vis)+'"',origin='delmod')
		ms.writehistory(message='otf         = "'+str(otf)+'"',origin='delmod')
		ms.writehistory(message='scr         = "'+str(scr)+'"',origin='delmod')
		ms.close()

	except Exception, instance:
		print '*** Error ***',instance
