import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest

'''
Unit tests of task clearstat. It tests the following parameters:
    clears read lock on table,
    clears write lock on table,
    clears read lock on image,
    clears write lock on image,
    clears all locks
'''

class clearstat_test(unittest.TestCase):
    
    # Input names
    msfile = 'ic2233_1.ms'
    res = None
    img = 'n4826_tmom1.im'
    
    def setUp(self):
        self.res = None
        default('clearstat')
        if (not os.path.exists(self.msfile)):            
            datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/ic2233/'
            shutil.copytree(datapath+self.msfile, self.msfile)
            
        if (not os.path.exists(self.img)):
            datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/ngc4826redux/reference/'
            shutil.copytree(datapath+self.img, self.img)
    
    def tearDown(self):
        pass
            
        
    def test1(self):
        '''Clear table read lock'''
        tb.open(self.msfile)
        lock = tb.haslock(write=False)
        self.assertTrue(lock,'Cannot acquire read lock on table')
        clearstat()
        lock = tb.haslock(write=False)
        tb.close()
        self.assertFalse(lock,'Failed to clear table read lock')

    def test2(self):
        '''Clear table write lock'''
        tb.open(self.msfile)
        tb.lock()
        lock = tb.haslock(write=True)
        self.assertTrue(lock,'Cannot acquire write lock on table')
        clearstat()
        lock = tb.haslock(write=True)
        tb.close()
        self.assertFalse(lock,'Failed to clear table write lock')

    def test3(self):
        '''Clear image read lock'''
        ia.open(self.img)
        lock = ia.haslock()
        self.assertTrue(lock[0]==True and lock[1]==False,'Cannot acquire read lock on image')
        clearstat()
        lock = ia.haslock()
        ia.close()
        self.assertTrue(lock[0]==False and lock[1]==False,'Failed to clear read lock on image')

    def test4(self):
        '''Clear image write lock'''
        ia.open(self.img)
        ia.lock(writelock=True)
        lock = ia.haslock()
        self.assertTrue(lock[0]==True and lock[1]==True,'Cannot acquire write lock on image')
        clearstat()
        lock = ia.haslock()
        ia.close()
        self.assertTrue(lock[0]==False and lock[1]==False,'Failed to clear write lock on image')

    def test5(self):
        '''Clear all locks'''
        tb.open(self.msfile)
        tbreadlock = tb.haslock(write=False)
        tb.lock()
        tbwritelock = tb.haslock(write=True)
        ia.open(self.img)
        ia.lock(writelock=True)
        lock = ia.haslock()
        self.assertTrue(tbreadlock==True and tbwritelock==True and lock[0]==True and lock[1]==True,
                        'Cannot acquire locks on table and/or image')
        clearstat()
        tbreadlock = tb.haslock(write=False)
        tbwritelock = tb.haslock(write=True)
        lock = ia.haslock()
        tb.close()
        ia.close()

        self.assertTrue(tbreadlock==False and tbwritelock==False and lock[0]==False and lock[1]==False,
                        'Failed to clear locks on table and/or image')
        


def suite():
    return [clearstat_test]
    
    
    