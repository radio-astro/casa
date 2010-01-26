
import os
import sys
#sys.path.append('/Users/scastro/casadir/utests')
from testwrapper import *
import unittest
#sys.path.append('/Library/Python/2.6/site-packages/nose-0.11.1-py2.6.egg')
import nose

ALL_TESTS = ['test_report']

i = sys.argv.index("-c")
this_file = sys.argv[i+1]
testnames = []

if sys.argv.__len__() == i+2:
    testnames = ALL_TESTS
else:
    testnames = [sys.argv[i+2]]

print testnames
PWD = os.getcwd()


for f in testnames:
    try:
        tests = UnitTest(f).runTest()
        xmlfile = f+'.xml'
        result = nose.run(argv=[sys.argv[0],"--with-xunit", "--xunit-file="+xmlfile], suite=tests)
        os.chdir(PWD)
    except:
        print "Exception: failed to run %s" %f

 
    
    
    
    
    
