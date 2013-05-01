import os
import re
import shutil
import time
import tempfile

# shutil.copytree is useless with directories created by tempfile
# (or any directories that already exist).
from distutils.dir_util import copy_tree

from taskinit import *
from update_spw import *

from parallel.parallel_task_helper import ParallelTaskHelper
from virtualconcat_cli import  virtualconcat_cli as virtualconcat

mycb, myms, mytb = gentools(['cb', 'ms', 'tb'])

def uvcontsub(vis, field, fitspw, excludechans, combine, solint, fitorder, spw, want_cont):
    
    if ParallelTaskHelper.isParallelMS(vis):
        helper = ParallelTaskHelper('uvcontsub', locals())
        helper._consolidateOutput = False
        retVar = helper.go()

        # Gather the list of continuum subtraction-SubMSs 
        cont_subMS_list = []
        contsub_subMS_list = []
        for subMS in retVar:
            if retVar[subMS]:
                cont_subMS_list.append(subMS + ".cont")
                contsub_subMS_list.append(subMS + ".contsub")

        # We have to sort the list because otherwise it  
        # depends on the time the engines dispatches their sub-MSs
        cont_subMS_list.sort()
        contsub_subMS_list.sort()

        # deal with the pointing table
        auxfile = "uvcontsub_aux2_"+str(time.time())
        pnrows = 0
        try:
            mytb.open(vis+'/POINTING')
            pnrows = mytb.nrows()
            mytb.close()
            if(pnrows>0): 
                shutil.copytree(os.path.realpath(vis+'/POINTING'), auxfile)
        except Exception, instance:
            casalog.post("Error handling POINTING table %s: %s" %
                         (vis+'/POINTING',str(instance)),'SEVERE')

        if want_cont:
            try:
                virtualconcat(concatvis=helper._arg['vis'] + ".cont",vis=cont_subMS_list,
                              copypointing=False)
            except Exception, instance:
                casalog.post("Error concatenating continuum sub-MSs %s: %s" % 
                             (str(cont_subMS_list),str(instance)),'SEVERE')     
        try:
            virtualconcat(concatvis=helper._arg['vis'] + ".contsub",vis=contsub_subMS_list,
                          copypointing=False)
        except Exception, instance:
            casalog.post("Error concatenating continuum-subtracted sub-MSs %s: %s" %
                         (str(contsub_subMS_list),str(instance)),'SEVERE')

        # Remove continuum subtraction-SubMSs
        if want_cont:
            for subMS in cont_subMS_list:
                if (os.system("rm -rf " + subMS ) != 0):
                    casalog.post("Problem removing continuum sub-MS %s into working directory" % (subMS),'SEVERE')
        for subMS in contsub_subMS_list:
            if (os.system("rm -rf " + subMS ) != 0):
                casalog.post("Problem removing continuum-subtracted sub-MS %s into working directory" % (subMS),'SEVERE')

        if(pnrows>0): # put back the POINTING table
            casalog.post("Restoring the POINTING table ...",'NORMAL')
            try:
                if want_cont:
                    theptab = os.path.realpath(helper._arg['vis'] + ".cont/POINTING")
                    os.system('rm -rf '+theptab)
                    shutil.copytree(auxfile, theptab)
                theptab = os.path.realpath(helper._arg['vis'] + ".contsub/POINTING")
                os.system('rm -rf '+theptab)
                os.system('mv '+auxfile+' '+theptab)
            except Exception, instance:
                casalog.post("Error restoring pointing table from %s: %s" % 
                             (auxfile,str(instance)),'SEVERE')
        
        # Restore origin (otherwise gcwrap shows virtualconcat)
        casalog.origin('uvcontsub')
        
        return True
    
    # Run normal code
    try:
        casalog.origin('uvcontsub')

        # Get these checks done and out of the way.
        # This one is redundant - it is already checked at the XML level.
        if not os.path.isdir(vis):
            raise Exception, 'Visibility data set not found - please verify the name'
        
        #
        if excludechans:
            locfitspw=_quantityRangesToChannels(vis,field,fitspw,excludechans)
            casalog.post("Exclude channels in fitspw: spws:channels will be used in the fit are:%s" % locfitspw)
        elif fitspw.count('Hz'):
            locfitspw=_quantityRangesToChannels(vis,field,fitspw,excludechans)
        else:
            locfitspw=fitspw
        #print "locfitspw=",locfitspw
        mytb.open(vis + '/SPECTRAL_WINDOW')
        allspw = '0~' + str(mytb.nrows() - 1)
        mytb.close()
        if 'spw' not in combine:
            #spwmfitspw = subtract_spws(spw, fitspw)
            spwmfitspw = subtract_spws(spw, locfitspw)
            if spwmfitspw == 'UNKNOWN':
                #spwmfitspw = subtract_spws(allspw, fitspw)
                spwmfitspw = subtract_spws(allspw, locfitspw)
            if spwmfitspw:
                raise Exception, "combine must include 'spw' when the fit is being applied to spws outside fitspw."
        
        # cb will put the continuum-subtracted data in CORRECTED_DATA, so
        # protect vis by splitting out its relevant parts to a working copy.
        csvis = vis.rstrip('/') + '.contsub'
        
        # The working copy will need all of the channels in fitspw + spw, so we
        # may or may not need to filter it down to spw at the end.
        #myfitspw = fitspw
        myfitspw = locfitspw
        myspw = spw

        # We only need the spws in the union of fitspw and spw, but keep all
        # the channels or the weights (as used by calibrater) will be messed up.
        #tempspw = re.sub(r':[^,]+', '', join_spws(fitspw, spw))
        tempspw = re.sub(r':[^,]+', '', join_spws(locfitspw, spw))
        if tempspw == allspw:
            tempspw = ''
        if tempspw:
            # The split will reindex the spws.  Update spw and fitspw.
            # Do fitspw first because the spws in spw are supposed to be
            # a subset of the ones in fitspw.
            casalog.post('split is being run internally, and the selected spws')
            casalog.post('will be renumbered to start from 0 in the output!')

            # Now get myfitspw.
            #myfitspw = update_spwchan(vis, tempspw, fitspw)
            myfitspw = update_spwchan(vis, tempspw, locfitspw)
            myspw = update_spwchan(vis, tempspw, spw)

        final_csvis = csvis
        workingdir = os.path.abspath(os.path.dirname(vis.rstrip('/')))
        csvis = tempfile.mkdtemp(prefix=csvis.split('/')[-1], dir=workingdir)

        # ms does not have a colnames method, so open vis with tb even though
        # it is already open with myms.  Note that both use nomodify=True,
        # however, and no problem was revealed in testing.
        mytb.open(vis, nomodify=True)
        if 'CORRECTED_DATA' in mytb.colnames():
            whichcol = 'CORRECTED_DATA'
        else:
            # DON'T remind the user that split before uvcontsub wastes time -
            # scratch columns will eventually go away.
            whichcol = 'DATA'
        mytb.close()

        casalog.post('Preparing to add scratch columns.')
        myfield = field
        if field == '':
            myfield = '*'
        if myfield != '*' and set(ms.msseltoindex(vis,
                               field=myfield)['field']) == set(ms.msseltoindex(vis,
                                                                 field='*')['field']):
            myfield = '*'
        if whichcol != 'DATA' or tempspw != '' or myfield != '*':
            casalog.post('splitting to ' + csvis + ' with spw="'
                         + tempspw + '"')
            myms.open(vis, nomodify=True)
            split_result = myms.split(csvis, spw=tempspw, field=myfield, whichcol=whichcol)
            myms.close()
            # jagonzal (CAS-4113): Take care of the trivial parallelization
            if not split_result:
                casalog.post("NullSelection: %s does not have data for spw=%s, field=%s, bailing out!" % (vis,tempspw,field),'SEVERE')
                shutil.rmtree(csvis)
                return False
        else:
            # This takes almost 30s/GB.  (lustre, 8/2011)
            casalog.post('Copying ' + vis + ' to ' + csvis + ' with cp.')
            copy_tree(vis, csvis)

        # It is less confusing if we write the history now that the "root" MS
        # is made, but before cb adds its messages.
        #
        # Not a dict, because we want to maintain the order.
        param_names = uvcontsub.func_code.co_varnames[:uvcontsub.func_code.co_argcount]
        param_vals = [eval(p) for p in param_names]
            
        write_history(myms, csvis, 'uvcontsub', param_names, param_vals,
                      casalog)

        if (type(csvis) == str) and os.path.isdir(csvis):
            mycb.open(csvis)
        else:
            raise Exception, 'Visibility data set not found - please verify the name'

        # select the data for continuum subtraction
        mycb.reset()
        mycb.selectvis(spw=myfitspw)

        # Set up the solve
        amuellertab = tempfile.mkdtemp(prefix='Temp_contsub.tab',
                                       dir=workingdir)

        mycb.setsolve(type='A', t=solint, table=amuellertab, combine=combine,
                      fitorder=fitorder)

        # solve for the continuum
        mycb.solve()

        # subtract the continuum
        mycb.selectvis(spw=myspw)
        aspwmap=-1
        # if we combined on spw in solve, fanout the result with spwmap=-999;
        if 'spw' in combine:
            aspwmap = -999
        mycb.setapply(table=amuellertab, spwmap=aspwmap)

        # Generate CORRECTED_DATA without continuum
        mycb.correct()

        if want_cont:
            # Generate MODEL_DATA with only the continuum model
            mycb.corrupt()

        mycb.close()

        # Delete the temporary caltable
        shutil.rmtree(amuellertab)

        # Move the line data from CORRECTED_DATA to DATA, and do any
        # final filtering by spw.
        myms.open(csvis)
        # Using ^ in spw is untested here!
        myms.split(final_csvis, spw=myspw, whichcol='corrected')
        myms.close()

        #casalog.post("\"want_cont\" = \"%s\"" % want_cont, 'WARN')
        #casalog.post("\"csvis\" = \"%s\"" % csvis, 'WARN')
        if want_cont:
            myms.open(csvis)
            # No channel averaging (== skipping for fitorder == 0) is done
            # here, but it would be a reasonable option.  The user can always
            # do it by running split again.
            myms.split(final_csvis[:-3],             # .contsub -> .cont
                       whichcol='MODEL_DATA',
                       spw=myspw)
            myms.close()

        #casalog.post("rming \"%s\"" % csvis, 'WARN')
        shutil.rmtree(csvis)
        
        # jagonzal (CAS-4113): We have to return a boolean so that we can identify 
        # the sub-MS that produce a continuum sub-MS to concatenate at the MMS level
        return True

    except Exception, instance:
        casalog.post('Error in uvcontsub: ' + str(instance), 'SEVERE')
        mycb.close()                        # Harmless if cb is closed.
        raise Exception


def _quantityRangesToChannels(vis,field,infitspw,excludechans):
    """
    private function to convert frequnecy (in the future, velocity
    as well) ranges in fitspw to channel indexes.
    For excludeechans=True, it will select channels outside given by infitspw
    returns: list containing new channel ranges
    """
    mytb.open(vis+'/SPECTRAL_WINDOW')
    nspw=mytb.nrows()
    mytb.close()
   
    fullspwids=str(range(nspw)).strip('[,]')
    tql={'field':field,'spw':fullspwids}
    myms.open(vis)
    myms.msselect(tql,True)
    allsels=myms.msselectedindices()
    myms.reset()  
    # input fitspw selection
    tql['spw']=infitspw
    myms.msselect(tql,True)
    usersels=myms.msselectedindices()['channel']
    myms.close()
    # sort the arrays so that chan ranges are
    # in order
    usersels.sort(axis=0)
    spwsels=''
    spwid=-1
    prevspwid=None
    newchanlist=[]
    nsels=len(usersels)
    #print "Usersels=",usersels
    if excludechans:
	for isel in range(nsels):
	    prevspwid = spwid
	    spwid=usersels[isel][0] 
	    lochan=usersels[isel][1]
	    hichan=usersels[isel][2]
	    stp=usersels[isel][3]
	    maxchanid=allsels['channel'][spwid][2]
	    # find left and right side ranges of the selected range
	    if spwid != prevspwid:
		# first line in the selected spw
		if lochan > 0:
		    outloL=0
		    outhiL=lochan-1
		    outloR= (0 if hichan+1>=maxchanid else hichan+1)
		    if outloR:
			if isel<nsels-1 and usersels[isel+1][0]==spwid:
			    outhiR=usersels[isel+1][1]-1
			else:
			    outhiR=maxchanid
		    else:
			outhiR=0 # higher end of the user selected range reaches maxchanid
				 # so no right hand side range
		    #print "outloL,outhiL,outloR,outhiR==", outloL,outhiL,outloR,outhiR
		else:
		    # no left hand side range
		    outloL=0
		    outhiL=0
		    outloR=hichan+1
		    if isel<nsels-1 and usersels[isel+1][0]==spwid:
			outhiR=usersels[isel+1][1]-1
		    else:
			outhiR=maxchanid
	    else:
		#expect the left side range is already taken care of
		outloL=0
		outhiL=0
		outloR=hichan+1
		if outloR>=maxchanid:
		    #No more boundaries to consider
		    outloR=0
		    outhiR=0
		else:
		    if isel<nsels-1 and usersels[isel+1][0]==spwid:
			outhiR=min(usersels[isel+1][1]-1,maxchanid)
		    else:
			outhiR=maxchanid
		    if outloR > outhiR:
			outloR = 0
			outhiR = 0

	#
	    if (not(outloL == 0 and outhiL == 0)) and outloL <= outhiL:
		newchanlist.append([spwid,outloL,outhiL,stp])
	    if (not(outloR == 0 and outhiR == 0)) and outloR <= outhiR:
		newchanlist.append([spwid,outloR,outhiR,stp])
	#print "newchanlist=",newchanlist
    else:
        # excludechans=False
        newchanlist=usersels

    #return newchanlist
    # create spw selection string from newchanlist
    spwstr=''
    for irange in range(len(newchanlist)):
        chanrange=newchanlist[irange]
        spwstr+=str(chanrange[0])+':'+str(chanrange[1])+'~'+str(chanrange[2])         
        if irange != len(newchanlist)-1:
            spwstr+=','
    return spwstr 
