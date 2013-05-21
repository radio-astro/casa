import os
import numpy as np
from taskinit import *

def blcal(vis=None,caltable=None,
	  field=None,spw=None,intent=None,
	  selectdata=None,timerange=None,uvrange=None,antenna=None,scan=None,
          observation=None, msselect=None,
	  solint=None,combine=None,freqdep=None,calmode=None,solnorm=None,
	  gaintable=None,gainfield=None,interp=None,spwmap=None,
	  parang=None):

	#Python script

	try:

                casalog.origin('blcal')
                if ((type(vis)==str) & (os.path.exists(vis))):
                        cb.open(filename=vis,compress=False,addcorr=False,addmodel=False)
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'

		cb.reset()

		# Do data selection according to selectdata
		if (selectdata):
			# pass all data selection parameters in as specified
			cb.selectvis(time=timerange,spw=spw,scan=scan,field=field,
				     intent=intent, observation=str(observation),
				     baseline=antenna,uvrange=uvrange,chanmode='none',
				     msselect=msselect);
		else:
			# selectdata=F, so time,scan,baseline,uvrange,msselect=''
			# using spw and field specifications only
			cb.selectvis(time='',spw=spw,scan='',field=field,intent=intent,
				     baseline='',uvrange='',chanmode='none',
				     observation='', msselect='')


                # Arrange apply of existing other calibrations
                # First do the existing cal tables...
                ngaintab = 0;
                if (gaintable!=['']):
                        ngaintab=len(gaintable)
                ngainfld = len(gainfield)
		nspwmap = len(spwmap)
		ninterp = len(interp)

		# handle list of list issues with spwmap
		if (nspwmap>0):
			if (type(spwmap[0])!=list):
				# first element not a list, only one spwmap specified
				# make it a list of list
				spwmap=[spwmap];
				nspwmap=1;

                for igt in range(ngaintab):
                        if (gaintable[igt]!=''):

                                # field selection is null unless specified
                                thisgainfield=''
                                if (igt<ngainfld):
                                        thisgainfield=gainfield[igt]

				# spwmap is null unless specifed
				thisspwmap=[-1]
				if (igt<nspwmap):
					thisspwmap=spwmap[igt];

				# interp is 'linear' unless specified
				thisinterp='linear'
				if (igt<ninterp):
					if (interp[igt]==''):
						interp[igt]=thisinterp
					thisinterp=interp[igt]
					
				cb.setapply(t=0.0,table=gaintable[igt],field=thisgainfield,
					    calwt=True,spwmap=thisspwmap,interp=thisinterp)

                # ...and now the specialized terms
                # (BTW, interp irrelevant for these, since they are evaluated)

		# Apply parallactic angle, if requested
		if parang: cb.setapply(type='P')

		# Set up the solve
		if freqdep:
			cb.setsolve(type='MF',t=solint,combine=combine,refant='',table=caltable,apmode=calmode,solnorm=solnorm)
		else:
			cb.setsolve(type='M',t=solint,combine=combine,refant='',table=caltable,apmode=calmode,solnorm=solnorm)

		cb.solve()
		cb.close()
	except Exception, instance:
		print '*** Error ***',instance
		cb.close()
		raise Exception, instance
