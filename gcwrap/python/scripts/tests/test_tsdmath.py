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

from tsdmath import tsdmath
import asap as sd
from asap.scantable import is_scantable

#
# Unit test of tsdmath task.
# 

###
# Base class for tsdmath unit test
###
class tsdmath_unittest_base:
    """
    Base class for tsdmath unit test
    """
    taskname='tsdmath'
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdmath/'
    
    def _checkfile( self, name ):
        isthere=os.path.exists(name)
        self.assertEqual(isthere,True,
                         msg='output file %s was not created because of the task failure'%(name))

    def _compare( self, name, ref, factor, op, scale=1.0 ):
        self._checkfile( name )
        # get original nchan and nrow
        tb.open(ref)
        nrow0=tb.nrows()
        rspchans=[]
        for irow in xrange(nrow0):
            rspchans.append(len(tb.getcell('SPECTRA',irow)))
        tb.close()
        # check shape
        tb.open(name)
        nrow=tb.nrows()
        self.assertEqual(nrow,2,
                         msg='number of rows mismatch')
        sp=[]
        for irow in xrange(nrow):
            sp.append(tb.getcell('SPECTRA',irow))
            self.assertEqual(len(sp[irow]),rspchans[irow],
                             msg='SPECTRA: number of channel mismatch in row%s'%(irow)) 
        tb.close()
        # check data
        valuetype=type(sp[0][0])
        print ''
        for irow in xrange(nrow):
            if type(factor) is not list:
                f = factor
            elif type(factor[0]) is not list:
                f = factor
            elif len(factor[0]) == 1:
                f = factor[irow][0]
            else:
                f = factor[irow]
            arrs = self._getref( numpy.ones(rspchans[irow],dtype=valuetype)*scale, op, f )
            print 'irow=%s'%(irow)
            print '   arrs=%s'%(arrs)
            print '   sp=%s'%(sp[irow])
            ret=numpy.allclose(arrs,sp[irow])
            self.assertEqual(ret,True,
                             msg='SPECTRA: data differ in row%s'%(irow))

    def _getref( self, ref, op, factor ):
        if type(factor) is not list:
            # scalar factor
            f = factor
        elif type(factor[0]) is not list:
            # 1D array factor
            f = numpy.array(factor)
        if op == '+':
            return ref+f
        elif op == '-':
            return ref-f
        elif op == '*':
            return ref*f
        else: # op == '/'
            return ref/f

    def _makedata(self,name,factor):
        f=open(name,'w')
        if type(factor) is not list:
            print >> f, '%s'%factor
        elif type(factor[0]) is not list:
            for irow in xrange(len(factor)):
                print >> f, '%s'%factor[irow]
        else:
            for irow in xrange(len(factor)):
                s=''
                for icol in xrange(len(factor[irow])):
                    s+='%s '%factor[irow][icol]
                print >> f, s
        f.close()
        #os.system('cat %s'%name)
                
    def _checkresult(self,r,l0,l1,op,scale=1.0):
        self._checkfile(r)
        tb.open(r)
        spr=tb.getcol('SPECTRA').transpose()
        nrow=tb.nrows()
        tb.close()
        tb.open(l0)
        spl0=tb.getcol('SPECTRA').transpose()
        tb.close()
        self.assertEqual(spr.shape[0],spl0.shape[0],
                         msg='number of rows mismatch')
        self.assertEqual(spr.shape[1],spl0.shape[1],
                         msg='number of channels mismatch')
        tb.open(l1)
        spl1=tb.getcol('SPECTRA').transpose()
        tb.close()
        self.assertEqual(spr.shape[0],spl1.shape[0],
                         msg='number of rows mismatch')
        self.assertEqual(spr.shape[1],spl1.shape[1],
                         msg='number of channels mismatch')

        spl0*=scale
        spl1*=scale
        
        if op=='+':
            spl=spl0+spl1
        elif op=='-':
            spl=spl0-spl1
        elif op=='*':
            spl=spl0*spl1
        else: # op=='/'
            spl=spl0/spl1
        print ''
        for irow in xrange(nrow):
            ret=numpy.allclose(spr[irow],spl[irow])
            print 'irow=%s'%(irow)
            print '   spr=%s'%(spr[irow])
            print '   spl=%s'%(spl[irow])
            self.assertEqual(ret,True,
                             msg='SPECTRA: data differ in row%s'%(irow))

###
# Test on bad parameter settings
###
class tsdmath_test0(unittest.TestCase,tsdmath_unittest_base):
    """
    Test on bad parameter setting

    Test data, sdmath0.asap, is artificial data with the following
    status:

       - nrow = 2
       - nchan = 4 for spectral data
       - all spectral values are 1.0
       
    """
    # Input and output names
    rawfile='sdmath0.asap'
    #prefix=sdmath_unittest_base.taskname+'Test0'
    prefix='sdmathTest0'
    outfile=prefix+'.asap'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(tsdmath)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test000(self):
        """Test 000: Default parameters"""
        try:
            res=tsdmath()
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('expr is undefined')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))        

    def test001(self):
        """Test 001: Empty varnames"""
        ex='V0+V1'
        v={}
        try:
            res=tsdmath(expr=ex,varnames=v,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('name \'V0\' is not defined')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))        

    def test002(self):
        """Test 002: Lack of some variables in varnames"""
        ex='V0+V1'
        v={'V0': self.rawfile}
        try:
            res=tsdmath(expr=ex,varnames=v,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('name \'V1\' is not defined')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))       
        
    def test003(self):
        """Test 003: Specify existing output file"""
        os.system('cp -r %s %s'%(self.rawfile,self.outfile))
        ex='V0+V1'
        factor=1.0
        v={'V0': self.rawfile,
           'V1': factor}
        try:
            res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=False)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Output file \'%s\' exists.'%(self.outfile))
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))       
        
    def test004(self):
        """Test 004: Bad operation (non-scantable is left side)"""
        ex='V0+V1'
        factor=1.0
        v={'V0': factor,
           'V1': self.rawfile}
        try:
            res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('unsupported operand type(s) for +:')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))       


    def test005(self):
        """Test 005: Bad operation (non-scantable operation)"""
        ex='V0+V1'
        factor=1.0
        v={'V0': factor,
           'V1': factor}
        try:
            res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('\'float\' object has no attribute ')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))       

    def test006(self):
        """Test 006: non-conform array operation"""
        ex='V0+V1'
        factor=[1.0, 2.0]
        v={'V0': self.rawfile,
           'V1': factor}
        try:
            res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Vector size must be 1 or be same as number of channel.')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))       

    def test007(self):
        """Test 007: non-conform scantable operation"""
        ex='V0+V1'
        infile2=self.prefix+'.in.asap'
        shutil.copytree(self.datapath+'sdmath2.asap', infile2)
        v={'V0': self.rawfile,
           'V1': infile2}
        try:
            res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('arrays do not conform')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))                   

    def test008(self):
        """Test 008: non-conform ASCII text data"""
        ex='V0+V1'
        datafile=self.prefix+'.dat'
        factor=[1.0, 2.0, 3.0]
        self._makedata(datafile,factor)
        v={'V0': self.rawfile,
           'V1': datafile}
        try:
            res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('len(value) must be 1 or conform to scan.nrow()')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))
        

###
# Test on scalar/array operation
###
class tsdmath_test1(unittest.TestCase,tsdmath_unittest_base):
    """
    Test on scalar/array operation

    Test data, sdmath0.asap, is artificial data with the following
    status:

       - nrow = 2
       - nchan = 4 for spectral data
       - all spectral values are 1.0
       
    """
    # Input and output names
    rawfile='sdmath0.asap'
    #prefix=sdmath_unittest_base.taskname+'Test2'
    prefix='sdmathTest2'
    outfile=prefix+'.asap'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(tsdmath)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test100(self):
        """Test 100: Addition of scalar"""
        op='+'
        ex='V0'+op+'V1'
        factor=1.0
        v={'V0': self.rawfile,
           'V1': factor}
        res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,factor,op)
        
    def test101(self):
        """Test 101: Subtraction of scalar"""
        op='-'
        ex='V0'+op+'V1'
        factor=1.0
        v={'V0': self.rawfile,
           'V1': factor}
        res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,factor,op)

    def test102(self):
        """Test 102: Multiplication of scalar"""
        op='*'
        ex='V0'+op+'V1'
        factor=2.0
        v={'V0': self.rawfile,
           'V1': factor}
        res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,factor,op)

    def test103(self):
        """Test 103: Division of scalar"""
        op='/'
        ex='V0'+op+'V1'
        factor=2.0
        v={'V0': self.rawfile,
           'V1': factor}
        res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,factor,op)

    def test104(self):
        """Test 104: Addition of 1D array of [nchan]"""
        op='+'
        ex='V0'+op+'V1'
        factor=[0.1,0.2,0.3,0.4]
        v={'V0': self.rawfile,
           'V1': factor}
        res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,factor,op)
        
    def test105(self):
        """Test 105: Subtraction of 1D array of [nchan]"""
        op='-'
        ex='V0'+op+'V1'
        factor=[0.1,0.2,0.3,0.4]
        v={'V0': self.rawfile,
           'V1': factor}
        res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,factor,op)

    def test106(self):
        """Test 106: Multiplication of 1D array of [nchan]"""
        op='*'
        ex='V0'+op+'V1'
        factor=[0.1,0.2,0.3,0.4]
        v={'V0': self.rawfile,
           'V1': factor}
        res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,factor,op)

    def test107(self):
        """Test 107: Division of 1D array of [nchan]"""
        op='/'
        ex='V0'+op+'V1'
        factor=[0.1,0.2,0.3,0.4]
        v={'V0': self.rawfile,
           'V1': factor}
        res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,factor,op)

    def test108(self):
        """Test 108: Addition of 2D array of [nrow,1]"""
        op='+'
        ex='V0'+op+'V1'
        factor=[[0.1],[0.2]]
        v={'V0': self.rawfile,
           'V1': factor}
        res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,factor,op)
        
    def test109(self):
        """Test 109: Subtraction of 2D array of [nrow,1]"""
        op='-'
        ex='V0'+op+'V1'
        factor=[[0.1],[0.2]]
        v={'V0': self.rawfile,
           'V1': factor}
        res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,factor,op)

    def test110(self):
        """Test 110: Multiplication of 2D array of [nrow,1]"""
        op='*'
        ex='V0'+op+'V1'
        factor=[[0.1],[0.2]]
        v={'V0': self.rawfile,
           'V1': factor}
        res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,factor,op)

    def test111(self):
        """Test 111: Division of 2D array of [nrow,1]"""
        op='/'
        ex='V0'+op+'V1'
        factor=[[0.1],[0.2]]
        v={'V0': self.rawfile,
           'V1': factor}
        res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,factor,op)

    def test112(self):
        """Test 112: Addition of 2D array of [nrow,nchan]"""
        op='+'
        ex='V0'+op+'V1'
        factor=[[0.1,0.3,0.5,0.7],[0.2,0.4,0.6,0.8]]
        v={'V0': self.rawfile,
           'V1': factor}
        res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,factor,op)
        
    def test113(self):
        """Test 113: Subtraction of 2D array of [nrow,nchan]"""
        op='-'
        ex='V0'+op+'V1'
        factor=[[0.1,0.3,0.5,0.7],[0.2,0.4,0.6,0.8]]
        v={'V0': self.rawfile,
           'V1': factor}
        res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,factor,op)

    def test114(self):
        """Test 114: Multiplication of 2D array of [nrow,nchan]"""
        op='*'
        ex='V0'+op+'V1'
        factor=[[0.1,0.3,0.5,0.7],[0.2,0.4,0.6,0.8]] 
        v={'V0': self.rawfile,
           'V1': factor}
        res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,factor,op)

    def test115(self):
        """Test 115: Division of 2D array of [nrow,nchan]"""
        op='/'
        ex='V0'+op+'V1'
        factor=[[0.1,0.3,0.5,0.7],[0.2,0.4,0.6,0.8]]
        v={'V0': self.rawfile,
           'V1': factor}
        res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,factor,op)

###
# Test on ASCII text data operation
###
class tsdmath_test2(unittest.TestCase,tsdmath_unittest_base):
    """
    Test on ASCII text data operation

    Test data, sdmath0.asap, is artificial data with the following
    status:

       - nrow = 2
       - nchan = 4 for spectral data
       - all spectral values are 1.0
       
    """
    # Input and output names
    rawfile='sdmath0.asap'
    #prefix=sdmath_unittest_base.taskname+'Test2'
    prefix='sdmathTest2'
    outfile=prefix+'.asap'
    factor=[[0.1,0.3,0.5,0.7],[0.2,0.4,0.6,0.8]]
    datafile=prefix+'.dat'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        self._makedata(self.datafile,self.factor)
        
        default(tsdmath)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test200(self):
        """Test 200: Addition of ASCII text data"""
        op='+'
        ex='V0'+op+'V1'
        v={'V0': self.rawfile,
           'V1': self.datafile}
        res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,self.factor,op)
        
    def test201(self):
        """Test 201: Subtraction of ASCII text data"""
        op='-'
        ex='V0'+op+'V1'
        v={'V0': self.rawfile,
           'V1': self.datafile}
        res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,self.factor,op)

    def test202(self):
        """Test 202: Multiplication of ASCII text data"""
        op='*'
        ex='V0'+op+'V1'
        v={'V0': self.rawfile,
           'V1': self.datafile}
        res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,self.factor,op)

    def test203(self):
        """Test 203: Division of ASCII text data"""
        op='/'
        ex='V0'+op+'V1'
        v={'V0': self.rawfile,
           'V1': self.datafile}
        res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,self.factor,op)

###
# Test on scantable operation
###
class tsdmath_test3(unittest.TestCase,tsdmath_unittest_base):
    """
    Test on scantable operation

    Test data, sdmath0.asap, is artificial data with the following
    status:

       - nrow = 2
       - nchan = 4 for spectral data
       - all spectral values are 1.0

    Another test data, sdmath1.asap, is artificial data with the
    following status:

       - shape is identical with sdmath0.asap
       - all spectral values are 0.1
       
    """
    # Input and output names
    rawfile0='sdmath0.asap'
    rawfile1='sdmath1.asap'
    #prefix=sdmath_unittest_base.taskname+'Test3'
    prefix='sdmathTest3'
    outfile=prefix+'.asap'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile0)):
            shutil.copytree(self.datapath+self.rawfile0, self.rawfile0)
        if (not os.path.exists(self.rawfile1)):
            shutil.copytree(self.datapath+self.rawfile1, self.rawfile1)
        
        default(tsdmath)

    def tearDown(self):
        if (os.path.exists(self.rawfile0)):
            shutil.rmtree(self.rawfile0)
        if (os.path.exists(self.rawfile1)):
            shutil.rmtree(self.rawfile1)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test300(self):
        """Test 300: Addition of scantable"""
        op='+'
        ex='V0'+op+'V1'
        v={'V0': self.rawfile0,
           'V1': self.rawfile1}
        res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._checkresult(self.outfile,self.rawfile0,self.rawfile1,op)

    def test301(self):
        """Test 301: Subtraction of scantable"""
        op='-'
        ex='V0'+op+'V1'
        v={'V0': self.rawfile0,
           'V1': self.rawfile1}
        res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._checkresult(self.outfile,self.rawfile0,self.rawfile1,op)

    def test302(self):
        """Test 302: Multiplication of scantable"""
        op='*'
        ex='V0'+op+'V1'
        v={'V0': self.rawfile0,
           'V1': self.rawfile1}
        res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._checkresult(self.outfile,self.rawfile0,self.rawfile1,op)

    def test303(self):
        """Test 303: Division of scantable"""
        op='/'
        ex='V0'+op+'V1'
        v={'V0': self.rawfile0,
           'V1': self.rawfile1}
        res=tsdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._checkresult(self.outfile,self.rawfile0,self.rawfile1,op)


###
# Test on scantable storage and insitu settings
###
class tsdmath_storageTest( unittest.TestCase, tsdmath_unittest_base ):
    """
    Test on scantable sotrage and insitu.

    Test data, sdmath0.asap, is artificial data with the following
    status:

       - nrow = 2
       - nchan = 4 for spectral data
       - all spectral values are 1.0

    Another test data, sdmath1.asap, is artificial data with the
    following status:

       - shape is identical with sdmath0.asap
       - all spectral values are 0.1

    The list of tests:
    testMT  --- storage = 'memory', insitu = True
    testMF  --- storage = 'memory', insitu = False
    testDT  --- storage = 'disk', insitu = True
    testDF  --- storage = 'disk', insitu = False

    Note on handlings of disk storage:
       Task script restores unit and frame information.

    Tested items:
       1. Test of output result by self._compare.
       2. Units and coordinates of output scantable.
       3. units and coordinates of input scantables before/after run.
    """
    # Input and output names
    rawfile='sdmath0.asap'
    rawfile1='sdmath1.asap'
    inlist = [rawfile, rawfile1]
    #outprefix=sdmath_unittest_base.taskname+'.Tstorage'
    outprefix='sdmath.Tstorage'
    outsuffix='.asap'

    # specunit is ignored in current task script (CAS-4094)
    #out_uc = {'spunit': 'GHz', 'flunit': 'Jy',\
    #          'frame': 'LSRD', 'doppler': 'OPTICAL'}
    #out_uc = {'spunit': 'channel', 'flunit': 'Jy',\
    #          'frame': 'LSRD', 'doppler': 'OPTICAL'}
    out_uc = {'flunit': 'Jy'}

    def setUp( self ):
        for infile in [self.rawfile, self.rawfile1]:
            if os.path.exists(infile):
                shutil.rmtree(infile)
            shutil.copytree(self.datapath+infile, infile)
        
        default(tsdmath)

    def tearDown( self ):
        for infile in [self.rawfile, self.rawfile1]:
            if os.path.exists(infile):
                shutil.rmtree(infile)
        os.system( 'rm -rf %s*'%(self.outprefix) )

    # helper functions of tests
    def _get_unit_coord( self, scanname ):
        # Returns a dictionary which stores units and coordinates of a
        # scantable, scanname. Returned dictionary stores spectral
        # unit, flux unit, frequency frame, and doppler of scanname.
        self.assertTrue(os.path.exists(scanname),\
                        "'%s' does not exists." % scanname)
        self.assertTrue(is_scantable(scanname),\
                        "Input table is not a scantable: %s" % scanname)
        scan = sd.scantable(scanname, average=False,parallactify=False)
        retdict = {}
        #retdict['spunit'] = scan.get_unit()
        retdict['flunit'] = scan.get_fluxunit()
        coord = scan._getcoordinfo()
        #retdict['frame'] = coord[1]
        #retdict['doppler'] = coord[2]
        return retdict

    def _get_uclist( self, stlist ):
        # Returns a list of dictionaries of units and coordinates of
        # a list of scantables in stlist. This method internally calls
        # _get_unit_coord(scanname).
        retlist = []
        for scanname in stlist:
            retlist.append(self._get_unit_coord(scanname))
        #print retlist
        return retlist

    def _comp_unit_coord( self, stlist, before):
        ### stlist: a list of scantable names
        if isinstance(stlist,str):
            stlist = [ stlist ]
        ### before: a return value of _get_uclist() before run
        if isinstance(before, dict):
            before = [ before ]
        if len(stlist) != len(before):
            raise Exception("Number of scantables in list is different from reference data.")
        self.assertTrue(isinstance(before[0],dict),\
                        "Reference data should be (a list of) dictionary")

        after = self._get_uclist(stlist)
        for i in range(len(stlist)):
            print "Comparing units and coordinates of '%s'" %\
                  stlist[i]
            self._compareDictVal(after[i],before[i])

    def _compareDictVal( self, testdict, refdict, reltol=1.0e-5, complist=None ):
        self.assertTrue(isinstance(testdict,dict) and \
                        isinstance(refdict, dict),\
                        "Need to specify two dictionaries to compare")
        if complist:
            keylist = complist
        else:
            keylist = refdict.keys()
        
        for key in keylist:
            self.assertTrue(testdict.has_key(key),\
                            msg="%s is not defined in the current results."\
                            % key)
            self.assertTrue(refdict.has_key(key),\
                            msg="%s is not defined in the reference data."\
                            % key)
            testval = self._to_list(testdict[key])
            refval = self._to_list(refdict[key])
            self.assertTrue(len(testval)==len(refval),"Number of elemnets differs.")
            for i in range(len(testval)):
                if isinstance(refval[i],str):
                    self.assertTrue(testval[i]==refval[i],\
                                    msg="%s[%d] differs: %s (expected: %s) " % \
                                    (key, i, str(testval[i]), str(refval[i])))
                else:
                    self.assertTrue(self._isInAllowedRange(testval[i],refval[i],reltol),\
                                    msg="%s[%d] differs: %s (expected: %s) " % \
                                    (key, i, str(testval[i]), str(refval[i])))
            del testval, refval
            

    def _isInAllowedRange( self, testval, refval, reltol=1.0e-5 ):
        """
        Check if a test value is within permissive relative difference from refval.
        Returns a boolean.
        testval & refval : two numerical values to compare
        reltol           : allowed relative difference to consider the two
                           values to be equal. (default 0.01)
        """
        denom = refval
        if refval == 0:
            if testval == 0:
                return True
            else:
                denom = testval
        rdiff = (testval-refval)/denom
        del denom,testval,refval
        return (abs(rdiff) <= reltol)

    def _to_list( self, input ):
        """
        Convert input to a list
        If input is None, this method simply returns None.
        """
        import numpy
        listtypes = (list, tuple, numpy.ndarray)
        if input == None:
            return None
        elif type(input) in listtypes:
            return list(input)
        else:
            return [input]

    # Actual tests
    def testMT( self ):
        """Storage Test MT: Division of scalar on storage='memory' and insitu=T"""
        # Operation with a numerical value calls stmath._unaryop()
        tid = "MT"
        op = '/'
        ex = 'V0'+op+'V1'
        factor = 2.0
        v = {'V0': self.rawfile,
             'V1': factor}
        outfile = self.outprefix+tid+self.outsuffix

        # Backup units and coords of input scantable before run.
        initval = self._get_unit_coord(self.rawfile)

        sd.rcParams['scantable.storage'] = 'memory'
        sd.rcParams['insitu'] = True
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        res = tsdmath(expr=ex,varnames=v,outfile=outfile,\
                     fluxunit=self.out_uc['flunit'])

        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(outfile,self.rawfile,factor,op,0.78232766)
        # Test  units and coords of output scantable
        self._comp_unit_coord(outfile,self.out_uc)
        # Compare units and coords of input scantable before/after run
        self._comp_unit_coord(self.rawfile,initval)


    def testMT2( self ):
        """Storage Test MT2: Division of scalar on storage='memory' and insitu=T without converting spectral values"""
        # Operation with a numerical value calls stmath._unaryop()
        tid = "MT"
        op = '/'
        ex = 'V0'+op+'V1'
        factor = 2.0
        v = {'V0': self.rawfile,
             'V1': factor}
        outfile = self.outprefix+tid+self.outsuffix

        # Backup units and coords of input scantable before run.
        initval = self._get_unit_coord(self.rawfile)

        sd.rcParams['scantable.storage'] = 'memory'
        sd.rcParams['insitu'] = True
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        res = tsdmath(expr=ex,varnames=v,outfile=outfile,\
                     fluxunit=self.out_uc['flunit'],\
                     telescopeparam='FIX')

        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(outfile,self.rawfile,factor,op)
        # Test  units and coords of output scantable
        self._comp_unit_coord(outfile,self.out_uc)
        # Compare units and coords of input scantable before/after run
        self._comp_unit_coord(self.rawfile,initval)


    def testMF( self ):
        """Storage Test MF: Multiplication of 1D array of [nchan] on storage='memory' and insitu=F"""
        # Operation with a numerical value calls stmath._arrayop()
        tid = "MF"
        op='*'
        ex='V0'+op+'V1'
        factor=[0.1,0.2,0.3,0.4]
        v={'V0': self.rawfile,
           'V1': factor}
        outfile = self.outprefix+tid+self.outsuffix
        
        # Backup units and coords of input scantable before run.
        initval = self._get_unit_coord(self.rawfile)

        sd.rcParams['scantable.storage'] = 'memory'
        sd.rcParams['insitu'] = False
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        res = tsdmath(expr=ex,varnames=v,outfile=outfile,\
                     fluxunit=self.out_uc['flunit'])

        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(outfile,self.rawfile,factor,op,0.78232766)
        # Test  units and coords of output scantable
        self._comp_unit_coord(outfile,self.out_uc)
        # Compare units and coords of input scantable before/after run
        self._comp_unit_coord(self.rawfile,initval)


    def testMF2( self ):
        """Storage Test MF2: Multiplication of 1D array of [nchan] on storage='memory' and insitu=F without converting spectral values"""
        # Operation with a numerical value calls stmath._arrayop()
        tid = "MF"
        op='*'
        ex='V0'+op+'V1'
        factor=[0.1,0.2,0.3,0.4]
        v={'V0': self.rawfile,
           'V1': factor}
        outfile = self.outprefix+tid+self.outsuffix
        
        # Backup units and coords of input scantable before run.
        initval = self._get_unit_coord(self.rawfile)

        sd.rcParams['scantable.storage'] = 'memory'
        sd.rcParams['insitu'] = False
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        res = tsdmath(expr=ex,varnames=v,outfile=outfile,\
                     fluxunit=self.out_uc['flunit'],\
                     telescopeparam='FIX')

        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(outfile,self.rawfile,factor,op)
        # Test  units and coords of output scantable
        self._comp_unit_coord(outfile,self.out_uc)
        # Compare units and coords of input scantable before/after run
        self._comp_unit_coord(self.rawfile,initval)


    def testDT( self ):
        """Storage Test DT: Subtraction of 2D array of [nrow,nchan] on storage='disk' and insitu=T"""
        # Operation of 2-D array calls asapmath._array2dOp()
        tid = "DT"
        op='-'
        ex='V0'+op+'V1'
        factor=[[0.1,0.3,0.5,0.7],[0.2,0.4,0.6,0.8]]
        v={'V0': self.rawfile,
           'V1': factor}
        outfile = self.outprefix+tid+self.outsuffix

        # Backup units and coords of input scantable before run.
        initval = self._get_unit_coord(self.rawfile)

        sd.rcParams['scantable.storage'] = 'disk'
        sd.rcParams['insitu'] = True
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        res = tsdmath(expr=ex,varnames=v,outfile=outfile,\
                     fluxunit=self.out_uc['flunit'])

        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(outfile,self.rawfile,factor,op,0.78232766)
        # Test  units and coords of output scantable
        self._comp_unit_coord(outfile,self.out_uc)
        # Compare units and coords of input scantable before/after run
        self._comp_unit_coord(self.rawfile,initval)


    def testDT2( self ):
        """Storage Test DT2: Subtraction of 2D array of [nrow,nchan] on storage='disk' and insitu=T without converting spectral values"""
        # Operation of 2-D array calls asapmath._array2dOp()
        tid = "DT"
        op='-'
        ex='V0'+op+'V1'
        factor=[[0.1,0.3,0.5,0.7],[0.2,0.4,0.6,0.8]]
        v={'V0': self.rawfile,
           'V1': factor}
        outfile = self.outprefix+tid+self.outsuffix

        # Backup units and coords of input scantable before run.
        initval = self._get_unit_coord(self.rawfile)

        sd.rcParams['scantable.storage'] = 'disk'
        sd.rcParams['insitu'] = True
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        res = tsdmath(expr=ex,varnames=v,outfile=outfile,\
                     fluxunit=self.out_uc['flunit'],\
                     telescopeparam='FIX')

        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(outfile,self.rawfile,factor,op)
        # Test  units and coords of output scantable
        self._comp_unit_coord(outfile,self.out_uc)
        # Compare units and coords of input scantable before/after run
        self._comp_unit_coord(self.rawfile,initval)


    def testDF( self ):
        """Storage Test DF: Addition of two scantables on storage='disk' and insitu=F"""
        # Operation of two scantables calls stmath._binaryop()
        tid = "DF"
        op='+'
        ex='V0'+op+'V1'
        v={'V0': self.rawfile,
           'V1': self.rawfile1}
        outfile = self.outprefix+tid+self.outsuffix

        # Backup units and coords of input scantable before run.
        initval = self._get_uclist([self.rawfile, self.rawfile1])

        sd.rcParams['scantable.storage'] = 'disk'
        sd.rcParams['insitu'] = False
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        res = tsdmath(expr=ex,varnames=v,outfile=outfile,\
                     fluxunit=self.out_uc['flunit'])

        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._checkresult(outfile,self.rawfile,self.rawfile1,op,0.78232766)
        # Test  units and coords of output scantable
        self._comp_unit_coord(outfile,self.out_uc)
        # Compare units and coords of input scantable before/after run
        self._comp_unit_coord([self.rawfile, self.rawfile1],initval)
        
    def testDF2( self ):
        """Storage Test DF2: Addition of two scantables on storage='disk' and insitu=F without converting spectral values"""
        # Operation of two scantables calls stmath._binaryop()
        tid = "DF"
        op='+'
        ex='V0'+op+'V1'
        v={'V0': self.rawfile,
           'V1': self.rawfile1}
        outfile = self.outprefix+tid+self.outsuffix

        # Backup units and coords of input scantable before run.
        initval = self._get_uclist([self.rawfile, self.rawfile1])

        sd.rcParams['scantable.storage'] = 'disk'
        sd.rcParams['insitu'] = False
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        res = tsdmath(expr=ex,varnames=v,outfile=outfile,\
                     fluxunit=self.out_uc['flunit'],\
                     telescopeparam='FIX')

        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._checkresult(outfile,self.rawfile,self.rawfile1,op)
        # Test  units and coords of output scantable
        self._comp_unit_coord(outfile,self.out_uc)
        # Compare units and coords of input scantable before/after run
        self._comp_unit_coord([self.rawfile, self.rawfile1],initval)
        

def suite():
    return [tsdmath_test0,tsdmath_test1,
            tsdmath_test2,tsdmath_test3,
            tsdmath_storageTest]
