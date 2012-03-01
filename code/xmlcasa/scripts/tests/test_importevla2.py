#############################################################################
# $Id:$
# Test Name:                                                                #
#    Regression Test Script for ASDM version 1.0 import to MS               #
#    and the "inverse filler" task importevla2 
#                                                                           #
# Rationale for Inclusion:                                                  #
#    The conversion of ASDM to MS and back needs to be verified.            #
#                                                                           # 
# Features tested:                                                          #
#    1) Is the import performed without raising exceptions                  #
#    2) Do all expected tables exist                                        #
#    3) Can the MS be opened                                                #
#    4) Do the tables contain expected values                               #
#    5) Is exportasdm performed without raising exceptions                  #
#    6) Is the created ASDM well-formed (XML) and complete                  #
#    7) Can the resulting ASDM be reimported without raising exceptions     #
#    8) Does it have the same number of integrations as the original        #
#                                                                           #
# Input data:                                                               #
#     one dataset for the filler of ASDM 1.0                                #
#     one simulated MS dataset                                              #
#                                                                           #
#############################################################################
import os
import sys
import shutil
import commands
from __main__ import default
from tasks import *
from taskinit import *
import unittest


def checktable(msname, thename, theexpectation):
    global myname
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
            return False
    tb.close()
    print myname, ": table ", thename, " as expected."
    return True

#########################

#def verify_asdm(asdmname, withPointing):
#    print "Verifying asdm ", asdmname
#    if(not os.path.exists(asdmname)):
#        print "asdm ", asdmname, " doesn't exist."
#        raise Exception
#    # test for the existence of all obligatory tables
#    allTables = [ "Antenna.xml",
#                  "ASDM.xml",
#                 # "CalData.xml",
#                 # "CalDelay.xml",
#                 # "CalReduction.xml",
#                  "ConfigDescription.xml",
#                  "CorrelatorMode.xml",
#                  "DataDescription.xml",
#                  "ExecBlock.xml",
#                  "Feed.xml",
#                  "Field.xml",
#                 #"FocusModel.xml",
#                 #"Focus.xml",
#                  "Main.xml",
#                  "PointingModel.xml",
#                  "Polarization.xml",
#                  "Processor.xml",
#                  "Receiver.xml",
#                  "SBSummary.xml",
#                  "Scan.xml",
#                  "Source.xml",
#                  "SpectralWindow.xml",
#                  "State.xml",
#                  "Station.xml",
#                  "Subscan.xml",
#                  "SwitchCycle.xml"
#                  ]
#    isOK = True
#    for fileName in allTables:
#        filePath = asdmname+'/'+fileName
#        if(not os.path.exists(filePath)):
#            print "ASDM table file ", filePath, " doesn't exist."
#            isOK = False
#        else:
#            # test if well formed
#            rval = os.system('xmllint --noout '+filePath)
#            if(rval !=0):
#                print "Table ", filePath, " is not a well formed XML document."
#                isOK = False
#
#    print "Note: xml validation not possible since ASDM DTDs (schemas) not yet online."
#        
#    if(not os.path.exists(asdmname+"/ASDMBinary")):
#        print "ASDM binary directory "+asdmname+"/ASDMBinary doesn't exist."
#        isOK = False
#
#    if(withPointing and not os.path.exists(asdmname+"/Pointing.bin")):
#        print "ASDM binary file "+asdmname+"/Pointing.bin doesn't exist."
#        isOK = False
#
#    if (not isOK):
#        raise Exception


###########################
# beginning of actual test 
myname = 'importevla2_ut'

# default ASDM dataset name
origname = 'TOSR0001_sb1308595_1.55294.83601028935'

asdmname = 'tosr0001'

# Copy SDM locally only once for all tests
if(not os.path.exists(asdmname)):
    datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/evla/'
    shutil.copytree(datapath + origname, asdmname)


class importevla2_test(unittest.TestCase):
    
    def setUp(self):
        res = None

#        if(not os.path.exists(myasdm_dataset_name)):
#            datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/evla/'
#            shutil.copytree(datapath + myasdm_dataset_name, myasdm_dataset_name)
        self.asdm = asdmname

        default(importevla2)
        
    def tearDown(self):
        pass
        
    def test1(self):
        '''Importevla2 test1: Default values'''
        self.res = importevla2()
        self.assertFalse(self.res)

    def test2(self):
        '''Importevla2 test2: Bad input asdm'''
        name = 'TOSR0001'
        self.res = importevla2(asdm=name)
        self.assertFalse(self.res)
        
    def test3(self):
        '''Importevla2 test3: Good input asdm'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    

        msname = self.asdm+'.ms'
        self.res = importevla2(asdm=self.asdm, scans='3')
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
                            "Antenna.xml",
                            "Flag.xml",
                            "SpectralWindow.xml",
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
                            "WEATHER/table.dat",
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
                            "SYSCAL/table.f0",
                            "WEATHER/table.f0"
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
    
            # check main table first
            name = ""
            #             col name, row number, expected value, tolerance
            expected = [
                         ['UVW',       42, [  47.48869115, 61.71087706 , -142.06903012], 1E-8],
                         ['EXPOSURE',  42, 1.0, 0]
#                         ['DATA',      42, [ [10.5526886+0.0j] ], 1E-7]
                         ]
            results = checktable(msname, name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table MAIN failed'
            else:
                retValue['success']=True
    
            expected = [
    # old values using TAI     ['UVW',       638, [-65.07623467,   1.05534109, -33.65801386], 1E-8],
                         ['UVW',       638, [-75.8725283, 239.64747061 , -154.46854611], 1E-8],
                         ['EXPOSURE',  638, 1.0, 0]
#                         ['DATA',      638, [ [0.00362284+0.00340279j] ], 1E-8]
                         ]
            results = checktable(msname, name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table MAIN failed'
            else:
                retValue['success']=True
            
            name = "ANTENNA"
            expected = [ ['OFFSET',       1, [ 0.,  0.,  0.], 0],
                         ['POSITION',     1, [-1601150.0595, -5042000.6198, 3554860.7294], 0.0001],
                         ['DISH_DIAMETER',1, 25.0, 0]
                         ]
            results = checktable(msname, name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table ANTENNA failed'
            else:
                retValue['success']=True
            
            name = "POINTING"
            expected = [ ['DIRECTION',       10, [[ 0.],[0.]], 1E-8],
                         ['INTERVAL',        10, 4.7672238080000007, 0],
                         ['TARGET',          10, [[ 0.], [ 0.]], 1E-8],
                         ['TIME',            10, 4777473953.6163883, 0],
                         ['TIME_ORIGIN',     10, 0., 0],
                         ['POINTING_OFFSET', 10, [[ 0.],[ 0.]], 0],
                         ['ENCODER',         10, [ 0.,  0.], 1E-8 ]
                         ]
            results = checktable(msname, name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table POINTING failed'
            else:
                retValue['success']=True
                
        self.assertTrue(results)


    def test_apply1(self):
        '''importevla2: apply all flags and save to file'''
        msname = 'online.ms'
        if os.path.exists(msname):
            os.system('rm -rf online.ms*')
            
        importevla2(asdm=self.asdm, vis=msname, scans='3',online=True, applyflags=True,
                    shadow=True,flagzero=True,flagbackup=False)
        
        # Check flags
        res = tflagdata(vis=msname, mode='summary')
        self.assertEqual(res['flagged'],294400)
        
        # Check output file existence
        cmdfile='online_cmd.txt'
        self.assertTrue(os.path.exists(cmdfile))
        
        # Check file content
        ff = open(cmdfile,'r')
        cmdlist = ff.readlines()
        ncmds = cmdlist.__len__()
        self.assertEqual(ncmds, 2, 'Only shadow and clip zeros should be saved to file')
        
        
    def test_apply2(self):
        '''importevla2: apply flags and save also online flags to file'''
        msname = 'applied.ms'
        cmdfile = 'mycmds.txt'
        if os.path.exists(msname):
            os.system('rm -rf '+msname)
        if os.path.exists(cmdfile):
            os.system('rm -rf '+cmdfile)
            
        # Save to different file
        importevla2(asdm=self.asdm, vis=msname, scans='3',online=True, shadow=True, flagzero=True,
                    applyflags=True,savetofile=True, savecmds=True, outfile=cmdfile, flagbackup=False)
        
        # Check flags only in RR and LL
        res = tflagdata(vis=msname, mode='summary')
        self.assertEqual(res['flagged'],294400)
        
        # Check output file existence
        self.assertTrue(os.path.exists(cmdfile))
        
        # Check file content
        ff = open(cmdfile,'r')
        cmdlist = ff.readlines()
        ncmds = cmdlist.__len__()
        ff.close()
        self.assertEqual(ncmds, 509, 'Online, shadow and clip zeros should be saved to file')

    def test_apply3(self):
        '''importevla2: apply clip zeros on RR and save to file'''
        msname = 'zeros.ms'
        if os.path.exists(msname):
            os.system('rm -rf zeros.ms*')
            
        importevla2(asdm=self.asdm, vis=msname, scans='3',online=False, applyflags=True,
                    shadow=False,flagzero=True,flagpol=False, flagbackup=False)
        
        # Check flags
        res = tflagdata(vis=msname, mode='summary')
        self.assertEqual(res['flagged'],0,'There are no zeros in this data set')
        
        # Check output file existence
        self.assertTrue(os.path.exists('zeros_cmd.txt'))

        # Check output file existence
        cmdfile='zeros_cmd.txt'
        self.assertTrue(os.path.exists(cmdfile))
        
        # Check file content
        ff = open(cmdfile,'r')
        cmdlist = ff.readlines()
        ncmds = cmdlist.__len__()
        self.assertEqual(ncmds, 1, 'Only clip zeros should be saved to file')
        
        
    def test_savepars(self):
        '''importevla2: save the flag commands and do not apply'''
        msname = 'notapplied.ms'
        if os.path.exists(msname):
            os.system('rm -rf '+msname)

        importevla2(asdm=self.asdm, vis=msname,scans='3',online=True,flagzero=True,shadow=True,savecmds=True,
                    applyflags=False,flagbackup=False)
        
        # Check flags
        res = tflagdata(vis=msname, mode='summary')
        self.assertEqual(res['flagged'],0,'No flags should have been applied')

        # Check output file existence
        cmdfile = 'notapplied_cmd.txt'
        self.assertTrue(os.path.exists(cmdfile))
        
        # Check file content
        ff = open(cmdfile,'r')
        cmdlist = ff.readlines()
        ncmds = cmdlist.__len__()
        ff.close()
        self.assertEqual(ncmds, 2, 'Online, shadow and clip zeros should be saved to file')
        
        # Apply flags using tflagdata
        tflagdata(vis=msname, mode='list', inpfile=cmdfile)
        
        res = tflagdata(vis=msname, mode='summary')
        self.assertEqual(res['flagged'],240640)

        

class cleanup(unittest.TestCase):

    def setUp(self):
        pass
    
    def tearDown(self):
        shutil.rmtree(asdmname, ignore_errors=True)
#        shutil.rmtree(msname,ignore_errors=True)
#        shutil.rmtree('*_cmd.txt', ignore_errors=True)
        
    def test1a(self):
        '''Importevla: Cleanup'''
        pass
                    
def suite():
    return [importevla2_test, cleanup]        
        
    