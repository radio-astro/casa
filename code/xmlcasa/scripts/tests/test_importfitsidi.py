#############################################################################
# $Id:$
# Test Name:                                                                #
#    Regression Test Script for the FITS-IDI import to MS                   #
#    
#                                                                           #
# Rationale for Inclusion:                                                  #
#    The conversion of FITS-IDI to MS needs to be verified.                 #
#                                                                           # 
# Features tested:                                                          #
#    1) Is the import performed without raising exceptions                  #
#    2) Do all expected tables exist                                        #
#    3) Can the MS be opened                                                #
#    4) Do the tables contain expected values                               #
#    5) Can several FITS-IDI files be read into one MS                      #
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
my_dataset_names = ['n09q2_1_1-shortened.IDI1',
                    'n09q2_1_1-shortened-part1.IDI1',
                    'n09q2_1_1-shortened-part2.IDI1']

# name of the resulting MS
msname = my_dataset_names[0]+'.ms'

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

        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/fitsidi_import/input/'
        for fname in my_dataset_names:
            if(os.path.exists(fname)):
                shutil.rm(fname)
            shutil.copy(datapath + fname, fname)
        default(importfitsidi)
        
    def tearDown(self):
        for fname in my_dataset_names:
            os.remove(fname)
        shutil.rmtree(msname,ignore_errors=True)
        shutil.rmtree(msname+'.flagversions',ignore_errors=True)
        
    def test1(self):
        '''fitsidi-import: Test good input'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    

        self.res = importfitsidi(my_dataset_names[0], msname)
        print myname, ": Success! Now checking output ..."
        mscomponents = set(["table.dat",
#                            "table.f0",
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
                         ['DATA',      42,[[ 1.06945515 +7.91624188e-09j,
                                             0.98315531 +9.31322575e-10j,
                                             1.05244470 +5.77396486e-09j,
                                             0.90496856 -0.00000000e+00j,
                                             0.93005872 -6.71682887e-09j,
                                             0.80769897 -0.00000000e+00j,
                                             0.93059886 -6.97121116e-09j,
                                             0.77081358 -0.00000000e+00j,
                                             0.93020338 -7.45058060e-09j,
                                             0.83353537 -0.00000000e+00j,
                                             0.91982168 -5.54113422e-09j,
                                             0.88411278 -4.65661287e-10j,
                                             1.02857709 +5.78550630e-09j,
                                             0.93398595 -0.00000000e+00j,
                                             1.13884020 +1.01289768e-08j,
                                             2.49237108 -0.00000000e+00j ]], 1E-8]
                         ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'
    
            expected = [
                         ['UVW',       638, [171529.37575288, -786712.70341456, 210321.20978818], 1E-8],
                         ['EXPOSURE',  638,  2.0, 1E-8],
                         ['DATA',      638, [[-0.00224198+0.00067056j,
                                              -0.00475123+0.0024323j,
                                              -0.00416393+0.00212671j,
                                              -0.00565350+0.00340364j,
                                              -0.00527357+0.00011977j,
                                              -0.00292699+0.00131954j,
                                              -0.00429945+0.00035823j,
                                              -0.00545671-0.00033945j,
                                              -0.00646004+0.00037293j,
                                              -0.00419376-0.00115011j,
                                              -0.00508117+0.00045939j,
                                              -0.00501660-0.00047975j,
                                              -0.00444734-0.00101535j,
                                              -0.00384988-0.00102731j,
                                              -0.00551326+0.00101364j,
                                              -0.00337701+0.00080481j]], 1E-8]
                         ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'
            
            name = "ANTENNA"
            expected = [ ['OFFSET',       1, [ 0.,  0.,  0.], 0],
                         ['POSITION',     1, [ 3370605.8469,  711917.6732,  5349830.8438], 0.0001],
                         ['DISH_DIAMETER',1, 0.0, 0.0] # the EVN default value
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
                
        self.assertTrue(retValue['success'])

    def test2(self):
        '''fitsidi-import: Test good input, list of two input files'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    

        self.res = importfitsidi([my_dataset_names[1],my_dataset_names[2]], msname)
        print myname, ": Success! Now checking output ..."
        mscomponents = set(["table.dat",
#                            "table.f0",
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
                         ['DATA',      42,[[ 1.06945515 +7.91624188e-09j,
                                             0.98315531 +9.31322575e-10j,
                                             1.05244470 +5.77396486e-09j,
                                             0.90496856 -0.00000000e+00j,
                                             0.93005872 -6.71682887e-09j,
                                             0.80769897 -0.00000000e+00j,
                                             0.93059886 -6.97121116e-09j,
                                             0.77081358 -0.00000000e+00j,
                                             0.93020338 -7.45058060e-09j,
                                             0.83353537 -0.00000000e+00j,
                                             0.91982168 -5.54113422e-09j,
                                             0.88411278 -4.65661287e-10j,
                                             1.02857709 +5.78550630e-09j,
                                             0.93398595 -0.00000000e+00j,
                                             1.13884020 +1.01289768e-08j,
                                             2.49237108 -0.00000000e+00j ]], 1E-8]
                         ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'
    
            expected = [
                         ['UVW',       638, [171529.37575288, -786712.70341456, 210321.20978818], 1E-8],
                         ['EXPOSURE',  638,  2.0, 1E-8],
                         ['DATA',      638, [[-0.00224198+0.00067056j,
                                              -0.00475123+0.0024323j,
                                              -0.00416393+0.00212671j,
                                              -0.00565350+0.00340364j,
                                              -0.00527357+0.00011977j,
                                              -0.00292699+0.00131954j,
                                              -0.00429945+0.00035823j,
                                              -0.00545671-0.00033945j,
                                              -0.00646004+0.00037293j,
                                              -0.00419376-0.00115011j,
                                              -0.00508117+0.00045939j,
                                              -0.00501660-0.00047975j,
                                              -0.00444734-0.00101535j,
                                              -0.00384988-0.00102731j,
                                              -0.00551326+0.00101364j,
                                              -0.00337701+0.00080481j]], 1E-8]
                         ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'
            
            name = "ANTENNA"
            expected = [ ['OFFSET',       1, [ 0.,  0.,  0.], 0],
                         ['POSITION',     1, [ 3370605.8469,  711917.6732,  5349830.8438], 0.0001],
                         ['DISH_DIAMETER',1, 0.0, 0.0] # the EVN default value
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
                
        self.assertTrue(retValue['success'])
                
    
def suite():
    return [test_importfitsidi]        
        
    
