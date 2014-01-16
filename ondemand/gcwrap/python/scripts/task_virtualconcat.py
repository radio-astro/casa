import os
import shutil
import stat
import time
from taskinit import *
import partitionhelper as ph
from parallel.parallel_task_helper import ParallelTaskHelper

def virtualconcat(vislist,concatvis,freqtol,dirtol,respectname,
		  visweightscale,keepcopy,copypointing):
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
	respectname -- If true, fields with a different name are not merged even if their 
                direction agrees (within dirtol)
                default: True
	visweightscale -- list of the weight scales to be applied to the individual MSs
	        default: [] (don't modify weights, equivalent to setting scale to 1 for each MS)
        keepcopy -- If true, a copy of the input MSs is kept in their original place.
                default: false
	copypointing --  If true, the POINTING table information will be present in the output.
                If false, the result is an empty POINTING table.
                default: True

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
			raise Exception, 'Parameter concatvis is invalid.'

		if(vis.count(concatvis) > 0):
			raise Exception, 'Parameter concatvis must not be equal to one of the members of parameter vis.'

		if(os.path.exists(concatvis)):
			raise Exception, 'The output MMS must not yet exist.'

		# process the input MSs in chronological order
		sortedvis = []
		sortedvisweightscale = []
		namestuples = []
		for name in vis:
			t.open(name)
			times = t.getcol('TIME')
			t.close()
			times.sort()
			if doweightscale:
				namestuples.append( (times[0], name, visweightscale[vis.index(name)]) )
			else:
				namestuples.append( (times[0], name, 0) )

		sorted_namestuples = sorted(namestuples, key=lambda msname: msname[0]) 
    
		for i in range(0,len(vis)):
			sortedvis.append(sorted_namestuples[i][1])
			sortedvisweightscale.append(sorted_namestuples[i][2])

		if not vis == sortedvis:
			casalog.post('The list of input MSs is not in chronological order and will need to be sorted.' , 'INFO')
			casalog.post('The chronological order in which the concatenation will take place is:' , 'INFO')
			for name in sortedvis:
				casalog.post('   MJD '+str(qa.splitdate(qa.quantity(sorted_namestuples[sortedvis.index(name)][0],'s'))['mjd'])+': '+name, 'INFO')
			if doweightscale:
				casalog.post('In this new order, the weights are:'+str(sortedvisweightscale) , 'INFO')

		# replace the original vis and visweightscale by the sorted ones (with concatvis removed if it exists)
		vis = sortedvis
		visweightscale = sortedvisweightscale

		# if there are MMSs among the input, make their constituents the new input
		mmslist = []
		ismaster = []
		for elvis in vis:
			ismaster.append(True) # may be revised later
			if(ParallelTaskHelper.isParallelMS(elvis)):
				mmslist.append(elvis)
		if len(mmslist)>0:
			casalog.post('*** The following input measurement sets are multi-MSs', 'INFO')
			for mname in mmslist:
				casalog.post('***   '+mname, 'INFO')
			oldvis = vis
			oldvisweightscale = visweightscale
			vis = []
			visweightscale = []
			ismaster = [] # reset ismaster
			i = 0
			for elvis in oldvis:
				if elvis in mmslist: # append the subMSs individually
					m.open(elvis)
					mses = m.getreferencedtables()
					m.close()
					mses.sort()
					mastername = os.path.basename(os.path.dirname(os.path.realpath(elvis+'/ANTENNA')))
					for mname in mses:
						#print 'subms: ', mname
						vis.append(mname)
						if doweightscale:
							visweightscale.append(oldvisweightscale[i])
						if os.path.basename(mname) == mastername:
							ismaster.append(True)
						else:
							ismaster.append(False)
				else:
					vis.append(elvis)
					if doweightscale:
						visweightscale.append(oldvisweightscale[i])
					ismaster.append(True)
				i += 1



		if keepcopy:
			casalog.post('*** keepcopy==True: creating copy of input MSs to keep ...' , 'INFO')
			tempdir = 'concat_tmp_'+str(time.time())
			os.mkdir(tempdir)
			for elvis in originalvis:
				shutil.move(elvis,tempdir) # keep timestamps and permissions
				shutil.copytree(tempdir+'/'+elvis, elvis, True) # symlinks=True

		casalog.post('Concatenating ...' , 'INFO')

		if not copypointing: # delete the rows of all pointing tables
			casalog.post('*** copypointing==False: resulting MMS will have empty POINTING table.', 'INFO')
			tmptabname = 'TMPPOINTING'+str(time.time())
			tmptabname2 = 'TMPPOINTING2'+str(time.time())
			shutil.rmtree(tmptabname, ignore_errors=True)
			shutil.rmtree(tmptabname2, ignore_errors=True)
			shutil.move(vis[0]+'/POINTING', tmptabname)
			t.open(tmptabname)
			if(t.nrows()>0): 
				ttab = t.copy(newtablename=tmptabname2, deep=False, valuecopy=True, norows=True)
				ttab.close()
				t.close()
				shutil.rmtree(tmptabname, ignore_errors=True)
			else: # the POINTING table is already empty
				t.close()
				casalog.post('***    Input POINTING table was already empty.', 'INFO')
				shutil.move(tmptabname, tmptabname2)
				
			for i in range(len(vis)): # replace the POINTING tables by the empty one
				os.system('rm -rf '+vis[i]+'/POINTING')
				shutil.copytree(tmptabname2, vis[i]+'/POINTING')
			shutil.rmtree(tmptabname2, ignore_errors=True)
					
		if(len(vis) >0): # (note: in case len is 1, we only copy, essentially)
			theconcatvis = vis[0]
			if(len(vis)==1):
				shutil.copytree(vis[0], concatvis, True)
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
					  freqtol=freqtol,dirtol=dirtol,respectname=respectname,
					  weightscale=wscale)
		#end for
		os.remove(auxfile)

		m.writehistory(message='taskname=virtualconcat',origin='virtualconcat')
		m.writehistory(message='vis          = "'+str(vis)+'"',origin='virtualconcat')
		m.writehistory(message='concatvis    = "'+str(concatvis)+'"',origin='virtualconcat')
		m.writehistory(message='freqtol      = "'+str(freqtol)+'"',origin='virtualconcat')
		m.writehistory(message='dirtol       = "'+str(dirtol)+'"',origin='virtualconcat')
		m.writehistory(message='respectname  = "'+str(respectname)+'"',origin='virtualconcat')
		m.writehistory(message='visweightscale = "'+str(visweightscale)+'"',origin='virtualconcat')

		m.close()

		# concatenate the POINTING tables
		masterptable = mmsmembers[0]+'/POINTING'
		ptablemembers = []
		if os.path.exists(masterptable) and copypointing:
			casalog.post('Concatenating the POINTING tables ...', 'INFO')
			i = 0
			for i in xrange(len(mmsmembers)):
				ptable = mmsmembers[i]+'/POINTING'
				if ismaster[i] and os.path.exists(ptable):
					casalog.post('   '+ptable, 'INFO')
					shutil.move(ptable, ptable+str(i))
					ptablemembers.append(ptable+str(i))
			#end for
			t.createmultitable(masterptable, ptablemembers, 'SUBTBS')
		# endif

	 	ph.makeMMS(concatvis, mmsmembers,
 			   True, # copy subtables from first to all other members 
 			   ['POINTING']) # excluding POINTING which will be linked

		# remove the remaining "hulls" of the emptied input MMSs (if there are any)
		for elvis in mmslist:
			shutil.rmtree(elvis)
		
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

