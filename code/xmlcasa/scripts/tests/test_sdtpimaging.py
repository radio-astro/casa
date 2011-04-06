import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
from asap_init import * 
import unittest
import sha
import time
import numpy

asap_init()
from sdtpimaging import sdtpimaging
import asap as sd

# Unit test of sdtpimaging task.


###
# Test on bad parameter settings, data selection, ...
###
class sdtpimaging_test0(unittest.TestCase):
    """
    Test on bad parameter settings
    """
    # Input and output names
    sdfile='tpimaging.ms'
    prefix='sdtpimaging.Test0'
    outfile=prefix+'.ms'
    outimage=prefix+'.im'
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdtpimaging/'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.sdfile)):
            shutil.copytree(self.datapath+self.sdfile, self.sdfile)

        default(sdtpimaging)

    def tearDown(self):
        if (os.path.exists(self.sdfile)):
            shutil.rmtree(self.sdfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test000(self):
        """Test 000: Default parameters"""
        self.res=sdtpimaging()
        self.assertFalse(self.res)
        
    def test001(self):
        """Test 001: Bad antenna id"""
        self.res=sdtpimaging(sdfile=self.sdfile,antenna='99')
        self.assertFalse(self.res)        

    def test002(self):
        """Test 002: Bad stokes string"""
        self.res=sdtpimaging(sdfile=self.sdfile,stokes='J')
        self.assertFalse(self.res)
        
    def test003(self):
        """Test 003: Try to create image without output image name"""
        self.res=sdtpimaging(sdfile=self.sdfile,createimage=True,imagename='')
        self.assertFalse(self.res)

    def test004(self):
        """Test 004: Negative imsize"""
        self.res=sdtpimaging(sdfile=self.sdfile,createimage=True,imagename=self.outimage,imsize=[-1])
        self.assertFalse(self.res)

    def test005(self):
        """Test 005: Negative cell size"""
        self.res=sdtpimaging(sdfile=self.sdfile,createimage=True,imagename=self.outimage,cell=[-1])
        self.assertFalse(self.res)

    def test006(self):
        """Test 006: Bad phase center string"""
        self.res=sdtpimaging(sdfile=self.sdfile,createimage=True,imagename=self.outimage,phasecenter='XXX')
        self.assertFalse(self.res)

    def test007(self):
        """Test 007: Bad pointing column name"""
        self.res=sdtpimaging(sdfile=self.sdfile,createimage=True,imagename=self.outimage,pointingcolumn='XXX')
        self.assertFalse(self.res)

    def test008(self):
        """Test 008: Unexisting grid function"""
        self.res=sdtpimaging(sdfile=self.sdfile,createimage=True,imagename=self.outimage,gridfunction='XXX')
        self.assertFalse(self.res)
 
    def test009(self):
        """Test 009: Invalid calmode"""
        self.res=sdtpimaging(sdfile=self.sdfile,calmode='ps')
        self.assertFalse(self.res)
 

###
# Test to image data without spatial baseline subtraction
###
class sdtpimaging_test1(unittest.TestCase):
    """
    Test to image data without spatial baseline subtraction
    """
    # Input and output names
    sdfile='tpimaging.ms'
    prefix='sdtpimaging.Test1'
    outfile=prefix+'.ms'
    outimage=prefix+'.im'
    refimage='nobaseline.im'
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdtpimaging/'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.sdfile)):
            shutil.copytree(self.datapath+self.sdfile, self.sdfile)
        if (not os.path.exists(self.refimage)):
            shutil.copytree(self.datapath+self.refimage, self.refimage)

        default(sdtpimaging)

    def tearDown(self):
        if (os.path.exists(self.sdfile)):
            shutil.rmtree(self.sdfile)
        if (os.path.exists(self.refimage)):
            shutil.rmtree(self.refimage)
        os.system( 'rm -rf '+self.prefix+'*' )
        os.system( 'rm -rf '+self.sdfile+'*' )

    def _compare(self):
        default(imval)
        refval=imval(imagename=self.refimage,box='-1,-1',stokes='XX')
        val=imval(imagename=self.outimage,box='-1,-1',stokes='XX')
        refdata=refval['data']
        data=val['data']
        diff=refdata-data
        maxdiff=abs(diff).max()/data.max()
        casalog.post( 'maxdiff=%s'%maxdiff )
        return maxdiff

    def test100(self):
        """Test 100: test to image data without spatial baseline subtraction"""
        self.res=sdtpimaging(sdfile=self.sdfile,calmode='none',stokes='XX',createimage=True,imagename=self.outimage,imsize=[64],cell=['15arcsec'],phasecenter='J2000 05h35m07s -5d21m00s',pointingcolumn='direction',gridfunction='SF')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare() < 0.001)
        
###
# Test to image data with spatial baseline subtraction
###
class sdtpimaging_test2(unittest.TestCase):
    """
    Test to image data with spatial baseline subtraction
    """
    # Input and output names
    sdfile='tpimaging.ms'
    prefix='sdtpimaging.Test2'
    outfile=prefix+'.ms'
    outimage=prefix+'.im'
    refimage='dobaseline.im'
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdtpimaging/'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.sdfile)):
            shutil.copytree(self.datapath+self.sdfile, self.sdfile)
        if (not os.path.exists(self.refimage)):
            shutil.copytree(self.datapath+self.refimage, self.refimage)

        default(sdtpimaging)

    def tearDown(self):
        if (os.path.exists(self.sdfile)):
            shutil.rmtree(self.sdfile)
        if (os.path.exists(self.refimage)):
            shutil.rmtree(self.refimage)
        os.system( 'rm -rf '+self.prefix+'*' )
        os.system( 'rm -rf '+self.sdfile+'*' )

    def _compare(self):
        default(imval)
        refval=imval(imagename=self.refimage,box='-1,-1',stokes='XX')
        val=imval(imagename=self.outimage,box='-1,-1',stokes='XX')
        refdata=refval['data']
        data=val['data']
        diff=refdata-data
        casalog.post( 'diff.shape=%s'%(list(diff.shape)) )
        maxdiff=abs(diff).max()/data.max()
        casalog.post( 'maxdiff=%s'%maxdiff )
        return maxdiff

    def test200(self):
        """Test 200: test to image data with spatial baseline subtraction"""
        self.res=sdtpimaging(sdfile=self.sdfile,calmode='baseline',masklist=[10,10],blpoly=1,stokes='XX',createimage=True,imagename=self.outimage,imsize=[64],cell=['15arcsec'],phasecenter='J2000 05h35m07s -5d21m00s',pointingcolumn='direction',gridfunction='SF')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare() < 0.001)


def suite():
    return [sdtpimaging_test0,sdtpimaging_test1,sdtpimaging_test2]
