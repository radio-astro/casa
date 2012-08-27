import os
import io
import sys
import shutil
import commands
from __main__ import default
from tasks import *
from taskinit import *
import unittest

datapath = os.environ.get('CASAPATH').split()[0] +\
                            '/data/regression/unittest/listhistory/'

testmms = False
if os.environ.has_key('TEST_DATADIR'):   
    DATADIR = str(os.environ.get('TEST_DATADIR'))+'/listhistory/'
    if os.path.isdir(DATADIR):
        testmms = True
        datapath = DATADIR
    else:
        print 'WARN: directory '+DATADIR+' does not exist'

print 'listhistory tests will use data from '+datapath         

class listhistory_test(unittest.TestCase):

    # Input and output names
    msfile = 'Itziar.ms'
    itismms = testmms

    def setUp(self):
        fpath = os.path.join(datapath,self.msfile)
        if os.path.lexists(fpath):
            os.symlink(fpath, self.msfile)
        else:
            self.fail('Data does not exist -> '+fpath)
            
    def tearDown(self):
        if os.path.lexists(self.msfile):
            os.unlink(self.msfile)
        
    def test1(self):
        '''Test 1: Empty input should return False'''
        myms = ''
        res = listhistory(myms)
        self.assertFalse(res)
        
    def test2(self):
        '''Test 2: Good input should return None'''
        res = listhistory(self.msfile)
        self.assertEqual(res,None)
        
    def test3(self):
        '''Test 3: Compare length of reference and new lists'''
        logfile= "mylisth.log"
        newfile= "newlisth.log"
        open(logfile,"w").close
        casalog.setlogfile(logfile)
        
        res = listhistory(self.msfile)
        cmd="sed -n \"/Begin Task/,/End Task/p\" %s > %s " %(logfile,newfile)
        os.system(cmd)
    
        # Get the number of lines in file
        refnum=13
        if self.itismms:
            refnum = 37

        cmd="wc -l %s |egrep \"[0-9]+\" -o" %newfile    
        output=commands.getoutput(cmd)
        num = int(output)
        self.assertEqual(refnum,num)


class listhistory_cleanup(unittest.TestCase):
    
    def tearDown(self):
        os.system('rm -rf *Itziar.*')

    def test_cleanup(self):
        '''listhistory: Cleanup'''
        pass
        
def suite():
    return [listhistory_test, listhistory_cleanup]

               
        
        
