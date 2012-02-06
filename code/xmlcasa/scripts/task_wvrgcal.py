import os
from taskinit import *

def wvrgcal(vis=None, caltable=None, toffset=None, nsol=None, segsource=False,
	    reversespw=None, disperse=False, cont=False, wvrflag=[], sourceflag=[],
	    statfield=None, statsource=None, tie=[], smooth=None, scale=None):
	"""
	Generate a gain table based on Water Vapour Radiometer data

        vis -- Name of input visibility file
              default: none; example: vis='ngc5921.ms'

        caltable -- Name of output gain calibration table
              default: none; example: caltable='ngc5921.wvr'

        toffset -- Time offset (sec) between interferometric and WVR data
             default: 0

        nsol -- Number of solutions for phase correction coefficients during this observation
             default: 1

        segsource -- Do a new coefficient calculation for each source
             default: True

        reversespw -- Reverse the sign of the correction for the listed SPWs
             default: '' (none), example: reversespw='0~2,4'; spectral windows 0,1,2,4

        disperse -- Apply correction for dispersion
             default: False

        cont -- Estimate the continuum (e.g., due to clouds)
             default: False

        wvrflag -- Flag the WVR data for these antenna(s) as bad and replace its data with interpolated values
               default: [] (none) example: ['DV03','DA05','PM02']           

        sourceflag -- Flag the WVR data for these source(s) as bad and do not produce corrections for it
               default: [] (none) example: ['3C273']

        statfield -- Compute the statistics (Phase RMS, Disc) on this field only
               default: '' (all) 

        statsource -- Compute the statistics (Phase RMS, Disc) on this source only
               default: '' (all)             

        tie -- Prioritise tieing the phase of these sources as well as possible
                default: [] example: ['3C273']

        smooth -- Smooth WVR data by this many samples before applying the correction
             default: 1 (no smoothing) example: 3

        scale -- Scale the entire phase correction by this factor
             default: 1. (no scaling)

	"""
	#Python script

	# make ms and tb tool local 
	ms = casac.homefinder.find_home_by_name('msHome').create()
	tb = casac.homefinder.find_home_by_name('tableHome').create()

	try:
		casalog.origin('wvrgcal')
		parsummary = 'vis=\"'+str(vis)+'\", caltable=\"'+str(caltable)+'\", toffset='+str(toffset)+','
		casalog.post(parsummary)
		parsummary = 'nsol='+str(nsol)+', segsource='+str(segsource)+', reversespw=\"'+str(reversespw)+'\",'
		casalog.post(parsummary)
		parsummary = 'disperse='+str(disperse)+', cont='+str(cont)+', wvrflag=\"'+str(wvrflag)+'\",'
		casalog.post(parsummary)
		parsummary = 'sourceflag=\"'+str(sourceflag)+'\", statfield='+str(statfield)+', statsource=\"'+str(statsource)+'\",'
		casalog.post(parsummary)
		parsummary = 'tie=\"'+str(tie)+'\", smooth='+str(smooth)+', scale='+str(scale)
		casalog.post(parsummary)

		if not (type(vis)==str) or not (os.path.exists(vis)):
			raise Exception, 'Visibility data set not found - please verify the name'
		
		if (caltable == ""):
			raise Exception, "Must provide output caltable name in parameter gaintable."            
		
		if os.path.exists(caltable):
			raise Exception, "Output gaintable %s already exists - will not overwrite." % caltable

		execute_string=  '--ms ' + vis 
		execute_string+= ' --output ' + caltable + ' --toffset ' + str(toffset)
		execute_string+= ' --nsol ' + str(nsol)

		if segsource:
			execute_string+= ' --segsource'

		if (not reversespw==''):
			spws = ms.msseltoindex(vis=vis,spw=reversespw)['spw']
			for id in spws:
				execute_string += ' --reversespw '+str(id)

		if disperse:
			execute_string+= ' --disperse'

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
					execute_string += ' --wvrflag '+str(ant)

		if (len(sourceflag)>0):
			for src in sourceflag:
				if not (type(src)==int or type(src)==str):
					raise Exception, "List elements of parameter sourceflag must be int or string."
				if (src != ''):
					execute_string += ' --sourceflag '+str(src)

		if not (statfield==None or statfield=="") and type(statfield)==str:
			execute_string += ' --statfield '+ statfield

		if not (statsource==None or statsource=="") and type(statsource)==str:
			execute_string += ' --statsource '+ statsource

		if (len(tie)>0):
			execute_string += ' --tie '
			for i in xrange(0,len(tie)):
				src = tie[i]
				if not (type(src)==int or type(src)==str):
					raise Exception, "List elements of parameter tie must be int or string."
				if (src != ''):
					execute_string += '\''+str(src)+'\''
					if not (i==len(tie)-1):
						execute_string += ','

		if (smooth > 1):
			execute_string+= ' --smooth ' + str(smooth)

		if (scale != 1.):
			execute_string+= ' --scale ' + str(scale)
		
		theexecutable = 'wvrgcal'

		execute_string = theexecutable+' '+execute_string

		casalog.post('Running '+theexecutable+' standalone invoked as:')
		casalog.post(execute_string)
		print execute_string

        	rval = os.system(execute_string)

		if(rval == 0):
			return True
		else:
			if(rval < 32512):
				casalog.post(theexecutable+' terminated with exit code '+str(rval),'WARN')
				return False
			else:
				raise Exception, "wvrgcal executable not available."
	
	except Exception, instance:
		casalog.post("Error ...", 'SEVERE')
		raise Exception, instance
