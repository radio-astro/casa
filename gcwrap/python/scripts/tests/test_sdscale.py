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

from sdscale import sdscale
import asap as sd

#
# Unit test of sdscale task.
# 

###
# Base class for sdscale unit test
###
class sdscale_unittest_base:
    """
    Base class for sdscale unit test
    """
    taskname='sdscale'
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdscale/'
    
    def _checkfile( self, name ):
        isthere=os.path.exists(name)
        self.assertEqual(isthere,True,
                         msg='output file %s was not created because of the task failure'%(name))

    def _compare( self, name, ref, factor, scaletsys ):
        self._checkfile( name )
        # get original nchan and nrow
        tb.open(ref)
        nrow0=tb.nrows()
        rspchans=[]
        rtsyschans=[]
        for irow in xrange(nrow0):
            rspchans.append(len(tb.getcell('SPECTRA',irow)))
            rtsyschans.append(len(tb.getcell('TSYS',irow)))
        tb.close()
        # check shape
        tb.open(name)
        nrow=tb.nrows()
        self.assertEqual(nrow,2,
                         msg='number of rows mismatch')
        sp=[]
        tsys=[]
        for irow in xrange(nrow):
            sp.append(tb.getcell('SPECTRA',irow))
            tsys.append(tb.getcell('TSYS',irow))
            self.assertEqual(len(sp[irow]),rspchans[irow],
                             msg='SPECTRA: number of channel mismatch in row%s'%(irow)) 
            self.assertEqual(len(tsys[irow]),rtsyschans[irow],
                             msg='TSYS: number of channel mismatch in row%s'%(irow))
        tb.close()
        # check data
        valuetype=type(sp[0][0])
        if type(factor) is not list:
            # scalar factor
            for irow in xrange(nrow):
                arrs = numpy.ones(rspchans[irow],dtype=valuetype)*factor
                ret=numpy.allclose(arrs,sp[irow])
                self.assertEqual(ret,True,
                                 msg='SPECTRA: data differ in row%s'%(irow))
                arrt = numpy.ones(rtsyschans[irow],dtype=valuetype)
                if scaletsys:
                    arrt *= factor
                ret=numpy.allclose(arrt,tsys[irow])
                self.assertEqual(ret,True,
                                 msg='TSYS: data differ in row%s'%(irow))
        elif type(factor[0]) is not list:
            # 1D array factor
            factor = numpy.array(factor)
            for irow in xrange(nrow):
                ret=numpy.allclose(factor,sp[irow])
                self.assertEqual(ret,True,
                                 msg='SPECTRA: data differ in row%s'%(irow))
                arrt = numpy.ones(rtsyschans[irow],dtype=valuetype)
                if scaletsys:
                    arrt *= factor
                ret=numpy.allclose(arrt,tsys[irow])
                self.assertEqual(ret,True,
                                 msg='TSYS: data differ in row%s'%(irow))
        elif len(factor[0]) == 1:
            # 2D array with shape [nrow,1]
            for irow in xrange(nrow):
                arrs = numpy.ones(rspchans[irow],dtype=valuetype)*factor[irow]
                ret=numpy.allclose(arrs,sp[irow])
                self.assertEqual(ret,True,
                                 msg='SPECTRA: data differ in row%s'%(irow))
                arrt = numpy.ones(rtsyschans[irow],dtype=valuetype)
                if scaletsys:
                    arrt *= factor[irow]
                ret=numpy.allclose(arrt,tsys[irow])
                self.assertEqual(ret,True,
                                 msg='TSYS: data differ in row%s'%(irow))
        else:
            # 2D array with shape [nrow,nchan]
            for irow in xrange(nrow):
                arrs = numpy.array(factor[irow])
                ret=numpy.allclose(arrs,sp[irow])
                self.assertEqual(ret,True,
                                 msg='SPECTRA: data differ in row%s'%(irow))
                arrt = numpy.ones(rtsyschans[irow],dtype=valuetype)
                if scaletsys:
                    arrt *= numpy.array(factor[irow])
                ret=numpy.allclose(arrt,tsys[irow])
                self.assertEqual(ret,True,
                                 msg='TSYS: data differ in row%s'%(irow))

                
###
# Test on bad parameter settings
###
class sdscale_test0(unittest.TestCase,sdscale_unittest_base):
    """
    Test on bad parameter setting

    Test data, sdscale1.asap, is artificial data with the following
    status:

       - nrow = 2
       - nchan = 4 for spectral data
       - 1 Tsys value for each spectrum
       - all spectral values are 1.0
       - all Tsys values are 1.0
       
    """
    # Input and output names
    rawfile='sdscale1.asap'
    prefix=sdscale_unittest_base.taskname+'Test0'
    outfile=prefix+'.asap'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(sdscale)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test000(self):
        """Test 000: Default parameters"""
        # argument verification error
        res=sdscale()
        self.assertFalse(res)        

    def test001(self):
        """Test 001: Existing outfile with overwrite=False"""
        os.system('cp -r %s %s'%(self.rawfile,self.outfile))
        try:
            res=sdscale(infile=self.rawfile,factor=2.0,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Output file \'%s\' exists.'%(self.outfile))
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test002(self):
        """Test 002: Bad shaped factor"""
        factor = [2.0,3.0]
        try:
            res=sdscale(infile=self.rawfile,factor=factor,scaletsys=False,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Vector size must be 1 or be same as number of channel.')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))
        
    def test003(self):
        """Test 003: Try to scale non-conform Tsys"""
        factor=[1.0,2.0,3.0,4.0]
        try:
            res=sdscale(infile=self.rawfile,factor=factor,scaletsys=True,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('SPECTRA and TSYS must conform in shape if you want to apply operation on Tsys.')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

###
# Test on scaling 1
###
class sdscale_test1(unittest.TestCase,sdscale_unittest_base):
    """
    Test on actual scaling

    Test data, sdscale0.asap, is artificial data with the following
    status:

       - nrow = 2
       - nchan = 4
       - all spectral values are 1.0
       - all Tsys values are 1.0

    scaletsys=True should be working.
       
    """
    # Input and output names
    rawfile='sdscale0.asap'
    prefix=sdscale_unittest_base.taskname+'Test1'
    outfile=prefix+'.asap'
    
    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(sdscale)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test100(self):
        """Test 100: scalar factor with Tsys scaling"""
        factor = 2.0
        scaletsys=True
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)

    def test101(self):
        """Test 101: scalar factor without Tsys scaling"""
        factor = 2.0
        scaletsys=False
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)

    def test102(self):
        """Test 102: 1D array factor with Tsys scaling"""
        factor = [2.0,3.0,4.0,5.0]
        scaletsys=True
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)

    def test103(self):
        """Test 103: 1D array factor without Tsys scaling"""
        factor = [2.0,3.0,4.0,5.0]
        scaletsys=False
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)

    def test104(self):
        """Test 104: 2D array ([nrow,1]) factor with Tsys scaling"""
        factor = [[2.0],[3.0]]
        scaletsys=True
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)

    def test105(self):
        """Test 105: 2D array ([nrow,1]) factor without Tsys scaling"""
        factor = [[2.0],[3.0]]
        scaletsys=False
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)

    def test106(self):
        """Test 106: 2D array ([nrow,nchan]) factor with Tsys scaling"""
        factor = [[2.0,4.0,6.0,8.0],[3.0,5.0,7.0,9.0]]
        scaletsys=True
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)

    def test107(self):
        """Test 107: 2D array ([nrow,nchan]) factor without Tsys scaling"""
        factor = [[2.0,4.0,6.0,8.0],[3.0,5.0,7.0,9.0]]
        scaletsys=False
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)

###
# Test on scaling 2
###
class sdscale_test2(unittest.TestCase,sdscale_unittest_base):
    """
    Test on actual scaling

    Test data, sdscale1.asap, is artificial data with the following
    status:

       - nrow = 2
       - nchan = 4 
       - 1 Tsys value for each spectrum
       - all spectral values are 1.0
       - all Tsys values are 1.0

    scaletsys=True should be working.
       
    """
    # Input and output names
    rawfile='sdscale1.asap'
    prefix=sdscale_unittest_base.taskname+'Test2'
    outfile=prefix+'.asap'
    
    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(sdscale)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test200(self):
        """Test 200: scalar factor with Tsys scaling"""
        factor = 2.0
        scaletsys=True
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)

    def test201(self):
        """Test 201: scalar factor without Tsys scaling"""
        factor = 2.0
        scaletsys=False
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)
        
    def test202(self):
        """Test 202: 1D array factor with Tsys scaling (must fail)"""
        factor = [2.0,3.0,4.0,5.0]
        scaletsys=True
        try:
            res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('SPECTRA and TSYS must conform in shape if you want to apply operation on Tsys.')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test203(self):
        """Test 203: 1D array factor without Tsys scaling"""
        factor = [2.0,3.0,4.0,5.0]
        scaletsys=False
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)

    def test204(self):
        """Test 204: 2D array ([nrow,1]) factor with Tsys scaling"""
        factor = [[2.0],[3.0]]
        scaletsys=True
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)

    def test205(self):
        """Test 205: 2D array ([nrow,1]) factor without Tsys scaling"""
        factor = [[2.0],[3.0]]
        scaletsys=False
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)

    def test206(self):
        """Test 206: 2D array ([nrow,nchan]) factor with Tsys scaling (must fail)"""
        factor = [[2.0,4.0,6.0,8.0],[3.0,5.0,7.0,9.0]]
        scaletsys=True
        try:
            res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('SPECTRA and TSYS must conform in shape if you want to apply operation on Tsys.')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test207(self):
        """Test 207: 2D array ([nrow,nchan]) factor without Tsys scaling"""
        factor = [[2.0,4.0,6.0,8.0],[3.0,5.0,7.0,9.0]]
        scaletsys=False
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)

###
# Test on scaling 3
###
class sdscale_test3(unittest.TestCase,sdscale_unittest_base):
    """
    Test on actual scaling

    Test data, sdscale2.asap, is artificial data with the following
    status:

       - nrow = 2
       - nchan = 4 in row0, 1 in row1
       - all spectral values are 1.0
       - all Tsys values are 1.0

    scaletsys=True should be working.
       
    """
    # Input and output names
    rawfile='sdscale2.asap'
    prefix=sdscale_unittest_base.taskname+'Test3'
    outfile=prefix+'.asap'
    
    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(sdscale)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test300(self):
        """Test 300: scalar factor with Tsys scaling"""
        factor = 2.0
        scaletsys=True
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)

    def test301(self):
        """Test 301: scalar factor without Tsys scaling"""
        factor = 2.0
        scaletsys=False
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)
        
    def test302(self):
        """Test 302: 1D array factor with Tsys scaling (must fail)"""
        factor = [2.0,3.0,4.0,5.0]
        scaletsys=True
        try:
            res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('ArrayColumn::getColumn cannot be done for column SPECTRA; the array shapes vary: Table array conformance error')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test303(self):
        """Test 303: 1D array factor without Tsys scaling (must fail)"""
        factor = [2.0,3.0,4.0,5.0]
        scaletsys=False
        try:
            res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('All spectra in the input scantable must have the same number of channel for vector operation.')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test304(self):
        """Test 304: 2D array ([nrow,1]) factor with Tsys scaling"""
        factor = [[2.0],[3.0]]
        scaletsys=True
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)

    def test305(self):
        """Test 305: 2D array ([nrow,1]) factor without Tsys scaling"""
        factor = [[2.0],[3.0]]
        scaletsys=False
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)

    def test306(self):
        """Test 306: 2D array ([nrow,nchan]) factor with Tsys scaling"""
        factor = [[2.0,4.0,6.0,8.0],[3.0]]
        scaletsys=True
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)

    def test307(self):
        """Test 307: 2D array ([nrow,nchan]) factor without Tsys scaling"""
        factor = [[2.0,4.0,6.0,8.0],[3.0]]
        scaletsys=False
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)

###
# Test on scaling 4
###
class sdscale_test4(unittest.TestCase,sdscale_unittest_base):
    """
    Test on actual scaling

    Test data, sdscale3.asap, is artificial data with the following
    status:

       - nrow = 2
       - nchan = 4 in row0, 1 in row1
       - 1 Tsys value for each spectrum
       - all spectral values are 1.0
       - all Tsys values are 1.0

    scaletsys=True should be working.
       
    """
    # Input and output names
    rawfile='sdscale3.asap'
    prefix=sdscale_unittest_base.taskname+'Test4'
    outfile=prefix+'.asap'
    
    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(sdscale)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test400(self):
        """Test 400: scalar factor with Tsys scaling"""
        factor = 2.0
        scaletsys=True
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)

    def test401(self):
        """Test 401: scalar factor without Tsys scaling"""
        factor = 2.0
        scaletsys=False
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)

    def test402(self):
        """Test 402: 1D array factor with Tsys scaling (must fail)"""
        factor = [2.0,3.0,4.0,5.0]
        scaletsys=True
        try:
            res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('ArrayColumn::getColumn cannot be done for column SPECTRA; the array shapes vary: Table array conformance error')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test403(self):
        """Test 403: 1D array factor without Tsys scaling (must fail)"""
        factor = [2.0,3.0,4.0,5.0]
        scaletsys=False
        try:
            res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('All spectra in the input scantable must have the same number of channel for vector operation.')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test404(self):
        """Test 404: 2D array ([nrow,1]) factor with Tsys scaling"""
        factor = [[2.0],[3.0]]
        scaletsys=True
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)

    def test405(self):
        """Test 405: 2D array ([nrow,1]) factor without Tsys scaling"""
        factor = [[2.0],[3.0]]
        scaletsys=False
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)

    def test406(self):
        """Test 406: 2D array ([nrow,nchan]) factor with Tsys scaling (must fail)"""
        factor = [[2.0,4.0,6.0,8.0],[3.0]]
        scaletsys=True
        try:
            res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('SPECTRA and TSYS must conform in shape if you want to apply operation on Tsys.')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))            

    def test407(self):
        """Test 407: 2D array ([nrow,nchan]) factor without Tsys scaling"""
        factor = [[2.0,4.0,6.0,8.0],[3.0]]
        scaletsys=False
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')
        self._compare(self.outfile,self.rawfile,factor,scaletsys)

###
# Test on flag
###
class sdscale_testflag(unittest.TestCase,sdscale_unittest_base):
    """
    Test on flag information handling

    Test data, sdscale_flagtest.asap, is artificial data with the following
    status:

       - nrow = 4
       - nchan = 100
       - rows #0 and #1 are row-flagged
       - channels 5 to 9, 15 to 19, and 94 to 98 are flagged in rows #1 and #2
       - all spectral values are 1.0
       - all tsys values are 1.0

    if a spectrum is row-flagged, scaling must not be applied on the spectrum.
    as for spectra which are not row-flagged, scaling must be applied to the 
    spectra/tsys values for all channels regardless of channel-flag values. 
    """
    # Input and output names
    rawfile='sdscale_flagtest.asap'
    prefix=sdscale_unittest_base.taskname+'TestFlag'
    outfile=prefix+'.asap'
    flagged_chan_list = [[5,9],[15,19],[94,98]]
    
    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(sdscale)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def testflag(self):
        """Testflag: verify proper handling of flag information"""
        factor = 2.0
        scaletsys=True
        res=sdscale(infile=self.rawfile,factor=factor,scaletsys=scaletsys,outfile=self.outfile)
        self.assertEqual(res,None,
                         msg='Any error occurred during calibration')

        # check if no changes applied on flag values
        tb.open(self.outfile)
        rowflags = tb.getcol('FLAGROW')
        rowflags_ref = numpy.array([1, 1, 0, 0])
        self.assertTrue(all(rowflags==rowflags_ref))
        for i in xrange(tb.nrows()):
            chanflag = tb.getcell('FLAGTRA', i)
            chanflag_ref = numpy.zeros(100, numpy.int32)
            if i in [1, 2]:
                for j in xrange(len(self.flagged_chan_list)):
                    idx_start = self.flagged_chan_list[j][0]
                    idx_end   = self.flagged_chan_list[j][1]+1
                    for k in xrange(idx_start, idx_end):
                        chanflag_ref[k] = 128
            self.assertTrue(all(chanflag==chanflag_ref))
        tb.close()

        #check spectra and tsys values
        tb.open(self.outfile)
        for i in xrange(tb.nrows()):
            spec = tb.getcell('SPECTRA', i)
            spec_ref = numpy.ones(100, numpy.float)
            tsys = tb.getcell('TSYS', i)
            tsys_ref = numpy.ones(100, numpy.float)
            if rowflags_ref[i] == 0:
                spec_ref *= factor
                tsys_ref *= factor
            self.assertTrue(all(spec==spec_ref))
            self.assertTrue(all(tsys==tsys_ref))
        tb.close()


def suite():
    return [sdscale_test0,sdscale_test1,
            sdscale_test2,sdscale_test3,
            sdscale_test4,sdscale_testflag]
