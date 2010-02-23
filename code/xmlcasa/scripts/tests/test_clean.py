import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest

'''
Unit tests for task clean. It tests the following parameters:
    vis:           wrong and correct values
    imagename:     if output exists
    field:         wrong field type; non-default value
    spw:           wrong value; non-default value
    mode:          empty value; non-default values
    gridmode:      unsupported value; non-default values
    niter:         wrong type; non-default values
    psfmode:       unsupported value; non-default values
    imagermode:    unsupported value; non-default values
    imsize:        zero value; non-default value
    stokes:        unsupported value; non-default value
    weighting:     unsupported value; non-default values
    selectdata:    True; subparameters:
                     timerange:    non-default value
                     antenna:      unsupported value; non-default value
    
    Other tests: check the value of a pixel.
'''
class clean_test(unittest.TestCase):
    
    # Input and output names
    msfile = 'ngc7538_ut.ms'
    res = None
    img = 'cleantest_im'

    def setUp(self):
        self.res = None
        default(clean)
        if (os.path.exists(self.msfile)):
            os.system('rm -rf ' + self.msfile)
            
        datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/clean/'
        shutil.copytree(datapath+self.msfile, self.msfile)
    
    def tearDown(self):
        if (os.path.exists(self.msfile)):
            os.system('rm -rf ' + self.msfile)

        os.system('rm -rf ' + self.img+'*')

    def getpixval(self,img,pixel):
        ia.open(img)
        px = ia.pixelvalue(pixel)
        ia.close()
        return px['value']['value']
        
    def test0(self):
        '''Test 0: Default values'''
        self.res = clean()
        self.assertFalse(self.res)
        
    def test1(self):
        """Test 1: Wrong input should return False"""
        msfile = 'badfilename'
        self.res = clean(vis=msfile, imagename=self.img)
        self.assertFalse(self.res)
        
    def test2(self):
        """Test 2: Good input should return None"""
        self.res = clean(vis=self.msfile,imagename=self.img)
        self.assertEqual(self.res,None)
        
    def test3(self):
        """Test 3: Check if output exists"""
        self.res = clean(vis=self.msfile,imagename=self.img)
        self.assertTrue(os.path.exists(self.img+'.image'))
        
    def test4(self):
        """Test 4: Wrong field type"""
        self.res = clean(vis=self.msfile,imagename=self.img,field=0)
        self.assertFalse(self.res)
        
    def test5(self):
        """Test 5: Non-default field value"""
        self.res = clean(vis=self.msfile,imagename=self.img,field='0~1')
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img+'.image'))           
        
    def test6(self):
        """Test 6: Wrong spw value"""
        self.res = clean(vis=self.msfile,imagename=self.img,spw='10')
        self.assertFalse(os.path.exists(self.img+'.image'))
       
    def test7(self):
        """Test 7: Non-default spw value"""
        self.res = clean(vis=self.msfile,imagename=self.img,spw='0')
        self.assertTrue(os.path.exists(self.img+'.image'))

    def test8(self):
        """Test 8: Empty mode value"""
        self.res = clean(vis=self.msfile,imagename=self.img,mode='')
        self.assertFalse(self.res)

    # FIXME: CHANGE SUBPARAMETERS FOR DIFFERENT MODES
    def test9(self):
        """Test 9: Non-default mode channel"""
        self.res = clean(vis=self.msfile,imagename=self.img,mode='channel')
        self.assertEqual(self.res,None)
        self.assertTrue(os.path.exists(self.img+'.image'),'Image %s does not exist'%self.img)

    def test10(self):
        """Test 10: Non-default mode velocity"""
        self.res = clean(vis=self.msfile,imagename=self.img,mode='velocity')
        self.assertEqual(self.res,None)
        self.assertTrue(os.path.exists(self.img+'.image'),'Image %s does not exist'%self.img)
        
    def test11(self):
        """Test 11: Non-default mode frequency"""
        self.res = clean(vis=self.msfile,imagename=self.img,mode='frequency')
        self.assertEqual(self.res,None)
        self.assertTrue(os.path.exists(self.img+'.image'),'Image %s does not exist'%self.img)
        
    def test10(self):
        """Test 10: Unsupported gridmode"""
        self.res = clean(vis=self.msfile,imagename=self.img,gridmode='grid')
        self.assertFalse(self.res)
        
    def test11(self):
        """Test 11: Non-default gridmode widefield"""
        self.res = clean(vis=self.msfile,imagename=self.img,gridmode='widefield',imsize=[20])
        self.assertEqual(self.res, None) 
        self.assertTrue(os.path.exists(self.img+'.image'),'Image %s does not exist'%self.img)

    # Takes too long!!!
#    def test11(self):
#        """Test 11: Non-default gridmode aprojection"""
#        self.res = clean(vis=self.msfile,imagename=self.img,gridmode='aprojection',imsize=[10],niter=1)
#        self.assertEqual(self.res, None) 
#        self.assertTrue(os.path.exists(self.img+'.image'),'Image %s does not exist'%self.img)
                     
    def test12(self):
        """Test 12: Wrong niter type"""
        self.res = clean(vis=self.msfile,imagename=self.img,niter='1')
        self.assertFalse(self.res)
        
    def test13(self):
        """Test 13: Non-default niter values"""
        for n in range(10,400,50):
            self.res = clean(vis=self.msfile,imagename=self.img,niter=n)
            self.assertEqual(self.res,None,'Failed for niter = %s' %n)
    
    def test14(self):
        """Test 14: Unsupported psfmode"""
        self.res = clean(vis=self.msfile,imagename=self.img,psfmode='psf')
        self.assertFalse(self.res)
        
    def test15(self):
        """Test 15: Non-default psfmode hogbom"""
        self.res = clean(vis=self.msfile,imagename=self.img,psfmode='hogbom')
        self.assertEqual(self.res, None)            
        self.assertTrue(os.path.exists(self.img+'.image'))
        
    def test16(self):
        """Test 16: Non-default psfmode clarkstokes"""
        self.res = clean(vis=self.msfile,imagename=self.img,psfmode='clarkstokes')
        self.assertEqual(self.res, None)            
        self.assertTrue(os.path.exists(self.img+'.image'))
      
    def test17(self):
        """Test 17: Unsupported imagermode"""
        self.res = clean(vis=self.msfile,imagename=self.img,imagermode='clark')
        self.assertFalse(self.res)      

    def test18(self):
        '''Test 18: Non-default imagermode csclean'''
        self.res = clean(vis=self.msfile,imagename=self.img,imagermode='csclean')
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img+'.image'))

    def test19(self):
        '''Test 19: Non-default imagermode mosaic'''
        self.res = clean(vis=self.msfile,imagename=self.img,imagermode='mosaic',imsize=[20])
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img+'.image'))

    def test20(self):
        """Test 20: Zero value of imsize"""
        self.res = clean(vis=self.msfile,imagename=self.img,imsize=0)
        self.assertFalse(os.path.exists(self.img+'.image'))

    def test21(self):
        '''Test 21: Non-default imsize values'''
        self.res = clean(vis=self.msfile,imagename=self.img,imsize=[80,80])
        self.assertEqual(self.res,None)
        self.assertTrue(os.path.exists(self.img+'.image'),'Image %s does not exist' %self.img)

    def test22(self):
        """Test 22: Non-default cell values"""
        self.res = clean(vis=self.msfile,imagename=self.img, cell=12.5)
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img+'.image'))
        
    def test23(self):
        """Test 23: Unsupported Stokes parameter"""
        self.res = clean(vis=self.msfile,imagename=self.img, stokes='V')
        self.assertFalse(self.res)
        
    def test24(self):
        """Test 24: Non-default Stokes parameter"""
        self.res = clean(vis=self.msfile,imagename=self.img, stokes='LL')
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img+'.image'))
        
    def test25(self):
        '''Test 25: Unsupported weighting mode'''
        self.res = clean(vis=self.msfile,imagename=self.img, weighting='median')
        self.assertFalse(self.res)
        
    def test24(self):
        '''Test 24: Non-default weighting uniform'''
        self.res = clean(vis=self.msfile,imagename=self.img, weighting='uniform')
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img+'.image'))

    def test24(self):
        '''Test 24: Non-default weighting briggs'''
        self.res = clean(vis=self.msfile,imagename=self.img, weighting='briggs')
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img+'.image'))

    def test24(self):
        '''Test 24: Non-default weighting radial'''
        self.res = clean(vis=self.msfile,imagename=self.img, weighting='radial')
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img+'.image'))
        
    def test25(self):
        '''Test 25: Non-default subparameters of selectdata'''
        self.res = clean(vis=self.msfile,imagename=self.img,selectdata=True,
                         timerange='>11:30:00',antenna='VA12')
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img+'.image'))

    def test26(self):
        '''Test 26: Wrong antenna subparameter of selectdata'''
        self.res = clean(vis=self.msfile,imagename=self.img,selectdata=True,
                         antenna='88')
        self.assertFalse(os.path.exists(self.img+'.image'))

    def test27(self):
        '''Test 27: Verify the value of pixel 50'''
        #run clean with some parameters
        self.res = clean(vis=self.msfile,imagename=self.img,selectdata=True,
                         timerange='>11:28:00',field='0~2',imsize=[100,100],niter=10)
        
        os.system('cp -r ' + self.img+'.image' + ' myimage.im')
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img+'.image'))
        ref = 0.007161217276006937
        value = self.getpixval(self.img+'.image',50)
        diff = abs(ref - value)
        self.assertTrue(diff < 10e-5)
        
    def test28(self):
        '''Test 28: Wrong type of phasecenter'''
        self.res=clean(vis=self.msfile,imagename=self.img,phasecenter=4.5)
        self.assertFalse(os.path.exists(self.img+'.image'))
        
    def test29(self):
        '''Test 29: Non-default value of phasecenter'''
        self.res=clean(vis=self.msfile,imagename=self.img,phasecenter=2)
        self.assertTrue(os.path.exists(self.img+'.image'))
        
    

def suite():
    return [clean_test,clean_temp]


