import os
import time
import numpy as np
from taskinit import *
from callibrary import *
from parallel.parallel_task_helper import ParallelTaskHelper

def applycal(vis=None,
	     field=None,
	     spw=None,
	     intent=None,
	     selectdata=None,
	     timerange=None,
	     uvrange=None,
	     antenna=None,
	     scan=None,
             observation=None,
	     msselect=None,
	     docallib=None,
	     callib=None,
	     gaintable=None,
	     gainfield=None,
	     interp=None,
	     spwmap=None,
	     calwt=None,
	     parang=None,
	     applymode=None,
	     flagbackup=None):


	#Python script
        casalog.origin('applycal')

        # Take care of the trivial parallelization
        if ParallelTaskHelper.isParallelMS(vis):
                # To be safe convert file names to absolute paths.
                gaintable = ParallelTaskHelper.findAbsPath(gaintable)
                helper = ParallelTaskHelper('applycal', locals())
                helper.go()
                return

	try:
                mycb = cbtool()
                if ((type(vis)==str) & (os.path.exists(vis))):
			# add CORRECTED_DATA column
                        mycb.open(filename=vis,compress=False,
				  addcorr=True,addmodel=False)
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'

		# enforce default if unspecified
		if applymode=='':
			applymode='calflag'
			
		# Back up the flags, if requested (and if necessary)
		if (flagbackup and
		    (applymode!='calonly' and
		     applymode!='trial') ):
			aflocal = casac.agentflagger()
			aflocal.open(vis)
			backup_flags(aflocal)
			aflocal.done()

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
                                     observation='', baseline='',uvrange='',
                                     chanmode='none', msselect='')


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

			ncalwt=len(calwt)
			if ncalwt==1:
				calwt = [calwt[0] for i in range(ngaintab)]

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
						      calwt=calwt[igt],spwmap=thisspwmap,interp=thisinterp)

		# ...and now the specialized terms

		# Apply parallactic angle, if requested
		if parang: mycb.setapply(type='P')

		mycb.correct(applymode)

		# report what the flags did
		reportflags(mycb.activityrec())
		
		mycb.close()

	        #write history
                try:
                        param_names = applycal.func_code.co_varnames[:applycal.func_code.co_argcount]
                        param_vals = [eval(p) for p in param_names]
                        write_history(mstool(), vis, 'applycal', param_names,
                                      param_vals, casalog)
                except Exception, instance:
                        casalog.post("*** Error \'%s\' updating HISTORY" % (instance),
                                     'WARN')

	except Exception, instance:
		print '*** Error ***',instance
		mycb.close()
		raise Exception, instance


def backup_flags(aflocal):

        # Create names like this:
        # before_applycal_1,
        # before_applycal_2,
        # before_applycal_3,
        # etc
        #
        # Generally  before_applycal_<i>, where i is the smallest
        # integer giving a name, which does not already exist
       
        existing = aflocal.getflagversionlist(printflags=False)

	# remove comments from strings
	existing = [x[0:x.find(' : ')] for x in existing]
	i = 1
	while True:
		versionname = "before_applycal_" + str(i)

		if not versionname in existing:
			break
		else:
			i = i + 1

        time_string = str(time.strftime('%Y-%m-%d %H:%M:%S'))

        aflocal.saveflagversion(versionname=versionname,
                           comment='Flag backup before applycal on ' + time_string,
                           merge='replace')


def reportflags(rec):
	try:
		if (rec.keys().count('origin')==1 and
		    rec['origin']=='Calibrater::correct' and
		    rec.keys().count('VisEquation')==1):
			casalog.post("Calibration apply flagging statistics (among calibrateable spws):")
			VE=rec['VisEquation']
			nterm=len(VE)
			if nterm>0:
				casalog.post("  Total selected visibilities (among calibrateable spws) = "+str(VE['*1']['ndata']))
				casalog.post("  Flags:")
				for iterm in range(nterm):
					VEi=VE['*'+str(iterm+1)]
					flstr='   '+VEi['type']
					flstr+=': '
					flstr+='In: '+str(VEi['nflagIn'])
					flstr+=' ('+str(100.*VEi['nflagIn']/VEi['ndata'])+'%) --> '
					flstr+='Out: '+str(VEi['nflagOut'])
					flstr+=' ('+str(100.*VEi['nflagOut']/VEi['ndata'])+'%)'
					if (VEi.has_key('table')):
						flstr+=' ('+VEi['table']+')'
					casalog.post(flstr)
	except Exception, instance:
		# complain mildly, but don't alarm
		casalog.post("Error formatting some or all of the applycal flagging log info: "+str(instance))
