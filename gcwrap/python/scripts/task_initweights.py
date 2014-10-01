import os
from taskinit import *
from parallel.parallel_task_helper import ParallelTaskHelper

def initweights(vis=None,dobt=None,dowtsp=None):

        casalog.origin('initweights')

        # Do the trivial parallelization
        if ParallelTaskHelper.isParallelMS(vis):
                helper = ParallelTaskHelper('initweights', locals())
                helper.go()
                return


	#Python script
	try:
		mycb=cbtool()

		# only if vis exists...
		if ((type(vis)==str) & (os.path.exists(vis))):
			# ... and we are asked to do something...
			# open without adding anything!
			mycb.open(vis,compress=False,addcorr=False,addmodel=False)
			mycb.initweights(dobt=dobt,dowtsp=dowtsp)
			mycb.close()
		else:
                        raise Exception, 'Visibility data set not found - please verify the name'

        	#write history
        	ms.open(vis,nomodify=False)
        	ms.writehistory(message='taskname = initweights',origin='initweights')
        	ms.writehistory(message='vis         = "'+str(vis)+'"',origin='initweights')
		ms.writehistory(message='dobt        = "'+str(dobt)+'"',origin='initweights')
		ms.writehistory(message='dowtsp      = "'+str(dowtsp)+'"',origin='initweights')
		ms.close()

	except Exception, instance:
		print '*** Error ***',instance
