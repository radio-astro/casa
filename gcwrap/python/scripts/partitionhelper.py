import os
import sys
import shutil
import numpy as np
import pprint as pp
import traceback
import time
from __main__ import *
from taskinit import *
#from tasks import *


class createMMS():
    def __init__(self,\
                 inpdir=None, \
                 mmsdir=None, \
                 createmslink=False, \
                 cleanup=False):

        '''Run the partition task to create MMSs from a directory with MSs'''
        casalog.origin('createMMS')
        
        self.inpdir = inpdir
        self.outdir = mmsdir
        self. createmslink = createmslink
        self.mmsdir = '/tmp/mmsdir'
        self.cleanup = cleanup        
        
        # Input directory is mandatory
        if self.inpdir is None:
            casalog.post('You must give an input directory to this script') 
            self.usage()
            return
            
        if not os.path.exists(self.inpdir):
            casalog.post('Input directory inpdir does not exist -> '+self.inpdir,'ERROR') 
            self.usage()
            return
        
        if not os.path.isdir(self.inpdir):                            
            casalog.post('Value of inpdir is not a directory -> '+self.inpdir,'ERROR') 
            self.usage()
            return


        # Only work with absolute paths
        self.inpdir = os.path.abspath(self.inpdir)
        casalog.post('Will read input MS from '+self.inpdir)

        # Verify output directory
        if self.outdir is None:
            self.mmsdir = os.path.join(os.getcwd(),'mmsdir')
        elif self.outdir == '/':
            casalog.post('inpdir is set to root!', 'WARN')
            self.mmsdir = os.path.join(os.getcwd(),'mmsdir')
        else:
            self.outdir = os.path.abspath(self.outdir)
            self.mmsdir = self.outdir

        # Cleanup output directory
        if self.cleanup:
            casalog.post('Cleaning up output directory '+self.mmsdir)
            if os.path.isdir(self.mmsdir):
                shutil.rmtree(self.mmsdir)
        
        if not os.path.exists(self.mmsdir):
            os.makedirs(self.mmsdir)
            
        
        casalog.post('Will save output MMS to '+self.mmsdir)

        # Walk through input directory
        files = os.walk(self.inpdir,followlinks=True).next()

        # Get MS list
        mslist = []
        mslist = self.getMSlist(files)
                        
        casalog.post('List of MSs in input directory')
        pp.pprint(mslist)        
        
        # Get non-directory entries
        nonmslist = []
        nonmslist = self.getFileslist(files)

        casalog.post('List of other files in input directory')
        pp.pprint(nonmslist)
                    
    
        # Create an MMS for each MS in list
        for ms in mslist:
            casalog.post('Will create an MMS for '+ms)
            ret = self.runPartition(ms, self.mmsdir, self.createmslink)
            if not ret:
                sys.exit(2)
            
            # Verify later if this is still needed
            time.sleep(10)
        
            casalog.origin('createMMS')
            casalog.post('--------------- Successfully created MMS')
                    
                
        # Create links to the other files
        for file in nonmslist:
            casalog.post('Creating symbolic links to other files')
            bfile = os.path.basename(file)
            lfile = os.path.join(self.mmsdir, bfile)
            os.symlink(file, lfile)
            
            


    def getMSlist(self, files):
        '''Get a list of MSs from a directory.
           files -> a tuple that is returned by the following call:
           files = os.walk(self.inpdir,followlinks=True).next() 
           
           It will test if a directory is an MS and will only return
           true MSs, that contain the file table.data. It will skip
           directories that start with .
           '''
        
        topdir = files[0]
        mslist = []
        
        # Loop through list of directories
        for d in files[1]:
            # Skip . entries
            if d.startswith('.'):
                continue
            
            # Full path for directory
            dir = os.path.join(topdir,d)
            
            # Listing of this directory
            ldir = os.listdir(dir)
            
            # Skip MMSs
            if ldir.__contains__('SUBMSS'):
                continue
                
            # It is not an MS, go to next file
            if not ldir.__contains__('table.dat'):
                continue
            
            # It is probably an MS
            mslist.append(dir)
        
        return mslist

    def getFileslist(self, files):
        '''Get a list of non-MS files from a directory.
           files -> a tuple that is returned by the following call:
           files = os.walk(self.inpdir,followlinks=True).next() 
           
           It will only return files, not directories. It will skip
           files that start with .
           '''
                
        topdir = files[0]
        fileslist = []
        
        for f in files[2]:
            # Skip . entries
            if f.startswith('.'):
                continue
            
            # Full path for file
            file = os.path.join(topdir, f)
            fileslist.append(file)
            
        return fileslist


    def runPartition(self, ms, mmsdir, createlink):
        '''Run partition with default values to create an MMS.
           ms         --> full pathname of the MS
           mmsdir     --> directory to save the MMS to
           createlink --> when True, it will create a symbolic link to the
                         just created MMS in the same directory with extension .ms        
        '''
        from tasks import partition

        if not os.path.lexists(ms):
            return False
        
        # Create MMS name
        bname = os.path.basename(ms)
        if bname.endswith('.ms'):
            mmsname = bname.replace('.ms','.mms')
        else:
            mmsname = bname+'.mms'
        
        mms = os.path.join(self.mmsdir, mmsname)
        if os.path.lexists(mms):
            casalog.post('Output MMS already exist -->'+mms,'ERROR')
            return False
        
        # Check for remainings of corrupted mms
        corrupted = mms.replace('.mms','.data')
        if os.path.exists(corrupted):
            casalog.post('Cleaning up left overs','WARN')
            shutil.rmtree(corrupted)
        
        # Run partition   
        default('partition')
        partition(vis=ms, outputvis=mms, createmms=True)
        casalog.origin('createMMS')
        
        # Check if MMS was created
        if not os.path.exists(mms):
            casalog.post('Cannot create MMS ->'+mms, 'ERROR')
            return False
        
        # If requested, create a link to this MMS
        if createlink:
            here = os.getcwd()
            os.chdir(mmsdir)
            mmsname = os.path.basename(mms)
            lms = mmsname.replace('.mms', '.ms')
            casalog.post('Creating symbolic link to MMS')
            os.symlink(mmsname, lms)
            os.chdir(here)
                
        return True
        
    def usage(self):
        print '========================================================================='
        print '          createMMS will create a directory with multi-MSs.'
        print 'Usage:\n'
        print '  import partitionhelper as ph'
        print '  ph.createMMS(inpdir=\'dir\') \n'
        print 'Options:'
        print '   inpdir <dir>        directory with input MS.'
        print '   mmsdir <dir>        directory to save output MMS. If not given, it will save '
        print '                       the MMS in a directory called mmsdir in the current directory.'
        print '   createmslink=False  if True it will create a link to the MMS with extension .ms.'
        print '   cleanup=False       if True it will remove the output directory before starting.\n'
        print ' NOTE: this script will run using the default values of partition. It will fail if '
        print ' run on single dish data because the datacolumn needs to be set in partition.\n'
        print ' The script will not walk through sub-directories of inpdir. It will also skip '
        print ' files or directories that start with a .'
        print '=========================================================================='
        return
        
        
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


def makeMMS(outputvis, submslist, copysubtables=False):
    '''
    Create an MMS named outputvis from the submss in list submslist.
    '''

    if os.path.exists(outputvis):
        raise ValueError, "Output MS already exists"

    if len(submslist)==0:
        raise ValueError, "No SubMSs given"

    ## make an MMS with all sub-MSs contained in a SUBMSS subdirectory
    origpath = os.getcwd()

    try:
        mymstool = mstool()
        mymstool.createmultims(outputvis,
                               submslist,
                               [],
                               True,  # nomodify
                               False, # lock
                               copysubtables) # copysubtables
        mymstool.close()
        # finally create symbolic links to the subtables of the first SubMS
        os.chdir(origpath)
        os.chdir(outputvis)
        mastersubms = os.path.basename(submslist[0].rstrip('/'))
        thesubtables = getSubtables('SUBMSS/'+mastersubms)
        for s in thesubtables:
            os.symlink('SUBMSS/'+mastersubms+'/'+s, s)

    except:
        os.chdir(origpath)
        raise ValueError, "Problem in MMS creation: "+sys.exc_info()[0] 

    os.chdir(origpath)

    return True



