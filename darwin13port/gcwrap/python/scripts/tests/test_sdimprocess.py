import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest
import sha
import time
import numpy
import re
import string

from sdimprocess import sdimprocess
import asap as sd

#
# Unit test of sdimprocess task.
# 

###
# Base class for sdimprocess unit test
###
class sdimprocess_unittest_base:
    """
    Base class for sdimprocess unit test
    """
    taskname='sdimprocess'
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdimprocess/'
    
    def _checkfile( self, name ):
        isthere=os.path.exists(name)
        self.assertEqual(isthere,True,
                         msg='output file %s was not created because of the task failure'%(name))

    def _checkstats(self,name,ref):
        self._checkfile(name)
        ia.open(name)
        stats=ia.statistics(list=True, verbose=True)
        ia.close()
        for key in stats.keys():
        #for key in self.keys:
            message='statistics \'%s\' does not match'%(key)
            if type(stats[key])==str:
                self.assertEqual(stats[key],ref[key],
                                 msg=message)
            else:
                #print stats[key]-ref[key]
                ret=numpy.allclose(stats[key],ref[key])
                self.assertEqual(ret,True,
                                 msg=message)
                
###
# Test on bad parameter settings
###
class sdimprocess_test0(unittest.TestCase,sdimprocess_unittest_base):
    """
    Test on bad parameter setting
    """
    # Input and output names
    rawfiles=['scan_x.im','scan_y.im']
    prefix=sdimprocess_unittest_base.taskname+'Test0'
    outfile=prefix+'.im'

    def setUp(self):
        self.res=None
        for name in self.rawfiles:
            if (not os.path.exists(name)):
                shutil.copytree(self.datapath+name, name)

        default(sdimprocess)

    def tearDown(self):
        for name in self.rawfiles:
            if (os.path.exists(name)):
                shutil.rmtree(name)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test000(self):
        """Test 000: Default parameters"""
        res=sdimprocess()
        self.assertEqual(res,False)

    def test001(self):
        """Test 001: only 1 image is given for Basket-Weaving"""
        try:
            res=sdimprocess(infiles=[self.rawfiles[0]],mode='basket',direction=[0.])
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('infiles should be a list of input images for Basket-Weaving.')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))        

    def test002(self):
        """Test 002: direction is not given for Basket-Weaving"""
        try:
            res=sdimprocess(infiles=self.rawfiles,mode='basket')
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('direction must have at least two different direction.')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))        

    def test003(self):
        """Test 003: Multiple images are given for Press"""
        try:
            res=sdimprocess(infiles=self.rawfiles,mode='press')
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('infiles allows only one input file for pressed-out method.')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))        

    def test004(self):
        """Test 004: direction is not given for Press"""
        try:
            res=sdimprocess(infiles=[self.rawfiles[0]],mode='press')
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('direction allows only one direction for pressed-out method.')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))        

    def test005(self):
        """Test 005: Existing output image file"""
        shutil.copytree(self.datapath+self.rawfiles[0], self.outfile)
        try:
            res=sdimprocess(infiles=self.rawfiles,mode='basket',direction=[0.,90.0],outfile=self.outfile,overwrite=False)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except StandardError, e:
            pos=str(e).find('%s exists'%(self.outfile))
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))        
        except Exception, e:
            self.assertTrue(False,
                            msg='Unexpected exception was thrown: %s'%(str(e)))        

    def test006(self):
        """Test 006: Zero beamsize for Press"""
        try:
            res=sdimprocess(infiles=[self.rawfiles[0]],mode='press',beamsize=0.0,direction=[0.],outfile=self.outfile,overwrite=True)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except StandardError, e:
            pos=str(e).find('Gaussian2DParam')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))        
        except Exception, e:
            self.assertTrue(False,
                            msg='Unexpected exception was thrown: %s'%(str(e)))        

###
# Test on Pressed method
###
class sdimprocess_test1(unittest.TestCase,sdimprocess_unittest_base):
    """
    Test on Pressed method.

    Test data, scan_x.im, is artificial data, which is 

       - 128x128 in R.A. and Dec.
       - 1 polarization component (Stokes I)
       - 1 spectral channel
       - Gaussian source in the center
       - 1% random noise
       - scanning noise in horizontal direction
       - smoothed by Gaussian kernel of FWHM of 10 pixel
       
    """
    # Input and output names
    rawfile='scan_x.im'
    prefix=sdimprocess_unittest_base.taskname+'Test1'
    outfile=prefix+'.im'
    mode='press'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(sdimprocess)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test100(self):
        """Test 100: Pressed method using whole pixels"""
        res=sdimprocess(infiles=self.rawfile,mode=self.mode,numpoly=2,beamsize=300.0,smoothsize=2.0,direction=0.0,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        refstats={'blc': numpy.array([0, 0, 0, 0], dtype=numpy.int32),
                 'blcf': '00:00:00.000, +00.00.00.000, I, 1.415e+09Hz',
                 'max': numpy.array([ 0.89482009]),
                 'maxpos': numpy.array([63, 63,  0,  0], dtype=numpy.int32),
                 'maxposf': '23:55:47.944, +01.03.00.212, I, 1.415e+09Hz',
                 'mean': numpy.array([ 0.02900992]),
                 'medabsdevmed': numpy.array([ 0.00768787]),
                 'median': numpy.array([ 0.00427032]),
                 'min': numpy.array([-0.02924964]),
                 'minpos': numpy.array([ 79, 115,   0,   0], dtype=numpy.int32),
                 'minposf': '23:54:43.795, +01.55.01.288, I, 1.415e+09Hz',
                 'npts': numpy.array([ 16384.]),
                 'quartile': numpy.array([ 0.01577091]),
                 'rms': numpy.array([ 0.10900906]),
                 'sigma': numpy.array([ 0.10508127]),
                 'sum': numpy.array([ 475.29847136]),
                 'sumsq': numpy.array([ 194.69064919]),
                 'trc': numpy.array([127, 127,   0,   0], dtype=numpy.int32),
                 'trcf': '23:51:31.537, +02.07.01.734, I, 1.415e+09Hz'}
        self._checkstats(self.outfile,refstats)

    def test101(self):
        """Test 101: Pressed method with certain threshold"""
        res=sdimprocess(infiles=self.rawfile,mode=self.mode,numpoly=2,beamsize=300.0,smoothsize=2.0,direction=0.0,tmax=0.5,tmin=-0.1,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        refstats={'blc': numpy.array([0, 0, 0, 0], dtype=numpy.int32),
                 'blcf': '00:00:00.000, +00.00.00.000, I, 1.415e+09Hz',
                 'max': numpy.array([ 0.97879869]),
                 'maxpos': numpy.array([63, 63,  0,  0], dtype=numpy.int32),
                 'maxposf': '23:55:47.944, +01.03.00.212, I, 1.415e+09Hz',
                 'mean': numpy.array([ 0.02900992]),
                 'medabsdevmed': numpy.array([ 0.00766596]),
                 'median': numpy.array([ 0.00407957]),
                 'min': numpy.array([-0.04284666]),
                 'minpos': numpy.array([ 90, 53,   0,   0], dtype=numpy.int32),
                 'minposf': '23:53:59.916, +00.53.00.126, I, 1.415e+09Hz',
                 'npts': numpy.array([ 16384.]),
                 'quartile': numpy.array([ 0.0157865]),
                 'rms': numpy.array([ 0.11193633]),
                 'sigma': numpy.array([ 0.10811512]),
                 'sum': numpy.array([ 475.29845356]),
                 'sumsq': numpy.array([ 205.28728974]),
                 'trc': numpy.array([127, 127,   0,   0], dtype=numpy.int32),
                 'trcf': '23:51:31.537, +02.07.01.734, I, 1.415e+09Hz'}
        self._checkstats(self.outfile,refstats)

###
# Test on FFT based Basket-Weaving
###
class sdimprocess_test2(unittest.TestCase,sdimprocess_unittest_base):
    """
    Test on FFT based Basket-Weaving
    
    Test data, scan_x.im and scan_y.im, are artificial data, which is 

       - 128x128 in R.A. and Dec.
       - 1 polarization component (Stokes I)
       - 1 spectral channel
       - Gaussian source in the center
       - 1% random noise
       - scanning noise in horizontal direction (scan_x.im)
         or vertical direction (scan_y.im)
       - smoothed by Gaussian kernel of FWHM of 10 pixel
       
    """
    # Input and output names
    rawfiles=['scan_x.im','scan_y.im']
    prefix=sdimprocess_unittest_base.taskname+'Test2'
    outfile=prefix+'.im'
    mode='basket'

    def setUp(self):
        self.res=None
        for name in self.rawfiles:
            if (not os.path.exists(name)):
                shutil.copytree(self.datapath+name, name)

        default(sdimprocess)

    def tearDown(self):
        for name in self.rawfiles:
            if (os.path.exists(name)):
                shutil.rmtree(name)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test2000(self):
        """Test 200: FFT based Basket-Weaving using whole pixels"""
        res=sdimprocess(infiles=self.rawfiles,mode=self.mode,direction=[0.0,90.0],masklist=20.0,outfile=self.outfile,overwrite=True)
        refstats={'blc': numpy.array([0, 0, 0, 0], dtype=numpy.int32),
                  'blcf': '00:00:00.000, +00.00.00.000, I, 1.415e+09Hz',
                  'max': numpy.array([ 0.92714936]),
                  'maxpos': numpy.array([64, 64,  0,  0], dtype=numpy.int32),
                  'maxposf': '23:55:43.941, +01.04.00.222, I, 1.415e+09Hz',
                  'mean': numpy.array([ 0.02962625]),
                  'medabsdevmed': numpy.array([ 0.00571492]),
                  'median': numpy.array([ 0.00429045]),
                  'min': numpy.array([-0.02618393]),
                  'minpos': numpy.array([ 56, 107,   0,   0], dtype=numpy.int32),
                  'minposf': '23:56:15.881, +01.47.01.037, I, 1.415e+09Hz',
                  'npts': numpy.array([ 16384.]),
                  'quartile': numpy.array([ 0.01154788]),
                  'rms': numpy.array([ 0.11236797]),
                  'sigma': numpy.array([ 0.1083954]),
                  'sum': numpy.array([ 485.39648429]),
                  'sumsq': numpy.array([ 206.87355986]),
                  'trc': numpy.array([127, 127,   0,   0], dtype=numpy.int32),
                  'trcf': '23:51:31.537, +02.07.01.734, I, 1.415e+09Hz'}
        self._checkstats(self.outfile,refstats)

    def test2001(self):
        """Test 201: FFT based Basket-Weaving with certain threshold"""
        res=sdimprocess(infiles=self.rawfiles,mode=self.mode,direction=[0.0,90.0],masklist=20.0,tmax=0.5,tmin=-0.1,outfile=self.outfile,overwrite=True)
        refstats={'blc': numpy.array([0, 0, 0, 0], dtype=numpy.int32),
                  'blcf': '00:00:00.000, +00.00.00.000, I, 1.415e+09Hz',
                  'max': numpy.array([ 0.99387228]),
                  'maxpos': numpy.array([63, 63,  0,  0], dtype=numpy.int32),
                  'maxposf': '23:55:47.944, +01.03.00.212, I, 1.415e+09Hz',
                  'mean': numpy.array([ 0.02962625]),
                  'medabsdevmed': numpy.array([ 0.00570825]),
                  'median': numpy.array([ 0.00428429]),
                  'min': numpy.array([-0.0260052]),
                  'minpos': numpy.array([ 56, 107,   0,   0], dtype=numpy.int32),
                  'minposf': '23:56:15.881, +01.47.01.037, I, 1.415e+09Hz',
                  'npts': numpy.array([ 16384.]),
                  'quartile': numpy.array([ 0.01155156]),
                  'rms': numpy.array([ 0.1128579]),
                  'sigma': numpy.array([ 0.10890324]),
                  'sum': numpy.array([ 485.39650849]),
                  'sumsq': numpy.array([ 208.68146098]),
                  'trc': numpy.array([127, 127,   0,   0], dtype=numpy.int32),
                  'trcf': '23:51:31.537, +02.07.01.734, I, 1.415e+09Hz'}
        self._checkstats(self.outfile,refstats)


def suite():
    return [sdimprocess_test0,sdimprocess_test1,sdimprocess_test2]
