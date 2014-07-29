#
# Run this script to create the MMS data needed to run the Python unit tests 
# of some tasks. This script will try to find the location of the data repository 
# based on the value of CASAPATH. The MSs will be read from the unittest directory 
# of the data repository. The output MMSs will be created in the local directory 
# under unittest_mms/<taskname>. The MMS directory will be removed if the command 
# is repeated for the same task.
# 


import os
import sys
import re
import getopt
import pprint
import traceback
import shutil
import partitionhelper as ph

# ---------- ADD NEW TASKS HERE ----------------
# These tasks have been verified to work on MMS. 
TASKLIST = [
            'bandpass',
            'flagdata',
            'fluxscale',
            'gaincal',
            'gencal',
            'listhistory',
            'listobs',
            'listvis', # shared with fixplanets
            'plotms',
            'split',
            'uvcontsub',
            'vishead',
            'wvrgcal',
            'concat' # shared with virtualconcat
            ]

# NOTE: task 'fixplanets' uses data from task 'listvis'

# NOTE: task 'concat' only works with MMSs if the first input MS is a normal MS;
#       the test data is also needed for testing virtualconcat

# Try to get the data repository path from the system
DATAPATH = os.environ.get('CASAPATH').split()[0] + '/data/regression/'

def usage():
    print '========================================================================='
    print '\nmake_mmsdata will create Multi-MS data for CASA tasks.'
    print 'Usage:\n'
    print 'casapy [casapy-options] -c make_mmsdata.py [options] <tasks>\n'
    print 'Options:'
    print '   no option         print this message and exit.'
    print '   --all             run the script for all tasks in TASKLIST.'
    print '   --ignore          do no create MMS for the given <tasks>.'
    print '   --list            print the list of tasks from TASKLIST and exit.'
    print '   --parallel        create MMSs in parallel using simple_cluster.'
    print 'NOTE: it will look for MS data in the data repository under unittest.\r'
    print '=========================================================================='


def selectList(nolist):
    '''Return the subtracted list of tasks
       nolist --> list of tasks to ignore'''
    
    newlist = []
    for t in TASKLIST:
        if t not in nolist:
            newlist.append(t)
            
    return newlist
    
    
# Function to call partitionhelper.convertToMMS()
def mmstest(mytask, parallel):

    TESTPATH = DATAPATH + 'unittest/'
    INPPATH = TESTPATH + mytask
    MMSPATH = './unittest_mms/'+mytask

    print '--------- Will create MMS data for test_'+mytask
    ph.convertToMMS(inpdir=INPPATH, mmsdir=MMSPATH, parallel=parallel, 
                    createmslink=True, cleanup=True)

      
# Location of the data repository
if not os.environ.has_key('CASAPATH'):
    print 'ERROR: Could not find variable CASAPATH'
    os._exit(2)
    

def main(thislist, parallel=False):
    
    if thislist == []:
        print 'Need list of tasks to run.'
        usage()
        os._exit(0)
    
        
    # Loop through task list
    for t in thislist:
        if t not in TASKLIST:
            print 'ERROR: task '+t+' is not in TASKLIST. Run this script with -l for the full list.'
            os._exit(0)
            
        mmstest(t, parallel)

    from tasks import partition

    if 'listvis' in thislist:
        # NOTE for test_listvis data:
        # You need to run partition by hand to create an MMS for the single-dish data set
        SDPATH = DATAPATH + 'unittest/listvis/'
        SDMMS = './unittest_mms/listvis/'
    
        partition(vis=SDPATH+'OrionS_rawACSmod', outputvis=SDMMS+'OrionS_rawACSmod.mms', 
                  datacolumn='float_data', createmms=True, flagbackup=False,
                  parallel=parallel)

    if 'split' in thislist:
        # some additional MMSs
        SPLITMMSPATH = './unittest_mms/split/'
        specialcase = ['0420+417/0420+417.ms',
                       'viewertest/ctb80-vsm.ms',
                       'split/labelled_by_time+ichan.ms']
        for myms in specialcase:
            shutil.rmtree(SPLITMMSPATH+os.path.basename(myms), ignore_errors=True)
            partition(vis=DATAPATH+myms, outputvis=SPLITMMSPATH+os.path.basename(myms), 
                      datacolumn='all', flagbackup=False,
                      parallel=parallel)

        # workaround for a partition shortcoming: column keywords not copied
        tb.open(SPLITMMSPATH+'hasfc.mms/SUBMSS/hasfc.0000.ms/', nomodify=False)
        tb.putcolkeyword('FLAG_CATEGORY','CATEGORY', ['FLAG_CMD', 'ORIGINAL', 'USER'])
        tb.close()


    if 'wvrgcal' in thislist:
        WVRGCALMMSPATH = './unittest_mms/wvrgcal/'
        WVRGCALPATH = DATAPATH+'unittest/wvrgcal/input/'
        origwd = os.getcwd()
        os.chdir(WVRGCALMMSPATH)
        shutil.rmtree('input', ignore_errors=True)
        os.mkdir('input')
        os.chdir('input')
        mydirs = os.listdir(WVRGCALPATH)
        for d in mydirs:
            print d
            if  os.path.splitext(d)[1]=='.ms':
                partition(vis=WVRGCALPATH+d, outputvis=d, datacolumn='all', numsubms=5,
                          flagbackup=False, parallel=parallel)
            else:
                os.symlink(WVRGCALPATH+d, d)
        os.chdir(origwd)

    if ('concat' in thislist):
        CONCATMMSPATH = './unittest_mms/concat/'
        CONCATPATH = DATAPATH+'unittest/concat/input/'
        origwd = os.getcwd()
        os.chdir(CONCATMMSPATH)
        shutil.rmtree('input', ignore_errors=True)
        os.mkdir('input')
        os.chdir('input')
        mydirs = os.listdir(CONCATPATH)
        for d in mydirs:
            print d
            if os.path.splitext(d)[1]=='.ms':
                partition(vis=CONCATPATH+d, outputvis=d, datacolumn='all', numsubms=6,
                          flagbackup=False, parallel=parallel)
            else:
                os.symlink(CONCATPATH+d, d)
        os.chdir(origwd)
        
    
if __name__ == "__main__":

    # Get command line arguments    
    if "-c" in sys.argv:
        # It is called with casapy ... -c make_mmsdata.py from the command line,
        i = sys.argv.index("-c")
        if len(sys.argv) >= i + 2 and \
               re.compile("make_mmsdata\.py$").search(sys.argv[i + 1]):
                    
            try:
                # Get only this script options
                opts,args=getopt.getopt(sys.argv[i+2:], "ailp", ["all", "ignore","list","parallel"])
                
            except getopt.GetoptError, err:
                # Print help information and exit:
                print str(err) # will print something like "option -a not recognized"
                usage()
                os._exit(2)
                
            # List of tests to run
            tasknames = []
            
            parallel = False            
            ignore = False
            all = False
            
            # Print help and exit
            if opts == [] and args == []:
                usage()
                os._exit(0)
            
            elif opts != []:
                for o, a in opts:
                    if o in ("-p", "--parallel"):
                        parallel = True
                        continue
                    
                    if o in ("-a", "--all"):
                        all = True
                        tasknames = TASKLIST
                        break
                    
                    elif o in ("-i", "--ignore"):
                        # Will ignore the tasks given in args
                        ignore = True
                        break
                    
                    elif o in ("-l", "--list"):
                        print 'List of tasks to create MMS for:'
                        pprint.pprint(TASKLIST)
                        os._exit(0)          
                                  
                    else:
                        assert False, "unhandled option"

            # Get tasks
            if args == [] and ignore:
                print "ERROR: --ignore needs a list of tasks."
                usage()
                os._exit(0)
                
            if args != [] and not all:
                tasknames = args
                if ignore:
                    tasknames = selectList(args)
                
    try:                 
        main(tasknames, parallel)
    except:
        traceback.print_exc()
    
    
    
    
