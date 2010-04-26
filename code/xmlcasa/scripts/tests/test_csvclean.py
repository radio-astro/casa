import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest

'''
Unit tests for task csvclean. It tests the following parameters:
    vis:           wrong and correct values
    imagename:     if output exists
    field:         wrong field type; non-default value
    spw:           wrong value; non-default value
    niter:         wrong type; non-default values
    imsize:        zero value; non-default value
    cell:        unsupported value; non-default value
    weighting:     unsupported value; non-default values
    restoringbeam:    
    
    Other tests: check the value of a pixel.
'''
class csvclean_test1(unittest.TestCase):

    # Input and output names
    msfile = 'J1058+015.ms'
    res = None
    img = 'csvcleantest1'

    def setUp(self):
        self.res = None
        default(csvclean)
        if (os.path.exists(self.msfile)):
            shutil.rmtree(self.msfile)
            
        datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/csvclean/'
        shutil.copytree(datapath+self.msfile, self.msfile)
    
    def tearDown(self):
        if (os.path.exists(self.msfile)):
            shutil.rmtree(self.msfile)

        os.system('rm -rf ' + self.img+'*')
     

    def getpixval(self,img,pixel):
        ia.open(img)
        px = ia.pixelvalue(pixel)
        ia.close()
        return px['value']['value']
        
    def test1(self):
        '''Csvclean 1: Default values'''
        self.res = csvclean()
        self.assertFalse(self.res)
        
    def test2(self):
        """Csvclean 2: Wrong input should return False"""
        msfile = 'badfilename'
        self.res = csvclean(vis=msfile, imagename=self.img)
        self.assertFalse(self.res)
        
    def test3(self):
        """Csvclean 3: Good input should return None"""
        self.res = csvclean(vis=self.msfile,imagename=self.img)
        self.assertTrue(self.res)
        
    def test4(self):
        """Csvclean 4: Check if output exists"""
        self.res = csvclean(vis=self.msfile,imagename=self.img)
        self.assertTrue(os.path.exists(self.img+'.image'))
        
    def test5(self):
        """Csvclean 5: Wrong field type"""
        self.res = csvclean(vis=self.msfile,imagename=self.img,field=0)
        self.assertFalse(self.res)
        
    def test6(self):
        """Csvclean 6: Non-default field value"""
        self.res = csvclean(vis=self.msfile,imagename=self.img,field='0')
        self.assertTrue(self.res)
        self.assertTrue(os.path.exists(self.img+'.image'))           
        
    def test7(self):
        """Csvclean 7: Wrong spw value"""
        self.res = csvclean(vis=self.msfile,imagename=self.img,spw='10')
        self.assertFalse(os.path.exists(self.img+'.image'))
       
    def test8(self):
        """Csvclean 8: Non-default spw value"""
        self.res = csvclean(vis=self.msfile,imagename=self.img,spw='0')
        self.assertTrue(os.path.exists(self.img+'.image'))

                     
    def test9(self):
        """Csvclean 9: Wrong niter type"""
        self.res = csvclean(vis=self.msfile,imagename=self.img,niter='1')
        self.assertFalse(self.res)
        
    def test10(self):
        """Csvclean 10: Non-default niter values"""
        for n in range(10,400,50):
            self.res = csvclean(vis=self.msfile,imagename=self.img,niter=n)
            self.assertTrue(self.res,'Failed for niter = %s' %n)    

    def test11(self):
        """Csvclean 11: Zero value of imsize"""
        self.res = csvclean(vis=self.msfile,imagename=self.img,imsize=0)
        self.assertFalse(os.path.exists(self.img+'.image'))

    def test12(self):
        '''Csvclean 12: Non-default imsize values'''
        self.res = csvclean(vis=self.msfile,imagename=self.img,imsize=[80,80])
        self.assertTrue(self.res)
        self.assertTrue(os.path.exists(self.img+'.image'),'Image %s does not exist' %self.img)

    def test13(self):
        """Csvclean 13: Non-default cell values"""
        self.res = csvclean(vis=self.msfile,imagename=self.img, cell=12.5)
        self.assertTrue(self.res,'Task returned %s'%self.res)
        self.assertTrue(os.path.exists(self.img+'.image'))
                
    def test14(self):
        '''Csvclean 14: Unsupported weighting mode'''
        self.res = csvclean(vis=self.msfile,imagename=self.img, weighting='median')
        self.assertFalse(self.res)
        
    def test15(self):
        '''Csvclean 15: Non-default weighting uniform'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }            
        weights = ['natural','uniform','briggs','radial']
        for w in weights:
            out = self.img+'.'+w
            self.res = csvclean(vis=self.msfile,imagename=out, imsize=500,weighting=w)
            print out
            if not self.res:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Failed for weighting=%s."%w
            if not (os.path.exists(out+'image')):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Failed to create image=%s when weighting=%s."%(out+'.image',w)

#            shutil.rmtree(out+'.image',ignore_errors=True)
#            shutil.rmtree(out+'.model',ignore_errors=True)
            
        self.assertTrue(retValue['success'],retValue['error_msgs'])

    def test16(self):
        '''Csvlean 16: Default weighting briggs'''
        self.res = csvclean(vis=self.msfile,imagename=self.img, weighting='briggs')
        self.assertTrue(self.res)
        self.assertTrue(os.path.exists(self.img+'.image'))

    def test31(self):
        '''Clean 31: Non-default weighting radial'''
        self.res = csvclean(vis=self.msfile,imagename=self.img, weighting='radial')
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img+'.image'))
        
    def test34(self):
        '''Clean 34: Verify the value of pixel 50'''
        #run csvclean with some parameters
        self.res = csvclean(vis=self.msfile,imagename=self.img,selectdata=True,
                         timerange='>11:28:00',field='0~2',imsize=[100,100],niter=10)
        
        os.system('cp -r ' + self.img+'.image' + ' myimage.im')
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img+'.image'))
#        ref = 0.007161217276006937
        ref = 0.011824539862573147
        value = self.getpixval(self.img+'.image',50)
        diff = abs(ref - value)
        self.assertTrue(diff < 10e-5,'Something changed the flux values. ref_val=%s, new_val=%s'
                                        %(ref,value))
                

def suite():
    return [csvclean_test1]


