import os
import re
import sys
import getopt
import shutil
import traceback
from casa_stack_manip import *
from publish_summary import runTest

## flush output
sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 0)
sys.stderr = os.fdopen(sys.stderr.fileno(), 'w', 0)

PYVER = str(sys.version_info[0]) + "." + str(sys.version_info[1])

CASA_DIR = os.environ["CASAPATH"].split()[0]
TESTS_DIR = CASA_DIR + "/" + os.environ["CASAPATH"].split()[1] + '/lib/python' + PYVER + '/regressions/'

_potential_data_directories = ( "/opt/casa/data",
                                "/home/casa/data",
                                "/home/casa/data/trunk",
                                "/home/casa/data/master",
                                "/opt/casa/data/master",
                                "/export/data/casa" )

REGRESSION_DATA = filter(lambda x: os.access(x,os.F_OK),map(lambda y: y+"/regression",_potential_data_directories))
                      

if not os.access(TESTS_DIR, os.F_OK):
    if os.access(CASA_DIR+'/lib64', os.F_OK):
        TESTS_DIR = CASA_DIR+'/lib64/python' + PYVER + '/regressions/'
    elif os.access(CASA_DIR+'/lib', os.F_OK):
        TESTS_DIR = CASA_DIR+'/lib/python' + PYVER + '/regressions/'
    else:            #Mac release
        TESTS_DIR = CASA_DIR+'/Resources/python/regressions/'
stack_frame_find()['TESTS_DIR']=TESTS_DIR

def _find_script_path( name ):
    if os.access(TESTS_DIR+name+".py",os.F_OK):
        return TESTS_DIR+name+".py"
    elif os.access(TESTS_DIR+name+"_regression.py",os.F_OK):
        return TESTS_DIR+name+"_regression.py"
    elif os.access(TESTS_DIR+name+"-regression.py",os.F_OK):
        return TESTS_DIR+name+"-regression.py"
    elif os.access(TESTS_DIR+name+"_regression1.py",os.F_OK):
        return TESTS_DIR+name+"_regression1.py"
    elif os.access(TESTS_DIR+name+"-regression1.py",os.F_OK):
        return TESTS_DIR+name+"-regression1.py"
    elif os.access(TESTS_DIR+name+"_regression2.py",os.F_OK):
        return TESTS_DIR+name+"_regression2.py"
    elif os.access(TESTS_DIR+name+"-regression2.py",os.F_OK):
        return TESTS_DIR+name+"-regression2.py"
    elif os.access(TESTS_DIR+"test_"+name+".py",os.F_OK):
        return TESTS_DIR+"test_"+name+".py"
    else:
        raise RuntimeError("task %s not found" % name)

#publish_summary.runTest('" + script + "', WORKING_DIR='"+self._path['test']+'/pubsum'+"', RESULT_DIR='"+self._path['output']+"', RESULT_SUBDIR='"+script+"', REDIRECT=False" + PYPROFILE + ")" ])

# ------------------ NOTE ---------------------------------------------
# Once CASA moves to Python 2.7, the getpopt module should be replaced
# by argparse. The next section will need to be updated accordingly
# ---------------------------------------------------------------------
if __name__ == "__main__":
    # Get command line arguments
    
    if "-c" in sys.argv:
        # If called with ... -c runUnitTest.py from the command line,
        # then parse the command line parameters
        i = sys.argv.index("-c")
        if len(sys.argv) >= i + 2 and \
               re.compile("runRegressionTest\.py$").search(sys.argv[i + 1]):
            
        
            try:
                # Get only this script options
                opts,args=getopt.getopt(sys.argv[i+2:], "Halmgs:f:d:", ["Help","all","list","mem",
                                                                     "debug","classes=","file=",
                                                                     "datadir="])
                
            except getopt.GetoptError, err:
                # Print help information and exit:
                print str(err) # will print something like "option -a not recognized"
                os._exit(2)
                
            # List of tests to run
            testnames = []
            
            # Boolean for file with tests.
            # One could allow the use of --file with specific tests given in
            # the command line by removing this option and appending to the
            # testnames list in the args handling
            hasfile = False
            alltests = False
            
            #If no option is given, show the Help page
            if opts == [] and args == []:
                print "no arguments given..."
                os._exit(1)
                
            
            # All other options       
            for o, a in opts:
                if o in ("-H", "--Help"):
                    usage()
                    os._exit(0) 
                if o in ("-l", "--list"):
                    list_tests()
                    os._exit(0)
                if o in ("-s", "--classes"): 
                    testnames.append(a)
                    getclasses(testnames)
                    os._exit(0)
                if o in ("-m", "--mem"):
                    # run specific tests in mem mode            
                    MEM = 1
                elif o in ("-g", "--debug"):
                    #Set the casalog to DEBUG
                    casalog.filter('DEBUG')
                elif o in ("-d", "--datadir"):
                    # This will create an environmental variable called
                    # TEST_DATADIR that can be read by the tests to use
                    # an alternative location for the data. This is used 
                    # to test tasks with MMS data
                    # directory with test data
                    datadir = a
                    if not os.path.isdir(datadir):                            
                        raise Exception, 'Value of --datadir is not a directory -> '+datadir  
                    
                    # Set an environmental variable for the data directory
                    settestdir(datadir)
                    if not os.environ.has_key('TEST_DATADIR'):    
                        raise Exception, 'Could not create environmental variable TEST_DATADIR'                        
                        
                elif o in ("-a", "--all"):
                    alltests = True
                    whichtests = 0
                    testnames = 'all'
                    break
                elif o in ("-f", "--file"):
                    hasfile = True
                    testnames = a
                    
                else:
                    assert False, "unhandled option"


            # Deal with other arguments
            if args != [] and not hasfile and not alltests:
                testnames = args
                                        
        else:
            testnames = []
        
    else:
        # Not called with -c (but possibly execfile() from iPython)
        testnames = []

    assert len(testnames) == 1
    name = testnames[0]
    path = _find_script_path(name)
    #publish_summary.runTest('" + script + "', WORKING_DIR='"+self._path['test']+'/pubsum'+"', RESULT_DIR='"+self._path['output']+"', RESULT_SUBDIR='"+script+"', REDIRECT=False" + PYPROFILE + ")" ])
    print "------------------------------------------------------------------------------------------------------------------------"
    print "starting test %s (%s)" % (name,path)
    print "------------------------------------------------------------------------------------------------------------------------"
    execfile(path)
