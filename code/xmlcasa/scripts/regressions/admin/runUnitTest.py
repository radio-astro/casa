
""" Script to run unit tests from the command line as: 
    casapy [casa-options] -c runUnitTest.py testname1 testname2...
    casapy [casa-options] -c runUnitTest.py testname1[test_r,test23] testname2...
    casapy [casa-options] -c runUnitTest.py --help
    casapy [casa-options] -c runUnitTest.py --list
    casapy [casa-options] -c runUnitTest.py
    
    or from inside casapy:
    runUnitTest.main(['testname']) 
    runUnitTest.main()
    runUnitTest.main(['--short'])
    
    NOTE: Tests scripts must be located in ....scripts/tests and installed before usage.
    """

# The main class in testwrapper.py is:
# class UnitTest, methods: getUnitTest(), getFuncTest()
#
# UnitTest.getUnitTest() --> for new scripts as described in ....doc....
# UnitTest.getFuncTest() --> for old tests, which contain functions data() and run()


import os
import sys
import traceback
import unittest
import string
import re
import shutil
import nose

PYVER = str(sys.version_info[0]) + "." + str(sys.version_info[1])

CASA_DIR = os.environ["CASAPATH"].split()[0]
TESTS_DIR = CASA_DIR + "/" + os.environ["CASAPATH"].split()[1] + '/python/' + PYVER + '/tests/'
if not os.access(TESTS_DIR, os.F_OK):
    if os.access(CASA_DIR+'/lib64', os.F_OK):
        TESTS_DIR = CASA_DIR+'/lib64/python' + PYVER + '/tests/'
    elif os.access(CASA_DIR+'/lib', os.F_OK):
        TESTS_DIR = CASA_DIR+'/lib/python/tests/'
    else:            #Mac release
        TESTS_DIR = CASA_DIR+'/Resources/python/tests/'

import memTest
import testwrapper
from testwrapper import *


# Tests included in the following file are run automatically by
# Hudson. This is also the list of tests run when no options are given
# to this program
LISTofTESTS = TESTS_DIR+'unittests_list.txt'

SHORT_LIST = [
             'test_importevla',
             'test_listvis',
             'test_plotms',
             'test_sdplot',
             'test_sdsave',
             'test_viewer'
             ]

# memory mode variable
MEM = 0

def usage():
    print '*************************************************************************'
    print '\nRunUnitTest will execute unit test(s) of CASA tasks.'
    print 'Usage:\n'
    print 'casapy [casapy-options] -c runUnitTest.py [options]\n'
    print 'options:'
    print 'no option:        runs all tests defined in unittests_list.txt'
    print '<test_name>:      runs only <test_name> (more tests are separated by spaces)'
    print '--short:          runs only a short list of tests defined in SHORT_LIST'
    print '--file <list>:    runs the tests defined in <list>; one test per line'
    print '--list:           prints the full list of available tests from unittests_list.txt'
    print '--mem:            runs the tests in debugging mode'
    print '--help:           prints this message\n'
    print 'NOTE: tests must be located in ....scripts/tests\n'
    print 'See documentation in: http://www.eso.org/~scastro/ALMA/CASAUnitTests.htm\n'
    print '**************************************************************************'

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


# Define which tests to run    
whichtests = 0
            

def main(testnames=[]):

    # Global variable used by regression framework to determine pass/failure status
    global regstate  
    regstate = False
        
    listtests = testnames
    if listtests == '--help':
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
    elif (listtests == SHORT_LIST or listtests == ['--short']
          or listtests == ['SHORT_LIST']):
        whichtests = 2
        listtests = SHORT_LIST
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
    print 'Creating working directory '+ workdir
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
                                                                     
    elif(whichtests == 2):
        '''Run the SHORT_LIST of tests only'''
        
        # Assemble the short list of tests
        listtests = SHORT_LIST        
        list = []    
        for f in listtests:
            try:
                tests = UnitTest(f).getUnitTest()
                list = list+tests
            except:
                traceback.print_exc()
                
    # Run all tests and create a XML report
    xmlfile = xmldir+'nose.xml'
    try:
        if (MEM):
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


if __name__ == "__main__":
    # Get command line arguments
    
    if "-c" in sys.argv:
        # If called with ... -c runUnitTest.py from the command line,
        # then parse the command line parameters
        i = sys.argv.index("-c")
        if "--mem" in sys.argv:
            # run tests in mem mode            
            MEM = 1
            sys.argv.pop(sys.argv.index("--mem"))
            
        if len(sys.argv) >= i + 2 and \
               re.compile("runUnitTest\.py$").search(sys.argv[i + 1]):

            this_file = sys.argv[i+1]

            # Get the tests to run
            testnames = []
    
            la = [] + sys.argv
            
            if len(sys.argv) == i+2:
                # Will run all tests
                whichtests = 0
                testnames = []
            elif len(sys.argv) > i+2:
                # Will run specific tests.
                whichtests = 1
                
                while len(la) > 0:
                    elem = la.pop()
                    if elem == '--help':
                        usage()
                        os._exit(0)
                    if elem == '--list':
                        list_tests()
                        os._exit(0)
                    if elem == '--file':
                        # read list from a text file
                        index = i + 3
                        testnames = sys.argv[index]
                        break
                    if elem == '--short':
                        # run only the SHORT_LIST
                        whichtests = 2
                        testnames = SHORT_LIST
                        break    
                    if elem == this_file:
                        break
                
                    testnames.append(elem)
        else:
            testnames = []
    else:
        # Not called with -c (but possibly execfile() from iPython)
        testnames = []

#    os._exit(0)
    try:
        main(testnames)
    except:
        traceback.print_exc()
