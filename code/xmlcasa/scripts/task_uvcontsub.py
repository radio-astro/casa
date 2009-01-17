import os
from taskinit import *

def uvcontsub(vis,field,fitspw,spw,solint,fitorder,fitmode,splitdata):

	#Python script
	try:
		casalog.origin('uvcontsub')
                if ((type(vis)==str) & (os.path.exists(vis))):
                        ms.open(vis,nomodify=False)
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'


		if ((splitdata==True) & (fitmode!='subtract')):
			raise Exception, "Use splitdata=True only with fitmode='subtract'"

		ms.continuumsub(field=field,fitspw=fitspw,spw=spw,
				solint=solint,fitorder=fitorder,mode=fitmode)

		if ((splitdata==True) & (fitmode=='subtract')):
			ms.split(vis.rstrip('/')+'.cont',field=field,spw=spw,whichcol='MODEL_DATA')
			ms.split(vis.rstrip('/')+'.contsub',field=field,spw=spw,whichcol='CORRECTED_DATA')
	
		#ms.close()
	        #ms.open(vis,nomodify=False)
       		ms.writehistory('taskname = uvcontsub',origin='uvcontsub')
       		ms.writehistory(message='vis         = "'+str(vis)+'"',origin='uvcontsub')
       		ms.writehistory(message='field       = "'+str(field)+'"',origin='uvcontsub')
        	ms.writehistory(message='fitspw       = '+str(spw),origin='uvcontsub')
        	ms.writehistory(message='spw       = '+str(spw),origin='uvcontsub')
        	ms.writehistory(message='solint      = '+str(solint),origin='uvcontsub')
        	ms.writehistory(message='fitorder    = '+str(fitorder),origin='uvcontsub')
        	ms.writehistory(message='fitmode     = "'+str(fitmode)+'"',origin='uvcontsub')
		ms.writehistory(message='splitdata   = '+str(splitdata),origin='uvcontsub')
        	ms.close()

	except Exception, instance:
		print '*** Error ***',instance
		raise Exception, instance

