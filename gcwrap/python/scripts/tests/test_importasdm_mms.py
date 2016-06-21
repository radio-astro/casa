#############################################################################
# $Id:$
# Test Name:                                                                #
#    Regression Test Script for ASDM version 1.2, 1.3 import to MS          #
#    and the "inverse filler" task exportasdm 
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
#    9) Can a mixed mode ASDM be imported in lazy mode                      #
#                                                                           #
# Input data:                                                               #
#     one dataset for the filler of ASDM 1.0                                #
#     one simulated MS dataset                                              #
#                                                                           #
#############################################################################
import os
import sys
import shutil
from __main__ import default
from tasks import importasdm, flagdata, exportasdm, listpartition
from taskinit import mstool, tbtool, msmdtool
import testhelper as th
import unittest
import partitionhelper as ph
from parallel.parallel_data_helper import ParallelDataHelper

myname = 'test_importasdm'

# default ASDM dataset name
myasdm_dataset_name = 'uid___X5f_X18951_X1'
myms_dataset_name = 'M51.ms'

# name of the resulting MS
msname = myasdm_dataset_name+'.ms'

# name of the exported ASDM
asdmname = myms_dataset_name+'.asdm'

# name of the reimported MS
reimp_msname = 'reimported-'+myms_dataset_name

# make local copies of the tools
tblocal = tbtool()
mslocal = mstool()

def checktable(thename, theexpectation):
    global msname, myname
    tblocal.open(msname+"/"+thename)
    if thename == "":
        thename = "MAIN"
    for mycell in theexpectation:
        print myname, ": comparing ", mycell
        value = tblocal.getcell(mycell[0], mycell[1])
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
            tblocal.close()
            return False
    tblocal.close()
    print myname, ": table ", thename, " as expected."
    return True

#########################

def verify_asdm(asdmname, withPointing):
    print "Verifying asdm ", asdmname
    if(not os.path.exists(asdmname)):
        print "asdm ", asdmname, " doesn't exist."
        raise Exception
    # test for the existence of all obligatory tables
    allTables = [ "Antenna.xml",
                  "ASDM.xml",
                 # "CalData.xml",
                 # "CalDelay.xml",
                 # "CalReduction.xml",
                  "ConfigDescription.xml",
                  "CorrelatorMode.xml",
                  "DataDescription.xml",
                  "ExecBlock.xml",
                  "Feed.xml",
                  "Field.xml",
                 #"FocusModel.xml",
                 #"Focus.xml",
                  "Main.xml",
                  "PointingModel.xml",
                  "Polarization.xml",
                  "Processor.xml",
                  "Receiver.xml",
                  "SBSummary.xml",
                  "Scan.xml",
                  "Source.xml",
                  "SpectralWindow.xml",
                  "State.xml",
                  "Station.xml",
                  "Subscan.xml",
                  "SwitchCycle.xml"
                  ]
    isOK = True
    for fileName in allTables:
        filePath = asdmname+'/'+fileName
        if(not os.path.exists(filePath)):
            print "ASDM table file ", filePath, " doesn't exist."
            isOK = False
        else:
            # test if well formed
            rval = os.system('xmllint --noout '+filePath)
            if(rval !=0):
                print "Table ", filePath, " is not a well formed XML document."
                isOK = False

    print "Note: xml validation not possible since ASDM DTDs (schemas) not yet online."
        
    if(not os.path.exists(asdmname+"/ASDMBinary")):
        print "ASDM binary directory "+asdmname+"/ASDMBinary doesn't exist."
        isOK = False

    if(withPointing and not os.path.exists(asdmname+"/Pointing.bin")):
        print "ASDM binary file "+asdmname+"/Pointing.bin doesn't exist."
        isOK = False

    if (not isOK):
        raise Exception

# Setup for different data importing
class test_base(unittest.TestCase):
    def setUp_m51(self):
        res = None
        if(os.path.exists(myasdm_dataset_name)):
            shutil.rmtree(myasdm_dataset_name)

        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/asdm-import/input/'
        shutil.copytree(datapath + myasdm_dataset_name, myasdm_dataset_name)
        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/exportasdm/input/'
        shutil.copytree(datapath + myms_dataset_name, myms_dataset_name)
        default(importasdm)

    def setUp_xosro(self):
        self.asdm = 'X_osro_013.55979.93803716435'
        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/flagdata/'
        if(not os.path.lexists(self.asdm)):
            os.system('ln -s '+datapath+self.asdm +' '+self.asdm)
            
        default(importasdm)


    def setUp_autocorr(self):
        self.asdm = 'AutocorrASDM'
        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/importasdm/'
        if(not os.path.lexists(self.asdm)):
            os.system('ln -s '+datapath+self.asdm +' '+self.asdm)
            
        default(importasdm)

    def setUp_acaex(self):
        res = None
        myasdmname = 'uid___A002_X72bc38_X000' # ACA example ASDM with mixed pol/channelisation

        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/asdm-import/input/'
        os.system('ln -sf '+datapath+myasdmname)
        default(importasdm)

    def setUp_12mex(self):
        res = None
        myasdmname = 'uid___A002_X71e4ae_X317_short' # 12m example ASDM with mixed pol/channelisation

        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/asdm-import/input/'
        os.system('ln -sf '+datapath+myasdmname)
        default(importasdm)

    def setUp_eph(self):
        res = None
        myasdmname = 'uid___A002_X997a62_X8c-short' # 12m example ASDM with ephemerides

        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/asdm-import/input/'
        os.system('ln -sf '+datapath+myasdmname)
        default(importasdm)

    def setUp_flags(self):
        res = None
        myasdmname = 'test_uid___A002_X997a62_X8c-short' # Flag.xml is modified

        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/importasdm/'
        os.system('ln -sf '+datapath+myasdmname)
        default(importasdm)

    def setUp_SD(self):
        res = None
        # Single-dish ASDM, 1 scan, 9 spws, 4 antennas, 10 baselines,  4 auto-corrs + cross
        myasdmname = 'uid___A002_X6218fb_X264'  

        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/alma-sd/M100/'
        os.system('ln -sf '+datapath+myasdmname)
        default(importasdm)


###########################
# beginning of actual test 
        
class asdm_import8(test_base):
    '''Test importasdm with Multi-MS'''
    
    def setUp(self):
        self.setUp_12mex()
        self.setUp_acaex()
        
    def tearDown(self):
        for myasdmname in ['uid___A002_X71e4ae_X317_short', 'uid___A002_X72bc38_X000']:
            os.system('rm -f '+myasdmname) # a link
            shutil.rmtree(myasdmname+".ms",ignore_errors=True)
            shutil.rmtree(myasdmname+'.ms.flagversions',ignore_errors=True)
            shutil.rmtree("reference.ms",ignore_errors=True)

    def test_mms1(self):
        '''test_mms1: Create an MMS with default name'''
        myasdmname = 'uid___A002_X71e4ae_X317_short'
        themsname = myasdmname+".ms"

        importasdm(myasdmname, createmms=True)
        self.assertTrue(ParallelDataHelper.isParallelMS(themsname), 'Output is not a Multi-MS')
        
    def test_mms2(self):
        '''test_mms2: Create an MMS with default name and lazy=True'''
        myasdmname = 'uid___A002_X71e4ae_X317_short'
        themsname = myasdmname+".ms"

        importasdm(myasdmname, createmms=True, lazy=True, scans='2')
        self.assertTrue(ParallelDataHelper.isParallelMS(themsname), 'Output is not a Multi-MS')

    def test_mms3(self):
        '''test_mms3: Create MMS with separationaxis=spw and lazy=True'''
        myasdmname = 'uid___A002_X71e4ae_X317_short'
        themsname = myasdmname+".ms"

        importasdm(myasdmname, createmms=True, lazy=True, scans='1,2', separationaxis='spw', flagbackup=False,
                   process_flags=False)
        self.assertTrue(ParallelDataHelper.isParallelMS(themsname), 'Output is not a Multi-MS')
        self.assertEqual(ph.axisType(themsname), 'spw', 'Separation axis of MMS should be spw')
        
    def test_mms4(self):
        '''test_mms4: Create MMS, lazy=True, with separationaxis=scan and scans selection in ASDM'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    

        myasdmname = 'uid___A002_X72bc38_X000'
        themsname = myasdmname + ".ms"

        # only the first 3 scans to save time
        importasdm(myasdmname, vis=themsname, lazy=True, scans='0:1~3', createmms=True, separationaxis='scan',
                   process_flags=False) 
        self.assertTrue(ParallelDataHelper.isParallelMS(themsname), 'Output is not a Multi-MS')
        self.assertEqual(ph.axisType(themsname), 'scan', 'Separation axis of MMS should be scan')
        print myname, ": Success! Now checking output ..."
        mscomponents = set(["ANTENNA/table.dat",
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
            if not os.access(themsname+"/"+name, os.F_OK):
                print myname, ": Error  ", themsname+"/"+name, "doesn't exist ..."
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+themsname+'/'+name+' does not exist'
            else:
                print myname, ": ", name, "present."
        print myname, ": MS exists. All tables present. Try opening as MS ..."
        try:
            mslocal.open(themsname)
        except:
            print myname, ": Error  Cannot open MS table", themsname
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']+'Cannot open MS table '+themsname
        else:
            mslocal.close()
            print myname, ": OK. Checking tables in detail ..."
    
            importasdm(asdm=myasdmname, vis='reference.ms', lazy=False, overwrite=True, scans='0:1~3',
                        createmms=True, separationaxis='scan', process_flags=False)

            if(os.path.exists('reference.ms')):
                retValue['success'] = th.checkwithtaql("select from [select from reference.ms orderby TIME, DATA_DESC_ID, ANTENNA1, ANTENNA2 ] t1, [select from "
                                                    +themsname+" orderby TIME, DATA_DESC_ID, ANTENNA1, ANTENNA2 ] t2 where (not all(near(t1.DATA,t2.DATA, 1.e-06)))") == 0
                if not retValue['success']:
                    print "ERROR: DATA does not agree with reference."
                else:
                    print "DATA columns agree."
                retValueTmp = th.checkwithtaql("select from [select from reference.ms orderby TIME, DATA_DESC_ID, ANTENNA1, ANTENNA2 ] t1, [select from "
                                            +themsname+" orderby TIME, DATA_DESC_ID, ANTENNA1, ANTENNA2 ] t2 where (not all(t1.FLAG==t2.FLAG)) ") == 0
                if not retValueTmp:
                    print "ERROR: FLAG does not agree with reference."
                else:
                    print "FLAG columns agree."

                retValue['success'] = retValue['success'] and retValueTmp

                for subtname in ["ANTENNA",
                                 "DATA_DESCRIPTION",
                                 "FEED",
                                 "FIELD",
                                 "FLAG_CMD",
                                 "OBSERVATION",
                                 "POINTING",
                                 "POLARIZATION",
                                 "PROCESSOR",
                                 "SOURCE",
                                 "SPECTRAL_WINDOW",
                                 "STATE",
                                 "SYSCAL"]:
                    
                    print "\n*** Subtable ",subtname
                    excllist = []
                    if subtname=='SOURCE':
                        excllist=['POSITION', 'TRANSITION', 'REST_FREQUENCY', 'SYSVEL']
                    if subtname=='SYSCAL':
                        excllist=['TANT_SPECTRUM', 'TANT_TSYS_SPECTRUM']
                    if subtname=='SPECTRAL_WINDOW':
                        excllist=['CHAN_FREQ', 'CHAN_WIDTH', 'EFFECTIVE_BW', 'RESOLUTION']
                        for colname in excllist: 
                            retValue['success'] = th.compVarColTables('reference.ms/SPECTRAL_WINDOW',
                                                                      themsname+'/SPECTRAL_WINDOW', colname, 0.01) and retValue['success']
                        
                    try:    
                        retValue['success'] = th.compTables('reference.ms/'+subtname,
                                                            themsname+'/'+subtname, excllist, 
                                                            0.01) and retValue['success']
                    except:
                        retValue['success'] = False
                        print "ERROR for table ", subtname
            
        print retValue    
        self.assertTrue(retValue['success'],retValue['error_msgs'])
        
    @unittest.skip('Skip until a suitable ASDM is found to run this test')
    def test_mms5(self):
        '''test_mms5: Create 2 MMS, 2 flagversions and 2 online flag files'''
        myasdmname = 'uid___A002_X71e4ae_X317_short'
        themsname = myasdmname+".ms"
        wvrmsname = myasdmname+'-wvr-corrected.ms'
        flagfile1 = myasdmname+'_cmd.txt'
        flagfile2 = myasdmname+'-wvr-corrected'+'_cmd.txt'
        
        importasdm(myasdmname, vis=themsname, lazy=True, scans='0:1~4', wvr_corrected_data='both', savecmds=True,
                   createmms=True)
        self.assertTrue(ParallelDataHelper.isParallelMS(themsname), 'Output is not a Multi-MS')
        self.assertTrue(ParallelDataHelper.isParallelMS(wvrmsname), 'Output is not a Multi-MS')
        self.assertTrue(os.path.exists(flagfile1))
        self.assertTrue(os.path.exists(flagfile2))

        
    def test_mms6(self):
        '''test_mms6: Create MMS and lazy=True'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    
        myasdmname = 'uid___A002_X71e4ae_X317_short'
        themsname = myasdmname+".ms"
        flagfile = myasdmname+'_cmd.txt'

        self.res = importasdm(myasdmname, vis=themsname, lazy=True, scans='0:1~4', createmms=True,
                              savecmds=True) # only the first 4 scans to save time
        self.assertEqual(self.res, None)
        self.assertTrue(ParallelDataHelper.isParallelMS(themsname), 'Output is not a Multi-MS')
        self.assertTrue(os.path.exists(flagfile))
        print myname, ": Success! Now checking output ..."
        mscomponents = set(["ANTENNA/table.dat",
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
            if not os.access(themsname+"/"+name, os.F_OK):
                print myname, ": Error  ", themsname+"/"+name, "doesn't exist ..."
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+themsname+'/'+name+' does not exist'
            else:
                print myname, ": ", name, "present."
        print myname, ": MS exists. All tables present. Try opening as MS ..."
        try:
            mslocal.open(themsname)
            mslocal.close()
            print  myname, ": MS can be opened. Now testing the changing of the asdmref ..."
            mslocal.open(themsname)
            mslocal.asdmref("./moved_"+myasdmname)
            mslocal.close()
            os.system("mv "+myasdmname+" moved_"+myasdmname)
            
            mslocal.open(themsname)
            
        except:
            print myname, ": Error  Cannot open MS table", themsname
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']+'Cannot open MS table '+themsname
        else:
            mslocal.close()
            print myname, ": OK. Checking tables in detail ..."
    
            importasdm(asdm="moved_"+myasdmname, vis='reference.ms', lazy=False, overwrite=True, scans='0:1~3', createmms=True)

            if(os.path.exists('reference.ms')):
                retValue['success'] = th.checkwithtaql("select from [select from reference.ms orderby TIME, DATA_DESC_ID, ANTENNA1, ANTENNA2 ] t1, [select from "
                                                    +themsname+" orderby TIME, DATA_DESC_ID, ANTENNA1, ANTENNA2 ] t2 where (not all(near(t1.DATA,t2.DATA, 1.e-06)))") == 0
                if not retValue['success']:
                    print "ERROR: DATA does not agree with reference."
                else:
                    print "DATA columns agree."

                retValueTmp = th.checkwithtaql("select from [select from reference.ms orderby TIME, DATA_DESC_ID, ANTENNA1, ANTENNA2 ] t1, [select from "
                                                    +themsname+" orderby TIME, DATA_DESC_ID, ANTENNA1, ANTENNA2 ] t2 where (not all(near(t1.WEIGHT,t2.WEIGHT, 1.e-06)))") == 0
                if not retValueTmp:
                    print "ERROR: WEIGHT does not agree with reference."
                else:
                    print "WEIGHT columns agree."
                    
                retValueTmp2 = th.checkwithtaql("select from [select from reference.ms orderby TIME, DATA_DESC_ID, ANTENNA1, ANTENNA2 ] t1, [select from "
                                            +themsname+" orderby TIME, DATA_DESC_ID, ANTENNA1, ANTENNA2 ] t2 where (not all(t1.FLAG==t2.FLAG)) ") == 0
                if not retValueTmp2:
                    print "ERROR: FLAG does not agree with reference."
                else:
                    print "FLAG columns agree."

                retValue['success'] = retValue['success'] and retValueTmp and retValueTmp2

                for subtname in ["ANTENNA",
                                 "DATA_DESCRIPTION",
                                 "FEED",
                                 "FIELD",
                                 "FLAG_CMD",
                                 "OBSERVATION",
                                 "POLARIZATION",
                                 "PROCESSOR",
                                 "SOURCE",
                                 "SPECTRAL_WINDOW",
                                 "STATE",
                                 "SYSCAL"]:
                    
                    print "\n*** Subtable ",subtname
                    excllist = []
                    if subtname=='SOURCE':
                        excllist=['POSITION', 'TRANSITION', 'REST_FREQUENCY', 'SYSVEL']
                    if subtname=='SYSCAL':
                        excllist=['TANT_SPECTRUM', 'TANT_TSYS_SPECTRUM']
                    if subtname=='SPECTRAL_WINDOW':
                        excllist=['CHAN_FREQ', 'CHAN_WIDTH', 'EFFECTIVE_BW', 'RESOLUTION', 'ASSOC_SPW_ID', 'ASSOC_NATURE']
                        for colname in excllist:
                            if colname!='ASSOC_NATURE':
                                retValue['success'] = th.compVarColTables('reference.ms/SPECTRAL_WINDOW',
                                                                          themsname+'/SPECTRAL_WINDOW', colname, 0.01) and retValue['success']
                    if subtname=='POLARIZATION':
                        excllist=['CORR_TYPE', 'CORR_PRODUCT']
                        for colname in excllist: 
                            retValue['success'] = th.compVarColTables('reference.ms/POLARIZATION',
                                                                      themsname+'/POLARIZATION', colname, 0.01) and retValue['success']
                    try:    
                        retValue['success'] = th.compTables('reference.ms/'+subtname,
                                                            themsname+'/'+subtname, excllist, 
                                                            0.01) and retValue['success']
                    except:
                        retValue['success'] = False
                        print "ERROR for table ", subtname

        os.system("mv moved_"+myasdmname+" "+myasdmname)
                
        self.assertTrue(retValue['success'],retValue['error_msgs'])
        
    def test_mms7(self):
        '''Asdm-import: Test good 12 m ASDM with mixed pol/channelisation input with default filler in lazy mode with reading the BDF flags. Output MMS'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    

        myasdmname = 'uid___A002_X71e4ae_X317_short'
        themsname = myasdmname+".ms"

        self.res = importasdm(myasdmname, vis=themsname, lazy=True, bdfflags=True, createmms=True) 
        self.assertEqual(self.res, None)
        print myname, ": Success! Now checking output ..."
        mscomponents = set(["ANTENNA/table.dat",
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
            if not os.access(themsname+"/"+name, os.F_OK):
                print myname, ": Error  ", themsname+"/"+name, "doesn't exist ..."
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+themsname+'/'+name+' does not exist'
            else:
                print myname, ": ", name, "present."
        print myname, ": MS exists. All tables present. Try opening as MS ..."
        try:
            mslocal.open(themsname)
        except:
            print myname, ": Error  Cannot open MS table", themsname
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']+'Cannot open MS table '+themsname
        else:
            mslocal.close()
            print myname, ": OK. Checking tables in detail ..."
    
            importasdm(asdm=myasdmname, vis='reference.ms', lazy=True, overwrite=True, bdfflags=False, createmms=True)

            if(os.path.exists('reference.ms')):
                retValue['success'] = th.checkwithtaql("select from [select from reference.ms orderby TIME, DATA_DESC_ID, ANTENNA1, ANTENNA2 ] t1, [select from "
                                                    +themsname+" orderby TIME, DATA_DESC_ID, ANTENNA1, ANTENNA2 ] t2 where (not all(near(t1.DATA,t2.DATA, 1.e-06)))") == 0
                if not retValue['success']:
                    print "ERROR: DATA does not agree with reference."
                else:
                    print "DATA columns agree."
                retValueTmp = th.checkwithtaql("select from [select from reference.ms orderby TIME, DATA_DESC_ID, ANTENNA1, ANTENNA2 ] t1, [select from "
                                            +themsname+" orderby TIME, DATA_DESC_ID, ANTENNA1, ANTENNA2 ] t2 where (not all(t1.FLAG==t2.FLAG)) ") != 0
                if not retValueTmp:
                    print "ERROR: FLAG columns do agree with reference but they shouldn't."
                else:
                    print "FLAG columns do not agree as expected."

                retValue['success'] = retValue['success'] and retValueTmp

                for subtname in ["ANTENNA",
                                 "DATA_DESCRIPTION",
                                 "FEED",
                                 "FIELD",
                                 "FLAG_CMD",
                                 "OBSERVATION",
                                 "POLARIZATION",
                                 "PROCESSOR",
                                 "SOURCE",
                                 "SPECTRAL_WINDOW",
                                 "STATE",
                                 "SYSCAL"]:
                    
                    print "\n*** Subtable ",subtname
                    excllist = []
                    if subtname=='SOURCE':
                        excllist=['POSITION', 'TRANSITION', 'REST_FREQUENCY', 'SYSVEL']
                    if subtname=='SYSCAL':
                        excllist=['TANT_SPECTRUM', 'TANT_TSYS_SPECTRUM']
                    if subtname=='SPECTRAL_WINDOW':
                        excllist=['CHAN_FREQ', 'CHAN_WIDTH', 'EFFECTIVE_BW', 'RESOLUTION', 'ASSOC_SPW_ID', 'ASSOC_NATURE']
                        for colname in excllist:
                            if colname!='ASSOC_NATURE':
                                retValue['success'] = th.compVarColTables('reference.ms/SPECTRAL_WINDOW',
                                                                          themsname+'/SPECTRAL_WINDOW', colname, 0.01) and retValue['success']
                    if subtname=='POLARIZATION':
                        excllist=['CORR_TYPE', 'CORR_PRODUCT']
                        for colname in excllist: 
                            retValue['success'] = th.compVarColTables('reference.ms/POLARIZATION',
                                                                      themsname+'/POLARIZATION', colname, 0.01) and retValue['success']
                    try:    
                        retValue['success'] = th.compTables('reference.ms/'+subtname,
                                                            themsname+'/'+subtname, excllist, 
                                                            0.01) and retValue['success']
                    except:
                        retValue['success'] = False
                        print "ERROR for table ", subtname
            
                
        self.assertTrue(retValue['success'],retValue['error_msgs'])
        

class asdm_import9(test_base):
    '''Test importasdm with spw selection in online flags'''
    
    def setUp(self):
        self.setUp_flags()
       
    def tearDown(self):
        for myasdmname in ['test_uid___A002_X997a62_X8c-short']:
            os.system('rm -f '+myasdmname) # a link
            shutil.rmtree(myasdmname+".ms",ignore_errors=True)
            shutil.rmtree(myasdmname+'.ms.flagversions',ignore_errors=True)

    def test_online1(self):
        '''importasdm: online flags file with spw selection by name'''
        myasdmname = 'test_uid___A002_X997a62_X8c-short'
        themsname = myasdmname+".ms"
        flagfile = myasdmname+'_cmd.txt'

        importasdm(myasdmname, vis=themsname, scans='1', savecmds=True)
        flist = open(flagfile,'r').read().splitlines()
        self.assertTrue(flist[0].__contains__('spw'))
        self.assertFalse(flist[3].__contains__('WVR#Antenna'))
        self.assertFalse(flist[4].__contains__('WVR#Antenna'))


class asdm_import10(test_base):
    '''Test importasdm with single-dish data'''
    
    def setUp(self):
        self.setUp_SD()
       
    def tearDown(self):
        for myasdmname in ['uid___A002_X6218fb_X264']:
            os.system('rm -f '+myasdmname) # a link
            shutil.rmtree(myasdmname+".ms",ignore_errors=True)
            shutil.rmtree(myasdmname+'.ms.flagversions',ignore_errors=True)

    def test_float_data_mms(self):
        '''importasdm: Create an MMS from a FLOAT_DATA MS '''
        myasdmname = 'uid___A002_X6218fb_X264'
        themsname = myasdmname+".ms"

        # The ocorr_mode='ao' option will create a FLOAT_DATA column instead of DATA
        importasdm(myasdmname, vis=themsname, ocorr_mode='ao', createmms=True, scans='1')
        self.assertTrue(ParallelDataHelper.isParallelMS(themsname), 'Output is not a Multi-MS')        
        self.assertTrue(len(th.getColDesc(themsname, 'FLOAT_DATA')) > 0)

    def test_sd_data_mms(self):
        '''importasdm: Create an MMS from a single-dish MS and DATA column '''
        myasdmname = 'uid___A002_X6218fb_X264'
        themsname = myasdmname+".ms"

        importasdm(myasdmname, vis=themsname, scans='1,4', createmms=True, separationaxis='scan', numsubms=2, flagbackup=False)
        self.assertTrue(ParallelDataHelper.isParallelMS(themsname), 'Output is not a Multi-MS')
        self.assertTrue(len(th.getColDesc(themsname, 'DATA')) > 0)

    def test_sd_data_mms_baseline_all(self):
        '''importasdm: Create an MMS separated per baseline, using default numsubms '''
        myasdmname = 'uid___A002_X6218fb_X264'
        themsname = myasdmname+".ms"

        # Create a single-dish MMS with auto and cross correlations
        importasdm(myasdmname, vis=themsname, ocorr_mode='ca', createmms=True, scans='1', separationaxis='baseline')
        self.assertTrue(ParallelDataHelper.isParallelMS(themsname), 'Output is not a Multi-MS')        
        self.assertTrue(len(th.getColDesc(themsname, 'DATA')) > 0)
        
        md = msmdtool()
        md.open(themsname)
        bsl = md.baselines()
        md.close()      
        
        # Check if all baselines are in there
        self.assertTrue(bsl.all(), 'Not all baselines are in the MMS')

    def test_float_data_mms_baseline_auto(self):
        '''importasdm: Create an MMS with a FLOAT_DATA column separated per baseline '''
        myasdmname = 'uid___A002_X6218fb_X264'
        themsname = myasdmname+".ms"

        # The ocorr_mode='ao' option will create a FLOAT_DATA column instead of DATA
        importasdm(myasdmname, vis=themsname, ocorr_mode='ao', createmms=True, scans='1', separationaxis='baseline', numsubms=4)
        self.assertTrue(ParallelDataHelper.isParallelMS(themsname), 'Output is not a Multi-MS')        
        self.assertTrue(len(th.getColDesc(themsname, 'FLOAT_DATA')) > 0)
        
        md = msmdtool()
        md.open(themsname)
        bsl = md.baselines()
        md.close()
        
        #diagnoals give the auto-corrs
        ac = bsl.diagonal()
        self.assertTrue(ac.all(), 'Not all auto-correlations are there')
        
        # Take the dictionary and compare with original MS
        thisdict = listpartition(vis=themsname, createdict=True)
        self.assertEqual(len(thisdict.keys()), 4, 'There should be 4 subMSs in output MMS')


class asdm_import11(test_base):
    '''Test importasdm with MMS and ephemerides data'''
    
    def setUp(self):
        self.setUp_eph()
       
    def tearDown(self):
        pass
#        for myasdmname in ['uid___A002_X997a62_X8c-short']:
##            os.system('rm -f '+myasdmname) # a link
#            shutil.rmtree(myasdmname+".ms",ignore_errors=True)
#            shutil.rmtree(myasdmname+'.ms.flagversions',ignore_errors=True)
            
    def test_mms_ephem(self):
        '''Asdm-import: Test good 12 m ASDM with Ephemeris table in lazy mode and MMS'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    

        myasdmname = 'uid___A002_X997a62_X8c-short'
        themsname = myasdmname+".ms"

        self.res = importasdm(myasdmname, vis=themsname, lazy=True, convert_ephem2geo=True, 
                              process_pointing=False, flagbackup=False, createmms=True) 
        self.assertEqual(self.res, None)
        print myname, ": Success! Now checking output ..."
        mscomponents = set(["FIELD/table.dat",
                            "FIELD/EPHEM0_Mars_57034.9.tab",
                            "FIELD/EPHEM1_Titania_57034.9.tab"
                            ])
        for name in mscomponents:
            if not os.access(themsname+"/"+name, os.F_OK):
                print myname, ": Error  ", themsname+"/"+name, "doesn't exist ..."
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+themsname+'/'+name+' does not exist'
            else:
                print myname, ": ", name, "present."
        print myname, ": MS exists. All relevant tables present. Try opening as MS ..."
        try:
            mslocal.open(themsname)
        except:
            print myname, ": Error  Cannot open MS table", themsname
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']+'Cannot open MS table '+themsname
        else:
            mslocal.close()
            print myname, ": OK."

        for name in ["FIELD/EPHEM0_Mars_57034.9.tab",
                     "FIELD/EPHEM1_Titania_57034.9.tab"]:
            tblocal.open(themsname+"/"+name)
            kw = tblocal.getkeywords()
            tblocal.close()
            geodist = kw['GeoDist'] # (km above reference ellipsoid)
            geolat = kw['GeoLat'] # (deg)
            geolong = kw['GeoLong'] # (deg)
            print myname, ": Testing if ephemeris ", name, " was converted to GEO ..."
            if not (geodist==geolat==geolong==0.):
                print myname, ": ERROR."
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+' Ephemeris was not converted to GEO for '+themsname+'\n'
            else:
                print myname, ": OK."
            prsys = kw['posrefsys']
            print myname, ": Testing if posrefsys was set correctly ..."
            if not (prsys=="ICRF/ICRS"):
                print myname, ": ERROR."
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']+' posrefsys keyword is not ICRF/ICRS '+themsname+'\n'
            else:
                print myname, ": OK."
 

        self.assertTrue(retValue['success'],retValue['error_msgs'])
        
def suite():
    return [asdm_import8,
            asdm_import9,
            asdm_import10,
            asdm_import11]        
        
    
