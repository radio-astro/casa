import os
import shutil
from taskinit import casalog, mstool, write_history

def importfitsidi(fitsidifile,vis):
	"""Convert FITS-IDI visibility file into a CASA visibility file (MS).

	Keyword arguments:
	fitsidifile -- Name(s) of input FITS IDI file(s)
		default: None; example='3C273XC1.IDI' or ['3C273XC1.IDI1', '3C273XC1.IDI2']
	vis -- Name of output visibility file (MS)
		default: None; example: vis='3C273XC1.ms'
		
	"""

	#Python script
        retval = True
	try:
		casalog.origin('importfitsidi')
		casalog.post("")
                myms = mstool()
		if(type(fitsidifile)==str):
			casalog.post('### Reading file '+fitsidifile, 'INFO')
			myms.fromfitsidi(vis,fitsidifile)
			myms.close()
		elif(type(fitsidifile)==list):
			clist = fitsidifile
			casalog.post('### Reading file '+clist[0], 'INFO')
			myms.fromfitsidi(vis,clist[0])
			myms.close()
			clist.pop(0)
			tname = '_importfitsidi_tmp_'+vis
			shutil.rmtree(tname, ignore_errors=True)
			for fidifile in clist:
				casalog.post('### Reading file '+fidifile, 'INFO')
				myms.fromfitsidi(tname,fidifile)
				myms.close()
				myms.open(vis, nomodify=False)
				myms.concatenate(msfile=tname, freqtol='', dirtol='')
				myms.close()
				shutil.rmtree(tname, ignore_errors=True)
		else:
                        raise Exception, 'Parameter fitsidifile should be of type str or list'			
	        # write history
                try:
                        param_names = importfitsidi.func_code.co_varnames[:importfitsidi.func_code.co_argcount]
                        param_vals = [eval(p) for p in param_names]   
                        retval &= write_history(myms, vis, 'importfitsidi', param_names,
                                                param_vals, casalog)
                except Exception, instance:
                        casalog.post("*** Error \'%s\' updating HISTORY" % (instance),
                                     'WARN')

	except Exception, instance: 
		print '*** Error ***',instance
		shutil.rmtree('_importfitsidi_tmp_'+vis, ignore_errors=True)
		raise Exception, instance


