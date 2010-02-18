
""" Script to run unit tests from the command line as: 
    casapy [casa-options] -c runUnitTest.py testnames
    casapy [casa-options] -c runUnitTest.py
    
    or from inside casapy:
    import runUnitTest.py
    runUnitTest.main(['testname']) """

# The main class in testwrapper.py is:
# class UnitTest, methods: getUnitTest(), getFuncTest()
#
# UnitTest.getUnitTest() --> for new scripts as described in ....doc....
# UnitTest.getFuncTest() --> for old tests, which contain functions data() and run()


import os
import sys
import traceback
import unittest
sys.path.append(os.environ["CASAPATH"].split()[0] + '/code/xmlcasa/scripts/regressions/admin')
import testwrapper
from testwrapper import *
import nose


# List of tests to run
OLD_TESTS = ['asdm-import','boxit_test','clearstat_test',
             'hanningsmooth_test','imcontsub_test','imfit_test','imhead_test','immath_test',
             'immoment_test','imregrid_test',
             'imsmooth_test','imval_test','plotants_test','smoothcal_test','vishead_test',
             'visstat_test']
NEW_TESTS = ['test_listhistory','test_clean','test_report','test_cvel','test_exportasdm']

whichtests = 0

def main(testnames=[]):
    
    if testnames == []:
        whichtests = 0
    else:
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
    
    
    # RUN THE TESTS
    
    if not whichtests:
        print "Starting unit tests for %s%s: " %(OLD_TESTS,NEW_TESTS)
        
        # Assemble the old tests       
        testnames = OLD_TESTS
        list = []
        for f in testnames:
            try:
                testcase = UnitTest(f).getFuncTest()
                list = list+[testcase]
            except:
                traceback.print_exc()
        
        # Assemble the new tests
        testnames = NEW_TESTS        
        for f in testnames:
            try:
                tests = UnitTest(f).getUnitTest()
                list = list+tests
            except:
                traceback.print_exc()
                
        # Run all tests and create a XML report
        xmlfile = xmldir+'nose.xml'
        try:
            result = nose.run(argv=[sys.argv[0],"-d","--with-xunit","--verbosity=2","--xunit-file="+xmlfile], 
                                  suite=list)
        except:
            print "Exception: failed to run the test"
            traceback.print_exc()
            
        else:
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
            os.chdir(PWD)
            raise Exception, 'Tests are not part of any list'
                
        print "Starting unit tests for %s%s: " %(old,new)
        
        # Assemble the old tests
        list = []    
        for f in old:
            try:
                testcase = UnitTest(f).getFuncTest()
                list = list+[testcase]
            except:
                traceback.print_exc()
                
        # Assemble the new tests
        for f in new:
            try:
                tests = UnitTest(f).getUnitTest()
                list = list+tests
            except:
                 traceback.print_exc()
                 
        # Run the tests and create a XML report
        xmlfile = xmldir+'nose.xml'
        try:
            result = nose.run(argv=[sys.argv[0],"-d","--with-xunit","--verbosity=2","--xunit-file="+xmlfile], 
                                  suite=list)
        except:
            print "Exception: failed to run the test"
            traceback.print_exc()
            
        else:
            os.chdir(PWD)

if __name__ == "__main__":
    # Get command line arguments
    
    # Get the tests to run
    i = sys.argv.index("-c")
    this_file = sys.argv[i+1]
    testnames = []
    
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
        
#    sys.exit(main(testnames))
    try:
        main(testnames)
    except:
        traceback.print_exc()
        


