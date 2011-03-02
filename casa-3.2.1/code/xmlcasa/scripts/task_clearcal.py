import os
from taskinit import *

def clearcal(vis=None,field=None,spw=None):

        casalog.origin('clearcal')

	#Python script
	try:

		# we will initialize scr cols only if we don't create them
		doinit=False;

                if ((type(vis)==str) & (os.path.exists(vis))):
			tb.open(vis)
			doinit = (tb.colnames().count('CORRECTED_DATA')>0)
			tb.close()

			# We ignore selection if creating the scratch columns
			if (not(doinit)):
				casalog.post('Need to create scratch columns; ignoring selection.')

                        cb.open(vis)
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'

		# If necessary (scr col not just created), initialize scr cols
		if doinit:
			cb.selectvis(field=field,spw=spw)
			cb.initcalset()
		cb.close()


        	#write history
        	ms.open(vis,nomodify=False)
        	ms.writehistory(message='taskname = clearcal',origin='clearcal')
		if (doinit):
			ms.writehistory(message='field       = "'+str(field)+'"',origin='clearcal')
			ms.writehistory(message='spw         = "'+str(spw)+'"',origin='clearcal')
        	ms.writehistory(message='vis         = "'+str(vis)+'"',origin='clearcal')
		ms.close()

	except Exception, instance:
		print '*** Error ***',instance
