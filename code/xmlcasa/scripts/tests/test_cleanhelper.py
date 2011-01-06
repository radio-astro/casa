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
        ia.open(maskimage)
        maskvals = ia.getchunk()
        ia.close()
        ia.open(refmask)
        refvals= ia.getchunk()
        ia.close()
        diff = maskvals - refvals
        return diff.sum()
 
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

    def run_defineimages(self):
        rootname=self.imgname
        makepbim=True
        imsizes, phasecenters, imageids=self.imset.readoutlier(self.outlierfile)
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
          if retval > 0.0:
            self.res=False
          else:
            self.res=True
          self.assertTrue(self.res)  
          os.system('rm -rf ' + maskimg)

    def testMakemultifieldmaskboxfile(self):
        """Cleanhelper makemultfieldmask2 test: boxes given as a boxfile"""
        self.imset.maskimages={}
        self.run_defineimages()
        self.imset.makemultifieldmask2(maskobject=self.outlierfile)
        for imgroot,maskimg in self.imset.maskimages.iteritems():
          self.assertTrue(os.path.exists(maskimg))
          retval=self.comparemask(self.refpath+'ref-'+maskimg,maskimg)
          if retval > 0.0:
            self.res=False
          else:
            self.res=True
          self.assertTrue(self.res)  
          os.system('rm -rf ' + maskimg)

def suite():
    return [cleanhelper_test]
