import os
from taskinit import *

def importasdm(asdm=None, corr_mode=None, srt=None, time_sampling=None, ocorr_mode=None):
	""" Convert an ALMA Science Data Model observation into a CASA visibility file (MS)
	The conversion of the ALMA SDM archive format into a measurement set.  This version
	is under development and is geared to handling many spectral windows of different
	shapes.
	
	Keyword arguments:
	asdm -- Name of input ASDM file (directory)
		default: none; example: asdm='ExecBlock3'

	"""
	#Python script
	try:
		casalog.origin('importasdm')
		execute_string='asdm2MS '+asdm +' --icm \"' +corr_mode + '\" --isrt \"' + srt+ '\" --its \"' + time_sampling+ '\" --ocm \"' + ocorr_mode + '\"'
		casalog.post('Running the asdm2MS standalone invoked as:')
		casalog.post(execute_string)
        	os.system(execute_string)
                ok=fg.open(asdm+'.ms');
                ok=fg.saveflagversion('Original',comment='Original flags at import into CASA',merge='save')
                ok=fg.done();
	except Exception, instance:
		print '*** Error ***',instance

