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

def mmstest(taskname):
    # **** Data for test_listhistory.py
    TESTPATH = DATAPATH + 'unittest/'
    INPPATH = TESTPATH + taskname
    MMSPATH = './unittest_mms/'+taskname
    print '--------- Will create MMS data for test_'+taskname
    ph.convertToMMS(inpdir=INPPATH, mmsdir=MMSPATH, createmslink=True, cleanup=True)

      
# Location of the data repository
if not os.environ.has_key('CASAPATH'):
    print 'ERROR: Could not find variable CASAPATH'
    os._exit(2)
    
DATAPATH = os.environ.get('CASAPATH').split()[0] + '/data/regression/'

    
# BEGINNING OF BLOCK THAT WILL CREATE MMS DATA

# **** Data for test_listhistory.py
tasktest = 'listhistory'
mmstest(tasktest)

# **** Data for test_listvis.py
tasktest = 'listvis'
mmstest(tasktest)
# NOTE:
# You need to run partition by hand to create an MMS for the single-dish data set
SDPATH = DATAPATH + 'unittest/listvis/'
SDMMS = './unittest_mms/listvis/'
partition(vis=SDPATH+'OrionS_rawACSmod', outputvis=SDMMS+'OrionS_rawACSmod.mms', datacolumn='float_data', createmms=True)

# **** Data for test_listobs.py
tasktest = 'listobs'
mmstest(tasktest)
    

    
    
    
    
