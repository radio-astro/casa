import os
import shutil
import stat
import time
from taskinit import *
import partitionhelper as ph

def virtualconcat(vislist,concatvis,freqtol,dirtol,visweightscale,keepcopy):
	"""
	Concatenate visibility data sets creating a multi-MS.
	
	Combine the input datasets into a multi-MS.
	NOTE: The input datasets are moved into the multi-MS and may be modified
	to account for subtable reference changes.
	If none of the input MSs have any scratch columns, none are created.
	Otherwise scratch columns are created and initialized in those MSs
	which don't have a complete set.


	Keyword arguments:
	vis -- Name of input visibility files (MS)
		default: none; example: vis=['ngc5921-1.ms', 'ngc5921-2.ms']
	concatvis -- Name of the output visibility file
		default: none; example: concatvis='src2.ms'
	freqtol -- Frequency shift tolerance for considering data as the same spwid
		default: ''  means always combine
		example: freqtol='10MHz' will not combine spwid unless they are
		within 10 MHz
	dirtol -- Direction shift tolerance for considering data as the same field
		default: ;; means always combine
		example: dirtol='1.arcsec' will not combine data for a field unless
		their phase center is less than 1 arcsec.
	visweightscale -- list of the weight scales to be applied to the individual MSs
	        default: [] (don't modify weights, equivalent to setting scale to 1 for each MS)
        keepcopy -- If true, a copy of the input MSs is kept in their original place.
                default: false

	"""

        ###
	#Python script

	tempdir = ''
	originalvis = vislist
	try:
		casalog.origin('virtualconcat')
		t = tbtool()
		m = mstool()
		
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

		if((type(concatvis)!=str) or (len(concatvis.split()) < 1)):
			raise Exception, 'parameter concatvis is invalid'

		if(vis.count(concatvis) > 0):
			vis.remove(concatvis)

		if(os.path.exists(concatvis)):
			raise Exception, 'The output MMS must not yet exist.'

		if keepcopy:
			tempdir = 'concat_tmp_'+str(time.time())
			os.mkdir(tempdir)
			for elvis in originalvis:
				shutil.move(elvis,tempdir) # keep timestamps and permissions
				shutil.copytree(tempdir+'/'+elvis, elvis)

		if(len(vis) >0): # (note: in case len is 1, we only copy, essentially)
			theconcatvis = vis[0]
			if(len(vis)==1):
				shutil.copytree(vis[0], concatvis)
			vis.remove(vis[0])

		# Determine if scratch columns should be considered at all
		# by checking if any of the MSs has them.
		
		considerscrcols = False
		needscrcols = []
                if ((type(theconcatvis)==str) and (os.path.exists(theconcatvis))):
			
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

		if(considerscrcols and needscrcols[0]):
			# create scratch cols			
			casalog.post('creating scratch columns in '+theconcatvis , 'INFO')
			cb.open(theconcatvis) # calibrator-open creates scratch columns
			cb.close()

		# scale the weights of the first MS in the chain
		if doweightscale:
			wscale = visweightscale[0]
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

		m.open(theconcatvis,nomodify=False)
		mmsmembers = [theconcatvis]
	
		auxfile = 'concat_aux_'+str(time.time())

		i = 0
		for elvis in vis : 
			i = i + 1

			mmsmembers.append(elvis)
			casalog.post('adding '+elvis+' to multi-MS '+concatvis, 'INFO')

			wscale = 1.
			if doweightscale:
				wscale = visweightscale[i]
				if(wscale==1.):
					casalog.post('Will leave the weights for this MS unchanged.', 'INFO')
				else:
					casalog.post('Will scale weights for this MS by factor '+str(wscale) , 'INFO')

			if(considerscrcols and needscrcols[i]):
				# create scratch cols			
				casalog.post('creating scratch columns for '+elvis, 'INFO')
				cb.open(elvis) # calibrator-open creates scratch columns
				cb.close()
				
			m.virtconcatenate(msfile=elvis,
					  auxfilename=auxfile,
					  freqtol=freqtol,dirtol=dirtol,weightscale=wscale)
		#end for
		os.remove(auxfile)

		m.writehistory(message='taskname=virtualconcat',origin='virtualconcat')
		m.writehistory(message='vis          = "'+str(vis)+'"',origin='virtualconcat')
		m.writehistory(message='concatvis    = "'+str(concatvis)+'"',origin='virtualconcat')
		m.writehistory(message='freqtol      = "'+str(freqtol)+'"',origin='virtualconcat')
		m.writehistory(message='dirtol       = "'+str(dirtol)+'"',origin='virtualconcat')
		m.writehistory(message='visweightscale = "'+str(visweightscale)+'"',origin='virtualconcat')

		m.close()

		# concatenate the POINTING tables
		masterptable = mmsmembers[0]+'/POINTING'
		ptablemembers = []
		if os.path.exists(masterptable):
			i = 0
			for i in xrange(len(mmsmembers)):
				ptable = mmsmembers[i]+'/POINTING'
				if os.path.exists(ptable):
					shutil.move(ptable, ptable+str(i))
					ptablemembers.append(ptable+str(i))
			#end for
			t.createmultitable(masterptable, ptablemembers, 'SUBTBS')
		# endif

	 	ph.makeMMS(concatvis, mmsmembers,
 			   True, # copy subtables from first to all other members 
 			   ['POINTING']) # excluding POINTING which will be linked
		
		if keepcopy:
			for elvis in originalvis:
				shutil.move(tempdir+'/'+elvis, elvis)
			os.rmdir(tempdir)


	except Exception, instance:
		print '*** Error ***',instance
		if keepcopy and tempdir!='':
			print "Restoring original MSs ..."
			for elvis in originalvis:
				if os.path.exists(tempdir+'/'+elvis):
					shutil.rmtree(elvis)
					shutil.move(tempdir+'/'+elvis, elvis)
			os.rmdir(tempdir)
		raise Exception, instance

