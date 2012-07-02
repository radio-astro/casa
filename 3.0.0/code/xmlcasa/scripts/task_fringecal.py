import os
from taskinit import *

def fringecal(vis=None,caltable=None,
	      field=None,spw=None,
	      selectdata=None,timerange=None,uvrange=None,antenna=None,scan=None,msselect=None,
	      gaincurve=None,opacity=None,solint=None,refant=None):
	
	#Python script
	try:
		casalog.origin('fringecal')
		cb.open(vis)
		cb.reset()

		cb.selectvis(time=timerange,spw=spw,scan=scan,field=field,
			     baseline=antenna,uvrange=uvrange,chanmode='none',
			     nchan=nchan,start=start,step=step,
			     msselect=msselect);

		if (opacity>0.0): cb.setapply(type='TOPAC',t=-1,opacity=opacity)
		if gaincurve: cb.setapply(type='GAINCURVE',t=-1)
		cb.setsolve(type='K',t=solint,refant=refant,table=caltable)
		cb.state()
		cb.solve()
		cb.close()
	except Exception, instance:
		print '*** Error ***',instance
		cb.close()
		raise Exception, instance

