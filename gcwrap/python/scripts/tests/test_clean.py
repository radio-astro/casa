import os
import sys
import shutil
import commands
import numpy
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
class clean_test1(unittest.TestCase):

    # Input and output names
#    msfile = 'ngc7538_ut.ms'
    msfile = 'ngc7538_ut1.ms'
    res = None
    img = 'cleantest1'
    img2 = '0-cleantest1'
    msk = 'cleantest1.in.mask'
    boxmsk = 'cleantest1.box'

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
        
    def compareimages(self,inimage,refimage):
        """
        compare the input image with reference image
        return true if pix values are identical
        usually useful for mask images
        """
        ia.open(inimage)
        invals = ia.getchunk()
        ia.close()
        ia.open(refimage)
        refvals= ia.getchunk()
        ia.close()
        diff = refvals - invals
        return (numpy.all(diff==0))

    def test1(self):
        '''Clean 1: Default values'''
        self.res = clean()
        self.assertFalse(self.res)
        
    def test2(self):
        """Clean 2: Wrong input should return False"""
        msfile = 'badfilename'
        self.res = clean(vis=msfile, imagename=self.img, imagermode='')
        self.assertFalse(self.res)
        
    def test3(self):
        """Clean 3: Good input should return None"""
        self.res = clean(vis=self.msfile,imagename=self.img, imagermode='')
        self.assertEqual(self.res,None)
        
    def test4(self):
        """Clean 4: Check if output exists"""
        self.res = clean(vis=self.msfile,imagename=self.img)
        self.assertTrue(os.path.exists(self.img+'.image'))
        
    def test5(self):
        """Clean 5: Wrong field type"""
        self.res = clean(vis=self.msfile,imagename=self.img,field=0)
        self.assertFalse(self.res)
        
    def test6(self):
        """Clean 6: Non-default field value"""
        self.res = clean(vis=self.msfile,imagename=self.img,field='0~1', imagermode='')
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img+'.image'))           
        
    def test7(self):
        """Clean 7: Wrong spw value"""
        self.res = clean(vis=self.msfile,imagename=self.img,spw='10')
        self.assertFalse(os.path.exists(self.img+'.image'))
       
    def test8(self):
        """Clean 8: Non-default spw value"""
        self.res = clean(vis=self.msfile,imagename=self.img,spw='0', imagermode='')
        self.assertTrue(os.path.exists(self.img+'.image'))

    def test9(self):
        """Clean 9: Empty mode value"""
        self.res = clean(vis=self.msfile,imagename=self.img,mode='')
        self.assertFalse(self.res)

    # FIXME: CHANGE SUBPARAMETERS FOR DIFFERENT MODES
    def test10(self):
        """Clean 10: Non-default mode channel"""
        self.res = clean(vis=self.msfile,imagename=self.img,mode='channel',imagermode='')
        self.assertEqual(self.res,None)
        self.assertTrue(os.path.exists(self.img+'.image'),'Image %s does not exist'%self.img)

    def test11(self):
        """Clean 11: Non-default mode velocity"""
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }            
        # The introduction of a new image frequency gridding method
        # in imager (CAS-2576) causes the image channel width 
        # to change (wider, since the velocity width was calculated from lower
        # frequency side) for the default velocity mode. This cause the last
        # channel image to be blank. To avoid this use nearest interpolation. 
        res = clean(vis=self.msfile,imagename=self.img,mode='velocity',restfreq='23600MHz', interpolation='nearest',imagermode='')
        if(res != None):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Failed to run in velocity mode."
        if(not os.path.exists(self.img+'.image')):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Failed to create output image."
                         
        # Verify if there are blank planes at the edges
        vals = imval(self.img+'.image')
        size = len(vals['data'])

        if (vals['data'][0]==0 or vals['data'][size-1]==0):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: There are blank planes in the edges of the image."
                    

        self.assertTrue(retValue['success'],retValue['error_msgs'])
        
    def test12(self):
        """Clean 12: Non-default mode frequency"""
        self.res = clean(vis=self.msfile,imagename=self.img,mode='frequency',imagermode='')
        self.assertEqual(self.res,None)
        self.assertTrue(os.path.exists(self.img+'.image'),'Image %s does not exist'%self.img)
        
    def test13(self):
        """Clean 13: Unsupported gridmode"""
        self.res = clean(vis=self.msfile,imagename=self.img,gridmode='grid')
        self.assertFalse(self.res)
        
    def test14(self):
        """Clean 14: Non-default gridmode widefield"""
        self.res = clean(vis=self.msfile,imagename=self.img,gridmode='widefield',imsize=[20],imagermode='')
        self.assertEqual(self.res, None) 
        self.assertTrue(os.path.exists(self.img+'.image'),'Image %s does not exist'%self.img)

    # Takes too long!!!
#    def test11(self):
#        """Clean 11: Non-default gridmode aprojection"""
#        self.res = clean(vis=self.msfile,imagename=self.img,gridmode='aprojection',imsize=[10],niter=1)
#        self.assertEqual(self.res, None) 
#        self.assertTrue(os.path.exists(self.img+'.image'),'Image %s does not exist'%self.img)
                     
    def test15(self):
        """Clean 15: Wrong niter type"""
        self.res = clean(vis=self.msfile,imagename=self.img,niter='1')
        self.assertFalse(self.res)
        
    def test16(self):
        """Clean 16: Non-default niter values"""
        for n in range(10,400,50):
            self.res = clean(vis=self.msfile,imagename=self.img,niter=n, imagermode='')
            self.assertEqual(self.res,None,'Failed for niter = %s' %n)
    
    def test17(self):
        """Clean 17: Unsupported psfmode"""
        self.res = clean(vis=self.msfile,imagename=self.img,psfmode='psf')
        self.assertFalse(self.res)
        
    def test18(self):
        """Clean 18: Non-default psfmode hogbom"""
        self.res = clean(vis=self.msfile,imagename=self.img,psfmode='hogbom',imagermode='')
        self.assertEqual(self.res, None)            
        self.assertTrue(os.path.exists(self.img+'.image'))
        
    def test19(self):
        """Clean 19: Non-default psfmode clarkstokes"""
        self.res = clean(vis=self.msfile,imagename=self.img,psfmode='clarkstokes',imagermode='')
        self.assertEqual(self.res, None)            
        self.assertTrue(os.path.exists(self.img+'.image'))
      
    def test20(self):
        """Clean 20: Unsupported imagermode"""
        self.res = clean(vis=self.msfile,imagename=self.img,imagermode='clark')
        self.assertFalse(self.res)      

    def test21(self):
        '''Clean 21: Non-default imagermode csclean'''
        # now csclean is the default
        self.res = clean(vis=self.msfile,imagename=self.img,imagermode='csclean')
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img+'.image'))

    def test22(self):
        '''Clean 22: Non-default imagermode mosaic''' 
        self.res = clean(vis=self.msfile,imagename=self.img,imagermode='mosaic',imsize=[20])
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img+'.image'))

    def test23(self):
        """Clean 23: Zero value of imsize"""
        self.res = clean(vis=self.msfile,imagename=self.img,imsize=0)
        self.assertFalse(os.path.exists(self.img+'.image'))

    def test24(self):
        '''Clean 24: Non-default imsize values'''
        self.res = clean(vis=self.msfile,imagename=self.img,imsize=[80,80], imagermode='')
        self.assertEqual(self.res,None)
        self.assertTrue(os.path.exists(self.img+'.image'),'Image %s does not exist' %self.img)

    def test25(self):
        """Clean 25: Non-default cell values"""
        self.res = clean(vis=self.msfile,imagename=self.img, cell=12.5, imagermode='')
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img+'.image'))
        
    def test26(self):
        """Clean 26: Unsupported Stokes parameter"""
        self.res = clean(vis=self.msfile,imagename=self.img, stokes='V')
        self.assertFalse(self.res)
        
    def test27(self):
        """Clean 27: Non-default Stokes parameter"""
        self.res = clean(vis=self.msfile,imagename=self.img, stokes='RR', imagermode='')
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img+'.image'))
        
    def test28(self):
        '''Clean 28: Unsupported weighting mode'''
        self.res = clean(vis=self.msfile,imagename=self.img, weighting='median')
        self.assertFalse(self.res)
        
    def test29(self):
        '''Clean 29: Non-default weighting uniform'''
        self.res = clean(vis=self.msfile,imagename=self.img, weighting='uniform', imagermode='')
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img+'.image'))

    def test30(self):
        '''Clean 30: Non-default weighting briggs'''
        self.res = clean(vis=self.msfile,imagename=self.img, weighting='briggs', imagermode='')
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img+'.image'))

    def test31(self):
        '''Clean 31: Non-default weighting radial'''
        self.res = clean(vis=self.msfile,imagename=self.img, weighting='radial', imagermode='')
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img+'.image'))
        
    def test32(self):
        '''Clean 32: Non-default subparameters of selectdata'''
#        self.res = clean(vis=self.msfile,imagename=self.img,selectdata=True,
#                         timerange='>11:30:00',antenna='VA12')
        self.res = clean(vis=self.msfile,imagename=self.img,selectdata=True,
                         timerange='>11:30:00',antenna='VA01', imagermode='')
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img+'.image'))

    def test33(self):
        '''Clean 33: Wrong antenna subparameter of selectdata'''
        self.res = clean(vis=self.msfile,imagename=self.img,selectdata=True,
                         antenna='88')
        self.assertFalse(os.path.exists(self.img+'.image'))

    def test34(self):
        '''Clean 34: Verify the value of pixel 50'''
        #run clean with some parameters
        self.res = clean(vis=self.msfile,imagename=self.img,selectdata=True,
                         timerange='>11:28:00',field='0~2',imsize=[100,100],niter=10,imagermode='')
        
        os.system('cp -r ' + self.img + '.image' + ' myimage.im')
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img + '.image'))
#        ref = 0.007161217276006937
        ref = 0.011824539862573147
#        ref = 0.009637                                 # active rev. 12908
        ref = 0.011824540793895721   # active rec. 19307+ (after change to image bandwidth and reffreq)
        value = self.getpixval(self.img+'.image', 50)
        diff = abs(ref - value)
        self.assertTrue(diff < 10e-4,
                        'Something changed the pixel brightness. ref_val=%s, new_val=%s'
                                        %(ref,value))
        
    def test35(self):
        '''Clean 35: Wrong type of phasecenter'''
        self.res=clean(vis=self.msfile,imagename=self.img,phasecenter=4.5)
        self.assertFalse(os.path.exists(self.img+'.image'))
        
    def test36(self):
        '''Clean 36: Non-default value of phasecenter'''
        self.res=clean(vis=self.msfile,imagename=self.img,phasecenter=2,imagermode='')
        self.assertTrue(os.path.exists(self.img+'.image'))

    def test37(self):
        '''Clean 37: Test box mask'''
        self.res=clean(vis=self.msfile,imagename=self.img,mask=[115,115,145,145],imagermode='')
        self.assertTrue(os.path.exists(self.img+'.image') and
			os.path.exists(self.img+'.mask'))

    def test38(self):
        '''Clean 38: Test freeing of resource for mask (checks CAS-954)'''
        self.res=clean(vis=self.msfile,imagename=self.img,mask=[115,115,145,145],imagermode='')
        cmd='/usr/sbin/lsof|grep %s' % self.img+'.mask'
        output=commands.getoutput(cmd)
        ret=output.find(self.img+'.mask')
        self.assertTrue(ret==-1)

    def test39(self):
        '''Clean 39: Input mask image specified'''
        datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/clean/'
        shutil.copytree(datapath+self.msk, '1_'+self.msk)
        self.res=clean(vis=self.msfile,imagename=self.img2,mask='1_'+self.msk, imagermode='')
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img2+'.image'))
        # cleanup
        os.system('rm -rf ' + '1_'+self.msk+ ' ' + self.img2+'*')

    def test40(self):
        '''Clean 40: Test chaniter=T clean with flagged channels'''
        # test CAS-2369 bug fix 
        flagdata(vis=self.msfile,mode='manualflag',spw='0:0~0')
        self.res=clean(vis=self.msfile,imagename=self.img,mode='channel',chaniter=True, spw='0', imagermode='')
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img+'.image'))
         
    def test41(self):
        '''Clean 41: Test nterms=2 and ref-freq > maximum-frequency'''
        # This tests if negative-weights are being correctly allowed through the gridders
        self.res=clean(vis=self.msfile,imagename=self.img,nterms=2,reffreq='25.0GHz',niter=5,imagermode='');
        self.assertEqual(self.res,None);

    def test42(self):
        '''Clean42: Test nterms=2, with only one channel whose frequency is same as ref-freq'''
        # This tests if a numerical failure-mode is detected and returned without fuss.
        self.res=clean(vis=self.msfile,imagename=self.img,nterms=2,reffreq='23691.4682MHz',spw='0:0',imagermode='');
        self.assertFalse(self.res);

    def test43(self):
        '''Clean 43: Test user input mask from a boxfile'''
        datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/clean/'
        refpath=datapath+'reference/'
        shutil.copyfile(datapath+self.boxmsk, self.boxmsk)
        self.res=clean(vis=self.msfile,imagename=self.img,mode='channel', mask=self.boxmsk,imagermode='')
        self.assertEqual(self.res, None)
        self.assertTrue(os.path.exists(self.img+'.image'))
        self.assertTrue(self.compareimages(self.img+'.mask', refpath+'ref_cleantest1boxfile.mask'))

    def test44(self):
        '''Clean 44: Test input mask image of different channel ranges'''
        datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/clean/'
        refpath=datapath+'reference/'
        shutil.copyfile(datapath+self.boxmsk, self.boxmsk)
        # wider channel range mask 
        self.res=clean(vis=self.msfile,imagename=self.img,mode='channel', mask=[115,115,145,145], niter=10,imagermode='')
        self.assertEqual(self.res, None)
        # apply to narrower channel range 
        self.res=clean(vis=self.msfile,imagename=self.img+'.narrow',mode='channel', 
                       nchan=3, start=2, mask=self.img+'.mask', niter=10,imagermode='')
        self.assertEqual(self.res, None)
        # apply the narrower channel range mask to wider channel range clean
        self.res=clean(vis=self.msfile,imagename=self.img+'.wide',mode='channel',mask=self.img+'.narrow.mask', niter=10,imagermode='')
      
        # make sub-image from masks for comparison 
        ia.open(self.img+'.mask')
        r1=rg.box([0,0,0,2],[256,256,0,4])
        sbim=ia.subimage(outfile=self.img+'.subim.mask', region=r1)
        ia.close()
        sbim.close()
        #
        os.system('cp -r '+self.img+'.mask '+self.img+'.ref.mask')
        ia.open(self.img+'.narrow.mask')
        # note: narrow mask made with a single spw does not exactly 
        # match with the wider cube with 2 spws in width
        pixs = ia.getchunk(blc=[0,0,0,0],trc=[256,256,0,1])
        r1=rg.box([0,0,0,0],[256,256,0,1])
        ia.close()
        ia.open(self.img+'.ref.mask')
        ia.set(pixelmask=False)
        ia.set(pixelmask=True, region=r1)
        ia.putchunk(pixels=pixs,blc=[0,0,0,2])
        ia.close()

        self.assertTrue(os.path.exists(self.img+'.narrow.image'))
        self.assertTrue(os.path.exists(self.img+'.wide.image'))
        self.assertTrue(self.compareimages(self.img+'.narrow.mask', self.img+'.subim.mask'), 
                        "mask mismatch for applying a wider chan. range mask to narrower chan. range clean")
        self.assertTrue(self.compareimages(self.img+'.wide.mask', self.img+'.ref.mask'), 
                        "mask mismatch for applying a narrower chan. range mask to wider chan. range clean")

    def test45(self):
        """Clean 45: Test selection of obs ID 0 (present)"""
        self.res = clean(vis=self.msfile, imagename=self.img + '0',
                         selectdata=True, observation='0', niter=10, imagermode='')
        self.assertTrue(os.path.exists(self.img + '0.image'))

    def test46(self):
        """Clean 46: Test selection of obs ID 2 (absent)"""
        self.res = clean(vis=self.msfile, imagename=self.img + '2',
                         selectdata=True, observation='2', niter=10)
        self.assertFalse(os.path.exists(self.img + '2.image'))
        
     
class clean_test2(unittest.TestCase):
    
    # Input and output names
    msfile = 'split1scan.ms'
    res = None
    img = 'cleantest2'

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
        
    def test1a(self):
        """Clean 1a: Non-default mode velocity"""
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }            
        res = clean(vis=self.msfile,imagename=self.img,mode='velocity',restfreq='231901MHz')
        if(res != None):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Failed to run in velocity mode."
        if(not os.path.exists(self.img+'.image')):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Failed to create output image."
                         
        # Verify if there are blank planes at the edges
        vals = imval(self.img+'.image')
        size = len(vals['data'])
        if (vals['data'][0]==0.0 or vals['data'][size-1]==0.0):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: There are blank planes in the edges of the image."
                    

        self.assertTrue(retValue['success'],retValue['error_msgs'])

class clean_multifield_test(unittest.TestCase):

    # Input and output names#    msfile = 'ngc7538_ut.ms'
    #msfile = '0556kf.ms'
    msfile = 'outlier_ut.ms'
    res = None
    img = ['cleantest3a','cleantest3b']
    newoutlierfile='newoutlier4cleantest.txt'
    inmodel = 'cleantest3in.model'

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

        #os.system('rm -rf ' + self.img[0]+'* ')
        #os.system('rm -rf ' + self.img[1]+'* ')
	for imext in ['.image','.model','.residual','.psf']:
            if (os.path.exists(self.img[0]+imext)):
	        shutil.rmtree(self.img[0]+imext)
            if (os.path.exists(self.img[1]+imext)):
	        shutil.rmtree(self.img[1]+imext)

    #def testCAS1972(self):
    #    """Clean test3:test bug fixes: CAS-1972"""
    #    self.res= clean(vis=self.msfile,imagename=self.img,mode="mfs",interpolation="linear",
    #                    niter=100, psfmode="clark", 
    #                    imsize=[[512, 512], [512, 512]],
    #                    cell="0.0001arcsec", phasecenter=['J2000 05h59m32.03313 23d53m53.9267', 
    #                                                      'J2000 05h59m32.03313 23d53m53.9263'],
    #                    weighting="natural",pbcor=False,minpb=0.1)
    #
    #    self.assertEqual(self.res,None)
    #    for im in self.img:
    #        self.assertTrue(os.path.exists(im+'.image'))
    #    stat0 = imstat(self.img[0]+'.model')
    #    stat1 = imstat(self.img[1]+'.model')
    #    self.assertEqual(stat0['max'],stat1['max'])
    #    self.assertTrue(all(stat0['maxpos']==numpy.array([256,256,0,0])) and
    #           all(stat1['maxpos']==numpy.array([256,260,0,0])))

    def testCAS1972(self):
        """Clean test3:test CAS-1972 bug fixes ver2 (with smaller dataset)"""
        self.res= clean(vis=self.msfile,
                        imagename=self.img,
                        mode="mfs",
                        interpolation="linear",
                        niter=100, psfmode="clark",
                        imagermode='',
                        #mask=[[250, 250, 262, 262], [250, 350, 262, 362]],
                        # use (new) CASA region 
                        mask=['box[ [250pix, 250pix], [262pix, 262pix]]', 'box[ [250pix, 350pix], [262pix, 362pix]]'],
                        imsize=[[512, 512], [512, 512]],
                        cell="0.0001arcsec", 
                        phasecenter=['J2000 05h59m32.03313 23d53m53.9267', 'J2000 05h59m32.03313 23d53m53.9167'],
                        weighting="natural",
                        pbcor=False,
                        minpb=0.1)

        self.assertEqual(self.res,None)
        # quick check on the peaks apear at the location as expected
        for img in self.img:
            self.assertTrue(os.path.exists(img+".image"))
        stat0 = imstat(self.img[0]+'.model')
        stat1 = imstat(self.img[1]+'.model')
        #self.assertEqual(stat0['max'],stat1['max'])
        self.assertTrue(all(stat0['maxpos']==numpy.array([256,256,0,0])) and
               all(stat1['maxpos']==numpy.array([256,356,0,0])))

    def testOutlier2(self):
        """Clean test3:test task parm input with outlier file (CAS-3221)"""
        datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/clean/'
        shutil.copy(datapath+self.newoutlierfile,self.newoutlierfile)
        self.res= clean(vis=self.msfile,
                        imagename=self.img[0],
                        outlierfile=self.newoutlierfile,
                        mode="mfs",
                        interpolation="linear",
                        niter=100, psfmode="clark",
                        imagermode='',
                        mask=[250, 250, 262, 262],
                        imsize=[512, 512],
                        cell="0.0001arcsec",
                        phasecenter='J2000 05h59m32.03313 23d53m53.9267',
                        weighting="natural",
                        pbcor=False,
                        minpb=0.1)

        #print "DONE clean"
        self.assertEqual(self.res,None)
        # quick check on the peaks apear at the location as expected
        for img in self.img:
            self.assertTrue(os.path.exists(img+".image"))
        stat0 = imstat(self.img[0]+'.model')
        stat1 = imstat(self.img[1]+'.model')
        #self.assertEqual(stat0['max'],stat1['max'])
        self.assertTrue(all(stat0['maxpos']==numpy.array([256,256,0,0])) and
               all(stat1['maxpos']==numpy.array([256,356,0,0])))

    def testOutlier3(self):
        """Clean test3:test task parm input with outlier file and  with user-specified mask and model"""
        datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/clean/'
        shutil.copy(datapath+self.newoutlierfile,self.newoutlierfile)
        shutil.copytree(datapath+self.inmodel, self.inmodel)
        self.res= clean(vis=self.msfile,
                        imagename=self.img[0],
                        outlierfile=self.newoutlierfile,
                        mode="mfs",
                        interpolation="linear",
                        niter=100, psfmode="clark",
                        imagermode='',
                        mask='box [[250pix, 250pix], [262pix, 262pix]]',
                        imsize=[512, 512],
                        cell="0.0001arcsec",
                        phasecenter='J2000 05h59m32.03313 23d53m53.9267',
                        weighting="natural",
                        pbcor=False,
                        minpb=0.1)

        #print "DONE clean"
        self.assertEqual(self.res,None)
        # quick check on the peaks apear at the location as expected
        for img in self.img:
            self.assertTrue(os.path.exists(img+".image"))
        stat0 = imstat(self.img[0]+'.model')
        stat1 = imstat(self.img[1]+'.model')
        #self.assertEqual(stat0['max'],stat1['max'])
        self.assertTrue(all(stat0['maxpos']==numpy.array([256,256,0,0])) and
               all(stat1['maxpos']==numpy.array([256,356,0,0])))

class clean_multiterm_multifield_test(unittest.TestCase):

    msfile = 'tpts_2chan_split.ms'
    #msfile = 'tpts_2chan.ms'
    res = None
    img = ['mtmf1','mtmf2']
    outlierfile = 'mtmf_outlier_withmodel.txt'
    inmodeldir = 'testmodels';
    modim2=['testmodels/inmodel0.model.tt0','testmodels/inmodel1.model.tt1']

    # Check which data cols exist on disk
    def checkdatacols(self,vis=''):
        tb.open(vis)
        colnames = tb.colnames()
        print 'data : ', colnames.count('DATA'), '  model : ', colnames.count('MODEL_DATA'),  ' corrected : ', colnames.count('CORRECTED_DATA')
        #os.system('du -khs '+vis)
        return [ colnames.count('DATA') , colnames.count('MODEL_DATA') , colnames.count('CORRECTED_DATA') ]

    # Delete model and corrected columns.
    def delmodelcorrcols(self,vis=''):
        tb.open(vis, nomodify=False);
        colnames = tb.colnames()
        remcols = []
        if colnames.count('CORRECTED_DATA'):
            remcols.append('CORRECTED_DATA')

        if colnames.count('MODEL_DATA'):
            remcols.append('MODEL_DATA')

        tb.removecols(remcols);
        tb.close();

    def setUp(self):
        self.res = None
        datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/clean/multiterm_multifield_data/'
##        datapath = '/home/vega/rurvashi/CASAtrees/DATATree/clean/multiterm_multifield_data/'
        default(clean)
        if (os.path.exists(self.msfile)):
            os.system('rm -rf ' + self.msfile)

        shutil.copytree(datapath+self.msfile, self.msfile)
        shutil.copy(datapath+self.outlierfile,self.outlierfile)
        shutil.copytree(datapath+self.inmodeldir, self.inmodeldir)

    def tearDown(self):
        if (os.path.exists(self.msfile)):
            os.system('rm -rf ' + self.msfile)
        if (os.path.exists(self.inmodeldir)):
            os.system('rm -rf ' + self.inmodeldir)
        if (os.path.exists(self.outlierfile)):
            os.system('rm -rf ' + self.outlierfile)

	for imext in ['.image.tt0','.image.tt1','.model.tt0','.model.tt1','.residual.tt0','.residual.tt1','.psf.tt0','.psf.tt1','.image.alpha','.flux','.mask']:
            if (os.path.exists(self.img[0]+imext)):
	        shutil.rmtree(self.img[0]+imext)
            if (os.path.exists(self.img[1]+imext)):
	        shutil.rmtree(self.img[1]+imext)

    def testMultiTermOutlier(self):
        """Clean testMTOutlier1 (multi-term+multi-field : CAS-2664) : test task param input with nterms=2 and outlier file and with user-specified mask and model"""

        # Run clean with outlier file
        self.res= clean(vis=self.msfile,
                        imagename=self.img[0],
                        outlierfile=self.outlierfile,
                        mode="mfs",
                        nterms=2,
                        niter=10,
                        imagermode='',
                        mask='circle[[50pix , 50pix] ,15pix ]', 
                        modelimage=['testmodels/inmodel0.model.tt0','testmodels/inmodel0.model.tt1'],
                        imsize=[100, 100],
                        interactive=False,
                        npercycle=10,
                        cell="8.0arcsec",
                        weighting="briggs",
                        pbcor=False,
                        minpb=0.1)

        self.assertEqual(self.res,None)

        # quick check on the peaks apear at the location as expected
        for img in self.img:
              self.assertTrue(os.path.exists(img+".image.tt0"))
              self.assertTrue(os.path.exists(img+".image.tt1"))
        stat00 = imstat(self.img[0]+'.model.tt0')
        stat10 = imstat(self.img[1]+'.model.tt0')
        stat01 = imstat(self.img[0]+'.model.tt1')
        stat11 = imstat(self.img[1]+'.model.tt1')

        self.assertTrue(all(stat00['maxpos']==numpy.array([50,50,0,0])) and
                 all(stat10['maxpos']==numpy.array([15,15,0,0])))
        self.assertTrue(all(stat01['minpos']==numpy.array([50,50,0,0])) and
                 all(stat11['minpos']==numpy.array([15,15,0,0])))

## Uncomment after CAS-3688 gets fixed, so that this can run in the same casapy session.
## It will test re-running clean twice, in different ways.
    def testMultiTermLists(self):
        """Clean testMTOutlier2 (multi-term+multi-field) : test task param input with nterms=2 and inline lists for modelimages and masks"""

        # Run clean with inline lists
        self.res= clean(vis=self.msfile,
                        imagename=self.img,
                        mode="mfs",
                        nterms=2,
                        niter=10,
                        mask=[ 'circle[[50pix , 50pix] ,15pix ]' , 'circle[[15pix , 15pix] ,8pix ]' ], 
                        modelimage=[ ['testmodels/inmodel0.model.tt0','testmodels/inmodel0.model.tt1'] , ['testmodels/inmodel1.model.tt0','testmodels/inmodel1.model.tt1' ] ],
                        imsize=[ [100,100] , [50,50] ],
                        phasecenter = [0 ,'J2000 19h58m34.032 +40d57m20.763' ],
                        interactive=False,
                        npercycle=10,
                        cell="8.0arcsec",
                        weighting="briggs",
                        pbcor=False,
                        minpb=0.1)

        self.assertEqual(self.res,None)

        # quick check on the peaks apear at the location as expected
        for img in self.img:
              self.assertTrue(os.path.exists(img+".image.tt0"))
              self.assertTrue(os.path.exists(img+".image.tt1"))
        stat00 = imstat(self.img[0]+'.model.tt0')
        stat10 = imstat(self.img[1]+'.model.tt0')
        stat01 = imstat(self.img[0]+'.model.tt1')
        stat11 = imstat(self.img[1]+'.model.tt1')

        self.assertTrue(all(stat00['maxpos']==numpy.array([50,50,0,0])) and
                 all(stat10['maxpos']==numpy.array([15,15,0,0])))
        self.assertTrue(all(stat01['minpos']==numpy.array([50,50,0,0])) and
                 all(stat11['minpos']==numpy.array([15,15,0,0])))

    def testMultiTermOutlierUvsub(self):
        """Clean testMTOutlier3 (multi-term+multi-field+uvsub) : test task param input with nterms=2, inline lists for outlier fields, clean with niter=0 to predict a model, and uvsub to subtract outliers.
        """

        msname = self.msfile

        usescratch = True  ##  Setting to False, has an error in casapy 4.2,4.3 for multi-term (in uvsub).
        testmschange = True  # Calculate models on one ms, subtract from another

        # State-checker...
        rstate = True
        
        print '(0) Start with only data column'
        self.delmodelcorrcols(msname);
        rstate = rstate &  ( self.checkdatacols(msname) == [1,0,0] )

        ## (1) Run clean with imsize=100, one field, 2 terms.
        os.system('rm -rf try_uvsub_1*');
        clean(vis = msname, \
                  imagename = 'try_uvsub_1', \
                  imsize=100,\
                  mask = 'circle[[50pix , 50pix] ,8pix ]',\
                  cell='8.0arcsec',\
                  niter=10, \
                  gain=0.5, \
                  interactive=False, \
                  npercycle=5,\
                  psfmode='hogbom',\
                  weighting='briggs',\
                  nterms=2,\
                  usescratch=usescratch);

        print '(1) After first run'
        rstate = rstate &  ( self.checkdatacols(msname) == [1,int(usescratch),0] )
        if rstate==False:
            print 'FAIL : usescratch=', usescratch, '  in clean'
        self.assertTrue( rstate )

        ## (2) Run clean with two fields, 100, 50, both with 2 terms
        os.system('rm -rf try_uvsub_2*');
        clean(vis = msname, \
                  imagename = [ 'try_uvsub_2_main', 'try_uvsub_2_outlier' ], \
                  imsize=[ [100,100], [50,50] ], \
                  phasecenter = [ 0 , 'J2000 19h58m34.032 +40d57m20.763' ], \
                  mask = ['circle[[50pix , 50pix] ,8pix ]' , 'circle[[15pix , 15pix] ,8pix ]'],\
                  cell=['8.0arcsec'],\
                  niter=10, \
                  gain=0.5, \
                  interactive=False, \
                  npercycle=5,\
                  psfmode='hogbom',\
                  weighting='briggs',\
                  nterms=2,\
                  usescratch=usescratch);
        
        print '(2) After the multi-field run'
        rstate = rstate &  ( self.checkdatacols(msname) == [1,int(usescratch),0] )
        if rstate==False:
            print 'FAIL : usescratch=', usescratch, '  in clean'
        self.assertTrue( rstate )
            
        if testmschange == True:
            self.delmodelcorrcols(msname);
            print '(2.5) Deleting model col again'
            rstate = rstate &  ( self.checkdatacols(msname) == [1,0,0] )
            if rstate==False:
                print 'FAIL : Error in deleting columns'
                
        ## (3) Run clean with niter=0 and only the second field model (2 terms)
        clean(vis = msname, \
                  imagename = 'try_uvsub_2_outlier', \
                  imsize=[50,50],\
                  phasecenter='J2000 19h58m34.032 +40d57m20.763' ,\
                  mask = 'circle[[15pix , 15pix] ,8pix ]',\
                  cell=['8.0arcsec'],\
                  niter=0, \
                  weighting='briggs',\
                  nterms=2,\
                  usescratch=usescratch);
        
        print '(3) After the niter=0 prediction run with usescratch=', usescratch
        rstate = rstate &  ( self.checkdatacols(msname) == [1,int(usescratch),0] )
        if rstate==False:
            print 'FAIL : usescratch=', usescratch, '  in clean'
        self.assertTrue( rstate )
            
        ## (4) uvsub
        uvsub(vis = msname)   

        print '(4) After uvsub'
        rstate = rstate &  ( self.checkdatacols(msname) == [1,int(usescratch),1] )
        if rstate==False:
            print 'FAIL : uvsub scratch-column construction. Should be ', [1,int(usescratch),1]
        self.assertTrue( rstate )
            
        ## (5) Repeat (1) and sidelobes from the faraway source should have gone.
        os.system('rm -rf try_uvsub_3*');
        clean(vis = msname, \
                  imagename = 'try_uvsub_3', \
                  imsize=100,\
                  mask =  'circle[[50pix , 50pix] ,8pix ]',\
                  cell='8.0arcsec',\
                  niter=10, \
                  gain=0.5, \
                  interactive=False, \
                  npercycle=5,\
                  psfmode='hogbom',\
                  weighting='briggs',\
                  nterms=2,\
                  usescratch=usescratch);
        
        print '(5) Finished second clean run'
        rstate = rstate &  ( self.checkdatacols(msname) == [1,int(usescratch),1] )
        if rstate==False:
            print 'FAIL : usescratch in clean'
        self.assertTrue( rstate )
            
        print '(6) Check that try_uvsub_3.image.tt0  looks like try_uvsub_2_main.image.tt0  and not have huge circles of try_uvsub_1.image.tt0 in the top right corner'

        stat1 = imstat('try_uvsub_1.image.tt0')
        stat2 = imstat('try_uvsub_2_main.image.tt0')
        stat3 = imstat('try_uvsub_3.image.tt0')
        
        print "Peak positions : ", stat1['maxpos'] , stat2['maxpos'] , stat3 ['maxpos']

        rstate = rstate & all(stat1['maxpos']==numpy.array([50,50,0,0]))
        rstate = rstate & all(stat1['maxpos']==numpy.array([50,50,0,0]))
        rstate = rstate & all(stat1['maxpos']==numpy.array([50,50,0,0]))
        
        statcorner1 = imstat('try_uvsub_1.image.tt0', box='70,70,99,99')
        statcorner2 = imstat('try_uvsub_2_main.image.tt0', box='70,70,99,99')
        statcorner3 = imstat('try_uvsub_3.image.tt0', box='70,70,99,99')
        
        print "Corner RMS : ", statcorner1['rms'] , statcorner2['rms'] , statcorner3 ['rms']
        
        rstate = rstate & (float(statcorner1['rms']) > 0.049 and float(statcorner1['rms']) < 0.05)
        rstate = rstate & (float(statcorner2['rms']) > 0.0020 and float(statcorner2['rms']) < 0.0022)
        rstate = rstate & (float(statcorner3['rms']) > 0.0020 and float(statcorner3['rms']) < 0.0022)

        self.assertTrue( rstate )
        
#########################################################

class clean_multims_test(unittest.TestCase):
    # unit tests for multiple ms inputs
    msfiles = ['point_spw1.ms', 'point_spw2.ms']
    res = None
    img = 'cleantest_multims'
    mask = 'multims-mfs.topo.mask'

    def setUp(self):
        self.res = None
        default(clean)
        for msfile in self.msfiles:
          if (os.path.exists(msfile)):
            os.system('rm -rf ' + msfile)
          datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/clean/'
          #datapath = '../'
          shutil.copytree(datapath+msfile, msfile)

    def tearDown(self):
        for msfile in self.msfiles:
            if (os.path.exists(msfile)):
                os.system('rm -rf ' + msfile)

        for imext in ['.image','.model','.residual','.psf','.flux','.mask']:
            if (os.path.exists(self.img+imext)):
                shutil.rmtree(self.img+imext)

    def test_multims1(self):
        '''Clean multims test1: Test two cases with wrong numbers of spw or field given)'''
        datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/clean/'
        resfail1=clean(vis=self.msfiles,spw=['0~1','0~1','0'],field=['0~2','0~2'],imagename=self.img,imagermode='')
        resfail2=clean(vis=self.msfiles,spw=['0~1','0~1'],field=['0~2','0~2','0~2'],imagename=self.img,imagermode='')
        self.res=clean(vis=self.msfiles,spw=['0~1','0~1'],field=['0~2','0~2'],imagename=self.img,imagermode='')
        self.assertFalse(resfail1)
        self.assertFalse(resfail2)
        self.assertEqual(self.res,None)

    def test_multims2(self):
        '''Clean multims test2: Test multiple MSes input in mfs mode'''
        # expected reference values on r.15199 
        #refimbandw=1.0e+09
        #refimfreq=1.49e+09
        #refimmax=1.198562
        # expected reference values for r19307+  (change to defineimage bandwidth and reffreq)
        refimbandw=1.019991294e+09
        refimfreq=1.489984788e+09
        refimmax=1.19856226444

        datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/clean/'
        self.res=clean(vis=self.msfiles,mode='mfs', spw='', field='',imsize=[200],
                       cell=['4.0arcsec', '4.0arcsec'], imagename=self.img,imagermode='')
        self.assertEqual(self.res,None)
        imhout=imhead(imagename=self.img+'.image',mode='list')
        #print "imhout['cdelt4']=", imhout['cdelt4'], " imhout['crval4']=", imhout['crval4'], " imhout['datamax']=",imhout['datamax']
        imbwrdiff = abs(refimbandw-float(imhout['cdelt4']))/refimbandw
        imfreqrdiff = abs(refimfreq-float(imhout['crval4']))/refimfreq
        maxrdiff = abs(refimmax-float(imhout['datamax']))/refimmax
        # should exactly match for bandwidth and center frequency
        self.assertTrue(imbwrdiff < 1.0e-9)
        self.assertTrue(imfreqrdiff < 1.0e-9)
        self.assertTrue(maxrdiff < 0.01)

    def test_multims3(self):
        '''Clean multims test3: Test multiple MSes input in mfs mode with spw channel selection'''
        # expected reference values on r.15199
        #refimbandw=9.2e+08
        #refimfreq=1.45e+09
        #refimmax=1.25990
        # expected reference values for r19307+  (change to defineimage bandwidth and reffreq)
        refimbandw=9.59991881e+08
        refimfreq=1.459985082e+09
        refimmax=1.2438338995

        datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/clean/'
        self.res=clean(vis=self.msfiles,mode='mfs', spw=['0:0~19','0:0~16'], field='',imsize=[200],
                       cell=['4.0arcsec', '4.0arcsec'], imagename=self.img, imagermode='')
        self.assertEqual(self.res,None)
        imhout=imhead(imagename=self.img+'.image',mode='list')
        imbwrdiff = abs(refimbandw-float(imhout['cdelt4']))/refimbandw
        imfreqrdiff = abs(refimfreq-float(imhout['crval4']))/refimfreq
        maxrdiff = abs(refimmax-float(imhout['datamax']))/refimmax
        # should exactly match for bandwidth and center frequency
        #print "imhout['cdelt4']=", imhout['cdelt4'], " imhout['crval4']=", imhout['crval4'], " imhout['datamax']=",imhout['datamax']
        self.assertTrue(imbwrdiff < 1.0e-9)
        self.assertTrue(imfreqrdiff < 1.0e-9)
        self.assertTrue(maxrdiff < 0.01)
        
    def test_multims4(self):
        '''Clean multims test4: Test multiple MSes input in frequency mode, make a single fat bw image'''
        # expected reference values on r.15199
        refimbandw=1.0e+09
        refimmax=1.19717
        refimfreq=1.5e+09

        datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/clean/'
        self.res=clean(vis=self.msfiles,mode='frequency', spw='', field='',imsize=[200],
                       cell=['4.0arcsec', '4.0arcsec'], imagename=self.img, start='1.5GHz', width='1GHz', nchan=1, imagermode='')
        self.assertEqual(self.res,None)
        imhout=imhead(imagename=self.img+'.image',mode='list')
        imbwrdiff = abs(refimbandw-float(imhout['cdelt4']))/refimbandw
        imfreqrdiff = abs(refimfreq-float(imhout['crval4']))/refimfreq
        maxrdiff = abs(refimmax-float(imhout['datamax']))/refimmax
        # should exactly match for bandwidth and center frequency
        #print "imhout['cdelt4']=", imhout['cdelt4'], " imhout['crval4']=", imhout['crval4'], " imhout['datamax']=",imhout['datamax']
        self.assertTrue(imbwrdiff < 1.0e-9)
        self.assertTrue(imfreqrdiff < 1.0e-9)
        self.assertTrue(maxrdiff < 0.01)

    def test_multims5(self):
        '''Clean multims test5: Test multiple MSes input with a mask in different spectral frame (mfs mode)'''
        # expected reference values on r.16164
        refimbandw=1.0e+09
        refimmax=1.19606661797
        refimfreq=1.49e+09

        datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/clean/'
        if not os.path.exists(self.mask):
            shutil.copytree(datapath+self.mask, self.mask)
        self.res=clean(vis=self.msfiles,mode='mfs', spw='', field='',imsize=[200],
                       cell=['4.0arcsec', '4.0arcsec'], imagename=self.img, mask=self.mask,imagermode='')
        self.assertEqual(self.res,None)
        imhout=imhead(imagename=self.img+'.image',mode='list')
        imbwrdiff = abs(refimbandw-float(imhout['cdelt4']))/refimbandw
        imfreqrdiff = abs(refimfreq-float(imhout['crval4']))/refimfreq
        maxrdiff = abs(refimmax-float(imhout['datamax']))/refimmax
        # 
        shutil.rmtree(self.mask)
        

def suite():
    #return [clean_test1]
    return [clean_test1,clean_multifield_test,clean_multims_test,clean_multiterm_multifield_test]

