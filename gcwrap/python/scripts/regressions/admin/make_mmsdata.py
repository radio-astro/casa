# Run this script to create the MMS data needed to run the Python unit tests 
# of some tasks. This script will try to find the location of the data repository 
# based on the value of CASAPATH. The MSs will be read from the unittest directory 
# of the data repository. The output MMSs will be created in the local directory 
# under unittest_mms/<taskname>. The MMS directory will be removed if the command 
# is repeated for the same task.
# 
# Comment out the tests that you do not want to re-create MMSs for. 


import os
import sys
import partitionhelper as ph

def mmstest(mytask):
    # **** Data for test_listhistory.py
    TESTPATH = DATAPATH + 'unittest/'
    INPPATH = TESTPATH + mytask
    MMSPATH = './unittest_mms/'+mytask
    print '--------- Will create MMS data for test_'+mytask
    ph.convertToMMS(inpdir=INPPATH, mmsdir=MMSPATH, createmslink=True, cleanup=True)

      
# Location of the data repository
if not os.environ.has_key('CASAPATH'):
    print 'ERROR: Could not find variable CASAPATH'
    os._exit(2)
    
DATAPATH = os.environ.get('CASAPATH').split()[0] + '/data/regression/'


def main():
    # BEGINNING OF BLOCK THAT WILL CREATE MMS DATA. 
    # Comment out the tasks that you do not want to create data for.
    
    thislist = [
        #'listhistory',
        #'listvis',
        #'listobs',
        #'gencal',
        #'vishead',
        'split'
        ]

    
    # Loop through task list
    for t in thislist:
        mmstest(t)

    from tasks import partition

    if 'listvis' in thislist:
        # NOTE for test_listvis data:
        # You need to run partition by hand to create an MMS for the single-dish data set
        SDPATH = DATAPATH + 'unittest/listvis/'
        SDMMS = './unittest_mms/listvis/'
    
        partition(vis=SDPATH+'OrionS_rawACSmod', outputvis=SDMMS+'OrionS_rawACSmod.mms', datacolumn='float_data', createmms=True)

    if 'split' in thislist:
        # some additional MMSs
        SPLITMMSPATH = './unittest_mms/split/'
        specialcase = ['0420+417/0420+417.ms',
                       'viewertest/ctb80-vsm.ms',
                       'split/labelled_by_time+ichan.ms']
        for myms in specialcase:
            shutil.rmtree(SPLITMMSPATH+os.path.basename(myms), ignore_errors=True)
            partition(vis=DATAPATH+myms, outputvis=SPLITMMSPATH+os.path.basename(myms), datacolumn='all')

        # workaround for a partition shortcoming: column keywords not copied
        tb.open(SPLITMMSPATH+'hasfc.mms/SUBMSS/hasfc.0000.ms/', nomodify=False)
        tb.putcolkeyword('FLAG_CATEGORY','CATEGORY', ['FLAG_CMD', 'ORIGINAL', 'USER'])
        tb.close()
    
if __name__ == "__main__":
    main()
    
    
    
    
