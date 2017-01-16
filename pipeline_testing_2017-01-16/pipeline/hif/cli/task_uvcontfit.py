import os

# Leave in place for the channel range computation.
# should that be needed.

import numpy as np
import re

from taskinit import *
from update_spw import *

# Get the tools
#    ms and table tools are used by the ranges to channel converter
#    no leave in place for now
mycb, myms, mytb = gentools(['cb', 'ms', 'tb'])

# Notes
#    It is not clear what field is for. It is not selected for in the
#    caltable computation, but it is used to computed channel ranges
#    Leave in place for now

def uvcontfit (vis=None, caltable=None, field=None, intent=None, spw=None,
    combine=None, solint=None, fitorder=None, append=None):
    
    # Python script
    casalog.origin('uvcontfit')

    # Run normal code
    try:

        # Determine the channels to be used in the fit
        #    Not sure why this was needed but leave code in place for now. What is wrong with frequency ranges ?
        #if spw.count('Hz'):
            #locfitspw = _new_quantityRangesToChannels(vis,field,fitspw,False)
        #else:
            #locfitspw=spw
        locfitspw = spw

        if (type(vis) == str) and os.path.isdir(vis):
            mycb.open(filename=vis, compress=False, addcorr=False, addmodel=False)
        else:
            raise Exception, 'Visibility data set not found - please verify the name'

        # Select the data for continuum subtraction
        #   Intent forces the selection to be on TARGET data only
        #   Field is needed because the continuum regions will be different for different target fields
        #   Spw selection will include an spw and frequency change   
        mycb.reset()
        mycb.selectvis(field=field, intent=intent, spw=locfitspw)

        # Add append parameter because it may be needed to deal with data sets with multiple
        # targets.
        if not combine:
            mycombine = ''
        else:
            mycombine = combine
        mycb.setsolve(type='A', t=solint, table=caltable, combine=mycombine,
            fitorder=fitorder, append=append)

        # Solve for the continuum
        mycb.solve()

        mycb.close()

    except Exception, instance:
        casalog.post('Error in uvcontfit: ' + str(instance), 'SEVERE')
        mycb.close()                        # Harmless if cb is closed.
        raise Exception, 'Error in uvcontfit: ' + str(instance)


def _new_quantityRangesToChannels (vis, field, infitspw, excludechans):

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
    usersels=usersels[np.lexsort((usersels[:,1],usersels[:,0]))]
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
