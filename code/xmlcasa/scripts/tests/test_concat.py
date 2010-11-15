#############################################################################
# $Id:$
# Test Name:                                                                #
#    Regression Test Script for the concat task
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

myname = 'test_concat'

# name of the resulting MS
msname = 'concatenated.ms'

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

class test_concat(unittest.TestCase):
    
    def setUp(self):
        res = None

        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/concat/input/'
        cpath = os.path.abspath(os.curdir)
        filespresent = sorted(glob.glob("*.ms"))
        os.chdir(datapath)
        for mymsname in sorted(glob.glob("*.ms")):
            if not mymsname in filespresent:
                print "Copying ", mymsname
                shutil.copytree(mymsname, cpath+'/'+mymsname)
        os.chdir(cpath)

        default(concat)
        
    def tearDown(self):
        shutil.rmtree(msname,ignore_errors=True)

    def test1(self):
        '''Concat 1: 4 parts, same sources but different spws'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    
        
        self.res = concat(vis=['part1.ms','part2.ms','part3.ms','part4.ms'],concatvis=msname)
        self.assertEqual(self.res,None)

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
            if 'test1.ms' in glob.glob("*.ms"):
                shutil.rmtree('test1.ms',ignore_errors=True)
            shutil.copytree(msname,'test1.ms')
            print myname, ": OK. Checking tables in detail ..."
            retValue['success']=True

            # check source table
            name = "SOURCE"
            #             col name, row number, expected value, tolerance
            expected = [
                ['SOURCE_ID',           55, 13, 0],
                ['SPECTRAL_WINDOW_ID',  55, 3, 0]
                ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'
            # check spw table
            name = "SPECTRAL_WINDOW"
            #             col name, row number, expected value, tolerance
            expected = [
                ['NUM_CHAN',           3, 128, 0]
                ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'


    def test2(self):
        '''Concat 2: 3 parts, different sources, different spws '''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    
        self.res = concat(vis=['part1.ms','part2-mod.ms','part3.ms'],concatvis=msname)
        self.assertEqual(self.res,None)
        
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
            if 'test2.ms' in glob.glob("*.ms"):
                shutil.rmtree('test2.ms',ignore_errors=True)
            shutil.copytree(msname,'test2.ms')
            print myname, ": OK. Checking tables in detail ..."
            retValue['success']=True

            # check source table
            name = "SOURCE"
            #             col name, row number, expected value, tolerance
            expected = [
                ['SOURCE_ID',           41, 13, 0],
                ['SPECTRAL_WINDOW_ID',  41, 2, 0]
                ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'
            # check spw table
            name = "SPECTRAL_WINDOW"
            #             col name, row number, expected value, tolerance
            expected = [
                ['NUM_CHAN',           2, 128, 0]
                ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'


    def test3(self):
        '''Concat 3: 3 parts, different sources, same spws'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    
        self.res = concat(vis=['part1.ms','part2-mod2.ms','part3.ms'],concatvis=msname)
        self.assertEqual(self.res,None)

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
            if 'test3.ms' in glob.glob("*.ms"):
                shutil.rmtree('test3.ms',ignore_errors=True)
            shutil.copytree(msname,'test3.ms')
            print myname, ": OK. Checking tables in detail ..."
            retValue['success']=True

            # check source table
            name = "SOURCE"
            #             col name, row number, expected value, tolerance
            expected = [
                ['SOURCE_ID',           28, 13, 0],
                ['SPECTRAL_WINDOW_ID',  28, 1, 0]
                ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'
            # check spw table
            name = "SPECTRAL_WINDOW"
            #             col name, row number, expected value, tolerance
            expected = [
                ['NUM_CHAN',           1, 128, 0]
                ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'
    

    def test4(self):
        '''Concat 4: five MSs with identical sources but different time/intervals on them (CSV-268)'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        
        self.res = concat(vis = ['shortpart1.ms', 'shortpart2.ms', 'shortpart3.ms', 'shortpart4.ms', 'shortpart5.ms'],
                          concatvis = msname)
        self.assertEqual(self.res,None)

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
            if 'test4.ms' in glob.glob("*.ms"):
                shutil.rmtree('test4.ms',ignore_errors=True)
            shutil.copytree(msname,'test4.ms')
            print myname, ": OK. Checking tables in detail ..."
            retValue['success']=True
        
    
            # check source table
            name = "SOURCE"
            #             col name, row number, expected value, tolerance
            expected = [
                ['SOURCE_ID',           0, 0, 0],
                ['SPECTRAL_WINDOW_ID',  0, 0, 0]
                ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'
            expected = [
                ['SOURCE_ID',           7, 0, 0],
                ['SPECTRAL_WINDOW_ID',  7, 7, 0]
                ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'
            expected = [
                ['SOURCE_ID',           8, 1, 0],
                ['SPECTRAL_WINDOW_ID',  8, 0, 0]
                ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'
            expected = [
                ['SOURCE_ID',           15, 1, 0],
                ['SPECTRAL_WINDOW_ID',  15, 7, 0]
                ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'
            expected = [
                ['SOURCE_ID',           16, 0, 100000],
                ['SPECTRAL_WINDOW_ID',  16, 0, 100000]
                ]
            print "The following should fail: SOURCE row 16 should not exist"
            try:
                results = checktable(name, expected)
            except:
                print "Expected error."
                results = False
            if results: 
                retValue['success']=False
                retValue['error_msgs']='SOURCE row 16 should not existCheck of table '+name+' failed'
            # check spw table
            name = "SPECTRAL_WINDOW"
            #             col name, row number, expected value, tolerance
            expected = [
                ['NUM_CHAN',           8, 4, 0]
                ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'
    
                
        self.assertTrue(retValue['success'])
        
    def test5(self):
        '''Concat 5: two MSs with different state table (CAS-2601)'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        
        self.res = concat(vis = ['A2256LC2_4.5s-1.ms','A2256LC2_4.5s-2.ms'],
                          concatvis = msname)
        self.assertEqual(self.res,None)

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
            if 'test5.ms' in glob.glob("*.ms"):
                shutil.rmtree('test5.ms',ignore_errors=True)
            shutil.copytree(msname,'test5.ms')
            print myname, ": OK. Checking tables in detail ..."
            retValue['success']=True        
    
            # check state table
            name = "STATE"
            #             col name, row number, expected value, tolerance
            expected = [
                ['CAL',  0, 0, 0],
                ['SIG',  0, 1, 0],
                ['SUB_SCAN',  2, 1, 0]
                ]
            results = checktable(name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'
                retValue['error_msgs']=retValue['error_msgs']+'Check of table '+name+' failed'
                
        self.assertTrue(retValue['success'])

class concat_cleanup(unittest.TestCase):           
    def setUp(self):
        pass
    
    def tearDown(self):
        os.system('rm -rf *part*.ms')   
        os.system('rm -rf test*.ms')

    def testrun(self):
        '''Concat: Cleanup'''
        pass
    
def suite():
    return [test_concat,concat_cleanup]        
        
