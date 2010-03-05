# unit test for the exportasdm task

import os

from __main__ import default
from tasks import *
from taskinit import *
import unittest

class exportasdm_test(unittest.TestCase):
    
    #vis_a = 'ngc4826.ms'
    vis_b = 'test.ms'
    #vis_c = 'jupiter6cm.demo.ms'
    vis_d = 'ngc4826.tutorial.ngc4826.ll.5.ms'
    vis_e = 'g19_d2usb_targets_line-shortened.ms'
    vis_f = 'Itziar.ms'
    vis_g = 'M51.ms'
    out = 'exportasdm-output.asdm'
    rval = False
    
    def setUp(self):    
        self.rval = False
        #if(not os.path.exists(vis_a)):
        #    importuvfits(fitsfile=os.environ['CASADATA']+'/regression/ngc4826/fitsfiles/ngc4826.ll.fits5', vis=vis_a)
        if(not os.path.exists(self.vis_b)):
            os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/fits-import-export/input/test.ms .')
        #if(not os.path.exists(vis_c)):
        #    importuvfits(fitsfile=os.environ['CASADATA']+'/regression/jupiter6cm/jupiter6cm.fits', vis=vis_c)
        if(not os.path.exists(self.vis_d)):
            importuvfits(fitsfile=os.environ['CASAPATH'].split()[0]+'/data/regression/ngc4826/fitsfiles/ngc4826.ll.fits5', 
                         vis=self.vis_d)
        if(not os.path.exists(self.vis_e)):
            os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/cvel/input/g19_d2usb_targets_line-shortened.ms .')
        if(not os.path.exists(self.vis_f)):
            os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/exportasdm/input/Itziar.ms .')            
        if(not os.path.exists(self.vis_g)):
            os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/exportasdm/input/M51.ms .')

        default(exportasdm)

    def tearDown(self):
        os.system('rm -rf exportasdm-output.asdm myinput.ms')


    def verify_asdm(self,asdmname, withPointing):
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

# Test cases    
    def test1(self):
        '''Test 1: Testing default'''
        myvis = self.vis_b
        os.system('cp -R ' + myvis + ' myinput.ms')
        self.rval = exportasdm()
        self.assertFalse(self.rval)

    def test2(self):
        '''Test 2: small input MS, default output'''
        myvis = self.vis_b
        os.system('cp -R ' + myvis + ' myinput.ms')
        self.rval = exportasdm(
                vis = 'myinput.ms',
                asdm = self.out,
                archiveid="S1",
                verbose=True,
                apcorrected=False)

        self.assertNotEqual(self.rval,False)
        omsname = "test"+str(2)+self.out
        os.system('rm -rf '+omsname+'; mv exportasdm-output.asdm '+omsname)
        self.verify_asdm(omsname, False)

    def test3(self):
        '''Test 3: simulated input MS, default output'''
        myvis = self.vis_f
        os.system('cp -R ' + myvis + ' myinput.ms')
        self.rval = exportasdm(vis = 'myinput.ms',asdm = self.out,archiveid="S1")
        self.assertNotEqual(self.rval,False)
        omsname = "test"+str(3)+self.out
        os.system('rm -rf '+omsname+'; mv exportasdm-output.asdm '+omsname)
        self.verify_asdm(omsname, True)

    def test4(self):
        '''Test 4: real input MS, default output'''
        myvis = self.vis_d
        os.system('cp -R ' + myvis + ' myinput.ms')
        self.rval = exportasdm(
            vis = 'myinput.ms',
            asdm = self.out,
            archiveid="S1",
            apcorrected=False
            )

        self.assertNotEqual(self.rval,False)
        omsname = "test"+str(4)+self.out
        os.system('rm -rf '+omsname+'; mv exportasdm-output.asdm '+omsname)
        self.verify_asdm(omsname, False)

    def test5(self):
        '''Test 5: real input MS, MS has several SPWs observed in parallel - not supported, expected error'''
        myvis = self.vis_e
        os.system('cp -R ' + myvis + ' myinput.ms')
        self.rval = exportasdm(
            vis = 'myinput.ms',
            asdm = self.out,
            archiveid="S1",
            apcorrected=False
            )

        self.assertFalse(self.rval)

    def test6(self):
        '''Test 6: simulated input MS with pointing table, default output'''
        myvis = self.vis_g
        os.system('cp -R ' + myvis + ' myinput.ms')
        self.rval = exportasdm(
            vis = 'myinput.ms',
            asdm = self.out,
            archiveid="S002",
            apcorrected=False
            )

        self.assertNotEqual(self.rval,False)
        omsname = "test"+str(6)+self.out
        os.system('rm -rf '+omsname+'; mv exportasdm-output.asdm '+omsname)
        self.verify_asdm(omsname, True)

def suite():
    return [exportasdm_test]

