import os
from taskinit import casalog, fgtool, mstool, write_history

def importuvfits(fitsfile, vis, antnamescheme=None):
	"""Convert a UVFITS file to a CASA visibility data set (MS):

	Keyword arguments:
	fitsfile -- Name of input UV FITS file
		default: <unset>; example='3C273XC1.fits'
	vis -- Name of output visibility file (MS)
		default: <unset>; example: vis='3C273XC1.ms'
	"""
	#Python script
	ok = True
	try:
		casalog.origin('importuvfits')
		casalog.post("")
		myms = mstool.create()
		myms.fromfits(vis, fitsfile, antnamescheme=antnamescheme)
		myms.close()
	except Exception, instance: 
		print "*** Error importing %s to %s ***" % (fitsfile, vis), instance
		raise Exception, instance

	# Write the args to HISTORY.
	try:
		param_names = importuvfits.func_code.co_varnames[:importuvfits.func_code.co_argcount]
		param_vals = [eval(p) for p in param_names]
		ok &= write_history(myms, vis, 'importuvfits', param_names, param_vals,
				    casalog)
	except Exception, instance:
		casalog.post("*** Error \'%s\' updating HISTORY (importuvfits)" % (instance),
			     'WARN')

	# save original flagversion
	try:
		myfg = fgtool.create()
		ok &= myfg.open(vis)
		ok &= myfg.saveflagversion('Original',
					   comment='Original flags at import into CASA',
					   merge='replace')
		ok &= myfg.done()
	except Exception, instance: 
		print '*** Error saving original flags (importuvfits) ***', instance
		raise Exception, instance
