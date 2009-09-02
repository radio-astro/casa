import os
import shutil
import stat
from taskinit import *

def initcal(vis=None):

	# we will initialize scr cols only if we don't create them
	doinit=False;

	tb.open(vis)
	doinit = (tb.colnames().count('CORRECTED_DATA')>0)
	tb.close()
	cb.open(vis)

	# If necessary (scr col not just created), initialize scr cols
	if doinit:
		cb.initcalset()
	cb.close()


def concat(vislist,concatvis,freqtol,dirtol,timesort):
	"""Concatenate two visibility data sets.
	A second data set is appended to the input data set with
	checking of the frequency and position.
	If none of the input MSs have any scratch columns, none are created.
	Otherwise scratch columns are created and initialized in those MSs
	which don't have a complete set.


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
	timesort -- if true, sort the main table of the resulting MS by time
	        default: false; example: timesort=true

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
				tb.open(vis[0])
				tb.copy(concatvis, deep=True, valuecopy=True)
				tb.close()
				# note that the resulting copy is writable even if the original was read-only
				vis.remove(vis[0])

		# Determine if scratch columns should be considered at all
		# by checking if any of the MSs has them.
		
		considerscrcols = False
		needscrcols = []
                if ((type(concatvis)==str) & (os.path.exists(concatvis))):
			
			# check if all scratch columns are present
			tb.open(concatvis)
			if(tb.colnames().count('CORRECTED_DATA')==1 
			   or  tb.colnames().count('MODEL_DATA')==1 
			   or  tb.colnames().count('IMAGING_WEIGHT')==1):
				considerscrcols = True  # there are scratch columns
				
			needscrcols.append(tb.colnames().count('CORRECTED_DATA')==0 
					   or  tb.colnames().count('MODEL_DATA')==0 
					   or  tb.colnames().count('IMAGING_WEIGHT')==0)			
			tb.close()
                else:
                        raise Exception, 'Visibility data set '+concatvis+' not found - please verify the name'

		for elvis in vis : 			###Oh no Elvis does not exist Mr Bill
			if(not os.path.exists(elvis)):
				raise Exception, 'Visibility data set '+elvis+' not found - please verify the name'

			# check if all scratch columns are present
			tb.open(elvis)
			if(tb.colnames().count('CORRECTED_DATA')==1 
			   or  tb.colnames().count('MODEL_DATA')==1 
			   or  tb.colnames().count('IMAGING_WEIGHT')==1):
				considerscrcols = True  # there are scratch columns

			needscrcols.append(tb.colnames().count('CORRECTED_DATA')==0 
					  or  tb.colnames().count('MODEL_DATA')==0 
					  or  tb.colnames().count('IMAGING_WEIGHT')==0)			
			tb.close()

		# start actual work, file existence has already been checked
		i = 0
		if(considerscrcols and needscrcols[i]):
			# create scratch cols			
			casalog.post('creating scratch columns in '+concatvis , 'INFO')
			cb.open(concatvis) # calibrator-open creates scratch columns
			cb.close()

		ms.open(concatvis,False) # nomodify=False to enable writing
	
		for elvis in vis : 
			i = i + 1
			casalog.post('concatenating '+elvis+' into '+concatvis , 'INFO')
			if(considerscrcols and needscrcols[i]):
				# create scratch cols			
				casalog.post('creating scratch columns for '+elvis+' (original MS unchanged)', 'INFO')
				tempname = elvis+'_with_scrcols'
				os.system('rm -rf '+tempname)
				os.system('cp -R '+elvis+' '+tempname)
				cb.open(tempname) # calibrator-open creates scratch columns
				cb.close()
				# concatenate copy instead of original file
				ms.concatenate(msfile=tempname,freqtol=freqtol,dirtol=dirtol)
				os.system('rm -rf '+tempname)
			else:
				ms.concatenate(msfile=elvis,freqtol=freqtol,dirtol=dirtol)

			ms.writehistory(message='taskname=concat',origin='concat')
			ms.writehistory(message='vis         = "'+str(concatvis)+'"',origin='concat')
			ms.writehistory(message='concatvis   = "'+str(elvis)+'"',origin='concat')
			ms.writehistory(message='freqtol     = "'+str(freqtol)+'"',origin='concat')
			ms.writehistory(message='dirtol      = "'+str(dirtol)+'"',origin='concat')

		if(timesort):
			casalog.post('Sorting main table by TIME ...', 'INFO')
			ms.timesort()

		ms.close()
			

	except Exception, instance:
		print '*** Error ***',instance
		raise Exception, instance

