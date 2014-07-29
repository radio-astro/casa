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

try:
    import selection_syntax
except:
    import tests.selection_syntax as selection_syntax

#to rethrow exception
import inspect
g = sys._getframe(len(inspect.stack())-1).f_globals
g['__rethrow_casa_exceptions'] = True
from sdmath import sdmath
import asap as sd
from asap.scantable import is_scantable

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
    taskname='sdmath'
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdmath/'
    
    def _checkfile( self, name ):
        isthere=os.path.exists(name)
        self.assertEqual(isthere,True,
                         msg='output file %s was not created because of the task failure'%(name))

    def _getdim( self, sp ):
        dim = 0
        import copy
        a = copy.deepcopy(sp)
        while isinstance(a, list):
            dim = dim + 1
            a = a[0]
        return dim
    
    def _checkshape( self, sp, ref ):
        # check array dimension
        self.assertEqual( self._getdim(sp), self._getdim(ref),
                          msg='array dimension differ' )
        # check number of spectra
        self.assertEqual( len(sp), len(ref),
                          msg='number of spectra differ' )
        # check number of channel
        self.assertEqual( len(sp[0]), len(ref[0]),
                          msg='number of channel differ' )

    def _diff(self, sp, ref):
        diff=abs((sp-ref)/ref)
        idx=numpy.argwhere(numpy.isnan(diff))
        #print idx
        if len(idx) > 0:
            diff[idx]=sp[idx]
        return diff
        
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

    def _getspectra( self, name ):
        isthere=os.path.exists(name)
        self.assertEqual(isthere,True,
                         msg='file %s does not exist'%(name))        
        tb.open(name)
        sp=tb.getcol('SPECTRA').transpose()
        tb.close()
        return sp

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
    #prefix=sdmath_unittest_base.taskname+'Test0'
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
        try:
            res=sdmath()
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
            res=sdmath(expr=ex,varnames=v,outfile=self.outfile)
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
            res=sdmath(expr=ex,varnames=v,outfile=self.outfile)
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
            res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=False)
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
            res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
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
            res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
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
            res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
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
            res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
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
            res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('len(value) must be 1 or conform to scan.nrow()')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))
        

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
    #prefix=sdmath_unittest_base.taskname+'Test2'
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
        res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
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
        res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,self.factor,op)
        
    def test201(self):
        """Test 201: Subtraction of ASCII text data"""
        op='-'
        ex='V0'+op+'V1'
        v={'V0': self.rawfile,
           'V1': self.datafile}
        res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,self.factor,op)

    def test202(self):
        """Test 202: Multiplication of ASCII text data"""
        op='*'
        ex='V0'+op+'V1'
        v={'V0': self.rawfile,
           'V1': self.datafile}
        res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile,self.factor,op)

    def test203(self):
        """Test 203: Division of ASCII text data"""
        op='/'
        ex='V0'+op+'V1'
        v={'V0': self.rawfile,
           'V1': self.datafile}
        res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
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
    #prefix=sdmath_unittest_base.taskname+'Test3'
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
        res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._checkresult(self.outfile,self.rawfile0,self.rawfile1,op)

    def test301(self):
        """Test 301: Subtraction of scantable"""
        op='-'
        ex='V0'+op+'V1'
        v={'V0': self.rawfile0,
           'V1': self.rawfile1}
        res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._checkresult(self.outfile,self.rawfile0,self.rawfile1,op)

    def test302(self):
        """Test 302: Multiplication of scantable"""
        op='*'
        ex='V0'+op+'V1'
        v={'V0': self.rawfile0,
           'V1': self.rawfile1}
        res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._checkresult(self.outfile,self.rawfile0,self.rawfile1,op)

    def test303(self):
        """Test 303: Division of scantable"""
        op='/'
        ex='V0'+op+'V1'
        v={'V0': self.rawfile0,
           'V1': self.rawfile1}
        res=sdmath(expr=ex,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._checkresult(self.outfile,self.rawfile0,self.rawfile1,op)

###
# Test on operation of scantable given in infiles
###
class sdmath_test4(unittest.TestCase,sdmath_unittest_base):
    """
    Test on operation of scantable given in infiles

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
    prefix='sdmathTest4'
    outfile=prefix+'.asap'
    factor=[[0.1,0.3,0.5,0.7],[0.2,0.4,0.6,0.8]]
    datafile=prefix+'.dat'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile0)):
            shutil.copytree(self.datapath+self.rawfile0, self.rawfile0)
        if (not os.path.exists(self.rawfile1)):
            shutil.copytree(self.datapath+self.rawfile1, self.rawfile1)
        self._makedata(self.datafile,self.factor)
        
        default(sdmath)

    def tearDown(self):
        if (os.path.exists(self.rawfile0)):
            shutil.rmtree(self.rawfile0)
        if (os.path.exists(self.rawfile1)):
            shutil.rmtree(self.rawfile1)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test400(self):
        """Test 400: Addition of scantables given in infiles"""
        op='+'
        ex='IN0'+op+'IN1'
        infiles=[self.rawfile0, self.rawfile1]
        res=sdmath(expr=ex,infiles=infiles,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._checkresult(self.outfile,self.rawfile0,self.rawfile1,op)

    def test401(self):
        """Test 401: Addition of scantables from infiles(first) and varnames"""
        op='+'
        ex='IN0'+op+'V0'
        infiles=[self.rawfile0]
        v={'V0':self.rawfile1}
        res=sdmath(expr=ex,infiles=infiles,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._checkresult(self.outfile,self.rawfile0,self.rawfile1,op)

    def test402(self):
        """Test 402: Addition of scantables from varnames(first) and infiles"""
        op='+'
        ex='V0'+op+'IN0'
        infiles=[self.rawfile0]
        v={'V0':self.rawfile1}
        res=sdmath(expr=ex,infiles=infiles,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._checkresult(self.outfile,self.rawfile0,self.rawfile1,op)

    def test403(self):
        """Test 403: Addition of scantables from infiles and scalar"""
        op='+'
        ex='IN0'+op+'V0'
        infiles=[self.rawfile0]
        factor403=1.0
        v={'V0':factor403}
        res=sdmath(expr=ex,infiles=infiles,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile0,factor403,op)

    def test404(self):
        """Test 404: Addition of scantables from infiles and 1D array of [nchan]"""
        op='+'
        ex='IN0'+op+'V0'
        infiles=[self.rawfile0]
        factor404=[0.1,0.2,0.3,0.4]
        v={'V0':factor404}
        res=sdmath(expr=ex,infiles=infiles,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile0,factor404,op)

    def test405(self):
        """Test 405: Addition of scantables from infiles and 2D array of [nrow,1]"""
        op='+'
        ex='IN0'+op+'V0'
        infiles=[self.rawfile0]
        factor405=[[0.1],[0.2]]
        v={'V0':factor405}
        res=sdmath(expr=ex,infiles=infiles,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile0,factor405,op)

    def test406(self):
        """Test 406: Addition of scantables from infiles and 2D array of [nrow,nchan]"""
        op='+'
        ex='IN0'+op+'V0'
        infiles=[self.rawfile0]
        factor406=[[0.1,0.3,0.5,0.7],[0.2,0.4,0.6,0.8]]
        v={'V0':factor406}
        res=sdmath(expr=ex,infiles=infiles,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile0,factor406,op)

    def test407(self):
        """Test 407: Addition of scantables from infiles and ASCII text data"""
        op='+'
        ex='IN0'+op+'V0'
        infiles=[self.rawfile0]
        v={'V0':self.datafile}
        res=sdmath(expr=ex,infiles=infiles,varnames=v,outfile=self.outfile,overwrite=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._compare(self.outfile,self.rawfile0,self.factor,op)

###
# Test on scantable storage and insitu settings
###
class sdmath_storageTest( unittest.TestCase,sdmath_unittest_base ):
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
        
        default(sdmath)

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
        res = sdmath(expr=ex,varnames=v,outfile=outfile,\
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
        res = sdmath(expr=ex,varnames=v,outfile=outfile,\
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
        res = sdmath(expr=ex,varnames=v,outfile=outfile,\
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
        res = sdmath(expr=ex,varnames=v,outfile=outfile,\
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
        res = sdmath(expr=ex,varnames=v,outfile=outfile,\
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
        res = sdmath(expr=ex,varnames=v,outfile=outfile,\
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
        res = sdmath(expr=ex,varnames=v,outfile=outfile,\
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
        res = sdmath(expr=ex,varnames=v,outfile=outfile,\
                     fluxunit=self.out_uc['flunit'],\
                     telescopeparam='FIX')

        self.assertEqual(res,None,
                         msg='Any error occurred during task execution')
        self._checkresult(outfile,self.rawfile,self.rawfile1,op)
        # Test  units and coords of output scantable
        self._comp_unit_coord(outfile,self.out_uc)
        # Compare units and coords of input scantable before/after run
        self._comp_unit_coord([self.rawfile, self.rawfile1],initval)

class sdmath_test_selection(selection_syntax.SelectionSyntaxTest,
                           sdmath_unittest_base,unittest.TestCase):
    """
    Test selection syntax. Selection parameters to test are:
    field, spw (no channel selection), scan, pol
    
    Data used for this test are sd_analytic_type1-3.asap (raw data)
    and sdmath_selection.asap.ref (reference data).
    The reference data are generated using the following expr parameter:
    expr='V1*V1+1.0' where V1='sd_analytic_type1-3.asap'.

    """
    # Input and output names
    rawfile='sd_analytic_type1-3.asap'
    reffile='sdmath_selection.asap.ref'
    prefix=sdmath_unittest_base.taskname+'TestSel'
    postfix='.math.asap'
    field_prefix = 'M100__'

    expr = '"' + rawfile + '"*"' + rawfile + '"+1.0'
    
    @property
    def task(self):
        return sdmath
    
    @property
    def spw_channel_selection(self):
        return False

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.reffile)):
            shutil.copytree(self.datapath+self.reffile, self.reffile)

        default(sdmath)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.reffile)):
            shutil.rmtree(self.reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def _compare( self, name, ref ):
        self._checkfile( name )
        # reference data
        tb.open(ref)
        nrow0=tb.nrows()
        rsp=[]
        for irow in xrange(nrow0):
            rsp.append(tb.getcell('SPECTRA',irow))
        tb.close()
        # check shape
        tb.open(name)
        nrow=tb.nrows()
        self.assertEqual(nrow,nrow0,msg='number of rows mismatch')
        sp=[]
        for irow in xrange(nrow):
            sp.append(tb.getcell('SPECTRA',irow))
            self.assertEqual(len(sp[irow]),len(rsp[irow]),
                             msg='SPECTRA: number of channel mismatch in row%s'%(irow)) 
        tb.close()
        # check data
        valuetype=type(sp[0][0])
        #print ''
        for irow in xrange(nrow):
            #print 'irow=%s'%(irow)
            #print '  rsp=%s'%(rsp[irow])
            #print '   sp=%s'%(sp[irow])
            ret=numpy.allclose(rsp[irow],sp[irow])
            self.assertEqual(ret,True,
                             msg='SPECTRA: data differ in row%s'%(irow))

    ####################
    # scan
    ####################
    def test_scan_id_default(self):
        """test scan selection (scan='')"""
        outname=self.prefix+self.postfix
        scan=''
        self.res=sdmath(scan=scan,expr=self.expr,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, self.reffile)

    def test_scan_id_exact(self):
        """ test scan selection (scan='15')"""
        outname=self.prefix+self.postfix
        scan = '15'
        self.res=sdmath(scan=scan,expr=self.expr,outfile=outname)
        tbsel = {'SCANNO': [15]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_scan_id_lt(self):
        """ test scan selection (scan='<17')"""
        outname=self.prefix+self.postfix
        scan = '<17'
        self.res=sdmath(scan=scan,expr=self.expr,outfile=outname)
        tbsel = {'SCANNO': [15,16]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_scan_id_gt(self):
        """ test scan selection (scan='>15')"""
        outname=self.prefix+self.postfix
        scan = '>15'
        self.res=sdmath(scan=scan,expr=self.expr,outfile=outname)
        tbsel = {'SCANNO': [16,17]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_scan_id_range(self):
        """ test scan selection (scan='15~16')"""
        outname=self.prefix+self.postfix
        scan = '15~16'
        self.res=sdmath(scan=scan,expr=self.expr,outfile=outname)
        tbsel = {'SCANNO': [15,16]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_scan_id_list(self):
        """ test scan selection (scan='15,17')"""
        outname=self.prefix+self.postfix
        scan = '15,17'
        self.res=sdmath(scan=scan,expr=self.expr,outfile=outname)
        tbsel = {'SCANNO': [15,17]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    def test_scan_id_exprlist(self):
        """ test scan selection (scan='<16, 17')"""
        outname=self.prefix+self.postfix
        scan = '<16, 17'
        self.res=sdmath(scan=scan,expr=self.expr,outfile=outname)
        tbsel = {'SCANNO': [15,17]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    ####################
    # pol
    ####################
    def test_pol_id_default(self):
        """test pol selection (pol='')"""
        outname=self.prefix+self.postfix
        pol=''
        self.res=sdmath(pol=pol,expr=self.expr,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, self.reffile)

    def test_pol_id_exact(self):
        """ test pol selection (pol='1')"""
        outname=self.prefix+self.postfix
        pol = '1'
        self.res=sdmath(pol=pol,expr=self.expr,outfile=outname)
        tbsel = {'POLNO': [1]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    def test_pol_id_lt(self):
        """ test pol selection (pol='<1')"""
        outname=self.prefix+self.postfix
        pol = '<1'
        self.res=sdmath(pol=pol,expr=self.expr,outfile=outname)
        tbsel = {'POLNO': [0]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    def test_pol_id_gt(self):
        """ test pol selection (pol='>0')"""
        outname=self.prefix+self.postfix
        pol = '>0'
        self.res=sdmath(pol=pol,expr=self.expr,outfile=outname)
        tbsel = {'POLNO': [1]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    def test_pol_id_range(self):
        """ test pol selection (pol='0~1')"""
        outname=self.prefix+self.postfix
        pol = '0~1'
        self.res=sdmath(pol=pol,expr=self.expr,outfile=outname)
        tbsel = {}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    def test_pol_id_list(self):
        """ test pol selection (pol='0,1')"""
        outname=self.prefix+self.postfix
        pol = '0,1'
        self.res=sdmath(pol=pol,expr=self.expr,outfile=outname)
        tbsel = {}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    def test_pol_id_exprlist(self):
        """test pol selection (pol='<1,1')"""
        outname=self.prefix+self.postfix
        pol = '0,1'
        self.res=sdmath(pol=pol,expr=self.expr,outfile=outname)
        tbsel = {}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    ####################
    # field
    ####################
    def test_field_value_default(self):
        """test field selection (field='')"""
        outname=self.prefix+self.postfix
        field=''
        self.res=sdmath(field=field,expr=self.expr,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, self.reffile)

    def test_field_id_exact(self):
        """ test field selection (field='6')"""
        outname=self.prefix+self.postfix
        field = '6'
        self.res=sdmath(field=field,expr=self.expr,outfile=outname)
        tbsel = {'FIELDNAME': ['M100__6']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    def test_field_id_lt(self):
        """ test field selection (field='<6')"""
        outname=self.prefix+self.postfix
        field = '<6'
        self.res=sdmath(field=field,expr=self.expr,outfile=outname)
        tbsel = {'FIELDNAME': ['M100__5']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    def test_field_id_gt(self):
        """ test field selection (field='>7')"""
        outname=self.prefix+self.postfix
        field = '>7'
        self.res=sdmath(field=field,expr=self.expr,outfile=outname)
        tbsel = {'FIELDNAME': ['3C273__8']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    def test_field_id_range(self):
        """ test field selection (field='5~7')"""
        outname=self.prefix+self.postfix
        field = '5~7'
        self.res=sdmath(field=field,expr=self.expr,outfile=outname)
        tbsel = {'FIELDNAME': ['M100__5', 'M100__6', 'M30__7']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    def test_field_id_list(self):
        """ test field selection (field='5,7')"""
        outname=self.prefix+self.postfix
        field = '5,7'
        self.res=sdmath(field=field,expr=self.expr,outfile=outname)
        tbsel = {'FIELDNAME': ['M100__5', 'M30__7']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    def test_field_id_exprlist(self):
        """ test field selection (field='<7,8')"""
        outname=self.prefix+self.postfix
        field = '<7,8'
        self.res=sdmath(field=field,expr=self.expr,outfile=outname)
        tbsel = {'FIELDNAME': ['M100__5', 'M100__6', '3C273__8']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    def test_field_value_exact(self):
        """ test field selection (field='M100')"""
        outname=self.prefix+self.postfix
        field = 'M100'
        self.res=sdmath(field=field,expr=self.expr,outfile=outname)
        tbsel = {'FIELDNAME': ['M100__5', 'M100__6']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    def test_field_value_pattern(self):
        """ test field selection (field='M*')"""
        outname=self.prefix+self.postfix
        field = 'M*'
        self.res=sdmath(field=field,expr=self.expr,outfile=outname)
        tbsel = {'FIELDNAME': ['M100__5', 'M100__6', 'M30__7']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    def test_field_value_list(self):
        """ test field selection (field='M30,3C273')"""
        outname=self.prefix+self.postfix
        field = 'M30,3C273'
        self.res=sdmath(field=field,expr=self.expr,outfile=outname)
        tbsel = {'FIELDNAME': ['M30__7', '3C273__8']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    def test_field_mix_exprlist(self):
        """ test field selection (field='<7,3C273')"""
        outname=self.prefix+self.postfix
        field = '<7,3C273'
        self.res=sdmath(field=field,expr=self.expr,outfile=outname)
        tbsel = {'FIELDNAME': ['M100__5', 'M100__6', '3C273__8']}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    ####################
    # spw 
    ####################
    def test_spw_id_default(self):
        """test spw selection (spw='')"""
        outname=self.prefix+self.postfix
        spw=''
        self.res=sdmath(spw=spw,expr=self.expr,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, self.reffile)

    def test_spw_id_exact(self):
        """ test spw selection (spw='21')"""
        outname=self.prefix+self.postfix
        spw = '21'
        self.res=sdmath(spw=spw,expr=self.expr,outfile=outname)
        tbsel = {'IFNO': [21]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    def test_spw_id_lt(self):
        """ test spw selection (spw='<25')"""
        outname=self.prefix+self.postfix
        spw = '<25'
        self.res=sdmath(spw=spw,expr=self.expr,outfile=outname)
        tbsel = {'IFNO': [20,21,22,23,24]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    def test_spw_id_gt(self):
        """ test spw selection (spw='>21')"""
        outname=self.prefix+self.postfix
        spw = '>21'
        self.res=sdmath(spw=spw,expr=self.expr,outfile=outname)
        tbsel = {'IFNO': [22,23,24,25]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    def test_spw_id_range(self):
        """ test spw selection (spw='21~24')"""
        outname=self.prefix+self.postfix
        spw = '21~24'
        self.res=sdmath(spw=spw,expr=self.expr,outfile=outname)
        tbsel = {'IFNO': [21,22,23,24]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    def test_spw_id_list(self):
        """ test spw selection (spw='21,22,23,25')"""
        outname=self.prefix+self.postfix
        spw = '21,22,23,25'
        self.res=sdmath(spw=spw,expr=self.expr,outfile=outname)
        tbsel = {'IFNO': [21,22,23,25]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    def test_spw_id_exprlist(self):
        """ test spw selection (spw='<22,>24')"""
        outname=self.prefix+self.postfix
        spw = '<22,>24'
        self.res=sdmath(spw=spw,expr=self.expr,outfile=outname)
        tbsel = {'IFNO': [20,21,25]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    def test_spw_id_pattern(self):
        """test spw selection (spw='*')"""
        outname=self.prefix+self.postfix
        spw='*'
        self.res=sdmath(spw=spw,expr=self.expr,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, self.reffile)

    def test_spw_value_frequency(self):
        """test spw selection (spw='299.5~310GHz')"""
        outname=self.prefix+self.postfix
        spw = '299.5~310GHz'
        self.res=sdmath(spw=spw,expr=self.expr,outfile=outname)
        tbsel = {'IFNO': [22,23,24,25]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    def test_spw_value_velocity(self):
        """test spw selection (spw='-50~50km/s')"""
        outname=self.prefix+self.postfix
        spw = '-50~50km/s'
        self.res=sdmath(spw=spw,expr=self.expr,outfile=outname)
        tbsel = {'IFNO': [22,23]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    def test_spw_mix_exprlist(self):
        """test spw selection (spw='150~550km/s,>23')"""
        outname=self.prefix+self.postfix
        spw = '150~550km/s,>23'
        self.res=sdmath(spw=spw,expr=self.expr,outfile=outname)
        tbsel = {'IFNO': [20,21,24,25]}
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)
    
    ####################
    # Helper functions
    ####################
    def _comparecal_with_selection( self, name, tbsel={} ):
        self._checkfile(name)
        sp=self._getspectra_selected(name, {})
        spref=self._getspectra_selected(self.reffile, tbsel)

        self._checkshape( sp, spref )
        
        for irow in xrange(len(sp)):
            diff=self._diff(numpy.array(sp[irow]),numpy.array(spref[irow]))
            retval=numpy.all(diff<0.01)
            maxdiff=diff.max()
            self.assertEqual( retval, True,
                             msg='calibrated result is wrong (irow=%s): maxdiff=%s'%(irow,diff.max()) )
        del sp, spref

    def _getspectra_selected( self, name, tbsel={} ):
        """
        Returns an array of spectra in rows selected in table.
        
        name  : the name of scantable
        tbsel : a dictionary of table selection information.
                The key should be column name and the value should be
                a list of column values to select.
        """
        isthere=os.path.exists(name)
        self.assertEqual(isthere,True,
                         msg='file %s does not exist'%(name))        
        tb.open(name)
        sp = []
        if len(tbsel) == 0:
            for i in range(tb.nrows()):
                sp.append(tb.getcell('SPECTRA', i).tolist())
        else:
            command = ''
            for key, val in tbsel.items():
                if len(command) > 0:
                    command += ' AND '
                command += ('%s in %s' % (key, str(val)))
            newtb = tb.query(command)
            for i in range(newtb.nrows()):
                sp.append(newtb.getcell('SPECTRA', i).tolist())
            newtb.close()

        tb.close()
        return sp

class sdmath_test_flag(sdmath_unittest_base,unittest.TestCase):
    """
    Examine if flag information is handled properly.
    
    Data used for this test are sdmath_flagtest[12].asap (input)
    and sdmath_flagtest.asap.ref (reference data).
    The reference data are generated using the following expr parameter:
    expr='IN0+IN1' where IN0 and IN1 are sdmath_flagtest1.asap
    and sdmath_flagtest2.asap, respectively.

    """
    # Input and output names
    infile1='sdmath_flagtest1.asap'
    infile2='sdmath_flagtest2.asap'
    infiles = [infile1, infile2]
    reffile='sdmath_flagtest.asap.ref'
    prefix=sdmath_unittest_base.taskname+'TestFlag'
    postfix='.math.asap'

    expr = 'IN0+IN1'
    
    @property
    def task(self):
        return sdmath
    
    @property
    def spw_channel_selection(self):
        return False

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.infile1)):
            shutil.copytree(self.datapath+self.infile1, self.infile1)
        if (not os.path.exists(self.infile2)):
            shutil.copytree(self.datapath+self.infile2, self.infile2)
        if (not os.path.exists(self.reffile)):
            shutil.copytree(self.datapath+self.reffile, self.reffile)

        default(sdmath)

    def tearDown(self):
        if (os.path.exists(self.infile1)):
            shutil.rmtree(self.infile1)
        if (os.path.exists(self.infile2)):
            shutil.rmtree(self.infile2)
        if (os.path.exists(self.reffile)):
            shutil.rmtree(self.reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def _compare( self, name, ref ):
        self._checkfile( name )
        # reference data
        tb.open(ref)
        nrow0=tb.nrows()
        rsp=[]
        rcf=[]
        for irow in xrange(nrow0):
            rsp.append(tb.getcell('SPECTRA',irow))
            rcf.append(tb.getcell('FLAGTRA',irow))
        rrf=tb.getcol('FLAGROW')
        tb.close()
        # check shape
        tb.open(name)
        nrow=tb.nrows()
        self.assertEqual(nrow,nrow0,msg='number of rows mismatch')
        sp=[]
        cf=[]
        for irow in xrange(nrow):
            sp.append(tb.getcell('SPECTRA',irow))
            cf.append(tb.getcell('FLAGTRA',irow))
            self.assertEqual(len(sp[irow]),len(rsp[irow]),
                             msg='SPECTRA: number of channel mismatch in row%s'%(irow)) 
            self.assertEqual(len(cf[irow]),len(rcf[irow]),
                             msg='FLAGTRA: number of channel mismatch in row%s'%(irow)) 
        rf=tb.getcol('FLAGROW')
        tb.close()

        for irow in xrange(nrow):
            self.assertTrue(all(rsp[irow]==sp[irow]))
            self.assertTrue(all(rcf[irow]==cf[irow]))
        self.assertTrue(all(rrf==rf))

    def testflag00(self):
        """test flag handling"""
        outname=self.prefix+self.postfix
        self.res=sdmath(infiles=self.infiles,expr=self.expr,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, self.reffile)

def suite():
    return [sdmath_test0,sdmath_test1,sdmath_test2,
            sdmath_test3,sdmath_test4,
            sdmath_storageTest,sdmath_test_selection,
            sdmath_test_flag
            ]
