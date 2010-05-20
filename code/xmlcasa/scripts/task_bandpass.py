import os
from taskinit import *

def bandpass(vis=None,caltable=None,
	     field=None,spw=None,
	     selectdata=None,timerange=None,uvrange=None,antenna=None,scan=None,msselect=None,
	     solint=None,combine=None,refant=None,minblperant=None,
	     minsnr=None,solnorm=None,
	     bandtype=None,append=None,fillgaps=None,
	     degamp=None,degphase=None,visnorm=None,
	     maskcenter=None,maskedge=None,
	     gaintable=None,gainfield=None,interp=None,spwmap=None,
	     gaincurve=None,opacity=None,parang=None):

	#Python script
        casalog.origin('bandpass')

	try:
                if ((type(vis)==str) & (os.path.exists(vis))):    
                        cb.open(vis)
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'
		cb.reset()

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


		#Apply other calibrations
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
		if (opacity>0.0): cb.setapply(type='TOPAC',t=-1,opacity=opacity,calwt=True)
		if gaincurve: cb.setapply(type='GAINCURVE',t=-1,calwt=True)

		# Apply parallactic angle, if requested
		if parang: cb.setapply(type='P')

		# set up for solving (
		if (bandtype=='B'):
			cb.setsolve(type='B',t=solint,combine=combine,refant=refant,minblperant=minblperant,
				    solnorm=solnorm,minsnr=minsnr,table=caltable,append=append,fillgaps=fillgaps)
		elif (bandtype=='BPOLY'):
			cb.setsolvebandpoly(refant=refant,table=caltable,append=append,
					    t=solint,combine=combine,
					    degamp=degamp,degphase=degphase,visnorm=visnorm,
					    solnorm=solnorm,minsnr=minsnr,maskcenter=maskcenter,maskedge=maskedge)

		cb.solve()
		cb.close()

	except Exception, instance:
		print '*** Error ***',instance
		cb.close()
		raise Exception, instance

