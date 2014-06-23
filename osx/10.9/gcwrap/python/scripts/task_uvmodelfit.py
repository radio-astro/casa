import os
from taskinit import *

def uvmodelfit(vis=None,
	       field=None,spw=None,
	       selectdata=None,timerange=None,uvrange=None,antenna=None,scan=None,msselect=None,
	       niter=None,comptype=None,sourcepar=None,varypar=None,outfile=None):

	#Python script
	try:
		casalog.origin('uvmodelfit')
                if ((type(vis)==str) & (os.path.exists(vis))):
                        cb.open(vis)
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'

		# Do data selection according to selectdata
		if (selectdata):
			# pass all data selection parameters in as specified
			cb.selectvis(time=timerange,spw=spw,scan=scan,field=field,
				     baseline=antenna,uvrange=uvrange,chanmode='none',
				     msselect=msselect);
		else:
			# selectdata=F, so time,scan,baseline,uvrange,msselect=''
			# using spw and field specifications only
			cb.selectvis(time='',spw=spw,scan='',field=field,
				     baseline='',uvrange='',chanmode='none',
				     msselect='');

		cb.modelfit(niter=niter,compshape=comptype,par=sourcepar,vary=varypar,file=outfile)
		cb.close()
	except Exception, instance:
		print '*** Error ***',instance
		cb.close()
		raise Exception, instance

