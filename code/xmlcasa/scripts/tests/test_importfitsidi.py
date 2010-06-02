#############################################################################
# $Id:$
# Test Name:                                                                #
#    Regression Test Script for the FITS-IDI import to MS                   #
#    
#                                                                           #
# Rationale for Inclusion:                                                  #
#    The conversion of FITS-IDI to MS and back needs to be verified.        #
#                                                                           # 
# Features tested:                                                          #
#    1) Is the import performed without raising exceptions                  #
#    2) Do all expected tables exist                                        #
#    3) Can the MS be opened                                                #
#    4) Do the tables contain expected values                               #
#                                                                           #
# Input data:                                                               #
#                                                                           #
#############################################################################
import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest

myname = 'importfitsidi-unit-test'

# default dataset name
my_dataset_name = 'n09q2_1_1-shortened.IDI1'

# name of the resulting MS
msname = my_dataset_name+'.ms'

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

class test_importfitsidi(unittest.TestCase):
    
    def setUp(self):
        res = None
        if(os.path.exists(my_dataset_name)):
            shutil.rm(my_dataset_name)

        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/fitsidi_import/input/'
        shutil.copy(datapath + my_dataset_name, my_dataset_name)
        default(importfitsidi)
        
    def tearDown(self):
        os.remove(my_dataset_name)
        shutil.rmtree(msname,ignore_errors=True)
        shutil.rmtree(msname+'.flagversions',ignore_errors=True)
        
    def test1(self):
        '''fitsidi-import: Test good input'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    

        self.res = importfitsidi(my_dataset_name, msname)
        print myname, ": Success! Now checking output ..."
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
                            "STATE/table.f0"
                            ])
        for name in mscomponents:
            if not os.access(msname+"/"+name, os.F_OK):
                print myname, ": Error  ", msname+"/"+name, "doesn't exist ..."
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+msname+'/'+name+' does not exist'
            else:
                print myname, ": ", name, "present."
        print myname, ": MS exists. All tables present. Try opening as MS ..."
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
    
            # check main table first
            name = ""
            #             col name, row number, expected value, tolerance
            expected = [
                         ['UVW',       42, [ 0., 0., 0. ], 1E-8],
                         ['EXPOSURE',  42, 2.0, 1E-8],
                         ['DATA',      42, [[  5.32094546e-09 +9.37484264e-01j,
                                                9.43301380e-01 +0.00000000e+00j,
                                                9.37484264e-01 +9.13762391e-01j,
                                                3.24592087e-09 +9.37484264e-01j,
                                                9.40941751e-01 -9.31322575e-10j,
                                                9.37484264e-01 +9.07695830e-01j,
                                                9.31322575e-09 +9.37484264e-01j,
                                                9.10699069e-01 -1.86264515e-09j,
                                                9.37484264e-01 +1.00180876e+00j,
                                                7.41426787e-09 +9.37484264e-01j,
                                                9.88621294e-01 -1.86264515e-09j,
                                                9.37484264e-01 +1.09973443e+00j,
                                                -1.09088809e-08 +9.37484264e-01j,
                                                1.12680471e+00 +5.58793545e-09j,
                                                9.37484264e-01 +1.01711810e+00j,
                                                -7.66970309e-09 +9.37484264e-01j]], 1E-8]
                         ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'
    
            expected = [
                         ['UVW',       638, [171529.37575288, -786712.70341456, 210321.20978818], 1E-8],
                         ['EXPOSURE',  638,  2.0, 1E-8],
                         ['DATA',      638, [[  7.71090447e-04+0.0023962j,
                                                9.75148320e-01+0.00148579j,
                                                2.78137764e-03+0.97514832j,
                                                2.52080127e-03+0.00414553j,
                                                9.75148320e-01+0.00409369j,
                                                4.81047202e-03+0.97514832j,
                                                5.20560332e-03+0.00454798j,
                                                9.75148320e-01+0.00328651j,
                                                4.51627094e-03+0.97514832j,
                                                4.54078289e-03+0.00360481j,
                                                9.75148320e-01+0.0057058j,
                                                5.68820536e-03+0.97514832j,
                                                2.59231543e-03+0.00398705j,
                                                9.75148320e-01+0.00281886j,
                                                4.98241186e-03+0.97514832j,
                                                2.35986966e-03+0.00580851j]], 1E-8]
                         ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'
            
            name = "ANTENNA"
            expected = [ ['OFFSET',       1, [ 0.,  0.,  0.], 0],
                         ['POSITION',     1, [ 3370605.8469,  711917.6732,  5349830.8438], 0.0001],
                         ['DISH_DIAMETER',1, 6.0, 0]
                         ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'
            
            name = "SPECTRAL_WINDOW"
            expected = [ ['NUM_CHAN',        7, 16, 0],
                         ['TOTAL_BANDWIDTH', 7, 8E6, 0],
                         ['CHAN_WIDTH',      7, [ 500000.,  500000.,  500000.,  500000.,  500000.,  500000.,
                                                  500000.,  500000.,  500000.,  500000.,  500000.,  500000.,
                                                  500000.,  500000.,  500000.,  500000.], 1E-8],
                         ['CHAN_FREQ',       7, [  4.32184900e+10,   4.32189900e+10,   4.32194900e+10,   4.32199900e+10,
                                                   4.32204900e+10,   4.32209900e+10,   4.32214900e+10,   4.32219900e+10,
                                                   4.32224900e+10,   4.32229900e+10,   4.32234900e+10,   4.32239900e+10,
                                                   4.32244900e+10,   4.32249900e+10,   4.32254900e+10,   4.32259900e+10], 1E-8]
                         ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'
                
        self.assertTrue(results)
                
    
def suite():
    return [test_importfitsidi]        
        
    
