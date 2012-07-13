import os
import sys
import shutil
import string
from taskinit import *
import numpy as np


def getScanNrows(msfile, myscan):
    '''Get the number of rows of a scan in a MS. It will add the nrows of all sub-scans.
       msfile  --> name of the MS
       myscan  --> scanID
       return the number of rows in the scan.
       
       To compare with the dictionary returned by listpartition, do the following:
        
        resdict = listpartition(vis='part.mms', createdict=True)
        for k in resdict.keys():
            subms = resdict[k]['MS']
            MS = 'part.mms/SUBMSS/'+subms
            scans = resdict[k]['scanId'].keys()
            for s in scans:
                nr = resdict[k]['scanId'][s]['nrows']
                refN = getScanNrows(MS,s)
                assert nr==refN, '%s, scan=%s, nrows=%s do not match reference nrows=%s'%(MS, s, nr, refN)

    '''

    msTool=mstool()

    msTool.open(msfile)
    scand = msTool.getscansummary()
    msTool.close()
    
    Nrows = 0
    subscans = scand[str(myscan)]
    for ii in subscans.keys():
        Nrows += scand[str(myscan)][ii]['nRow']
    
    return Nrows


def getDiskUsage(msfile):
    '''Return the size in bytes of an MS in disk.
       msfile  --> name of the MS
       This function will return a value given by
       the command du -hs'''
    
    from subprocess import Popen, PIPE, STDOUT

    # Command line to run
    ducmd = 'du -hs '+msfile
    
    p = Popen(ducmd, shell=True, stdin=PIPE, stdout=PIPE, stderr=STDOUT, close_fds=True)
    
    sizeline = p.stdout.read()
    
    # Create a list of the output string, which looks like this:
    # ' 75M\tuidScan23.data/uidScan23.0000.ms\n'
    # This will create a list with [size,sub-ms]
    mssize = sizeline.split()

    return mssize[0]

def getSpwIds(msfile, myscan):
    '''Get the Spw IDs of a scan. 
       msfile  --> name of the MS
       myscan  --> scanID
       return a numpy array with the Spw IDs. Note that the returned spw IDs are sorted.
       
       To compare with the dictionary returned by listpartition, do the following:
       
        resdict = listpartition(vis='part.mms', createdict=True)
        for k in resdict.keys():
            subms = resdict[k]['MS']
            MS = 'part.mms/SUBMSS/'+subms
            scans = resdict[k]['scanId'].keys()
            for s in scans:
                spws = resdict[k]['scanId'][s]['spwIds']
                refspws = getSpwIds(MS,s)
                
                # Compare the two arrays
                compspws = np.in1d(refspws, spws)
                indices = np.where(compspws==False)
                if np.size(indices) != 0:
                    for i in indices:
                        print "Spw=%s is not in reference array %s"%(spws[indices[i]], refspws)
                    
    '''

    msTool=mstool()
            
    msTool.open(msfile)
    scand = msTool.getscansummary()
    msTool.close()
    
    subscans = scand[str(myscan)]
    aspws = np.array([],dtype=int)
    
    for ii in subscans.keys():
        sscanid = ii
        spwids = scand[str(myscan)][sscanid]['SpwIds']
        aspws = np.append(aspws,spwids)
    
    # Sort spws  and remove duplicates
    aspws.sort()
    uniquespws = np.unique(aspws)
    
    return uniquespws


def getSubtables(vis):
    tbTool = tbtool()
    theSubTables = []
    tbTool.open(vis)
    myKeyw = tbTool.getkeywords()
    tbTool.close()
    for k in myKeyw.keys():
        theKeyw = myKeyw[k]
        if (type(theKeyw)==str and theKeyw.split(' ')[0]=='Table:'
            and not theKeyw=='SORTED_TABLE'):
            theSubTables.append(os.path.basename(theKeyw.split(' ')[1]))
            
    return theSubTables


def makeMMS(outputvis, submslist, tmpmsname=''):
    '''
    Create a MMS named outputvis from the submss in list submslist.
    All submss must be in the same directory and that directory
    must contain nothing else
    '''

    if os.path.exists(outputvis):
        raise ValueError, "Output MS already exists"

    if len(submslist)==0:
        raise ValueError, "No SubMSs given"

    ## make an MMS with all sub-MSs contained in a SUBMSS subdirectory
    thems = outputvis
    themsname = os.path.basename(thems.rstrip('/'))
    thecontainingdir = os.path.dirname(thems.rstrip('/'))
    thesubmscontainingdir = os.path.dirname(submslist[0].rstrip('/'))

    os.mkdir(thems)
    shutil.move(thesubmscontainingdir, thems+'/SUBMSS')

    thesubmss = []
    for submsname in submslist:
        thesubmsname = os.path.basename(submsname.rstrip('/'))
        thesubmss.append(themsname+'/SUBMSS/'+thesubmsname)


    ## create the MMS via a temporary directory in order to be able
    ## to have the members inside the outputvis
            
    origpath = os.getcwd()

    # need to be in the containing directory of outputvis in order
    # to have the right relative paths in the MMS header
    if not (thecontainingdir==''):
        os.chdir(thecontainingdir)

    if tmpmsname=='':
        tmpmsname = themsname+'_createmms_tmp'
    shutil.rmtree(tmpmsname, ignore_errors=True)

    try:
        mymstool = mstool()
        mymstool.createmultims(tmpmsname,
                               thesubmss,
                               [],
                               True,  # nomodify
                               False, # lock
                               False) # copysubtables
        shutil.move(thems+'/SUBMSS', tmpmsname)
        shutil.rmtree(thems, ignore_errors=True)
        shutil.move(tmpmsname, thems)

        # finally create symbolic links to the subtables of the first SubMS
        os.chdir(origpath)
        os.chdir(thems)
        mastersubms = os.path.basename(submslist[0].rstrip('/'))
        thesubtables = getSubtables('SUBMSS/'+mastersubms)
        for s in thesubtables:
            os.symlink('SUBMSS/'+mastersubms+'/'+s, s)

    except:
        os.chdir(origpath)
        raise ValueError, "Problem in MMS creation: "+sys.exc_info()[0] 

    os.chdir(origpath)

    return True
