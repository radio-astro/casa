import os
from taskinit import *

def importasdm(asdm=None, vis=None, singledish=None, corr_mode=None, srt=None, time_sampling=None, ocorr_mode=None, compression=None, asis=None, wvr_corrected_data=None, verbose=None, overwrite=None, showversion=None):
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
                if singledish:
                   ocorr_mode = 'ao'
                   corr_mode = 'ao'
                   casalog.post('corr_mode and ocorr_mode is forcibly set to ao.')
                   if compression:
                      casalog.post('compression=True has no effect for single-dish format.')
		if(len(vis) > 0) :
		   viso = vis
                   if singledish:
                      viso = vis.rstrip('/') + '.importasdm.tmp.ms'
		else :
		   viso = asdm + '.ms'
		   vis = asdm
                   if singledish:
                      viso = asdm.rstrip('/') + '.importasdm.tmp.ms'
                      vis = asdm.rstrip('/') + '.asap'
		execute_string='asdm2MS  --icm \"' +corr_mode + '\" --isrt \"' + srt+ '\" --its \"' + time_sampling+ '\" --ocm \"' + ocorr_mode + '\" --wvr-corrected-data \"' + wvr_corrected_data + '\" --asis \"' + asis + '\" --logfile \"' +casalog.logfile() +'\"'
		if(compression) :
		   execute_string= execute_string +' --compression'
		if(verbose) :
		   execute_string= execute_string +' --verbose'
		if(showversion) :
		   execute_string= execute_string +' --revision'
		if not overwrite and os.path.exists(viso) :
		   raise Exception, "You have specified and existing ms and have indicated you do not wish to overwrite it"
		execute_string = execute_string + ' ' + asdm + ' ' + viso
		casalog.post('Running the asdm2MS standalone invoked as:')
		#print execute_string
		casalog.post(execute_string)
        	os.system(execute_string)
		if compression :
                   #viso = viso + '.compressed'
                   viso = visover.rstrip('.ms') + '.compressed.ms'
                ok=fg.open(viso);
                ok=fg.saveflagversion('Original',comment='Original flags at import into CASA',merge='save')
                ok=fg.done();
                if singledish:
                   casalog.post('temporary MS file: %s'%viso)
                   casalog.post('        ASAP file: %s'%vis)
                   try:
                      from asap import scantable
                      s = scantable(viso,average=False,getpt=True)
                      s.save(vis,format='ASAP',overwrite=overwrite)
                      # remove intermediate products
                      if os.path.exists(viso):
                         os.system('rm -rf %s'%viso)
                         os.system('rm -rf %s.flagversions'%viso)
                   except Exception, instance:
                      if type(instance) == ImportError and (str(instance)).find('asap') != -1:
                         casalog.post(str(instance),'SEVERE')
                         casalog.post('You should build ASAP to be able to create single-dish data.','SEVERE')
                         if os.path.exists(viso):
                            os.system('rm -rf %s'%viso)
                            os.system('rm -rf %s.flagversions'%viso)
                         
                      else:
                         raise Exception, instance
	except Exception, instance:
		print '*** Error ***',instance

