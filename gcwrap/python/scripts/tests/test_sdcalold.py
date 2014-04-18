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

# to rethrow exception 
import inspect
g = sys._getframe(len(inspect.stack())-1).f_globals
g['__rethrow_casa_exceptions'] = True
from sdcalold_cli import sdcalold_cli as sdcalold
from sdstatold_cli import sdstatold_cli as sdstatold
#from sdcalold import sdcalold
import asap as sd

#
# Unit test of sdcalold task.
# 

###
# Base class for sdcalold unit test
###
class sdcalold_unittest_base:
    """
    Base class for sdcalold unit test
    """
    taskname='sdcalold'
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdcal/'
    tolerance=1.0e-15

    def _checkfile( self, name ):
        isthere=os.path.exists(name)
        self.assertEqual(isthere,True,
                         msg='output file %s was not created because of the task failure'%(name))
        

    def _getspectra( self, name ):
        isthere=os.path.exists(name)
        self.assertEqual(isthere,True,
                         msg='file %s does not exist'%(name))        
        tb.open(name)
        sp=tb.getcol('SPECTRA').transpose()
        tb.close()
        return sp

    def _checkshape( self, sp, ref ):
        # check array dimension 
        self.assertEqual( sp.ndim, ref.ndim,
                          msg='array dimension differ' )
        # check number of spectra
        self.assertEqual( sp.shape[0], ref.shape[0],
                          msg='number of spectra differ' )
        # check number of channel
        self.assertEqual( sp.shape[1], ref.shape[1],
                          msg='number of channel differ' )

    def _diff(self, sp, ref):
        diff=abs((sp-ref)/ref)
        idx=numpy.argwhere(numpy.isnan(diff))
        #print idx
        if len(idx) > 0:
            diff[idx]=sp[idx]
        return diff
        

###
# Base class for calibration test
###
class sdcalold_caltest_base(sdcalold_unittest_base):
    """
    Base class for calibration test
    """
    reffile=''
    postfix='.cal.asap'

    def _comparecal( self, name ):
        self._checkfile(name)
        sp=self._getspectra(name)
        spref=self._getspectra(self.reffile)

        self._checkshape( sp, spref )
        
        for irow in xrange(sp.shape[0]):
            diff=self._diff(sp[irow],spref[irow])
            retval=numpy.all(diff<0.01)
            maxdiff=diff.max()
            self.assertEqual( retval, True,
                             msg='calibrated result is wrong (irow=%s): maxdiff=%s'%(irow,diff.max()) )
        del sp, spref

###
# Base class for averaging test
###
class sdcalold_avetest_base(sdcalold_unittest_base):
    """
    Base class for averaging test
    """
    postfix='ave.asap'

    def _split( self, s ):
        pos0=s.find('[')+1
        pos1=s.find(']')
        substr=s[pos0:pos1]
        return substr.split(',')

    def _getrefdata( self, name ):
        isthere=os.path.exists(name)
        self.assertEqual(isthere,True,
                         msg='file %s does not exist'%(name))
        f=open(name)
        lines=f.readlines()
        f.close
        # scanno
        scans=[]
        if re.match('^SCANNO:',lines[0]) is not None:
            ss=self._split(lines[0])
            for s in ss:
                scans.append(string.atoi(s))

        # polno
        pols=[]
        if re.match('^POLNO:',lines[1]) is not None:
            ss=self._split(lines[1])
            for s in ss:
                pols.append(string.atoi(s))

        # spectral data
        sp=[]
        for line in lines[2:]:
            tmp=[]
            ss=self._split(line)
            for s in ss:
                tmp.append(string.atof(s))
            sp.append(tmp)

        # python list -> numpy array
        scans=numpy.unique(scans)
        pols=numpy.unique(pols)
        sp=numpy.array(sp)

        return (scans, pols, sp)
    

    def _compare( self, name1, name2 ):
        self._checkfile(name1)
        sp=self._getspectra( name1 )
        (scan0,pol0,sp0)=self._getrefdata( name2 )

        self._checkshape( sp, sp0 )

        s=sd.scantable(name1,False)
        scan=numpy.array(s.getscannos())
        pol=numpy.array(s.getpolnos())
        #print scan0, scan
        #print pol0, pol
        retval=len(pol)==len(pol0) and all(pol==pol0)
        self.assertEqual( retval, True,
                          msg='POLNO is wrong' )
        retval=len(scan)==len(scan0) and all(scan==scan0)
        self.assertEqual( retval, True,
                          msg='SCANNO is wrong' )
        del s
        
        for irow in xrange(sp.shape[0]):
            diff=self._diff(sp[irow],sp0[irow])
            retval=numpy.all(diff<0.01)
            self.assertEqual( retval, True,
                              msg='averaged result is wrong (irow=%s): maxdiff=%s'%(irow,diff.max()) )
        del sp, sp0

###
# Base class for edgemarker testing
###
class sdcalold_edgemarker_base(sdcalold_unittest_base):
    """
    Base class for edgemarker testing
    """
    def _readref( self, name ):
        f = open( name, 'r' )
        lines = f.readlines()
        f.close()

        ret = []
        nline = len(lines)
        for line in lines:
            s = line.split()
            ret.append( [float(s[0]),float(s[1])] )
        return numpy.array(ret).transpose()
    
    def _checkmarker( self, name, refdata ):
        # refdata shape is (2,noff)
        noff = refdata.shape[1]
        tb.open(name)
        tsel = tb.query('SRCTYPE==1')
        nrow = tsel.nrows()
        self.assertTrue( nrow > 0,
                         msg='data doesn\'t have OFF spectra' )
        dir = tsel.getcol('DIRECTION')
        tsel.close()
        tb.close()
        
        self.assertEqual( 2*noff, dir.shape[1], # refdata store only POLNO==0
                          msg='number of OFF differ: %s (should be %s)'%(dir.shape[1],2*noff) )

        for irow in xrange(2*noff):
            idx = int(irow) / 2
            diff = self._diff( dir[:,irow], refdata[:,idx] )
            self.assertEqual( numpy.all(diff<0.01), True,
                              msg='direction for OFF differ (irow=%s): [%s,%s] (should be [%s,%s])'%(irow,dir[0,irow],dir[1,irow],refdata[0,idx],refdata[1,idx]) )

###
# Test on bad parameter settings
###
class sdcalold_test0(sdcalold_unittest_base,unittest.TestCase):
    """
    Test on bad parameter setting
    """
    # Input and output names
    rawfile='calpsGBT.asap'
    prefix=sdcalold_unittest_base.taskname+'Test0'
    outfile=prefix+'.asap'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(sdcalold)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test000(self):
        """Test 000: Default parameters"""
        # argument verification error
        try:
            self.res=sdcalold()
            #self.assertFalse(self.res)
            self.fail("The task must throw exception.")
        except Exception, e:
            pos=str(e).find("Parameter verification failed")
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))
        
    def test001(self):
        """Test 001: Time averaging without weight"""
        try:
            self.res=sdcalold(infile=self.rawfile,timeaverage=True,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Please specify weight type of time averaging')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test002(self):
        """Test 002: Polarization averaging without weight"""
        try:
            self.res=sdcalold(infile=self.rawfile,polaverage=True,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Please specify weight type of polarization averaging')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test003(self):
        """Test 003: Invalid calibration mode"""
        # argument verification error
        try:
            self.res=sdcalold(infile=self.rawfile,calmode='invalid',outfile=self.outfile)
            #self.assertFalse(self.res)
            self.fail("The task must throw exception.")
        except Exception, e:
            pos=str(e).find("Parameter verification failed")
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test004(self):
        """Test 004: Specify existing output file name with overwrite=False"""
        outfile='calpsGBT.cal.asap'
        if (not os.path.exists(outfile)):
            shutil.copytree(self.datapath+outfile, outfile)
        try:
            self.res=sdcalold(infile=self.rawfile,outfile=outfile,overwrite=False)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Output file \'%s\' exists.'%(outfile))
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))
        finally:
            os.system( 'rm -rf %s'%outfile )        



###
# Test GBT position switch calibration 
###
class sdcalold_test1(sdcalold_caltest_base,unittest.TestCase):
    """
    Test GBT position switch calibration 
    
    Data is taken from OrionS_rawACSmod and created by the following
    script:

    asap_init()
    s=sd.scantable('OrionS_rawACSmod',average=False)
    sel=sd.selector()
    sel.set_ifs([0])
    sel.set_scans([20,21,22,23])
    s.set_selection(sel)
    s.save('calpsGBT.asap','ASAP')

    In addition, unnecessary TCAL rows were removed.
    """
    # Input and output names
    rawfile='calpsGBT.asap'
    reffile='calpsGBT.cal.asap'
    prefix=sdcalold_unittest_base.taskname+'Test1'
    calmode='ps'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.reffile)):
            shutil.copytree(self.datapath+self.reffile, self.reffile)

        default(sdcalold)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.reffile)):
            shutil.rmtree(self.reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test100(self):
        """Test 100: test to calibrate data (GBT position switch)"""
        outname=self.prefix+self.postfix
        self.res=sdcalold(infile=self.rawfile,calmode=self.calmode,tau=0.09,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname)

###
# Test GBT nodding calibration 
###
class sdcalold_test2(sdcalold_caltest_base,unittest.TestCase):
    """
    Test GBT nodding calibration 
    
    Data is taken from IRC+10216_rawACSmod and created by the following
    script:

    asap_init()
    s=sd.scantable('IRC+10216_rawACSmod',average=False)
    sel=sd.selector()
    sel.set_scans([229,230])
    sel.set_ifs([3])
    sel.set_cycles([0,1])
    s.set_selection(sel)
    s.save('calnodGBT.asap','ASAP')

    In addition, unnecessary TCAL rows were removed.
    """
    # Input and output names
    rawfile='calnodGBT.asap'
    reffile='calnodGBT.cal.asap'
    prefix=sdcalold_unittest_base.taskname+'Test2'
    calmode='nod'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.reffile)):
            shutil.copytree(self.datapath+self.reffile, self.reffile)

        default(sdcalold)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.reffile)):
            shutil.rmtree(self.reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test200(self):
        """Test 200: test to calibrate data (GBT nod)"""
        outname=self.prefix+self.postfix
        self.res=sdcalold(infile=self.rawfile,calmode=self.calmode,tau=0.09,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname)

###
# Test GBT frequency switch calibration 
###
class sdcalold_test3(sdcalold_caltest_base,unittest.TestCase):
    """
    Test GBT frequency switch calibration 
    
    Data is taken from FLS3_all_newcal_SP and created by the following
    script:

    asap_init()
    sd.rc('scantable',storage='disk')
    s=sd.scantable('FLS3_all_newcal_SP',average=False)
    sel=sd.selector()
    sel.set_scans([14914])
    sel.set_cycles([0,1])
    s.set_selection(sel)
    s.save('calfsGBT.asap','ASAP')

    In addition, unnecessary TCAL rows were removed.
    """
    # Input and output names
    rawfile='calfsGBT.asap'
    reffile='calfsGBT.cal.asap'
    prefix=sdcalold_unittest_base.taskname+'Test3'
    calmode='fs'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.reffile)):
            shutil.copytree(self.datapath+self.reffile, self.reffile)

        default(sdcalold)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.reffile)):
            shutil.rmtree(self.reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test300(self):
        """Test 300: test to calibrate data (GBT frequency switch)"""
        outname=self.prefix+self.postfix
        self.res=sdcalold(infile=self.rawfile,calmode=self.calmode,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname)
        
###
# Test quotient
###
class sdcalold_test4(sdcalold_caltest_base,unittest.TestCase):
    """
    Test quotient.
    
    Data is taken from MOPS.rpf, which is included in ASAP package
    for testing, and created by the following script:

    asap_init()
    s=sd.scantable('MOPS.rpf',average=False)
    sel=sd.selector()
    sel.set_cycles([0,1])
    s.set_selection(sel)
    s.save('quotient.asap','ASAP')

    """
    # Input and output names
    rawfile='quotient.asap'
    reffile='quotient.cal.asap'
    prefix=sdcalold_unittest_base.taskname+'Test4'
    calmode='quotient'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.reffile)):
            shutil.copytree(self.datapath+self.reffile, self.reffile)

        default(sdcalold)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.reffile)):
            shutil.rmtree(self.reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test400(self):
        """Test 400: test to calibrate data (quotient)"""
        outname=self.prefix+self.postfix
        self.res=sdcalold(infile=self.rawfile,calmode=self.calmode,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname)

###
# Test ALMA position switch calibration
###
class sdcalold_test5(sdcalold_caltest_base,unittest.TestCase):
    """
    Test ALMA position switch calibration (OTF raster with OFF scan)
    
    Data is taken from uid___A002_X8ae1b_X1 (DV01) and created by 
    the following script:

    asap_init()
    sd.splitant('uid___A002_X8ae1b_X1') # to split data by antenna
    s=sd.scantable('uid___A002_X8ae1b.DV01.asap',average=False)
    sel=sd.selector()
    sel.set_ifs([2])
    sel.set_cycles([20,118,205])
    s.set_selection(sel)
    s.save('calpsALMA.asap','ASAP')

    """
    # Input and output names
    rawfile='calpsALMA.asap'
    reffile='calpsALMA.cal.asap'
    prefix=sdcalold_unittest_base.taskname+'Test5'
    calmode='ps'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.reffile)):
            shutil.copytree(self.datapath+self.reffile, self.reffile)

        default(sdcalold)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.reffile)):
            shutil.rmtree(self.reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test500(self):
        """Test 500: test to calibrate data (ALMA position switch)"""
        outname=self.prefix+self.postfix
        self.res=sdcalold(infile=self.rawfile,calmode=self.calmode,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname)


###
# Test polarization averaging with/without scan average
###
class sdcalold_test6(sdcalold_avetest_base,unittest.TestCase):
    """
    Test polarization averaging with/without scan average

    Data is calpsGBT.cal.asap.

    ###NOTE###
    Polarization average averages data in both time and polarization.
    """
    # Input and output names
    rawfile='calpsGBT.cal.asap'
    prefix=sdcalold_unittest_base.taskname+'Test6'
    reffiles=['polaverage.ref0',
              'polaverage.ref1',
              'polaverage.ref2',
              'polaverage.ref3']
    
    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        for reffile in self.reffiles:
            if (not os.path.exists(reffile)):
                shutil.copyfile(self.datapath+reffile, reffile)

        default(sdcalold)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        for reffile in self.reffiles:
            if (os.path.exists(reffile)):
                os.remove(reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test600(self):
        """Test 600: test polarization average with pweight='var' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcalold(infile=self.rawfile,scanaverage=True,polaverage=True,pweight='var',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during polarization averaging')
        self._compare(outname,self.reffiles[0])
        
    def test601(self):
        """Test 601: test polarization average with pweight='var' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcalold(infile=self.rawfile,scanaverage=False,polaverage=True,pweight='var',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during polarization averaging')
        self._compare(outname,self.reffiles[1])

    def test602(self):
        """Test 602: test polarization average with pweight='tsys' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcalold(infile=self.rawfile,scanaverage=True,polaverage=True,pweight='tsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during polarization averaging')
        self._compare(outname,self.reffiles[2])
        
    def test603(self):
        """Test 603: test polarization average with pweight='tsys' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcalold(infile=self.rawfile,scanaverage=False,polaverage=True,pweight='tsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during polarization averaging')
        self._compare(outname,self.reffiles[3])


###
# Test time averaging with/without scan average
###
class sdcalold_test7(sdcalold_avetest_base,unittest.TestCase):
    """
    Test time averaging with/without scan average

    Data is calpsGBT.cal.asap.
    """
    # Input and output names
    rawfile='calpsGBT.cal.asap'
    prefix=sdcalold_unittest_base.taskname+'Test7'
    reffiles=['timeaverage.ref0',
              'timeaverage.ref1',
              'timeaverage.ref2',
              'timeaverage.ref3',
              'timeaverage.ref4',
              'timeaverage.ref5',
              'timeaverage.ref6',
              'timeaverage.ref7',
              'timeaverage.ref8',
              'timeaverage.ref9']

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        for reffile in self.reffiles:
            if (not os.path.exists(reffile)):
                shutil.copyfile(self.datapath+reffile, reffile)

        default(sdcalold)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        for reffile in self.reffiles:
            if (os.path.exists(reffile)):
                os.remove(reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test700(self):
        """Test 700: test time average with tweight='var' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcalold(infile=self.rawfile,scanaverage=True,timeaverage=True,tweight='var',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[0])
        
    def test701(self):
        """Test 701: test time average with tweight='var' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcalold(infile=self.rawfile,scanaverage=False,timeaverage=True,tweight='var',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[1])
        
    def test702(self):
        """Test 702: test time average with tweight='tsys' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcalold(infile=self.rawfile,scanaverage=True,timeaverage=True,tweight='tsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[2])
        
    def test703(self):
        """Test 703: test time average with tweight='tsys' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcalold(infile=self.rawfile,scanaverage=False,timeaverage=True,tweight='tsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[3])
        
    def test704(self):
        """Test 704: test time average with tweight='tint' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcalold(infile=self.rawfile,scanaverage=True,timeaverage=True,tweight='tint',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[4])
        
    def test705(self):
        """Test 705: test time average with tweight='tint' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcalold(infile=self.rawfile,scanaverage=False,timeaverage=True,tweight='tint',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[5])
        
    def test706(self):
        """Test 706: test time average with tweight='tintsys' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcalold(infile=self.rawfile,scanaverage=True,timeaverage=True,tweight='tintsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[6])
        
    def test707(self):
        """Test 707: test time average with tweight='tintsys' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcalold(infile=self.rawfile,scanaverage=False,timeaverage=True,tweight='tintsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[7])
        
    def test708(self):
        """Test 708: test time average with tweight='median' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcalold(infile=self.rawfile,scanaverage=True,timeaverage=True,tweight='median',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[8])
        
    def test709(self):
        """Test 709: test time average with tweight='median' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcalold(infile=self.rawfile,scanaverage=False,timeaverage=True,tweight='median',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[9])
        

###
# Test mixed operation (cal+average,time+pol average,...)
###
class sdcalold_test8(sdcalold_avetest_base,unittest.TestCase):
    """
    Test other things including mixed opearation

       - calibration + time average
       - calibration + polarization average
       - time average + polarzation average
       - channelrange parameter
       
    """
    # Input and output names
    rawfile='calpsGBT.asap'
    calfile='calpsGBT.cal.asap'
    prefix=sdcalold_unittest_base.taskname+'Test8'
    reffiles=['polaverage.ref2',
              'timeaverage.ref6',
              'timepolaverage.ref']

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.calfile)):
            shutil.copytree(self.datapath+self.calfile, self.calfile)
        for reffile in self.reffiles:
            if (not os.path.exists(reffile)):
                shutil.copyfile(self.datapath+reffile, reffile)

        default(sdcalold)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.calfile)):
            shutil.rmtree(self.calfile)
        for reffile in self.reffiles:
            if (os.path.exists(reffile)):
                os.remove(reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test800(self):
        """Test 800: test calibration + polarization average with pweight='tsys' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcalold(infile=self.rawfile,calmode='ps',scanaverage=True,polaverage=True,pweight='tsys',tau=0.09,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration and averaging')
        self._compare(outname,self.reffiles[0])
        
    def test801(self):
        """Test 801: test calibration + time average with tweight='tintsys' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcalold(infile=self.rawfile,calmode='ps',scanaverage=True,timeaverage=True,tweight='tintsys',tau=0.09,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration and averaging')
        self._compare(outname,self.reffiles[1])
        
    def test802(self):
        """Test 802: test polarization average with pweight='tsys' + time average with tweight='tintsys' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcalold(infile=self.calfile,scanaverage=False,timeaverage=True,tweight='tintsys',polaverage=True,pweight='tsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during averaging')
        self._compare(outname,self.reffiles[2])
        
    def test803(self):
        """Test 803: test channelrange selection"""
        chrange=[1000,8000]
        outname=self.prefix+self.postfix
        self.res=sdcalold(infile=self.calfile,channelrange=chrange,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during channel range selection')

        s0=sd.scantable(self.calfile,False)
        s1=sd.scantable(outname,False)

        # check spectral value
        sp0=numpy.array(s0._getspectrum(0))
        sp1=numpy.array(s1._getspectrum(0))
        self.res=all(sp0[chrange[0]:chrange[1]+1]==sp1)
        self.assertEqual(self.res,True,
                         msg='spectral data do not match')

        # check frequency
        s0.set_unit('GHz')
        s1.set_unit('GHz')
        abc0=numpy.array(s0._getabcissa(0))
        abc1=numpy.array(s1._getabcissa(0))
        self.res=numpy.allclose(abc0[chrange[0]:chrange[1]+1],abc1)
        self.assertEqual(self.res,True,
                         msg='frequencies do not match')

        del s0,s1

###
# Test averageall parameter
###
class sdcalold_test9(sdcalold_avetest_base,unittest.TestCase):
    """
    Test averageall parameter that forces to average spectra with
    different spectral resolution.

    Here, test to average the following two spectra that have same
    frequency in band center.

       - nchan = 8192, resolution = 6104.23 Hz
       - nchan = 8192, resolution = 12208.5 Hz
       
    """
    # Input and output names
    rawfile='averageall.asap'
    prefix=sdcalold_unittest_base.taskname+'Test9'
    reffiles=['averageall.ref']

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        for reffile in self.reffiles:
            if (not os.path.exists(reffile)):
                shutil.copyfile(self.datapath+reffile, reffile)

        default(sdcalold)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        for reffile in self.reffiles:
            if (os.path.exists(reffile)):
                os.remove(reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test900(self):
        """Test 900: test averageall parameter"""
        outname=self.prefix+self.postfix
        self.res=sdcalold(infile=self.rawfile,scanaverage=False,timeaverage=True,tweight='tintsys',averageall=True,polaverage=False,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during averaging')
        self._compare(outname,self.reffiles[0])
        self._checkFrequencies(outname)

    def _checkFrequencies(self,outfile):
        tb.open(self.rawfile.rstrip('/')+'/FREQUENCIES')
        rpin=tb.getcol('REFPIX')
        rvin=tb.getcol('REFVAL')
        icin=tb.getcol('INCREMENT')
        tb.close
        tb.open(outfile.rstrip('/')+'/FREQUENCIES')
        rpout=tb.getcol('REFPIX')
        rvout=tb.getcol('REFVAL')
        icout=tb.getcol('INCREMENT')
        tb.close
        ic=[icin[2:4].max(),icin[0:2].max()]
        ledge=[rvin[2]-ic[0]*(rpin[2]+0.5),
               rvin[0]-ic[1]*(rpin[0]+0.5)]
        redge=[rvin[2]+ic[0]*(8191-rpin[2]+0.5),
               rvin[0]+ic[1]*(8191-rpin[0]+0.5)]
        eps = 1.0e-15
        for i in xrange(2):
            self.assertEqual(ic[i],icout[i],
                             msg='INCREMENT for FREQ_ID=%s differ'%(i))
            fmin=rvout[i]-icout[i]*(rpout[i]+0.5)
            self.assertTrue(abs((fmin-ledge[i])/ledge[i]) < eps,
                             msg='Left frequency edge for FREQ_ID=%s does not match'%(i))
            fmax=rvout[i]+icout[i]*(8191-rpout[i]+0.5)
            self.assertTrue(abs((fmax-redge[i])/redge[i]) < eps,
                             msg='Right frequency edge for FREQ_ID=%s does not match'%(i))

###
# Test edgemarker
###
class sdcalold_test_edgemarker_generic(sdcalold_edgemarker_base,unittest.TestCase):
    """
    Test edgemarker function that is available for calmode='otf'. 

    Here, data will not be calibrated and only edge marking process
    will be executed. 
    """
    # Input and output names
    rawfile='lissajous.asap'
    prefix=sdcalold_unittest_base.taskname+'TestEdgeMarkerGeneric'
    reffiles = [ 'marker.otf.default.ref',
                 'marker.otf.custom.ref' ]

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        for reffile in self.reffiles:
            if (not os.path.exists(reffile)):
                shutil.copyfile(self.datapath+reffile, reffile)
        default(sdcalold)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        for reffile in self.reffiles:
            if (os.path.exists(reffile)):
                os.remove(reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def testEdgeMarkerGeneric0(self):
        """
        Test default setting for edgemarker
        """
        outname = self.prefix+'.asap'
        self.res = sdcalold(infile=self.rawfile,calmode='otf',markonly=True,scanaverage=False,timeaverage=False,polaverage=False,outfile=outname,outform='ASAP')
        refdir = self._readref( self.reffiles[0] )
        self._checkfile( outname ) 
        self._checkmarker( outname, refdir )

    def testEdgeMarkerGeneric1(self):
        """
        Test customized edge marking
        """
        outname = self.prefix+'.asap'
        self.res = sdcalold(infile=self.rawfile,calmode='otf',fraction='3%',markonly=True,scanaverage=False,timeaverage=False,polaverage=False,outfile=outname,outform='ASAP')
        refdir = self._readref( self.reffiles[1] )
        self._checkfile( outname ) 
        self._checkmarker( outname, refdir )


class sdcalold_test_edgemarker_raster(sdcalold_edgemarker_base,unittest.TestCase):
    """
    Test edgemarker function that is available for calmode='otfraster'. 

    Here, data will not be calibrated and only edge marking process
    will be executed. 
    """
    # Input and output names
    rawfile='raster.asap'
    prefix=sdcalold_unittest_base.taskname+'TestEdgeMarkerRaster'
    reffiles=[ 'marker.raster.default.ref',
               'marker.raster.custom.ref' ]

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        for reffile in self.reffiles:
            if (not os.path.exists(reffile)):
                shutil.copyfile(self.datapath+reffile, reffile)
        default(sdcalold)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        for reffile in self.reffiles:
            if (os.path.exists(reffile)):
                os.remove(reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def testEdgeMarkerRaster0(self):
        """
        Test default setting for edgemarker
        """
        outname = self.prefix+'.asap'
        self.res = sdcalold(infile=self.rawfile,calmode='otfraster',markonly=True,scanaverage=False,timeaverage=False,polaverage=False,outfile=outname,outform='ASAP')
        refdir = self._readref( self.reffiles[0] )
        self._checkfile( outname ) 
        self._checkmarker( outname, refdir )

    def testEdgeMarkerRaster1(self):
        """
        Test default setting for edgemarker
        """
        outname = self.prefix+'.asap'
        self.res = sdcalold(infile=self.rawfile,calmode='otfraster',noff=1,markonly=True,scanaverage=False,timeaverage=False,polaverage=False,outfile=outname,outform='ASAP')
        refdir = self._readref( self.reffiles[1] )
        self._checkfile( outname ) 
        self._checkmarker( outname, refdir )


###
# Averaging of flagged data
###
class sdcalold_test_flag(sdcalold_avetest_base,unittest.TestCase):
    """
    Test Averaging of flagged data.

    Data varies with tests
    """
    # Input and output names
    prefix=sdcalold_unittest_base.taskname+'Test7'
    filelist = []

    def setUp(self):
        self.res=None
        self.filelist = []
        default(sdcalold)

    def tearDown(self):
        for name in self.filelist:
            self._remove(name)

    def _remove(self, name):
        # remove file/directory if exists
        if os.path.isdir(name):
            shutil.rmtree(name)
        elif os.path.exists(name):
            os.remove(name)

    def _mark_to_delete(self, name):
        self.filelist.append(name)

    def _copy(self, source, dest):
        # copy source directory to dest
        self.assertTrue(os.path.exists(source),
                        msg="Could not find a source file '%s'" % source)
        if os.path.exists(dest):
            shutil.rmtree(dest)
        shutil.copytree(source, dest)

    def _copyInputs(self, inputs):
        if type(inputs)==str:
            inputs = [inputs]

        for inname in inputs:
            inname = inname.rstrip('/')
            outname = os.path.basename(inname)
            self._copy(inname, outname)
            self._mark_to_delete(outname)


    def _comp_stat(self, data, refstat):
        if type(data)==str:
            data = sdstatold(infile=data)
        elif type(data)!=dict:
            self.fail(msg="Internal Error: invalid data given to calculate statistics.")
        for key, refvals in refstat.items():
            refvals = self._to_list(refvals)
            testvals = self._to_list(data[key])
            for idx in range(len(refvals)):
                rdiff = (testvals[idx] - refvals[idx])/refvals[idx]
                self.assertAlmostEqual(rdiff, 0., 4, msg="Verification of '%s' (idx=%d) failed: %f (expected: %f)" % (key, idx, testvals[idx], refvals[idx]))

    def _to_list(self, indata):
        try: len(indata)
        except TypeError: return [indata]
        return list(indata)


    def testFlag01(self):
        """TestFlag01: test time average of row flagged data set"""
        outname=self.prefix+self.postfix
        self._mark_to_delete(outname)
        # Prepare input
        inname = 'flatspec_rowflag.asap'
        self.rawfile = self.datapath+inname
        self._copyInputs(self.rawfile)
        self._remove(outname)

        # run task
        self.res=sdcalold(infile=inname,scanaverage=False,timeaverage=True,
                       tweight='tint',outfile=outname,outform='ASAP')
        # verification
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        refstat = {'max': 1.5, 'min': 1.5, 'sum': 12288.0}
        self._comp_stat(outname, refstat)

    def testFlag02(self):
        """TestFlag02: test time average of channel flagged data set"""
        outname=self.prefix+self.postfix
        self._mark_to_delete(outname)
        # Prepare input
        inname = 'flatspec_chanflag0.asap'
        self.rawfile = self.datapath+inname
        self._copyInputs(self.rawfile)
        self._remove(outname)

        # run task
        self.res=sdcalold(infile=inname,scanaverage=False,timeaverage=True,
                       tweight='tint',outfile=outname,outform='ASAP')
        # verification
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        refedge = {'max': 1.5, 'min': 1.5, 'sum': 9286.5}
        refinner = {'max': 2.0, 'min': 2.0, 'sum': 4002.0}
        outstat = sdstatold(outname, masklist=[3000, 5000])
        self._comp_stat(outstat, refinner)
        outstat = sdstatold(outname, masklist=[[0, 2999], [5001, 8192]])        
        self._comp_stat(outstat, refedge)

    def testFlag03(self):
        """TestFlag03: test polarization average of row flagged data set"""
        outname=self.prefix+self.postfix
        self._mark_to_delete(outname)
        # Prepare input
        inname = 'flatspec_2pol_rowflag.asap'
        self.rawfile = self.datapath+inname
        self._copyInputs(self.rawfile)
        self._remove(outname)

        # run task
        self.res=sdcalold(infile=inname,scanaverage=False,timeaverage=False,
                       polaverage=True,pweight='tsys',
                       outfile=outname,outform='ASAP')
        # verification
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        refstat = {'max': 1.5, 'min': 1.5, 'sum': 12288.0}
        self._comp_stat(outname, refstat)

    def testFlag04(self):
        """TestFlag04: test polarization average of channel flagged data set"""
        outname=self.prefix+self.postfix
        self._mark_to_delete(outname)
        # Prepare input
        inname = 'flatspec_2pol_chanflag0.asap'
        self.rawfile = self.datapath+inname
        self._copyInputs(self.rawfile)
        self._remove(outname)

        # run task
        self.res=sdcalold(infile=inname,scanaverage=False,timeaverage=False,
                       polaverage=True,pweight='tsys',
                       outfile=outname,outform='ASAP')
        # verification
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        refedge = {'max': 1.5, 'min': 1.5, 'sum': 9286.5}
        refinner = {'max': 2.0, 'min': 2.0, 'sum': 4002.0}
        outstat = sdstatold(outname, masklist=[3000, 5000])
        self._comp_stat(outstat, refinner)
        outstat = sdstatold(outname, masklist=[[0, 2999], [5001, 8192]])        
        self._comp_stat(outstat, refedge)


def suite():
    return [sdcalold_test0, sdcalold_test1,
            sdcalold_test2, sdcalold_test3,
            sdcalold_test4, sdcalold_test5,
            sdcalold_test6, sdcalold_test7,
            sdcalold_test8, sdcalold_test9,
            sdcalold_test_edgemarker_generic,
            sdcalold_test_edgemarker_raster,
            sdcalold_test_flag]
