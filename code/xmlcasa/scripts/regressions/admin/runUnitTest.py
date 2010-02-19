
""" Script to run unit tests from the command line as: 
    casapy [casa-options] -c runUnitTest.py testname1 testname2...
    casapy [casa-options] -c runUnitTest.py testname1[test_r,test23] testname2...
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

def is_old(name):
    '''Check if the test is old or new'''
    if (OLD_TESTS.__contains__(name)):
        return True
    elif (NEW_TESTS.__contains__(name)):
        return False
    else:
        print 'WARN: %s is not a valid test name'%name
        return None

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
    '''Run all tests'''
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
        '''Run specific tests'''
        # is this an old or a new test?
        oldlist = []
        newlist = []
        for f in testnames:
            if not haslist(f):
                if is_old(f):
                    oldlist.append(f)
                elif not is_old(f):
                    newlist.append(f)
            else:
                # they are always new tests. check if they are in list anyway
                ff = getname(f)
                if not is_old(ff):
                    newlist.append(f)
                else:
                    print 'Cannot find test '+ff
                
                
        if (len(oldlist) == 0 and len(newlist) == 0):
            os.chdir(PWD)
            raise Exception, 'Tests are not part of any list'
                
        print "Starting unit tests for %s%s: " %(oldlist,newlist)
        
        # Assemble the old tests
        list = []    
        for f in oldlist:
            try:
                testcase = UnitTest(f).getFuncTest()
                list = list+[testcase]
            except:
                traceback.print_exc()
                
        # Assemble the new tests
        for f in newlist:
            try:
                if haslist(f):
                    file = getname(f)
                    tests = gettests(f)
                    testcases = UnitTest(file).getUnitTest(tests)
                    list = list+testcases
                else:
                    testcases = UnitTest(f).getUnitTest()
                    list = list+testcases
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
        


