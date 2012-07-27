import os
import shutil
import testhelper as th
from __main__ import default
from tasks import gaincal
from taskinit import *
import unittest


''' Python unit tests for the gaincal task

These tests will only verify if the gain calibration
tables created for an MS and an MMS agree. These are
not full unit tests for the gaincal task.
'''

datapath = os.environ.get('CASAPATH').split()[0] +\
                            '/data/regression/unittest/gaincal/'

# Base class which defines setUp functions
# for importing different data sets
class test_base(unittest.TestCase):
    
    def setUp_ngc5921(self):
        
        # Input names
        prefix = 'ngc5921'
        self.msfile = prefix + '.ms'
        self.mmsfile = prefix + '.mms'
        
        fpath = os.path.join(datapath,self.msfile)
        if os.path.lexists(fpath):        
            shutil.copytree(fpath, self.msfile)
        else:
            self.fail('Data does not exist -> '+fpath)

        fpath = os.path.join(datapath,self.mmsfile)
        if os.path.lexists(fpath):
            shutil.copytree(fpath, self.mmsfile)
        else:
            self.fail('Data does not exist -> '+fpath)

        default('gaincal')
               
        
    def setUp_ngc4826(self):
        
        # Input names
        prefix = 'ngc4826'
        self.msfile = prefix + '.ms'
        self.mmsfile = prefix + '_spw.mms'

        fpath = os.path.join(datapath,self.msfile)
        if os.path.lexists(fpath):
            shutil.copytree(fpath, self.msfile)
        else:
            self.fail('Data does not exist -> '+fpath)

        fpath = os.path.join(datapath,self.mmsfile)
        if os.path.lexists(fpath):
            shutil.copytree(fpath, self.mmsfile)
        else:
            self.fail('Data does not exist -> '+fpath)

        default('gaincal')


class gaincal1_test(test_base):

    def setUp(self):
        self.setUp_ngc5921()

    def tearDown(self):
        if os.path.lexists(self.msfile):
            shutil.rmtree(self.msfile)

        if os.path.lexists(self.mmsfile):
            shutil.rmtree(self.mmsfile)
        
    def test1a(self):
        '''Gaincal 1a: Create cal tables for the MS and MMS'''
        msgcal = self.msfile + '.gcal'
        gaincal(vis=self.msfile, caltable=msgcal)
        self.assertTrue(os.path.exists(msgcal))
        
        mmsgcal = self.mmsfile + '.gcal'
        gaincal(vis=self.mmsfile, caltable=mmsgcal)
        self.assertTrue(os.path.exists(mmsgcal))
        
        # Compare the calibration tables
        self.assertTrue(th.compTables(msgcal, mmsgcal, ['WEIGHT']))

    # Add test with data selection
    def test2a(self):
        '''Gaincal 2a: field selection'''
        
        msgcal = self.msfile + '.field0.gcal'
        gaincal(vis=self.msfile, caltable=msgcal, field='0', gaintype='G',solint='int',
                combine='',refant='VA02')
        self.assertTrue(os.path.exists(msgcal))
        
        mmsgcal = self.mmsfile + '.field0.gcal'
        gaincal(vis=self.mmsfile, caltable=mmsgcal, field='0', gaintype='G',solint='int',
                combine='',refant='VA02')
        self.assertTrue(os.path.exists(mmsgcal))
        
        # Compare the calibration tables
        self.assertTrue(th.compTables(msgcal, mmsgcal, ['WEIGHT']))
        


class gaincal2_test(test_base):

    def setUp(self):
        self.setUp_ngc4826()
           
            
    def tearDown(self):
        if os.path.lexists(self.msfile):
            shutil.rmtree(self.msfile)

        if os.path.lexists(self.mmsfile):
            shutil.rmtree(self.mmsfile)
        
        
    def test1b(self):
        '''Gaincal 1b: Create cal tables for the MS and MMS split by spws'''
        msgcal = self.msfile + '.gcal'
        gaincal(vis=self.msfile, caltable=msgcal, field='0,1',spw='0', gaintype='G',minsnr=2.0,
                refant='ANT5',gaincurve=False, opacity=0.0, solint='inf',combine='')
        self.assertTrue(os.path.exists(msgcal))
        
        mmsgcal = self.mmsfile + '.gcal'
        gaincal(vis=self.mmsfile, caltable=mmsgcal, field='0,1',spw='0', gaintype='G',minsnr=2.0,
                refant='ANT5',gaincurve=False, opacity=0.0, solint='inf',combine='')
        self.assertTrue(os.path.exists(mmsgcal))
        
        # Compare the calibration tables
        self.assertTrue(th.compTables(msgcal, mmsgcal, ['WEIGHT']))

class gaincal_cleanup(unittest.TestCase):
    
    def tearDown(self):
        os.system('rm -rf ngc5921.*')
        os.system('rm -rf ngc4826*')

    def test_cleanup(self):
        '''Gaincal: Cleanup'''
        pass

def suite():
    return [gaincal1_test, gaincal2_test, gaincal_cleanup]






        
