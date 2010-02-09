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
#                            '/data/regression/smoothcal/'+self.msfile, self.msfile)
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
        results = clean(vis=self.msfile,imagename=output,spw=10)
        
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
        """Test8: Wrong niter type"""
        output = 'cleantest8.im'
        results = clean(vis=self.msfile,imagename=output,niter='1')
        self.assertFalse(results)
        
       
 


#    def test_execution_failure(self):
#        raise Exception("die")
#
#    def test_r_r_1(self):
#        "test revision vs revision"
#        a = "CASA Version 3.0.1 (r10006)"
#        b = "CASA Version 3.0.1 (r9933)"
#
#        self.order(b, a)
#
#    def test_r_r_2(self):
#        a = "CASA Version 3.0.1 (r9936)"
#        b = "CASA Version 3.0.1 (r9933)"
#
#        self.order(b, a)
#
#    def test_r_r_3(self):
#        a = "CASA Version 3.0.0 (r9888)"
#        b = "CASA Version 3.0.1 (r9913)"
#
#        self.order(a, b)
#
#    def test_build_build_1(self):
#        a = "CASA Version 2.4.0 (build #8115)"
#        b = "CASA Version 2.4.0 (build #7782)"
#
#        self.order(b, a)
#        
#    def test_build_build_2(self):
#        a = "CASA Version 2.4.0 (build #8115)"
#        b = "CASA Version 3.0.0 (build #9684)"
#
#        self.order(a, b)
#
#    def test_build_r_1(self):
#        a = "CASA Version 3.0.0 (r9886)"
#        b = "CASA Version 3.0.0 (build #9684)"
#
#        self.order(b, a)
#
#    def test_build_r_2(self):
#        a = "CASA Version 3.0.1 (r10006)"
#        b = "CASA Version 3.0.0 (build #9684)"
#
#        self.order(b, a)
#
#
#    def order(self, a, b):
#        """Verify that the cmp_version function behaves
#        as it should, given that a is earlier than b"""
#        
#        assert report.cmp_version(a, b) < 0
#        assert report.cmp_version(b, a) > 0
#        assert report.cmp_version(a, a) == 0
#        assert report.cmp_version(b, b) == 0

def suite():
    return [clean_test]
