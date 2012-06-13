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
from sdcal import sdcal
import asap as sd

#
# Unit test of sdcal task.
# 

###
# Base class for sdcal unit test
###
class sdcal_unittest_base:
    """
    Base class for sdcal unit test
    """
    taskname='sdcal'
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
class sdcal_caltest_base(sdcal_unittest_base):
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
class sdcal_avetest_base(sdcal_unittest_base):
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
# Test on bad parameter settings
###
class sdcal_test0(sdcal_unittest_base,unittest.TestCase):
    """
    Test on bad parameter setting
    """
    # Input and output names
    rawfile='calpsGBT.asap'
    prefix=sdcal_unittest_base.taskname+'Test0'
    outfile=prefix+'.asap'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(sdcal)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test000(self):
        """Test 000: Default parameters"""
        self.res=sdcal()
        self.assertFalse(self.res)
        
    def test001(self):
        """Test 001: Time averaging without weight"""
        self.res=sdcal(infile=self.rawfile,timeaverage=True,outfile=self.outfile)
        self.assertFalse(self.res)        

    def test002(self):
        """Test 002: Polarization averaging without weight"""
        self.res=sdcal(infile=self.rawfile,polaverage=True,outfile=self.outfile)
        self.assertFalse(self.res)        

    def test003(self):
        """Test 003: Invalid calibration mode"""
        self.res=sdcal(infile=self.rawfile,calmode='invalid',outfile=self.outfile)
        self.assertFalse(self.res)

    def test004(self):
        """Test 004: Specify existing output file name with overwrite=False"""
        outfile='calpsGBT.cal.asap'
        if (not os.path.exists(outfile)):
            shutil.copytree(self.datapath+outfile, outfile)
        self.res=sdcal(infile=self.rawfile,outfile=outfile,overwrite=False)
        os.system( 'rm -rf %s'%outfile )        
        self.assertFalse(self.res)


###
# Test GBT position switch calibration 
###
class sdcal_test1(sdcal_caltest_base,unittest.TestCase):
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
    prefix=sdcal_unittest_base.taskname+'Test1'
    calmode='ps'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.reffile)):
            shutil.copytree(self.datapath+self.reffile, self.reffile)

        default(sdcal)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.reffile)):
            shutil.rmtree(self.reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test100(self):
        """Test 100: test to calibrate data (GBT position switch)"""
        outname=self.prefix+self.postfix
        self.res=sdcal(infile=self.rawfile,calmode=self.calmode,tau=0.09,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname)

###
# Test GBT nodding calibration 
###
class sdcal_test2(sdcal_caltest_base,unittest.TestCase):
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
    prefix=sdcal_unittest_base.taskname+'Test2'
    calmode='nod'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.reffile)):
            shutil.copytree(self.datapath+self.reffile, self.reffile)

        default(sdcal)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.reffile)):
            shutil.rmtree(self.reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test200(self):
        """Test 200: test to calibrate data (GBT nod)"""
        outname=self.prefix+self.postfix
        self.res=sdcal(infile=self.rawfile,calmode=self.calmode,tau=0.09,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname)

###
# Test GBT frequency switch calibration 
###
class sdcal_test3(sdcal_caltest_base,unittest.TestCase):
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
    prefix=sdcal_unittest_base.taskname+'Test3'
    calmode='fs'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.reffile)):
            shutil.copytree(self.datapath+self.reffile, self.reffile)

        default(sdcal)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.reffile)):
            shutil.rmtree(self.reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test300(self):
        """Test 300: test to calibrate data (GBT frequency switch)"""
        outname=self.prefix+self.postfix
        self.res=sdcal(infile=self.rawfile,calmode=self.calmode,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname)
        
###
# Test quotient
###
class sdcal_test4(sdcal_caltest_base,unittest.TestCase):
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
    prefix=sdcal_unittest_base.taskname+'Test4'
    calmode='quotient'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.reffile)):
            shutil.copytree(self.datapath+self.reffile, self.reffile)

        default(sdcal)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.reffile)):
            shutil.rmtree(self.reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test400(self):
        """Test 400: test to calibrate data (quotient)"""
        outname=self.prefix+self.postfix
        self.res=sdcal(infile=self.rawfile,calmode=self.calmode,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname)

###
# Test ALMA position switch calibration
###
class sdcal_test5(sdcal_caltest_base,unittest.TestCase):
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
    prefix=sdcal_unittest_base.taskname+'Test5'
    calmode='ps'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.reffile)):
            shutil.copytree(self.datapath+self.reffile, self.reffile)

        default(sdcal)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.reffile)):
            shutil.rmtree(self.reffile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test500(self):
        """Test 500: test to calibrate data (ALMA position switch)"""
        outname=self.prefix+self.postfix
        self.res=sdcal(infile=self.rawfile,calmode=self.calmode,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration')
        self._comparecal(outname)


###
# Test polarization averaging with/without scan average
###
class sdcal_test6(sdcal_avetest_base,unittest.TestCase):
    """
    Test polarization averaging with/without scan average

    Data is calpsGBT.cal.asap.

    ###NOTE###
    Polarization average averages data in both time and polarization.
    """
    # Input and output names
    rawfile='calpsGBT.cal.asap'
    prefix=sdcal_unittest_base.taskname+'Test6'
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

        default(sdcal)

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
        self.res=sdcal(infile=self.rawfile,scanaverage=True,polaverage=True,pweight='var',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during polarization averaging')
        self._compare(outname,self.reffiles[0])
        
    def test601(self):
        """Test 601: test polarization average with pweight='var' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcal(infile=self.rawfile,scanaverage=False,polaverage=True,pweight='var',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during polarization averaging')
        self._compare(outname,self.reffiles[1])

    def test602(self):
        """Test 602: test polarization average with pweight='tsys' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcal(infile=self.rawfile,scanaverage=True,polaverage=True,pweight='tsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during polarization averaging')
        self._compare(outname,self.reffiles[2])
        
    def test603(self):
        """Test 603: test polarization average with pweight='tsys' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcal(infile=self.rawfile,scanaverage=False,polaverage=True,pweight='tsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during polarization averaging')
        self._compare(outname,self.reffiles[3])


###
# Test time averaging with/without scan average
###
class sdcal_test7(sdcal_avetest_base,unittest.TestCase):
    """
    Test time averaging with/without scan average

    Data is calpsGBT.cal.asap.
    """
    # Input and output names
    rawfile='calpsGBT.cal.asap'
    prefix=sdcal_unittest_base.taskname+'Test7'
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

        default(sdcal)

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
        self.res=sdcal(infile=self.rawfile,scanaverage=True,timeaverage=True,tweight='var',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[0])
        
    def test701(self):
        """Test 701: test time average with tweight='var' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcal(infile=self.rawfile,scanaverage=False,timeaverage=True,tweight='var',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[1])
        
    def test702(self):
        """Test 702: test time average with tweight='tsys' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcal(infile=self.rawfile,scanaverage=True,timeaverage=True,tweight='tsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[2])
        
    def test703(self):
        """Test 703: test time average with tweight='tsys' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcal(infile=self.rawfile,scanaverage=False,timeaverage=True,tweight='tsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[3])
        
    def test704(self):
        """Test 704: test time average with tweight='tint' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcal(infile=self.rawfile,scanaverage=True,timeaverage=True,tweight='tint',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[4])
        
    def test705(self):
        """Test 705: test time average with tweight='tint' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcal(infile=self.rawfile,scanaverage=False,timeaverage=True,tweight='tint',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[5])
        
    def test706(self):
        """Test 706: test time average with tweight='tintsys' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcal(infile=self.rawfile,scanaverage=True,timeaverage=True,tweight='tintsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[6])
        
    def test707(self):
        """Test 707: test time average with tweight='tintsys' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcal(infile=self.rawfile,scanaverage=False,timeaverage=True,tweight='tintsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[7])
        
    def test708(self):
        """Test 708: test time average with tweight='median' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcal(infile=self.rawfile,scanaverage=True,timeaverage=True,tweight='median',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[8])
        
    def test709(self):
        """Test 709: test time average with tweight='median' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcal(infile=self.rawfile,scanaverage=False,timeaverage=True,tweight='median',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during time averaging')
        self._compare(outname,self.reffiles[9])
        


###
# Test mixed operation (cal+average,time+pol average,...)
###
class sdcal_test8(sdcal_avetest_base,unittest.TestCase):
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
    prefix=sdcal_unittest_base.taskname+'Test8'
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

        default(sdcal)

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
        self.res=sdcal(infile=self.rawfile,calmode='ps',scanaverage=True,polaverage=True,pweight='tsys',tau=0.09,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration and averaging')
        self._compare(outname,self.reffiles[0])
        
    def test801(self):
        """Test 801: test calibration + time average with tweight='tintsys' (scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcal(infile=self.rawfile,calmode='ps',scanaverage=True,timeaverage=True,tweight='tintsys',tau=0.09,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during calibration and averaging')
        self._compare(outname,self.reffiles[1])
        
    def test802(self):
        """Test 802: test polarization average with pweight='tsys' + time average with tweight='tintsys' (no scan average)"""
        outname=self.prefix+self.postfix
        self.res=sdcal(infile=self.calfile,scanaverage=False,timeaverage=True,tweight='tintsys',polaverage=True,pweight='tsys',outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during averaging')
        self._compare(outname,self.reffiles[2])
        
    def test803(self):
        """Test 803: test channelrange selection"""
        chrange=[1000,8000]
        outname=self.prefix+self.postfix
        self.res=sdcal(infile=self.calfile,channelrange=chrange,outfile=outname,outform='ASAP')
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
class sdcal_test9(sdcal_avetest_base,unittest.TestCase):
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
    prefix=sdcal_unittest_base.taskname+'Test9'
    reffiles=['averageall.ref']

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        for reffile in self.reffiles:
            if (not os.path.exists(reffile)):
                shutil.copyfile(self.datapath+reffile, reffile)

        default(sdcal)

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
        self.res=sdcal(infile=self.rawfile,scanaverage=False,timeaverage=True,tweight='tintsys',averageall=True,polaverage=False,outfile=outname,outform='ASAP')
        self.assertEqual(self.res,None,
                         msg='Any error occurred during averaging')
        self._compare(outname,self.reffiles[0])
        


def suite():
    return [sdcal_test0, sdcal_test1,
            sdcal_test2, sdcal_test3,
            sdcal_test4, sdcal_test5,
            sdcal_test6, sdcal_test7,
            sdcal_test8, sdcal_test9]
