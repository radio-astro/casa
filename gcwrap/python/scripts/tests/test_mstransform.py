import shutil
import unittest
import os
import numpy
from tasks import *
from taskinit import *
from __main__ import default
import testhelper as th
from recipes.listshapes import listshapes

    
    
# Define the root for the data files
datapath = os.environ.get('CASAPATH').split()[0] + "/data/regression/unittest/mstransform/"

# Base class which defines setUp functions
# for importing different data sets
class test_base(unittest.TestCase):
    
    def setUp_ctb80(self):
        self.vis = 'ctb80-vsm.ms'
        fpath = datapath+ self.vis
        
        # Only link to this data set        
        if not os.path.lexists(self.vis):
#           self.cleanup()
#            os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
            os.symlink(fpath, self.vis)
            
        default(mstransform)

    def setUp_cveltest(self):
        # data set with spw=0, 64 channels in LSRK
        self.vis = "test.ms"

        if os.path.exists(self.vis):
           self.cleanup()
            
        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

    def setUp_ngc5921(self):
        # data set with spw=0, 63 channels in LSRK
        self.vis = "ngc5921.ms"

        if os.path.exists(self.vis):
           self.cleanup()
            
        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

    def setUp_data4tfcrop(self):
        # data set with spw=0~15, 64 channels each in TOPO
        self.vis = "Four_ants_3C286.ms"

        if os.path.exists(self.vis):
           self.cleanup()
            
        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)
        
    def setUp_ngc4826(self):
        # data set with spw=0, 64 channels in LSRK
        self.vis = 'ngc4826a.ms'
        if os.path.exists(self.vis):
           self.cleanup()
            
        os.system('cp -RL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

    def setUp_jupiter(self):
        # data with spw=0,1 1 channel each in TOPO, field=0~12, 
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


    
# TODO: Look at tests in test_cvel-B.py too                  
class test_Combspw1(test_base):
    ''' Tests for combinespws'''
    
    def setUp(self):
        self.setUp_data4tfcrop()
        
    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf combspw*.ms')
                
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
        '''mstransform: Split MS with two spws, one field selected'''
        # cvel: test7
        self.setUp_jupiter()
        outputms = 'combspw13.ms'
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, spw='0,1',field = '12',
             datacolumn='DATA')
        
        self.assertTrue(os.path.exists(outputms))
        ret = th.verifyMS(outputms, 1, 2, 0)
        self.assertTrue(ret[0],ret[1])

class test_Combspw2(test_base):
    ''' Tests for combinespws'''
    
    def setUp(self):
        self.setUp_g19()
        
    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf combspw*.ms')

    def test_combspw2_1(self):
        '''mstransform: Combine MS with two 24 spws and 128 channels each'''
        # cvel: test27
        outputms = 'combspw21.ms'
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, datacolumn='DATA')
        
        self.assertTrue(os.path.exists(outputms))
        ret = th.verifyMS(outputms, 1, 2440, 0)
        self.assertTrue(ret[0],ret[1])

    def test_combspw2_2(self):
        '''mstransform: Combine MS with two 24 spws and 128 channels each, channel_b mode'''
        # cvel: test30
        # channel_b does not force an equidistant grid
        outputms = 'combspw22.ms'
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, regridms=True, datacolumn='DATA',
                    mode='channel_b')
        
        self.assertTrue(os.path.exists(outputms))
        ret = th.verifyMS(outputms, 1, 2425, 0)
        self.assertTrue(ret[0],ret[1])
        

class test_Regridms1(test_base):
    '''Tests for regridms using Four_ants_3C286.ms'''
       
    def setUp(self):
        self.setUp_data4tfcrop()
        
    def tearDown(self):
        pass
        os.system('rm -rf '+ self.vis)
#        os.system('rm -rf reg*.ms')
        
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

class test_Regridms2(test_base):
    '''Tests for regridms using ngc4826.ms'''
       
    def setUp(self):
        self.setUp_ngc4826()        
        
    def tearDown(self):
        pass
        os.system('rm -rf '+ self.vis)
#        os.system('rm -rf reg*.ms')
            
    def test_regrid2_1(self):
        '''mstransform: Regrid MS with one field and one spw'''
        # cvel: test6
        outputms = "reg21.ms"
        mstransform(vis=self.vis, outputvis=outputms, regridms=True, spw='0',field = '1',
             nchan = 32, start = 10, datacolumn='DATA')
        self.assertTrue(os.path.exists(outputms))
        
        # Verify that output channels are correct
        inpfreq = th.getChannels(self.vis, 0, range(10,42,1))
        ret = th.verifyMS(outputms, 1, 32, 0, inpfreq)
        self.assertTrue(ret[0],ret[1])        

    def test_regrid2_2(self):
        '''mstransform: Regrid one spw and change frame to BARY'''
        #cvel: test13
        outputms = "reg22.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=False, regridms=True, spw='0',
                    field = '1',mode='frequency', nchan=2, start='115GHz', width='3MHz', 
                    outframe='BARY', phasecenter=1, datacolumn='DATA')
        self.assertTrue(os.path.exists(outputms))
        
        ret = th.verifyMS(outputms, 1, 2, 0)
        self.assertTrue(ret[0],ret[1])        

    def test_regrid2_3(self):
        '''mstransform: Regrid one spw and change frame to BARY, set non-existing center'''
        #cvel: test14
        outputms = "reg23.ms"
        try:
            mstransform(vis=self.vis, outputvis=outputms, combinespws=False, regridms=True, spw='0',
                        field = '1',mode='frequency', nchan=2, start='150GHz', width='3MHz', 
                        outframe='BARY', phasecenter=12, datacolumn='DATA')
            self.assertTrue(os.path.exists(outputms))
            
            ret = th.verifyMS(outputms, 1, 2, 0)
            self.assertTrue(ret[0],ret[1])        
        except:
            print "\n**** Expected Error! ****"

    def test_regrid2_4(self):
        '''mstransform: Regrid one spw, 2 fields and change frame to LSRD'''
        #cvel: test16
        outputms = "reg24.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=False, regridms=True, spw='0',
                    field = '2,3',mode='channel', nchan=10, start=2, phasecenter=2, 
                    outframe='lsrd', datacolumn='DATA')
        self.assertTrue(os.path.exists(outputms))
        
        ret = th.verifyMS(outputms, 1, 10, 0)
        self.assertTrue(ret[0],ret[1])      
        
        # Verify that output channels are correct
        inpfreq = th.getChannels(self.vis, 0, range(2,12,1))
        ret = th.verifyMS(outputms, 1, 10, 0, inpfreq)
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
              

    def test_regrid3_2(self):
        '''mstransform: Regrid MS with two spws, select one field and 2 spws'''
        # cvel: test9
        outputms = "reg32.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, regridms=True, 
                    spw='0,1',field = '10',mode='channel', nchan=1, start=1, datacolumn='DATA')
        self.assertTrue(os.path.exists(outputms))
        
        ret = th.verifyMS(outputms, 1, 1, 0)
        self.assertTrue(ret[0],ret[1])        

    def test_regrid3_3(self):
        '''mstransform: Regrid MS with two spws, select one field and 2 spws, mode frequency'''
        # cvel: test10
        outputms = "reg33.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, regridms=True, spw='0,1',
                    field = '9',mode='frequency', nchan=1, start='4.8351GHz', width='50MHz', 
                    datacolumn='DATA')
        self.assertTrue(os.path.exists(outputms))
        
        ret = th.verifyMS(outputms, 1, 1, 0)
        self.assertTrue(ret[0],ret[1])        

    def test_regrid3_4(self):
        '''mstransform: Regrid MS, change frame, with two spws, select one field and 2 spws'''
        # cvel: test11
        outputms = "reg34.ms"
        mstransform(vis=self.vis, outputvis=outputms, regridms=True, combinespws=True, spw='0,1',
                    field = '10',mode='channel', nchan=1, start=1, outframe='lsrk', datacolumn='DATA')
        self.assertTrue(os.path.exists(outputms))
        
        ret = th.verifyMS(outputms, 1, 1, 0)
        self.assertTrue(ret[0],ret[1])        

    def test_regrid3_5(self):
        '''mstransform: Regrid MS, with two spws, select two fields and 2 spws, mode frequency'''
        # cvel: test12
        outputms = "reg35.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, regridms=True, spw='0,1',
                    field = '5,6',mode='frequency', nchan=2, start='4.8101 GHz', width='50 MHz', 
                    datacolumn='DATA')
        self.assertTrue(os.path.exists(outputms))
        
        ret = th.verifyMS(outputms, 1, 2, 0)
        self.assertTrue(ret[0],ret[1])        


    def test_regrid3_6(self):
        '''mstransform: Regrid MS, with two spws, select two fields and 2 spws, mode frequency'''
        # cvel: test15
        outputms = "reg36.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, regridms=True, spw='0,1',
                    field = '12',mode='frequency', nchan=1, start='4.850GHz', width='50MHz', 
                    outframe='', datacolumn='DATA')
        self.assertTrue(os.path.exists(outputms))
        
        ret = th.verifyMS(outputms, 1, 1, 0)
        self.assertTrue(ret[0],ret[1])        

class test_Regridms4(test_base):
    '''Tests for regridms using G19 MS'''
       
    def setUp(self):
        self.setUp_g19()
        
    def tearDown(self):
        pass
        os.system('rm -rf '+ self.vis)
#        os.system('rm -rf reg*.ms')

    def test_regrid4_1(self):
        '''mstransform: Regrid MS with 24 spws to combine, channel mode, 10 output channels'''
        # cvel: test19
        outputms = "reg41.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, regridms=True,
                    mode='channel', nchan=10, start=100, width=2, 
                    phasecenter="J2000 18h25m56.09 -12d04m28.20", datacolumn='DATA')
        self.assertTrue(os.path.exists(outputms))
        
        # TODO: how to verify that the frequencies are correct?
        ret = th.verifyMS(outputms, 1, 10, 0)
        self.assertTrue(ret[0],ret[1])        

    def test_regrid4_2(self):
        '''mstransform: Regrid MS with 24 spws to combine, channel mode, 111 output channels'''
        # cvel: test20
        outputms = "reg42.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, regridms=True,
                    mode='channel', nchan=111, start=201, width=3, 
                    phasecenter="J2000 18h25m56.09 -12d04m28.20", datacolumn='DATA')
        self.assertTrue(os.path.exists(outputms))
        
        ret = th.verifyMS(outputms, 1, 111, 0)
        self.assertTrue(ret[0],ret[1])        

    def test_regrid4_3(self):
        '''mstransform: Regrid MS with 24 spws to combine, mode frequency, 111 output channels'''
        # cvel: test21
        outputms = "reg43.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, regridms=True,
                    mode='frequency', nchan=21, start='229587.0MHz', width='1600kHz', 
                    phasecenter="J2000 18h25m56.09 -12d04m28.20", datacolumn='DATA')
        self.assertTrue(os.path.exists(outputms))
        
        ret = th.verifyMS(outputms, 1, 21, 0)
        self.assertTrue(ret[0],ret[1])        

    def test_regrid4_4(self):
        '''mstransform: Regrid MS with 24 spws to combine, mode frequency, negative with, 210 output channels'''
        # cvel: test22
        outputms = "reg44.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, regridms=True,
                    mode='frequency', nchan=210, start='229588.0MHz', width='-2400kHz', 
                    phasecenter="J2000 18h25m56.09 -12d04m28.20", datacolumn='DATA')
        self.assertTrue(os.path.exists(outputms))
        
        ret = th.verifyMS(outputms, 1, 210, 0)
        self.assertTrue(ret[0],ret[1])        

    def test_regrid4_5(self):
        '''mstransform: Regrid and combine 24 spws in radio mode, create 30 output channels'''
        # cvel: test23
        outputms = "reg45.ms"
        vrad = (220398.676E6 - 229586E6)/220398.676E6 * 2.99792E8
        vwidth = ((220398.676E6 - 229586E6+1600E3)/220398.676E6 * 2.99792E8) - vrad
        vrad = vrad-vwidth/2.
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, regridms=True,
                    mode='velocity', nchan = 30, restfreq = '220398.676MHz', start = str(vrad)+'m/s',
                    width = str(vwidth)+'m/s', phasecenter = "J2000 18h25m56.09 -12d04m28.20",
                    veltype = 'radio', datacolumn='DATA')
        
        self.assertTrue(os.path.exists(outputms))
        ret = th.verifyMS(outputms, 1, 30, 0)
        self.assertTrue(ret[0],ret[1])        

        
    def test_regrid4_6(self):
        '''mstransform: Regrid and combine 24 spws in optical vel type, create 40 output channels'''
        # cvel: test25
        outputms = "reg46.ms"
        lambda0 = 2.99792E8/220398.676E6
        lambda1 = 2.99792E8/229586E6
        lambda2 = 2.99792E8/(229586E6+1600E3)
        vopt = (lambda1-lambda0)/lambda0 * 2.99792E8
        vwidth = vopt - (lambda2-lambda0)/lambda0 * 2.99792E8
        vopt = vopt-vwidth/2.
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, regridms=True, mode='velocity',
            nchan = 40, restfreq = '220398.676MHz', start = str(vopt)+'m/s', width = str(vwidth)+'m/s',
            phasecenter = "J2000 18h25m56.09 -12d04m28.20", veltype = 'optical', datacolumn='data')
        
        self.assertTrue(os.path.exists(outputms))
        ret = th.verifyMS(outputms, 1, 40, 0)
        self.assertTrue(ret[0],ret[1])        

    def test_regrid4_7(self):
        '''mstransform: Regrid 1 spw with default nchan'''
        # cvel: test43
        outputms = "reg47.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=False, regridms=True, mode='channel',
            spw='1', start=98, width=3, phasecenter="J2000 18h25m56.09 -12d04m28.20")
        
        self.assertTrue(os.path.exists(outputms))
        ret = th.verifyMS(outputms, 1, 10, 0)
        self.assertTrue(ret[0],ret[1])   
             
    def test_regrid4_8(self):
        '''mstransform: Regrid 2 spw2 with default nchan'''
        # cvel: test44
        outputms = "reg48.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, regridms=True, mode='channel',
            spw='1,15', start=198, width=3, phasecenter="J2000 18h25m56.09 -12d04m28.20")
        
        self.assertTrue(os.path.exists(outputms))
        ret = th.verifyMS(outputms, 1, 10, 0)
        self.assertTrue(ret[0],ret[1])   


        
class test_Regridms5(test_base):
    '''Tests to compare with the cvel task using test.ms'''
    
    def setUp(self):
        self.setUp_cveltest()
        
    def test_regrid5_1(self):
        '''mstransform: Test effect of sign of width parameter: channel mode, width positive'''
        #cvel: test35
        outputms = 'reg51.ms'
                
        mstransform(vis=self.vis, outputvis=outputms, combinespws=False, regridms=True,
                           spw='0', datacolumn='data', nchan=3, start=1, width=1)
        self.assertTrue(os.path.exists(outputms))

        # Get input channel frequencies to compare
        inpfreq = th.getChannels(self.vis, 0, range(1,4,1))
        
        # compare with output created channels
        ret = th.verifyMS(outputms, 1, 3, 0, inpfreq)
        self.assertTrue(ret[0],ret[1])
        
    def test_regrid5_2(self):
        '''mstransform: Test effect of sign of width parameter: channel mode, width negative'''
        #cvel: test36
        outputms = 'reg52.ms'
                
        mstransform(vis=self.vis, outputvis=outputms, combinespws=False, regridms=True,
                           spw='0', datacolumn='data', nchan=3, start=3, width=1)
        self.assertTrue(os.path.exists(outputms))

        # Get input channel frequencies to compare
        inpfreq = th.getChannels(self.vis, 0, range(1,4,1))
        
        # compare with output created channels
        ret = th.verifyMS(outputms, 1, 3, 0, inpfreq)
        self.assertTrue(ret[0],ret[1])

    # Wait until fftshift interpolation is implemented by Justo
    # TODO: add the other cvel tests with this interpolation
#    def test_regrid5_3(self):
#        '''mstransform: Test fftshift regridding: channel mode, width positive'''
#        #cvel: test48
#        outputms = 'reg53.ms'
#                
#        mstransform(vis=self.vis, outputvis=outputms, combinespws=False, regridms=True,
#                           nchan=3, start=1, width=1, interpolation='fftshift', datacolumn='data')
#        self.assertTrue(os.path.exists(outputms))
#
#        # Get input channel frequencies to compare
#        inpfreq = th.getChannels(self.vis, 0, range(1,4,1))
#        
#        # compare with output created channels
#        ret = th.verifyMS(outputms, 1, 3, 0, inpfreq)
#        self.assertTrue(ret[0],ret[1])


class test_Hanning(test_base):
    
    def setUp(self):
        self.setUp_g19()
        
    def tearDown(self):
        pass
        os.system('rm -rf '+ self.vis)
#        os.system('rm -rf reg*.ms')

    def test_hanning1(self):
        '''mstransform: Apply Hanning smoothing in MS with 24 spws. Do not combine spws.'''
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
        '''mstransform: Apply Hanning smoothing and select spw=1.'''
        # cvel: test33
        outputms = "hann2.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, hanning=True,
                    spw='1')
        
        self.assertTrue(os.path.exists(outputms))
        ret = th.verifyMS(outputms, 1, 128, 0)
        self.assertTrue(ret[0],ret[1])        

    def test_hanning3(self):
        '''mstransform: Apply Hanning smoothing and combine spw=1,2,3.'''
        outputms = "hann3.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, hanning=True,
                    spw='1,2,3', datacolumn='data')
        
        self.assertTrue(os.path.exists(outputms))
        ret = th.verifyMS(outputms, 1, 1448, 0)
        self.assertTrue(ret[0],ret[1])        
            
    def test_hanning4(self):
        '''mstransform: Apply Hanning smoothing  and combine 24 spws; change frame to BARY'''
        # cvel: test32
        outputms = "hann4.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, regridms=True, hanning=True,
                    mode='channel', datacolumn='data', outframe = "BARY",
                    phasecenter = "J2000 18h25m56.09 -12d04m28.20")
        
        self.assertTrue(os.path.exists(outputms))
        ret = th.verifyMS(outputms, 1, 2440, 0)
        self.assertTrue(ret[0],ret[1])        

    def test_hanning5(self):
        '''mstransform: Hanning theoretical and calculated values should be the same'''
        # hanning: test4
        self.setUp_ngc5921()
        outputms = "hann5.ms"
        
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


class test_FreqAvg(test_base):
    
    def setUp(self):
        self.setUp_g19()
        
    def tearDown(self):
        pass
        os.system('rm -rf '+ self.vis)
#        os.system('rm -rf reg*.ms')

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



# TODO: cleanup output MSs after test phase
# Cleanup class 
class Cleanup(test_base):
    
    def tearDown(self):
        os.system('rm -rf ngc5921.*ms*')
        os.system('rm -rf Four_ants_3C286.*ms*')
        os.system('rm -rf comb*.*ms*')

    def test_runTest(self):
        '''mstransform: Cleanup'''
        pass


def suite():
    return [
            test_Combspw1,
            test_Combspw2,
            test_Regridms1,
            test_Regridms2,
            test_Regridms3,
            test_Regridms4,
            test_Regridms5,
            test_Hanning,
            test_FreqAvg,
            Cleanup]
