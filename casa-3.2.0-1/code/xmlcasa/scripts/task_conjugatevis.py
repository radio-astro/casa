import os
from taskinit import *

def conjugatevis(vis,spwlist=[],outputvis="",overwrite=False):
	""":
	Change the sign of the phases in all visibility columns

	Keyword arguments:
	vis -- Name of input visibility file
		default: none; example='3C273XC1.ms'
	spwlist -- Select spectral window
		default: [] all spws will be conjugated; example: spw=[1,2]
	outputvis -- name of output visibility file
	        default: 'conjugated_'+vis; example= 'conjugated.ms'
	overwrite -- Overwrite the outputvis if it exists
		default=False; example: overwrite=True

	"""

	#Python script
	try:
		casalog.origin('conjugatevis')
		myddlist = []
		tb.open(vis+'/SPECTRAL_WINDOW')
		maxspw = tb.nrows()-1
		tb.close()
		if (type(spwlist)==type(1)):
			spwlist = [spwlist]
		elif(spwlist==None or spwlist==[] or spwlist==""):
			spwlist = []
			casalog.post("Will conjugate visibilities for all spws.", 'INFO')
		if not spwlist==[]:
			try:
				tb.open(vis+'/DATA_DESCRIPTION')
				for k in spwlist:
					if (k<-1 or k>maxspw):
						raise Exception, "Error: max valid spw id is "+str(maxspw)
						raise
					else:
						for j in range(0,tb.nrows()):
							if(tb.getcell("SPECTRAL_WINDOW_ID",j)==k and not (j in myddlist)):
								myddlist = myddlist + [j]
			        #end for k
				tb.close()
				casalog.post('DATA_DESC_IDs to process: '+str(myddlist), 'INFO')
			except:
				raise Exception, 'Error reading DATA_DESCRIPTION table'
		#endif
		outname = 'conjugatedvis_'+vis
		if not outputvis=="":
			if((type(outputvis)!=str) or (len(outputvis.split()) < 1)):
				raise Exception, 'parameter outputvis is invalid'
			outname = outputvis
		if not overwrite and os.path.exists(outname):
			raise Exception, 'outputvis '+outname+' exists and you did not permit overwrite'
		os.system('rm -rf '+outname)
		os.system('cp -R '+vis+' '+outname)
		tb.open(outname, nomodify=False)
		if tb.iswritable():
			if(spwlist==None):
				for colname in [ 'DATA', 'CORRECTED_DATA', 'FLOAT_DATA' ]:
					if colname in tb.colnames():
						casalog.post('Conjugating '+str(colname), 'INFO')
						for i in xrange(0,tb.nrows()):
							a = tb.getcell(colname, i)
							a = a.conjugate()
							tb.putcell(colname, i, a)
			else:
				for colname in [ 'DATA', 'CORRECTED_DATA', 'FLOAT_DATA' ]:
					if colname in tb.colnames():
						casalog.post('Conjugating '+str(colname), 'INFO')
						for i in xrange(0,tb.nrows()):
							if(tb.getcell("DATA_DESC_ID",i) in myddlist):
								a = tb.getcell(colname, i)
								a = a.conjugate()
								tb.putcell(colname, i, a)
			#endif
			tb.flush()
			tb.close()
			casalog.post('Created '+str(outname), 'INFO')
		else:
			tb.close()
			casalog.post('Cannot write to output MS '+str(outname), 'WARN')

	except Exception, instance:
		tb.close()
		print '*** Error ***', instance
		raise Exception, instance


