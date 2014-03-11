import os
from taskinit import *

def exportasdm(vis=None, asdm=None, datacolumn=None, archiveid=None, rangeid=None,
	       subscanduration=None, sbduration=None, apcorrected=None,
	       verbose=None, showversion=None, useversion=None):
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

	sbduration -- maximum duration of a scheduling block in the output ASDM
	          default: "2700s"

	apcorrected -- If true, the data in column datacolumn should be regarded
	          as having atmospheric phase correction, default: True

	verbose     -- produce log output, default: True

	showversion -- report the version of the ASDM class set, 
                 default: True

	useversion -- Selects the version of MS2asdm to be used (presently only \'v3\' is available)
              default: v3

	"""
	#Python script

	# make ms and tb tool local 
	ms = casac.ms()
	tb = casac.table()

	try:
		casalog.origin('exportasdm')
		parsummary = 'vis=\"'+str(vis)+'\", asdm=\"'+str(asdm)+'\", datacolumn=\"'+str(datacolumn)+'\",'
		casalog.post(parsummary)
		parsummary = 'archiveid=\"'+str(archiveid)+'\", rangeid=\"'+str(rangeid)+'\", subscanduration=\"'+str(subscanduration)+'\",'
		casalog.post(parsummary)
		parsummary = 'sbduration=\"'+str(sbduration)+'\", apcorrected='+str(apcorrected)+', verbose='+str(verbose)+','
		casalog.post(parsummary)
		parsummary = 'showversion='+str(showversion)+', useversion=\"'+str(useversion)+'\"'
		casalog.post(parsummary)

		if not (type(vis)==str) or not (os.path.exists(vis)):
			raise Exception, 'Visibility data set not found - please verify the name'
		
		if (asdm == ""):
			raise Exception, "Must provide output data set name in parameter asdm."            
		
		if os.path.exists(asdm):
			raise Exception, "Output ASDM %s already exists - will not overwrite." % asdm

		# determine sb and subscan duration
		ssdur_secs = 24.*3600 # default is one day, i.e. there will be only one subscan per scan
		if not(subscanduration==""):
			if (qa.canonical(subscanduration)['unit'].find('s') < 0):
				raise TypeError, "subscanduration is not a valid time quantity: %s" % subscanduration
			else:
				ssdur_secs = qa.canonical(subscanduration)['value']

		sbdur_secs = 2700. # default is 45 minutes
		if not(sbduration==""):
			if (qa.canonical(sbduration)['unit'].find('s') < 0):
				raise TypeError, "sbduration is not a valid time quantity: %s" % sbduration
			else:
				sbdur_secs = qa.canonical(sbduration)['value']

		# create timesorted copy of the input ms
		os.system('rm -rf '+vis+'-tsorted')
		ms.open(vis)
		ms.timesort(vis+'-tsorted')
		ms.close()

		tsortvis = vis+'-tsorted'

		# Prepare for actual exportasdm
		casalog.post("Checking timesorted MS for potential problems ... ")
		tb.open(tsortvis)
		a = tb.getcol('PROCESSOR_ID')
		a0 = a[0]
		candoit = True
		for i in xrange(0,len(a)-1):
			if(a[i]!=a[i+1]):
				candoit = False
				break
		tb.close()

		if candoit:
			casalog.post("   Checking if PROCESSOR and MAIN need modifications ...")
			tb.open(tsortvis+'/PROCESSOR')
			nprocrows = tb.nrows()
			tb.close()
			if ((nprocrows>0) and (a0>-1)):
				tb.open(tsortvis+'/PROCESSOR')
				therow = tb.nrows()-1 
				mode0 = tb.getcell('MODE_ID',a0)
				tb.close()
				offset = 1
				if nprocrows>1:
					casalog.post("   Modifying PROCESSOR subtable ...")
					while (nprocrows>1 and therow>0):
						tb.open(tsortvis+'/PROCESSOR', nomodify=False)
						therow = tb.nrows()-offset
						if(tb.getcell('MODE_ID',therow)!=mode0):
							tb.removerows([therow])
						else:
							offset += 1
						nprocrows = tb.nrows()
					casalog.post("... done.")

					casalog.post("   Modifying processor ids in main table ...")
					a = a - a # set all precessor ids to zero
					tb.open(tsortvis, nomodify=False)
					tb.putcol('PROCESSOR_ID', a)
					tb.close()
					casalog.post(" ... done.")
				else:
					casalog.post("   No modifications to proc id in PROCESSOR and MAIN necessary.")
			casalog.post("   Checking if SYSCAL needs modifications ...")
			if(os.path.exists(tsortvis+'/SYSCAL')):
				for cname in ['TANT_SPECTRUM',
					      'TSYS_SPECTRUM',
					      'TANT_TSYS_SPECTRUM',
					      'TCAL_SPECTRUM',
					      'TRX_SPECTRUM',
					      'TSKY_SPECTRUM',
					      'PHASE_DIFF_SPECTRUM']:
					tb.open(tsortvis+'/SYSCAL', nomodify=False)
					if(cname in tb.colnames()):
						cdesc = tb.getcoldesc(cname)
						if cdesc.has_key('ndim') and (cdesc['ndim']==-1):
							tb.removecols([cname])
							casalog.post('   Removed empty array column '+cname+' from table SYSCAL.')
					tb.close()

			casalog.post("   Checking if OBSERVATION needs modifications ...")
			tb.open(tsortvis+'/OBSERVATION')
			nobsrows = tb.nrows()
			tb.close()
			if(nobsrows>0):
				tb.open(tsortvis+'/OBSERVATION', nomodify=False)
				cdesc = tb.getcoldesc('LOG')
				if cdesc.has_key('ndim') and (cdesc['ndim']>0):
					b = tb.getvarcol('LOG')
					if not (type(b['r1'])==bool):
						kys = b.keys()
						modified = False
						b2 = []
						for i in xrange(0,len(kys)):
							k = 'r'+str(i+1)
							if (b[k][0] == [''])[0]:
								b[k][0] = ["-"]
								modified = True
							b2.append([b[k][0][0]])
						if modified:
							tb.putcol('LOG',b2)
							casalog.post("   Modified log column in OBSERVATION table.")
				tb.close()
			casalog.post("Done.")
		else:
			raise Exception, "More than one processor id in use in the main table. Cannot proceed."		    
		
		# generate call to ms2ASDM executable

		execute_string=  '--datacolumn \"' + datacolumn 
		execute_string+= '\" --archiveid \"' + archiveid + '\" --rangeid \"' + rangeid
		execute_string+= '\" --subscanduration \"' + str(ssdur_secs)
		execute_string+= '\" --schedblockduration \"' + str(sbdur_secs) 
		execute_string+= '\" --logfile \"' + casalog.logfile() +'\"'
		
		if(not apcorrected):
			execute_string+= ' --apuncorrected'
		if(verbose):
			execute_string+= ' --verbose'
		if(showversion):
			execute_string+= ' --revision'

		theexecutable = 'MS2asdm'
		if (useversion == 'v3'):
			theexecutable = 'MS2asdm'

		execute_string += ' ' + vis + '-tsorted ' + asdm

		execute_string = theexecutable+' '+execute_string

		if(verbose):
			casalog.post('Running '+theexecutable+' standalone invoked as:')
			casalog.post(execute_string)
		else:
			print execute_string

        	rval = os.system(execute_string)

		os.system('rm -rf '+vis+'-tsorted')

		if(rval == 0):
			return True
		else:
			casalog.post(theexecutable+' terminated with exit code '+str(rval),'WARN')
			return False
	
	except Exception, instance:
		casalog.post("Error: "+str(instance), 'SEVERE')
		raise Exception, instance
