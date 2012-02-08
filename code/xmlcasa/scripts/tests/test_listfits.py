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

        shutil.copytree(os.environ.get('CASAPATH').split()[0] +\
                            '/data/regression/ngc5921/'+self.fitsfile, self.fitsfile)
    
    def tearDown(self):
        if (os.path.exists(self.msfile)):
            os.system('rm -rf ' + self.msfile)
        
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
        '''Test 3: Compare length of reference and new lists'''
        logfile= "mylistfits.log"
        newfile= "newlistfits.log"
        open(logfile,"w").close
        casalog.setlogfile(logfile)
        
        self.res = listfits(self.fitsfile)
        cmd="sed -n \"/Begin Task/,/End Task/p\" %s > %s " %(logfile,newfile)
        os.system(cmd)
    
        # Get the number of lines in file
        refnum=14
        cmd="wc -l %s |egrep \"[0-9]+\" -o" %newfile    
        output=commands.getoutput(cmd)
        num = int(output)
        self.assertEqual(refnum,num)

        
def suite():
    return [listfits_test]

       
        
        
        
        
        
