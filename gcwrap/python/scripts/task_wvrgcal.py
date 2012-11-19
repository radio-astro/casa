import os
import numpy
from taskinit import *

def wvrgcal(vis=None, caltable=None, toffset=None, segsource=None,
	    sourceflag=None, tie=None, nsol=None, disperse=None, 
	    wvrflag=None, statfield=None, statsource=None, smooth=None,
	    scale=None, reversespw=None,  cont=None):
	"""
	Generate a gain table based on Water Vapour Radiometer data

        															
	  vis -- Name of input visibility file											
	              default: none; example: vis='ngc5921.ms'									
																
	  caltable -- Name of output gain calibration table									
	              default: none; example: caltable='ngc5921.wvr'								
																
	  toffset -- Time offset (sec) between interferometric and WVR data							
	             default: -1 (ALMA default)											
																
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
        """
	#Python script

	# make ms tool local 
	mst = casac.ms()

	try:
		casalog.origin('wvrgcal')

		if not (type(vis)==str) or not (os.path.exists(vis)):
			raise Exception, 'Visibility data set not found - please verify the name'
		
		if (caltable == ""):
			raise Exception, "Must provide output caltable name in parameter gaintable."            
		
		if os.path.exists(caltable):
			raise Exception, "Output gaintable %s already exists - will not overwrite." % caltable

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
		for ll in loglines:
			casalog.post(ll.expandtabs())
		os.system('rm -rf '+templogfile)

		if(rval == 0):
			if (smoothing>0):
				mycb = cbtool()
				mycb.open(filename=vis, compress=False, addcorr=False, addmodel=False)
				mycb.smooth(tablein=caltable+'_unsmoothed', tableout=caltable,
					    smoothtype='mean', smoothtime=smoothing)
				mycb.close()
			return True
		else:
			if(rval < 32512):
				casalog.post(theexecutable+' terminated with exit code '+str(rval),'WARN')
				return False
			else:
				raise Exception, "wvrgcal executable not available."
	
	except Exception, instance:
		print '*** Error *** ',instance
		raise Exception, instance
