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
import re
import string

asap_init()
from sdmath import sdmath
import asap as sd

#
# Unit test of sdmath task.
# 

###
# Base class for sdmath unit test
###
class sdmath_unittest_base:
    """
    Base class for sdmath unit test
    """
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdmath/'
    
    def _checkfile( self, name ):
        isthere=os.path.exists(name)
        self.assertEqual(isthere,True,
                         msg='output file %s was not created because of the task failure'%(name))

    def _compare( self, name, ref, factor, op ):
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
            arrs = self._getref( numpy.ones(rspchans[irow],dtype=valuetype), op, f )
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
                
###
# Test on bad parameter settings
###
class sdmath_test0(unittest.TestCase,sdmath_unittest_base):
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
    prefix='sdmathTest0'
    outfile=prefix+'.asap'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(sdmath)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test000(self):
        """Test 000: Default parameters"""
        res=sdmath()
        self.assertFalse(res)        

    def test001(self):
        """Test 001: Empty varlist"""
        ex='V0+V1'
        v={}
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile)
        self.assertFalse(res)

    def test002(self):
        """Test 002: Lack of some variables in varlist"""
        ex='V0+V1'
        v={'V0': self.rawfile}
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile)
        self.assertFalse(res)
        
    def test003(self):
        """Test 003: Specify existing output file"""
        os.system('cp -r %s %s'%(self.rawfile,self.outfile))
        ex='V0+V1'
        factor=1.0
        v={'V0': self.rawfile,
           'V1': factor}
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=False)
        self.assertFalse(res)
        
    def test004(self):
        """Test 004: Bad operation (non-scantable is left side)"""
        ex='V0+V1'
        factor=1.0
        v={'V0': factor,
           'V1': self.rawfile}
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
        self.assertFalse(res)

    def test005(self):
        """Test 005: Bad operation (non-scantable operation)"""
        ex='V0+V1'
        factor=1.0
        v={'V0': factor,
           'V1': factor}
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
        self.assertFalse(res)

    def test006(self):
        """Test 006: non-conform array operation"""
        ex='V0+V1'
        factor=[1.0, 2.0]
        v={'V0': self.rawfile,
           'V1': factor}
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
        self.assertFalse(res)

    def test007(self):
        """Test 007: non-conform scantable operation"""
        ex='V0+V1'
        infile2=self.prefix+'.in.asap'
        shutil.copytree(self.datapath+'sdmath2.asap', infile2)
        v={'V0': self.rawfile,
           'V1': infile2}
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
        self.assertFalse(res)

    def test008(self):
        """Test 008: non-conform ASCII text data"""
        ex='V0+V1'
        datafile=self.prefix+'.dat'
        factor=[1.0, 2.0]
        self._makedata(datafile,factor)
        v={'V0': self.rawfile,
           'V1': datafile}
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
        self.assertFalse(res)
        

###
# Test on scalar/array operation
###
class sdmath_test1(unittest.TestCase,sdmath_unittest_base):
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
    prefix='sdmathTest2'
    outfile=prefix+'.asap'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(sdmath)

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
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,factor,op)

###
# Test on ASCII text data operation
###
class sdmath_test2(unittest.TestCase,sdmath_unittest_base):
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
    prefix='sdmathTest2'
    outfile=prefix+'.asap'
    factor=[[0.1,0.3,0.5,0.7],[0.2,0.4,0.6,0.8]]
    datafile=prefix+'.dat'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        self._makedata(self.datafile,self.factor)
        
        default(sdmath)

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
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,self.factor,op)
        
    def test201(self):
        """Test 201: Subtraction of ASCII text data"""
        op='-'
        ex='V0'+op+'V1'
        v={'V0': self.rawfile,
           'V1': self.datafile}
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,self.factor,op)

    def test202(self):
        """Test 202: Multiplication of ASCII text data"""
        op='*'
        ex='V0'+op+'V1'
        v={'V0': self.rawfile,
           'V1': self.datafile}
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,self.factor,op)

    def test203(self):
        """Test 203: Division of ASCII text data"""
        op='/'
        ex='V0'+op+'V1'
        v={'V0': self.rawfile,
           'V1': self.datafile}
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,self.factor,op)

###
# Test on scantable operation
###
class sdmath_test3(unittest.TestCase,sdmath_unittest_base):
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
    prefix='sdmathTest3'
    outfile=prefix+'.asap'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile0)):
            shutil.copytree(self.datapath+self.rawfile0, self.rawfile0)
        if (not os.path.exists(self.rawfile1)):
            shutil.copytree(self.datapath+self.rawfile1, self.rawfile1)
        
        default(sdmath)

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
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._checkresult(self.outfile,self.rawfile0,self.rawfile1,op)

    def test301(self):
        """Test 301: Subtraction of scantable"""
        op='-'
        ex='V0'+op+'V1'
        v={'V0': self.rawfile0,
           'V1': self.rawfile1}
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._checkresult(self.outfile,self.rawfile0,self.rawfile1,op)

    def test302(self):
        """Test 302: Multiplication of scantable"""
        op='*'
        ex='V0'+op+'V1'
        v={'V0': self.rawfile0,
           'V1': self.rawfile1}
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._checkresult(self.outfile,self.rawfile0,self.rawfile1,op)

    def test303(self):
        """Test 303: Division of scantable"""
        op='/'
        ex='V0'+op+'V1'
        v={'V0': self.rawfile0,
           'V1': self.rawfile1}
        res=sdmath(expr=ex,varlist=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._checkresult(self.outfile,self.rawfile0,self.rawfile1,op)

    def _checkresult(self,r,l0,l1,op):
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
        

def suite():
    return [sdmath_test0,sdmath_test1,
            sdmath_test2,sdmath_test3]
