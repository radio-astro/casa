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
    boxfilename='cleanhelpertest-sf.box'
    imgname='cleanhelpertest'
    # some other fixed parameters
    imsize=[300,300]
    cell='0.1arcsec'
    stokes='I'
    mode='mfs'
    spw=''
    nchan=1
    start=0
    width=0
    restfreq=''
    field=''
    phasecenter="J2000 17:45:40.0 -29.00.28"
    refpath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/clean/cleanhelper/reference/'
    datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/clean/cleanhelper/'
    outlierfile=datapath+outlierfilename 
    boxfile=datapath+boxfilename
    res = None

    def setUp(self):
        calready=False
        mosweight=False
        if (os.path.exists(self.msfile)):
            os.system('rm -rf ' + self.msfile)
            
        shutil.copytree(self.datapath+self.msfile, self.msfile)
        self.imset = cleanhelper(im, self.msfile, (calready or mosweight))
    
    def tearDown(self):
        if (os.path.exists(self.msfile)):
            os.system('rm -rf ' + self.msfile)

        #os.system('rm -rf ' + self.imgname+'*')
     
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
        return (diff.all()==0) 
 
    def testDefineimages(self):
        """Cleanhelper defineimages test"""
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
        """Cleanhelper definemultimages test"""

        self.run_defineimages()
        self.res=im.summary()
        self.assertTrue(self.res)

    def run_defineimages(self,sf=False):
        rootname=self.imgname
        makepbim=True
        if sf:
          imsizes=self.imsize
          phasecenters=self.phasecenter
          imageids='sf'
        else:
          imsizes, phasecenters, imageids=self.imset.readoutlier(self.outlierfile)
        print "imsizes,imageids=", imsizes,imageids
        self.imset.definemultiimages(rootname=rootname, imsizes=imsizes,
                            cell=self.cell, stokes=self.stokes, mode=self.mode, 
                            spw=self.spw, nchan=self.nchan, start=self.start,
                            width=self.width, restfreq=self.restfreq, 
                            field=self.field,
                            phasecenters=phasecenters,
                            names=imageids, facets=1, makepbim=makepbim)

        
    def testReadoutlier(self):
        """Cleanhelper readoutlier test"""
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
        
    def testMakemultifieldmaskbox(self):
        """Cleanhelper makemultfieldmask2 test: boxes given in argument"""
        self.imset.maskimages={}
        mmask=[[[55,55,65,65],[40,70,50,75]],[20,20,40,40],[5,5,15,10]]
        self.run_defineimages()
        self.imset.makemultifieldmask2(mmask)
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
        """Cleanhelper makemultfieldmask2 test: boxes given as a AIPS boxfile"""
        self.imset.maskimages={}
        self.run_defineimages()
        self.imset.makemultifieldmask2(maskobject=self.outlierfile)
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
        """Cleanhelper makemaskimage test: 2 boxes"""
        self.run_defineimages(sf=True)
        ibmask=[[100,85,120,95],[145,145,155,155]]
        maskimage=self.imset.imagelist[0]+'.mask'
        self.imset.makemaskimage(outputmask=maskimage,imagename=self.imset.imagelist[0],maskobject=ibmask)
        self.assertTrue(os.path.exists(maskimage)," int box maskimage does not exist")
        #retval=self.comparemask(maskimg,self.refpath+'ref-'+maskimg)
        retval=self.comparemask(maskimage, self.refpath+'ref-'+maskimage)
        self.assertTrue(retval,"test on int box mask failed")  
        os.system('rm -rf ' + self.imset.imagelist[0]+'*')
        #
        retval=False
        fibmask=[[100.0,85.0,120.0,95.0],[145,145,155,155]]
        self.imset.makemaskimage(outputmask=maskimage,imagename=self.imset.imagelist[0],maskobject=fibmask)
        self.assertTrue(os.path.exists(maskimage)," float +int box maskimage does not exist")
        retval=self.comparemask(maskimage, self.refpath+'ref-'+maskimage)
        self.assertTrue(retval,"test on float+int boxes failed")  
        os.system('rm -rf ' + self.imset.imagelist[0]+'*')
        #
        retval=False
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
        box1=[np.float_(i) for i in fibmask[0]]
        box2=[np.float_(i) for i in fibmask[1]]
        numpyintmask=[box1,box2]
        self.imset.makemaskimage(outputmask=maskimage,imagename=self.imset.imagelist[0],maskobject=fibmask)
        self.assertTrue(os.path.exists(maskimage)," numpy.float box maskimage does not exist")
        retval=self.comparemask(maskimage, self.refpath+'ref-'+maskimage)
        self.assertTrue(retval,"test on numpy.float boxes failed")
        os.system('rm -rf ' + self.imset.imagelist[0]+'*')
 

    def testMakemaskimageboxfile(self):
        """Cleanhelper makemaskimage test: make mask from boxfile ("worldbox")"""
        self.run_defineimages(sf=True)
        boxfile=self.boxfile
        maskimage=self.imset.imagelist[0]+'.mask'
        self.imset.makemaskimage(outputmask=maskimage,imagename=self.imset.imagelist[0],maskobject=boxfile)
        self.assertTrue(os.path.exists(maskimage)," boxfile  maskimage does not exist")
        #retval=self.comparemask(maskimg,self.refpath+'ref-'+maskimg)
        retval=self.comparemask(maskimage, self.refpath+'ref-'+maskimage)
        self.assertTrue(retval,"test on int box mask failed")
        os.system('rm -rf ' + self.imset.imagelist[0]+'*')
        #

def suite():
    return [cleanhelper_test]
