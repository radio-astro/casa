import os
import sys
import shutil
import pprint as pp
import traceback
import time
import commands
import numpy as np
from __main__ import *
from taskinit import *


class convertToMMS():
    def __init__(self,\
                 inpdir=None, \
                 mmsdir=None, \
#                 parallel=False, \
                 axis='auto', \
                 createmslink=False, \
                 cleanup=False):

        '''Run the partition task to create MMSs from a directory with MSs'''
        casalog.origin('convertToMMS')

        self.inpdir = inpdir
        self.outdir = mmsdir
#        self.parallel = parallel
        self.axis = axis
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
        
        # Get non-MS directories and other files
        nonmslist = []
        nonmslist = self.getFileslist(files)

        casalog.post('List of other files in input directory')
        pp.pprint(nonmslist)
                    
    
        # Create an MMS for each MS in list
        for ms in mslist:
            casalog.post('Will create an MMS for '+ms)
            ret = self.runPartition(ms, self.mmsdir, self.createmslink, self.axis)
            if not ret:
                sys.exit(2)
            
            # Verify later if this is still needed
            time.sleep(10)
        
            casalog.origin('convertToMMS')
            casalog.post('--------------- Successfully created MMS')
                    
                
        # Create links to the other files
        for file in nonmslist:
            bfile = os.path.basename(file)
            lfile = os.path.join(self.mmsdir, bfile)
            casalog.post('Creating symbolic link to '+bfile)
            os.symlink(file, lfile)
            

    def getMSlist(self, files):
        '''Get a list of MSs from a directory.
           files -> a tuple that is returned by the following call:
           files = os.walk(self.inpdir,followlinks=True).next() 
           
           It will test if a directory is an MS and will only return
           true MSs, that have Type:Measurement Set in table.info. It will skip
           directories that start with . and those that do not end with
           extension .ms.
           '''
        
        topdir = files[0]
        mslist = []
        
        # Loop through list of directories
        for d in files[1]:
            # Skip . entries
            if d.startswith('.'):
                continue
            
            if not d.endswith('.ms'):
                continue
            
            # Full path for directory
            dir = os.path.join(topdir,d)
                        
            # It is probably an MS
            if self.isItMS(dir) == 1:                                                
                mslist.append(dir)
        
        return mslist

    def isItMS(self, dir):
        '''Check the type of a directory.
           dir  --> full path of a directory.
                Returns 1 for an MS, 2 for a cal table and 3 for a MMS.
                If 0 is returned, it means any other type or an error.'''
                
        ret = 0
        
        # Listing of this directory
        ldir = os.listdir(dir)
        
        if not ldir.__contains__('table.info'): 
            return ret
                
        cmd1 = 'grep Type '+dir+'/table.info'
        type = commands.getoutput(cmd1)
        cmd2 = 'grep SubType '+dir+'/table.info'
        stype = commands.getoutput(cmd2)
        
        # It is a cal table
        if type.__contains__('Calibration'):
            ret = 2
        
        elif type.__contains__('Measurement'):
            # It is a Multi-MS
            if stype.__contains__('CONCATENATED'):
                # Further check
                if ldir.__contains__('SUBMSS'):            
                    ret = 3
            # It is an MS
            else:
                ret = 1
            
        return ret
                        

    def getFileslist(self, files):
        '''Get a list of non-MS files from a directory.
           files -> a tuple that is returned by the following call:
           files = os.walk(self.inpdir,followlinks=True).next() 
           
           It will return files and directories that are not MSs. It will skip
           files that start with .
           '''
                
        topdir = files[0]
        fileslist = []
        
        # Get other directories that are not MSs
        for d in files[1]:
            
            # Skip . entries
            if d.startswith('.'):
                continue
            
            # Skip MS directories
            if d.endswith('.ms'):
                continue
            
            # Full path for directory
            dir = os.path.join(topdir,d)
            
            # It is a Calibration
            if self.isItMS(dir) == 2:
                fileslist.append(dir)


        # Get non-directory files        
        for f in files[2]:
            # Skip . entries
            if f.startswith('.'):
                continue
            
            # Full path for file
            file = os.path.join(topdir, f)
            fileslist.append(file)
            
        return fileslist


    def runPartition(self, ms, mmsdir, createlink, axis):
        '''Run partition with default values to create an MMS.
           ms         --> full pathname of the MS
           mmsdir     --> directory to save the MMS to
           createlink --> when True, it will create a symbolic link to the
                         just created MMS in the same directory with extension .ms  
           axis      --> separationaxis to use (spw, scan, auto)
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
        partition(vis=ms, outputvis=mms, createmms=True, datacolumn='all', flagbackup=False,
                  separationaxis=axis)
        casalog.origin('convertToMMS')
        
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
        print '          convertToMMS will create a directory with multi-MSs.'
        print 'Usage:\n'
        print '  import partitionhelper as ph'
        print '  ph.convertToMMS(inpdir=\'dir\') \n'
        print 'Options:'
        print '   inpdir <dir>        directory with input MS.'
        print '   mmsdir <dir>        directory to save output MMS. If not given, it will save '
        print '                       the MMS in a directory called mmsdir in the current directory.'
        print '   parallel=False      run partition in parallel or not.'
        print "   axis='auto'         separationaxis parameter of partition (spw,scan,auto)."
        print '   createmslink=False  if True it will create a link to the new MMS with extension .ms.'
        print '   cleanup=False       if True it will remove the output directory before starting.\n'
        
        print ' NOTE: this script will run using the default values of partition. It will try to '
        print ' create an MMS for every MS in the input directory. It will skip non-MS directories '
        print ' such as cal tables. If partition succeeds, the script will create a link to every '
        print ' other directory or file in the output directory. This script might fail if run on '
        print ' single dish MS because the datacolumn needs to be set in partition.\n'
        print ' The script will not walk through sub-directories of inpdir. It will also skip '
        print ' files or directories that start with a .'
        print '=========================================================================='
        return
        
#
# -------------- HELPER functions for dealing with an MMS --------------
#
#    getMMSScans        'Get the list of scans of an MMS dictionary'
#    getScanList        'Get the list of scans of an MS or MMS'
#    getScanNrows       'Get the number of rows of a scan in a MS. It will add the 
#                         nrows of all sub-scans.'
#    getMMSScanNrows    'Get the number of rows of a scan in an MMS dictionary.'
#    getSpwIds          'Get the Spw IDs of a scan.'
#    getDiskUsage       'eturn the size in bytes of an MS in disk.'
#
# ----------------------------------------------------------------------

# def getNumberOf(msfile, item='row'):
#     '''Using the msmd tool, it gets the number of
#        scan, spw, antenna, baseline, field, state,
#        channel, row in a MS or MMS'''
#     
#     md = msmdtool()
#     try:
#         md.open(msfile)
#     except:
#         print 'Cannot open the msfile'
#         return 0
#     
#     if item == 'row':
#         numof = md.nrows()
#     elif item == 'scan':
#         numof = md.nscans()
#     elif item == 'spw':
#         numof = md.nspw()
#     elif item == 'antenna':
#         numof = md.nantennas()
#     elif item == 'baseline':
#         numof = md.nbaselines()
#     elif item == 'channel':
#         numof = md.nchan()
#     elif item == 'field':
#         numof = md.nfields()
#     elif item == 'state':
#         numof = md.nstates()
#     else:
#         numof = 0
#         
#     md.close()
#     return numof
    
      
# NOTE
# There is a bug in ms.getscansummary() that does not give the scans for all 
# observation Ids, but only for the last one. See CAS-4409
def getMMSScans(mmsdict):
    '''Get the list of scans of an MMS dictionary.
       mmsdict  --> output dictionary from listpartition(MMS,createdict=true)
       Return a list of the scans in this MMS. '''
    
    if not isinstance(mmsdict, dict):
        print 'ERROR: Input is not a dictionary'
        return []
    
    tkeys = mmsdict.keys()
    scanlist = []
    slist = set(scanlist)
    for k in tkeys:
        skeys = mmsdict[k]['scanId'].keys()
        for j in skeys:
            slist.add(j)
    
    return list(slist)
    
def getScanList(msfile, selection={}):
    '''Get the list of scans of an MS or MMS. 
       msfile     --> name of MS or MMS
       selection  --> dictionary with data selection
       
       Return a list of the scans in this MS/MMS. '''
    
    msTool=mstool()
    msTool.open(msfile)
    if isinstance(selection, dict) and selection != {}:
        msTool.msselect(items=selection)
        
    scand = msTool.getscansummary()
    msTool.close()
        
    scanlist = scand.keys()
    
    return scanlist
    
    
def getScanNrows(msfile, myscan, selection={}):
    '''Get the number of rows of a scan in a MS. It will add the nrows of all sub-scans.
       This will not take into account any selection done on the MS.
       msfile     --> name of the MS or MMS
       myscan     --> scan ID (int)
       selection  --> dictionary with data selection
       
       Return the number of rows in the scan.
       
       To compare with the dictionary returned by listpartition, do the following:
       
        resdict = listpartition('file.mms', createdict=True)
        slist = ph.getMMSScans(thisdict)
        for s in slist:
            mmsN = ph.getMMSScanNrows(thisdict, s)
            msN = ph.getScanNrows('referenceMS', s)
            assert (mmsN == msN)
    '''
    msTool=mstool()
    msTool.open(msfile)
    if isinstance(selection, dict) and selection != {}:
        msTool.msselect(items=selection)
        
    scand = msTool.getscansummary()
    msTool.close()
    
    Nrows = 0
    if not scand.has_key(str(myscan)):
        return Nrows
    
    subscans = scand[str(myscan)]
    for ii in subscans.keys():
        Nrows += scand[str(myscan)][ii]['nRow']
    
    return Nrows


def getMMSScanNrows(thisdict, myscan):
    '''Get the number of rows of a scan in an MMS dictionary.
       thisdict  --> output dictionary from listpartition(MMS,createdict=true)
       myscan    --> scan ID (int) 
       Return the number of rows in the given scan. '''
    
    if not isinstance(thisdict, dict):
        print 'ERROR: Input is not a dictionary'
        return -1
    
    tkeys = thisdict.keys()
    scanrows = 0
    for k in tkeys:
        if thisdict[k]['scanId'].has_key(myscan):
            scanrows += thisdict[k]['scanId'][myscan]['nrows']
        
    return scanrows
   

def getSpwIds(msfile, myscan, selection={}):
    '''Get the Spw IDs of a scan. 
       msfile     --> name of the MS or MMS
       myscan     --> scan Id (int)
       selection  --> dictionary with data selection
       
       Return a list with the Spw IDs. Note that the returned spw IDs are sorted.
                           
    '''
    import numpy as np
    
    msTool=mstool()
    msTool.open(msfile)
    if isinstance(selection, dict) and selection != {}:
        msTool.msselect(items=selection)
        
    scand = msTool.getscansummary()
    msTool.close()
    
    spwlist = []

    if not scand.has_key(str(myscan)):
        return spwlist
    
    subscans = scand[str(myscan)]
    aspws = np.array([],dtype=int)
    
    for ii in subscans.keys():
        sscanid = ii
        spwids = scand[str(myscan)][sscanid]['SpwIds']
        aspws = np.append(aspws,spwids)
    
    # Sort spws  and remove duplicates
    aspws.sort()
    uniquespws = np.unique(aspws)
    
    # Try to return a list
    spwlist = uniquespws.ravel().tolist()
    return spwlist


def getScanSpwSummary(mslist=[]):
    """ Get a consolidated dictionary with scan, spw, channel information
       of a list of MSs. It adds the nrows of all sub-scans of a scan.
       
       Keyword arguments:
       mslist    --> list with names of MSs
       
       Returns a dictionary such as:
       mylist=['subms1.ms','subms2.ms']
       outdict = getScanSpwSummary(mylist)
       outdict = {0: {'MS': 'subms1.ms',
                      'scanId': {30: {'nchans': array([64, 64]),
                                      'nrows': 544,
                                      'spwIds': array([ 0,  1])}},
                      'size': '214M'},
                  1: {'MS': 'ngc5921.ms',
                      'scanId': {1: {'nchans': array([63]),
                                     'nrows': 4509,
                                     'spwIds': array([0])},
                                 2: {'nchans': array([63]),
                                     'nrows': 1890,
                                     'spwIds': array([0])}},
                      'size': '72M'}}
    """
                     
    if mslist == []:
        return {}

    mslocal1 = casac.ms()

    # Create lists for scan and spw dictionaries of each MS
    msscanlist = []
    msspwlist = []

    # List with sizes in bytes per sub-MS
    sizelist = []
    
    # Loop through all MSs
    for subms in mslist:
        try:
            mslocal1.open(subms)
            scans = mslocal1.getscansummary()
            msscanlist.append(scans)
            spws = mslocal1.getspectralwindowinfo()
            msspwlist.append(spws)
            mslocal1.close()
        except:
            mslocal1.close()
            raise Exception, 'Cannot get scan/spw information from subMS'

        # Get the data volume in bytes per sub-MS
        sizelist.append(getDiskUsage(subms))

    # Get the information to list in output
    # Dictionary to return
    outdict = {}

    for ims in range(mslist.__len__()):   
        # Create temp dictionary for each sub-MS
        tempdict = {}
        msname = os.path.basename(mslist[ims])
        tempdict['MS'] = msname
        tempdict['size'] = sizelist[ims]
        
        # Get scan dictionary for this sub-MS
        scandict = msscanlist[ims]
        
        # Get spw dictionary for this sub-MS
        # NOTE: the keys of spwdict.keys() are NOT the spw Ids
        spwdict = msspwlist[ims]
        
        # The keys are the scan numbers
        scanlist = scandict.keys()
        
        # Get information per scan
        tempdict['scanId'] = {}
        for scan in scanlist:
            newscandict = {}
            subscanlist = scandict[scan].keys()
            
            # Get spws and nrows per sub-scan
            nrows = 0
            aspws = np.array([],dtype='int32')
            for subscan in subscanlist:
                nrows += scandict[scan][subscan]['nRow']

                # Get the spws for each sub-scan
                spwids = scandict[scan][subscan]['SpwIds']
                aspws = np.append(aspws,spwids)

            newscandict['nrows'] = nrows

            # Sort spws  and remove duplicates
            aspws.sort()
            uniquespws = np.unique(aspws)
            newscandict['spwIds'] = uniquespws
                            
            # Array to hold channels
            charray = np.empty_like(uniquespws)
            spwsize = np.size(uniquespws)
            
            # Now get the number of channels per spw
            for ind in range(spwsize):
                spwid = uniquespws[ind]
                for sid in spwdict.keys():
                    if spwdict[sid]['SpectralWindowId'] == spwid:
                        nchans = spwdict[sid]['NumChan']
                        charray[ind] = nchans
                        continue
                
            newscandict['nchans'] = charray
            tempdict['scanId'][int(scan)] = newscandict
                
            
        outdict[ims] = tempdict
    #pprint.pprint(outdict)

    return outdict


def getMMSSpwIds(thisdict):
    '''Get the list of spws from an MMS dictionary.
       thisdict  --> output dictionary from listpartition(MMS,createdict=true)
       Return a list of the spw Ids in the dictionary. '''

    import numpy as np
    
    if not isinstance(thisdict, dict):
        print 'ERROR: Input is not a dictionary'
        return []
    
    tkeys = thisdict.keys()

    aspws = np.array([],dtype='int32')
    for k in tkeys:
        scanlist = thisdict[k]['scanId'].keys()
        for s in scanlist:
            spwids = thisdict[k]['scanId'][s]['spwIds']
            aspws = np.append(aspws, spwids)

    # Sort spws  and remove duplicates
    aspws.sort()
    uniquespws = np.unique(aspws)
    
    # Try to return a list
    spwlist = uniquespws.ravel().tolist()
        
    return spwlist

def getSubMSSpwIds(subms, thisdict):
    
    import numpy as np
    tkeys = thisdict.keys()
    aspws = np.array([],dtype='int32')
    mysubms = os.path.basename(subms)
    for k in tkeys:
        if thisdict[k]['MS'] == mysubms:
            # get the spwIds of this subMS
            scanlist = thisdict[k]['scanId'].keys()
            for s in scanlist:
                spwids = thisdict[k]['scanId'][s]['spwIds']
                aspws = np.append(aspws, spwids)
            break
                
    # Sort spws  and remove duplicates
    aspws.sort()
    uniquespws = np.unique(aspws)
    
    # Try to return a list
    spwlist = uniquespws.ravel().tolist()
    return spwlist

def getDiskUsage(msfile):
    """Return the size in bytes of an MS or MMS in disk.
    
    Keyword arguments:
       msfile  --> name of the MS
       This function will return a value given by the command du -hs
    """
    
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


def getSubtables(vis):
    tbTool = tbtool()
    theSubTables = []
    tbTool.open(vis)
    myKeyw = tbTool.getkeywords()
    tbTool.close()
    for k in myKeyw.keys():
        theKeyw = myKeyw[k]
        if (type(theKeyw)==str and theKeyw.split(' ')[0]=='Table:'
            and not k=='SORTED_TABLE'):
            theSubTables.append(os.path.basename(theKeyw.split(' ')[1]))
            
    return theSubTables


def makeMMS(outputvis, submslist, copysubtables=False, omitsubtables=[], parallelaxis=''):
    """Create a Multi-MS from a list of MSs
    
    Keyword arguments:
        outputvis        -- name of the output MMS
        submslist        -- list of input subMSs to create the output from
        copysubtables    -- True will copy the sub-tables from the first subMS to the others in the
                            output MMS. Default to False.
        omitsubtables    -- List of sub-tables to omit when copying to output MMS. They will be linked instead
        parallelasxis    -- Optionally, set the value to be written to AxisType in table.info of the output MMS
                            Usually this value comes from the separationaxis keyword of partition or mstransform.
                            
          Be AWARE that this function will remove the tables listed in submslist.
    """

    if os.path.exists(outputvis):
        raise ValueError, "Output MS already exists"

    if len(submslist)==0:
        raise ValueError, "No SubMSs given"

    ## make an MMS with all sub-MSs contained in a SUBMSS subdirectory
    origpath = os.getcwd()
    mymstool = mstool()
    mytbtool = tbtool()
    try:
        try:
            mymstool.createmultims(outputvis,
                                   submslist,
                                   [],
                                   True,  # nomodify
                                   False, # lock
                                   copysubtables,
                                   omitsubtables
                                   ) # when copying the subtables, omit these
        except:
            mymstool.close()
            raise
        mymstool.close()
        

        # remove the SORTED_TABLE keywords because the sorting is not reliable after partitioning
        try:
            mytbtool.open(outputvis, nomodify=False)
            if 'SORTED_TABLE' in mytbtool.keywordnames():
                mytbtool.removekeyword('SORTED_TABLE')
                mytbtool.close()
            for thesubms in submslist:
                mytbtool.open(outputvis+'/SUBMSS/'+os.path.basename(thesubms), nomodify=False)
                if 'SORTED_TABLE' in mytbtool.keywordnames():
                    tobedel = mytbtool.getkeyword('SORTED_TABLE').split(' ')[1]
                    mytbtool.removekeyword('SORTED_TABLE')
                    os.system('rm -rf '+tobedel)
                mytbtool.close()
        except:
            mytbtool.close()
            raise
            
        # finally create symbolic links to the subtables of the first SubMS
        os.chdir(origpath)
        os.chdir(outputvis)
        mastersubms = os.path.basename(submslist[0].rstrip('/'))
        thesubtables = getSubtables('SUBMSS/'+mastersubms)
        for s in thesubtables:
            os.symlink('SUBMSS/'+mastersubms+'/'+s, s)

        # AND put links for those subtables omitted
        os.chdir('SUBMSS/'+mastersubms)
        
        # Check if firtst subMS has POINTING and SYSCAL
        hasPointing = os.path.exists('POINTING')
        hasSyscal = os.path.exists('SYSCAL')

        for i in xrange(1,len(submslist)):
            thesubms = os.path.basename(submslist[i].rstrip('/'))
            os.chdir('../'+thesubms)
            for s in omitsubtables:
                if s == 'POINTING' and hasPointing == False:
                    continue
                if s == 'SYSCAL' and hasSyscal == False:
                    continue
                os.system('rm -rf '+s) # shutil does not work in the general case
                os.symlink('../'+mastersubms+'/'+s, s)
                
        # Write the AxisType info in the MMS
        if parallelaxis != '':
            setAxisType(outputvis, parallelaxis)

    except:
        theproblem = str(sys.exc_info())
        os.chdir(origpath)
        raise ValueError, "Problem in MMS creation: "+theproblem

    os.chdir(origpath)

    return True

def axisType(mmsname):
    """Get the axisType information from a Multi-MS. The AxisType information
       is usually added for Multi-MS with the axis which data is parallelized across.
       
       Keyword arguments:
           mmsname    --    name of the Multi-MS

        It returns the value of AxisType or an empty string if it doesn't exist.
    """
    tblocal = tbtool()
    
    axis = ''

    try:
        tblocal.open(mmsname, nomodify=True)
    except:
        raise ValueError, "Unable to open table %s" % mmsname
    
    tbinfo = tblocal.info()
    tblocal.close()
    
    if tbinfo.has_key('readme'):
        readme = tbinfo['readme']
        readlist = readme.splitlines()
        for val in readlist:
            if val.__contains__('AxisType'):
                a,b,axis = val.partition('=')
                
                
    return axis.strip()

def setAxisType(mmsname, axis=''):
    """Set the AxisType keyword in a Multi-MS info. If AxisType already
       exists, it will be overwritten.
    
    Keyword arguments:
        mmsname    --    name of the Multi-MS
        axis       --    parallel axis of the Multi-MS. Options: scan; spw or scan,spw
        
        Return True on success, False otherwise.
    """
    
    if axis == '':
        raise ValueError, "Axis value cannot be empty"
    
    tblocal = tbtool()
    try:
        tblocal.open(mmsname, nomodify=False)
    except:
        raise ValueError, "Unable to open table %s" % mmsname
    
    import copy

    tbinfo = tblocal.info()
    readme = ''
    # Save original readme
    if tbinfo.has_key('readme'):
        readme = tbinfo['readme']

    # Check if AxisType already exist and remove it
    if axisType(mmsname) != '':
        print 'WARN: Will overwrite the existing AxisType value'
        readlist = readme.splitlines()
        newlist = copy.deepcopy(readlist)
        for val in newlist:
            if val.__contains__('AxisType'):
                readlist.remove(val)

        # Recreate the string
        nr = ''
        for val in readlist:
            nr = nr + val + '\n'

        readme=nr.rstrip()
        
        
    # Preset for axis info
    axisInfo = "AxisType = "
    axis.rstrip()
    axisInfo = axisInfo + axis + '\n'
    
    # New readme
    newReadme = axisInfo + readme
    
    # Create readme record
    readmerec = {'readme':newReadme}
    
    tblocal.putinfo(readmerec)
    tblocal.close()
    
    # Check if the axis was correctly added
    check_axis = axisType(mmsname)

    if check_axis != axis:
        return False
 
    return True
    
    
    
    
    
    
    
    
    

