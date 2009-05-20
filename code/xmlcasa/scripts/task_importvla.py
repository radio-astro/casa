import os
from taskinit import *

def importvla(archivefiles,vis,
	      bandname,frequencytol,
	      project,
	      starttime,stoptime,
	      applytsys,
	      autocorr,antnamescheme,keepblanks):

	#Python script
	i=0
	overwrite=True
	try:
		casalog.origin('importvla')
		if ((type(vis)!=str) | (vis=='') | (os.path.exists(vis))): 
			raise Exception, 'Need valid visibility file name (bad name or already exists)'
		if (os.path.exists(vis)): raise Exception, 'Visibility file already exists - remove or rename'
		for archivefile in archivefiles:
			if i>0: overwrite=False
			if ((type(archivefile)==str) & (os.path.exists(archivefile))):
				vlafiller(msname=vis,inputfile=archivefile,
					  overwrite=overwrite,
					  bandname=bandname,freqtol=frequencytol,
					  project=project, start=starttime,
					  stop=stoptime, applytsys=applytsys,
					  keepautocorr=autocorr,
					  antnamescheme=antnamescheme,
					  keepblanks=keepblanks)
				i=i+1
			else:
				raise Exception, 'Archive file not found - please verify the name'

	        #write history
                if ((type(vis)==str) & (os.path.exists(vis))):
                        ms.open(vis,nomodify=False)
                	ms.writehistory(message='taskname = importvla',origin='imporvla')
                	ms.writehistory(message='archivefiles= '+str(archivefiles),origin='imporvla')
                	ms.writehistory(message='vis         = "'+str(vis)+'"',origin='imporvla')
                	ms.writehistory(message='bandname    = "'+str(bandname)+'"',origin='imporvla')
                	ms.writehistory(message='frequencytol= '+str(frequencytol),origin='imporvla')
                	ms.writehistory(message='applytsys= '+str(applytsys),origin='imporvla')
                	ms.writehistory(message='keepblanks= '+str(keepblanks),origin='imporvla')
                	ms.close()

                else:
                        raise Exception, 'Visibility data set not found - please verify the name'

                # write initial flag version
                ok=fg.open(vis);
                ok=fg.saveflagversion('Original',comment='Original flags at import into CASA',merge='replace')
                ok=fg.done();


	except Exception, instance:
		print '*** Error ***',instance
		raise Exception, instance

