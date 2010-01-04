import os
from taskinit import *

def exportasdm(vis=None, asdm=None, datacolumn=None, archiveid=None, rangeid=None, verbose=None, showversion=None):
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

	verbose     -- produce log output, default: True

	showversion -- report the version of the ASDM class set, 
                 default: True
	"""
	#Python script
	try:
		casalog.origin('exportasdm')
		parsummary = 'vis=\"'+str(vis)+'\", asdm=\"'+str(asdm)+'\", datacolumn=\"'+str(datacolumn)+'\",'
		casalog.post(parsummary)
		parsummary = 'archiveid=\"'+str(archiveid)+'\", rangeid=\"'+str(rangeid)+'\", verbose='+str(verbose)+', showversion='+str(showversion)
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

		execute_string='MS2asdm  --datacolumn \"' + datacolumn + '\" --archiveid \"' + archiveid + '\" --rangeid \"' + rangeid + '\" --logfile \"' + casalog.logfile() +'\"'
		if(verbose):
		   execute_string= execute_string +' --verbose'
		if(showversion):
		   execute_string= execute_string +' --revision'

		execute_string = execute_string + ' ' + vis + ' ' + asdm
		casalog.post('Running the MS2asdm standalone invoked as:')
		#print execute_string
		casalog.post(execute_string)
        	rval = os.system(execute_string)

		if(rval == 0):
			return True
		else:
			return False
	
	except Exception, instance:
		casalog.post("Error ...", 'SEVERE')
		raise Exception, instance
