import os
from taskinit import *


def listpartition(vis=None, listfile=None):
    
    """List information about an MMS data set in the logger:

       Keyword arguments:
       vis -- Name of multi-MS or normal MS.
               default: none. example: vis='uidA002.mms'
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
            sname = 'Sub-MS'
        else:
            mslist.append(vis)
            sname = 'MS'
            
        # Close top MS
        mslocal.close()
        
        # Create lists for scan and spw dictionaries of each sub-MS
        scanlist = []
        spwlist = []
        
        # Loop through all sub-Mss
        for subms in mslist:
            mslocal1.open(subms)
            scans = mslocal1.getscansummary()
            scanlist.append(scans)
            spws = mslocal1.getspectralwindowinfo()
            spwlist.append(spws)
            mslocal1.close()
            
        # Get the width for printing the output        
        # MS name width
        mn = os.path.basename(mslist[0])
        ml = mn.__len__() + 2
        mnw = '%-'+ str(ml) +'s'
        
        # Scan list width
        sd = scanlist[0]
        ss = sd['summary'].keys()
        sl = str(ss).__len__() + 2
        swd = '%-'+str(sl)+'s '
        
        # SPW list width
        pd = spwlist[0]
        pp = pd['spwInfo'].keys()
        pl = str(pp).__len__() + 2
        pwd = '%-'+str(pl)+'s '
        
        # Print header
        fheader = mnw+swd+pwd+pwd+'%-6s '
        ftext = mnw+swd+pwd+pwd+'%-6d '

        hdr = fheader % (sname, 'Scan', 'SPW', 'NChan', 'nRow')
        
        if listfile != '':
            print >> ffout, hdr
        else:
            casalog.post(hdr)        
            casalog.post()

        # Get the information to list in output
        # Dictionary to return
        outdict = {}

        for i in range(mslist.__len__()):   
            # Get scans
            scandict = scanlist[i]
            scans = scandict['summary'].keys()
            
            # Get the total nRows per scan
            nrows = 0
            for ii in scans:
                sscans = scandict['summary'][ii].keys()
                for kk in sscans:
                    nrows += scandict['summary'][ii][kk]['nRow']
            
            # Add the MS name in dictionary
            scandict['MS'] = mslist[i]

            # Add and index
            outdict[str(i)] = scandict
            
            # Get spws
            spwdict = spwlist[i]
            spws = spwdict['spwInfo'].keys()
            
            # Get channels and nRows per spw
            chlist = []
            for j in spws:
                nchans = spwdict['spwInfo'][j]['NumChan']
                chlist.append(nchans)
                
            msname = os.path.basename(mslist[i])
            text = ftext % (msname, scans, spws, chlist, nrows)
            
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
    



