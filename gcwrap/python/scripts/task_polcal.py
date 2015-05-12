import os
import numpy as np
from callibrary import *
from taskinit import *

def polcal(vis=None,caltable=None,
	   field=None,spw=None,intent=None,
	   selectdata=None,timerange=None,uvrange=None,antenna=None,scan=None,
           observation=None, msselect=None,
	   solint=None,combine=None,preavg=None,refant=None,minblperant=None,minsnr=None,
	   poltype=None,smodel=None,append=None,
	   docallib=None,callib=None,
	   gaintable=None,gainfield=None,interp=None,spwmap=None):

	#Python script
        casalog.origin('polcal')

	try:

                mycb=cbtool()
		
                if ((type(vis)==str) & (os.path.exists(vis))):
                        mycb.open(filename=vis,compress=False,addcorr=False,addmodel=False)
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'

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
				       baseline='',uvrange='',chanmode='none',
				       observation='', msselect='')

                # set the model, if specified
                if (len(smodel)>0):
                        mycb.setptmodel(smodel);

		# Arrange applies....

		if docallib:
			# by cal library from file
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

		# D-terms require parallactic angle
		mycb.setapply(type='P')

		# Set up for solving: 
		phaseonly=False
		mycb.setsolve(type=poltype,t=solint,combine=combine,preavg=preavg,refant=refant,
			      minblperant=minblperant,solnorm=False,minsnr=minsnr,table=caltable,
			      apmode='ap',phaseonly=phaseonly,append=append)

		mycb.solve()
		mycb.close()

	except Exception, instance:
		print '*** Error ***',instance
		mycb.close()
		raise Exception, instance

