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
    scannum = tb.getcol('SCAN_NUMBER')
    tb.putcol('SCAN_NUMBER', scannum*0)
    # flag spurious scans
    for irow in [15,16]:
        flg = tb.getcell('FLAG', irow)
        tb.putcell('FLAG', irow, flg*0+1)
    tb.close()
    split(vis='tpimaging/tpimaging.aca.tp.sd.ms', outputvis='tpimaging_1row.ms',datacolumn='data')
    """
    infile = 'tpimaging_1row.ms'
    # phasecenter need to be shifted for -1 pixel do to boundary difference.
    center = 'J2000 23:59:59.81295 -27.00.00.0'
    cell = '5arcsec'
    inmsdata = [0,1,2,3,4,5,6,10,11,12,13,14,15,13,14,100,100,17,18,19]
    nrow = len(inmsdata)
    inflag = numpy.array([False]*nrow)
    inflag[15:17] = True
    prefix=sdtpimaging_unittest_base.taskname+'Flag'
    outimage = prefix+'.image'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.infile)):
            shutil.copytree(self.datapath+self.infile, self.infile)
        self._checkfile(self.infile)
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
        tb.open(self.infile)
        flagchan_pre = tb.getcol('FLAG')
        flagrow_pre = tb.getcol('FLAG_ROW')
        tb.close()
        sdtpimaging(infile=self.infile,calmode='baseline', masklist=[5,5],
                    createimage=False)
        # make sure flag is not changed
        tb.open(self.infile)
        flagchan_post = tb.getcol('FLAG')
        flagrow_post = tb.getcol('FLAG_ROW')
        data = tb.getcol('DATA')
        tb.close()
        self.assertTrue(flagchan_pre.shape==flagchan_post.shape,
                        "Data shape has been changed by task operation")
        self.assertTrue((flagrow_post==flagrow_pre).all(),
                        "FLAG_ROW has been changed by task operation")
        self.assertTrue((flagchan_post==flagchan_pre).all(),
                        "FLAG has been changed by task operation")
        # now check baseline result
        ref_bl = [0,0,0,0,0,0,0,3,3,3,3,3,3,0,0,85,84,0,0,0]
        (npol, nchan, nrow) = data.shape
        self.assertEqual(nchan, 1, 'number of channels is not 1')
        self.assertEqual(nrow, len(ref_bl), 'number of rows is not %d' % len(ref_bl))
        for irow in range(nrow):
            for ipol in range(npol):
                if flagchan_post[ipol][0][irow] > 0: continue
                self.assertEqual(data[ipol][0][irow].real, ref_bl[irow],
                                 "Spectral value differs in row %d: %f (expected: %f)" % (irow, data[ipol][0][irow].real, ref_bl[irow]))
    

    def test_flag_image1(self):
        """test flag in imaging (1 integration -> 1 pixel)"""
        ref_ma = numpy.ma.masked_array(self.inmsdata, self.inflag)
        # invoke task
        outfile=self.outimage
        cell=[self.cell, self.cell]
        imsize=[self.nrow, 1]
        sdtpimaging(infile=self.infile,calmode='none', createimage=True,
                    outfile=outfile,cell=cell,imsize=imsize,
                    phasecenter=self.center,gridfunction='BOX')
        self._test_image(outfile, (self.nrow,1,1,1), ref_ma)
        
    def test_flag_image2(self):
        """test flag in imaging (2 integration -> 1 pixel)"""
        # flag scans 15~17
        self._chanflag_ms_rows(self.infile, [15,16,17])
        # input MS = [0,1,2,3,4,5,6,10,11,12,13,14,15,13,14,F,F,F,18,19]
        # an image pixel should be masked out only if all data mapped
        # to the pixel are flagged
        ref_ma = numpy.ma.masked_array([0.5,2.5,4.5,8,11.5,13.5,14,14,0,18.5],
                                       ([False]*8+[True, False]))
        # invoke task
        cell_factor = 2
        npixx = self.nrow/cell_factor
        outfile=self.outimage
        cell=[qa.tos(qa.mul(self.cell,cell_factor)), self.cell]
        imsize=[npixx, 1]
        center = 'J2000 23:59:59.6259 -27.00.00.0'
        sdtpimaging(infile=self.infile,calmode='none', createimage=True,
                    outfile=outfile,cell=cell,imsize=imsize,
                    phasecenter=center,gridfunction='BOX')
        self._test_image(outfile, (npixx,1,1,1), ref_ma)

    def _test_image(self,imagename, imshape, refval):
        # make sure image exists
        self._checkfile(imagename)
        # compare mask and pixel value
        ia.open(imagename)
        imdata = ia.getchunk()
        immask = ia.getchunk(getmask=True)
        ia.close()
        self.assertTrue(imdata.shape==imshape, "Unexpected image shape: %s (expected: %s)" % (str(imdata.shape), str(list(imshape))))
        # image mask is invert of mask in masked array
        imdata = list(imdata.flatten())
        immask = [(not m) for m in immask.flatten()]
        # image pixel order -R.A.
        imdata.reverse()
        immask.reverse()
        im_ma = numpy.ma.masked_array(imdata,immask)
        self.assertTrue(self._compare_mased_array(im_ma, refval),
                        "Unexpected image pixel value: %s (expected: %s)" % (str(im_ma), str(refval)))

    def _compare_mased_array( self, testval, refval, reltol=1.0e-5 ):
        """
        Check if a masked array of test values is within permissive relative
        difference from refval.
        Returns a boolean.
        testval & refval : two masked arrays to compare
        reltol           : allowed relative difference to consider the two
                           values to be equal. (default 1.e-5)
        """
        maskok = (testval.mask == refval.mask).all()
        if not maskok: return False
        for i in range(len(refval)):
            if refval.mask[i]: continue
            if not numpy.isclose(refval[i], testval[i], rtol=reltol, atol=1.e-7):
                return False
        return True

def suite():
    return [sdtpimaging_test0,sdtpimaging_test1,sdtpimaging_test2,
            sdtpimaging_flag]
