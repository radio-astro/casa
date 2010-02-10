
""" Script to run unit tests from the command line as: """
"""    casapy --nologger --log2term -c runUnitTest.py """
"""    or """
"""    casapy --nologger --log2term -c runUnitTest.py test_name """

# There are two classes in testwrapper.py:
# class UnitTest, methods: runTest(), runFuncTest()
# class ExecTest, methods: setup(), testrun()
#
# UnitTest.runTest() --> for new scripts as described in ....doc....
# UnitTest.runFuncTest() --> for tests located in regressions/tests
# ExecTest.testrun() --> for tests in regressions/

import os
import sys
import traceback
sys.path.append(os.environ["CASAPATH"].split()[0] + '/code/xmlcasa/scripts/regressions/admin')
from testwrapper import *
import unittest
sys.path.append('/Library/Python/2.6/site-packages/nose-0.11.1-py2.6.egg')
import nose

# List of tests to run
E2E_TESTS = []
OLD_TESTS = []
NEW_TESTS = []

# Get the tests to run
i = sys.argv.index("-c")
this_file = sys.argv[i+1]
testnames = []
whichtests = 0  #all tests

if sys.argv.__len__() == i+2:
    whichtests = 0
elif sys.argv.__len__() == i+3:
    whichtests = 1
    testnames = [sys.argv[i+2]]

PWD = os.getcwd()
NOSE_XML = PWD+'/nosexml/'

# Create a directory for nose's xml files
xmldir = NOSE_XML
print ' Creating '+xmldir
if os.access(xmldir, os.F_OK) is False:
    os.makedirs(xmldir)
else:
    shutil.rmtree(xmldir)
    os.makedirs(xmldir)



if not whichtests:
    # Run new tests
    testnames = NEW_TESTS        

    for f in testnames:
        try:
            # Get a list of tests contained in the class
            tests = UnitTest(f).runTest()
            xmlfile = xmldir+f+'.xml'
            result = nose.run(argv=[sys.argv[0],"-d","--with-xunit","--verbosity=2","--xunit-file="+xmlfile], 
                              suite=tests)
            os.chdir(PWD)
        except:
            os.chdir(PWD)
            print "Exception: failed to run %s" %f
            traceback.print_exc()

    # Run old tests
    testnames = OLD_TESTS
    for f in testnames:
        try:
            os.chdir(PWD)
            # Get the testcase
            test = UnitTest(f).runFuncTest()
            xmlfile = xmldir+f+'.xml'
            result = nose.run(argv=[sys.argv[0],"-d","--with-xunit","--verbosity=2","--xunit-file="+xmlfile], 
                              suite=[test])
            os.chdir(PWD)
        except:
            os.chdir(PWD)
            print "Exception: failed to run %s" %f
            traceback.print_exc()


    # Run e2e tests
    testnames = E2E_TESTS
    for f in testnames:
        try:
            os.chdir(PWD)
            # Create a testcase of an e2e test
            testcase = ExecTest('testrun')
            testcase.testname = f       
            print '-------------- Unit Test for %s ---------------'%f
    
            testcase.setup()
           
            xmlfile = xmldir+f+'.xml'
            result = nose.run(argv=[sys.argv[0],"-d","--with-xunit","--verbosity=2","--xunit-file="+xmlfile], 
                              suite=[testcase])
            os.chdir(PWD)
        except:
            os.chdir(PWD)
            print "Exception: failed to run %s" %f
            traceback.print_exc()

else:
    print "Will run %s"%testnames
    # At the moment it only supports new unit tests
    try:
        # Get a list of tests contained in the class
        f = testnames[0]
        tests = UnitTest(f).runTest()
        xmlfile = xmldir+f+'.xml'
        result = nose.run(argv=[sys.argv[0],"-d","--with-xunit","--verbosity=2","--xunit-file="+xmlfile,], 
                          suite=tests)
        
        os.chdir(PWD)
    except:
        os.chdir(PWD)
        print "Exception: failed to run %s" %f
        traceback.print_exc()
        
    
    
    








