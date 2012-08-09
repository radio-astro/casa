#############################################################################
# $Id:$
# Test Name:                                                                #
#    Unit test script for the ms.createmultims method                       #
#                                                                           #
#                                                                           #
#############################################################################
import os
import sys
import shutil
import glob
from __main__ import default
from tasks import *
from taskinit import *
import unittest

myname = 'test_createmultims'

# name of the resulting MS
msname = 'multims.ms'

def checktable(thename, theexpectation):
    global msname, myname
    tb.open(msname+"/"+thename)
    for mycell in theexpectation:
        print myname, ": comparing ", mycell
        value = tb.getcell(mycell[0], mycell[1])
        # see if value is array
        try:
            isarray = value.__len__
        except:
            # it's not an array
            # zero tolerance?
            if mycell[3] == 0:
                in_agreement = (value == mycell[2])
            else:
                in_agreement = ( abs(value - mycell[2]) < mycell[3]) 
        else:
            # it's an array
            # zero tolerance?
            if mycell[3] == 0:
                in_agreement =  (value == mycell[2]).all() 
            else:
                try:
                    in_agreement = (abs(value - mycell[2]) < mycell[3]).all()
                except:
                    in_agreement = False
        if not in_agreement:
            print myname, ":  Error in MS subtable", thename, ":"
            print "     column ", mycell[0], " row ", mycell[1], " contains ", value
            print "     expected value is ", mycell[2]
            tb.close()
            return False
    tb.close()
    print myname, ": table ", thename, " as expected."
    return True


###########################
# beginning of actual test 

class test_createmultims(unittest.TestCase):
    
    def setUp(self):
        res = None

        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/concat/input/'
        cpath = os.path.abspath(os.curdir)
        filespresent = sorted(glob.glob("part*.ms"))
        os.chdir(datapath)
        for mymsname in sorted(glob.glob("part*.ms")):
            if not mymsname in filespresent:
                print "Copying ", mymsname
                shutil.copytree(mymsname, cpath+'/'+mymsname)
        os.chdir(cpath)

        
    def tearDown(self):
        pass
        shutil.rmtree(msname,ignore_errors=True)

    def test1(self):
        '''Test_createmultims 1: 4 parts, same sources but different spws'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    
        
        shutil.rmtree(msname,ignore_errors=True)

        self.res = ms.createmultims(msname,
                                    ['part1.ms','part2.ms','part3.ms','part4.ms'],
                                    [],
                                    True, # nomodify
                                    False,# lock
                                    True) # copysubtables from first to all other members
        ms.close()

        self.assertEqual(self.res,True)

        ldict = listpartition(vis=msname, createdict=True)

        self.assertEqual(sorted(ldict.keys()), [0, 1, 2, 3])

        self.assertEqual(ldict[0]['MS'].split('/').pop(), 'part1.ms')
        self.assertEqual(ldict[1]['MS'].split('/').pop(), 'part2.ms')
        self.assertEqual(ldict[2]['MS'].split('/').pop(), 'part3.ms')
        self.assertEqual(ldict[3]['MS'].split('/').pop(), 'part4.ms')


class testcreatemultims_cleanup(unittest.TestCase):           
    def setUp(self):
        pass
    
    def tearDown(self):
        pass
        os.system('rm -rf *.ms')   

    def testrun(self):
        '''Test_createmultims: Cleanup'''
        pass
    
def suite():
    return [test_createmultims,testcreatemultims_cleanup]        
        
