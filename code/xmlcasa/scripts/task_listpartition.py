import os
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
        mn = os.path.basename(mslist[0])
        ml = mn.__len__() + 2
        mnw = '%-'+ str(ml) +'s'
        
        # Scan list width        
        swidth = getMaxWidth(scanlist, 'scan') + 2
        swd = '%-'+str(swidth)+'s '
        
        # SPW list width        
        pwidth = getMaxWidth(spwlist, 'spw') + 2
        pwd = '%-'+str(pwidth)+'s '
        
        # Channel list width
        cwidth = getMaxWidth(spwlist, 'channel') + 2
        cwd = '%-'+str(cwidth)+'s '
        
        # Print header
        fheader = mnw+swd+pwd+cwd+'%-6s '+'%-5s '
        ftext = mnw+swd+pwd+cwd+'%-6d '+'%-5s '

        hdr = fheader % (sname, 'Scan', 'SPW', 'NChan', 'nRow', 'Size')
        
        if listfile != '':
            print >> ffout, hdr
        else:
            casalog.post(hdr)        

        # Get the information to list in output
        # Dictionary to return
        outdict = {}

        for i in range(mslist.__len__()):   
            # Get scans
            scandict = scanlist[i]
            
            # The keys are the scan numbers
            scans = scandict['summary'].keys()
            
            # Get the total nRows per scan
            nrows = 0
            for ii in scans:
                sscans = scandict['summary'][ii].keys()
                for kk in sscans:
                    nrows += scandict['summary'][ii][kk]['nRow']

            # Convert string scans to integer to take less space
            # when printing on the screen
            for index in range(scans.__len__()):
                scans[index] = int(scans[index])            

            # Add and index and the MS name to the output dictionary
            if createdict:
                scandict['MS'] = mslist[i]
                outdict[str(i)] = scandict
            
            # Get spws
            spwdict = spwlist[i]
            spws = spwdict['spwInfo'].keys()
            
            # Get the number of channels per spw
            chlist = []
            for j in range(spws.__len__()):
                spwid = spws[j]
                nchans = spwdict['spwInfo'][spwid]['NumChan']
                chlist.append(nchans)
                
                # Convert string spws to integer to take less space
                # when printing on the screen
                spws[j] = int(spws[j])
                
                
            # Create the text to print
            msname = os.path.basename(mslist[i])
            text = ftext % (msname, scans, spws, chlist, nrows, sizelist[i])
            
            # Print to a file
            if listfile != '':
                print >> ffout, text
            else:
                # Print to the logger
                casalog.post(text)
                    
        if listfile != '':    
            ffout.close()
    
        # Return the scan dictionary
        return outdict
            
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








