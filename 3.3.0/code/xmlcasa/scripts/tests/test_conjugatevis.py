#############################################################################
# $Id:$
# Test Name:                                                                #
#    Regression Test Script for the conjugatevis task
#    
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

myname = 'test_conjugatevis'

# name of the resulting MS
msname = 'conjugated.ms'

def checktable(thename, theexpectation):
    global msname, myname
    tb.open(msname+"/"+thename)
    if thename == "":
        thename = "MAIN"
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

class test_conjugatevis(unittest.TestCase):
    
    def setUp(self):
        res = None

        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/concat/input/'
        cpath = os.path.abspath(os.curdir)
        filespresent = sorted(glob.glob("*.ms"))
        os.chdir(datapath)
        mymsname = 'shortpart1.ms'
        if not mymsname in filespresent:
            print "Copying ", mymsname
            shutil.copytree(mymsname, cpath+'/'+mymsname)
        os.chdir(cpath)

        default(conjugatevis)
        
    def tearDown(self):
        shutil.rmtree(msname,ignore_errors=True)

    def test1(self):
        '''Conjugatevis 1: '''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    
        
        self.res = conjugatevis(vis='shortpart1.ms', spwlist=[5,7], outputvis=msname)
        self.assertEqual(self.res,None)

        print myname, ": Success! Now checking output ..."
        mscomponents = set(["table.dat",
                            "table.f0"
                            ])
        for name in mscomponents:
            if not os.access(msname+"/"+name, os.F_OK):
                print myname, ": Error  ", msname+"/"+name, "doesn't exist ..."
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+msname+'/'+name+' does not exist'
            else:
                print myname, ": ", name, "present."
        print myname, ": MS exists. Try opening as MS ..."
        try:
            ms.open(msname)
        except:
            print myname, ": Error  Cannot open MS table", tablename
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']+'Cannot open MS table '+tablename
        else:
            ms.close()
            print myname, ": OK. Checking tables in detail ..."
            retValue['success']=True

            # check main table
            name = ""
            #             col name, row number, expected value, tolerance
            expected = [
                ['DATA',           4000,
                 [[-0.00426177-0.00387163j],
                  [ 0.00058119+0.00283016j]],
                 0.00000001]
                ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'
            expected = [
                ['DATA',           3000,
                 [[ 0.00347826-0.00406267j],
                  [ 0.00458098-0.00508398j]],
                 0.00000001]
                ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'


class conjugatevis_cleanup(unittest.TestCase):           
    def setUp(self):
        pass
    
    def tearDown(self):
        os.system('rm -rf *.ms')

    def testrun(self):
        '''Conjugatevis: Cleanup'''
        pass
    
def suite():
    return [test_conjugatevis,conjugatevis_cleanup]        
        
