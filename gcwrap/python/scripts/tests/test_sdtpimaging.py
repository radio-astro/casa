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

from sdtpimaging import sdtpimaging
import asap as sd

# Unit test of sdtpimaging task.

###
# Base class for sdtpimaging unit test
###
class sdtpimaging_unittest_base:
    """
    Base class for sdtpimaging unit test.
    """
    taskname='sdtpimaging'
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdtpimaging/'
    allowance=0.01

    def _set_mask(self, name, mask=None):
        """Set mask to image. the mask is cleared up if mask=None"""
        ia.open(name)
        try:
            old_mask = ia.maskhandler('default')
            if mask is None:
                ia.calcmask('T', asdefault=True)
            else:
                ia.maskhandler('set',mask)
        except: raise
        finally: ia.close()
        return old_mask
    
    def _compare(self, refdata, outdata, ignoremask=True):
        self._checkfile(outdata)
        if ignoremask: old_mask = self._set_mask(outdata)
        refstat = imstat( imagename=refdata )
        outstat = imstat( imagename=outdata )
        if ignoremask: ret = self._set_mask(outdata, old_mask)
        keys = ['max', 'min', 'mean', 'maxpos', 'minpos', 'rms' ]
        for key in keys:
            ref = refstat[key]
            out = outstat[key]
            print 'stat %s: ref %s, out %s'%(key,ref,out)
            for i in xrange( len(ref) ):
                if ref[i] != 0.0:
                    diff = (out[i]-ref[i])/ref[i]
                    self.assertTrue( all( abs(diff.flatten()) < self.allowance ),
                                     msg='statistics %s differ: reference %s, output %s'%(key,ref,out) )
                else:
                    self.assertTrue( out[i] == 0.0,
                                     msg='statistics %s differ: reference %s, output %s'%(key,ref,out) )

    def _checkfile( self, name ):
        isthere=os.path.exists(name)
        self.assertEqual(isthere,True,
                         msg='output file %s was not created because of the task failure'%(name))


###
# Test on bad parameter settings, data selection, ...
###
class sdtpimaging_test0(unittest.TestCase,sdtpimaging_unittest_base):
    """
    Test on bad parameter settings
    """
    # Input and output names
    infile='tpimaging.ms'
    prefix=sdtpimaging_unittest_base.taskname+'Test0'
    outfile=prefix+'.ms'
    outimage=prefix+'.im'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.infile)):
            shutil.copytree(self.datapath+self.infile, self.infile)

        default(sdtpimaging)

    def tearDown(self):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test000(self):
        """Test 000: Default parameters"""
        # argument verification error
        self.res=sdtpimaging()
        self.assertFalse(self.res)
        
    def test001(self):
        """Test 001: Bad antenna id"""
        try:
            self.res=sdtpimaging(infile=self.infile,spw=2,antenna='99')
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('No matching antenna ID or name in the data, please check antenna parameter')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test002(self):
        """Test 002: Bad stokes string"""
        try:
            self.res=sdtpimaging(infile=self.infile,spw=2,stokes='J')
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('stokes=J specified but the data contains only [\'XX\']')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))
        
    def test003(self):
        """Test 003: Try to create image without output image name"""
        try:
            self.res=sdtpimaging(infile=self.infile,spw=2,createimage=True,outfile='')
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Please specify out image name')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test004(self):
        """Test 004: Negative imsize"""
        try:
            self.res=sdtpimaging(infile=self.infile,spw=2,createimage=True,outfile=self.outimage,imsize=[-1])
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            # failed to create image so that outimage not exist
            pos=str(e).find('%s does not exist'%(self.outimage))
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

# this test is meaningless w.r.t. its original purpose
#    def test005(self):
#        """Test 005: Negative cell size"""
#        self.res=sdtpimaging(infile=self.infile,spw=2,createimage=True,outfile=self.outimage,cell=[-1])
#        self.assertFalse(self.res)

    def test006(self):
        """Test 006: Bad phase center string"""
        try:
            self.res=sdtpimaging(infile=self.infile,spw=2,createimage=True,outfile=self.outimage,phasecenter='XXX')
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Could not interprete phasecenter parameter')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test007(self):
        """Test 007: Bad pointing column name"""
        # argument verification error
        self.res=sdtpimaging(infile=self.infile,createimage=True,outfile=self.outimage,pointingcolumn='XXX')
        self.assertFalse(self.res)

    def test008(self):
        """Test 008: Unexisting grid function"""
        # argument verification error
        self.res=sdtpimaging(infile=self.infile,createimage=True,outfile=self.outimage,gridfunction='XXX')
        self.assertFalse(self.res)
 
    def test009(self):
        """Test 009: Invalid calmode"""
        # argument verification error
        self.res=sdtpimaging(infile=self.infile,calmode='ps')
        self.assertFalse(self.res)
 

###
# Test to image data without spatial baseline subtraction
###
class sdtpimaging_test1(unittest.TestCase,sdtpimaging_unittest_base):
    """
    Test to image data without spatial baseline subtraction
    """
    # Input and output names
    infile='tpimaging.ms'
    prefix=sdtpimaging_unittest_base.taskname+'Test1'
    outfile=prefix+'.ms'
    outimage=prefix+'.im'
    refimage='nobaseline.im'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.infile)):
            shutil.copytree(self.datapath+self.infile, self.infile)
        if (not os.path.exists(self.refimage)):
            shutil.copytree(self.datapath+self.refimage, self.refimage)

        default(sdtpimaging)

    def tearDown(self):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)
        if (os.path.exists(self.refimage)):
            shutil.rmtree(self.refimage)
        os.system( 'rm -rf '+self.prefix+'*' )
        os.system( 'rm -rf '+self.infile+'*' )

    def test100(self):
        """Test 100: test to image data without spatial baseline subtraction"""
        self.res=sdtpimaging(infile=self.infile,calmode='none',stokes='XX',spw=2,createimage=True,outfile=self.outimage,imsize=[64],cell=['15arcsec'],phasecenter='J2000 05h35m07s -5d21m00s',pointingcolumn='direction',gridfunction='SF')
        self.assertEqual(self.res,None)
        self._compare(self.refimage,self.outimage)
                
        
###
# Test to image data with spatial baseline subtraction
###
class sdtpimaging_test2(unittest.TestCase,sdtpimaging_unittest_base):
    """
    Test to image data with spatial baseline subtraction
    """
    # Input and output names
    infile='tpimaging.ms'
    prefix=sdtpimaging_unittest_base.taskname+'Test2'
    outfile=prefix+'.ms'
    outimage=prefix+'.im'
    refimage='dobaseline.im'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.infile)):
            shutil.copytree(self.datapath+self.infile, self.infile)
        if (not os.path.exists(self.refimage)):
            shutil.copytree(self.datapath+self.refimage, self.refimage)

        default(sdtpimaging)

    def tearDown(self):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)
        if (os.path.exists(self.refimage)):
            shutil.rmtree(self.refimage)
        os.system( 'rm -rf '+self.prefix+'*' )
        os.system( 'rm -rf '+self.infile+'*' )

    def test200(self):
        """Test 200: test to image data with spatial baseline subtraction"""
        self.res=sdtpimaging(infile=self.infile,calmode='baseline',masklist=[10,10],blpoly=1,stokes='XX',spw=2,createimage=True,outfile=self.outimage,imsize=[64],cell=['15arcsec'],phasecenter='J2000 05h35m07s -5d21m00s',pointingcolumn='direction',gridfunction='SF')
        self.assertEqual(self.res,None)
        self._compare(self.refimage,self.outimage)
        

###
# Test flag handling in sdtpimaging
###
class sdtpimaging_flag(unittest.TestCase,sdtpimaging_unittest_base):
    """
    Test flag handling in sdtpimaging

    How to generate input data:
    Use simobserve to generate an MS of single strip of scans
    (20 integration):
    
    simobserve(project='tpimaging', skymodel='m51ha.model',
    indirection='J2000 00h00m00 -27d00m00',incell='0.5arcsec',
    incenter='300GHz',inwidth='10kHz',mapsize=['100arcsec','5arcsec'],
    maptype='square',pointingspacing='5arcsec',obsmode='sd',totaltime='1')
    # modify table
    tb.open('tpimaging/tpimaging.aca.tp.sd.ms', nomodify=False)
    data = tb.getcol('DATA')
    for ipol in range(len(data)):
        data[ipol][0].real = range(len(data[ipol][0]))
        data[ipol][0][7:13].real += 3. # add source offset to scan=7-12
        data[ipol][0][15:17].real = 100. # add spurious to scan=15,16
    tb.putcol('DATA', data)
    # set scan number 0
    zeros = tb.getcol('SCAN_NUMBER')
    zeros *= 0
    tb.putcol('SCAN_NUMBER', zeros)
    tb.putcol('FIELD_ID', zeros)
    tb.close()
    split(vis='tpimaging/tpimaging.aca.tp.sd.ms', outputvis='tpimaging_1row.ms',datacolumn='data')
    """
    infile = 'tpimaging_1row.ms'
    # phasecenter need to be shifted for -1 pixel do to boundary difference.
    center = 'J2000 23:59:59.81295 -27d00m00'
    cell = '5arcsec'
    flagrows = [15,16]
    ref_bl = [0,0,0,0,0,0,0,3,3,3,3,3,3,0,0,85,84,0,0,0]
    nrow = len(ref_bl)
    prefix=sdtpimaging_unittest_base.taskname+'Flag'
    outimage = prefix+'.image'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.infile)):
            shutil.copytree(self.datapath+self.infile, self.infile)

        default(sdtpimaging)

    def tearDown(self):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def _chanflag_ms_rows(self, filename, rows):
        self._checkfile(filename)
        if type(rows) == int: rows = [int]
        tb.open(filename, nomodify=False)
        try:
            flg = tb.getcol("FLAG")
            for irow in rows:
                for ipol in range(len(flg)):
                    flg[ipol][0][irow] = True
            tb.putcol("FLAG", flg)
        except: raise
        finally: tb.close()

    def test_flag_baseline(self):
        """test flag in calmode=baseline"""
        self._chanflag_ms_rows(self.infile,self.flagrows)
        sdtpimaging(infile=self.infile,calmode='baseline', masklist=[5,5],
                    createimage=False)

    def test_flag_image1(self):
        """test flag in imaging (1 chan -> 1chan)"""
        self._chanflag_ms_rows(self.infile,self.flagrows)
        outfile=self.outimage
        cell=[self.cell, self.cell]
        imsize=[self.nrow, 1]
        sdtpimaging(infile=self.infile,calmode='none', createimage=True,
                    outfile=outfile,cell=cell,imsize=imsize,
                    phasecenter=self.center,gridfunction='BOX')
        


def suite():
    return [sdtpimaging_test0,sdtpimaging_test1,sdtpimaging_test2,
            sdtpimaging_flag]
