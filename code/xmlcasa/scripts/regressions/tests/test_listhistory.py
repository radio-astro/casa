import os
import sys
import shutil
import commands
from __main__ import default
from tasks import *
from taskinit import *
import unittest

class listhistory_test(unittest.TestCase):
    
    # Input and output names
    msfile = 'Itziar.ms'
    res = None

    def setUp(self):
        self.res = None
        default(listhistory)

        shutil.copytree(os.environ.get('CASAPATH').split()[0] +\
                            '/data/regression/exportasdm/input/'+self.msfile, self.msfile)
    
    def tearDown(self):
        if (os.path.exists(self.msfile)):
            os.system('rm -rf ' + self.msfile)
        
    def test1(self):
        '''Test 1: Empty input should return False'''
        msfile = ''
        self.res = listhistory(msfile)
        self.assertFalse(self.res)
        
    def test2(self):
        '''Test 2: Good input should return None'''
        self.res=listhistory(self.msfile)
        self.assertEqual(self.res,None)
        
    def test3(self):
        '''Test 3: Compare length of reference and new lists'''
        logfile= "mylisth.log"
        newfile= "newlisth.log"
        open(logfile,"w").close
        casalog.setlogfile(logfile)
        
        self.res = listhistory(self.msfile)
        cmd="sed -n \"/Begin Task/,/End Task/p\" %s > %s " %(logfile,newfile)
        os.system(cmd)
    
        # Get the number of lines in file
        refnum=25
        cmd="wc -l %s |egrep \"[0-9]+\" -o" %newfile    
        output=commands.getoutput(cmd)
        num = int(output)
        self.assertEqual(refnum,num)

        
def suite():
    return [listhistory_test]

        
        
        
        
        
        
        
        
