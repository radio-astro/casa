import os
from taskinit import *

def exportasdm(vis=None, asdm=None, datacolumn=None, archiveid=None, rangeid=None,
	       subscanduration=None, apcorrected=None, verbose=None, showversion=None):
	""" Convert a CASA visibility file (MS) into an ALMA Science Data Model.
                                          
	Keyword arguments:
	vis       -- MS name,
             default: none

	asdm -- Name of output ASDM file (directory),
	     default: none; example: asdm='ExecBlock3'

	datacolumn -- specifies which of the MS data columns (DATA,
	          CORRECTED_DATA, or MODEL_DATA) should be used as the
                  visibilities in the ASDM, default: DATA

	archiveid -- the X0 in uid://X0/X1/X<running>
                  default: "S0"

	rangeid -- the X1 in uid://X0/X1/X<running>
                  default: "X1"

	subscanduration -- maximum duration of a subscan in the output ASDM
	          default: "24h"

	apcorrected -- If true, the data in column datacolumn should be regarded
	          as having atmospheric phase correction, default: True

	verbose     -- produce log output, default: True

	showversion -- report the version of the ASDM class set, 
                 default: True
	"""
	#Python script
	try:
		casalog.origin('exportasdm')
		parsummary = 'vis=\"'+str(vis)+'\", asdm=\"'+str(asdm)+'\", datacolumn=\"'+str(datacolumn)+'\",'
		casalog.post(parsummary)
		parsummary = 'archiveid=\"'+str(archiveid)+'\", rangeid=\"'+str(rangeid)+'\", subscanduration=\"'+str(subscanduration)+'\", apcorrected='+str(apcorrected)+'\",'
		casalog.post(parsummary)
		parsummary = 'verbose='+str(verbose)+', showversion='+str(showversion)
		casalog.post(parsummary)

		if not (type(vis)==str) or not (os.path.exists(vis)):
			raise Exception, 'Visibility data set not found - please verify the name'
		
		if (asdm == ""):
			raise Exception, "Must provide output data set name in parameter asdm."            
		
		if os.path.exists(asdm):
			raise Exception, "Output ASDM %s already exists - will not overwrite." % asdm

		# determine parameters datacolumn
		tb.open(vis)
		allcols = tb.colnames()
		tb.close()
		if not (datacolumn in allcols):
			raise Exception, "Input MS does not contain datacolumn %s" % datacolumn

		ssdur_secs = 24.*3600 # default is one day, i.e. there will be only one subscan per scan
		if not(subscanduration==""):
			if (qa.canonical(subscanduration)['unit'].find('s') < 0):
				raise TypeError, "subscanduration is not a valid time quantity: %s" % subscanduration
			else:
				ssdur_secs = qa.canonical(subscanduration)['value']

		execute_string='--datacolumn \"' + datacolumn + '\" --archiveid \"' + archiveid + '\" --rangeid \"' + rangeid
		execute_string+= "\" --subscanduration " + str(ssdur_secs) + ' --logfile \"' + casalog.logfile() +'\"'
		
		if(not apcorrected):
			execute_string= execute_string +' --apuncorrected'
		if(verbose):
			execute_string= execute_string +' --verbose'
		if(showversion):
			execute_string= execute_string +' --revision'

		execute_string = execute_string + ' ' + vis + ' ' + asdm

		execute_string = 'MS2asdm '+execute_string

		casalog.post('Running the MS2asdm standalone invoked as:')
		print execute_string
		casalog.post(execute_string)
        	rval = os.system(execute_string)

		if(rval == 0):
			return True
		else:
			return False
	
	except Exception, instance:
		casalog.post("Error ...", 'SEVERE')
		raise Exception, instance
