import os
import numpy as np
import pprint
from taskinit import *
import partitionhelper as ph
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

    mslocal = casac.ms()
    mslocal1 = casac.ms()
            

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
        ismms = mslocal.ismultims()
        
        # List of MSs to process
        mslist = []
        
        # It is a multi-MS
        if ismms:
            casalog.post('This is a multi-MS')
            mslist = mslocal.getreferencedtables()
#            mslist.sort()
            sname = 'Sub-MS'
        else:
            mslist.append(vis)
            sname = 'MS'
            
        # Close top MS
        mslocal.close()
        
        # Create lists for scan and spw dictionaries of each sub-MS
        msscanlist = []
        msspwlist = []

        # List with sizes in bytes per sub-MS
        sizelist = []
        
        # Loop through all sub-Mss
        for subms in mslist:
            mslocal1.open(subms)
            scans = mslocal1.getscansummary()
            msscanlist.append(scans)
            spws = mslocal1.getspectralwindowinfo()
            msspwlist.append(spws)
            mslocal1.close()

            # Get the data volume in bytes per sub-MS
            sizelist.append(ph.getDiskUsage(subms))

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
#            pprint.pprint(outdict)


        # Now loop through the dictionary to print the information
        if outdict.keys() == []:
            casalog.post('Error in processing dictionaries','ERROR')
        
        indices = outdict.keys()
        indices.sort()
            
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
                NROWS = outdict[index]['scanId'][myscan]['nrows']
                
                # Get maximum widths
                smxw = getWidth(outdict, 'spw')
                cmxw = getWidth(outdict, 'channel')
                
                # Create format
                fhdr = '%-'+str(len(MS)+2)+'s' + '%-6s' + '%-'+str(smxw+2)+'s' + \
                        '%-'+str(cmxw+2)+'s' + '%-8s' + '%-6s'
    
                
                # Print header
                text = ''
                if counter == 0:
                    text = text + fhdr % (sname,'Scan','Spw','Nchan','Nrows','Size')  
                    text = text + '\n'                  
                counter += 1
                
                # Print first scan
                if firstscan:
                    text = text + fhdr % (MS, myscan, SPW, NCHAN, NROWS, SIZE)   
                else:
                    text = text + fhdr % ('', myscan, SPW, NCHAN, NROWS, '')
                
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
    if width < 5:
        width = 5
        
    return width










