import os
import shutil
import testhelper as th
import numpy as np
from __main__ import default
from tasks import fluxscale
from taskinit import *
import unittest


''' Python unit tests for the fluxscale task

These tests will only verify if the fluxscale
tables created for an MS and an MMS agree. These are
not full unit tests for the fluxscale task.
'''

datapath = os.environ.get('CASAPATH').split()[0] +\
                            '/data/regression/unittest/fluxscale/'


# Pick up alternative data directory to run tests on MMSs
testmms = False
if os.environ.has_key('TEST_DATADIR'):   
    DATADIR = str(os.environ.get('TEST_DATADIR'))+'/fluxscale/'
    if os.path.isdir(DATADIR):
        testmms = True
        datapath = DATADIR
    else:
        print 'WARN: directory '+DATADIR+' does not exist'

print 'fluxscale tests will use data from '+datapath         

    
# Not used yet
def getRefDict():
    '''Return a reference dictionary'''

    # This dictionary was created from the reference fluxscale table
    # stored in $CASADATA/regression/unittest_fluxscale. If that
    # file is recreated, this dictionary should be updated.
    
    refdict = {'1': {'fieldName': '1445+09900002_0',
                     'fluxd': np.array([ 0.1684016]),
                     'fluxdErr': np.array([ 0.00055571]),
                     'numSol': np.array([54], dtype='int32')},
               '2': {'fieldName': 'N5921_2',
                     'fluxd': np.array([ 0.00604025]),
                     'fluxdErr': np.array([ 0.00304653]),
                     'numSol': np.array([54], dtype='int32')},
                'freq': np.array([  1.41266507e+09]),
                'spwID': np.array([0], dtype='int32'),
                'spwName': np.array(['none'], dtype='|S5')}
        
    return refdict


class fluxscale1_test(unittest.TestCase):

    def setUp(self):
        # Input names
        prefix = 'ngc5921'
        self.msfile = prefix + '.ms'
        if testmms:
            self.msfile = prefix + '.mms'
                        
        self.gtable = prefix + '.ref1a.gcal'
        self.reffile = prefix + '.def.fcal'
        
        fpath = os.path.join(datapath,self.msfile)
        if os.path.lexists(fpath):        
            shutil.copytree(fpath, self.msfile)
            fpath = os.path.join(datapath,self.gtable)
            shutil.copytree(fpath, self.gtable)
            fpath = os.path.join(datapath,self.reffile)
            shutil.copytree(fpath, self.reffile)
        else:
            self.fail('Data does not exist -> '+fpath)

        default('fluxscale')

    def tearDown(self):
        if os.path.lexists(self.msfile):
            shutil.rmtree(self.msfile)
        
        os.system('rm -rf ngc5921*.fcal')
        os.system('rm -rf ngc5921*.gcal')

        
    def test_default(self):
        '''Fluxscale 1a: Create a flux table using field=0 as reference'''
        
        # Input
        gtable = self.gtable
        
        # Output
        outtable = self.msfile + '.fcal'
        
        thisdict = fluxscale(vis=self.msfile, caltable=gtable, fluxtable=outtable, reference='1331*', 
                  transfer='1445*')
        self.assertTrue(os.path.exists(outtable))
        
        # File to compare with
        reference = self.reffile
        
        # Compare the calibration table with a reference
        self.assertTrue(th.compTables(outtable, reference, ['WEIGHT']))
        
        # Check the created dictionary with the reference one
#        refdict = self.getRefDict()
#        diff_ret = th.DictDiffer(thisdict, refdict)
#        
#        self.assertFalse(diff_ret.changed())        
#        self.assertFalse(diff_ret.added())
#        self.assertFalse(diff_ret.removed())
#        self.assertEqual(diff_ret.unchanged().__len__(), 5)

        

class fluxscale2_test(unittest.TestCase):

    def setUp(self):
        # Input names
        prefix = 'ngc4826'
        self.msfile = prefix + '.ms'
        if testmms:
            self.msfile = prefix + '.mms'
            
        self.gtable = prefix + '.spw.gcal'
        self.reffile = prefix + '.spw.fcal'

        fpath = os.path.join(datapath,self.msfile)
        if os.path.lexists(fpath):
            shutil.copytree(fpath, self.msfile)
            fpath = os.path.join(datapath,self.gtable)
            shutil.copytree(fpath, self.gtable)
            fpath = os.path.join(datapath,self.reffile)
            shutil.copytree(fpath, self.reffile)
        else:
            self.fail('Data does not exist -> '+fpath)

        default('fluxscale')
           
            
    def tearDown(self):
        if os.path.lexists(self.msfile):
            shutil.rmtree(self.msfile)

        os.system('rm -rf ngc4826*.gcal')
        os.system('rm -rf ngc4826*.fcal')
        
        
    def test_spws(self):
        '''Fluxscale 1b: Create a fluxscale table for an MS with many spws'''
        # Input
        gtable = self.gtable
        
        # Output
        outtable = self.msfile + '.fcal'
        
        thisdict = fluxscale(vis=self.msfile, caltable=gtable, fluxtable=outtable, reference='3C273-F0',
                             transfer=['1310+323-F0'],refspwmap=[0,0])
        self.assertTrue(os.path.exists(outtable))
        
        # File to compare with
        reference = self.reffile
        
        # Compare the calibration table with a reference
        self.assertTrue(th.compTables(outtable, reference, ['WEIGHT']))
                

def suite():
    return [fluxscale1_test, fluxscale2_test]






        
