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

    
class fluxscale1_test(unittest.TestCase):

    def setUp(self):
        # Input names
        self.prefix = 'ngc5921'
        self.msfile = self.prefix + '.ms'
        if testmms:
            self.msfile = self.prefix + '.mms'
                        
        self.gtable = self.prefix + '.ref1a.gcal'
        self.reffile = self.prefix + '.def.fcal'
        self.reffile2 = self.prefix + '.def.inc.fcal'
        self.tearDown()
        
        fpath = os.path.join(datapath,self.msfile)
        if os.path.lexists(fpath):        
            shutil.copytree(fpath, self.msfile, symlinks=True)
            fpath = os.path.join(datapath,self.gtable)
            shutil.copytree(fpath, self.gtable, symlinks=True)
            fpath = os.path.join(datapath,self.reffile)
            shutil.copytree(fpath, self.reffile, symlinks=True)
            fpath = os.path.join(datapath,self.reffile2)
            shutil.copytree(fpath, self.reffile2, symlinks=True)
        else:
            self.fail('Data does not exist -> '+fpath)

        default('fluxscale')

    def tearDown(self):
        shutil.rmtree(self.msfile, ignore_errors=True)        
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

        # compare some determined values returned in the dict
        refdict={'1': {'spidxerr': np.array([ 0.,  0.,  0.]), 'spidx': np.array([ 0.,  0.,  0.]), \
                 'fluxdErr': np.array([0.00055571]), \
                 'fieldName': '1445+09900002_0', 'numSol': np.array([54]), \
                 'fluxd': np.array([0.16825763])}, \
                 'freq': np.array([1.41266507e+09]), \
                 'spwName': np.array(['none'], dtype='|S5'), \
                 'spwID': np.array([0])} 
        
        diff_fluxd=abs(refdict['1']['fluxd'][0]-thisdict['1']['fluxd'][0])/refdict['1']['fluxd'][0]
        self.assertTrue(diff_fluxd<1.5e-8)
        
            
    def test_incremental(self): 
        '''Fluxscale 1b: Create an incremental flux table using field=0 as reference'''
        # Input
        gtable = self.gtable

        # Output
        outtable = self.msfile + '.inc.fcal'

        thisdict = fluxscale(vis=self.msfile, caltable=gtable, fluxtable=outtable, reference='1331*',
                  transfer='1445*', incremental=True)
        self.assertTrue(os.path.exists(outtable))

        # File to compare with
        reference = self.reffile2

        # Compare the calibration table with a reference
        self.assertTrue(th.compTables(outtable, reference, ['WEIGHT']))


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
            shutil.copytree(fpath, self.msfile, symlinks=True)
            fpath = os.path.join(datapath,self.gtable)
            shutil.copytree(fpath, self.gtable, symlinks=True)
            fpath = os.path.join(datapath,self.reffile)
            shutil.copytree(fpath, self.reffile, symlinks=True)
        else:
            self.fail('Data does not exist -> '+fpath)

        default('fluxscale')
           
            
    def tearDown(self):
        shutil.rmtree(self.msfile, ignore_errors=True)
        os.system('rm -rf ngc4826*.gcal')
        os.system('rm -rf ngc4826*.fcal')
        
        
    def test_spws(self):
        '''Fluxscale 2: Create a fluxscale table for an MS with many spws'''
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
        
        # compare some determined values returned in the dict
        refdict={'1': {'spidxerr': np.array([ 0.,  0.,  0.]), 'spidx': np.array([ 0.,  0.,  0.]), \
                 'fluxdErr': np.array([-1.        ,  0.04080052, -1.        , -1.        , -1.        , -1.        ]), \
                 'fieldName': '1310+323-F0', 'numSol': np.array([-1,  8, -1, -1, -1, -1], dtype=np.int32), \
                 'fluxd': np.array([-1.        ,  1.44578847, -1.        , -1.        , -1.        , -1.        ])}, \
                 'freq': np.array([  1.15138579e+11,   1.15217017e+11,  -1.00000000e+00, -1.00000000e+00,  -1.00000000e+00, \
                                 -1.00000000e+00]), 'spwName': np.array(['', '', '', '', '', ''], dtype='|S1'), \
                 'spwID': np.array([0, 1, 2, 3, 4, 5], dtype=np.int32)} 
        diff_fluxd=abs(refdict['1']['fluxd'][1]-thisdict['1']['fluxd'][1])/refdict['1']['fluxd'][1]
        self.assertTrue(diff_fluxd<1.e-8)
 
def suite():
    return [fluxscale1_test, fluxscale2_test]






        
