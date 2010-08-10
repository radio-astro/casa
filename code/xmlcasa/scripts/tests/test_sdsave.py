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
from sdsave import sdsave
import asap as sd

# Unit test of sdsave task.
# 
# Currently, the test only examine if supported types of data can be
# read, and if supporeted types of data can be written.
# 
# The test on data selection and data averaging will not be done.

###
# Base class for all testing classes
###
class sdsave_test_base:
    """
    Base class for testing classes.
    Implements several methods to compare the results.
    """
    basefile='OrionS_rawACSmod_cal2123.asap'
    summaryStr = None
    firstSpec = None
    nrow = None
    ifno = None
    cycleno = None
    scanno = None

    def _setAttributes(self):
        """
        Set summary string from the original data.
        """
        tb.open(self.basefile)
        #s=sd.scantable(self.basefile,False)
        #self.summaryStr=s._summary()
        #self.firstSpec=numpy.array(s._getspectrum(0))
        #self.nrow=s.nrow()
        self.firstSpec=tb.getcell('SPECTRA',0)
        self.nrow=tb.nrows()
        self.scanno=tb.getcell('SCANNO',0)
        self.ifno=tb.getcell('IFNO',0)
        self.cycleno=tb.getcell('CYCLENO',0)
        self.npol=tb.getkeyword('nPol')
        #del s
        tb.close()


    def _compare(self,filename):
        """
        Compare results

           - check number of rows
           - check first spectrum
        """
        [nrow,sp0] = self._get(filename)
        #casalog.post('nrow=%s'%nrow)
        #casalog.post('maxdiff=%s'%((abs(self.firstSpec-sp0)).max()))
        if nrow != self.nrow:
            return False
        if any((abs(self.firstSpec-sp0))>1.0e-6):
            return False
        return True

    def _get(self,filename):
        """
        """
        n=None
        st=filename.split('.')
        extension=st[-1]
        #casalog.post('filename='+filename)
        if extension == 'asap' or extension == 'ms' or extension == 'fits':
            s=sd.scantable(filename,False)
            n=s.nrow()
            sp=numpy.array(s._getspectrum(0))
            del s
        else:
            import commands
            wcout=commands.getoutput('ls '+st[0]+'*.txt'+' | wc')
            n=int(wcout.split()[0])*self.npol
            filein=st[0]+'_SCAN%d_CYCLE%d_IF%d.txt'%(self.scanno,self.cycleno,self.ifno)
            f=open(filein)
            sp=[]
            line = f.readline()
            while ( line != '' ):
                if line[0] != '#' and line[0] != 'x':
                    lines = line.split()
                    sp.append(float(lines[1]))
                line = f.readline()
            sp = numpy.array(sp)
            f.close()
        return [n,sp]            


###
# Test on bad parameter settings, data selection, data averaging, ...
###
class sdsave_test0(unittest.TestCase):
    """
    Test on data selection, data averaging...
    """
    # Input and output names
    sdfile='OrionS_rawACSmod_cal2123.asap'
    prefix='sdsaveTest0'
    outfile=prefix+'.asap'
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdsave/'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.sdfile)):
            shutil.copytree(self.datapath+self.sdfile, self.sdfile)

        default(sdsave)

    def tearDown(self):
        if (os.path.exists(self.sdfile)):
            shutil.rmtree(self.sdfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test00(self):
        """Test 000: Default parameters"""
        self.res=sdsave()
        self.assertFalse(self.res)
        
    def test01(self):
        """Test 001: Time averaging without weight"""
        self.res=sdsave(sdfile=self.sdfile,timeaverage=True,outfile=self.outfile)
        self.assertFalse(self.res)        

    def test02(self):
        """Test 002: Polarization averaging without weight"""
        self.res=sdsave(sdfile=self.sdfile,polaverage=True,outfile=self.outfile)
        self.assertFalse(self.res)        


###
# Test to read scantable and write various types of format
###
class sdsave_test1(unittest.TestCase,sdsave_test_base):
    """
    Read scantable data, write various types of format.
    """
    # Input and output names
    sdfile='OrionS_rawACSmod_cal2123.asap'
    prefix='sdsaveTest1'
    outfile0=prefix+'.asap'
    outfile1=prefix+'.ms'
    outfile2=prefix+'.fits'
    outfile3=prefix
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdsave/'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.sdfile)):
            shutil.copytree(self.datapath+self.sdfile, self.sdfile)
        if (not os.path.exists(self.basefile)):
            shutil.copytree(self.datapath+self.basefile, self.basefile)

        default(sdsave)
        self._setAttributes()

    def tearDown(self):
        if (os.path.exists(self.sdfile)):
            shutil.rmtree(self.sdfile)
        if (os.path.exists(self.basefile)):
            shutil.rmtree(self.basefile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test00(self):
        """Test 100: test to read scantable and to write as scantable"""
        self.res=sdsave(sdfile=self.sdfile,outfile=self.outfile0,outform='ASAP')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare(self.outfile0))

    def test01(self):
        """Test 101: test to read scantable and to write as MS"""
        self.res=sdsave(sdfile=self.sdfile,outfile=self.outfile1,outform='MS2')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare(self.outfile1))
        
    def test02(self):
        """Test 102: test to read scantable and to write as SDFITS"""
        self.res=sdsave(sdfile=self.sdfile,outfile=self.outfile2,outform='SDFITS')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare(self.outfile2))

    def test03(self):
        """Test 103: test to read scantable and to write as ASCII"""
        self.res=sdsave(sdfile=self.sdfile,outfile=self.outfile3,outform='ASCII')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare(self.outfile3))
        

###
# Test to read MS and write various types of format
###
class sdsave_test2(unittest.TestCase,sdsave_test_base):
    """
    Read MS data, write various types of format.
    """
    # Input and output names
    sdfile='OrionS_rawACSmod_cal2123.ms'
    prefix='sdsaveTest2'
    outfile0=prefix+'.asap'
    outfile1=prefix+'.ms'
    outfile2=prefix+'.fits'
    outfile3=prefix
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdsave/'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.sdfile)):
            shutil.copytree(self.datapath+self.sdfile, self.sdfile)
        if (not os.path.exists(self.basefile)):
            shutil.copytree(self.datapath+self.basefile, self.basefile)

        default(sdsave)
        self._setAttributes()
        self.scanno=0

    def tearDown(self):
        if (os.path.exists(self.sdfile)):
            shutil.rmtree(self.sdfile)
        if (os.path.exists(self.basefile)):
            shutil.rmtree(self.basefile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test00(self):
        """Test 200: test to read MS and to write as scantable"""
        self.res=sdsave(sdfile=self.sdfile,outfile=self.outfile0,outform='ASAP')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare(self.outfile0))
        
    def test01(self):
        """Test 201: test to read MS and to write as MS"""
        self.res=sdsave(sdfile=self.sdfile,outfile=self.outfile1,outform='MS2')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare(self.outfile1))
        
    def test02(self):
        """Test 202: test to read MS and to write as SDFITS"""
        self.res=sdsave(sdfile=self.sdfile,outfile=self.outfile2,outform='SDFITS')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare(self.outfile2))

    def test03(self):
        """Test 203: test to read MS and to write as ASCII"""
        self.res=sdsave(sdfile=self.sdfile,outfile=self.outfile3,outform='ASCII')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare(self.outfile3))
        

###
# Test to read ATNF SDFITS and write various types of format
###
class sdsave_test3(unittest.TestCase,sdsave_test_base):
    """
    Read ATNF SDFITS data, write various types of format.
    """
    # Input and output names
    sdfile='OrionS_rawACSmod_cal2123.fits'
    prefix='sdsaveTest3'
    outfile0=prefix+'.asap'
    outfile1=prefix+'.ms'
    outfile2=prefix+'.fits'
    outfile3=prefix
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdsave/'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.sdfile)):
            shutil.copy(self.datapath+self.sdfile, self.sdfile)
        if (not os.path.exists(self.basefile)):
            shutil.copytree(self.datapath+self.basefile, self.basefile)

        default(sdsave)
        self._setAttributes()
        self.scanno=0

    def tearDown(self):
        if (os.path.exists(self.sdfile)):
            os.system( 'rm -f '+self.sdfile )
        if (os.path.exists(self.basefile)):
            shutil.rmtree(self.basefile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test00(self):
        """Test 300: test to read ATNF SDFITS and to write as scantable"""
        self.res=sdsave(sdfile=self.sdfile,outfile=self.outfile0,outform='ASAP')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare(self.outfile0))

    def test01(self):
        """Test 301: test to read ATNF SDFITS and to write as MS"""
        self.res=sdsave(sdfile=self.sdfile,outfile=self.outfile1,outform='MS2')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare(self.outfile1))
        
    def test02(self):
        """Test 302: test to read ATNF SDFITS and to write as SDFITS"""
        self.res=sdsave(sdfile=self.sdfile,outfile=self.outfile2,outform='SDFITS')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare(self.outfile2))

    def test03(self):
        """Test 303: test to read ATNF SDFITS and to write as ASCII"""
        self.res=sdsave(sdfile=self.sdfile,outfile=self.outfile3,outform='ASCII')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare(self.outfile3))
        

###
# Test to read GBT SDFITS and write various types of format
###
class sdsave_test4(unittest.TestCase):
    """
    Read GBT SDFITS data, write various types of format.
    """
    # Input and output names
    sdfile='AGBT06A_sliced.fits'
    prefix='sdsaveTest4'
    outfile0=prefix+'.asap'
    outfile1=prefix+'.ms'
    outfile2=prefix+'.fits'
    outfile3=prefix
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdsave/'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.sdfile)):
            shutil.copy(self.datapath+self.sdfile, self.sdfile)

        default(sdsave)
        #self._setAttributes()

    def tearDown(self):
        if (os.path.exists(self.sdfile)):
            os.system( 'rm -f '+self.sdfile )
        os.system( 'rm -rf '+self.prefix+'*' )

    def test00(self):
        """Test 400: test to read GBT SDFITS and to write as scantable"""
        self.res=sdsave(sdfile=self.sdfile,outfile=self.outfile0,outform='ASAP')
        self.assertEqual(self.res,None)
        #self.assertTrue(self._compare(self.outfile0))
        self.assertTrue(self._compare())

    def test01(self):
        """Test 401: test to read GBT SDFITS and to write as MS"""
        self.res=sdsave(sdfile=self.sdfile,outfile=self.outfile1,outform='MS2')
        self.assertEqual(self.res,None)
        #self.assertTrue(self._compare(self.outfile1))
        
    def test02(self):
        """Test 402: test to read GBT SDFITS and to write as SDFITS"""
        self.res=sdsave(sdfile=self.sdfile,outfile=self.outfile2,outform='SDFITS')
        self.assertEqual(self.res,None)
        #self.assertTrue(self._compare(self.outfile2))

    def test03(self):
        """Test 403: test to read GBT SDFITS and to write as ASCII"""
        self.res=sdsave(sdfile=self.sdfile,outfile=self.outfile3,outform='ASCII')
        self.assertEqual(self.res,None)
        #self.assertTrue(self._compare(self.outfile3))

    def _compare(self,filename=''):
        """
        Check a few things for the data.
        """
        s=sd.scantable(self.sdfile,False)
        if ( s.nrow() != 48 ):
            return False
        if ( s.nif() != 6 ):
            return False
        if ( s.nchan(0) != 4096 ):
            return False
        if ( s.nchan(2) != 8192 ):
            return False
        if ( s.npol() != 1 ):
            return False
        return True
        
###
# Test to read NROFITS and write various types of format
###
class sdsave_test5(unittest.TestCase):
    """
    Read NROFITS data, write various types of format.
    """
    # Input and output names
    sdfile='B68test.nro'
    prefix='sdsaveTest5'
    outfile0=prefix+'.asap'
    outfile1=prefix+'.ms'
    outfile2=prefix+'.fits'
    outfile3=prefix
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdsave/'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.sdfile)):
            shutil.copy(self.datapath+self.sdfile, self.sdfile)

        default(sdsave)
        #self._setAttributes()

    def tearDown(self):
        if (os.path.exists(self.sdfile)):
            os.system( 'rm -f '+self.sdfile )
        os.system( 'rm -rf '+self.prefix+'*' )

    def test00(self):
        """Test 500: test to read NROFITS and to write as scantable"""
        self.res=sdsave(sdfile=self.sdfile,outfile=self.outfile0,outform='ASAP')
        self.assertEqual(self.res,None)
        #self.assertTrue(self._compare(self.outfile0))
        self.assertTrue(self._compare())

    def test01(self):
        """Test 501: test to read NROFITS and to write as MS"""
        self.res=sdsave(sdfile=self.sdfile,outfile=self.outfile1,outform='MS2')
        self.assertEqual(self.res,None)
        #self.assertTrue(self._compare(self.outfile1))
        
    def test02(self):
        """Test 502: test to read NROFITS and to write as SDFITS"""
        self.res=sdsave(sdfile=self.sdfile,outfile=self.outfile2,outform='SDFITS')
        self.assertEqual(self.res,None)
        #self.assertTrue(self._compare(self.outfile2))

    def test03(self):
        """Test 503: test to read NROFITS and to write as ASCII"""
        self.res=sdsave(sdfile=self.sdfile,outfile=self.outfile3,outform='ASCII')
        self.assertEqual(self.res,None)
        #self.assertTrue(self._compare(self.outfile3))
        
    def _compare(self,filename=''):
        """
        Check a few things for the data.
        """
        s=sd.scantable(self.sdfile,False)
        if ( s.nrow() != 36 ):
            return False
        if ( s.nif() != 4 ):
            return False
        if ( s.nchan() != 2048 ):
            return False
        if ( s.npol() != 1 ):
            return False
        return True        


def suite():
    return [sdsave_test0,sdsave_test1,sdsave_test2,
            sdsave_test3,sdsave_test4]
#    return [sdsave_test0,sdsave_test1,sdsave_test2,
#            sdsave_test3,sdsave_test4,sdsave_test5]
