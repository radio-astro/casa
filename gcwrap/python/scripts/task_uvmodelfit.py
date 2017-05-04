import os
from taskinit import *

def uvmodelfit(vis=None,
	       field=None,spw=None,
	       selectdata=None,timerange=None,uvrange=None,antenna=None,scan=None,msselect=None,
	       niter=None,comptype=None,sourcepar=None,varypar=None,outfile=None):

	#Python script
	try:
		mycb = cbtool()

		casalog.origin('uvmodelfit')
                if ((type(vis)==str) & (os.path.exists(vis))):
                        mycb.setvi(old=True,quiet=False);  # old VI for now
                        mycb.open(vis)
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'

		# Do data selection according to selectdata
		if (selectdata):
			# pass all data selection parameters in as specified
			mycb.selectvis(time=timerange,spw=spw,scan=scan,field=field,
				       baseline=antenna,uvrange=uvrange,chanmode='none',
				       msselect=msselect);
		else:
			# selectdata=F, so time,scan,baseline,uvrange,msselect=''
			# using spw and field specifications only
			mycb.selectvis(time='',spw=spw,scan='',field=field,
				       baseline='',uvrange='',chanmode='none',
				       msselect='');

		mycb.modelfit(niter=niter,compshape=comptype,par=sourcepar,vary=varypar,file=outfile)
		mycb.close()
	except Exception, instance:
		print '*** Error ***',instance
		mycb.close()
		raise Exception, instance

