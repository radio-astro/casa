import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest

class clean_test(unittest.TestCase):
    
#    msfile = 'at166B.ms'
    msfile = 'Itziar.ms'

#    def shortDescription(self):
#        return "Unit tests of task clean"

    def setUp(self):
        res = None
        default(clean)
        shutil.copytree(os.environ.get('CASAPATH').split()[0] +\
                            '/data/regression/exportasdm/input/'+self.msfile, self.msfile)
#        shutil.copytree('/Users/scastro/casadir/utests/clean/data/'+self.msfile, self.msfile)
        
        
    
    def tearDown(self):
        os.system('rm -rf ' + self.msfile)
        
    def test1(self):
        """Test 1: Wrong input should return False"""
        msfile = 'badfilename'
        self.res = clean(vis=msfile)
        self.assertFalse(self.res)
        
    def test2(self):
        """Test 2: Good input should return None"""
        output = 'cleantest2.im'
        self.res = clean(vis=self.msfile,imagename=output)
        self.assertEqual(self.res,None)
        
    def test3(self):
        """Test 3: Check if output exists"""
        output = 'cleantest3.im'
        self.res = clean(vis=self.msfile,imagename=output)
        self.assertTrue(os.path.exists(output+'.image'))
        
    def test4(self):
        """Test 4: Wrong field type"""
        output = 'cleantest4.im'
        self.res = clean(vis=self.msfile,imagename=output,field=0)
        self.assertFalse(self.res)
        
    def test5(self):
        """Test 5: Wrong spw value"""
        output = 'cleantest5.im'
        self.res = clean(vis=self.msfile,imagename=output,spw=10)
        self.assertFalse(os.path.exists(output+'.image'))

        
    def test6(self):
        """Test 6: Empty mode value"""
        output = 'cleantest6.im'
        self.res = clean(vis=self.msfile,imagename=output,mode='')
        self.assertFalse(self.res)
        
    def test7(self):
        """Test 7: Unsupported gridmode"""
        output = 'cleantest7.im'
        self.res = clean(vis=self.msfile,imagename=output,gridmode='grid')
        self.assertFalse(self.res)
        
    def test8(self):
        """Test 8: Test gridmode=widefield"""
        output = 'cleantest8.im'
        self.res = clean(vis=self.msfile,imagename=output,gridmode='widefield')
        self.assertEqual(self.res, None) 
        self.assertTrue(os.path.exists(output+'.image'))
 
    #FIXME: should this fail????
    def test9(self):
        """Test 9: Test gridmode=aprojection"""
        output = 'cleantest9.im'
        self.res = clean(vis=self.msfile,imagename=output,
                                               gridmode='aprojection')
        pass
                 
    def test10(self):
        """Test 10: Wrong niter type"""
        output = 'cleantest10.im'
        self.res = clean(vis=self.msfile,imagename=output,niter='1')
        self.assertFalse(self.res)
        
    def test11(self):
        """Test 11: Unsupported psfmode"""
        output = 'cleantest11.im'
        self.res = clean(vis=self.msfile,imagename=output,psfmode='psf')
        self.assertFalse(self.res)
        
    def test12(self):
        """Test 12: Test psfmode=clark"""
        output = 'cleantest12.im'
        self.res = clean(vis=self.msfile,imagename=output,psfmode='clark')
        self.assertEqual(self.res, None)
       
    def test13(self):
        """Test 13: Test psfmode=hogbom"""
        output = 'cleantest13.im'
        self.res = clean(vis=self.msfile,imagename=output,psfmode='hogbom')
        self.assertEqual(self.res, None)
       
    def test14(self):
        """Test 14: Test psfmode=clarkstokes"""
        output = 'cleantest14.im'
        self.res = clean(vis=self.msfile,imagename=output,psfmode='clarkstokes')
        self.assertEqual(self.res, None)

    def test15(self):
        """Test 15: Unsupported imagermode"""
        output = 'cleantest15.im'
        self.res = clean(vis=self.msfile,imagename=output,imagermode='clark')
        self.assertFalse(self.res)

    def test16(self):
        """Test 16: Test imagermode=csclean"""
        output = 'cleantest16.im'
        self.res = clean(vis=self.msfile,imagename=output,imagermode='csclean')
        self.assertEqual(self.res, None)

    def test17(self):
        """Test 17: Test imagermode=mosaic"""
        output = 'cleantest17.im'
        self.res = clean(vis=self.msfile,imagename=output,imagermode='mosaic')
        self.assertEqual(self.res, None)

    def test18(self):
        """Test 18: Zero value of imsize"""
        output = 'cleantest18.im'
        self.res = clean(vis=self.msfile,imagename=output,imsize=0)
        self.assertFalse(os.path.exists(output+'.image'))

    def test19(self):
        """Test 19: Correct cell values"""
        output = 'cleantest19.im'
        self.res = clean(vis=self.msfile,imagename=output, cell=2.0)
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(output+'.image'))
        
    def test20(self):
        """Test 20: Unsupported Stokes parameter"""
        output = 'cleantest20.im'
        self.res = clean(vis=self.msfile,imagename=output, stokes='V')
        self.assertFalse(self.res)
        
#    def test30(self):
#        """Test 30: X equal 0"""
#        x = 1
#        self.assertEqual(x,0)
        
    def test21(self):
        """Test 21: Correct Stokes parameter"""
        output = 'cleantest21.im'
        self.res = clean(vis=self.msfile,imagename=output, stokes='XX')
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(output+'.image'))
        
        



def suite():
    return [clean_test]
