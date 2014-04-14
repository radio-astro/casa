import os
import numpy
from taskinit import *

def wvrgcal(vis=None, caltable=None, toffset=None, segsource=None,
	    sourceflag=None, tie=None, nsol=None, disperse=None, 
	    wvrflag=None, statfield=None, statsource=None, smooth=None,
	    scale=None, reversespw=None,  cont=None, maxdistm=None,
	    minnumants=None, mingoodfrac=None, usefieldtab=None):
	"""
	Generate a gain table based on Water Vapour Radiometer data.
	Returns a dictionary containing the RMS of the path length variation
	with time towards that antenna (RMS) and the discrepency between the RMS
	path length calculated separately for different WVR channels (Disc.).

        															
	  vis -- Name of input visibility file											
	              default: none; example: vis='ngc5921.ms'									
																
	  caltable -- Name of output gain calibration table									
	              default: none; example: caltable='ngc5921.wvr'								
																
	  toffset -- Time offset (sec) between interferometric and WVR data							
	             default: 0 (ALMA default for cycle 1, for cycle 0 it was -1)											
																
	  segsource -- Do a new coefficient calculation for each source								
	             default: True												
																
	  tie -- Prioritise tieing the phase of these sources as well as possible						
	         (requires segsource=True)											
	             default: [] example: ['3C273,NGC253', 'IC433,3C279']							
																
	  sourceflag -- Flag the WVR data for these source(s) as bad and do not produce corrections for it			
	               (requires segsource=True)										
	               default: [] (none) example: ['3C273']									
																
	  nsol -- Number of solutions for phase correction coefficients during this observation.				
	          By default only one set of coefficients is generated for the entire observation. 				
	          If more sets are requested, then they will be evenly distributed in time throughout 				
	          the observation. Values > 1 require segsource=False.								
	             default: 1													
																
	  disperse -- Apply correction for dispersion										
	             default: False												
																
	  wvrflag -- Flag the WVR data for these antenna(s) as bad and replace its data with interpolated values		
	               default: [] (none) example: ['DV03','DA05','PM02']           						
																
	  statfield -- Compute the statistics (Phase RMS, Disc) on this field only						
	               default: '' (all) 											
																
	  statsource -- Compute the statistics (Phase RMS, Disc) on this source only						
	               default: '' (all)             										
																
	  smooth -- Smooth the calibration solution on the given timescale 
	             default: '' (no smoothing), example: '3s' smooth on a timescale of 3 seconds
		     
	  scale -- Scale the entire phase correction by this factor								
	             default: 1. (no scaling)											
																
	  reversespw -- Reverse the sign of the correction for the listed SPWs							
	                (only neede for early ALMA data before Cycle 0)								
	             default: '' (none), example: reversespw='0~2,4'; spectral windows 0,1,2,4					
																
	  cont -- Estimate the continuum (e.g., due to clouds)									
                     default: False

          maxdistm -- maximum distance (m) an antenna may have to be considered for being part
	              of the <=3 antenna set for interpolation of a solution for a flagged antenna
		      default: 500

          minnumants -- minimum number of near antennas required for interpolation
	                default: 2

          mingoodfrac -- If the fraction of unflagged data for an antenna is below this value (0. to 1.),
	                 the antenna is flagged.
			 default: 0.8

          usefieldtab -- derive the antenna AZ/EL values from the FIELD rather than the POINTING table
	                 default: False

        """
	#Python script

	# make ms tool local 
	mst = casac.ms()

	try:
		casalog.origin('wvrgcal')

		if not (type(vis)==str) or not (os.path.exists(vis)):
			raise Exception, 'Visibility data set not found - please verify the name'
		
		if (caltable == ""):
			raise Exception, "Must provide output calibration table name in parameter caltable."            
		
		if os.path.exists(caltable):
			raise Exception, "Output caltable %s already exists - will not overwrite." % caltable

		execute_string=  '--ms ' + vis

		smoothing = -1
		if (type(smooth)==str and smooth!=''):
			smoothing = qa.convert(qa.quantity(smooth), 's')['value']
			execute_string+= ' --output ' + caltable + '_unsmoothed'
		else:
			execute_string+= ' --output ' + caltable

		
		execute_string+= ' --toffset ' + str(toffset)

		if nsol>1:
			if not segsource:
				execute_string+= ' --nsol ' + str(nsol)
			else:
				raise Exception, "In order to use nsol>1, segsource must be set to False." % caltable
		if segsource:
			execute_string+= ' --segsource'

		if segsource and (len(sourceflag)>0):
			for src in sourceflag:
				if not (type(src)==int or type(src)==str):
					raise Exception, "List elements of parameter sourceflag must be int or string."
				if (src != ''):
					execute_string += ' --sourceflag \"'+str(src)+'\"'

		if segsource and (len(tie)>0):
			for i in xrange(0,len(tie)):
				src = tie[i]
				if not (type(src)==str):
					raise Exception, "List elements of parameter tie must be strings."
				if (src != ''):
					execute_string += ' --tie '
					execute_string += '\"'+str(src)+'\"'
					if not (i==len(tie)-1):
						execute_string += ' '

		if (not reversespw==''):
			spws = mst.msseltoindex(vis=vis,spw=reversespw)['spw']
			for id in spws:
				execute_string += ' --reversespw '+str(id)

		if disperse:
			dispdirpath = os.getenv('WVRGCAL_DISPDIR', '')
			if not os.path.exists(dispdirpath+'/libair-ddefault.csv'):
				path1 = dispdirpath
				dispdirpath = os.getenv("CASAPATH").split(' ')[0] + "/data/alma/wvrgcal"
				if not os.path.exists(dispdirpath+'/libair-ddefault.csv'):
					raise Exception, "Dispersion table libair-ddefault.csv not found in path "\
					      +"given by WVRGCAL_DISPDIR nor in \""+dispdirpath+"\""
				
				os.putenv('WVRGCAL_DISPDIR', dispdirpath)
				
			execute_string+= ' --disperse'
			casalog.post('Using dispersion table '+dispdirpath+'/libair-ddefault.csv')

		if cont:
			if not segsource:
				execute_string+= ' --cont'
			else:
				raise Exception, "cont and segsource are not permitted to be True at the same time."

		if usefieldtab:
			execute_string+= ' --usefieldtab'
				
		if (len(wvrflag)>0):
			for ant in wvrflag:
				if not (type(ant)==int or type(ant)==str):
					raise Exception, "List elements of parameter wvrflag must be int or string."
				if (ant != ''):
					execute_string += ' --wvrflag \"'+str(ant)+'\"'

		if not (statfield==None or statfield=="") and type(statfield)==str:
			execute_string += ' --statfield \"'+ statfield + '\"'

		if not (statsource==None or statsource=="") and type(statsource)==str:
			execute_string += ' --statsource \"'+ statsource + '\"'

		if (scale != 1.):
			execute_string+= ' --scale ' + str(scale)
		
		if (maxdistm>=0.):
			execute_string+= ' --maxdistm ' + str(maxdistm)
		
		if (minnumants>=0):
			execute_string+= ' --minnumants ' + str(minnumants)
		
		if (0.<=mingoodfrac and mingoodfrac<=1.):
			execute_string+= ' --mingoodfrac ' + str(mingoodfrac)

		theexecutable = 'wvrgcal'

		execute_string = theexecutable+' '+execute_string

		casalog.post('Running '+theexecutable+' standalone invoked as:')
		casalog.post(execute_string)
		print execute_string

		templogfile = 'wvrgcal_tmp_'+str(numpy.random.randint(1E6,1E8))

        	rval = os.system(execute_string + " > "+ templogfile)

		fp = file(templogfile)
		loglines = fp.readlines()
		fp.close()

		# prepare variables for parsing log lines to extract info table
		hfound = False
		hend = False
		namel = []
		wvrl = []
		flagl = []
		rmsl = []
		discl = []
		parsingok = True
		
		for ll in loglines:
			casalog.post(ll.expandtabs())
			if hfound:
				if "Expected performance" in ll:
					hend = True
				elif not hend:
					vals = ll.split()
					wvrv = False
					flagv = False
					rmsv = 0.
					discv = 0.
					if(len(vals)!=6):
						casalog.post('Error parsing wvrgcal info table.line: '+ll,'WARN')
						parsingok=False
					else:
						if vals[2]=='Yes':
							wvrv=True
						else:
							wvrv=False
						if vals[3]=='Yes':
							flagv=True
						else:
							flagv=False
					try:
						rmsv = float(vals[4])
					except:
						casalog.post('Error parsing RMS value in info table line: '+ll,'WARN')
						rmsv = -1.
						parsingok=False
					try:
						discv = float(vals[5])
					except:
						casalog.post('Error parsing Disc. value in info table line: '+ll,'WARN')
						discv = -1.
						parsingok=False

					namel.append(vals[1])
					wvrl.append(wvrv)
					flagl.append(flagv)
					rmsl.append(rmsv)
					discl.append(discv)

					
			elif (rval==0) and (not hend) and ("Disc (um)" in ll):
				hfound = True

		# end for ll

		os.system('rm -rf '+templogfile)


		taskrval = { 'Name': namel,
			     'WVR': wvrl,
			     'Flag': flagl,
			     'RMS_um': rmsl,
			     'Disc_um': discl,
			     'rval': rval,
			     'success': False}

		for k in range(len(namel)):
			if(flagl[k] and rmsl[k]==0. and discl[k]==0.):
				casalog.post('Solution for flagged antenna '+namel[k]
					     +' could not be interpolated due to insufficient number of near antennas. Was set to unity.',
					     'WARN')

		if (rval==0) and parsingok:
			taskrval['success'] = True
		

		if(rval == 0):
			if (smoothing>0):
				mycb = cbtool()
				mycb.open(filename=vis, compress=False, addcorr=False, addmodel=False)
				mycb.smooth(tablein=caltable+'_unsmoothed', tableout=caltable,
					    smoothtype='mean', smoothtime=smoothing)
				mycb.close()
			return taskrval
		else:
			if(rval == 32512):
				raise Exception, "wvrgcal executable not available."
			elif(rval == 65280):
				casalog.post(theexecutable+' terminated with exit code '+str(rval),'SEVERE')
				return taskrval
			elif(rval == 34304):
				casalog.post(theexecutable+' terminated with exit code '+str(rval),'WARN')
				casalog.post("No useful input data.",'SEVERE')
				return taskrval
			else:
				casalog.post(theexecutable+' terminated with exit code '+str(rval),'WARN')
				return taskrval
	
	except Exception, instance:
		print '*** Error *** ',instance
		raise Exception, instance
