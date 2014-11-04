import shutil
import unittest
import os
import numpy
import exceptions
from tasks import *
from taskinit import mstool, tbtool, msmdtool, aftool
from __main__ import default
import testhelper as th
import partitionhelper as ph
from recipes.listshapes import listshapes
from parallel.parallel_task_helper import ParallelTaskHelper
from parallel.parallel_data_helper import ParallelDataHelper
from unittest.case import expectedFailure


# Define the root for the data files
datapath = os.environ.get('CASAPATH').split()[0] + "/data/regression/unittest/mstransform/"

aflocal = aftool()

# Base class which defines setUp functions
# for importing different data sets
class test_base(unittest.TestCase):

    def setUp_4ants(self):
        # data set with spw=0~15, 64 channels each in TOPO
        self.vis = "Four_ants_3C286.ms"

        if os.path.exists(self.vis):
           self.cleanup()

        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

    def setUp_jupiter(self):
        # data col, spw=0,1 1 channel each, TOPO, field=0~12, 93 scans
        self.vis = 'jupiter6cm.demo-thinned.ms'
        if os.path.exists(self.vis):
           self.cleanup()

        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

    def setUp_g19(self):
        # data with spw=0~23 128 channel each in LSRK, field=0,1
        self.vis = 'g19_d2usb_targets_line-shortened-thinned.ms'
        if os.path.exists(self.vis):
           self.cleanup()

        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

    def setUp_3c84(self):
        # MS is as follows (scan=1):
        #  SpwID   #Chans   Corrs
        #   0      256      RR
        #   0      256      LL
        #   1      128      RR  LL
        #   2      64       RR  RL  LR  LL

        self.vis = '3c84scan1.ms'
        if os.path.exists(self.vis):
           self.cleanup()

        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

    def setUp_CAS_5013(self):

        self.vis = 'ALMA-data-mst-science-testing-CAS-5013-one-baseline-one-timestamp.ms'
        if os.path.exists(self.vis):
           self.cleanup()

        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

    def setUp_CAS_6733(self):

        self.vis = 'CAS-6733.ms'
        if os.path.exists(self.vis):
           self.cleanup()
            
        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)  
        
    def setUp_CAS_6941(self):

        self.vis = 'CAS-6941.ms'
        if os.path.exists(self.vis):
           self.cleanup()
            
        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default(mstransform) 
                           
    def createMMS(self, msfile, axis='auto',scans='',spws=''):
        '''Create MMSs for tests with input MMS'''
        prefix = msfile.rstrip('.ms')
        if not os.path.exists(msfile):
            os.system('cp -RL '+datapath + msfile +' '+ msfile)
        
        # Create an MMS for the tests
        self.testmms = prefix + ".test.mms"
        default(mstransform)
        
        if os.path.exists(self.testmms):
            os.system("rm -rf " + self.testmms)
            
        print "................. Creating test MMS .................."
        mstransform(vis=msfile, outputvis=self.testmms, datacolumn='data',
                    createmms=True,separationaxis=axis, scan=scans, spw=spws)
        

    def cleanup(self):
        os.system('rm -rf '+ self.vis)


class test_mms_transformations(test_base):
    ''' Tests for combinespws parameter'''

    def setUp(self):
        self.setUp_4ants()

    def tearDown(self):
        os.system('rm -rf '+ self.vis)
#        os.system('rm -rf '+ self.outputms)
        os.system('rm -rf inpmms*.*ms combcvel*ms testmms*ms list.obs')

    def test_combspw1_3(self):
        '''mstransform: Do not combine spws and create MMS with axis scan.'''
        self.setUp_jupiter()
        self.outputms = 'combspw13.mms'
        mstransform(vis=self.vis, outputvis=self.outputms, combinespws=False, spw='0,1',field = '12',
             datacolumn='DATA', createmms=True, separationaxis='scan')

        self.assertTrue(os.path.exists(self.outputms))

        # Should create 6 subMSs
        mslocal = mstool()
        mslocal.open(thems=self.outputms)
        sublist = mslocal.getreferencedtables()
        mslocal.close()
        self.assertEqual(sublist.__len__(), 6, 'Should have created 6 subMSs')

        ret = th.verifyMS(self.outputms, 2, 1, 0)
        self.assertTrue(ret[0],ret[1])

    def test_combspw1_4(self):
        '''mstransform: Combine some channels of two spws using MMS input'''
        # same test as test_combspw1_2
        mmsfile = "inpmms14.mms"
        # First create an MMS
        mstransform(vis=self.vis, outputvis=mmsfile, spw='0,1', createmms=True)

        # Now do the same as in test_combspw1_2. Datacolumn moved to DATA
        self.outputms = "combspw14.ms"
        mstransform(vis=mmsfile, outputvis=self.outputms, combinespws=True, spw='0:60~63,1:60~63',
                    datacolumn='data', disableparallel=True)
        self.assertTrue(ParallelDataHelper.isParallelMS(self.outputms), 'Output should be an MMS')

        # The spws contain gaps, therefore the number of channels is bigger
        ret = th.verifyMS(self.outputms, 1, 68, 0)
        self.assertTrue(ret[0],ret[1])

        # Compare with cvel results
        default(cvel)
        cvel(vis=self.vis, outputvis='combcvel14.ms', spw='0:60~63,1:60~63')
        ret = th.verifyMS('combcvel14.ms', 1, 68, 0)
        self.assertTrue(ret[0],ret[1])        

    def test_regrid1_3(self):
        '''mstransform: Default regridms with spw selection using input MMS'''
        # same as test_regrid1_1
        mmsfile = 'testmms13.mms'
        # Create input MMS
        mstransform(vis=self.vis, outputvis=mmsfile, createmms=True, disableparallel=True,
                    separationaxis='scan')

        self.outputms = "reg13.ms"
        mstransform(vis=mmsfile, outputvis=self.outputms, regridms=True, spw='1,3,5,7',
                    datacolumn='DATA')
        self.assertTrue(os.path.exists(self.outputms))

        # The regriding should be the same as the input
        for i in range(4):
            ret = th.verifyMS(self.outputms, 4, 64, i)
            self.assertTrue(ret[0],ret[1])

        listobs(self.outputms)

        # Verify that some sub-tables are properly re-indexed.
        spw_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(spw_col.keys().__len__(), 4, 'Wrong number of rows in DD table')
        self.assertEqual(spw_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(spw_col['r2'][0], 1,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(spw_col['r3'][0], 2,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(spw_col['r4'][0], 3,'Error re-indexing DATA_DESCRIPTION table')

    def test_shape3(self):
        '''mstransform: DATA and FLAG tileshapes should be the same'''
        self.outputms = "shape3.ms"
        inptsh = [4,10,1024]
        mstransform(vis=self.vis, outputvis=self.outputms, createmms=True, tileshape=inptsh)

        self.assertTrue(os.path.exists(self.outputms))

        # Get the tile shape for the DATA output
        tblocal = tbtool()
        tblocal.open(self.outputms)
        outdm = tblocal.getdminfo()
        tblocal.close()
        outtsh = th.getTileShape(outdm)
        # And for the FLAG column
        flagtsh = th.getTileShape(outdm, 'FLAG')

        self.assertTrue((outtsh==flagtsh).all(), 'Tile shapes are different')

    def test_channels_mms1(self):
        '''mstransform: create MMS with spw separation and channel selections'''
        self.outputms = "testmms1.mms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='0~4,5:1~10',createmms=True,
                    separationaxis='spw',disableparallel=True)

        self.assertTrue(os.path.exists(self.outputms))

        # It should create 6 subMS, with spw=0~5
        # spw=5 should have only 10 channels
        ret = th.verifyMS(self.outputms, 6, 10, 5,ignoreflags=True)
        self.assertTrue(ret[0],ret[1])

        # The separation axis should be written to the output MMS
        sepaxis = ph.axisType(self.outputms)
        self.assertEqual(sepaxis, 'spw', 'AxisType is not correctly written to output MMS')

    def test_channels_mms2(self):
        '''mstransform: create MMS with spw/scan separation and channel selections'''
        self.outputms = "testmms2.mms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='0:0~10,1:60~63',createmms=True,
                    separationaxis='auto', disableparallel=True)

        self.assertTrue(os.path.exists(self.outputms))

        # It should create 4 subMS, with spw=0~1
        # spw=0 has 11 channels, spw=1 has 4 channels
        ret = th.verifyMS(self.outputms, 2, 11, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 2, 4, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])

        # Verify that some sub-tables are properly re-indexed.
        spw_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(spw_col.keys().__len__(), 2, 'Wrong number of rows in DD table')
        self.assertEqual(spw_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(spw_col['r2'][0], 1,'Error re-indexing DATA_DESCRIPTION table')

        # The separation axis should be written to the output MMS
        sepaxis = ph.axisType(self.outputms)
        self.assertEqual(sepaxis, 'scan,spw', 'AxisType is not correctly written to output MMS')

    def test_channels_mms3(self):
        '''mstransform: create MMS with scan separation and channel selections'''
        self.outputms = "testmms3.mms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='0:0~10,1:60~63',createmms=True,
                    separationaxis='scan', disableparallel=True)
        self.assertTrue(os.path.exists(self.outputms))

        # It should create 2 subMS, with spw=0~1
        # spw=0 has 11 channels, spw=1 has 4 channels
        ret = th.verifyMS(self.outputms, 2, 11, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 2, 4, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])

        # The separation axis should be written to the output MMS
        sepaxis = ph.axisType(self.outputms)
        self.assertEqual(sepaxis, 'scan', 'AxisType is not correctly written to output MMS')

    def test_channels_mms4(self):
        '''mstransform: verify spw sub-table consolidation in sequential'''
        self.outputms = "testmms4.mms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='3,5:10~20,7,11,13',createmms=True,
                    separationaxis='spw', disableparallel=True)
        self.assertTrue(os.path.exists(self.outputms))

        # spw=5 should be spw=1 after consolidation, with 10 channels
        ret = th.verifyMS(self.outputms, 7, 10, 1, ignoreflags=True)
        
    def test_CAS6206(self):
        '''mstransform: verify that all columns are re-indexed in SPW sub-table'''
        self.outputmms='test.mms'
        self.outputms='assoc.ms'
        self.setUp_CAS_5013()
        mstransform(vis=self.vis, outputvis=self.outputmms,createmms=True, datacolumn='corrected')
        
        # Check that optional ASSOC_SPW_ID is the same in input and output
        tblocal = tbtool()
        tblocal.open(self.vis+'/SPECTRAL_WINDOW',nomodify=True)
        in_assoc = tblocal.iscelldefined('ASSOC_SPW_ID',0)
        tblocal.close()
        tblocal.open(self.outputmms+'/SPECTRAL_WINDOW',nomodify=True)
        out_assoc = tblocal.iscelldefined('ASSOC_SPW_ID',0)
        tblocal.close()
        self.assertEqual(in_assoc, out_assoc, 'Error in SPW sub-table creation; ASSOC_SPW_ID is different')
        
        # if SPW sub-table is not correct, the next step might fail
        self.assertTrue(mstransform(vis=self.outputmms, outputvis=self.outputms, hanning=True, datacolumn='data'))
        

class test_mms_freqavg(test_base):
    '''Tests for frequency averaging'''
    def setUp(self):
        self.setUp_g19()

    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outputms)

    def test_freqavg6(self):
        '''mstranform: Average all channels of one spw, save as an MMS'''
        # same as test_freqavg3
        self.outputms = "favg6.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='23', chanaverage=True, chanbin=128,
                    createmms=True, disableparallel=True)

        self.assertTrue(os.path.exists(self.outputms))
        ret = th.verifyMS(self.outputms, 1, 1, 0)
        self.assertTrue(ret[0],ret[1])

    def test_freqavg7(self):
        '''mstranform: Average using different bins for several spws, output MMS'''
        # same as test_freqavg4
        self.outputms = "favg7.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='10,12,20', chanaverage=True,
                    chanbin=[128,4,10], createmms=True, separationaxis='scan', disableparallel=True)

        self.assertTrue(os.path.exists(self.outputms))

        # Output should be:
        # spw=0 1 channel
        # spw=1 32 channels
        # spw=3 13 channels
        ret = th.verifyMS(self.outputms, 3, 1, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 3, 32, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 3, 12, 2, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])

        # Verify that some sub-tables are properly re-indexed.
        spw_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(spw_col.keys().__len__(), 3, 'Wrong number of rows in DD table')
        self.assertEqual(spw_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(spw_col['r2'][0], 1,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(spw_col['r3'][0], 2,'Error re-indexing DATA_DESCRIPTION table')

    def test_freqavg8(self):
        '''mstranform: Average using different bins for several spws, output MMS'''
        # same as test_freqavg4
        self.outputms = "favg8.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='10,12,20', chanaverage=True,
                    chanbin=[128,4,10], createmms=True, separationaxis='spw',numsubms=2,
                    disableparallel=True)

        self.assertTrue(os.path.exists(self.outputms))

        # Output should be:
        # spw=0 1 channel
        # spw=1 32 channels
        # spw=3 13 channels
        ret = th.verifyMS(self.outputms, 3, 1, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 3, 32, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 3, 12, 2, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])

    def test_freqavg9(self):
        '''mstranform: Average using different bins and a channel selection, output MMS'''
        self.outputms = "favg9.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='2,12,10:1~10', chanaverage=True,
                    chanbin=[32,128,5], createmms=True, separationaxis='spw')

        self.assertTrue(os.path.exists(self.outputms))

        # Output should be:
        # spw=0 4 channels
        # spw=1 1 channel
        # spw=2 2 channels
        ret = th.verifyMS(self.outputms, 3, 4, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 3, 1, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 3, 2, 2, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])

        # Verify that some sub-tables are properly re-indexed.
        spw_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(spw_col.keys().__len__(), 3, 'Wrong number of rows in DD table')
        self.assertEqual(spw_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(spw_col['r2'][0], 1,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(spw_col['r3'][0], 2,'Error re-indexing DATA_DESCRIPTION table')

    def test_freqavg10(self):
        '''mstranform: Average using different bins, channel selection, both axes, output MMS'''
        self.outputms = "favg10.ms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='2,12,10:1~10', chanaverage=True,
                    chanbin=[32,128,5], createmms=True, separationaxis='auto')

        self.assertTrue(os.path.exists(self.outputms))

        # Should create 6 subMSs
        mslocal = mstool()
        mslocal.open(thems=self.outputms)
        sublist = mslocal.getreferencedtables()
        mslocal.close()
        self.assertEqual(sublist.__len__(), 6, 'Should have created 6 subMSs')

        # Output should be:
        # spw=0 4 channels
        # spw=1 1 channel
        # spw=2 2 channels
        ret = th.verifyMS(self.outputms, 3, 4, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 3, 1, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 3, 2, 2, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])

        # Verify that some sub-tables are properly re-indexed.
        spw_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(spw_col.keys().__len__(), 3, 'Wrong number of rows in DD table')
        self.assertEqual(spw_col['r1'][0], 0,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(spw_col['r2'][0], 1,'Error re-indexing DATA_DESCRIPTION table')
        self.assertEqual(spw_col['r3'][0], 2,'Error re-indexing DATA_DESCRIPTION table')


class test_mms_parallel(test_base):
    '''Run some of the same tests in parallel'''
    def setUp(self):
        self.setUp_4ants()

    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outputms)

    def test_parallel1(self):
        '''mstransform: create MMS with spw separation and channel selections in parallel'''
        self.outputms = "parallel1.mms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='0~4,5:1~10',createmms=True,
                    separationaxis='spw')

        self.assertTrue(os.path.exists(self.outputms))

        # It should create 6 subMS, with spw=0~5
        # spw=5 should have only 10 channels
        ret = th.verifyMS(self.outputms, 6, 10, 5,ignoreflags=True)
        self.assertTrue(ret[0],ret[1])

    def test_parallel2(self):
        '''mstransform: create MMS with spw/scan separation and channel selections in parallel'''
        self.outputms = "parallel2.mms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='0:0~10,1:60~63',createmms=True,
                    separationaxis='auto')

        self.assertTrue(os.path.exists(self.outputms))

        # It should create 4 subMS, with spw=0~1
        # spw=0 has 11 channels, spw=1 has 4 channels
        ret = th.verifyMS(self.outputms, 2, 11, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 2, 4, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])

        # Check the FEED table
        out_feed_spw = th.getVarCol(self.outputms+'/FEED', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(len(out_feed_spw.keys()), 8)
        self.assertEqual(out_feed_spw['r1'], 0)
        self.assertEqual(out_feed_spw['r5'], 1)

    def test_parallel3(self):
        '''mstransform: create MMS with scan separation and channel selections in parallel'''
        self.outputms = "parallel3.mms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='0:0~10,1:60~63',createmms=True,
                    separationaxis='scan')
        self.assertTrue(os.path.exists(self.outputms))

        # It should create 2 subMS, with spw=0~1
        # spw=0 has 11 channels, spw=1 has 4 channels
        ret = th.verifyMS(self.outputms, 2, 11, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])
        ret = th.verifyMS(self.outputms, 2, 4, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])

    def test_parallel4(self):
        '''mstransform: verify spw sub-table consolidation in sequential'''
        self.outputms = "parallel4.mms"
        mstransform(vis=self.vis, outputvis=self.outputms, spw='3,5:10~20,7,9,15',createmms=True,
                    separationaxis='spw')
        self.assertTrue(os.path.exists(self.outputms))

        # spw=5 should be spw=1 after consolidation, with 10 channels
        ret = th.verifyMS(self.outputms, 7, 10, 1, ignoreflags=True)
        
        # Check the FEED table
        out_feed_spw = th.getVarCol(self.outputms+'/FEED', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(len(out_feed_spw.keys()), 20)
        self.assertEqual(out_feed_spw['r1'], 0)
        self.assertEqual(out_feed_spw['r5'], 1)
        self.assertEqual(out_feed_spw['r9'], 2)
        self.assertEqual(out_feed_spw['r13'], 3)
        self.assertEqual(out_feed_spw['r17'], 4)

    def test_parallel5(self):
        '''mstransform: Do not combine spws and create MMS with axis scan in parallel.'''
        self.setUp_jupiter()
        self.outputms = 'parallel5.mms'
        mstransform(vis=self.vis, outputvis=self.outputms, combinespws=False, spw='0,1',field = '12',
             datacolumn='DATA', createmms=True, separationaxis='scan')

        self.assertTrue(os.path.exists(self.outputms))

        # Should create 6 subMSs
        mslocal = mstool()
        mslocal.open(thems=self.outputms)
        sublist = mslocal.getreferencedtables()
        mslocal.close()
        self.assertEqual(sublist.__len__(), 6, 'Should have created 6 subMSs')

        ret = th.verifyMS(self.outputms, 2, 1, 0)
        self.assertTrue(ret[0],ret[1])


class test_mms_spw_poln(test_base):
    '''tests for spw with different correlation shapes'''

    def setUp(self):
        self.setUp_3c84()

    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outputms)
        os.system('rm -rf list.obs')

    def test_mms_spw_selection(self):
        '''mstransform: Create MMS and select two spws with different polarization shapes'''
        self.outputms = '3cspw12.mms'
        mstransform(vis=self.vis, outputvis=self.outputms, datacolumn='data', spw='1,2',
                    createmms=True, separationaxis='spw')

        # Verify the input versus the output
        myms = mstool()
        myms.open(self.vis)
        myms.msselect({'spw':'1,2'})
        inp_nrow = myms.nrow()
        myms.close()

        myms.open(self.outputms)
        out_nrow = myms.nrow()
        myms.close()
        self.assertEqual(inp_nrow, out_nrow)

        # Verify that DATA_DESCRIPTION table is properly re-indexed.
        spw_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(spw_col.keys().__len__(), 2, 'Wrong number of rows in DD table')
        self.assertEqual(spw_col['r1'][0], 0,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')
        self.assertEqual(spw_col['r2'][0], 1,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')

        pol_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'POLARIZATION_ID')
        self.assertEqual(pol_col['r1'][0], 2,'Error in POLARIZATION_ID of DATA_DESCRIPTION table')
        self.assertEqual(pol_col['r2'][0], 3,'Error in POLARIZATION_ID of DATA_DESCRIPTION table')

        # Verify that POLARIZATION table is not re-sized.
        corr_col = th.getVarCol(self.outputms+'/POLARIZATION', 'NUM_CORR')
        self.assertEqual(corr_col.keys().__len__(), 4, 'Wrong number of rows in POLARIZATION table')

        # Check the FEED table
        out_feed_spw = th.getVarCol(self.outputms+'/FEED', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(len(out_feed_spw.keys()), 52)

    def test_mms_spw_selection2(self):
        '''mstransform: Create MMS and select two spws with different polarization shapes'''
        self.outputms = '3cspw01.mms'
        # spw=0 contains two DD in DATA_DESCRIPTION table
        mstransform(vis=self.vis, outputvis=self.outputms, datacolumn='data', spw='0,1',
                    createmms=True, separationaxis='spw')

        # Verify the input versus the output
        myms = mstool()
        myms.open(self.vis)
        myms.msselect({'spw':'0,1'})
        inp_nrow = myms.nrow()
        myms.close()

        myms.open(self.outputms)
        out_nrow = myms.nrow()
        myms.close()
        self.assertEqual(inp_nrow, out_nrow)

        # Verify that DATA_DESCRIPTION table is properly re-indexed.
        spw_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(spw_col.keys().__len__(), 3, 'Wrong number of rows in DD table')
        self.assertEqual(spw_col['r1'][0], 0,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')
        self.assertEqual(spw_col['r2'][0], 0,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')
        self.assertEqual(spw_col['r3'][0], 1,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')

        pol_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'POLARIZATION_ID')
        self.assertEqual(pol_col['r1'][0], 0,'Error in POLARIZATION_ID of DATA_DESCRIPTION table')
        self.assertEqual(pol_col['r2'][0], 1,'Error in POLARIZATION_ID of DATA_DESCRIPTION table')
        self.assertEqual(pol_col['r3'][0], 2,'Error in POLARIZATION_ID of DATA_DESCRIPTION table')

        # Verify that POLARIZATION table is not re-sized.
        corr_col = th.getVarCol(self.outputms+'/POLARIZATION', 'NUM_CORR')
        self.assertEqual(corr_col.keys().__len__(), 4, 'Wrong number of rows in POLARIZATION table')

        # Check the FEED table
        out_feed_spw = th.getVarCol(self.outputms+'/FEED', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(len(out_feed_spw.keys()), 52)

    def test_mms_spw_selection3(self):
        '''mstransform: Create MMS and select three spws with numsubms=2'''
        self.outputms = '3cspw012.mms'
        mstransform(vis=self.vis, outputvis=self.outputms, datacolumn='data', spw='0,1,2',
                    createmms=True, separationaxis='spw', numsubms=2)

        # Verify the input versus the output
        msmdt = msmdtool()
        msmdt.open(self.outputms)
        out_dds = msmdt.datadescids()
        out_nrow = msmdt.nrows()
        msmdt.done()

        self.assertTrue(out_nrow,5200)
        ref = [0,1,2,3]
        for i in out_dds:
            self.assertEqual(out_dds[i], ref[i])

        # Verify that DATA_DESCRIPTION table is properly re-indexed.
        spw_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(spw_col.keys().__len__(), 4, 'Wrong number of rows in DD table')
        self.assertEqual(spw_col['r1'][0], 0,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')
        self.assertEqual(spw_col['r2'][0], 0,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')
        self.assertEqual(spw_col['r3'][0], 1,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')
        self.assertEqual(spw_col['r4'][0], 2,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')
        
        in_feed_tb = th.getVarCol(self.vis+'/FEED', 'SPECTRAL_WINDOW_ID')
        out_feed_tb = th.getVarCol(self.outputms+'/FEED', 'SPECTRAL_WINDOW_ID')
        
        # Check the FEED table
        th.compTables(self.vis+'/FEED', self.outputms+'/FEED', ['FOCUS_LENGTH'])

    def test_mms_scan_spw_partition(self):
        '''mstransform: Create MMS and part by scan/spw'''
        self.outputms = '3cscanspw02.mms'
        mstransform(vis=self.vis, outputvis=self.outputms, datacolumn='data', spw='0,2',
                    createmms=True, disableparallel=True)

        # Verify the input versus the output
        msmdt = msmdtool()
        msmdt.open(self.outputms)
        out_dds = msmdt.datadescids()
        msmdt.done()

        ref = [0,1,2]
        for i in out_dds:
            self.assertEqual(out_dds[i], ref[i])

        # Verify that DATA_DESCRIPTION table is properly re-indexed.
        spw_col = th.getVarCol(self.outputms+'/DATA_DESCRIPTION', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(spw_col.keys().__len__(), 3, 'Wrong number of rows in DD table')
        self.assertEqual(spw_col['r1'][0], 0,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')
        self.assertEqual(spw_col['r2'][0], 0,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')
        self.assertEqual(spw_col['r3'][0], 1,'Error re-indexing SPECTRAL_WINDOW_ID of DATA_DESCRIPTION table')
        
        # Check the FEED table
        out_feed_spw = th.getVarCol(self.outputms+'/FEED', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(out_feed_spw['r1'],[0])
        self.assertEqual(out_feed_spw['r26'],[0])
        self.assertEqual(out_feed_spw['r27'],[1])
        self.assertEqual(out_feed_spw['r28'],[1])
        self.assertEqual(out_feed_spw['r51'],[1])
        self.assertEqual(len(out_feed_spw.keys()), 52)

    def test_mms_XXYY_selection(self):
        '''mstransform: correlation='RR,LL' should select and re-index properly'''
        self.outputms = '3cRRLL.mms'
        # spw 0 should not be processed. The selection should happen before the MMS work
        mstransform(vis=self.vis, outputvis=self.outputms, datacolumn='data', correlation='RR,LL',
                    createmms=True)
        
        msmdt = msmdtool()
        msmdt.open(self.outputms)
        out_dds = msmdt.datadescids()
        msmdt.done()
        
        ref = [0,1]
        for i in out_dds:
            self.assertEqual(out_dds[i], ref[i])
        
        pol_col = th.getVarCol(self.outputms+'/POLARIZATION','NUM_CORR')
        self.assertEqual(pol_col['r1'][0], 0,'Error in NUM_CORR of POLARIZATION table')
        self.assertEqual(pol_col['r2'][0], 0,'Error in NUM_CORR of POLARIZATION table')
        self.assertEqual(pol_col['r3'][0], 2,'Error in NUM_CORR of POLARIZATION table')
        self.assertEqual(pol_col['r4'][0], 2,'Error in NUM_CORR of POLARIZATION table')

        # Verify that POLARIZATION table is not re-sized.
        corr_col = th.getVarCol(self.outputms+'/POLARIZATION', 'NUM_CORR')
        self.assertEqual(corr_col.keys().__len__(), 4, 'Wrong number of rows in POLARIZATION table')

        # Check the FEED table
        out_feed_spw = th.getVarCol(self.outputms+'/FEED', 'SPECTRAL_WINDOW_ID')
        self.assertEqual(len(out_feed_spw.keys()), 52)
        
        # listobs, listpartition should not fail
        listobs(self.outputms, listfile='3c_1.obs')
        self.assertTrue(os.path.exists('3c_1.obs'), 'Probable error in sub-table re-indexing')
        listpartition(self.outputms, listfile='3c_2.obs')
        self.assertTrue(os.path.exists('3c_2.obs'), 'Probable error in sub-table re-indexing')


      
class test_mms_input(test_base):
    '''Tests when vis is an MMS'''
    
    def setUp(self):
        self.setUp_4ants()
        
    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outputms)
        
    def test_MMS1(self):
        '''mstransform: input MMS should be the same as output MMS'''
        
        # Create an MMS in the setup
        self.createMMS(self.vis, axis='scan', spws='0,1')
                
        # Create another MS and compare. They should be the same
        self.outputms = 'thesame.mms'
        mstransform(vis=self.testmms, outputvis=self.outputms, datacolumn='data')
        
        self.assertTrue(ParallelDataHelper.isParallelMS(self.outputms),'Output is not an MMS')
                
        # Sort the MSs so that they can be compared
        myms = mstool()
        
        myms.open(self.testmms)
        myms.sort('input_sorted.ms',['OBSERVATION_ID','ARRAY_ID','SCAN_NUMBER','FIELD_ID','DATA_DESC_ID','ANTENNA1','ANTENNA2','TIME'])
        myms.done()
        
        myms.open(self.outputms)
        myms.sort('output_sorted.ms',['OBSERVATION_ID','ARRAY_ID','SCAN_NUMBER','FIELD_ID','DATA_DESC_ID','ANTENNA1','ANTENNA2','TIME'])
        myms.done()

        # Compare both tables. Ignore the DATA column and compare it in next line
        self.assertTrue(th.compTables('input_sorted.ms','output_sorted.ms', 
                                      ['FLAG_CATEGORY','FLAG','WEIGHT_SPECTRUM','SIGMA_SPECTRUM','DATA']))
        
        # Compare the DATA column
        self.assertTrue(th.compVarColTables('input_sorted.ms','output_sorted.ms','DATA'))
        
        # The separation axis should be copied to the output MMS
        in_sepaxis = ph.axisType(self.testmms)
        out_sepaxis = ph.axisType(self.outputms)
        self.assertEqual(in_sepaxis, out_sepaxis, 'AxisTypes from input and output MMS do not match')

    def test_split_MMS(self):
        '''mstransform: Split MMS in parallel'''
        # Create an MMS in the setup. It creates self.testmms
        self.createMMS(self.vis, axis='scan', spws='0,1')
        
        self.outputms = 'scan30.mms'
        mstransform(vis=self.testmms, outputvis=self.outputms, datacolumn='data', scan='30')
        
        self.assertTrue(ParallelTaskHelper.isParallelMS(self.outputms),'Output is not an MMS')
        
        mslocal = mstool()
        mslocal.open(self.outputms)
        sublist = mslocal.getreferencedtables()
        self.assertEqual(len(sublist), 1)
        
        # Test DD table
        msmdt = msmdtool()
        msmdt.open(self.outputms)
        out_dds = msmdt.datadescids()
        msmdt.done()
        
        ref = [0,1]
        for i in out_dds:
            self.assertEqual(out_dds[i], ref[i])

        # The separation axis should be copied to the output MMS
        in_sepaxis = ph.axisType(self.testmms)
        out_sepaxis = ph.axisType(self.outputms)
        self.assertEqual(in_sepaxis, out_sepaxis, 'AxisTypes from input and output MMS do not match')
            
    def test_MMS_as_monolithicMS(self):
        '''mstransform: MMS should be processed as a monolithic MS'''
        # Create an MMS in the setup. It creates self.testmms
        self.createMMS(self.vis, axis='spw', spws='2,4,6')
        
        self.outputms = 'monolithicMMS.mms'
        # Treat MMS as a monolithic MS and create an output MMS with different separation axis.
        mstransform(vis=self.testmms, outputvis=self.outputms, datacolumn='data', combinespws=True)
        self.assertTrue(ParallelDataHelper.isParallelMS(self.outputms),'Output should be an MMS')
        
        # The separation axis should be copied to the output MMS
        in_sepaxis = ph.axisType(self.testmms)
        out_sepaxis = ph.axisType(self.outputms)
        self.assertNotEqual(in_sepaxis, out_sepaxis, 'AxisTypes from input and output MMS should not match')        

        ret = th.verifyMS(self.outputms, 1, 320, 0)
        self.assertTrue(ret[0],ret[1])

        listobs(self.outputms, listfile='list1.obs')
        self.assertTrue(os.path.exists('list1.obs'), 'Probable error in sub-table re-indexing')
        
    def test_monolithic_combspw1_1(self):
        '''mstransform: Combine four spws into one using a monolithic-MMS'''
        self.createMMS(self.vis, axis='spw',spws='0~3')

        self.outputms = "monocombspw11.ms"
        mstransform(vis=self.testmms, outputvis=self.outputms, datacolumn='data',combinespws=True, spw='0~3')
        self.assertTrue(ParallelDataHelper.isParallelMS(self.outputms),'Output should be an MMS')

        ret = th.verifyMS(self.outputms, 1, 256, 0)
        self.assertTrue(ret[0],ret[1])

        listobs(self.outputms, listfile='list2.obs')
        self.assertTrue(os.path.exists('list2.obs'), 'Probable error in sub-table re-indexing')
        
    def test_timespan_scan_axis(self):
        '''mstransform: timeaverage=True, timespan=scan, separationaxis=scan'''
        self.createMMS(self.vis, axis='scan',spws='10')
        self.outputms = "spanscan_scan.mms"
        # subMSs do not have all scans (30,31). Treat MMS as a monolithic MS
        mstransform(vis=self.testmms, outputvis=self.outputms, datacolumn='data',timeaverage=True, 
                    timebin='100s',timespan='scan')
        self.assertTrue(ParallelDataHelper.isParallelMS(self.outputms),'Output should be an MMS')
        self.assertEqual(ph.axisType(self.outputms),'spw')
        
        mymsmd = msmdtool()
        mymsmd.open(self.outputms)
        t30 = mymsmd.exposuretime(30)['value']
        t31 = mymsmd.exposuretime(31)['value']
        mymsmd.close()
        self.assertEqual(t30, 100)
        self.assertEqual(t31, 79)
        
    def test_timespan_spw_axis(self):
        '''mstransform: timeaverage=True, timespan=scan, separationaxis=spw'''
        self.createMMS(self.vis, axis='spw',spws='1,3')
        self.outputms = "spanscan_spw.mms"
        mstransform(vis=self.testmms, outputvis=self.outputms, datacolumn='data',timeaverage=True, 
                    timebin='100s',timespan='scan')
        self.assertTrue(ParallelDataHelper.isParallelMS(self.outputms),'Output should be an MMS')
        self.assertEqual(ph.axisType(self.outputms),'spw')
        
        mymsmd = msmdtool()
        mymsmd.open(self.outputms)
        t30 = mymsmd.exposuretime(30)['value']
        t31 = mymsmd.exposuretime(31)['value']
        mymsmd.close()
        self.assertEqual(t30, 100)
        self.assertEqual(t31, 79)
        
    def test_combspws_timespan_error(self):
        '''mstransform: combinespws=True, timespan=scan axis=auto timebin=40s'''
        self.createMMS(self.vis, axis='auto',spws='1,3')
        self.outputms = "spanscan_comb.mms"
        # combinespws is not possible. It should create and MS
        mstransform(vis=self.testmms, outputvis=self.outputms, datacolumn='data',
                    combinespws=True, timeaverage=True, timebin='40s',timespan='scan')
        self.assertFalse(ParallelDataHelper.isParallelMS(self.outputms),'Output should be an MS')
        mymsmd = msmdtool()
        mymsmd.open(self.outputms)
        nspw = mymsmd.nspw()
        mymsmd.close()
        self.assertEqual(nspw,1)
        
    def test_combspws_timespan(self):
        '''mstransform: combinespws=True, timespan=scan axis=auto'''
        self.createMMS(self.vis, axis='auto',spws='3')
        self.outputms = "2transformations.mms"
        # This should work. 
        mstransform(vis=self.testmms, outputvis=self.outputms, datacolumn='data',
                        combinespws=True, timeaverage=True, timebin='40s',timespan='scan')
        self.assertFalse(ParallelDataHelper.isParallelMS(self.outputms),'Output should be an MS')
      
    def test_combspws_timespan_fail(self):
        '''mstransform: combinespws=True, timespan=scan axis=auto timebin=200s'''
        self.createMMS(self.vis, axis='auto',spws='3')
        self.outputms = "errormms.mms"
        # Scans are shorter than timebin. Create an MS
        mstransform(vis=self.testmms, outputvis=self.outputms, datacolumn='data',
                    combinespws=True, timeaverage=True, timebin='200s',timespan='scan')
        self.assertFalse(ParallelDataHelper.isParallelMS(self.outputms),'Output should be an MS')
        mymsmd = msmdtool()
        mymsmd.open(self.outputms)
        nscan = mymsmd.nscans()
        exposure = mymsmd.exposuretime(30)['value']
        mymsmd.close()
        self.assertEqual(nscan,1)
        self.assertEqual(exposure, 179)

    def test_combspws_timespan_spw_axis(self):
        '''mstransform: combinespws=True, timespan=scan axis=spw'''
        self.createMMS(self.vis, axis='spw',scans='30',spws='10')
        self.outputms = "spwaxisok.mms"
        # This should work
        try:
            mstransform(vis=self.testmms, outputvis=self.outputms, datacolumn='data',
                        combinespws=True, timeaverage=True, timebin='20s',timespan='scan')
            self.assertTrue((ParallelDataHelper.isParallelMS(self.outputms),'Output should be an MMS'))
        
        except Exception, instance:
            print 'This error should have not happened %s'%instance
        
    def test_combspws_timespan_spw_axis_error(self):
        '''mstransform: combinespws=True, timespan=scan axis=spw'''
        self.createMMS(self.vis, axis='spw',scans='30',spws='10,11')
        self.outputms = "spwaxiserror.mms"
        # subMSs do not have all spws. Create an MS
        mstransform(vis=self.testmms, outputvis=self.outputms, datacolumn='data',
                    combinespws=True, timeaverage=True, timebin='20s',timespan='scan')
        self.assertFalse(ParallelDataHelper.isParallelMS(self.outputms),'Output should be an MS')
        mymsmd = msmdtool()
        mymsmd.open(self.outputms)
        nspw = mymsmd.nspw()
        mymsmd.close()
        self.assertEqual(nspw,1)
        
    def test_combspws_timespan_scan_axis(self):
        '''mstransform: combinespws=True, timespan=scan axis=scan'''
        self.createMMS(self.vis, axis='scan',spws='0')
        self.outputms = "scanaxiserror.mms"
        # subMSs do not have all scans. Create an MS.
        try:
            mstransform(vis=self.testmms, outputvis=self.outputms, datacolumn='data',
                        combinespws=True, timeaverage=True, timebin='20s',timespan='scan')
            self.assertTrue((ParallelDataHelper.isParallelMS(self.outputms),'Output should be an MMS'))
        
        except Exception, instance:
            print 'Expected error: %s'%instance

    @unittest.skip('Skip until CAS-6946 is fixed')
    def test_split_MMS_weight_corr_sel(self):
        '''mstransform: Split MMS in parallel. Check WEIGHT shape when selecting correlation'''
        # Create an MMS in the setup. It creates self.testmms
        self.createMMS(self.vis, axis='scan', spws='0,1')
        
        self.outputms = 'corrRR_LL.mms'
        mstransform(vis=self.testmms, outputvis=self.outputms, datacolumn='data', correlation='RR,LL',spw='0')
        
        self.assertTrue(ParallelTaskHelper.isParallelMS(self.outputms),'Output is not an MMS')
        
        mslocal = mstool()
        mslocal.open(self.outputms)
        sublist = mslocal.getreferencedtables()
        self.assertEqual(len(sublist), 2)
        
        # Test DD table
        msmdt = msmdtool()
        msmdt.open(self.outputms)
        out_dds = msmdt.datadescids()
        msmdt.done()
        
        ref = [0]
        for i in out_dds:
            self.assertEqual(out_dds[i], ref[i])

        # The separation axis should be copied to the output MMS
        in_sepaxis = ph.axisType(self.testmms)
        out_sepaxis = ph.axisType(self.outputms)
        self.assertEqual(in_sepaxis, out_sepaxis, 'AxisTypes from input and output MMS do not match')

        # Check the dimensions of the WEIGHT and SIGMA columns. CAS-6946
        out_ws = th.getColShape(self.outputms,'WEIGHT')
        out_ss = th.getColShape(self.outputms,'SIGMA')
        self.assertEqual(out_ws[0],'[2]','WEIGHT shape is not correct')
        self.assertEqual(out_ss[0],'[2]','SIGMA shape is not correct')


class test_mms_output(test_base):
    '''Tests for outputMMS and transformations'''
    
    def setUp(self):
        self.setUp_4ants()

    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outputms)
    
    def test_output_mms1(self):
        '''mstransform: combinespws=True, output axis=auto. Expect error.'''
        self.outputms = 'outmms1.mms'
        try:
            mstransform(self.vis, outputvis=self.outputms, datacolumn='corrected', createmms=True, combinespws=True, spw='12,13', scan='31')
        except Exception, instance:
            print 'Expected error: %s'%instance

    def test_output_mms2(self):
        '''mstransform: combinespws=True, output axis=spw. Expect error.'''
        self.outputms = 'outmms2.mms'
        try:
            mstransform(self.vis, outputvis=self.outputms, datacolumn='corrected', createmms=True, combinespws=True, spw='12,13',
                        separationaxis='scan')
        except Exception, instance:
            print 'Expected error: %s'%instance

    def test_output_mms3(self):
        '''mstransform: timeaverage=True, timespan=scan, output axis=auto.'''
        self.outputms = 'outmms3.mms'
        # Just give a WARNING
        mstransform(self.vis, outputvis=self.outputms, datacolumn='data', createmms=True, timeaverage=True, spw='1,3',
                        separationaxis='auto', timebin='100s',timespan='scan')
        self.assertTrue(ParallelDataHelper.isParallelMS(self.outputms),'Output should be an MMS')

    def test_output_mms4(self):
        '''mstransform: timeaverage=True, output axis=scan, timespan=scan'''
        self.outputms = 'outmms4.mms'
        # Just give a WARNING
        mstransform(self.vis, outputvis=self.outputms, datacolumn='corrected', createmms=True, timeaverage=True, spw='12,13',
                    separationaxis='scan',timebin='10s',timespan='scan')
        self.assertTrue(ParallelDataHelper.isParallelMS(self.outputms),'Output should be an MMS')


class test_vla_mixed_polarizations_mms(test_base):
    '''Test behaviour of mstransform in split mode when the input MMS contains mixed VLA correlations XY/LR'''
    
    def setUp(self):
                
        self.setUp_CAS_6733()
        
    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outputms)
    
    def test_vla_mixed_polarizations_mms1(self):
        
        self.outputms = 'test_vla_mixed_polarizations_1.mms'
        
        # scan=14 should have only circular polarizations in pol id 1
        mstransform(vis=self.vis,outputvis=self.outputms,scan='16',datacolumn='DATA', createmms=True,
                    separationaxis='spw',spw='16~18')
        
        # Check that DDI sub-table is consistent with POLARIZATION sub-table
        mytb = tbtool()
        mytb.open(self.outputms + '/POLARIZATION')
        npols = mytb.nrows()
        mytb.close()
        
        mytb = tbtool()
        mytb.open(self.outputms + '/DATA_DESCRIPTION')
        polIds = mytb.getcol('POLARIZATION_ID')
        mytb.close()    
        for id in polIds:
            self.assertTrue(id in range(npols),'PolarizationId in DATA_DESCRIPTION not consistent with POLARIZATION table')
         
#        self.assertTrue(all(polIds < npols), 'PolarizationId in DATA_DESCRIPTION not consistent with POLARIZATION table') 
        
        # Check that flagdata can run properly with output MS
        summary = flagdata(vis=self.outputms,mode='summary')
        self.assertTrue(summary.has_key('correlation'), 'Flagdata failure due to missformated MS') 

    def test_vla_mixed_polarizations_mms2(self):
        
        self.outputms = 'test_vla_mixed_polarizations_2.mms'
        
        mstransform(vis=self.vis,outputvis=self.outputms,scan='16',datacolumn='DATA', createmms=True,
                    separationaxis='spw',spw='16~18',correlation='XX')
        
        # Check that DDI sub-table is consistent with POLARIZATION sub-table
        mytb = tbtool()
        mytb.open(self.outputms + '/POLARIZATION')
        npols = mytb.nrows()
        mytb.close()
        
        mytb = tbtool()
        mytb.open(self.outputms + '/DATA_DESCRIPTION')
        polIds = mytb.getcol('POLARIZATION_ID')
        mytb.close()    
        for id in polIds:
            self.assertTrue(id in range(npols),'PolarizationId in DATA_DESCRIPTION not consistent with POLARIZATION table')
        
#        self.assertTrue(all(polIds < npols), 'PolarizationId in DATA_DESCRIPTION not consistent with POLARIZATION table') 
        
        # Check that flagdata can run properly with output MS
        summary = flagdata(vis=self.outputms,mode='summary')
        self.assertTrue(summary.has_key('correlation'), 'Flagdata failure due to missformated MS') 
        
        
class test_alma_wvr_correlation_products_mms(test_base):
    '''Test behaviour of mstransform in split mode when the input MS contains ALMA WVR correlation products'''
    
    def setUp(self):
                
        self.setUp_CAS_6941()
        
    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf '+ self.outputms)
    
    def test_alma_wvr_correlation_products_mms1(self):
        
        self.outputms = 'test_alma_wvr_correlation_products_1.mms'
        # Only spw=2 exist in MS
        mstransform(vis=self.vis,outputvis=self.outputms,spw='0,1,2',datacolumn='DATA',createmms=True)
        
        # Check that POLARIZATION sub-table is properly sorted
        mytb = tbtool()
        mytb.open(self.outputms + '/POLARIZATION')
        numCorr = mytb.getcol('NUM_CORR')
        mytb.close()    
        
        self.assertEqual(numCorr[0],2,'POLARIZATION table miss-sorted')         
        self.assertEqual(numCorr[1],1, 'POLARIZATION table miss-sorted')         
        
        # Check that flagdata can run properly with output MS
        summary = flagdata(vis=self.outputms,mode='summary')
        self.assertTrue(summary.has_key('correlation'), 'Flagdata failure due to missformated MS')   
        
              

# Cleanup class
class Cleanup(test_base):

    def tearDown(self):
        os.system('rm -rf ngc5921.*ms* jupiter6cm.demo*')
        os.system('rm -rf Four_ants_3C286.*ms* g19_d2usb_targets*')
        os.system('rm -rf comb*.*ms* reg*.*ms hann*.*ms favg*.*ms')
        os.system('rm -rf split*.*ms')
        os.system('rm -rf 3c84scan1*ms* test.mms')

    def test_runTest(self):
        '''mstransform: Cleanup'''
        pass


def suite():
    return [
            test_mms_transformations,
            test_mms_freqavg,
            test_mms_parallel,
            test_mms_spw_poln,
            test_mms_input,
            test_mms_output,
            test_vla_mixed_polarizations_mms,
            test_alma_wvr_correlation_products_mms,
            Cleanup]
