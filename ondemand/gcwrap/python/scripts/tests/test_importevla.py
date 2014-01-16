#############################################################################
# $Id:$
# Test Name:                                                                #
#    Regression Test Script for ASDM version 1.0 import to MS               #
#    and the "inverse filler" task importevla 
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


myname = 'importevla_ut'


class importevla_test(unittest.TestCase):
    
    def setUp(self):
#        res = None

        origname = 'X_osro_013.55979.93803716435'
        asdmname = 'xosro'
        if (not os.path.exists(asdmname)):
            datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/importevla/'
            os.system('ln -s '+datapath+origname+' '+asdmname)
            
        self.asdm = asdmname

        default(importevla)
        
                
    def test1(self):
        '''Importevla test1: Good input asdm'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    

        msname = 'xosro1.ms'
#        self.res = importevla(asdm=self.asdm, scans='3')
        self.res = importevla(asdm=self.asdm, vis=msname, scans='2')
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
                         ['UVW',       42, [  1607.50778695, -1241.40287976 , 584.50368163], 1E-8],
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
                         ['UVW',       638, [14.20193237, 722.59606805 , 57.57988905], 1E-8],
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
            expected = [ ['OFFSET',       1, [ -4.80000000e-12,  0.,  0.], 0],
                         ['POSITION',     1, [-1599644.8611, -5042953.6623, 3554197.0332], 0.0001],
                         ['DISH_DIAMETER',1, 25.0, 0]
                         ]
            results = checktable(msname, name, expected)
            if not results:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+'Check of table ANTENNA failed'
            else:
                retValue['success']=True
            
#            name = "POINTING"
#            expected = [ ['DIRECTION',       10, [[ 0.],[0.]], 1E-8],
#                         ['INTERVAL',        10, 4.7672238080000007, 0],
#                         ['TARGET',          10, [[ 0.], [ 0.]], 1E-8],
#                         ['TIME',            10, 4777473953.6163883, 0],
#                         ['TIME_ORIGIN',     10, 0., 0],
#                         ['POINTING_OFFSET', 10, [[ 0.],[ 0.]], 0],
#                         ['ENCODER',         10, [ 0.,  0.], 1E-8 ]
#                         ]
#            results = checktable(msname, name, expected)
#            if not results:
#                retValue['success']=False
#                retValue['error_msgs']=retValue['error_msgs']+'Check of table POINTING failed'
#            else:
#                retValue['success']=True
                
        self.assertTrue(results)


    def test_apply1(self):
        '''importevla: apply all flags and save to file'''
        msname = 'online.ms'
        cmdfile = 'online_cmd.txt'
        if os.path.exists(msname):
            os.system('rm -rf '+msname)
        if os.path.exists(cmdfile):
            os.system('rm -rf '+cmdfile)
            
        importevla(asdm=self.asdm, vis=msname, scans='2',online=True, applyflags=True,
                    shadow=True,flagzero=True,flagbackup=False, savecmds=True)
        
        # Check flags
        res = flagdata(vis=msname, mode='summary')
        self.assertEqual(res['flagged'],2446080)
        
        # Check output file existence
        self.assertTrue(os.path.exists(cmdfile))
        
        # Check file content
        ff = open(cmdfile,'r')
        cmdlist = ff.readlines()
        ncmds = cmdlist.__len__()
        self.assertEqual(ncmds, 216)
        
        
    def test_apply2(self):
        '''importevla: apply only online flags'''
        msname = 'applied.ms'
        if os.path.exists(msname):
            os.system('rm -rf '+msname)
            
        # Save to different file
        importevla(asdm=self.asdm, vis=msname, scans='2',online=True, shadow=False, flagzero=False,
                    applyflags=True, savecmds=False, flagbackup=False)
        
        # Check flags only in RR and LL
        res = flagdata(vis=msname, mode='summary')
        self.assertEqual(res['flagged'],2446080)
        self.assertEqual(res['scan']['2']['flagged'],2446080)
        

    def test_apply3(self):
        '''importevla: apply clip zeros on RR and LL and save to file'''
        msname = 'zeros.ms'
        cmdfile = 'zeros_cmd.txt'
        if os.path.exists(msname):
            os.system('rm -rf '+msname)
        if os.path.exists(cmdfile):
            os.system('rm -rf '+cmdfile)
            
            
        importevla(asdm=self.asdm, vis=msname, scans='2,13',online=False, applyflags=True,
                    shadow=False,flagzero=True,flagpol=False, flagbackup=False, savecmds=True)
        
        # Check flags
        res = flagdata(vis=msname, mode='summary')
        self.assertEqual(res['flagged'],0,'There are no zeros in this data set')
        self.assertEqual(res['scan']['2']['flagged'],0,'No flags should have been applied')
        self.assertEqual(res['scan']['13']['flagged'],0,'No flags should have been applied')
        
        # Check output file existence
        self.assertTrue(os.path.exists(cmdfile))

        # Check output file existence
        self.assertTrue(os.path.exists(cmdfile))
        
        # Check file content
        ff = open(cmdfile,'r')
        cmdlist = ff.readlines()
        ncmds = cmdlist.__len__()
        self.assertEqual(ncmds, 2, 'Only clip zeros should be saved to file')
        
    def test_apply4(self):
        '''importevla: Save online flags to FLAG_CMD and file; do not apply'''

        # Use default msname and outfile
        msname = 'xosro4.ms'
        cmdfile = msname.replace('.ms','_cmd.txt')
        if os.path.exists(msname):
            os.system('rm -rf '+msname)
        if os.path.exists(cmdfile):
            os.system('rm -rf '+cmdfile)
            
        importevla(asdm=self.asdm, vis=msname, scans='2',online=True, shadow=False, flagzero=False,
                    applyflags=False,savecmds=True, flagbackup=False)

        # No flags were applied
        res = flagdata(vis=msname, mode='summary')
        self.assertEqual(res['flagged'],0)
        
        # Apply only row 213 using flagcmd
        # The command in row 213 is the following:
        # antenna='ea06' timerange='2012/02/22/22:30:55.200~2012/02/22/22:35:08.199' 
        # spw='EVLA_X#A0C0#0' correlation='LL,LR,RL
        flagcmd(vis=msname, action='apply', tablerows=213)
        
        # Check flags. RR should no be flagged
        res = flagdata(vis=msname, mode='summary')
        self.assertEqual(res['correlation']['RR']['flagged'],0,'RR should not be flagged')
        self.assertEqual(res['correlation']['LL']['flagged'],29440)
        self.assertEqual(res['correlation']['LR']['flagged'],29440)
        self.assertEqual(res['correlation']['RL']['flagged'],29440)
        self.assertEqual(res['antenna']['ea06']['flagged'],88320)
        self.assertEqual(res['antenna']['ea07']['flagged'],3840,'Only a few baselines should be flagged')
        self.assertEqual(res['antenna']['ea08']['flagged'],3840,'Only a few baselines should be flagged')
        
        # Check output file existence       
        self.assertTrue(os.path.exists(cmdfile))
        
        # Check file content
        ff = open(cmdfile,'r')
        cmdlist = ff.readlines()
        ncmds = cmdlist.__len__()
        ff.close()
        self.assertEqual(ncmds, 214, 'Only Online cmds should have been saved to file')
        
        # Unapply row 213 and apply it in flagdata using the file
        # TO DO : after row selection is available in file

    def test_apply5(self):
        '''importevla: Apply only shadow flags'''
        msname = 'xosro_shadow.ms'
        importevla(asdm=self.asdm, vis=msname,online=False, shadow=True, flagzero=False,
                    applyflags=True,savecmds=False, flagbackup=False)
        
        # This data set doesn't have shadow. Maybe change later!
        res = flagdata(vis=msname, mode='summary')
        self.assertEqual(res['flagged'],0,'There are shadowed antenna in this data set')


        
    def test_savepars(self):
        '''importevla: save the flag commands and do not apply'''
        msname = 'notapplied.ms'
        cmdfile = msname.replace('.ms','_cmd.txt')
        if os.path.exists(msname):
            os.system('rm -rf '+msname)
        if os.path.exists(cmdfile):
            os.system('rm -rf '+cmdfile)

        importevla(asdm=self.asdm, vis=msname,scans='11~13',online=True,flagzero=True,shadow=True,savecmds=True,
                    applyflags=False,flagbackup=False)
        
        # Check flags
        res = flagdata(vis=msname, mode='summary')
        self.assertEqual(res['flagged'],0,'No flags should have been applied')

        # Check output file existence
        self.assertTrue(os.path.exists(cmdfile))
        
        # Check file content
        ff = open(cmdfile,'r')
        cmdlist = ff.readlines()
        ncmds = cmdlist.__len__()
        ff.close()
        self.assertEqual(ncmds, 216, 'Online, shadow and clip zeros should be saved to file')
        
        # Apply flags using flagdata
        flagdata(vis=msname, mode='list', inpfile=cmdfile)
        
        res = flagdata(vis=msname, mode='summary')
        self.assertEqual(res['flagged'],6090624)
                

class cleanup(unittest.TestCase):

    def setUp(self):
        pass
    
    def tearDown(self):
        os.system('rm -rf *applied*ms*')
        os.system('rm -rf *online*ms*')
        os.system('rm -rf *zeros*ms*')
        os.system('rm -rf *xosro*.ms*')
#        pass
           
    def test1a(self):
        '''Importevla: Cleanup'''
        pass
                    
def suite():
    return [importevla_test, cleanup]        
        
    