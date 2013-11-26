import os
import sys
import shutil
import commands
import numpy
from __main__ import default
from tasks import *
from taskinit import *
from cleanhelper import *
import unittest

'''
Unit tests for cleanhelper class members.
'''
class cleanhelper_test(unittest.TestCase):

    # Input and output names
    # use simulated data with 3 point sources
    msfile = 'simptsrcs.ms'
    outlierfilename='outlier.txt'
    newoutlierfilename='newoutlier_v0.txt'
    newoutlierfile2name='newoutlier_manyfields.txt'
    newoutlierreffilename='newoutlier_ref.txt'
    boxfilename='cleanhelpertest-sf.box'
    boxfilename2='boxf.txt'
    rgnfilename='box0.rgn'
    rgntextfilename='regmask.txt'
    imgname='cleanhelpertest'
    # some other fixed parameters
    imsize=[300,300]
    cell='0.1arcsec'
    stokes='I'
    mode='mfs'
    spw=''
    nchan=1
    start=0
    width=1
    restfreq=''
    field=''
    phasecenter="J2000 17:45:40.0 -29.00.28"
    refpath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/clean/cleanhelper/reference/'
    datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/clean/cleanhelper/'
    outlierfile=datapath+outlierfilename 
    newoutlierfile=datapath+newoutlierfilename 
    newoutlierfile2=datapath+newoutlierfile2name 
    newoutlierreffile=refpath+newoutlierreffilename 
    boxfile=datapath+boxfilename
    boxfile2=datapath+boxfilename2
    rgnfile=datapath+rgnfilename
    rgntextfile=datapath+rgntextfilename
    res = None

    def setUp(self):
        usescratch=False
        mosweight=False
        if (os.path.exists(self.msfile)):
            os.system('rm -rf ' + self.msfile)
  
        shutil.copytree(self.datapath+self.msfile, self.msfile)

        self.imset = cleanhelper(im, self.msfile, (usescratch or mosweight))
        self.imset.datsel(field=self.field, spw=self.spw, nchan=self.nchan,
                         start=self.start, width=self.width,
                         timerange='', uvrange='', antenna='', scan='', 
                         observation='',intent='', usescratch=usescratch)


    def tearDown(self):
        if (os.path.exists(self.msfile)):
            os.system('rm -rf ' + self.msfile)

        im.close()
        os.system('rm -rf ' + self.imgname+'*')
     
    def getpixval(self,img,pixel):
        ia.open(img)
        px = ia.pixelvalue(pixel)
        ia.close()
        return px['value']['value']

    def comparemask(self,maskimage,refmask):
        """
        compare the input image with reference image
        return true if pix values are identical

        """
        ia.open(maskimage)
        maskvals = ia.getchunk()
        ia.close()
        ia.open(refmask)
        refvals= ia.getchunk()
        ia.close()
        diff = refvals - maskvals
        #print "image diff by pix values=", diff
        return (numpy.all(diff==0)) 
 
    def testDefineimages(self):
        """[Cleanhelper defineimages test]"""
        self.imset.defineimages(imsize=self.imsize, cell=self.cell, 
                       stokes=self.stokes, mode=self.mode, 
                       spw=self.spw, nchan=self.nchan,
                       start=self.start,  width=self.width,
                       restfreq=self.restfreq, field=self.field,
                       phasecenter=self.phasecenter)
        # need to have better checking....
        self.res=im.summary()
        self.assertTrue(self.res)

    def testDefinemultimages(self):
        """[Cleanhelper definemultimages test]"""

        self.run_defineimages()
        self.res=im.summary()
        self.assertTrue(self.res)

    def run_defineimages(self,sf=False, makelargeim=False):
        rootname=self.imgname
        makepbim=True
        if sf:
          if makelargeim:
              imsizes=[1000,1000]
          else:
              imsizes=self.imsize
          phasecenters=self.phasecenter
          imageids='sf'
        else:
          imsizes, phasecenters, imageids=self.imset.readoutlier(self.outlierfile)
        self.imset.definemultiimages(rootname=rootname, imsizes=imsizes,
                            cell=self.cell, stokes=self.stokes, mode=self.mode, 
                            spw=self.spw, nchan=self.nchan, start=self.start,
                            width=self.width, restfreq=self.restfreq, 
                            field=self.field,
                            phasecenters=phasecenters,
                            names=imageids, facets=1, makepbim=makepbim)

        
    def testReadoutlier(self):
        """[Cleanhelper readoutlier test (reader for older format)]"""
        imsizes, phasecenters, imageids=self.imset.readoutlier(self.outlierfile)
        f = open(self.outlierfile,'r')
        lines=f.readlines()
        f.close()
        cnt = 0
        for elm in lines:
          if elm.upper().rfind('C') ==0:
            cnt +=1
        if len(imsizes) == len(phasecenters) == len(imageids) ==cnt:
          self.res=True
        else:
          self.res=False
        self.assertTrue(self.res) 
       
    def testNewreadoutlier(self):
        """[Cleanhelper newreadoutlier test (reader for new outlier file format with irregular line breaks)]"""
        imageids, imsizes, phasecenters, masks, models, paramdic, newformat=self.imset.newreadoutlier(self.newoutlierfile)
        # the reference outlier file contains each field's paramaters per line
        print "Using ", self.newoutlierfile
        f = open(self.newoutlierreffile,'r')
        lines=f.readlines()
        f.close()
        cnt = 0
        for elm in lines:
          if len(elm.split())!=0 and elm.split()[0]!='#' :
            cnt +=1
        print "N fields=",cnt
        #print "imsizes=",imsizes," phasecenters=",phasecenters
        print "len(imsizes)=",len(imsizes), " len(imageids)=",len(imageids)
        if len(imsizes) == len(phasecenters) == len(imageids) == len(masks) == len(models) == cnt:
          self.res=True
        else:
          self.res=False
        self.assertTrue(self.res)

    def testNewreadoutlier2(self):
        """[Cleanhelper newreadoutlier test2 (reader for new outlier file format with a large number (129) of outliers)]"""
        imageids, imsizes, phasecenters, masks, models, paramdic, newformat=self.imset.newreadoutlier(self.newoutlierfile2)
        print "Using ", self.newoutlierfile2
        noutliers=129
        #print "imsizes=",imsizes," phasecenters=",phasecenters
        print "len(imsizes)=",len(imsizes), " len(imageids)=",len(imageids)
        if len(imsizes) == len(phasecenters) == len(imageids) == len(masks) == len(models) == noutliers:
          self.res=True
        else:
          self.res=False
        self.assertTrue(self.res)
 
    def testMakemultifieldmaskbox(self):
        """[Cleanhelper (new)makemultfieldmask2 test: boxes given in argument]"""
        # makemultifieldmask3 was renamed to makemultfieldmask2 TT-Dec.22,2011
        self.imset.maskimages={}
        mmask=[[[55,55,65,65],[40,70,50,75]],[20,20,40,40],[5,5,15,10]]
        self.run_defineimages()
        self.imset.makemultifieldmask2(mmask)
        #self.imset.makemultifieldmask3(mmask)
        for imgroot,maskimg in self.imset.maskimages.iteritems():
          self.assertTrue(os.path.exists(maskimg))
          retval=self.comparemask(maskimg, self.refpath+'ref-'+maskimg)
          if retval:
            self.res=True
          else:
            self.res=False
          self.assertTrue(self.res)  
          os.system('rm -rf ' + maskimg)

    def testMakemultifieldmaskboxfile(self):
        """[Cleanhelper (new)makemultfieldmask2 test: boxes given as a AIPS boxfile]"""
        self.imset.maskimages={}
        self.run_defineimages()
        #self.imset.makemultifieldmask3(maskobject=self.outlierfile,newformat=False)
        self.imset.makemultifieldmask2(maskobject=self.outlierfile,newformat=False)
        for imgroot,maskimg in self.imset.maskimages.iteritems():
          self.assertTrue(os.path.exists(maskimg))
          retval=self.comparemask(maskimg,self.refpath+'ref-'+maskimg)
          if retval:
            self.res=True
          else:
            self.res=False
          self.assertTrue(self.res)  
          os.system('rm -rf ' + maskimg)

    def testMakemaskimagebox(self):
        """[Cleanhelper makemaskimage test: 2 boxes]"""
        self.run_defineimages(sf=True)
        print "int boxes test"
        ibmask=[[100,85,120,95],[145,145,155,155]]
        maskimage=self.imset.imagelist[0]+'.0.mask'
        self.imset.makemaskimage(outputmask=maskimage,imagename=self.imset.imagelist[0],maskobject=ibmask)
        self.assertTrue(os.path.exists(maskimage)," int box maskimage does not exist")
        #retval=self.comparemask(maskimg,self.refpath+'ref-'+maskimg)
        retval=self.comparemask(maskimage, self.refpath+'ref-'+maskimage)
        self.assertTrue(retval,"test on int box mask failed")  
        os.system('rm -rf ' + self.imset.imagelist[0]+'*')
        #
        retval=False
        print "float box and int box test"
        fibmask=[[100.0,85.0,120.0,95.0],[145,145,155,155]]
        self.imset.makemaskimage(outputmask=maskimage,imagename=self.imset.imagelist[0],maskobject=fibmask)
        self.assertTrue(os.path.exists(maskimage)," float +int box maskimage does not exist")
        retval=self.comparemask(maskimage, self.refpath+'ref-'+maskimage)
        self.assertTrue(retval,"test on float+int boxes failed")  
        os.system('rm -rf ' + self.imset.imagelist[0]+'*')
        #
        retval=False
        print "numpy.int boxes test"
        import numpy as np
        box1=[np.int_(i) for i in ibmask[0]] 
        box2=[np.int_(i) for i in ibmask[1]] 
        numpyintmask=[box1,box2]
        self.imset.makemaskimage(outputmask=maskimage,imagename=self.imset.imagelist[0],maskobject=fibmask)
        self.assertTrue(os.path.exists(maskimage)," numpy.int box maskimage does not exist")
        retval=self.comparemask(maskimage, self.refpath+'ref-'+maskimage)
        self.assertTrue(retval,"test on numpy.int boxes failed")
        os.system('rm -rf ' + self.imset.imagelist[0]+'*')
        #
        retval=False
        print "numpy.float boxes test"
        box1=[np.float_(i) for i in fibmask[0]]
        box2=[np.float_(i) for i in fibmask[1]]
        numpyintmask=[box1,box2]
        self.imset.makemaskimage(outputmask=maskimage,imagename=self.imset.imagelist[0],maskobject=fibmask)
        self.assertTrue(os.path.exists(maskimage)," numpy.float box maskimage does not exist")
        retval=self.comparemask(maskimage, self.refpath+'ref-'+maskimage)
        self.assertTrue(retval,"test on numpy.float boxes failed")
        os.system('rm -rf ' + self.imset.imagelist[0]+'*')
 
        
    def testMakemaskimageboxfile(self):
        """[Cleanhelper makemaskimage test: make mask from boxfile ("worldbox")]"""
        self.run_defineimages(sf=True)
        boxfile=self.boxfile
        maskimage=self.imset.imagelist[0]+'.mask'
        self.imset.makemaskimage(outputmask=maskimage,imagename=self.imset.imagelist[0],maskobject=boxfile)
        self.assertTrue(os.path.exists(maskimage)," boxfile  maskimage does not exist")
        retval=self.comparemask(maskimage, self.refpath+'ref-'+maskimage)
        self.assertTrue(retval,"test on box mask failed")
        os.system('rm -rf ' + self.imset.imagelist[0]+'*')

    def testMakemaskimagemixed(self):
        """[Mixed input to mask parameter (single field)] """
        # there is descrepancy in circle mask created by 32bit anc 64bit machine
        # if the circle is relatively small ....(the mask made by 32bit machine
        # miss 1 or 2 pixels at some edges), so make a image size bigger for this test... 
        self.run_defineimages(sf=True,makelargeim=True)
        maskimage=self.imset.imagelist[0]+'_mixed.mask'
        masks = ['circle [ [ 220pix , 650pix] ,100pix ]', self.boxfile2, self.rgnfile,self.rgntextfile]
        self.imset.makemaskimage(outputmask=maskimage,imagename=self.imset.imagelist[0],maskobject=masks)
        self.assertTrue(os.path.exists(maskimage)," boxfile  maskimage does not exist")
        retval=self.comparemask(maskimage, self.refpath+'ref-'+maskimage)
        self.assertTrue(retval,"test on box mask failed")
        os.system('rm -rf ' + self.imset.imagelist[0]+'*')

    def testGetOptimumSize(self):
        import random
        import time
        # Check that factors of 2,3, and 5 do the right thing
        self.assertEqual(cleanhelper.getOptimumSize(1024),1024)    #2^10
        self.assertEqual(cleanhelper.getOptimumSize(59049), 60000) #2^5*3*5^4
        self.assertEqual(cleanhelper.getOptimumSize(15625), 15680) #2^6*5*7^2

        # Now lets do some random checks to make sure we get the same value
        self.assertEqual(cleanhelper.getOptimumSize(1375),1400) #2^3*5^2*7 
        self.assertEqual(cleanhelper.getOptimumSize(62354),62500) #2^2*5^6
        self.assertEqual(cleanhelper.getOptimumSize(981),1000) # 2^3*5^3
        self.assertEqual(cleanhelper.getOptimumSize(8123), 8192) # 2^13
        self.assertEqual(cleanhelper.getOptimumSize(82863),82944) # 2^10*3^4

        # Now do some real random checks
        random.seed(time.time())
        factorList = [2,3,5,7]

        for i in xrange(100):
            x = random.randint(100,1000000)
            y = cleanhelper.getOptimumSize(x)
            self.assertTrue(y>=x,"input=%s output=%s" % (x,y))
            
            # Now Factorize this return
            remainder = float(y)
            expList = [0,0,0,0]
            for idx in range(len(factorList)):
                while remainder > 0 and \
                        (remainder/factorList[idx] == int(remainder/factorList[idx])):
                    expList[idx] += 1
                    remainder /= factorList[idx]

            # Check that the number is completely factored
            value = 1
            for idx in range(len(factorList)):
                value *= factorList[idx]**expList[idx]
            self.assertEqual(y,value)
            #self.assertTrue(expList.count(0) > 0)
            self.assertTrue(expList.count(0) < len(expList))
            


def suite():
    return [cleanhelper_test]

if __name__ == '__main__':
    testSuite = []
    for testClass in suite():
        testSuite.append(unittest.makeSuite(testClass,'test'))
    allTests = unittest.TestSuite(testSuite)
    unittest.TextTestRunner(verbosity=2).run(allTests)
