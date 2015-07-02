import os
import sys
import shutil
from __main__ import default
from tasks import applycal
from taskinit import cbtool, casalog
import unittest
import time

    
class test_base(unittest.TestCase):
        
    def setUpFile(self,file,type_file):
        
        if type(file) is list:
            for file_i in file:
                self.setUpFileCore(file_i,type_file)
        else:
            self.setUpFileCore(file,type_file)
                
        if type_file=='vis':
            self.vis = file
        elif type_file =='ref':
            self.ref = file
        elif type_file=='aux':
            self.aux = file
        
    def setUpFileCore(self,file,type_file):
        
        if os.path.exists(file):
             print "%s file %s is already in the working area, deleting ..." % (type_file,file)
             os.system('rm -rf ' + file)
        print "Copy %s file %s into the working area..." % (type_file,file)
        os.system('cp -R ' + os.environ.get('CASAPATH').split()[0] +
                  '/data/regression/unittest/simplecluster/' + file + ' ' + file)
    
 
class Applycal_mms_tests(test_base):

    def setUp(self):
        # Set-up MMS
        self.setUpFile("ngc5921.applycal.mms",'vis')
        # Set-up auxiliary files
        self.setUpFile(["ngc5921.fluxscale", "ngc5921.gcal", "ngc5921.bcal"],'aux')
        

    def tearDown(self):

        # Remove MMS
        os.system('rm -rf ' + self.vis) 
        os.system('rm -rf ' + '*.flagversions') 

        # Remove aux files
        for file in self.aux:
            os.system('rm -rf ' + file)        
        
    def test1_applycal_fluxscale_gcal_bcal(self):
        """Test 1: Apply calibration using fluxscal gcal and bcal tables. Create flagbackup for an MMS"""

        # Repository caltables are pre-v4.1, and we
        # must update them _before_ applycal to avoid contention
        casalog.post("Updating pre-v4.1 caltables: %s" % str(self.aux),"WARN","test1_applycal_fluxscale_gcal_bcal")
        cblocal = cbtool()
        for oldct in self.aux:
            cblocal.updatecaltable(oldct)
        casalog.post("Pre-v4.1 caltables updated","INFO","test1_applycal_fluxscale_gcal_bcal")
                
        # Run applycal in MMS mode. Verify that the flagbackup is correctly created for the top-level MMS only
        applycal(vis=self.vis,field='',spw='',selectdata=False,gaintable=self.aux,
                 gainfield=['nearest','nearest','0'],
                 interp=['linear', 'linear','nearest'],spwmap=[], flagbackup=True)
                
        # Verify that flagbackup works
        self.assertTrue(os.path.exists(self.vis+'.flagversions'), 'Backup of flags was not created') 
        files = os.listdir(self.vis+'/SUBMSS')
        print files
        for ff in files:
            self.assertFalse(ff.__contains__('flagversions'))
            
                   

def suite():
    return [Applycal_mms_tests]
     
