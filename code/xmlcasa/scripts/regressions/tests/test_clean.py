import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest

class clean_test(unittest.TestCase):
    
    msfile = 'at166B.ms'

#    def shortDescription(self):
#        return "Unit tests of task clean"
    
    def setUp(self):
        results = None
        default(clean)
#        if(os.path.exists(self.msfile)):
#            pass
#        else:
#            shutil.copytree(os.environ.get('CASAPATH').split()[0] +\
#                            '/data/unittests/clean/'+self.msfile, self.msfile)
        shutil.copytree('/Users/scastro/casadir/utests/clean/data/'+self.msfile, self.msfile)
        
        
    
    def tearDown(self):
        os.system('rm -rf ' + self.msfile)
        
    def test1(self):
        """Test1: Wrong input should return False"""
        msfile = 'badfilename'
        results = clean(vis=msfile)
        self.assertFalse(results)
        
    def test2(self):
        """Test2: Good input should return None"""
        output = 'cleantest2.im'
        results = clean(vis=self.msfile,imagename=output)
        self.assertEqual(results,None)
        
    def test3(self):
        """Test3: Check if output exists"""
        output = 'cleantest3.im'
        results = clean(vis=self.msfile,imagename=output)
        self.assertTrue(os.path.exists(output+'.image'))
        
    def test4(self):
        """Test4: Wrong field type"""
        output = 'cleantest4.im'
        results = clean(vis=self.msfile,imagename=output,field=0)
        self.assertFalse(results)
        
    #FIXME: catch the error    
    def test5(self):
        """Test5: Wrong spw value"""
        output = 'cleantest5.im'
        self.failUnlessRaises(Exception, clean(vis=self.msfile,imagename=output,spw=10))
        
    def test6(self):
        """Test6: Empty mode value"""
        output = 'cleantest6.im'
        results = clean(vis=self.msfile,imagename=output,mode='')
        self.assertFalse(results)
        
    def test7(self):
        """Test7: Unsupported gridmode"""
        output = 'cleantest7.im'
        results = clean(vis=self.msfile,imagename=output,gridmode='grid')
        self.assertFalse(results)
        
    def test8(self):
        """Test8: Test gridmode=widefield"""
        output = 'cleantest8.im'
        results = clean(vis=self.msfile,imagename=output,gridmode='widefield')
        self.assertEqual(results, None)
 
    #FIXME: catch error
    def test9(self):
        """Test8: Test gridmode=aprojection"""
        output = 'cleantest9.im'
        self.failUnlessRaises(Exception, clean(vis=self.msfile,imagename=output,
                                               gridmode='aprojection'))
        pass
                 
    def test10(self):
        """Test10: Wrong niter type"""
        output = 'cleantest10.im'
        results = clean(vis=self.msfile,imagename=output,niter='1')
        self.assertFalse(results)
        
    def test11(self):
        """Test11: Unsupported psfmode"""
        output = 'cleantest11.im'
        results = clean(vis=self.msfile,imagename=output,psfmode='psf')
        self.assertFalse(results)
        
    def test12(self):
        """Test12: Test psfmode=clark"""
        output = 'cleantest12.im'
        results = clean(vis=self.msfile,imagename=output,psfmode='clark')
        self.assertEqual(results, None)
       
    def test13(self):
        """Test13: Test psfmode=hogbom"""
        output = 'cleantest13.im'
        results = clean(vis=self.msfile,imagename=output,psfmode='hogbom')
        self.assertEqual(results, None)
       
    def test14(self):
        """Test14: Test psfmode=clarkstokes"""
        output = 'cleantest14.im'
        results = clean(vis=self.msfile,imagename=output,psfmode='clarkstokes')
        self.assertEqual(results, None)

    def test15(self):
        """Test15: Unsupported imagermode"""
        output = 'cleantest15.im'
        results = clean(vis=self.msfile,imagename=output,imagermode='clark')
        self.assertFalse(results)

    def test16(self):
        """Test16: Test imagermode=csclean"""
        output = 'cleantest16.im'
        results = clean(vis=self.msfile,imagename=output,imagermode='csclean')
        self.assertEqual(results, None)

    def test17(self):
        """Test17: Test imagermode=mosaic"""
        output = 'cleantest17.im'
        results = clean(vis=self.msfile,imagename=output,imagermode='mosaic')
        self.assertEqual(results, None)

#    FIXME: Catch exception
    def test18(self):
        """Test18: Zero value of imsize"""
        output = 'cleantest18.im'
        self.failUnlessRaises(Exception, clean(vis=self.msfile,imagename=output,imsize=0))



def suite():
    return [clean_test]
