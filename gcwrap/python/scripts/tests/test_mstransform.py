import shutil
import unittest
import os
import filecmp
from tasks import *
from taskinit import *
from __main__ import default
import testhelper as th

    
    
# Define the root for the data files
datapath = os.environ.get('CASAPATH').split()[0] + "/data/regression/unittest/mstransform/"

# Base class which defines setUp functions
# for importing different data sets
class test_base(unittest.TestCase):

    def setUp_cveltest(self):
        # data set with spw=0, 64 channels in LSRK
        self.vis = "test.ms"

        if os.path.exists(self.vis):
           self.cleanup()
            
        os.system('cp -rL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

    def setUp_ngc5921(self):
        # data set with spw=0, 63 channels in LSRK
        self.vis = "ngc5921.ms"

        if os.path.exists(self.vis):
           self.cleanup()
            
        os.system('cp -rL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)


    def setUp_data4tfcrop(self):
        # data set with spw=0~15, 64 channels each in TOPO
        self.vis = "Four_ants_3C286.ms"

        if os.path.exists(self.vis):
           self.cleanup()
            
        os.system('cp -rL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)
        
    def setUp_ngc4826(self):
        # data set with spw=0, 64 channels in LSRK
        self.vis = 'ngc4826a.ms'
        if os.path.exists(self.vis):
           self.cleanup()
            
        os.system('cp -rL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

    def setUp_jupiter(self):
        # data with spw=0,1 1 channel each in TOPO, field=0~12, 
        self.vis = 'jupiter6cm.demo-thinned.ms'
        if os.path.exists(self.vis):
           self.cleanup()
            
        os.system('cp -rL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)
        
    def setUp_g19(self):
        # data with spw=0~23 128 channel each in LSRK, field=0,1
        self.vis = 'g19_d2usb_targets_line-shortened-thinned.ms'
        if os.path.exists(self.vis):
           self.cleanup()
            
        os.system('cp -rL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)
        
    def cleanup(self):
        os.system('rm -rf '+ self.vis)


class test_Split(test_base):
    '''Tests to compare with the split task'''
    def setUp(self):
        self.setUp_data4tfcrop()
        
        
    
# TODO: Look at tests in test_cvel-B.py too
class test_Cvel1(test_base):
    '''Tests to compare with the cvel task'''
    
    def setUp(self):
        self.setUp_cveltest()
        
    def test_cveltest3(self):
        '''mstransform: Do not apply any transformation. Default values.'''
        
        outfile = 'cveltest3.ms'
        rval = mstransform(vis=self.vis, outputvis=outfile, datacolumn='data')
        self.assertNotEqual(rval,False)
        ret = th.verify_ms(outfile, 1, 64, 0)
        self.assertTrue(ret[0],ret[1])
        
                     
class test_Combspw(test_base):
    ''' Tests for combinespws'''
    
    def setUp(self):
        self.setUp_data4tfcrop()
        
    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf combspw*.ms')
                
    def test_combspw1(self):
        '''mstransform: Combine four spws into one'''
        
        outputms = "combspw1.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, spw='0~3')
        self.assertTrue(os.path.exists(outputms))
        
        ret = th.verify_ms(outputms, 1, 256, 0)
        self.assertTrue(ret[0],ret[1])
        
    def test_combspw2(self):
        '''mstransform: Combine some channels of two spws'''
        
        outputms = "combspw2.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, spw='0:60~63,1:60~63')
        self.assertTrue(os.path.exists(outputms))
        
        # The spws contain gaps, therefore the number of channels is bigger
        ret = th.verify_ms(outputms, 1, 68, 0)
        self.assertTrue(ret[0],ret[1])
        
        # Compare with cvel results
        default(cvel)
        cvel(vis=self.vis, outputvis='combcvel1.ms', spw='0:60~63,1:60~63')
        ret = th.verify_ms('combcvel1.ms', 1, 68, 0)
        self.assertTrue(ret[0],ret[1])

    def test_combspw3(self):
        '''mstransform: Split MS with two spws, one field selected'''
        # cvel: test7
        self.setUp_jupiter()
        outputms = 'combspw3.ms'
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, spw='0,1',field = '12',
             datacolumn='DATA')
        
        self.assertTrue(os.path.exists(outputms))
        ret = th.verify_ms(outputms, 1, 2, 0)
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
            ret = th.verify_ms(outputms, 16, 64, i)
            self.assertTrue(ret[0],ret[1])
            
    def test_regrid1_2(self):
        '''mstransform: Default regridms with spw selection'''
        
        outputms = "reg12.ms"
        mstransform(vis=self.vis, outputvis=outputms, regridms=True, spw='1,3,5,7')
        self.assertTrue(os.path.exists(outputms))
        
        # The output should be the same as the input
        for i in range(4):
            ret = th.verify_ms(outputms, 4, 64, i)
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
        inpfreq = th.getChannels(self.vis, 0, range(10,32,1))
        ret = th.verify_ms(outputms, 1, 32, 0, inpfreq)
        self.assertTrue(ret[0],ret[1])        

    def test_regrid2_2(self):
        '''mstransform: Regrid one spw and change frame to BARY'''
        #cvel: test13
        outputms = "reg22.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=False, regridms=True, spw='0',
                    field = '1',mode='frequency', nchan=2, start='115GHz', width='3MHz', 
                    outframe='BARY', phasecenter=1, datacolumn='DATA')
        self.assertTrue(os.path.exists(outputms))
        
        ret = th.verify_ms(outputms, 1, 2, 0)
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
            
            ret = th.verify_ms(outputms, 1, 2, 0)
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
        
        ret = th.verify_ms(outputms, 1, 10, 0)
        self.assertTrue(ret[0],ret[1])      
        
        # Verify that output channels are correct
        inpfreq = th.getChannels(self.vis, 0, range(2,11,1))
        ret = th.verify_ms(outputms, 1, 10, 0, inpfreq)
        self.assertTrue(ret[0],ret[1])        


class test_Regridms3(test_base):
    '''Tests for regridms using Jupiter MS'''
       
    def setUp(self):
        self.setUp_jupiter()
        
    def tearDown(self):
        pass
        os.system('rm -rf '+ self.vis)
#        os.system('rm -rf reg*.ms')

    def test_regrid3_1(self):
        '''mstransform: Combine spw and regrid MS with two spws, select one field and 2 spws'''
        # cvel: test8
        outputms = "reg31a.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, regridms=True, 
                    spw='0,1',field = '11',nchan=1, width=2, datacolumn='DATA')
        self.assertTrue(os.path.exists(outputms))
        
        ret = th.verify_ms(outputms, 1, 1, 0)
        self.assertTrue(ret[0],ret[1])  
        
        # Now, do only the regridding and do not combine spws
        outputms = "reg31b.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=False, regridms=True, 
                    spw='0,1',field = '11',nchan=1, width=2, datacolumn='DATA')
        self.assertTrue(os.path.exists(outputms))
        
        ret = th.verify_ms(outputms, 2, 1, 0)
        self.assertTrue(ret[0],ret[1])  
        ret = th.verify_ms(outputms, 2, 1, 1)
        self.assertTrue(ret[0],ret[1])  
              

    def test_regrid3_2(self):
        '''mstransform: Regrid MS with two spws, select one field and 2 spws'''
        # cvel: test9
        outputms = "reg32.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, regridms=True, 
                    spw='0,1',field = '10',mode='channel', nchan=1, start=1, datacolumn='DATA')
        self.assertTrue(os.path.exists(outputms))
        
        ret = th.verify_ms(outputms, 1, 1, 0)
        self.assertTrue(ret[0],ret[1])        

    def test_regrid3_3(self):
        '''mstransform: Regrid MS with two spws, select one field and 2 spws, mode frequency'''
        # cvel: test10
        outputms = "reg33.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, regridms=True, spw='0,1',
                    field = '9',mode='frequency', nchan=1, start='4.8351GHz', width='50MHz', 
                    datacolumn='DATA')
        self.assertTrue(os.path.exists(outputms))
        
        ret = th.verify_ms(outputms, 1, 1, 0)
        self.assertTrue(ret[0],ret[1])        

    def test_regrid3_4(self):
        '''mstransform: Regrid MS, change frame, with two spws, select one field and 2 spws'''
        # cvel: test11
        outputms = "reg34.ms"
        mstransform(vis=self.vis, outputvis=outputms, regridms=True, combinespws=True, spw='0,1',
                    field = '10',mode='channel', nchan=1, start=1, outframe='lsrk', datacolumn='DATA')
        self.assertTrue(os.path.exists(outputms))
        
        ret = th.verify_ms(outputms, 1, 1, 0)
        self.assertTrue(ret[0],ret[1])        

    def test_regrid3_5(self):
        '''mstransform: Regrid MS, with two spws, select two fields and 2 spws, mode frequency'''
        # cvel: test12
        outputms = "reg35.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, regridms=True, spw='0,1',
                    field = '5,6',mode='frequency', nchan=2, start='4.8101 GHz', width='50 MHz', 
                    datacolumn='DATA')
        self.assertTrue(os.path.exists(outputms))
        
        ret = th.verify_ms(outputms, 1, 2, 0)
        self.assertTrue(ret[0],ret[1])        


    def test_regrid3_6(self):
        '''mstransform: Regrid MS, with two spws, select two fields and 2 spws, mode frequency'''
        # cvel: test15
        outputms = "reg36.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, regridms=True, spw='0,1',
                    field = '12',mode='frequency', nchan=1, start='4.850GHz', width='50MHz', 
                    outframe='', datacolumn='DATA')
        self.assertTrue(os.path.exists(outputms))
        
        ret = th.verify_ms(outputms, 1, 1, 0)
        self.assertTrue(ret[0],ret[1])        

class test_Regridms4(test_base):
    '''Tests for regridms using Jupiter MS'''
       
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
        
        # Compare input->output channel frequencies
        # TODO: verify the following checks. I think it is not
        # checking the frequencies correctly.
        inpfreq = th.getChannels(self.vis, 0, range(100,110,1))
        print inpfreq.size
        ret = th.verify_ms(outputms, 1, 10, 0, inpfreq)
        self.assertTrue(ret[0],ret[1])        

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
    return [test_Split,
            test_Combspw,
            test_Cvel1,
            test_Regridms1,
            test_Regridms2,
            test_Regridms3,
            test_Regridms4,
            Cleanup]
