
""" Script to run unit tests from the command line as: 
    casapy [casa-options] -c runUnitTest.py testname1 testname2 ...
    casapy [casa-options] -c runUnitTest.py testname1[test_r,test23] testname2...
    casapy [casa-options] -c runUnitTest.py --Help
    casapy [casa-options] -c runUnitTest.py --list
    casapy [casa-options] -c runUnitTest.py --list
    casapy [casa-options] -c runUnitTest.py --file Tests.txt
    casapy [casa-options] -c runUnitTest.py --classes test_listobs
    
    or from inside casapy:
    runUnitTest.main(['testname']) 
    runUnitTest.main()
    
    NOTE: It will search for scripts in the casapy installation directory, which usually is in:
           <casa_install_dir>/python/2.6/tests"""

# The main class in testwrapper.py is:
# class UnitTest, methods: getUnitTest(), getFuncTest()
#
# UnitTest.getUnitTest() --> for new scripts as described in ....doc....
# UnitTest.getFuncTest() --> for old tests, which contain functions data() and run()


import os
import sys
import getopt
import traceback
import unittest
import string
import re
import shutil
import pprint
import nose
from taskinit import casalog

PYVER = str(sys.version_info[0]) + "." + str(sys.version_info[1])

CASA_DIR = os.environ["CASAPATH"].split()[0]
TESTS_DIR = CASA_DIR + "/" + os.environ["CASAPATH"].split()[1] + '/python/' + PYVER + '/tests/'
#DATA_DIR = CASA_DIR+'/data/'
#print 'HELLOR DATA_DIR'
#print DATA_DIR
if not os.access(TESTS_DIR, os.F_OK):
    if os.access(CASA_DIR+'/lib64', os.F_OK):
        TESTS_DIR = CASA_DIR+'/lib64/python' + PYVER + '/tests/'
    elif os.access(CASA_DIR+'/lib', os.F_OK):
        TESTS_DIR = CASA_DIR+'/lib/python/tests/'
    else:            #Mac release
        TESTS_DIR = CASA_DIR+'/Resources/python/tests/'

HAVE_MEMTEST=True
try:
    import memTest
except:
    HAVE_MEMTEST = False

import testwrapper
from testwrapper import *


# Tests included in the following file are run automatically by
# Hudson. This is also the list of tests run when no options are given
# to this program
LISTofTESTS = TESTS_DIR+'unittests_list.txt'


# memory mode variable
MEM = 0

def usage():
    print '========================================================================='
    print '\nRunUnitTest will execute Python unit test(s) of CASA tasks.'
    print 'Usage:\n'
    print 'casapy [casapy-options] -c runUnitTest.py [options] test_name\n'
    print 'Options:'
    print '  no option              run all tests defined in '
    print '                         active/gcwrap/python/scripts/tests/unittests_list.txt.'
    print '  <test_name>            run only <test_name> (more tests are separated by spaces).'
    print '  -f or --file <list>    run the tests defined in an ASCII file <list>; one test per line.'
    print '  -d or --datadir <dir>  set an env. variable to a directory, TEST_DATADIR=<dir> '
    print '                         that can be used inside the tests.'
    print '  -m or --mem            show the memory used by the tests and the number of files left open.'
    print '  -g or --debug          set casalog.filter to DEBUG.'
    print '  -l or --list           print the list of tests from '
    print '                         active/gcwrap/python/scripts/tests/unittests_list.txt.'
    print '  -s or --classes        print the classes from a test script (those returned by suite()).'
    print '  -H or --Help           print this message and exit.\n'
    print 'NOTE: it will look for tests in the install directory, which usually is \r'
    print '      <casa_install_dir>/python/2.6/tests'
    print 'See documentation in: http://www.eso.org/~scastro/ALMA/CASAUnitTests.htm\n'
    print '=========================================================================='

def list_tests():
    print 'Full list of unit tests'
    print '-----------------------'
    for t in readfile(LISTofTESTS):
        print t
    

def haslist(name):
    '''Check if specific list of tests have been requested'''
    n0 = name.rfind('[')
    n1 = name.rfind(']')
    if n0 == -1:
        return False
    return True

def getname(testfile):
    '''Get the test name from the command-line
       Ex: from test_clean[test1], returns test_clean'''
    n0 = testfile.rfind('[')
    n1 = testfile.rfind(']')
    if n0 != -1:
        return testfile[:n0]

def gettests(testfile):
    '''Get the list of specific tests from the command-line
       Ex: from test_clean[test1,test3] returns [test1,test3]'''
    n0 = testfile.rfind('[')
    n1 = testfile.rfind(']')
    if n0 != -1:
        temp = testfile[n0+1:n1]
        tests = temp.split(',')
        return tests

def readfile(FILE):
    # It will skip lines that contain '#' and
    # it will only read words starting with test
    if(not os.path.exists(FILE)):
        print 'ERROR: List of tests does not exist'
        return []
    
    List = []
    infile = open(FILE, "r")
    for newline in infile:
        if newline.__contains__('#'):
            continue
        
        if newline.startswith('test'):
            words = newline.split()
            List.append(words[0])
    
    infile.close()
    return List

def settestdir(datadir):
    '''Set an environmental variable for the data directory'''
    absdatadir = os.path.abspath(datadir)
    os.environ.__setitem__('TEST_DATADIR',absdatadir)
    return

def getclasses(testnames):
    '''Get the classes of a test script
       It will copy the test script to /tmp
       and remove it afterwards'''

    here = os.getcwd()
    tmpdir = '/tmp'
    try:
        os.chdir(tmpdir)
        
        for filename in testnames:
            tt = UnitTest(filename)
            tt.copyTest(copyto=tmpdir)

            classes = tt.getTestClasses(filename)
            for c in classes:
                pprint.pprint('Class '+c.__name__)       
                for attr, value in c.__dict__.iteritems():
                    if len(attr) >= len("test") and attr[:len("test")] == "test":
                        print '\t%s'%c(attr)
                  
            os.remove(filename+'.py')       
            os.remove(filename+'.pyc')       
        
        os.chdir(here)
    except:
        print '--> ERROR: Cannot copy script to %s'%tmpdir
        return
    

# Define which tests to run    
whichtests = 0
            

def main(testnames=[]):

    # Global variable used by regression framework to determine pass/failure status
    global regstate  
    regstate = False
        
    listtests = testnames
    if listtests == '--Help':
        usage()
        sys.exit()
    if listtests == '--list':
        list_tests()
        sys.exit()
        
    if listtests == []:
        whichtests = 0
        # Get the full list of tests from file
        listtests = readfile(LISTofTESTS)
        if listtests == []:
            raise Exception, 'List of tests \"%s\" is empty or does not exist'%LISTofTESTS

    elif (type(testnames) != type([])):
        if (os.path.isfile(testnames)):
            # How to prevent it from opening a real test???
            whichtests = 1
            listtests = readfile(testnames)
            if listtests == []:
                raise Exception, 'List of tests is empty'
        else:
            raise Exception, 'List of tests does not exist'
            
    else:
        # run specific tests
        whichtests = 1


    # Directories
    PWD = os.getcwd()
    WDIR = PWD+'/nosedir/'
    
    # Create a working directory
    workdir = WDIR
    print 'Creating work directory '+ workdir
    if os.access(workdir, os.F_OK) is False:
        os.makedirs(workdir)
    else:
        shutil.rmtree(workdir)
        os.makedirs(workdir)
    
    # Move to working dir
    os.chdir(workdir)
    
    # Create a directory for nose's xml files
    xmldir = WDIR+'xml/'
    if os.access(xmldir, os.F_OK) is False:
        os.makedirs(xmldir)
    else:
        shutil.rmtree(xmldir)
        os.makedirs(xmldir)
    
    print "Starting unit tests for %s: " %(listtests)
    
    # ASSEMBLE and RUN the TESTS
    if not whichtests:
        '''Run all tests'''
        list = []
        
        for f in listtests:
            try:
                tests = UnitTest(f).getUnitTest()
                list = list+tests
            except:
                traceback.print_exc()
                    
    elif (whichtests == 1):
        '''Run specific tests'''
        list = []
        for f in listtests:
            if not haslist(f):                
                testcases = UnitTest(f).getUnitTest()
                list = list+testcases

            else:
                ff = getname(f)
                tests = gettests(f)
                testcases = UnitTest(ff).getUnitTest(tests)
                list = list+testcases                
                
        if (len(list) == 0):
            os.chdir(PWD)
            raise Exception, 'ERROR: There are no valid tests to run'
                                                                     
                
    # Run all tests and create a XML report
    xmlfile = xmldir+'nose.xml'
    try:
        if (HAVE_MEMTEST and MEM):
            regstate = nose.run(argv=[sys.argv[0],"-d","-s","--with-memtest","--verbosity=2",
                            "--memtest-file="+xmlfile], suite=list, addplugins=[memTest.MemTest()])
        else:
            regstate = nose.run(argv=[sys.argv[0],"-d","-s","--with-xunit","--verbosity=2",
                            "--xunit-file="+xmlfile], suite=list)

        os.chdir(PWD)
    except:
        print "Failed to run one or more tests"
        traceback.print_exc()
    else:
        os.chdir(PWD)


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
               re.compile("runUnitTest\.py$").search(sys.argv[i + 1]):
            
        
            try:
                # Get only this script options
                opts,args=getopt.getopt(sys.argv[i+2:], "Hlmgs:f:d:", ["Help","list","mem",
                                                                     "debug","classes=","file=",
                                                                     "datadir="])
                
            except getopt.GetoptError, err:
                # Print help information and exit:
                print str(err) # will print something like "option -a not recognized"
                usage()
                os._exit(2)
                
            # List of tests to run
            testnames = []
            
            # Boolean for file with tests.
            # One could allow the use of --file with specific tests given in
            # the command line by removing this option and appending to the
            # testnames list in the args handling
            hasfile = False
            
            # If no option is given to this script, run all tests that are defined in
            # ../gcwrap/python/scripts/tests/unittests_list.txt
            if opts == [] and args == []:
                whichtests = 0
                testnames = []
            # run all tests in memory mode
            elif (args == [] and opts.__len__() == 1 and opts[0][0] == "--mem"):
                MEM = 1
                whichtests = 0
                testnames = []     
            # All other options       
            else:
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
                    elif o in ("-f", "--file"):
                        hasfile = True
                        testnames = a
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
                        
                        
                    else:
                        assert False, "unhandled option"


                # Deal with other arguments
                if args != [] and not hasfile:
                    testnames = args
                                        
        else:
            testnames = []
        
    else:
        # Not called with -c (but possibly execfile() from iPython)
        testnames = []

                    
    try:
        main(testnames)
    except:
        traceback.print_exc()
        

