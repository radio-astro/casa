import os
from taskinit import *

def gaincal(vis=None,caltable=None,
	    field=None,spw=None,
	    selectdata=None,timerange=None,uvrange=None,antenna=None,scan=None,msselect=None,
	    solint=None,combine=None,preavg=None,refant=None,minsnr=None,solnorm=None,
	    gaintype=None,calmode=None,append=None,
	    splinetime=None,npointaver=None,phasewrap=None,
	    gaintable=None,gainfield=None,interp=None,spwmap=None,
	    gaincurve=None,opacity=None,parang=None):

	#Python script
        casalog.origin('gaincal')

	try: 

                if ((type(vis)==str) & (os.path.exists(vis))):
                        cb.open(vis)
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'

		# Enforce selectdata=False on its subparameters
		if (selectdata==False):
			timerange=''
			uvrange=''
			antenna=''
			scan=''
			msselect=''
		
		cb.selectvis(time=timerange,spw=spw,scan=scan,field=field,
			     baseline=antenna,uvrange=uvrange,chanmode='none',
			     msselect=msselect);

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
					thisinterp=interp[igt];

				cb.setapply(t=0.0,table=gaintable[igt],field=thisgainfield,
					    calwt=True,spwmap=thisspwmap,interp=thisinterp)
		
		# ...and now the specialized terms
		# (BTW, interp irrelevant for these, since they are evaluated)
		if (opacity>0.0): cb.setapply(type='TOPAC',t=-1,opacity=opacity,calwt=True)
		if gaincurve: cb.setapply(type='GAINCURVE',t=-1,calwt=True)

		# Apply parallactic angle, if requested
		if parang: cb.setapply(type='P')

		# Set up for solving:  
		phaseonly=False
		if (gaintype=='G'):
			cb.setsolve(type='G',t=solint,combine=combine,preavg=preavg,refant=refant,
				    solnorm=solnorm,minsnr=minsnr,table=caltable,
				    apmode=calmode,phaseonly=phaseonly,append=append)
		elif (gaintype=='T'):
			cb.setsolve(type='T',t=solint,combine=combine,preavg=preavg,refant=refant,
				    solnorm=solnorm,minsnr=minsnr,table=caltable,
				    apmode=calmode,phaseonly=phaseonly,append=append)
		elif (gaintype=='GSPLINE'):
			cb.setsolvegainspline(table=caltable,append=append,mode=calmode,
					      refant=refant,splinetime=splinetime,preavg=preavg,
					      npointaver=npointaver,phasewrap=phasewrap)
		cb.solve()
		cb.close()

	except Exception, instance:
		print '*** Error ***',instance
		cb.close()
		raise Exception, instance

