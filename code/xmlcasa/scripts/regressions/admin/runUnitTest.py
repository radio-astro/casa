
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
OLD_TESTS = ['accum_test','hanningsmooth_test','imfit_test']
NEW_TESTS = ['test_report','test_clean']

# Get the tests to run
i = sys.argv.index("-c")
this_file = sys.argv[i+1]
testnames = []
whichtests = 0  #all tests

la = [] + sys.argv

if sys.argv.__len__() == i+2:
    # Will run all tests
    whichtests = 0
elif sys.argv.__len__() > i+2:
    # Will run specific tests
    whichtests = 1
    
    while len(la) > 0:
        elem = la.pop()
        if elem == this_file:
            break           
        
        testnames.append(elem)
        
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


# RUN THE TESTS
if not whichtests:
    print "Starting unit tests for %s%s: " %(OLD_TESTS,NEW_TESTS)
    
    # Assemble the old tests       
    testnames = OLD_TESTS
    list = []
    for f in testnames:
        testcase = UnitTest(f).getFuncTest()
        list = list+[testcase]

    # Assemble the new tests
    testnames = NEW_TESTS        
    for f in testnames:
        tests = UnitTest(f).getUnitTest()
        list = list+tests
    
    # Run all tests and create a XML report
    xmlfile = xmldir+'nose.xml'
    try:
        result = nose.run(argv=[sys.argv[0],"-d","--with-xunit","--verbosity=2","--xunit-file="+xmlfile], 
                              suite=list)
    except:
        print "Exception: failed to run the test"
        traceback.print_exc()

    os.chdir(PWD)
    
else:
    # is this an old or new test?
    old = []
    new = []
    for f in testnames:
        if (OLD_TESTS.__contains__(f)):
            old.append(f)
        elif (NEW_TESTS.__contains__(f)):
            new.append(f)
        else:
            print 'WARN: %s is not a valid test name'%f
        
    if (len(old) == 0 and len(new) == 0):
        raise Exception, 'Tests are not part of any list'
            
    print "Starting unit tests for %s: " %testnames
    
    # Assemble the old tests
    list = []    
    for f in old:
        testcase = UnitTest(f).getFuncTest()
        list = list+[testcase]
    
    # Assemble the new tests
    for f in new:
        tests = UnitTest(f).getUnitTest()
        list = list+tests
         
    # Run the tests and create a XML report
    xmlfile = xmldir+'nose.xml'
    try:
        result = nose.run(argv=[sys.argv[0],"-d","--with-xunit","--verbosity=2","--xunit-file="+xmlfile], 
                              suite=list)
    except:
        print "Exception: failed to run the test"
        traceback.print_exc()

    os.chdir(PWD)

        
  








