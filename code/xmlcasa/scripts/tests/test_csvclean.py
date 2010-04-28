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
#    msfile = 'J1058+015.ms'
    msfile = 'ngc5921.ms'
    fits = 'ngc5921.fits'
    res = None
    img = 'csvcleantest1'

    def setUp(self):
        self.res = None
        if (os.path.exists(self.fits)):
#            shutil.rmtree(self.msfile)
            os.remove(self.fits)
            
        datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/ngc5921/'
        shutil.copyfile(datapath+self.fits, self.fits)
        importuvfits(fitsfile=self.fits, vis=self.msfile)
        default(csvclean)

    def tearDown(self):
        if (os.path.exists(self.msfile)):
            shutil.rmtree(self.msfile)

        os.system('rm -rf ' + self.img+'*')
     

    def getpixval(self,img,pixel):
        ia.open(img)
        px = ia.pixelvalue(pixel)
        ia.close()
        return px['value']['value']
    
    def verify_stats(self,stats):
        results = {'success': True, 'msgs': "", 'error_msgs': '' }            
        # Reference statistical values
        ref = {'flux': ([-1.28087772]),
               'max': ([ 0.00011397]),
               'mean': ([ -1.18477533e-05]),
               'min': ([-0.00013796]),
               'npts': ([ 250000.]),
               'rms': ([  4.25508006e-05]),
               'sigma': ([  4.08681795e-05]),
               'sum': ([-2.96193832]),
               'sumsq': ([ 0.00045264])}
        
        for r in ref:
            diff = abs(ref[r] - stats[r][0])
            if (diff > 10e-5):
                results['success']=False
                results['error_msgs']=results['error_msgs']\
                     +"\nError: Statistics failed for %s. "%r
                        
        return results
                    
        
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
        """Csvclean 3: Good input should return True"""
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
        self.res = csvclean(vis=self.msfile,imagename=self.img,field='2')
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
            if not (os.path.exists(out+'.image')):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Failed to create image=%s when weighting=%s."%(out+'.image',w)
                     
        self.assertTrue(retValue['success'],retValue['error_msgs'])
       
    def test16(self):
        '''Csvclean 16: Test parameter restoringbeam'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }            
        p = ['0.5arcsec','0.25arcsec','0.3']
        out = 'myimg'
        self.res = csvclean(vis=self.msfile,imagename=out, imsize=500,restoringbeam='0.5arcsec')
        if not (os.path.exists(out+'.image')):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Failed to create image=%s when restoringbeam=\'0.25arcsec\'."%out+'.image'
        os.system('rm -rf myimg.image')

        beam = ['0.5arcsec','0.25arcsec']
        self.res = csvclean(vis=self.msfile,imagename=out, imsize=500,restoringbeam=beam)
        if not (os.path.exists(out+'.image')):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Failed to create image=%s when restoringbeam=%s."%(out+'.image',beam)
        os.system('rm -rf myimg.image')

        beam = ['0.5arcsec','0.25arcsec','0.3deg']
        self.res = csvclean(vis=self.msfile,imagename=out, imsize=500,restoringbeam=beam)
        if not (os.path.exists(out+'.image')):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Failed to create image=%s when restoringbeam=%s."%(out+'.image',beam)
        os.system('rm -rf myimg.image')
        
        self.assertTrue(retValue['success'],retValue['error_msgs'])

    def test17(self):
        '''Csvclean 17: Verify statistics of image'''
        self.res = csvclean(vis=self.msfile,imagename=self.img,field='2',imsize=[500,500],
                            niter=10, restoringbeam=['0.5arcsec'],cell='0.35arcsec')
        ia.open(self.img+'.image')
        stats = ia.statistics()
        ia.close()
        retValue = self.verify_stats(stats)
        self.assertTrue(retValue['success'],retValue['error_msgs'])
        

def suite():
    return [csvclean_test1]


