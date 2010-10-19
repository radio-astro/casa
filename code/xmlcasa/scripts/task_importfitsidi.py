import os
import shutil
from taskinit import *

def importfitsidi(fitsidifile,vis):
	"""Convert FITS-IDI visibility file into a CASA visibility file (MS).

	Keyword arguments:
	fitsidifile -- Name(s) of input FITS IDI file(s)
		default: None; example='3C273XC1.IDI' or ['3C273XC1.IDI1', '3C273XC1.IDI2']
	vis -- Name of output visibility file (MS)
		default: None; example: vis='3C273XC1.ms'
		
	"""

	#Python script
	try:
		casalog.origin('importfitsidi')
		casalog.post("")
		if(type(fitsidifile)==str):
			casalog.post('### Reading file '+fitsidifile, 'INFO')
			ms.fromfitsidi(vis,fitsidifile)
			ms.close()
		elif(type(fitsidifile)==list):
			clist = fitsidifile
			casalog.post('### Reading file '+clist[0], 'INFO')
			ms.fromfitsidi(vis,clist[0])
			ms.close()
			clist.pop(0)
			tname = '_importfitsidi_tmp_'+vis
			shutil.rmtree(tname, ignore_errors=True)
			for fidifile in clist:
				casalog.post('### Reading file '+fidifile, 'INFO')
				ms.fromfitsidi(tname,fidifile)
				ms.close()
				ms.open(vis, nomodify=False)
				ms.concatenate(msfile=tname, freqtol='', dirtol='')
				ms.close()
				shutil.rmtree(tname, ignore_errors=True)
		else:
                        raise Exception, 'Parameter fitsidifile should be of type str or list'			
	        # write history
		if ((type(vis)==str) & (os.path.exists(vis)) & ms.open(vis,nomodify=False)):
			ms.writehistory(message='taskname     = importfitsidi',origin='importfitsidi')
			ms.writehistory(message='fitsidifile  = "'+str(fitsidifile)+'"',origin='importfitsidi')
			ms.writehistory(message='vis          = "'+str(vis)+'"',origin='importfitsidi')
			ms.close()
                else:
                        raise Exception, 'Cannot write to history table of output MS.'

	except Exception, instance: 
		print '*** Error ***',instance
		shutil.rmtree('_importfitsidi_tmp_'+vis, ignore_errors=True)
		raise Exception, instance


