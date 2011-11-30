import os
import shutil
import stat
import time
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


def concat(vislist,concatvis,freqtol,dirtol,timesort,copypointing,visweightscale,createmms):
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
	copypointing -- copy all rows of the pointing table
	        default: True
	visweightscale -- list of the weight scales to be applied to the individual MSs
	        default: [] (don't modify weights, equivalent to setting scale to 1 for each MS)
	createmms -- should this create multi-MS?
	        default: False

	"""

        ###
	#Python script
	try:
		casalog.origin('concat')
		t = tbtool.create()
		m = mstool.create()
		
		#break the reference between vis and vislist as we modify vis
		if(type(vislist)==str):
			vis=[vislist]
		else:
			vis=list(vislist)
		#dto. for concavis
		theconcatvis = concatvis

		doweightscale = False
		if(len(visweightscale)>0):
			if (len(visweightscale) != len(vis)):
				raise Exception, 'parameter visweightscale must have same number of elements as parameter vis'
			for factor in visweightscale:
				if factor<0.:
					raise Exception, 'parameter visweightscale must only contain positive numbers'
				elif factor!=1.:
					doweightscale=True

		if(timesort and createmms):
			raise Exception, 'Sorting by time not possible in multi-MS.'
			
		if((type(concatvis)!=str) or (len(concatvis.split()) < 1)):
			raise Exception, 'parameter concatvis is invalid'
		if(vis.count(concatvis) > 0):
			vis.remove(concatvis)

		if(os.path.exists(concatvis)):
			if(createmms):
				raise Exception, 'When creating a multi-MS, the output MS must not yet exist.'
			else:
				casalog.post('Will be concatenating into the existing ms '+concatvis , 'WARN')
		else:
			if(createmms):
				if(os.path.exists(concatvis+'.data')):
					raise Exception, 'When creating a multi-MS, the directory concavis+\'.data\' must not yet exist.'
				else:
					os.mkdir(concatvis+'.data')
			if(len(vis) >0): # (note: in case len is 1, we only copy, essentially)
				if(createmms):
					theconcatvis = concatvis+'.data/'+vis[0]
				casalog.post('copying '+vis[0]+' to '+theconcatvis , 'INFO')
				shutil.copytree(vis[0], theconcatvis)
				# note that the resulting copy is writable even if the original was read-only
				vis.remove(vis[0])

		if not copypointing: # remove the rows from the POINTING table of the first MS
			casalog.post('*** copypointing==False: resulting MS will have empty POINTING table.', 'INFO')
			tmptabname = 'TMPPOINTING'+str(time.time())
			shutil.rmtree(tmptabname, ignore_errors=True)
			shutil.move(theconcatvis+'/POINTING', tmptabname)
			t.open(tmptabname)
			if(t.nrows()>0): 
				t.copy(newtablename=theconcatvis+'/POINTING', deep=False, valuecopy=True, norows=True)
				t.close()
				shutil.rmtree(tmptabname, ignore_errors=True)
			else: # the POINTING table is already empty
				casalog.post('***    Input POINTING table was already empty.', 'INFO')
				shutil.move(tmptabname, theconcatvis+'/POINTING')
				t.close()

		# Determine if scratch columns should be considered at all
		# by checking if any of the MSs has them.
		
		considerscrcols = False
		needscrcols = []
                if ((type(theconcatvis)==str) & (os.path.exists(theconcatvis))):
			
			# check if all scratch columns are present
			t.open(theconcatvis)
			if(t.colnames().count('CORRECTED_DATA')==1 
			   or  t.colnames().count('MODEL_DATA')==1):
				considerscrcols = True  # there are scratch columns
				
			needscrcols.append(t.colnames().count('CORRECTED_DATA')==0 
					   or  t.colnames().count('MODEL_DATA')==0)
			t.close()
                else:
                        raise Exception, 'Visibility data set '+theconcatvis+' not found - please verify the name'

		for elvis in vis : 			###Oh no Elvis does not exist Mr Bill
			if(not os.path.exists(elvis)):
				raise Exception, 'Visibility data set '+elvis+' not found - please verify the name'

			# check if all scratch columns are present
			t.open(elvis)
			if(t.colnames().count('CORRECTED_DATA')==1 
                           or  t.colnames().count('MODEL_DATA')==1):
				considerscrcols = True  # there are scratch columns

			needscrcols.append(t.colnames().count('CORRECTED_DATA')==0 
					  or  t.colnames().count('MODEL_DATA')==0)
			t.close()

		# start actual work, file existence has already been checked
		i = 0
		if(considerscrcols and needscrcols[i]):
			# create scratch cols			
			casalog.post('creating scratch columns in '+theconcatvis , 'INFO')
			cb.open(theconcatvis) # calibrator-open creates scratch columns
			cb.close()

		# scale the weights of the first MS in the chain
		if doweightscale:
			wscale = visweightscale[i]
			if(wscale==1.):
				casalog.post('Will leave the weights for this MS unchanged.', 'INFO')
			else:
				casalog.post('Scaling weights for first MS by factor '+str(wscale), 'INFO')
				t.open(theconcatvis, nomodify=False)
				for colname in [ 'WEIGHT', 'WEIGHT_SPECTRUM']:
					if (colname in t.colnames()) and (t.iscelldefined(colname,0)):
						for j in xrange(0,t.nrows()):
							a = t.getcell(colname, j)
							a *= wscale
							t.putcell(colname, j, a)
				t.close()

		# determine handling switch value
		handlingswitch = 0
		if not copypointing:
			handlingswitch = 2

		m.open(theconcatvis,nomodify=False)
		mmsmembers = [theconcatvis]
	
		for elvis in vis : 
			i = i + 1
			destms = ""
			if createmms:
				destms = concatvis+'.data/'+elvis
				mmsmembers.append(destms)
				casalog.post('adding '+elvis+' to multi-MS '+concatvis+' creating '+destms, 'INFO')
			else:
				casalog.post('concatenating '+elvis+' into '+theconcatvis , 'INFO')

			wscale = 1.
			if doweightscale:
				wscale = visweightscale[i]
				if(wscale==1.):
					casalog.post('Will leave the weights for this MS unchanged.', 'INFO')
				else:
					casalog.post('Will scale weights for this MS by factor '+str(wscale) , 'INFO')

			if(considerscrcols and needscrcols[i]):
				# create scratch cols			
				casalog.post('creating scratch columns for '+elvis+' (original MS unchanged)', 'INFO')
				tempname = elvis+'_with_scrcols'
				os.system('rm -rf '+tempname)
				os.system('cp -R '+elvis+' '+tempname)
				cb.open(tempname) # calibrator-open creates scratch columns
				cb.close()
				# concatenate copy instead of original file
				m.concatenate(msfile=tempname,freqtol=freqtol,dirtol=dirtol,weightscale=wscale,handling=handlingswitch,
					      destmsfile=destms)
				os.system('rm -rf '+tempname)
			else:
				m.concatenate(msfile=elvis,freqtol=freqtol,dirtol=dirtol,weightscale=wscale,handling=handlingswitch,
					      destmsfile=destms)

			m.writehistory(message='taskname=concat',origin='concat')
			m.writehistory(message='vis          = "'+str(concatvis)+'"',origin='concat')
			m.writehistory(message='concatvis    = "'+str(elvis)+'"',origin='concat')
			m.writehistory(message='freqtol      = "'+str(freqtol)+'"',origin='concat')
			m.writehistory(message='dirtol       = "'+str(dirtol)+'"',origin='concat')
			m.writehistory(message='copypointing = "'+str(copypointing)+'"',origin='concat')
			m.writehistory(message='visweightscale = "'+str(visweightscale)+'"',origin='concat')
			m.writehistory(message='createmms    = "'+str(createmms)+'"',origin='concat')

		if timesort:
			casalog.post('Sorting main table by TIME ...', 'INFO')
			m.timesort()

		if createmms:
			m.createmultims(concatvis, mmsmembers,
					[],
					True, # nomodify
					False,# lock
					True) # copysubtables from first to all other members

		m.close()


	except Exception, instance:
		print '*** Error ***',instance
		raise Exception, instance

