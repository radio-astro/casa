import os
import shutil
import stat
from taskinit import *

def concat(vislist,concatvis,freqtol,dirtol):
	"""Concatenate two visibility data sets.
	A second data set is appended to the input data set with
	checking of the frequency and position.

	Keyword arguments:
	vis -- Name of input visibility file (MS)
		default: none; example: vis='ngc5921.ms'
	concatvis -- Name of visibility file to append to the input'		
		default: none; example: concatvis='src2.ms'
	freqtol -- Frequency shift tolerance for considering data as the same spwid
		default: ''  means always combine
		example: freqtol='10MHz' will not combine spwid unless they are
		within 10 MHz
	dirtol -- Direction shift tolerance for considering data as the same field
		default: ;; means always combine
		example: dirtol='1.arcsec' will not combine data for a field unless
		their phase center is less than 1 arcsec.

	"""

        ###
	#Python script
	try:
		casalog.origin('concat')
		#break the reference between vis and vislist as we modify vis
		if(type(vislist)==str):
			vis=[vislist]
		else:
			vis=list(vislist)
		if((type(concatvis)!=str) or (len(concatvis.split()) < 1)):
			raise Exception, 'parameter concatvis is invalid'
		if(vis.count(concatvis) > 0):
			vis.remove(concatvis)

		if(os.path.exists(concatvis)):
			casalog.post('Will be concatenating into the existing ms '+concatvis , 'WARN')
		else:
			if(len(vis) >1):
				casalog.post('copying '+vis[0]+' to '+concatvis , 'INFO')
				shutil.copytree(vis[0],concatvis)
				# set the mode of the entire target MS to rwxr-x-r-x (recursive chmod does not exist in Python)
				os.chmod(concatvis, stat.S_IRWXU | stat.S_IRGRP |  stat.S_IXGRP | stat.S_IROTH | stat.S_IXOTH )
				for root, dirs, files in os.walk(concatvis):
					for name in files:
						os.chmod(os.path.join(root, name), stat.S_IRWXU | stat.S_IRGRP |  stat.S_IXGRP | stat.S_IROTH | stat.S_IXOTH )
					for name in dirs:
						os.chmod(os.path.join(root, name), stat.S_IRWXU | stat.S_IRGRP |  stat.S_IXGRP | stat.S_IROTH | stat.S_IXOTH )

				vis.remove(vis[0])

                if ((type(concatvis)==str) & (os.path.exists(concatvis))):
                        ms.open(concatvis,False) # nomodify=False to enable writing
                else:
                        raise Exception, 'Visibility data set '+concatvis+' not found - please verify the name'
	
		for elvis in vis :
			###Oh no Elvis does not exist Mr Bill
			if(not os.path.exists(elvis)):
				raise Exception, 'Visibility data set '+elvis+' not found - please verify the name'
			casalog.post('concatenating '+elvis+' into '+concatvis , 'INFO')
			ms.concatenate(msfile=elvis,freqtol=freqtol,dirtol=dirtol)
			ms.writehistory(message='taskname=concat',origin='concat')
			ms.writehistory(message='vis         = "'+str(concatvis)+'"',origin='concat')
			ms.writehistory(message='concatvis   = "'+str(elvis)+'"',origin='concat')
			ms.writehistory(message='freqtol     = "'+str(freqtol)+'"',origin='concat')
			ms.writehistory(message='dirtol      = "'+str(dirtol)+'"',origin='concat')
		ms.close()

	except Exception, instance:
		print '*** Error ***',instance
		raise Exception, instance

