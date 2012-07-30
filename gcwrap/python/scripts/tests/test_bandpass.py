import os
import shutil
import testhelper as th
from __main__ import default
from tasks import bandpass
from taskinit import *
import unittest


''' Python unit tests for the bandpass task

These tests will only verify if the bandpass calibration
tables created for an MS and an MMS agree. These are
not full unit tests for the bandpass task.
'''

datapath = os.environ.get('CASAPATH').split()[0] +\
                            '/data/regression/unittest/bandpass/'

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

        default('bandpass')
               
        
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

        default('bandpass')


class bandpass1_test(test_base):

    def setUp(self):
        self.setUp_ngc5921()

    def tearDown(self):
        if os.path.lexists(self.msfile):
            shutil.rmtree(self.msfile)

        if os.path.lexists(self.mmsfile):
            shutil.rmtree(self.mmsfile)
        
    def test1a(self):
        '''Bandpass 1a: Create cal tables for the MS and MMS in field=0'''
        msbcal = self.msfile + '.bcal'
        bandpass(vis=self.msfile, caltable=msbcal, field='0',opacity=0.0,bandtype='B',
                 solint='inf',combine='scan',refant='VA15')
        self.assertTrue(os.path.exists(msbcal))
        
        mmsbcal = self.mmsfile + '.bcal'
        bandpass(vis=self.mmsfile, caltable=mmsbcal, field='0',opacity=0.0,bandtype='B',
                 solint='inf',combine='scan',refant='VA15')
        self.assertTrue(os.path.exists(mmsbcal))
        
        # Compare the calibration tables
        self.assertTrue(th.compTables(msbcal, mmsbcal, ['WEIGHT']))


class bandpass2_test(test_base):

    def setUp(self):
        self.setUp_ngc4826()
           
            
    def tearDown(self):
        if os.path.lexists(self.msfile):
            shutil.rmtree(self.msfile)

        if os.path.lexists(self.mmsfile):
            shutil.rmtree(self.mmsfile)
        
        
    def test1b(self):
        '''Bandpass 1b: Create cal tables for the MS and MMS split by spws'''
        msbcal = self.msfile + '.bcal'
        bandpass(vis=self.msfile, caltable=msbcal, field='0',spw='0', opacity=0.0,bandtype='B',
                 solint='inf',combine='scan',refant='ANT5')
        self.assertTrue(os.path.exists(msbcal))
        
        mmsbcal = self.mmsfile + '.bcal'
        bandpass(vis=self.mmsfile, caltable=mmsbcal, field='0',spw='0', opacity=0.0,bandtype='B',
                 solint='inf',combine='scan',refant='ANT5')
        self.assertTrue(os.path.exists(mmsbcal))
        
        # Compare the calibration tables
        self.assertTrue(th.compTables(msbcal, mmsbcal, ['WEIGHT']))

class bandpass_cleanup(unittest.TestCase):
    
    def tearDown(self):
        os.system('rm -rf ngc5921.*')
        os.system('rm -rf ngc4826*')

    def test_cleanup(self):
        '''Bandpass: Cleanup'''
        pass

def suite():
    return [bandpass1_test, bandpass2_test, bandpass_cleanup]






        
