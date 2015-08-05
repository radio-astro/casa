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
                    'n09q2_1_1-shortened-part2.IDI1',
                    'emerlin_multiuv.IDI1']

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
            if isinstance(value, basestring):
                in_agreement = value == mycell[2]
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
                os.remove(fname)
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

        self.res = importfitsidi(my_dataset_names[0], msname,  scanreindexgap_s=100., constobsid=True)
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

    def test3(self):
        '''fitsidi-import: Test antenna name and multi uvtable'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    

        self.res = importfitsidi(my_dataset_names[3], msname)
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
            # this is the first row of the second uv table
            expected = [
                         ['UVW',       253, [ -209524.112917, 52443.4688607, 25501.833085 ], 1E-6],
                         ['EXPOSURE',  253, 1.0, 1E-8],
                         ['WEIGHT',    253, 1.0, 1E-8]
                         ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'
    
            
            name = "ANTENNA"
            expected = [ ['OFFSET',       1, [ 0.,  0.,  0.], 0],
                         ['POSITION',     1, [ 3859711.503, -201995.077, 5056134.251], 0.001],
                         ['DISH_DIAMETER',1, 15.0, 0.001],
                         ['NAME',         1, 'Kn', 0]
                         ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'
            
            name = "SPECTRAL_WINDOW"
            expected = [ ['NUM_CHAN',        3, 512, 0],
                         ['TOTAL_BANDWIDTH', 3, 128e+6, 0],
                         ['CHAN_WIDTH',      3, [250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,
  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000.,  250000. ], 1E-8],
                         ['CHAN_FREQ',       3, [   5.87200000e+09,   5.87225000e+09,   5.87250000e+09,   5.87275000e+09,
   5.87300000e+09,   5.87325000e+09,   5.87350000e+09,   5.87375000e+09,
   5.87400000e+09,   5.87425000e+09,   5.87450000e+09,   5.87475000e+09,
   5.87500000e+09,   5.87525000e+09,   5.87550000e+09,   5.87575000e+09,
   5.87600000e+09,   5.87625000e+09,   5.87650000e+09,   5.87675000e+09,
   5.87700000e+09,   5.87725000e+09,   5.87750000e+09,   5.87775000e+09,
   5.87800000e+09,   5.87825000e+09,   5.87850000e+09,   5.87875000e+09,
   5.87900000e+09,   5.87925000e+09,   5.87950000e+09,   5.87975000e+09,
   5.88000000e+09,   5.88025000e+09,   5.88050000e+09,   5.88075000e+09,
   5.88100000e+09,   5.88125000e+09,   5.88150000e+09,   5.88175000e+09,
   5.88200000e+09,   5.88225000e+09,   5.88250000e+09,   5.88275000e+09,
   5.88300000e+09,   5.88325000e+09,   5.88350000e+09,   5.88375000e+09,
   5.88400000e+09,   5.88425000e+09,   5.88450000e+09,   5.88475000e+09,
   5.88500000e+09,   5.88525000e+09,   5.88550000e+09,   5.88575000e+09,
   5.88600000e+09,   5.88625000e+09,   5.88650000e+09,   5.88675000e+09,
   5.88700000e+09,   5.88725000e+09,   5.88750000e+09,   5.88775000e+09,
   5.88800000e+09,   5.88825000e+09,   5.88850000e+09,   5.88875000e+09,
   5.88900000e+09,   5.88925000e+09,   5.88950000e+09,   5.88975000e+09,
   5.89000000e+09,   5.89025000e+09,   5.89050000e+09,   5.89075000e+09,
   5.89100000e+09,   5.89125000e+09,   5.89150000e+09,   5.89175000e+09,
   5.89200000e+09,   5.89225000e+09,   5.89250000e+09,   5.89275000e+09,
   5.89300000e+09,   5.89325000e+09,   5.89350000e+09,   5.89375000e+09,
   5.89400000e+09,   5.89425000e+09,   5.89450000e+09,   5.89475000e+09,
   5.89500000e+09,   5.89525000e+09,   5.89550000e+09,   5.89575000e+09,
   5.89600000e+09,   5.89625000e+09,   5.89650000e+09,   5.89675000e+09,
   5.89700000e+09,   5.89725000e+09,   5.89750000e+09,   5.89775000e+09,
   5.89800000e+09,   5.89825000e+09,   5.89850000e+09,   5.89875000e+09,
   5.89900000e+09,   5.89925000e+09,   5.89950000e+09,   5.89975000e+09,
   5.90000000e+09,   5.90025000e+09,   5.90050000e+09,   5.90075000e+09,
   5.90100000e+09,   5.90125000e+09,   5.90150000e+09,   5.90175000e+09,
   5.90200000e+09,   5.90225000e+09,   5.90250000e+09,   5.90275000e+09,
   5.90300000e+09,   5.90325000e+09,   5.90350000e+09,   5.90375000e+09,
   5.90400000e+09,   5.90425000e+09,   5.90450000e+09,   5.90475000e+09,
   5.90500000e+09,   5.90525000e+09,   5.90550000e+09,   5.90575000e+09,
   5.90600000e+09,   5.90625000e+09,   5.90650000e+09,   5.90675000e+09,
   5.90700000e+09,   5.90725000e+09,   5.90750000e+09,   5.90775000e+09,
   5.90800000e+09,   5.90825000e+09,   5.90850000e+09,   5.90875000e+09,
   5.90900000e+09,   5.90925000e+09,   5.90950000e+09,   5.90975000e+09,
   5.91000000e+09,   5.91025000e+09,   5.91050000e+09,   5.91075000e+09,
   5.91100000e+09,   5.91125000e+09,   5.91150000e+09,   5.91175000e+09,
   5.91200000e+09,   5.91225000e+09,   5.91250000e+09,   5.91275000e+09,
   5.91300000e+09,   5.91325000e+09,   5.91350000e+09,   5.91375000e+09,
   5.91400000e+09,   5.91425000e+09,   5.91450000e+09,   5.91475000e+09,
   5.91500000e+09,   5.91525000e+09,   5.91550000e+09,   5.91575000e+09,
   5.91600000e+09,   5.91625000e+09,   5.91650000e+09,   5.91675000e+09,
   5.91700000e+09,   5.91725000e+09,   5.91750000e+09,   5.91775000e+09,
   5.91800000e+09,   5.91825000e+09,   5.91850000e+09,   5.91875000e+09,
   5.91900000e+09,   5.91925000e+09,   5.91950000e+09,   5.91975000e+09,
   5.92000000e+09,   5.92025000e+09,   5.92050000e+09,   5.92075000e+09,
   5.92100000e+09,   5.92125000e+09,   5.92150000e+09,   5.92175000e+09,
   5.92200000e+09,   5.92225000e+09,   5.92250000e+09,   5.92275000e+09,
   5.92300000e+09,   5.92325000e+09,   5.92350000e+09,   5.92375000e+09,
   5.92400000e+09,   5.92425000e+09,   5.92450000e+09,   5.92475000e+09,
   5.92500000e+09,   5.92525000e+09,   5.92550000e+09,   5.92575000e+09,
   5.92600000e+09,   5.92625000e+09,   5.92650000e+09,   5.92675000e+09,
   5.92700000e+09,   5.92725000e+09,   5.92750000e+09,   5.92775000e+09,
   5.92800000e+09,   5.92825000e+09,   5.92850000e+09,   5.92875000e+09,
   5.92900000e+09,   5.92925000e+09,   5.92950000e+09,   5.92975000e+09,
   5.93000000e+09,   5.93025000e+09,   5.93050000e+09,   5.93075000e+09,
   5.93100000e+09,   5.93125000e+09,   5.93150000e+09,   5.93175000e+09,
   5.93200000e+09,   5.93225000e+09,   5.93250000e+09,   5.93275000e+09,
   5.93300000e+09,   5.93325000e+09,   5.93350000e+09,   5.93375000e+09,
   5.93400000e+09,   5.93425000e+09,   5.93450000e+09,   5.93475000e+09,
   5.93500000e+09,   5.93525000e+09,   5.93550000e+09,   5.93575000e+09,
   5.93600000e+09,   5.93625000e+09,   5.93650000e+09,   5.93675000e+09,
   5.93700000e+09,   5.93725000e+09,   5.93750000e+09,   5.93775000e+09,
   5.93800000e+09,   5.93825000e+09,   5.93850000e+09,   5.93875000e+09,
   5.93900000e+09,   5.93925000e+09,   5.93950000e+09,   5.93975000e+09,
   5.94000000e+09,   5.94025000e+09,   5.94050000e+09,   5.94075000e+09,
   5.94100000e+09,   5.94125000e+09,   5.94150000e+09,   5.94175000e+09,
   5.94200000e+09,   5.94225000e+09,   5.94250000e+09,   5.94275000e+09,
   5.94300000e+09,   5.94325000e+09,   5.94350000e+09,   5.94375000e+09,
   5.94400000e+09,   5.94425000e+09,   5.94450000e+09,   5.94475000e+09,
   5.94500000e+09,   5.94525000e+09,   5.94550000e+09,   5.94575000e+09,
   5.94600000e+09,   5.94625000e+09,   5.94650000e+09,   5.94675000e+09,
   5.94700000e+09,   5.94725000e+09,   5.94750000e+09,   5.94775000e+09,
   5.94800000e+09,   5.94825000e+09,   5.94850000e+09,   5.94875000e+09,
   5.94900000e+09,   5.94925000e+09,   5.94950000e+09,   5.94975000e+09,
   5.95000000e+09,   5.95025000e+09,   5.95050000e+09,   5.95075000e+09,
   5.95100000e+09,   5.95125000e+09,   5.95150000e+09,   5.95175000e+09,
   5.95200000e+09,   5.95225000e+09,   5.95250000e+09,   5.95275000e+09,
   5.95300000e+09,   5.95325000e+09,   5.95350000e+09,   5.95375000e+09,
   5.95400000e+09,   5.95425000e+09,   5.95450000e+09,   5.95475000e+09,
   5.95500000e+09,   5.95525000e+09,   5.95550000e+09,   5.95575000e+09,
   5.95600000e+09,   5.95625000e+09,   5.95650000e+09,   5.95675000e+09,
   5.95700000e+09,   5.95725000e+09,   5.95750000e+09,   5.95775000e+09,
   5.95800000e+09,   5.95825000e+09,   5.95850000e+09,   5.95875000e+09,
   5.95900000e+09,   5.95925000e+09,   5.95950000e+09,   5.95975000e+09,
   5.96000000e+09,   5.96025000e+09,   5.96050000e+09,   5.96075000e+09,
   5.96100000e+09,   5.96125000e+09,   5.96150000e+09,   5.96175000e+09,
   5.96200000e+09,   5.96225000e+09,   5.96250000e+09,   5.96275000e+09,
   5.96300000e+09,   5.96325000e+09,   5.96350000e+09,   5.96375000e+09,
   5.96400000e+09,   5.96425000e+09,   5.96450000e+09,   5.96475000e+09,
   5.96500000e+09,   5.96525000e+09,   5.96550000e+09,   5.96575000e+09,
   5.96600000e+09,   5.96625000e+09,   5.96650000e+09,   5.96675000e+09,
   5.96700000e+09,   5.96725000e+09,   5.96750000e+09,   5.96775000e+09,
   5.96800000e+09,   5.96825000e+09,   5.96850000e+09,   5.96875000e+09,
   5.96900000e+09,   5.96925000e+09,   5.96950000e+09,   5.96975000e+09,
   5.97000000e+09,   5.97025000e+09,   5.97050000e+09,   5.97075000e+09,
   5.97100000e+09,   5.97125000e+09,   5.97150000e+09,   5.97175000e+09,
   5.97200000e+09,   5.97225000e+09,   5.97250000e+09,   5.97275000e+09,
   5.97300000e+09,   5.97325000e+09,   5.97350000e+09,   5.97375000e+09,
   5.97400000e+09,   5.97425000e+09,   5.97450000e+09,   5.97475000e+09,
   5.97500000e+09,   5.97525000e+09,   5.97550000e+09,   5.97575000e+09,
   5.97600000e+09,   5.97625000e+09,   5.97650000e+09,   5.97675000e+09,
   5.97700000e+09,   5.97725000e+09,   5.97750000e+09,   5.97775000e+09,
   5.97800000e+09,   5.97825000e+09,   5.97850000e+09,   5.97875000e+09,
   5.97900000e+09,   5.97925000e+09,   5.97950000e+09,   5.97975000e+09,
   5.98000000e+09,   5.98025000e+09,   5.98050000e+09,   5.98075000e+09,
   5.98100000e+09,   5.98125000e+09,   5.98150000e+09,   5.98175000e+09,
   5.98200000e+09,   5.98225000e+09,   5.98250000e+09,   5.98275000e+09,
   5.98300000e+09,   5.98325000e+09,   5.98350000e+09,   5.98375000e+09,
   5.98400000e+09,   5.98425000e+09,   5.98450000e+09,   5.98475000e+09,
   5.98500000e+09,   5.98525000e+09,   5.98550000e+09,   5.98575000e+09,
   5.98600000e+09,   5.98625000e+09,   5.98650000e+09,   5.98675000e+09,
   5.98700000e+09,   5.98725000e+09,   5.98750000e+09,   5.98775000e+09,
   5.98800000e+09,   5.98825000e+09,   5.98850000e+09,   5.98875000e+09,
   5.98900000e+09,   5.98925000e+09,   5.98950000e+09,   5.98975000e+09,
   5.99000000e+09,   5.99025000e+09,   5.99050000e+09,   5.99075000e+09,
   5.99100000e+09,   5.99125000e+09,   5.99150000e+09,   5.99175000e+09,
   5.99200000e+09,   5.99225000e+09,   5.99250000e+09,   5.99275000e+09,
   5.99300000e+09,   5.99325000e+09,   5.99350000e+09,   5.99375000e+09,
   5.99400000e+09,   5.99425000e+09,   5.99450000e+09,   5.99475000e+09,
   5.99500000e+09,   5.99525000e+09,   5.99550000e+09,   5.99575000e+09,
   5.99600000e+09,   5.99625000e+09,   5.99650000e+09,   5.99675000e+09,
   5.99700000e+09,   5.99725000e+09,   5.99750000e+09,   5.99775000e+09,
   5.99800000e+09,   5.99825000e+09,   5.99850000e+09,   5.99875000e+09,
   5.99900000e+09,   5.99925000e+09,   5.99950000e+09,   5.99975000e+09,], 1E-8]
                         ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'
                
        self.assertTrue(retValue['success'])

    def test4(self):
        '''fitsidi-import: Test good input, list of two input files, constobsid and scanreindexing'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    

        self.res = importfitsidi([my_dataset_names[1],my_dataset_names[2]], msname, 
                                 constobsid=True, scanreindexgap_s=1.5)
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
                                             2.49237108 -0.00000000e+00j ]], 1E-8],
                         ['OBSERVATION_ID', 42, 0, 0],
                         ['SCAN_NUMBER', 42, 1, 0]
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
                                              -0.00337701+0.00080481j]], 1E-8],
                         ['OBSERVATION_ID', 638, 0, 0],
                         ['SCAN_NUMBER', 638, 14, 0]
                         ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'
            
            tb.open(msname+'/OBSERVATION')
            nr = tb.nrows()
            tb.close()
            if not nr==1:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table OBSERVATION failed'
                
        self.assertTrue(retValue['success'])
                
    
def suite():
    return [test_importfitsidi]
