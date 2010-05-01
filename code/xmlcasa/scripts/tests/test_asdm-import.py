#############################################################################
# $Id:$
# Test Name:                                                                #
#    Unit Test Script for ASDM "version 0.9"  import to MS                  #
#                                                                           #
# Rationale for Inclusion:                                                  #
#    The conversion of ASDM to MS needs to be verified.                     #
#                                                                           # 
# Features tested:                                                          #
#    1) Is the import performed without raising exceptions                  #
#    2) Do all expected tables exist                                        #
#    3) Can the MS be opened                                                #
#    4) Do the tables contain expected values                               #
#                                                                           #
# Input data:                                                               #
#     one dataset for the old filler (ASDM "0.9")                           #
#   
# How to run it outside casapy:                                             #
# casapy -c runUnitTest.py test_imhead
#
# This example shows who to run it manually from within casapy.
# sys.path.append( os.environ["CASAPATH"].split()[0]+'/code/xmlcasa/scripts/regressions/admin' )
# import runUnitTest
# runUnitTest.main(['test_imhead'])
#                                                                     
#############################################################################
import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest

myname = 'asdm-import_regression'

# default ASDM dataset name
myasdm_dataset_name = 'uid___X54_X1418_X1'

# get the dataset name from the wrapper if possible
#mydict = locals()
#if mydict.has_key("asdm_dataset_name"):
#    myasdm_dataset_name = mydict["asdm_dataset_name"]

# name of the resulting MS
msname = myasdm_dataset_name+'.ms'

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
                in_agreement = (abs(value - mycell[2]) < mycell[3]).all() 
        if not in_agreement:
            print myname, ":  Error in MS subtable", thename, ":"
            print "     column ", mycell[0], " row ", mycell[1], " contains ", value
            print "     expected value is ", mycell[2]
            tb.close()
#            raise
            return False
    tb.close()
    print myname, ": table ", thename, " as expected."
#    return
    return True

class asdm_import(unittest.TestCase):
    
    def setUp(self):
        res = None
        if(os.path.exists(myasdm_dataset_name)):
            os.system('rm -rf ' +myasdm_dataset_name)

        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/asdm-import/input/'
        os.system('cp -r ' +datapath + myasdm_dataset_name +' ' + myasdm_dataset_name)
        default(importoldasdm)
        
    def tearDown(self):
        os.system('rm -rf ' +myasdm_dataset_name)
        os.system('rm -rf ' +myasdm_dataset_name+'.ms*')
        
    def test1(self):
        '''Asdm-import 1: Default parameters should return False'''
        self.res = importoldasdm()
        self.assertFalse(self.res,'Default parameters did not return False')
        
    def test2(self):
        '''Asdm-import 2: Bad input file'''
        self.res = importoldasdm(asdm='somefile')
        self.assertFalse(self.res,'Bad input file did not return False')
        
    def test3(self):
        '''Asdm-import 3: Good input'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    
        try:
            self.res = importoldasdm(myasdm_dataset_name)
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']+'Failed to run on good input.'
#            print myname, ": Error ", sys.exc_info()[0]
#            raise
        else:
#            print myname, ": Success! Now checking output ..."
            mscomponents = set(["table.dat",
                                "table.f0",
                                "table.f1",
                                "table.f2",
                                "table.f3",
                                "table.f4",
                                "table.f5",
                                "table.f6",
                                "table.f7",
                                "table.f8",
                                "ANTENNA/table.dat",
                                "DATA_DESCRIPTION/table.dat",
                                "FEED/table.dat",
                                "FIELD/table.dat",
                                "FLAG_CMD/table.dat",
                                "HISTORY/table.dat",
                                "OBSERVATION/table.dat",
                                "POINTING/table.dat",
                                "POLARIZATION/table.dat",
                                "PROCESSOR/table.dat",
                                "SOURCE/table.dat",
                                "SPECTRAL_WINDOW/table.dat",
                                "STATE/table.dat",
                                "SYSCAL/table.dat",
                                "ANTENNA/table.f0",
                                "DATA_DESCRIPTION/table.f0",
                                "FEED/table.f0",
                                "FIELD/table.f0",
                                "FLAG_CMD/table.f0",
                                "HISTORY/table.f0",
                                "OBSERVATION/table.f0",
                                "POINTING/table.f0",
                                "POLARIZATION/table.f0",
                                "PROCESSOR/table.f0",
                                "SOURCE/table.f0",
                                "SPECTRAL_WINDOW/table.f0",
                                "STATE/table.f0",
                                "SYSCAL/table.f0"
                                ])
            for name in mscomponents:
                if not os.access(msname+"/"+name, os.F_OK):
                    print myname, ": Error  ", msname+"/"+name, "doesn't exist ..."
#                    raise
                    retValue['success']=False
                    retValue['error_msgs']=retValue['error_msgs']+msname+'/'+name+' does not exist'
                else:
                    retValue['success']=True
                    print myname, ": ", name, "present."
            print myname, ": MS exists. All tables present. Try opening as MS ..."
            try:
                ms.open(msname)
            except:
#                print myname, ": Error  Cannot open MS table", tablename
#                raise
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Cannot open MS table '+msname
            else:
                ms.close()
                print myname, ": OK. Checking tables in detail ..."
                retValue['success']=True
        
                # check main table first
                name = ""
                #             col name, row number, expected value, tolerance
                expected = [ ['UVW',       42, [1.9006977, -22.538636, 26.703018], 1E-6],
                             ['EXPOSURE',  42, 1.008, 0],
                             ['DATA',      42, [ [0.00025587+0.00000797j] ], 1E-8]
                             ]
                results = checktable(name, expected)
                if not results:
                    retValue['success']=False
                    retValue['error_msgs']=retValue['error_msgs']+'Check of table MAIN failed'
                else:
                    retValue['success']=True

                
                name = "ANTENNA"
                expected = [ ['OFFSET',       1, [ 0.,  0.,  0.], 0],
                             ['POSITION',     1, [-1601361.760577, -5042192.535329,  3554531.519329], 0],
                             ['DISH_DIAMETER',1, 12.0, 0]
                             ]
                results = checktable(name, expected)
                if not results:
                    retValue['success']=False
                    retValue['error_msgs']=retValue['error_msgs']+'Check of table ANTENNA failed'
                else:
                    retValue['success']=True
                    
                
                name = "POINTING"
                expected = [ ['DIRECTION',       10, [[0.958],[0.416791]], 1E-6 ],
                             ['INTERVAL',        10, 0.048, 0],
                             ['TARGET',          10, [[ 0.95722],[ 0.4183]], 1E-6 ],
                             ['TIME',            10, 4735409708, 1],
                             ['TIME_ORIGIN',     10, 0., 0],
                             ['POINTING_OFFSET', 10, [[ 0.],[ 0.]], 0],
                             ['ENCODER',         10, [0.958002, 0.416791], 1E-6 ]
                             ]
                results = checktable(name, expected)
                if not results:
                    retValue['success']=False
                    retValue['error_msgs']=retValue['error_msgs']+'Check of table POINTING failed'
                else:
                    retValue['success']=True
        
        self.assertTrue(retValue['success'],retValue['error_msgs'])
            
def suite():
    return [asdm_import]        


