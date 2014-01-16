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

# Pick up alternative data directory to run tests on MMSs
testmms = False
if os.environ.has_key('TEST_DATADIR'):   
    DATADIR = str(os.environ.get('TEST_DATADIR'))+'/bandpass/'
    if os.path.isdir(DATADIR):
        testmms = True
        datapath = DATADIR
    else:
        print 'WARN: directory '+DATADIR+' does not exist'

print 'bandpass tests will use data from '+datapath         

# Base class which defines setUp functions
# for importing different data sets
class test_base(unittest.TestCase):

    def cleanUp(self):
        shutil.rmtree(self.msfile, ignore_errors=True)
        os.system('rm -rf '+self.msfile+'.bcal')
    
    def setUp_ngc5921(self):
        
        # Input names
        prefix = 'ngc5921'
        self.msfile = prefix + '.ms'
        if testmms:
            self.msfile = prefix + '.mms'

        self.reffile = datapath + prefix
        self.cleanUp()
        
        fpath = os.path.join(datapath,self.msfile)
        if os.path.lexists(fpath):        
            shutil.copytree(fpath, self.msfile)
        else:
            self.fail('Data does not exist -> '+fpath)

        default('bandpass')
               
        
    def setUp_ngc4826(self):
        
        # Input names
        prefix = 'ngc4826'
        self.msfile = prefix + '.ms'
        if testmms:
            self.msfile = prefix + '.mms'

        self.reffile = datapath + prefix
        self.cleanUp()
        
        fpath = os.path.join(datapath,self.msfile)
        if os.path.lexists(fpath):
            shutil.copytree(fpath, self.msfile)
        else:
            self.fail('Data does not exist -> '+fpath)

        default('bandpass')


class bandpass1_test(test_base):

    def setUp(self):
        self.setUp_ngc5921()

    def tearDown(self):
        if os.path.lexists(self.msfile):
            shutil.rmtree(self.msfile)

        os.system('rm -rf ngc5921*.bcal')
        
    def test1a(self):
        '''Bandpass 1a: Create bandpass table using field=0'''
        msbcal = self.msfile + '.bcal'
        reference = self.reffile + '.ref1a.bcal'
        bandpass(vis=self.msfile, caltable=msbcal, field='0',bandtype='B',
                 solint='inf',combine='scan',refant='VA15')
        self.assertTrue(os.path.exists(msbcal))
                
        # Compare the calibration tables
        self.assertTrue(th.compTables(msbcal, reference, ['WEIGHT']))


class bandpass2_test(test_base):

    def setUp(self):
        self.setUp_ngc4826()
                       
    def tearDown(self):
        if os.path.lexists(self.msfile):
            shutil.rmtree(self.msfile)


        os.system('rm -rf ngc4826*.bcal')
        
        
    def test1b(self):
        '''Bandpass 1b: Create cal tables for the MS and MMS split by spws'''
        msbcal = self.msfile + '.bcal'
        reference = self.reffile + '.ref1b.bcal'
        bandpass(vis=self.msfile, caltable=msbcal, field='0',spw='0',bandtype='B',
                 solint='inf',combine='scan',refant='ANT5')
        self.assertTrue(os.path.exists(msbcal))
        
        # Compare the calibration tables
        self.assertTrue(th.compTables(msbcal, reference, ['WEIGHT']))


def suite():
    return [bandpass1_test, bandpass2_test]






        
