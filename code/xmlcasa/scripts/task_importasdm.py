import os
from taskinit import *

def importasdm(asdm=None, vis=None, corr_mode=None, srt=None, time_sampling=None, ocorr_mode=None, compression=None, asis=None, wvr_corrected_data=None, verbose=None, showversion=None):
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
		viso = ''
		if(len(vis) > 0) :
		   viso = vis + '.ms'
		else :
		   viso = asdm + '.ms'
		   vis = asdm
		execute_string='asdm2MS  --icm \"' +corr_mode + '\" --isrt \"' + srt+ '\" --its \"' + time_sampling+ '\" --ocm \"' + ocorr_mode + '\" --wvr-corrected-data \"' + wvr_corrected_data + '\" --asis \"' + asis + '\" --logfile \"' +casalog.logfile() +'\"'
		if(compression) :
		   execute_string= execute_string +' --compression'
		if(verbose) :
		   execute_string= execute_string +' --verbose'
		if(showversion) :
		   execute_string= execute_string +' --revision'
		execute_string = execute_string + ' ' + asdm + ' ' + viso
		casalog.post('Running the asdm2MS standalone invoked as:')
		#print execute_string
		casalog.post(execute_string)
        	os.system(execute_string)
		if compression :
			viso = vis + '.compressed.ms'
                ok=fg.open(viso);
                ok=fg.saveflagversion('Original',comment='Original flags at import into CASA',merge='save')
                ok=fg.done();
	except Exception, instance:
		print '*** Error ***',instance

