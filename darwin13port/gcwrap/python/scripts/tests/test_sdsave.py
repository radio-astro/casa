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
class sdsave_unittest_base:
    """
    Base class for testing classes.
    Implements several methods to compare the results.
    """
    taskname='sdsave'
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdsave/'
    basefile='OrionS_rawACSmod_cal2123.asap'
    summaryStr = None
    firstSpec = None
    nrow = None
    ifno = None
    cycleno = None
    scanno = None

    def _checkfile( self, name ):
        isthere=os.path.exists(name)
        self.assertEqual(isthere,True,
                         msg='output file %s was not created because of the task failure'%(name))

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
            self._checkfile(filename)
            s=sd.scantable(filename,False)
            n=s.nrow()
            sp=numpy.array(s._getspectrum(0))
            del s
        else:
            import commands
            wcout=commands.getoutput('ls '+st[0]+'*.txt'+' | wc')
            n=int(wcout.split()[0])*self.npol
            filein=st[0]+'_SCAN%d_CYCLE%d_IF%d.txt'%(self.scanno,self.cycleno,self.ifno)
            self._checkfile(filein)
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
class sdsave_test0(unittest.TestCase,sdsave_unittest_base):
    """
    Test on data selection, data averaging...
    """
    # Input and output names
    infile='OrionS_rawACSmod_cal2123.asap'
    prefix=sdsave_unittest_base.taskname+'Test0'
    outfile=prefix+'.asap'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.infile)):
            shutil.copytree(self.datapath+self.infile, self.infile)

        default(sdsave)

    def tearDown(self):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test000(self):
        """Test 000: Default parameters"""
        # argument verification error
        self.res=sdsave()
        self.assertFalse(self.res)
        
    def test001(self):
        """Test 001: Time averaging without weight"""
        try:
            self.res=sdsave(infile=self.infile,timeaverage=True,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Please specify weight type of time averaging')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test002(self):
        """Test 002: Polarization averaging without weight"""
        try:
            self.res=sdsave(infile=self.infile,polaverage=True,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Please specify weight type of polarization averaging')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))


###
# Test to read scantable and write various types of format
###
class sdsave_test1(unittest.TestCase,sdsave_unittest_base):
    """
    Read scantable data, write various types of format.
    """
    # Input and output names
    infile='OrionS_rawACSmod_cal2123.asap'
    prefix=sdsave_unittest_base.taskname+'Test1'
    outfile0=prefix+'.asap'
    outfile1=prefix+'.ms'
    outfile2=prefix+'.fits'
    outfile3=prefix

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.infile)):
            shutil.copytree(self.datapath+self.infile, self.infile)
        if (not os.path.exists(self.basefile)):
            shutil.copytree(self.datapath+self.basefile, self.basefile)

        default(sdsave)
        self._setAttributes()

    def tearDown(self):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)
        if (os.path.exists(self.basefile)):
            shutil.rmtree(self.basefile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test100(self):
        """Test 100: test to read scantable and to write as scantable"""
        self.res=sdsave(infile=self.infile,outfile=self.outfile0,outform='ASAP')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare(self.outfile0))

    def test101(self):
        """Test 101: test to read scantable and to write as MS"""
        self.res=sdsave(infile=self.infile,outfile=self.outfile1,outform='MS2')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare(self.outfile1))
        
    def test102(self):
        """Test 102: test to read scantable and to write as SDFITS"""
        self.res=sdsave(infile=self.infile,outfile=self.outfile2,outform='SDFITS')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare(self.outfile2))

    def test103(self):
        """Test 103: test to read scantable and to write as ASCII"""
        self.res=sdsave(infile=self.infile,outfile=self.outfile3,outform='ASCII')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare(self.outfile3))
        

###
# Test to read MS and write various types of format
###
class sdsave_test2(unittest.TestCase,sdsave_unittest_base):
    """
    Read MS data, write various types of format.
    """
    # Input and output names
    infile='OrionS_rawACSmod_cal2123.ms'
    prefix=sdsave_unittest_base.taskname+'Test2'
    outfile0=prefix+'.asap'
    outfile1=prefix+'.ms'
    outfile2=prefix+'.fits'
    outfile3=prefix

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.infile)):
            shutil.copytree(self.datapath+self.infile, self.infile)
        if (not os.path.exists(self.basefile)):
            shutil.copytree(self.datapath+self.basefile, self.basefile)

        default(sdsave)
        self._setAttributes()
        self.scanno=1

    def tearDown(self):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)
        if (os.path.exists(self.basefile)):
            shutil.rmtree(self.basefile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test200(self):
        """Test 200: test to read MS and to write as scantable"""
        self.res=sdsave(infile=self.infile,outfile=self.outfile0,outform='ASAP')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare(self.outfile0))
        
    def test201(self):
        """Test 201: test to read MS and to write as MS"""
        self.res=sdsave(infile=self.infile,outfile=self.outfile1,outform='MS2')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare(self.outfile1))
        
    def test202(self):
        """Test 202: test to read MS and to write as SDFITS"""
        self.res=sdsave(infile=self.infile,outfile=self.outfile2,outform='SDFITS')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare(self.outfile2))

    def test203(self):
        """Test 203: test to read MS and to write as ASCII"""
        self.res=sdsave(infile=self.infile,outfile=self.outfile3,outform='ASCII')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare(self.outfile3))

    def test204(self):
        """Test 204: test failure case that unexisting antenna is specified"""
        try:
            self.res=sdsave(infile=self.infile,antenna='ROSWELL',outfile=self.outfile0,outform='ASAP')
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            # the task failed to import data so that failed to open output file
            pos=str(e).find('Failed to open file')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test205(self):
        """Test 205: test to read USB spectral window"""
        self.__spwtest()
        
    def test206(self):
        """Test 206: test to read LSB spectral window"""
        tb.open('%s/SPECTRAL_WINDOW'%(self.infile),nomodify=False)
        chanw = tb.getcol('CHAN_WIDTH')
        chanf = tb.getcol('CHAN_FREQ')
        chanw *= -1.0
        chanf = numpy.flipud(chanf)
        tb.putcol('CHAN_WIDTH',chanw)
        tb.putcol('CHAN_FREQ',chanf)
        netsb = numpy.ones( tb.nrows(), int )
        tb.putcol('NET_SIDEBAND', netsb )
        tb.close()
        self.__spwtest()

    def __spwtest(self):
        self.res=sdsave(infile=self.infile,outfile=self.outfile0,outform='ASAP')
        self.assertFalse(self.res,False)
        self.__compareIncrement( self.outfile0, self.infile )
        self.res=sdsave(infile=self.outfile0,outfile=self.outfile1,outform='MS2')
        self.assertFalse(self.res,False)
        self.__compareIncrement( self.outfile0, self.outfile1 )        

    def __compareIncrement(self,stdata,msdata):
        tb.open('%s/FREQUENCIES'%(stdata))
        incr=tb.getcol('INCREMENT')
        tb.close()
        tb.open('%s/SPECTRAL_WINDOW'%(msdata))
        chanw=tb.getcol('CHAN_WIDTH')
        tb.close()
        for i in xrange(len(incr)):
            #print 'incr[%s]=%s,chanw[0][%s]=%s(diff=%s)'%(i,incr[i],i,chanw[0][i],(incr[i]-chanw[0][i]))
            self.assertEqual(incr[i],chanw[0][i])
        
###
# Test to read ATNF SDFITS and write various types of format
###
class sdsave_test3(unittest.TestCase,sdsave_unittest_base):
    """
    Read ATNF SDFITS data, write various types of format.
    """
    # Input and output names
    infile='OrionS_rawACSmod_cal2123.fits'
    prefix=sdsave_unittest_base.taskname+'Test3'
    outfile0=prefix+'.asap'
    outfile1=prefix+'.ms'
    outfile2=prefix+'.fits'
    outfile3=prefix

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.infile)):
            shutil.copy(self.datapath+self.infile, self.infile)
        if (not os.path.exists(self.basefile)):
            shutil.copytree(self.datapath+self.basefile, self.basefile)

        default(sdsave)
        self._setAttributes()
        self.scanno=0

    def tearDown(self):
        if (os.path.exists(self.infile)):
            os.system( 'rm -f '+self.infile )
        if (os.path.exists(self.basefile)):
            shutil.rmtree(self.basefile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test300(self):
        """Test 300: test to read ATNF SDFITS and to write as scantable"""
        self.res=sdsave(infile=self.infile,outfile=self.outfile0,outform='ASAP')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare(self.outfile0))

    def test301(self):
        """Test 301: test to read ATNF SDFITS and to write as MS"""
        self.res=sdsave(infile=self.infile,outfile=self.outfile1,outform='MS2')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare(self.outfile1))
        
    def test302(self):
        """Test 302: test to read ATNF SDFITS and to write as SDFITS"""
        self.res=sdsave(infile=self.infile,outfile=self.outfile2,outform='SDFITS')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare(self.outfile2))

    def test303(self):
        """Test 303: test to read ATNF SDFITS and to write as ASCII"""
        self.res=sdsave(infile=self.infile,outfile=self.outfile3,outform='ASCII')
        self.assertEqual(self.res,None)
        self.assertTrue(self._compare(self.outfile3))
        

###
# Test to read GBT SDFITS and write various types of format
###
class sdsave_test4(unittest.TestCase,sdsave_unittest_base):
    """
    Read GBT SDFITS data, write various types of format.
    """
    # Input and output names
    infile='AGBT06A_sliced.fits'
    prefix=sdsave_unittest_base.taskname+'Test4'
    outfile0=prefix+'.asap'
    outfile1=prefix+'.ms'
    outfile2=prefix+'.fits'
    outfile3=prefix

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.infile)):
            shutil.copy(self.datapath+self.infile, self.infile)

        default(sdsave)
        #self._setAttributes()

    def tearDown(self):
        if (os.path.exists(self.infile)):
            os.system( 'rm -f '+self.infile )
        os.system( 'rm -rf '+self.prefix+'*' )

    def test400(self):
        """Test 400: test to read GBT SDFITS and to write as scantable"""
        self.res=sdsave(infile=self.infile,outfile=self.outfile0,outform='ASAP')
        self.assertEqual(self.res,None)
        #self.assertTrue(self._compare(self.outfile0))
        self.assertTrue(self._compare())

    def test401(self):
        """Test 401: test to read GBT SDFITS and to write as MS"""
        self.res=sdsave(infile=self.infile,outfile=self.outfile1,outform='MS2')
        self.assertEqual(self.res,None)
        #self.assertTrue(self._compare(self.outfile1))
        
    def test402(self):
        """Test 402: test to read GBT SDFITS and to write as SDFITS"""
        self.res=sdsave(infile=self.infile,outfile=self.outfile2,outform='SDFITS')
        self.assertEqual(self.res,None)
        #self.assertTrue(self._compare(self.outfile2))

    def test403(self):
        """Test 403: test to read GBT SDFITS and to write as ASCII"""
        self.res=sdsave(infile=self.infile,outfile=self.outfile3,outform='ASCII')
        self.assertEqual(self.res,None)
        #self.assertTrue(self._compare(self.outfile3))

    def _compare(self,filename=''):
        """
        Check a few things for the data.
        """
        s=sd.scantable(self.infile,False)
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
class sdsave_test5(unittest.TestCase,sdsave_unittest_base):
    """
    Read NROFITS data, write various types of format.
    """
    # Input and output names
    infile='B68test.nro'
    prefix=sdsave_unittest_base.taskname+'Test5'
    outfile0=prefix+'.asap'
    outfile1=prefix+'.ms'
    outfile2=prefix+'.fits'
    outfile3=prefix

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.infile)):
            shutil.copy(self.datapath+self.infile, self.infile)

        default(sdsave)
        #self._setAttributes()

    def tearDown(self):
        if (os.path.exists(self.infile)):
            os.system( 'rm -f '+self.infile )
        os.system( 'rm -rf '+self.prefix+'*' )

    def test500(self):
        """Test 500: test to read NROFITS and to write as scantable"""
        self.res=sdsave(infile=self.infile,outfile=self.outfile0,outform='ASAP')
        self.assertEqual(self.res,None)
        #self.assertTrue(self._compare(self.outfile0))
        self.assertTrue(self._compare())

    def test501(self):
        """Test 501: test to read NROFITS and to write as MS"""
        self.res=sdsave(infile=self.infile,outfile=self.outfile1,outform='MS2')
        self.assertEqual(self.res,None)
        #self.assertTrue(self._compare(self.outfile1))
        
    def test502(self):
        """Test 502: test to read NROFITS and to write as SDFITS"""
        self.res=sdsave(infile=self.infile,outfile=self.outfile2,outform='SDFITS')
        self.assertEqual(self.res,None)
        #self.assertTrue(self._compare(self.outfile2))

    def test503(self):
        """Test 503: test to read NROFITS and to write as ASCII"""
        self.res=sdsave(infile=self.infile,outfile=self.outfile3,outform='ASCII')
        self.assertEqual(self.res,None)
        #self.assertTrue(self._compare(self.outfile3))
        
    def _compare(self,filename=''):
        """
        Check a few things for the data.
        """
        s=sd.scantable(self.infile,False)
        if ( s.nrow() != 36 ):
            return False
        if ( s.nif() != 4 ):
            return False
        if ( s.nchan() != 2048 ):
            return False
        if ( s.npol() != 1 ):
            return False
        return True        


###
# Test getpt parameter
###
class sdsave_test6( unittest.TestCase, sdsave_unittest_base ):
    """
    Test getpt parameter

    1) import MS to Scantable format with getpt=True 
       1-1) check POINTING table keyword is missing
       1-2) export Scantable to MS format
       1-3) compare POINTING table
    2) import MS to Scantable format with getpt=False
       1-1) check POINTING table keyword exists
       1-2) export Scantable to MS format
       1-3) compare POINTING table

    """
    # Input and output names
    infile='OrionS_rawACSmod_cal2123.ms'
    prefix=sdsave_unittest_base.taskname+'Test6'
    outfile0=prefix+'.asap'
    outfile1=prefix+'.ms'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.infile)):
            shutil.copytree(self.datapath+self.infile, self.infile)

        default(sdsave)
        #self._setAttributes()

    def tearDown(self):
        if (os.path.exists(self.infile)):
            os.system( 'rm -rf '+self.infile )
        os.system( 'rm -rf '+self.prefix+'*' )

    def test600(self):
        """Test 600: test getpt=True"""
        self.res=sdsave(infile=self.infile,getpt=True,outfile=self.outfile0,outform='ASAP')
        self.assertEqual(self.res,None)
        self.assertFalse(self._pointingKeywordExists())
        self.res=sdsave(infile=self.outfile0,outfile=self.outfile1,outform='MS2')
        self.assertTrue(self._compare())

    def test601(self):
        """Test 601: test getpt=False"""
        self.res=sdsave(infile=self.infile,getpt=False,outfile=self.outfile0,outform='ASAP')
        self.assertEqual(self.res,None)
        self.assertTrue(self._pointingKeywordExists())
        self.res=sdsave(infile=self.outfile0,outfile=self.outfile1,outform='MS2')
        self.assertTrue(self._compare())

    def _pointingKeywordExists(self):
        _tb=tbtool()
        _tb.open(self.outfile0)
        keys=_tb.getkeywords()
        _tb.close()
        del _tb
        return 'POINTING' in keys

    def _compare(self):
        ret = True
        _tb1=tbtool()
        _tb2=tbtool()
        _tb1.open(self.infile)
        #ptab1=_tb1.getkeyword('POINTING').split()[-1]
        ptab1=_tb1.getkeyword('POINTING').lstrip('Table: ')
        _tb1.close()
        _tb1.open(ptab1)
        _tb2.open(self.outfile1)
        #ptab2=_tb2.getkeyword('POINTING').split()[-1]
        ptab2=_tb2.getkeyword('POINTING').lstrip('Table: ')
        _tb2.close()
        _tb2.open(ptab1)
        badcols = []
        for col in _tb1.colnames():
            if not all(_tb1.getcol(col).flatten()==_tb2.getcol(col).flatten()):
                badcols.append( col )
        _tb1.close()
        _tb2.close()
        del _tb1, _tb2
        if len(badcols) != 0:
            print 'Bad column: %s'%(badcols)
            ret = False
        return ret

###
# Test rest frequency
###
class sdsave_test7( sdsave_unittest_base, unittest.TestCase ):
    """
    Unit tests for task sdsave. Test scantable available restfreq

    The list of tests:
    test701-704  --- a value (float, int, quantity w/ and w/o a unit)
    test711-715  --- a list [length=1]
                     (float, int, quantity w/ and w/o a unit, dict)
    test721-725  --- a list [length>1]
                     (float, int, quantity w/ and w/o a unit, dict)
    test731-733  --- a bad quantity unit (a value, a list)
    """
    # Input and output names
    infile = 'OrionS_rawACSmod_cal2123.asap'
    outname = sdsave_unittest_base.taskname+'_test'
    iflist = [1,2]
    frf = [45.301e9,44.075e9]
    irf = [45301000000,44075000000]
    qurf = ['45301.MHz','44.075GHz']
    qrf = [str(frf[0]), str(irf[1])]
    drf = [{'name': "IF1 Rest", 'value': frf[0]}, \
           {'name': "IF2 Rest", 'value': qurf[1]}]
    badq = ['45301.km','44.075bad']

    def setUp( self ):
        # copy input scantables
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)

        default(sdsave)

    def tearDown( self ):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)


    # Helper functions for testing
    def _check_restfreq( self, outfile, restfreq, iflist ):
        self._checkfile(outfile)
        #print "To be checked 0 : ifnos =", iflist, ", restfreqs = ", restfreq
        if not (type(restfreq) in (list, tuple, numpy.ndarray)):
            restfreq = [restfreq] * len(iflist)
        elif len(restfreq) == 1:
            restfreq = restfreq * len(iflist)
        #print "To be checked: ifnos =", iflist, ", restfreqs = ", restfreq
        scan = sd.scantable(outfile, average = False)
        for i in range(len(restfreq)):
            ifno = iflist[i]
            rf = restfreq[i]
            if type(rf) == dict:
                rf = rf['value']
            if qa.isquantity(rf):
                rfv = qa.convert(rf,'Hz')['value']
            else:
                rfv = float(rf)
            scan.set_selection(ifs=[ifno])
            molid = scan.getmolnos()
            self.assertTrue(len(molid)==1,msg="IFNO=%d has multiple MOLECULE_IDs: %s" % (ifno,str(molid)))
            newrf = scan.get_restfreqs(molid[0])
            #self.assertTrue(len(newrf)==1,msg="IFNO=%d has multiple rest frequencies: %s" % (ifno,str(newrf)))
            self.assertEqual(newrf[0], rfv,\
                             msg="Rest frequency differs(IFNO=%d): %f (expected: %f)" % (ifno, newrf[0], rfv))
            print "Rest frequency (IFNO=%d): %f (expected: %f)" % (ifno, newrf[0], rfv)
    
    # Actual tests
    def test701( self ):
        """Test 701: restfreq (a float value)"""
        tid = "701"
        infile = self.infile
        outfile = self.outname+tid
        iflist = self.iflist
        restfreq = self.frf[1]

        print "Setting restfreq = %s (%s)" % (str(restfreq), str(type(restfreq)))
        result = sdsave(infile=infile,outfile=outfile,\
                        scanaverage=False,timeaverage=False,polaverage=False,\
                        iflist=iflist,restfreq=restfreq)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        print "Testing rest frequencies of output scantable"
        self._check_restfreq(outfile, restfreq, iflist)

    def test702( self ):
        """Test 702: restfreq (an int value)"""
        tid = "702"
        infile = self.infile
        outfile = self.outname+tid
        iflist = self.iflist
        restfreq = self.irf[1]

        print "Setting restfreq = %s (%s)" % (str(restfreq), str(type(restfreq)))
        result = sdsave(infile=infile,outfile=outfile,\
                        scanaverage=False,timeaverage=False,polaverage=False,\
                        iflist=iflist,restfreq=restfreq)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        print "Testing rest frequencies of output scantable"
        self._check_restfreq(outfile, restfreq, iflist)

    def test703( self ):
        """Test 703: restfreq (a quantity with unit)"""
        tid = "703"
        infile = self.infile
        outfile = self.outname+tid
        iflist = self.iflist
        restfreq = self.qurf[1]

        print "Setting restfreq = %s" % (str(restfreq))
        result = sdsave(infile=infile,outfile=outfile,\
                        scanaverage=False,timeaverage=False,polaverage=False,\
                        iflist=iflist,restfreq=restfreq)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        print "Testing rest frequencies of output scantable"
        self._check_restfreq(outfile, restfreq, iflist)

    def test704( self ):
        """Test 704: restfreq (a quantity withOUT unit)"""
        tid = "704"
        infile = self.infile
        outfile = self.outname+tid
        iflist = self.iflist
        restfreq = self.qrf[1]

        print "Setting restfreq = %s (%s)" % (str(restfreq), str(type(restfreq)))
        result = sdsave(infile=infile,outfile=outfile,\
                        scanaverage=False,timeaverage=False,polaverage=False,\
                        iflist=iflist,restfreq=restfreq)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        print "Testing rest frequencies of output scantable"
        self._check_restfreq(outfile, restfreq, iflist)

    def test711( self ):
        """Test 711: restfreq (a list of float value [length = 1])"""
        tid = "711"
        infile = self.infile
        outfile = self.outname+tid
        iflist = self.iflist
        restfreq = [ self.frf[1] ]

        print "Setting restfreq = %s" % (str(restfreq))
        result = sdsave(infile=infile,outfile=outfile,\
                        scanaverage=False,timeaverage=False,polaverage=False,\
                        iflist=iflist,restfreq=restfreq)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        print "Testing rest frequencies of output scantable"
        self._check_restfreq(outfile, restfreq, iflist)

    def test712( self ):
        """Test 712: restfreq (a list of int value [length = 1])"""
        tid = "712"
        infile = self.infile
        outfile = self.outname+tid
        iflist = self.iflist
        restfreq = [ self.irf[1] ]

        print "Setting restfreq = %s" % (str(restfreq))
        result = sdsave(infile=infile,outfile=outfile,\
                        scanaverage=False,timeaverage=False,polaverage=False,\
                        iflist=iflist,restfreq=restfreq)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        print "Testing rest frequencies of output scantable"
        self._check_restfreq(outfile, restfreq, iflist)

    def test713( self ):
        """Test 713: restfreq (a list of quantity with unit [length = 1])"""
        tid = "713"
        infile = self.infile
        outfile = self.outname+tid
        iflist = self.iflist
        restfreq = [ self.qurf[1] ]

        print "Setting restfreq = %s" % (str(restfreq))
        result = sdsave(infile=infile,outfile=outfile,\
                        scanaverage=False,timeaverage=False,polaverage=False,\
                        iflist=iflist,restfreq=restfreq)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        print "Testing rest frequencies of output scantable"
        self._check_restfreq(outfile, restfreq, iflist)

    def test714( self ):
        """Test 714: restfreq (a list of quantity withOUT unit [length = 1])"""
        tid = "714"
        infile = self.infile
        outfile = self.outname+tid
        iflist = self.iflist
        restfreq = [ self.qrf[1] ]

        print "Setting restfreq = %s" % (str(restfreq))
        result = sdsave(infile=infile,outfile=outfile,\
                        scanaverage=False,timeaverage=False,polaverage=False,\
                        iflist=iflist,restfreq=restfreq)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        print "Testing rest frequencies of output scantable"
        self._check_restfreq(outfile, restfreq, iflist)

    def test715( self ):
        """Test 715: restfreq (a list of dictionary [length = 1])"""
        tid = "715"
        infile = self.infile
        outfile = self.outname+tid
        iflist = self.iflist
        restfreq = [ self.drf[1] ]

        print "Setting restfreq = %s" % (str(restfreq))
        result = sdsave(infile=infile,outfile=outfile,\
                        scanaverage=False,timeaverage=False,polaverage=False,\
                        iflist=iflist,restfreq=restfreq)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        print "Testing rest frequencies of output scantable"
        self._check_restfreq(outfile, restfreq, iflist)

    def test721( self ):
        """Test 721: restfreq (a list of float value [length > 1])"""
        tid = "721"
        infile = self.infile
        outfile = self.outname+tid
        iflist = self.iflist
        restfreq = self.frf

        print "Setting restfreq = %s" % (str(restfreq))
        result = sdsave(infile=infile,outfile=outfile,\
                        scanaverage=False,timeaverage=False,polaverage=False,\
                        iflist=iflist,restfreq=restfreq)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        print "Testing rest frequencies of output scantable"
        self._check_restfreq(outfile, restfreq, iflist)

    def test722( self ):
        """Test 722: restfreq (a list of int value [length > 1])"""
        tid = "722"
        infile = self.infile
        outfile = self.outname+tid
        iflist = self.iflist
        restfreq = self.irf

        print "Setting restfreq = %s" % (str(restfreq))
        result = sdsave(infile=infile,outfile=outfile,\
                        scanaverage=False,timeaverage=False,polaverage=False,\
                        iflist=iflist,restfreq=restfreq)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        print "Testing rest frequencies of output scantable"
        self._check_restfreq(outfile, restfreq, iflist)

    def test723( self ):
        """Test 723: restfreq (a list of quantity with unit [length > 1])"""
        tid = "723"
        infile = self.infile
        outfile = self.outname+tid
        iflist = self.iflist
        restfreq = self.qurf

        print "Setting restfreq = %s" % (str(restfreq))
        result = sdsave(infile=infile,outfile=outfile,\
                        scanaverage=False,timeaverage=False,polaverage=False,\
                        iflist=iflist,restfreq=restfreq)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        print "Testing rest frequencies of output scantable"
        self._check_restfreq(outfile, restfreq, iflist)

    def test724( self ):
        """Test 724: restfreq (a list of quantity withOUT unit [length > 1])"""
        tid = "724"
        infile = self.infile
        outfile = self.outname+tid
        iflist = self.iflist
        restfreq = self.qrf

        print "Setting restfreq = %s" % (str(restfreq))
        result = sdsave(infile=infile,outfile=outfile,\
                        scanaverage=False,timeaverage=False,polaverage=False,\
                        iflist=iflist,restfreq=restfreq)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        print "Testing rest frequencies of output scantable"
        self._check_restfreq(outfile, restfreq, iflist)

    def test725( self ):
        """Test 725: restfreq (a list of dictionary [length > 1])"""
        tid = "725"
        infile = self.infile
        outfile = self.outname+tid
        iflist = self.iflist
        restfreq = self.drf

        print "Setting restfreq = %s" % (str(restfreq))
        result = sdsave(infile=infile,outfile=outfile,\
                        scanaverage=False,timeaverage=False,polaverage=False,\
                        iflist=iflist,restfreq=restfreq)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        print "Testing rest frequencies of output scantable"
        self._check_restfreq(outfile, restfreq[0], iflist)

    def test731( self ):
        """Test 731: restfreq (a BAD quantity unit)"""
        tid = "731"
        infile = self.infile
        outfile = self.outname+tid
        iflist = self.iflist
        restfreq = self.badq[0]

        print "Setting restfreq = %s" % (str(restfreq))
        try:
            result = sdsave(infile=infile,outfile=outfile,\
                                scanaverage=False,timeaverage=False,polaverage=False,\
                                iflist=iflist,restfreq=restfreq)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('wrong unit of restfreq')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test732( self ):
        """Test 732: restfreq (a list of BAD quantity unit [length = 1])"""
        tid = "732"
        infile = self.infile
        outfile = self.outname+tid
        iflist = self.iflist
        restfreq = [ self.badq[1] ]

        print "Setting restfreq = %s" % (str(restfreq))
        try:
            result = sdsave(infile=infile,outfile=outfile,\
                                scanaverage=False,timeaverage=False,polaverage=False,\
                                iflist=iflist,restfreq=restfreq)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Input value is not a quantity: ')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))


    def test733( self ):
        """Test 733: restfreq (a list of BAD quantity unit [length > 1])"""
        tid = "733"
        infile = self.infile
        outfile = self.outname+tid
        iflist = self.iflist
        restfreq = self.badq

        print "Setting restfreq = %s" % (str(restfreq))
        try:
            result = sdsave(infile=infile,outfile=outfile,\
                                scanaverage=False,timeaverage=False,polaverage=False,\
                                iflist=iflist,restfreq=restfreq)

            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('wrong unit of restfreq')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))


###
# Test combinations of scantable.storage='disk'/'memory' and insitu=T/F
###
class sdsave_storageTest( sdsave_unittest_base, unittest.TestCase ):
    """
    Unit tests for task sdsave. Test scantable sotrage and insitu
    parameters

    The list of tests:
    testMT  --- storage = 'memory', insitu = True
    testMF  --- storage = 'memory', insitu = False
    testDT  --- storage = 'disk', insitu = True
    testDF  --- storage = 'disk', insitu = False

    Note on handlings of disk storage:
       Task script restores MOLECULE_ID column.

    Tested items:
       1. Number of rows in tables and list of IDs of output scantable.
       2. Units and coordinates of output scantable.
       3. units and coordinates of input scantables before/after run.
    """
    # Input and output names
    infile = 'OrionS_rawACSmod_cal2123.asap'
    outname = sdsave_unittest_base.taskname+'_test'
    pollist = [1]
    iflist = [2]
    restfreq = [44.075e9]
    # Reference data of output scantable
    refout = {"nRow": 8, "SCANNOS": [21,23], "POLNOS": pollist,\
              "IFNOS": iflist, "MOLNOS": [1], "RestFreq": restfreq}

    def setUp( self ):
        # copy input scantables
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)
        # back up the original settings
        self.storage = sd.rcParams['scantable.storage']
        self.insitu = sd.rcParams['insitu']

        default(sdsave)

    def tearDown( self ):
        # restore settings
        sd.rcParams['scantable.storage'] = self.storage
        sd.rcParams['insitu'] = self.insitu
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)


    # Helper functions for testing
    def _get_scantable_params( self, scanname ):
        self._checkfile(scanname)
        res = {}
        testvals = ["scannos", "polnos", "ifnos", "molnos"]
        scan = sd.scantable(scanname,average=False)
        res['nRow'] = scan.nrow()
        for val in testvals:
            res[val.upper()] =  getattr(scan,"get"+val)()
        # rest frequencies
        rflist = []
        for molno in res["MOLNOS"]:
            rflist.append(scan.get_restfreqs(molno)[0])
        res["RestFreq"] = rflist
        del scan
        return res

    def _compare_scantable_params( self, test , refval):
        if type(test) == str:
            testval = self._get_scantable_params(test)
        elif type(test) == dict:
            testval = test
        else:
            msg = "Invalid test value (should be either dict or file name)."
            raise Exception, msg
        #print "Test data = ", testval
        #print "Ref data =  ", refval
        if not type(refval) == dict:
            raise Exception, "The reference data should be a dictionary"
        for key, rval in refval.iteritems():
            if not testval.has_key(key):
                raise KeyError, "Test data does not have key, '%s'" % key
            if type(rval) in [list, tuple, numpy.ndarray]:
                self.assertEqual(len(testval[key]), len(rval), \
                                 msg = "Number of elements in '%s' differs." % key)
                for i in range(len(rval)):
                    rv = rval[i]
                    if type(rv) == float:
                        self.assertAlmostEqual(testval[key][i], rv, \
                                               msg = "%s[%d] differs: %s (expected: %s) "\
                                               % (key, i, str(testval[key][i]), str(rv)))
                    else:
                        self.assertEqual(testval[key][i], rv, \
                                         msg = "%s[%d] differs: %s (expected: %s) "\
                                         % (key, i, str(testval[key][i]), str(rv)))
            else:
                if type(rval) == float:
                    self.assertAlmostEqual(testval[key], rval, \
                                     msg = "%s differs: %s (expected: %s)" \
                                     % (key, str(testval[key]), rval))
                else:
                    self.assertEqual(testval[key], rval, \
                                     msg = "%s differs: %s (expected: %s)" \
                                     % (key, str(testval[key]), rval))
    

    # Actual tests
    def testMT( self ):
        """Storage Test MT: sdsave on storage='memory' and insitu=T"""
        tid = "MT"
        infile = self.infile
        outfile = self.outname+tid
        iflist = self.iflist
        pollist = self.pollist
        restfreq = self.restfreq

        # Backup units and coords of input scantable before run.
        initval = self._get_scantable_params(infile)

        sd.rcParams['scantable.storage'] = 'memory'
        sd.rcParams['insitu'] = True
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        result = sdsave(infile=infile,outfile=outfile,\
                        scanaverage=False,timeaverage=False,polaverage=False,\
                        iflist=iflist,pollist=pollist,restfreq=restfreq)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        print "Testing output scantable"
        self._compare_scantable_params(outfile,self.refout)

        print "Comparing input scantable before/after run"
        self._compare_scantable_params(infile,initval)


    def testMF( self ):
        """Storage Test MF: sdsave on storage='memory' and insitu=F"""
        tid = "MF"
        infile = self.infile
        outfile = self.outname+tid
        iflist = self.iflist
        pollist = self.pollist
        restfreq = self.restfreq

        # Backup units and coords of input scantable before run.
        initval = self._get_scantable_params(infile)

        sd.rcParams['scantable.storage'] = 'memory'
        sd.rcParams['insitu'] = False
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        result = sdsave(infile=infile,outfile=outfile,\
                        scanaverage=False,timeaverage=False,polaverage=False,\
                        iflist=iflist,pollist=pollist,restfreq=restfreq)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        print "Testing output scantable"
        self._compare_scantable_params(outfile,self.refout)

        print "Comparing input scantable before/after run"
        self._compare_scantable_params(infile,initval)


    def testDT( self ):
        """Storage Test DT: sdsave on storage='disk' and insitu=T"""
        tid = "DT"
        infile = self.infile
        outfile = self.outname+tid
        iflist = self.iflist
        pollist = self.pollist
        restfreq = self.restfreq

        # Backup units and coords of input scantable before run.
        initval = self._get_scantable_params(infile)

        sd.rcParams['scantable.storage'] = 'disk'
        sd.rcParams['insitu'] = True
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        result = sdsave(infile=infile,outfile=outfile,\
                        scanaverage=False,timeaverage=False,polaverage=False,\
                        iflist=iflist,pollist=pollist,restfreq=restfreq)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        print "Testing output scantable"
        self._compare_scantable_params(outfile,self.refout)

        print "Comparing input scantable before/after run"
        self._compare_scantable_params(infile,initval)


    def testDF( self ):
        """Storage Test DF: sdsave on storage='disk' and insitu=F"""
        tid = "DF"
        infile = self.infile
        outfile = self.outname+tid
        iflist = self.iflist
        pollist = self.pollist
        restfreq = self.restfreq

        # Backup units and coords of input scantable before run.
        initval = self._get_scantable_params(infile)

        sd.rcParams['scantable.storage'] = 'disk'
        sd.rcParams['insitu'] = False
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        result = sdsave(infile=infile,outfile=outfile,\
                        scanaverage=False,timeaverage=False,polaverage=False,\
                        iflist=iflist,pollist=pollist,restfreq=restfreq)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        print "Testing output scantable"
        self._compare_scantable_params(outfile,self.refout)

        print "Comparing input scantable before/after run"
        self._compare_scantable_params(infile,initval)

###
# Test for frequency labeling
###
class sdsave_freq_labeling(unittest.TestCase,sdsave_unittest_base):
    """
    Read MS data, write various types of format.
    """
    # Input and output names
    infile='OrionS_rawACSmod_cal2123.ms'
    prefix=sdsave_unittest_base.taskname+'Test2'
    outfile=prefix+'.ms'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.infile)):
            shutil.copytree(self.datapath+self.infile, self.infile)
        if (not os.path.exists(self.basefile)):
            shutil.copytree(self.datapath+self.basefile, self.basefile)

        default(sdsave)
        self._setAttributes()
        self.scanno=0

    def tearDown(self):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)
        if (os.path.exists(self.basefile)):
            shutil.rmtree(self.basefile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test_freq_labeling01(self):
        """test_freq_labeling01: test for frequency labeling, by importing MS and exporting data to MS and compare frequency label with original MS"""
        # edit SPECTRAL_WINDOW table
        tb.open(self.infile+'/SPECTRAL_WINDOW',nomodify=False)
        mfr=tb.getcol('MEAS_FREQ_REF')
        mfr[:]=5 # TOPO
        cf_ref=tb.getvarcol('CHAN_FREQ')
        tb.putcol('MEAS_FREQ_REF',mfr)
        tb.close()
        
        self.res=sdsave(infile=self.infile,outfile=self.outfile,outform='MS2')
        self.assertEqual(self.res,None)

        # compare frequency label
        tb.open(self.outfile+'/SPECTRAL_WINDOW')
        cf=tb.getvarcol('CHAN_FREQ')
        tb.close()

        for k in cf.keys():
            v=cf[k]
            r=cf_ref[k]
            maxdiff=abs((v-r)/r).max()
            self.assertEqual(len(v), len(r),
                             msg="spw %s: length mismatch"%(int(k[1:])))
            self.assertTrue(maxdiff < 1.0e-15,
                            msg="spw %s: frequency labels differ"%(int(k[1:])))

###
# Test for handling flags in MSWriter
###
class sdsave_flagging(unittest.TestCase,sdsave_unittest_base):
    """
    Read Scantable data, modify flags in various ways, and write as MS.
    """
    # Input and output names
    infile='OrionS_rawACSmod_cal2123.asap'
    prefix=sdsave_unittest_base.taskname+'Test2'
    outfile=prefix+'.ms'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.infile)):
            shutil.copytree(self.datapath+self.infile, self.infile)
        if (not os.path.exists(self.basefile)):
            shutil.copytree(self.datapath+self.basefile, self.basefile)

        default(sdsave)
        self._setAttributes()
        self.scanno=0

    def tearDown(self):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)
        if (os.path.exists(self.basefile)):
            shutil.rmtree(self.basefile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test_noflag(self):
        """
        test_noflag: test for unflagged data. All the flags must be False.
        """
        # execute task
        sdsave(infile=self.infile, outfile=self.outfile, outform='MS2')

        # verification
        try:
            tb.open(self.outfile)
            for irow in xrange(tb.nrows()):
                flag_row = tb.getcell('FLAG_ROW', irow)
                flag = tb.getcell('FLAG', irow)
                self.assertFalse(flag_row,
                                 msg='FLAG_ROW must be False in row %s'%(irow))
                self.assertTrue(all(flag.flatten()==False),
                                msg='all FLAG values must be False in row %s'%(irow))
        finally:
            tb.close()
        
    def test_channelflag(self):
        """
        test_channelflag: test for channel flagged data. All the flags must be transferred properly.
        """
        # put channel flag to the first row
        tb.open(self.infile, nomodify=False)
        flagtra = tb.getcell('FLAGTRA', 0)
        flagtra[:] = 1
        tb.putcell('FLAGTRA', 0, flagtra)
        tb.close()

        # execute task
        sdsave(infile=self.infile, outfile=self.outfile, outform='MS2')

        # verification
        try:
            tb.open(self.outfile)
            # all channels in first row must be flagged
            irow = 0
            flag_row = tb.getcell('FLAG_ROW', irow)
            flag = tb.getcell('FLAG', irow)
            self.assertFalse(flag_row,
                             msg='FLAG_ROW must be False in row %s'%(irow))
            self.assertTrue(all(flag[0]==True),
                            msg='FLAG values for first polarization must be True in row %s'%(irow))
            self.assertTrue(all(flag[1:].flatten()==False),
                            msg='FLAG values for other polarizations must be False in row %s'%(irow))
            for irow in xrange(1, tb.nrows()):
                flag_row = tb.getcell('FLAG_ROW', irow)
                flag = tb.getcell('FLAG', irow)
                self.assertFalse(flag_row,
                                 msg='FLAG_ROW must be False in row %s'%(irow))
                self.assertTrue(all(flag.flatten()==False),
                                msg='all FLAG values must be False in row %s'%(irow))
        finally:
            tb.close()        

    def test_rowflag(self):
        """
        test_rowflag: test for row flagged data. In this case, channel flags have to be set while row flags should not be set. 
        """
        # put row flag to the first row
        tb.open(self.infile, nomodify=False)
        tb.putcell('FLAGROW', 0, 1)
        tb.close()        
        
        # execute task
        sdsave(infile=self.infile, outfile=self.outfile, outform='MS2')

        # verification
        try:
            tb.open(self.outfile)
            # all channels in first row must be flagged
            irow = 0
            flag_row = tb.getcell('FLAG_ROW', irow)
            flag = tb.getcell('FLAG', irow)
            self.assertFalse(flag_row,
                            msg='FLAG_ROW must be False in row %s'%(irow))
            self.assertTrue(all(flag[0]==True),
                            msg='FLAG values for first polarization must be True in row %s'%(irow))
            self.assertTrue(all(flag[1:].flatten()==False),
                            msg='FLAG values for other polarizations must be False in row %s'%(irow))

            for irow in xrange(1, tb.nrows()):
                flag_row = tb.getcell('FLAG_ROW', irow)
                flag = tb.getcell('FLAG', irow)
                self.assertFalse(flag_row,
                                 msg='FLAG_ROW must be False in row %s'%(irow))
                self.assertTrue(all(flag.flatten()==False),
                                msg='all FLAG values must be False in row %s'%(irow))
        finally:
            tb.close()        

###
# Test for scan number (CAS-5841)
###
class sdsave_scan_number(unittest.TestCase,sdsave_unittest_base):
    """
    Check scan number
    """
    # Input and output names
    infile='OrionS_rawACSmod_cal2123.asap'
    vis='OrionS_rawACSmod_cal2123.ms'
    prefix=sdsave_unittest_base.taskname+'Test2'
    outfile=prefix+'.asap'
    outvis=prefix+'.ms'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.infile)):
            shutil.copytree(self.datapath+self.infile, self.infile)
        if (not os.path.exists(self.vis)):
            shutil.copytree(self.datapath+self.vis, self.vis)

        default(sdsave)
        #self._setAttributes()
        #self.scanno=0

    def tearDown(self):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)
        if (os.path.exists(self.vis)):
            shutil.rmtree(self.vis)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test_import(self):
        """Test if SCANNO is consistent with original MS."""
        self.res = sdsave(infile=self.vis, outfile=self.outfile,
                          outform='ASAP')

        # compare scan number
        tb.open(self.vis)
        scan_number_org = numpy.unique(tb.getcol('SCAN_NUMBER'))
        tb.close()
        tb.open(self.outfile)
        scan_number = numpy.unique(tb.getcol('SCANNO'))
        tb.close()
        print 'scan_number_org=', scan_number_org
        print 'scan_number=', scan_number
        self.assertEqual(len(scan_number_org), len(scan_number))
        self.assertTrue(all(scan_number_org == scan_number))

    def test_export(self):
        """Test if SCAN_NUMBER is consistent with original Scantable."""
        self.res = sdsave(infile=self.infile, outfile=self.outvis,
                          outform='MS2')

        # compare scan number
        tb.open(self.infile)
        scan_number_org = numpy.unique(tb.getcol('SCANNO'))
        tb.close()
        tb.open(self.outvis)
        scan_number = numpy.unique(tb.getcol('SCAN_NUMBER'))
        tb.close()
        print 'scan_number_org=', scan_number_org
        print 'scan_number=', scan_number
        self.assertEqual(len(scan_number_org), len(scan_number))
        self.assertTrue(all(scan_number_org == scan_number))

def suite():
    return [sdsave_test0,sdsave_test1,sdsave_test2,
            sdsave_test3,sdsave_test4,sdsave_test5,
            sdsave_test6,sdsave_test7,sdsave_storageTest,
            sdsave_freq_labeling,sdsave_flagging,
            sdsave_scan_number]
