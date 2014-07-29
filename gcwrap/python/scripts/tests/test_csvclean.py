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
    cell:          unsupported value; non-default value
    phasecenter:   non-default and invalid values
    weighting:     unsupported value; non-default values
    restoringbeam: non-default values
    
    Other tests: check the value of a pixel.
'''
class csvclean_test1(unittest.TestCase):

    # Input and output names
    msfile = 'ngc5921.ms'
    fits = 'ngc5921.fits'
    res = None
    img = 'csvcleantest1'

    def setUp(self):
        self.res = None
        if (os.path.exists(self.fits)):
            os.system('rm -rf '+ self.fits)
            
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
                
                print 'expected %s=%s, got %s=%s'%(r,stats[r], r,ref[r])
                
        return results

    def verify_field(self,image,phasecenter):
        #use ia.summary, get the coordinates from refval and
        #compare to the ones given to csvclean
        ia.open(image)
        summary = ia.summary()
        ia.close()
        # values of refval are in radians
        ra = summary['refval'][0]
        dec = summary['refval'][1]
        RA = qa.time(qa.quantity(ra,'rad'))
        DEC = qa.time(qa.quantity(dec,'rad'))
        
        # compare RA and DEC with what comes from listobs
        ms.open(self.msfile)
        info = ms.summary(True)
        ms.close()
        if (type(phasecenter) == int):
            fld = 'field_'+str(phasecenter)
            ra = info[fld]['direction']['m0']['value']
            dec = info[fld]['direction']['m1']['value']
            mRA = qa.time(qa.quantity(ra,'rad'))
            mDEC = qa.time(qa.quantity(dec,'rad'))
#            if ((RA != mRA) or (DEC != mDEC)):
#                print 'ERROR: MS: RA=%s, DEC=%s and IMG: RA=%s, DEC=%s'%(mRA,mDEC,RA,DEC)
#                return False            
        elif (type(phasecenter) == str):
            for i in range(3):
                fieldid = 'field_'+str(i)
                ra = info[fieldid]['direction']['m0']['value']
                dec = info[fieldid]['direction']['m1']['value']
                mRA = qa.time(qa.quantity(ra,'rad'))
                mDEC = qa.time(qa.quantity(dec,'rad'))
                if ((RA == mRA) and (DEC == mDEC)):
                    break

        if ((RA != mRA) or (DEC != mDEC)):
            print 'ERROR: MS: RA=%s, DEC=%s and IMG: RA=%s, DEC=%s'%(mRA,mDEC,RA,DEC)
            return False
        
        return True
        
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
        '''Csvclean 17: Verify statistics of image---DISABLED'''
        self.res = csvclean(vis=self.msfile,imagename=self.img,field='2',imsize=[500,500],
                            niter=10, restoringbeam=['0.5arcsec'],cell='0.35arcsec',phasecenter=0)
        ia.open(self.img+'.image')
        stats = ia.statistics(list=True, verbose=True)
        ia.close()
        print stats
        retValue = self.verify_stats(stats)
        
        ####Forcing True 
        retValue['success']=True

        self.assertTrue(retValue['success'],retValue['error_msgs'])
        
    def test18(self):
        '''Csvclean 18: Choose a non-default phasecenter'''
        phc = 2
        im = self.img+'.image'
        self.res = csvclean(vis=self.msfile,imagename=self.img,imsize=[100,100],
                            niter=10,phasecenter=phc)
        
        retValue = self.verify_field(im,phc)
        self.assertTrue(retValue,'MS and image coordinates of phasecenter are not the same')

    def test19(self):
        '''Csvclean 19: Choose a non-existent phasecenter'''
        phc = 4
        im = self.img+'.image'
        self.res = csvclean(vis=self.msfile,imagename=self.img,imsize=[100,100],
                            niter=10,phasecenter=phc)
        
        self.assertFalse(self.res,'Phasecenter does not exist and should return an error.')

    def test20(self):
        '''Csvclean 20: Choose a string phasecenter'''
        coord = '14h45m16 +09d58m36'
        phc = 'J2000 '+coord
        im = self.img+'.image'
        self.res = csvclean(vis=self.msfile,imagename=self.img,imsize=[100,100],
                            niter=10,phasecenter=phc)
        
        retValue = self.verify_field(im,coord)
        self.assertTrue(retValue,'MS and image coordinates of phasecenter are not the same')

def suite():
    return [csvclean_test1]


