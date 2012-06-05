import os
import numpy as np
import pprint
from taskinit import *
from parallel.parallel_task_helper import ParallelTaskHelper


def listpartition(vis=None, createdict=None, listfile=None):
    
    """List information about an MMS data set in the logger:

       Keyword arguments:
       vis -- Name of multi-MS or normal MS.
               default: none. example: vis='uidA002.mms'
       createdict -- Create and return a dictionary with information about
                     the sub-MSs.
               default: False
       listfile -- save the output to a file
             default: ''. Example: listfile="mylist.txt"
             
        The task also returns a dictionary with scan summary information
        for each sub-MS. 
                      

       """

    casalog.origin('listpartition')

    mslocal = casac.homefinder.find_home_by_name('msHome').create()
    mslocal1 = casac.homefinder.find_home_by_name('msHome').create()
            

    try:
        if (type(vis) == str) & os.path.exists(vis):
            mslocal.open(thems=vis)
        else:
            raise Exception, \
                'Visibility data set not found - please verify the name'

        # Check output filename existence 
        if listfile != '':
            if (type(listfile) == str) & os.path.exists(listfile):
                raise Exception, 'Output file \'%s\' already exists'%listfile
            
            casalog.post('Will save output to \'%s\''%listfile)
            ffout = open(listfile, 'w')
            
                
        # Is it a multi-MS?
        ismms = mslocal.ismultims(vis)
        
        # List of MSs to process
        mslist = []
        
        # It is a multi-MS
        if ismms:
            casalog.post('This is a multi-MS')
            mslist = mslocal.getreferencedtables()
            mslist.sort()
            sname = 'Sub-MS'
        else:
            mslist.append(vis)
            sname = 'MS'
            
        # Close top MS
        mslocal.close()
        
        # Create lists for scan and spw dictionaries of each sub-MS
        scanlist = []
        spwlist = []

        # List with sizes in bytes per sub-MS
        sizelist = []
        
        # Loop through all sub-Mss
        for subms in mslist:
            mslocal1.open(subms)
            scans = mslocal1.getscansummary()
            scanlist.append(scans)
            spws = mslocal1.getspectralwindowinfo()
            spwlist.append(spws)
            mslocal1.close()

            # Get the data volume in bytes per sub-MS
            sizelist.append(getDiskUsage(subms))

            
        # Get the width for printing the output     
        # MS name width
#        mn = os.path.basename(mslist[0])
#        ml = mn.__len__() + 2
#        mnw = '%-'+ str(ml) +'s'
#        
#        # Scan list width        
#        swidth = getMaxWidth(scanlist, 'scan') + 2
#        swd = '%-'+str(swidth)+'s '
#        
#        # SPW list width        
#        pwidth = getMaxWidth(scanlist, 'scan-spw') + 2
#        pwd = '%-'+str(pwidth)+'s '
#        
#        # Channel list width
#        cwidth = getMaxWidth(spwlist, 'channel') + 2
#        cwd = '%-'+str(cwidth)+'s '
#        
#        # Print header
#        fheader = mnw+swd+pwd+cwd+'%-6s '+'%-5s '
#        ftext = mnw+swd+pwd+cwd+'%-6d '+'%-5s '
#
#        hdr = fheader % (sname, 'Scan', 'SPW', 'NChan', 'nRow', 'Size')
        
#        if listfile != '':
#            print >> ffout, hdr
#        else:
#            casalog.post(hdr)        

        # Get the information to list in output
        # Dictionary to return
        outdict = {}

        for subms in range(mslist.__len__()):   
            
            # Create temp dictionary for each sub-MS
            tempdict = {}
            msname = os.path.basename(mslist[subms])
            tempdict['MS'] = msname
            tempdict['size'] = sizelist[subms]
            
            # Get scan dictionary for this sub-MS
            scandict = scanlist[subms]
            
            # Get spw dictionary for this sub-MS
            spwdict = spwlist[subms]
            
            # The keys are the scan numbers
            scans = scandict['summary'].keys()
            
            # Get information per scan
            tempdict['scanId'] = {}
            for ii in scans:
                scanid = ii
                newscandict = {}

                sscans = scandict['summary'][scanid].keys()
                
                # Get spws and nrows per sub-scan
                nrows = 0
                aspws = np.array([],dtype=int)
                for kk in sscans:
                    sscanid = kk
                    nrows += scandict['summary'][scanid][sscanid]['nRow']

                    # Get the spws for each sub-scan
                    spwids = scandict['summary'][scanid][sscanid]['SpwIds']
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
                    nchans = spwdict['spwInfo'][str(spwid)]['NumChan']
                    charray[ind] = nchans
                    
                newscandict['nchans'] = charray
                tempdict['scanId'][int(scanid)] = newscandict
                    
                
            outdict[subms] = tempdict
#            pprint.pprint(outdict)


        # Now loop through the dictionary to print the information
        if outdict.keys() == []:
            casalog.post('Error in processing dictionaries','ERROR')
        
        indices = outdict.keys()
            
        counter = 0
        for index in indices:
            
            # Get data
            MS = outdict[index]['MS']
            SIZE = outdict[index]['size']
            SCAN = outdict[index]['scanId']
                        
            # Sort scans for more optimal printing
            # Print information per scan
            firstscan = True
            skeys = SCAN.keys()
            skeys.sort()
            for myscan in skeys:
                SPW = outdict[index]['scanId'][myscan]['spwIds']
                NCHAN = outdict[index]['scanId'][myscan]['nchans']
                
                # Get maximum widths
                smxw = getWidth(outdict, 'spw')
                cmxw = getWidth(outdict, 'channel')
                
                # Create format
                fhdr = '%-'+str(len(MS)+2)+'s' + '%-6s' + '%-'+str(smxw+2)+'s' + '%-'+str(cmxw+2)+'s' + '%-6s'
    
                
                # Print header
                text = ''
                if counter == 0:
                    text = text + fhdr % (sname,'Scan','Spw','Nchan','Size')  
                    text = text + '\n'                  
                counter += 1
                
                # Print first scan
                if firstscan:
                    text = text + fhdr % (MS, myscan, SPW, NCHAN, SIZE)   
                else:
                    text = text + fhdr % ('', myscan, SPW, NCHAN, '')
                
                firstscan = False            

                # Print to a file
                if listfile != '':
                    print >> ffout, text
                else:
                    # Print to the logger
                    casalog.post(text)
                                
                
        if listfile != '':    
            ffout.close()
                                        
     
        # Return the scan dictionary
        if createdict:
            return outdict
        
        return {}
            
    except Exception, instance:
#        mslocal.close()
        print '*** Error ***', instance
    

def getMaxWidth(listdict, par):
    '''From a list of dictionaries, return the maximum width of a string.
       The dictionaries are either taken from ms.getspectralwindow() or
       ms.getscansummary(). This function will compare the lists of scans or
       spws in each dictionary and return the largest width of a string.
       
       listdict -->  list with dictionaries
       par      --> 'scan', 'spw' or 'channel'
       Example:
       listdict = 
    '''
    
    max_width = 0
    
    for dict in range(listdict.__len__()):
        if par == 'scan':
            ss = listdict[dict]['summary'].keys()
            
            # convert strings to integers
            for i in range(ss.__len__()):
                ss[i] = int(ss[i])
            sl = str(ss).__len__()
            if sl > max_width:
                max_width = sl
                
        elif par == 'scan-spw':
            ss = listdict[dict]['summary'].keys()
            
            # convert strings to integers
            for i in range(ss.__len__()):
                sss = listdict[dict]['summary'][i].keys()
                
                for subscan in sss:
                    spwids = listdict[dict]['summary'][i][subscan]['SpwIds']
                    for id in spwids:
                        if id > max_width:
                            max_width = id            
                
        elif par == 'spw':
            spws = listdict[dict]['spwInfo'].keys()
            for j in range(spws.__len__()):
                spws[j] = int(spws[j])
                
            pl = str(spws).__len__()
            if pl > max_width:
                max_width = pl
                
        elif par == 'channel':
            chlist = []
            spws = listdict[dict]['spwInfo'].keys()
            for j in range(spws.__len__()):
                spwid = spws[j]
                nchans = listdict[dict]['spwInfo'][spwid]['NumChan']            
                chlist.append(nchans)
                
            cl = str(chlist).__len__()
            if cl > max_width:
                max_width = cl
               
                
    return max_width
            

def getDiskUsage(msfile):
    '''Return the size in bytes of an MS'''
    
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


def sortedDictValues(adict):
    '''Sort the keys of a dictionary coming from
       ms.getscansummary'''
    
    items = adict.items()
    items.sort()
    return [value for key, value in items]


def getWidth(adict, par):
    
    width = 0
    for aa in adict.keys():
        scans = adict[aa]['scanId'].keys()
        for bb in scans:
            if par == 'spw':
                spws = adict[aa]['scanId'][bb]['spwIds']
                mystr = str(spws)
                length = len(mystr)
                if length > width:
                    width = length
                    
            elif par == 'channel':
                chans = adict[aa]['scanId'][bb]['nchans']
                mystr = str(chans)
                length = len(mystr)
                if length > width:
                    width = length
    
    return width


def sortScandict(adict):
    '''Sort a given dictionary'''
    
    keys = adict.keys()
    keys.sort()
        
    return [dict[key] for key in keys]











