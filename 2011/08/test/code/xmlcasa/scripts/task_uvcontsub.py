import os
from taskinit import *

def uvcontsub(vis,field,fitspw,spw,solint,fitorder,fitmode,splitdata):

	#Python script
	try:
		casalog.origin('uvcontsub')
                myms = mstool.create()
                if ((type(vis)==str) & (os.path.exists(vis))):
                        myms.open(vis,nomodify=False)
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'


		if ((splitdata==True) & (fitmode!='subtract')):
			raise Exception, "Use splitdata=True only with fitmode='subtract'"

		myms.continuumsub(field=field,fitspw=fitspw,spw=spw,
				solint=solint,fitorder=fitorder,mode=fitmode)

		if ((splitdata==True) & (fitmode=='subtract')):
			myms.split(vis.rstrip('/')+'.cont',field=field,spw=spw,whichcol='MODEL_DATA')
			myms.split(vis.rstrip('/')+'.contsub',field=field,spw=spw,whichcol='CORRECTED_DATA')
	
       		myms.writehistory('taskname = uvcontsub',origin='uvcontsub')
       		myms.writehistory(message='vis         = "'+str(vis)+'"',origin='uvcontsub')
       		myms.writehistory(message='field       = "'+str(field)+'"',origin='uvcontsub')
        	myms.writehistory(message='fitspw       = '+str(spw),origin='uvcontsub')
        	myms.writehistory(message='spw       = '+str(spw),origin='uvcontsub')
        	myms.writehistory(message='solint      = '+str(solint),origin='uvcontsub')
        	myms.writehistory(message='fitorder    = '+str(fitorder),origin='uvcontsub')
        	myms.writehistory(message='fitmode     = "'+str(fitmode)+'"',origin='uvcontsub')
		myms.writehistory(message='splitdata   = '+str(splitdata),origin='uvcontsub')
        	myms.close()

	except Exception, instance:
		print '*** Error ***',instance
		raise Exception, instance

