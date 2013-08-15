import os
import numpy as np
from callibrary import *
from taskinit import *

def bandpass(vis=None,caltable=None,
	     field=None,spw=None,intent=None,
	     selectdata=None,timerange=None,uvrange=None,antenna=None,scan=None,
             observation=None,msselect=None,
	     solint=None,combine=None,refant=None,minblperant=None,
	     minsnr=None,solnorm=None,
	     bandtype=None,smodel=None,
	     append=None,fillgaps=None,
	     degamp=None,degphase=None,visnorm=None,
	     maskcenter=None,maskedge=None,
	     docallib=None,callib=None,
	     gaintable=None,gainfield=None,interp=None,spwmap=None,
	     parang=None):

	#Python script
        casalog.origin('bandpass')

	try:
		mycb=cbtool()
		
                if ((type(vis)==str) & (os.path.exists(vis))):    
                        mycb.open(filename=vis,compress=False,addcorr=False,addmodel=False)
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'
		mycb.reset()

		# Do data selection according to selectdata
		if (selectdata):
			# pass all data selection parameters in as specified
			mycb.selectvis(time=timerange,spw=spw,scan=scan,field=field,
				       intent=intent, observation=str(observation),
				       baseline=antenna,uvrange=uvrange,chanmode='none',
				       msselect=msselect);
		else:
			# selectdata=F, so time,scan,baseline,uvrange,msselect=''
			# using spw and field specifications only
			mycb.selectvis(time='',spw=spw,scan='',field=field,intent=intent,
				       observation='', baseline='',uvrange='',chanmode='none',
				       msselect='');


		# set the model, if specified
		if (len(smodel)>0):
			mycb.setptmodel(smodel);
		

		# Arrange applies....

		if docallib:
			# by cal library
			if isinstance(callib,dict):
				mycb.setcallib(callib)
			elif isinstance(callib,str):
				# parse from file
				mycallib=callibrary()
				mycallib.read(callib)
				mycb.setcallib(mycallib.cld)

		else:

			# by traditional parameters

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
							interp[igt]=thisinterp;
						thisinterp=interp[igt];
					
					mycb.setapply(t=0.0,table=gaintable[igt],field=thisgainfield,
						      calwt=True,spwmap=thisspwmap,interp=thisinterp)

		# ...and now the specialized terms
		# (BTW, interp irrelevant for these, since they are evaluated)

		# Apply parallactic angle, if requested
		if parang: mycb.setapply(type='P')

		# set up for solving (
		if (bandtype=='B'):
			mycb.setsolve(type='B',t=solint,combine=combine,refant=refant,minblperant=minblperant,
				    solnorm=solnorm,minsnr=minsnr,table=caltable,append=append,fillgaps=fillgaps)
		elif (bandtype=='BPOLY'):
			mycb.setsolvebandpoly(refant=refant,table=caltable,append=append,
					    t=solint,combine=combine,
					    degamp=degamp,degphase=degphase,visnorm=visnorm,
					    solnorm=solnorm,maskcenter=maskcenter,maskedge=maskedge)

		mycb.solve()
		mycb.close()

	except Exception, instance:
		print '*** Error ***',instance
		mycb.close()
		raise Exception, instance

