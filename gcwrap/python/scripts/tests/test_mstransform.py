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
        self.vis = 'ngc4826.ms'
        if os.path.exists(self.vis):
           self.cleanup()
            
        os.system('cp -rL '+datapath + self.vis +' '+ self.vis)
        default(mstransform)
        
    def cleanup(self):
        os.system('rm -rf '+ self.vis)



# Add tests from test_cvel.py
# Look at tests in test_cvel-B.py
class test_cvel1(test_base):
    '''Tests without any transformation applied'''
    def setUp(self):
        self.setUp_cveltest()
        
    def test_cveltest3(self):
        '''mstransform: Do not apply any transformation. Default values.'''
        
        outfile = 'cveltest3.ms'
        rval = mstransform(vis=self.vis, outputvis=outfile, datacolumn='data')
        self.assertNotEqual(rval,False)
        ret = th.verify_ms(outfile, 1, 64, 0)
        self.assertTrue(ret[0],ret[1])
        
#class test_cvel2(test_base):
#    def setUp(self):
#        self.setUp_ngc4826()
#
#    def tearDown(self):
#        os.system('rm -rf '+ self.vis)
#        os.system('rm -rf cveltest*.ms')
#        
#    def test_cveltest4(self):
#        '''mstransform: Do not apply any transformation '''
#        outfile = 'cveltest4.ms'
#        rval = mstransform(vis=self.vis, outputvis=outfile, datacolumn='data')
#        self.assertNotEqual(rval,False)
#        ret = th.verify_ms(outfile, 1, 64, 0)
#        self.assertTrue(ret[0],ret[1])

                     
class test_combspw(test_base):
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

class test_regridms(test_base):
    '''Tests for regridms'''
       
    def setUp(self):
        self.setUp_data4tfcrop()
        
    def tearDown(self):
        os.system('rm -rf '+ self.vis)
        os.system('rm -rf reg*.ms')
        
    def test_regrid1(self):
        '''mstransform: Default of regridms parameters'''
        
        outputms = "reg1.ms"
        mstransform(vis=self.vis, outputvis=outputms, regridms=True)
        self.assertTrue(os.path.exists(outputms))
        
        # The output should be the same as the input
        for i in range(16):
            ret = th.verify_ms(outputms, 16, 64, i)
            self.assertTrue(ret[0],ret[1])
            
    def test_regrid2(self):
        '''mstransform: Default regridms with spw selection'''
        
        outputms = "reg2.ms"
        mstransform(vis=self.vis, outputvis=outputms, regridms=True, spw='1,3,5,7')
        self.assertTrue(os.path.exists(outputms))
        
        # The output should be the same as the input
        for i in range(4):
            ret = th.verify_ms(outputms, 4, 64, i)
            self.assertTrue(ret[0],ret[1])
            
    def test_regrid3(self):
        '''mstransform: Regrid MS'''
        self.setUp_ngc4826()
        outputms = "reg3.ms"
        mstransform(vis=self.vis, outputvis=outputms, regridms=True, spw='0',field = '1',
             nchan = 32, start = 10)



 
# Cleanup class 
class cleanup(test_base):
    
    def tearDown(self):
        os.system('rm -rf ngc5921.*ms*')
        os.system('rm -rf Four_ants_3C286.*ms*')
        os.system('rm -rf comb*.*ms*')

    def test_runTest(self):
        '''mstransform: Cleanup'''
        pass


def suite():
    return [test_combspw,
            test_cvel1,
            test_cvel2,
            test_regridms,
            cleanup]
