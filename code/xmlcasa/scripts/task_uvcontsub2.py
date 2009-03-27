import os
from taskinit import *

def uvcontsub2(vis,field,fitspw,combine,solint,fitorder,spw,splitdata):

	#Python script

	try:

                casalog.origin('uvcontsub2')
                if ((type(vis)==str) & (os.path.exists(vis))):
                        cb.open(vis)
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'

		# for now, forbid requests for fitorder>0 
		if (fitorder>0):
			raise Exception, "Sorry, uvcontsub2 currently only supports fitorder=0."
		

		# select the data for continuum subtraction
		cb.reset()
		cb.selectvis(spw=fitspw,field=field)

                # Arrange apply of existing other calibrations
                # First do the existing cal tables...
                #ngaintab = 0;
                #if (gaintable!=['']):
                #        ngaintab=len(gaintable)
                #ngainfld = len(gainfield)
		#nspwmap = len(spwmap)
		#ninterp = len(interp)

		# handle list of list issues with spwmap
		#if (nspwmap>0):
		#	if (type(spwmap[0])!=list):
		#		# first element not a list, only one spwmap specified
		#		# make it a list of list
		#		spwmap=[spwmap];
		#		nspwmap=1;

                #for igt in range(ngaintab):
                #        if (gaintable[igt]!=''):

                #                # field selection is null unless specified
                #                thisgainfield=''
                #                if (igt<ngainfld):
                #                        thisgainfield=gainfield[igt]

		#		# spwmap is null unless specifed
		#		thisspwmap=[-1]
		#		if (igt<nspwmap):
		#			thisspwmap=spwmap[igt];

		#		# interp is 'linear' unless specified
		#		thisinterp='linear'
		#		if (igt<ninterp):
		#			if (interp[igt]==''):
		#				interp[igt]=thisinterp
		#			thisinterp=interp[igt]
					
		#		cb.setapply(t=0.0,table=gaintable[igt],field=thisgainfield,
		#			    calwt=True,spwmap=thisspwmap,interp=thisinterp)

                # ...and now the specialized terms
                # (BTW, interp irrelevant for these, since they are evaluated)
                #if (opacity>0.0): cb.setapply(type='TOPAC',t=-1,opacity=opacity,calwt=True)
                #if gaincurve: cb.setapply(type='GAINCURVE',t=-1,calwt=True)

		# Apply parallactic angle, if requested
		#if parang: cb.setapply(type='P')

		# Set up the solve
		amuellertab='Temp_contsub.tab'

		cb.setsolve(type='A',t=solint,table=amuellertab,combine=combine)

		# solve for the continuum
		cb.solve()

		# subtract the continuum
		cb.selectvis(field=field,spw=spw)
		aspwmap=-1
		# if we combined on spw in solve, fanout the result with spwmap=-999;
		if (combine.find('spw')==0):
			aspwmap=-999
		cb.setapply(table=amuellertab,spwmap=aspwmap)

		# Generate CORRECTED_DATA without continuum
		cb.correct()
		# Generate MODEL_DATA with only the continuum model
		cb.corrupt()

		cb.close()

		# Delete the temporary caltable
		os.system('rm -rf '+amuellertab)

		# if requeseted, generate output datasets:
		if (splitdata==True):
			ms.open(vis,nomodify=False)
                        ms.split(vis.rstrip('/')+'.cont',field=field,spw=spw,whichcol='MODEL_DATA')
                        ms.split(vis.rstrip('/')+'.contsub',field=field,spw=spw,whichcol='CORRECTED_DATA')
			ms.close()

			
		ms.open(vis,nomodify=False)
                ms.writehistory('taskname = uvcontsub2',origin='uvcontsub2')
                ms.writehistory(message='vis         = "'+str(vis)+'"',origin='uvcontsub2')
                ms.writehistory(message='field       = "'+str(field)+'"',origin='uvcontsub2')
                ms.writehistory(message='fitspw       = '+str(spw),origin='uvcontsub2')
                ms.writehistory(message='solint      = '+str(solint),origin='uvcontsub2')
                ms.writehistory(message='fitorder    = '+str(fitorder),origin='uvcontsub2')
                ms.writehistory(message='spw       = '+str(spw),origin='uvcontsub2')
                ms.writehistory(message='splitdata   = '+str(splitdata),origin='uvcontsub2')
                ms.close()

	except Exception, instance:
		print '*** Error ***',instance
		cb.close()
		raise Exception, instance
