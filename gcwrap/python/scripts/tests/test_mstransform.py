import shutil
import unittest
import os
import numpy
from tasks import *
from taskinit import mstool, tbtool
from __main__ import default
import testhelper as th
from recipes.listshapes import listshapes
from parallel.parallel_task_helper import ParallelTaskHelper


# Add a cluster-less mode to by-pass parallel processing for MMSs
#if os.environ.has_key('BYPASS_SEQUENTIAL_PROCESSING'):
#    ParallelTaskHelper.bypassParallelProcessing(1)
    
# Define the root for the data files
datapath = os.environ.get('CASAPATH').split()[0] + "/data/regression/unittest/mstransform/"

# Base class which defines setUp functions
# for importing different data sets
class test_base(unittest.TestCase):
    
    def setUp_ngc5921(self):
        # data set with spw=0, 63 channels in LSRK
        self.vis = "ngc5921.ms"

        if os.path.exists(self.vis):
           self.cleanup()
            
        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

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
        
    def cleanup(self):
        os.system('rm -rf '+ self.vis)


class test_Combspw1(test_base):
    ''' Tests for combinespws'''
    
    def setUp(self):
        self.setUp_4ants()
        
    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf combspw*.*ms')
                
    def test_combspw1_1(self):
        '''mstransform: Combine four spws into one'''
        
        outputms = "combspw11.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, spw='0~3')
        self.assertTrue(os.path.exists(outputms))
        
        ret = th.verifyMS(outputms, 1, 256, 0)
        self.assertTrue(ret[0],ret[1])
        
    def test_combspw1_2(self):
        '''mstransform: Combine some channels of two spws'''
        
        outputms = "combspw12.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, spw='0:60~63,1:60~63')
        self.assertTrue(os.path.exists(outputms))
        
        # The spws contain gaps, therefore the number of channels is bigger
        ret = th.verifyMS(outputms, 1, 68, 0)
        self.assertTrue(ret[0],ret[1])
        
        # Compare with cvel results
        default(cvel)
        cvel(vis=self.vis, outputvis='combcvel1.ms', spw='0:60~63,1:60~63')
        ret = th.verifyMS('combcvel1.ms', 1, 68, 0)
        self.assertTrue(ret[0],ret[1])

    def test_combspw1_3(self):
        '''mstransform: Do not combine spws and create MMS with axis scan.'''
        self.setUp_jupiter()
        outputms = 'combspw13.mms'
        mstransform(vis=self.vis, outputvis=outputms, combinespws=False, spw='0,1',field = '12',
             datacolumn='DATA', createmms=True, separationaxis='scan')
        
        self.assertTrue(os.path.exists(outputms))

        # Should create 6 subMSs
        mslocal = mstool()
        mslocal.open(thems=outputms)
        sublist = mslocal.getreferencedtables()
        mslocal.close()
        self.assertEqual(sublist.__len__(), 6, 'Should have created 6 subMSs')
        
        ret = th.verifyMS(outputms, 2, 1, 0)
        self.assertTrue(ret[0],ret[1])
        

class test_Regridms1(test_base):
    '''Tests for regridms using Four_ants_3C286.ms'''
       
    def setUp(self):
        self.setUp_4ants()
        
    def tearDown(self):
        pass
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf reg*.*ms')
        
    def test_regrid1_1(self):
        '''mstransform: Default of regridms parameters'''
        
        outputms = "reg11.ms"
        mstransform(vis=self.vis, outputvis=outputms, regridms=True)
        self.assertTrue(os.path.exists(outputms))
        
        # The output should be the same as the input
        for i in range(16):
            ret = th.verifyMS(outputms, 16, 64, i)
            self.assertTrue(ret[0],ret[1])
            
    def test_regrid1_2(self):
        '''mstransform: Default regridms with spw selection'''
        
        outputms = "reg12.ms"
        mstransform(vis=self.vis, outputvis=outputms, regridms=True, spw='1,3,5,7')
        self.assertTrue(os.path.exists(outputms))
        
        # The output should be the same as the input
        for i in range(4):
            ret = th.verifyMS(outputms, 4, 64, i)
            self.assertTrue(ret[0],ret[1])


class test_Regridms3(test_base):
    '''Tests for regridms using Jupiter MS'''
       
    def setUp(self):
        self.setUp_jupiter()
        
    def tearDown(self):
        pass
        os.system('rm -rf '+ self.vis)
#        os.system('rm -rf reg*.ms')

# Uncomment after seg fault is fixed
#    def test_regrid3_1(self):
#        '''mstransform: Combine spw and regrid MS with two spws, select one field and 2 spws'''
#        # cvel: test8
#        outputms = "reg31a.ms"
#        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, regridms=True, 
#                    spw='0,1',field = '11',nchan=1, width=2, datacolumn='DATA')
#        self.assertTrue(os.path.exists(outputms))
#        
#        ret = th.verifyMS(outputms, 1, 1, 0)
#        self.assertTrue(ret[0],ret[1])  
#        
#        # Now, do only the regridding and do not combine spws
#        outputms = "reg31b.ms"
#        mstransform(vis=self.vis, outputvis=outputms, combinespws=False, regridms=True, 
#                    spw='0,1',field = '11',nchan=1, width=2, datacolumn='DATA')
#        self.assertTrue(os.path.exists(outputms))
#        
#        ret = th.verifyMS(outputms, 2, 1, 0)
#        self.assertTrue(ret[0],ret[1])  
#        ret = th.verifyMS(outputms, 2, 1, 1)
#        self.assertTrue(ret[0],ret[1])  


class test_Hanning(test_base):
    
    def setUp(self):
        self.setUp_ngc5921()
        
    def tearDown(self):
        pass
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf hann*.*ms')

    def test_hanning1(self):
        '''mstransform: Apply Hanning smoothing in MS with 24 spws. Do not combine spws.'''
        self.setUp_g19()
        outputms = "hann1.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=False, hanning=True,
                    datacolumn='data')
        
        self.assertTrue(os.path.exists(outputms))
        ret = th.verifyMS(outputms, 24, 128, 0)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 24, 128, 2)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 24, 128, 15)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 24, 128, 18)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 24, 128, 23)
        self.assertTrue(ret[0],ret[1])        

    def test_hanning2(self):
        '''mstransform: Apply Hanning smoothing and combine spw=1,2,3.'''
        self.setUp_g19()
        outputms = "hann2.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, hanning=True,
                    spw='1,2,3', datacolumn='data')
        
        self.assertTrue(os.path.exists(outputms))
        ret = th.verifyMS(outputms, 1, 1448, 0)
        self.assertTrue(ret[0],ret[1])        
            
    def test_hanning3(self):
        '''mstransform: Hanning theoretical and calculated values should be the same'''
        # hanning: test4
        outputms = "hann3.ms"
        
        # The hanningsmooth task flags the first and last channels. Check it!
        # Before running the task
        flag_col = th.getVarCol(self.vis, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [False])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [False])
        
        # Get the DATA column before the transformation
        data_col = th.getVarCol(self.vis, 'DATA')
        
        mstransform(vis=self.vis, outputvis=outputms, datacolumn='data', hanning=True)

        # After running the task
        flag_col = th.getVarCol(outputms, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [True])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [True])

        corr_col = th.getVarCol(outputms, 'DATA')
        nrows = len(corr_col)
        
      # Loop over every 2nd row,pol and get the data for each channel
        max = 1e-05
        for i in range(1,nrows,2) :
            row = 'r%s'%i            
            # polarization is 0-1
            for pol in range(0,2) :
                # array's channels is 0-63
                for chan in range(1,62) :
                    # channels must start from second and end before the last
                    data = data_col[row][pol][chan]
                    dataB = data_col[row][pol][chan-1]
                    dataA = data_col[row][pol][chan+1]
        
                    Smoothed = th.calculateHanning(dataB,data,dataA)
                    CorData = corr_col[row][pol][chan]
                    
                    # Check the difference
                    self.assertTrue(abs(CorData-Smoothed) < max )

    def test_hanning4(self):
        '''mstransform: Flagging should be correct after hanning smoothing and frame transformation.'''
        # hanning: test8
#        clearcal(vis=self.vis)
        outputms = "hann4.ms"
        
      # check correct flagging (just for one row as a sample)
        flag_col = th.getVarCol(self.vis, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [False])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [False])
        self.assertTrue(flag_col['r1'][0][62] == [False])

        mstransform(vis=self.vis, outputvis=outputms, hanning=True, regridms=True,
                        outframe='cmb',datacolumn='data')

      # check correct flagging (just for one row as a sample)
        flag_col = th.getVarCol(outputms, 'FLAG')
        self.assertTrue(flag_col['r1'][0][0] == [True])
        self.assertTrue(flag_col['r1'][0][1] == [False])
        self.assertTrue(flag_col['r1'][0][2] == [False])
        self.assertTrue(flag_col['r1'][0][60] == [False])
        self.assertTrue(flag_col['r1'][0][61] == [True])
        self.assertTrue(flag_col['r1'][0][62] == [True])

class test_FreqAvg(test_base):
    
    def setUp(self):
        self.setUp_g19()
        
    def tearDown(self):
        pass
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf favg*.*ms')

    def test_freqavg1(self):
        '''mstranform: Average 20 channels of one spw'''
        outputms = "favg1.ms"
        mstransform(vis=self.vis, outputvis=outputms, spw='2', freqaverage=True, freqbin=20)
                            
        self.assertTrue(os.path.exists(outputms))
        ret = th.verifyMS(outputms, 1, 7, 0)
        self.assertTrue(ret[0],ret[1])        
        
    def test_freqavg2(self):
        '''mstranform: Select a few channels to average from one spw'''
        outputms = "favg2.ms"
        mstransform(vis=self.vis, outputvis=outputms, spw='2:10~20', freqaverage=True, freqbin=2)
                            
        self.assertTrue(os.path.exists(outputms))
        ret = th.verifyMS(outputms, 1, 6, 0)
        self.assertTrue(ret[0],ret[1])        
        
    def test_freqavg3(self):
        '''mstranform: Average all channels of one spw'''
        outputms = "favg3.ms"
        mstransform(vis=self.vis, outputvis=outputms, spw='23', freqaverage=True, freqbin=128)
                            
        self.assertTrue(os.path.exists(outputms))
        ret = th.verifyMS(outputms, 1, 1, 0)
        self.assertTrue(ret[0],ret[1])        

class test_Shape(test_base):
    '''Test the tileshape parameter'''
    def setUp(self):
        self.setUp_4ants()
        
    def test_shape1(self):
        '''mstransform: default tileshape'''
        outputms = "shape1.ms"
        mstransform(vis=self.vis, outputvis=outputms, createmms=False, tileshape=[0])
                            
        self.assertTrue(os.path.exists(outputms))
        
        # Get the tile shape in input
        tblocal = tbtool()
        tblocal.open(self.vis)
        inpdm = tblocal.getdminfo()
        tblocal.close()
        inptsh = th.getTileShape(inpdm)
        
        # Get the tile shape for the output
        tblocal.open(outputms)
        outdm = tblocal.getdminfo()
        tblocal.close()
        outtsh = th.getTileShape(outdm)
        
        # Compare both
        self.assertTrue((inptsh==outtsh).all(), 'Tile shapes are different')
                

    def test_shape2(self):
        '''mstransform: custom tileshape'''
        outputms = "shape2.ms"
        inptsh = [4,20,1024]
        mstransform(vis=self.vis, outputvis=outputms, createmms=False, tileshape=inptsh)
                            
        self.assertTrue(os.path.exists(outputms))
                
        # Check the tile shape for the output
        tblocal = tbtool()
        tblocal.open(outputms)
        outdm = tblocal.getdminfo()
        tblocal.close()
        outtsh = th.getTileShape(outdm)
        
        self.assertTrue((inptsh==outtsh).all(), 'Tile shapes are different')
        

class test_MMS(test_base):
    def setUp(self):
        self.setUp_4ants()
        
    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf testmms*.*ms')
    
    def test_mms1(self):
        '''mstransform: create MMS with spw separation and channel selections'''
        outputms = "testmms1.mms"
        mstransform(vis=self.vis, outputvis=outputms, spw='0~4,5:1~10',createmms=True,
                    separationaxis='spw')
                            
        self.assertTrue(os.path.exists(outputms))
        
        # It should create 6 subMS, with spw=0~5
        # spw=5 should have only 10 channels
        ret = th.verifyMS(outputms, 6, 10, 5,ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        
               
        
    def test_mms2(self):
        '''mstransform: create MMS with spw/scan separation and channel selections'''
        outputms = "testmms2.mms"
        mstransform(vis=self.vis, outputvis=outputms, spw='0:0~10,1:60~63',createmms=True,
                    separationaxis='both')
                            
        self.assertTrue(os.path.exists(outputms))
        
        # It should create 4 subMS, with spw=0~1
        # spw=0 has 11 channels, spw=1 has 4 channels
        ret = th.verifyMS(outputms, 2, 11, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 2, 4, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        

    def test_mms3(self):
        '''mstransform: create MMS with scan separation and channel selections'''
        outputms = "testmms3.mms"
        mstransform(vis=self.vis, outputvis=outputms, spw='0:0~10,1:60~63',createmms=True,
                    separationaxis='scan')                            
        self.assertTrue(os.path.exists(outputms))
        
        # It should create 2 subMS, with spw=0~1
        # spw=0 has 11 channels, spw=1 has 4 channels
        ret = th.verifyMS(outputms, 2, 11, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 2, 4, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])            
        
    def test_mms4(self):
        '''mstransform: verify spw sub-table consolidation'''
        outputms = "testmms4.mms"
        mstransform(vis=self.vis, outputvis=outputms, spw='3,5:10~20,7,9,11,13,15',createmms=True,
                    separationaxis='spw')                            
        self.assertTrue(os.path.exists(outputms))
        
        # spw=5 should be spw=1 after consolidation, with 10 channels
        ret = th.verifyMS(outputms, 7, 10, 1, ignoreflags=True)
            
class test_Parallel(test_base):
    '''Run some of the same tests in parallel'''
    def setUp(self):
        self.setUp_4ants()
        
    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf parallel*.*ms')
    
    def test_parallel1(self):
        '''mstransform: create MMS with spw separation and channel selections in parallel'''
        outputms = "parallel1.mms"
        mstransform(vis=self.vis, outputvis=outputms, spw='0~4,5:1~10',createmms=True,
                    separationaxis='spw', parallel=True)
                            
        self.assertTrue(os.path.exists(outputms))
        
        # It should create 6 subMS, with spw=0~5
        # spw=5 should have only 10 channels
        ret = th.verifyMS(outputms, 6, 10, 5,ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        
                       
    def test_parallel2(self):
        '''mstransform: create MMS with spw/scan separation and channel selections in parallel'''
        outputms = "parallel2.mms"
        mstransform(vis=self.vis, outputvis=outputms, spw='0:0~10,1:60~63',createmms=True,
                    separationaxis='both', parallel=True)
                            
        self.assertTrue(os.path.exists(outputms))
        
        # It should create 4 subMS, with spw=0~1
        # spw=0 has 11 channels, spw=1 has 4 channels
        ret = th.verifyMS(outputms, 2, 11, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 2, 4, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        

    def test_parallel3(self):
        '''mstransform: create MMS with scan separation and channel selections in parallel'''
        outputms = "parallel3.mms"
        mstransform(vis=self.vis, outputvis=outputms, spw='0:0~10,1:60~63',createmms=True,
                    separationaxis='scan', parallel=True)                            
        self.assertTrue(os.path.exists(outputms))
        
        # It should create 2 subMS, with spw=0~1
        # spw=0 has 11 channels, spw=1 has 4 channels
        ret = th.verifyMS(outputms, 2, 11, 0, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])        
        ret = th.verifyMS(outputms, 2, 4, 1, ignoreflags=True)
        self.assertTrue(ret[0],ret[1])            
        
    def test_parallel4(self):
        '''mstransform: verify spw sub-table consolidation in parallel'''
        outputms = "parallel4.mms"
        mstransform(vis=self.vis, outputvis=outputms, spw='3,5:10~20,7,9,11,13,15',createmms=True,
                    separationaxis='spw', parallel=True)                            
        self.assertTrue(os.path.exists(outputms))
        
        # spw=5 should be spw=1 after consolidation, with 10 channels
        ret = th.verifyMS(outputms, 7, 10, 1, ignoreflags=True)

    def test_parallel5(self):
        '''mstransform: Do not combine spws and create MMS with axis scan in parallel.'''
        self.setUp_jupiter()
        outputms = 'parallel5.mms'
        mstransform(vis=self.vis, outputvis=outputms, combinespws=False, spw='0,1',field = '12',
             datacolumn='DATA', createmms=True, separationaxis='scan', parallel=True)
        
        self.assertTrue(os.path.exists(outputms))

        # Should create 6 subMSs
        mslocal = mstool()
        mslocal.open(thems=outputms)
        sublist = mslocal.getreferencedtables()
        mslocal.close()
        self.assertEqual(sublist.__len__(), 6, 'Should have created 6 subMSs')
        
        ret = th.verifyMS(outputms, 2, 1, 0)
        self.assertTrue(ret[0],ret[1])
 
# Cleanup class 
class Cleanup(test_base):
    
    def tearDown(self):
        os.system('rm -rf ngc5921.*ms*')
        os.system('rm -rf Four_ants_3C286.*ms*')
        os.system('rm -rf comb*.*ms* reg*.*ms hann*.*ms favg*.*ms')
        os.system('rm -rf testmms*.*ms parallel*.*ms')

    def test_runTest(self):
        '''mstransform: Cleanup'''
        pass


def suite():
    return [
            test_Combspw1,
            test_Regridms1,
#            test_Regridms3,
            test_Hanning,
            test_FreqAvg,
            test_Shape,
            test_MMS,
            test_Parallel,
            Cleanup]
