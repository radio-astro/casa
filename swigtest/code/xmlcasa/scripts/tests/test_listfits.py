import os
import sys
import shutil
import commands
from __main__ import default
from tasks import *
from taskinit import *
import unittest

class listfits_test(unittest.TestCase):
    
    # Input and output names
    fitsfile = 'ngc5921.fits'
    res = None

    def setUp(self):
        self.res = None
        default(listfits)
        
        if(os.path.exists(self.fitsfile)):
            os.system('rm -rf ' + self.fitsfile)

        shutil.copyfile(os.environ.get('CASAPATH').split()[0] +\
                            '/data/regression/ngc5921/'+self.fitsfile, self.fitsfile)
    
    def tearDown(self):
        if (os.path.exists(self.fitsfile)):
            os.system('rm -rf ' + self.fitsfile)
        
    def test1(self):
        '''Test 1: Empty input should return False'''
        fitsfile = ''
        self.res = listfits(fitsfile)
        self.assertFalse(self.res)
        
    def test2(self):
        '''Test 2: Good input should return None'''
        self.res=listfits(self.fitsfile)
        self.assertEqual(self.res,None)
        
    def test3(self):
        '''Test 3: list the fits into a private logfile'''
        thelogfile=casalog.logfile()
        logfile= "mylistfits.log"
        try:
           open(logfile,"w").close()
           casalog.setlogfile(logfile)
           listfits(self.fitsfile)
           casalog.setlogfile(thelogfile)
        except:
           print 'could not open "%s" for writing' % logfile
        
def suite():
    return [listfits_test]

       
        
        
        
        
        

