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

from tsdimaging import tsdimaging
import asap as sd

#
# Unit test of sdimaging task.
# 

###
# Base class for sdimaging unit test
###
class sdimaging_unittest_base:
    """
    Base class for sdimaging unit test

    Test data is originally FLS3_all_newcal_SP and created
    by the following script:

    asap_init()
    sd.rc('scantable',storage='disk')
    s=sd.scantable('FLS3_all_newcal_SP',average=False,getpt=False)
    s0=s.get_scan('FLS3a')
    s0.save('FLS3a_HI.asap')
    del s,s0
    s=sd.scantable('FLS3a_HI.asap',average=False)
    s.set_fluxunit('K')
    scannos=s.getscannos()
    res=sd.calfs(s,scannos)
    del s
    res.save('FLS3a_calfs','MS2')
    tb.open('FLS3a_calfs')
    tbsel=tb.query('SCAN_NUMBER<100')
    tbc=tbsel.copy('sdimaging.ms',deep=True,valuecopy=True)
    tbsel.close()
    tb.close()
    tbc.close()

    Furthermore, SYSCAL and POINTING tables are downsized.
    
    """
    taskname='sdimaging'
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdimaging/'
    rawfile='sdimaging.ms'
    postfix='.im'
    ms_nchan = 1024
    phasecenter='J2000 17:18:29 +59.31.23'
    imsize=[75,75]
    cell=['3.0arcmin','3.0arcmin']
    gridfunction='PB'
    minweight0 = 0.
    statsinteg={'blc': numpy.array([0, 0, 0, 0], dtype=numpy.int32),
                'blcf': '17:32:18.690, +57.37.28.536, I, 1.42064e+09Hz',
                'max': numpy.array([ 0.6109162]),
                'maxpos': numpy.array([4, 62,  0,  0], dtype=numpy.int32),
                'maxposf': '17:31:59.439, +60.43.52.421, I, 1.42064e+09Hz',
                'mean': numpy.array([ 0.39524983]),
                'min': numpy.array([ 0.]),
                'minpos': numpy.array([0, 0, 0, 0], dtype=numpy.int32),
                'minposf': '17:32:18.690, +57.37.28.536, I, 1.42064e+09Hz',
                'npts': numpy.array([ 5625.]),
                'rms': numpy.array([ 0.43127564]),
                'sigma': numpy.array([ 0.17257331]),
                'sum': numpy.array([ 2223.28028646]),
                'sumsq': numpy.array([ 1046.2425779]),
                'trc': numpy.array([74, 74,  0,  0], dtype=numpy.int32),
                'trcf': '17:03:03.151, +61.19.10.757, I, 1.42064e+09Hz'}
    keys=['max','mean','min','npts','rms','blc','blcf','trc','trcf','sigma','sum','sumsq']

    def _checkfile( self, name ):
        isthere=os.path.exists(name)
        self.assertEqual(isthere,True,
                         msg='output file %s was not created because of the task failure'%(name))

    def _checkshape(self,name,nx,ny,npol,nchan):
        self._checkfile(name)
        ia.open(name)
        imshape=ia.shape()
        ia.close()
        self.assertEqual(nx,imshape[0],
                    msg='nx does not match')
        self.assertEqual(ny,imshape[1],
                    msg='ny does not match')
        self.assertEqual(npol,imshape[2],
                    msg='npol does not match')
        self.assertEqual(nchan,imshape[3],
                    msg='nchan does not match')
        
    def _checkstats(self,name,ref,ignoremask=False):
        self._checkfile(name)
        ia.open(name)
        if ignoremask:
            def_mask = ia.maskhandler('default')
            ia.calcmask('T')
        stats=ia.statistics(list=True, verbose=True)
        if ignoremask:
            ia.maskhandler('set',def_mask)
        ia.close()
        #for key in stats.keys():
        for key in self.keys:
            message='statistics \'%s\' does not match'%(key)
            if type(stats[key])==str:
                self.assertEqual(stats[key],ref[key],
                                 msg=message)
            else:
                #print stats[key]-ref[key]
                ret=numpy.allclose(stats[key],ref[key])
                self.assertEqual(ret,True,
                                 msg=message)

    def _checkdirax(self, imagename, center, cell, imsize):
        """ Test image center, cell size and imsize"""
        cell = self._format_dir_list(cell)
        imsize = self._format_dir_list(imsize)
        ia.open(imagename)
        csys = ia.coordsys()
        ret = ia.summary()
        ia.close()
        ra_idx = csys.findaxisbyname('ra')
        dec_idx = csys.findaxisbyname('dec')
        ra_unit = ret['axisunits'][ra_idx]
        dec_unit = ret['axisunits'][dec_idx]
        # imsize
        self.assertEqual(imsize[0], ret['shape'][ra_idx],\
                         msg="nx = %d (expected: %d)" % \
                         (imsize[0], ret['shape'][ra_idx]))
        self.assertEqual(imsize[1], ret['shape'][dec_idx],\
                         msg="nx = %d (expected: %d)" % \
                         (imsize[1], ret['shape'][dec_idx]))
        # image center
        tol = "1arcsec"
        cen_arr = center.split()
        cen_ref = me.direction(*cen_arr)
        cen_x = (qa.convert(cen_ref['m0'], 'rad')['value'] % (numpy.pi*2))
        cen_y = qa.convert(cen_ref['m1'], 'rad')['value']
        ref_x = qa.convert(qa.quantity(ret['refval'][ra_idx],ra_unit),'rad')['value']
        ref_x = (ref_x % (numpy.pi*2))
        ref_y = qa.convert(qa.quantity(ret['refval'][dec_idx],dec_unit),'rad')['value']
        tol_val = qa.convert(tol, 'rad')['value']
        self.assertTrue(abs(ref_x-cen_x) < tol_val,
                        msg="center_x = %f %s (expected: %f)" % \
                        (ref_x, ra_unit, cen_x))
        self.assertTrue(abs(ref_y-cen_y) < tol_val,
                        msg="center_y = %f %s (expected: %f)" % \
                        (ref_x, ra_unit, cen_x))
        
        # cell (imager seems to set negative incr for dx)
        dx = - qa.convert(cell[0], ra_unit)['value']
        dy = qa.convert(cell[1], dec_unit)['value']
        incx = ret['incr'][ra_idx]
        incy = ret['incr'][dec_idx]
        self.assertAlmostEqual((incx-dx)/dx, 0., places=5, \
                               msg="cellx = %f %s (expected: %f)" % \
                               (incx, ra_unit, dx))
        self.assertAlmostEqual((incy-dy)/dy, 0., places=5, \
                               msg="celly = %f %s (expected: %f)" % \
                               (incy, dec_unit, dy))

    def _format_dir_list(self, inval):
        if type(inval) == str:
            return [inval, inval]
        elif len(inval) == 1:
            return [inval[0], inval[0]]
        return inval[0:2]
    
###
# Test on bad parameter settings
###
class sdimaging_test0(sdimaging_unittest_base,unittest.TestCase):
    """
    Test on bad parameter setting
    """
    # Input and output names
    prefix = sdimaging_unittest_base.taskname+'Test0'
    badid = '99'
    outfile = prefix+sdimaging_unittest_base.postfix

    def setUp(self):
        if os.path.exists(self.rawfile):
            shutil.rmtree(self.rawfile)
        shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(tsdimaging)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test000(self):
        """Test 000: Default parameters"""
        # argument verification error
        res=tsdimaging()
        self.assertFalse(res)

    def test001(self):
        """Test001: Bad mode"""
        # argument verification error
        res=tsdimaging(infiles=self.rawfile,mode='badmode',outfile=self.outfile)
        self.assertFalse(res)

    def test002(self):
        """Test002: Bad field id"""
        outfile=self.prefix+self.postfix
        try:
            res=tsdimaging(infiles=self.rawfile,field=self.badid,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Field Expression: Partial or no match for Field ID list [%s]' % self.badid)
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test003(self):
        """Test003: Bad spectral window id"""
        try:
            res=tsdimaging(infiles=self.rawfile,spw=self.badid,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('No Spw ID(s) matched specifications')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test004(self):
        """Test004: Bad antenna id"""
        try:
            res=tsdimaging(infiles=self.rawfile,antenna=self.badid,
                          imsize=self.imsize,cell=self.cell,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('No match found for the antenna specificion')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))
        
    def test005(self):
        """Test005: Bad stokes parameter"""
        try:
            res=tsdimaging(infiles=self.rawfile,stokes='BAD',outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Stokes selection BAD is currently not supported.')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))
        
    def test006(self):
        """Test006: Bad gridfunction"""
        # argument verification error
        res=tsdimaging(infiles=self.rawfile,gridfunction='BAD',outfile=self.outfile)
        self.assertFalse(res)

    def test007(self):
        """Test007: Bad scanlist"""
        try:
            res=tsdimaging(infiles=self.rawfile,scan=self.badid,outfile=self.outfile)
        except Exception, e:
            pos=str(e).find('Failed to generate output image')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test008(self):
        """Test008: Existing outfile with overwrite=False"""
        f=open(self.outfile,'w')
        print >> f, 'existing file'
        f.close()
        try:
            res=tsdimaging(infiles=self.rawfile,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Output file \'%s\' exists.'%(self.outfile))
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test009(self):
        """Test009: Bad phasecenter string"""
        try:
            res=tsdimaging(infiles=self.rawfile,outfile=self.outfile,cell=self.cell,imsize=self.imsize,phasecenter='This is bad')
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Empty QuantumHolder argument for asQuantumDouble')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test010(self):
        """Test010: Bad phasecenter reference (J2000 is assumed)"""
        # default for unknown direction frame is J2000 
        refimage=self.outfile+'2'
        tsdimaging(infiles=self.rawfile,outfile=self.outfile,cell=self.cell,imsize=self.imsize,phasecenter=self.phasecenter.replace('J2000','J3000'),minweight=self.minweight0)
        tsdimaging(infiles=self.rawfile,outfile=refimage,cell=self.cell,imsize=self.imsize,phasecenter=self.phasecenter,minweight=self.minweight0)
        tb.open(self.outfile)
        chunk=tb.getcol('map')
        tb.close()
        tb.open(refimage)
        refchunk=tb.getcol('map')
        tb.close()
        ret=all(chunk.flatten()==refchunk.flatten())
        #print ret
        self.assertTrue(ret)

    def test011(self):
        """Test011: Bad pointingcolumn name"""
        # argument verification error
        res=tsdimaging(infiles=self.rawfile,outfile=self.outfile,cell=self.cell,imsize=self.imsize,phasecenter=self.phasecenter,pointingcolumn='non_exist')
        self.assertFalse(res)

    def test012(self):
        """Test012: Bad imsize"""
        # This does not raise error anymore.
        res=tsdimaging(infiles=self.rawfile,outfile=self.outfile,cell=self.cell,imsize=[1,0],phasecenter=self.phasecenter)
        self.assertFalse(res)

    def test013(self):
        """Test013: Bad cell size"""
        # empty image will be created
        try:
            res=tsdimaging(infiles=self.rawfile,outfile=self.outfile,cell=[0.,0.],imsize=self.imsize,phasecenter=self.phasecenter)
            self.assertFail(msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Infinite resolution not possible.')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test014(self):
        """Test014: Too fine resolution (smaller than original channel width"""
        try:
            specunit = 'GHz'
            start = '%f%s' % (1.4202, specunit)
            width = '%e%s' % (1.0e-10, specunit)
            res=tsdimaging(infiles=self.rawfile,outfile=self.outfile,cell=self.cell,imsize=self.imsize,phasecenter=self.phasecenter,gridfunction=self.gridfunction,mode='frequency',nchan=10,start=start,width=width)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Output frequency grid cannot be calculated:  please check start and width parameters')
            #pos=str(e).find('calcChanFreqs failed, check input start and width parameters')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test015(self):
        """Test015: negative minweight"""
        res=tsdimaging(infiles=self.rawfile,outfile=self.outfile,cell=self.cell,imsize=self.imsize,phasecenter=self.phasecenter,minweight=-1.)
        self.assertFalse(res)


###
# Test channel imaging
###
class sdimaging_test1(sdimaging_unittest_base,unittest.TestCase):
    """
    Test channel imaging

       - integrated image
       - full channel image
       - selected channel image
       - BOX and SF imaging (default is PB)
       - two polarization imaging (XX and YY, default is Stokes I)
       - empty phasecenter
       - settting minweight = 0.2
       
    """
    # Input and output names
    prefix=sdimaging_unittest_base.taskname+'Test1'
    outfile=prefix+sdimaging_unittest_base.postfix
    mode='channel'

    def setUp(self):
        if os.path.exists(self.rawfile):
            shutil.rmtree(self.rawfile)
        shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(tsdimaging)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test100(self):
        """Test 100: Integrated image"""
        res=tsdimaging(infiles=self.rawfile,mode=self.mode,outfile=self.outfile,cell=self.cell,imsize=self.imsize,phasecenter=self.phasecenter,gridfunction=self.gridfunction,nchan=1,start=0,width=self.ms_nchan,minweight=self.minweight0)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        self._checkshape(self.outfile,self.imsize[0],self.imsize[1],1,1)
        refstats=self.statsinteg
        self._checkstats(self.outfile,refstats,ignoremask=True)

    def test101(self):
        """Test 101: Full channel image (nchan = -1)"""
        res=tsdimaging(infiles=self.rawfile,mode=self.mode,outfile=self.outfile,cell=self.cell,imsize=self.imsize,phasecenter=self.phasecenter,gridfunction=self.gridfunction,nchan=-1,start="",width="",minweight=self.minweight0)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        self._checkshape(self.outfile,self.imsize[0],self.imsize[1],1,self.ms_nchan)
        refstats={'blc': numpy.array([0, 0, 0, 0], dtype=numpy.int32),
                  'blcf': '17:32:18.690, +57.37.28.536, I, 1.419395e+09Hz',
                  'max': numpy.array([ 24.77152824]),
                  'maxpos': numpy.array([ 59,  21,   0, 605], dtype=numpy.int32),
                  'maxposf': '17:10:00.642, +58.42.19.808, I, 1.420872e+09Hz',
                  'mean': numpy.array([ 0.39542111]),
                  'min': numpy.array([-1.84636593]),
                  'minpos': numpy.array([  73,    6,    0, 1023], dtype=numpy.int32),
                  'minposf': '17:04:54.966, +57.55.36.907, I, 1.421893e+09Hz',
                  'npts': numpy.array([ 5760000.]),
                  'rms': numpy.array([ 1.01357317]),
                  'sigma': numpy.array([ 0.93325921]),
                  'sum': numpy.array([ 2277625.60731485]),
                  'sumsq': numpy.array([ 5917423.42281288]),
                  'trc': numpy.array([  74,   74,    0, 1023], dtype=numpy.int32),
                  'trcf': '17:03:03.151, +61.19.10.757, I, 1.421893e+09Hz'}
        self._checkstats(self.outfile,refstats,ignoremask=True)

        
    def test102(self):
        """Test 102: Full channel image"""
        tb.open(self.rawfile)
        if 'FLOAT_DATA' in tb.colnames():
            nchan=tb.getcell('FLOAT_DATA').shape[1]
        else:
            nchan=tb.getcell('DATA').shape[1]
        tb.close()
        res=tsdimaging(infiles=self.rawfile,mode=self.mode,outfile=self.outfile,cell=self.cell,imsize=self.imsize,phasecenter=self.phasecenter,gridfunction=self.gridfunction,nchan=nchan,start=0,width=1,minweight=self.minweight0)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        self._checkshape(self.outfile,self.imsize[0],self.imsize[1],1,nchan)
        refstats={'blc': numpy.array([0, 0, 0, 0], dtype=numpy.int32),
                  'blcf': '17:32:18.690, +57.37.28.536, I, 1.419395e+09Hz',
                  'max': numpy.array([ 24.77152824]),
                  'maxpos': numpy.array([ 59,  21,   0, 605], dtype=numpy.int32),
                  'maxposf': '17:10:00.642, +58.42.19.808, I, 1.420872e+09Hz',
                  'mean': numpy.array([ 0.39542111]),
                  'min': numpy.array([-1.84636593]),
                  'minpos': numpy.array([  73,    6,    0, 1023], dtype=numpy.int32),
                  'minposf': '17:04:54.966, +57.55.36.907, I, 1.421893e+09Hz',
                  'npts': numpy.array([ 5760000.]),
                  'rms': numpy.array([ 1.01357317]),
                  'sigma': numpy.array([ 0.93325921]),
                  'sum': numpy.array([ 2277625.60731485]),
                  'sumsq': numpy.array([ 5917423.42281288]),
                  'trc': numpy.array([  74,   74,    0, 1023], dtype=numpy.int32),
                  'trcf': '17:03:03.151, +61.19.10.757, I, 1.421893e+09Hz'}
        self._checkstats(self.outfile,refstats,ignoremask=True)

    def test103(self):
        """Test 103: Selected channel image"""
        nchan=40
        res=tsdimaging(infiles=self.rawfile,mode=self.mode,outfile=self.outfile,cell=self.cell,imsize=self.imsize,phasecenter=self.phasecenter,gridfunction=self.gridfunction,nchan=nchan,start=400,width=10,minweight=self.minweight0)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        self._checkshape(self.outfile,self.imsize[0],self.imsize[1],1,nchan)
        refstats={'blc': numpy.array([0, 0, 0, 0], dtype=numpy.int32),
                  'blcf': '17:32:18.690, +57.37.28.536, I, 1.42038e+09Hz',
                  'max': numpy.array([ 14.79568005]),
                  'maxpos': numpy.array([57, 20,  0, 20], dtype=numpy.int32),
                  'maxposf': '17:10:47.496, +58.39.30.813, I, 1.42087e+09Hz',
                  'mean': numpy.array([ 0.82293006]),
                  'min': numpy.array([-0.08763941]),
                  'minpos': numpy.array([61, 71,  0, 35], dtype=numpy.int32),
                  'minposf': '17:08:30.980, +61.12.02.893, I, 1.42124e+09Hz',
                  'npts': numpy.array([ 225000.]),
                  'rms': numpy.array([ 1.54734671]),
                  'sigma': numpy.array([ 1.31037237]),
                  'sum': numpy.array([ 185159.263672]),
                  'sumsq': numpy.array([ 538713.45272028]),
                  'trc': numpy.array([74, 74,  0, 39], dtype=numpy.int32),
                  'trcf': '17:03:03.151, +61.19.10.757, I, 1.42133e+09Hz'}
        self._checkstats(self.outfile,refstats,ignoremask=True)

    def test104(self):
        """Test 104: Box-car gridding"""
        nchan=40
        res=tsdimaging(infiles=self.rawfile,mode=self.mode,outfile=self.outfile,cell=self.cell,imsize=self.imsize,phasecenter=self.phasecenter,gridfunction='BOX',nchan=nchan,start=400,width=10,minweight=self.minweight0)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        self._checkshape(self.outfile,self.imsize[0],self.imsize[1],1,nchan)
        refstats={'blc': numpy.array([0, 0, 0, 0], dtype=numpy.int32),
                  'blcf': '17:32:18.690, +57.37.28.536, I, 1.42038e+09Hz',
                  'max': numpy.array([ 15.64525127]),
                  'maxpos': numpy.array([58, 20,  0, 20], dtype=numpy.int32),
                  'maxposf': '17:10:24.433, +58.39.25.476, I, 1.42087e+09Hz',
                  'mean': numpy.array([ 0.66097592]),
                  'min': numpy.array([-0.42533547]),
                  'minpos': numpy.array([69, 62,  0, 38], dtype=numpy.int32),
                  'minposf': '17:05:23.086, +60.44.01.427, I, 1.42131e+09Hz',
                  'npts': numpy.array([ 225000.]),
                  'rms': numpy.array([ 1.38591599]),
                  'sigma': numpy.array([ 1.2181464]),
                  'sum': numpy.array([ 148719.58227018]),
                  'sumsq': numpy.array([ 432171.72687429]),
                  'trc': numpy.array([74, 74,  0, 39], dtype=numpy.int32),
                  'trcf': '17:03:03.151, +61.19.10.757, I, 1.42133e+09Hz'}
        self._checkstats(self.outfile,refstats,ignoremask=True)

    def test105(self):
        """Test 105: Prolate Spheroidal gridding"""
        nchan=40
        res=tsdimaging(infiles=self.rawfile,mode=self.mode,outfile=self.outfile,cell=self.cell,imsize=self.imsize,phasecenter=self.phasecenter,gridfunction='SF',nchan=nchan,start=400,width=10,minweight=self.minweight0)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        self._checkshape(self.outfile,self.imsize[0],self.imsize[1],1,nchan)
        refstats={'blc': numpy.array([0, 0, 0, 0], dtype=numpy.int32),
                  'blcf': '17:32:18.690, +57.37.28.536, I, 1.42038e+09Hz',
                  'max': numpy.array([ 15.13189793]),
                  'maxpos': numpy.array([58, 21,  0, 20], dtype=numpy.int32),
                  'maxposf': '17:10:23.737, +58.42.25.413, I, 1.42087e+09Hz',
                  'mean': numpy.array([ 0.773227]),
                  'min': numpy.array([-0.07284018]),
                  'minpos': numpy.array([ 5, 67,  0, 30], dtype=numpy.int32),
                  'minposf': '17:31:41.090, +60.59.00.556, I, 1.42112e+09Hz',
                  'npts': numpy.array([ 225000.]),
                  'rms': numpy.array([ 1.49926317]),
                  'sigma': numpy.array([ 1.28449107]),
                  'sum': numpy.array([ 173976.07570213]),
                  'sumsq': numpy.array([ 505752.74505987]),
                  'trc': numpy.array([74, 74,  0, 39], dtype=numpy.int32),
                  'trcf': '17:03:03.151, +61.19.10.757, I, 1.42133e+09Hz'}
        self._checkstats(self.outfile,refstats,ignoremask=True)

    def test106(self):
        """Test 106: Imaging two polarization separately (XX and YY, not Stokes I)"""
        nchan=40
        res=tsdimaging(infiles=self.rawfile,mode=self.mode,stokes='XXYY',outfile=self.outfile,cell=self.cell,imsize=self.imsize,phasecenter=self.phasecenter,gridfunction='PB',nchan=nchan,start=400,width=10,minweight=self.minweight0)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        self._checkshape(self.outfile,self.imsize[0],self.imsize[1],2,nchan)
        refstats={'blc': numpy.array([0, 0, 0, 0], dtype=numpy.int32),
                  'blcf': '17:32:18.690, +57.37.28.536, XX, 1.42038e+09Hz',
                  'max': numpy.array([ 15.057868]),
                  'maxpos': numpy.array([57, 20,  1, 20], dtype=numpy.int32),
                  'maxposf': '17:10:47.496, +58.39.30.813, YY, 1.42087e+09Hz',
                  'mean': numpy.array([ 0.82292841]),
                  'min': numpy.array([-0.41953856]),
                  'minpos': numpy.array([10,  3,  1, 31], dtype=numpy.int32),
                  'minposf': '17:28:37.170, +57.47.49.422, YY, 1.42114e+09Hz',
                  'npts': numpy.array([ 450000.]),
                  'rms': numpy.array([ 1.55436146]),
                  'sigma': numpy.array([ 1.31864787]),
                  'sum': numpy.array([ 370317.78554221]),
                  'sumsq': numpy.array([ 1087217.77687839]),
                  'trc': numpy.array([74, 74,  1, 39], dtype=numpy.int32),
                  'trcf': '17:03:03.151, +61.19.10.757, YY, 1.42133e+09Hz'}
        self._checkstats(self.outfile,refstats,ignoremask=True)

    def test107(self):
        """Test 107: Gaussian gridding"""
        nchan=40
        res=tsdimaging(infiles=self.rawfile,mode=self.mode,outfile=self.outfile,cell=self.cell,imsize=self.imsize,phasecenter=self.phasecenter,gridfunction='GAUSS',nchan=nchan,start=400,width=10,minweight=self.minweight0)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        self._checkshape(self.outfile,self.imsize[0],self.imsize[1],1,nchan)
        refstats={'blc': numpy.array([0, 0, 0, 0], dtype=numpy.int32),
                  'blcf': '17:32:18.690, +57.37.28.536, I, 1.42038e+09Hz',
                  'max': numpy.array([ 15.28046036]),
                  'maxpos': numpy.array([58, 21,  0, 20], dtype=numpy.int32),
                  'maxposf': '17:10:23.737, +58.42.25.413, I, 1.42087e+09Hz',
                  'mean': numpy.array([ 0.75082603]),
                  'min': numpy.array([-0.14009152]),
                  'minpos': numpy.array([34, 69,  0, 33], dtype=numpy.int32),
                  'minposf': '17:19:43.545, +61.07.22.487, I, 1.42119e+09Hz',
                  'npts': numpy.array([ 225000.]),
                  'rms': numpy.array([ 1.47686982]),
                  'sigma': numpy.array([ 1.2717751]),
                  'sum': numpy.array([ 168935.85698331]),
                  'sumsq': numpy.array([ 490757.49952306]),
                  'trc': numpy.array([74, 74,  0, 39], dtype=numpy.int32),
                  'trcf': '17:03:03.151, +61.19.10.757, I, 1.42133e+09Hz'}
        self._checkstats(self.outfile,refstats,ignoremask=True)

    def test108(self):
        """Test 108: Gaussian*Jinc gridding"""
        nchan=40
        res=tsdimaging(infiles=self.rawfile,mode=self.mode,outfile=self.outfile,cell=self.cell,imsize=self.imsize,phasecenter=self.phasecenter,gridfunction='GJINC',nchan=nchan,start=400,width=10,minweight=self.minweight0)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        self._checkshape(self.outfile,self.imsize[0],self.imsize[1],1,nchan)
        refstats={'blc':numpy.array([0, 0, 0, 0], dtype=numpy.int32),
                  'blcf': '17:32:18.690, +57.37.28.536, I, 1.42038e+09Hz',
                  'max':numpy.array([ 15.31498909]),
                  'maxpos':numpy.array([58, 21,  0, 20], dtype=numpy.int32),
                  'maxposf': '17:10:23.737, +58.42.25.413, I, 1.42087e+09Hz',
                  'mean':numpy.array([ 0.72415226]),
                  'min':numpy.array([-0.16245638]),
                  'minpos':numpy.array([68, 69,  0, 36], dtype=numpy.int32),
                  'minposf': '17:05:39.206, +61.05.09.055, I, 1.42126e+09Hz',
                  'npts':numpy.array([ 225000.]),
                  'rms':numpy.array([ 1.44985926]),
                  'sigma':numpy.array([ 1.25606618]),
                  'sum':numpy.array([ 162934.25891985]),
                  'sumsq':numpy.array([ 472970.63791706]),
                  'trc':numpy.array([74, 74,  0, 39], dtype=numpy.int32),
                  'trcf': '17:03:03.151, +61.19.10.757, I, 1.42133e+09Hz'}
        self._checkstats(self.outfile,refstats,ignoremask=True)

    def test109(self):
        """Test 109: Empty phasecenter (auto-calculation)"""
        nchan=40
        res=tsdimaging(infiles=self.rawfile,mode=self.mode,outfile=self.outfile,cell=self.cell,imsize=self.imsize,nchan=nchan,start=400,width=10,minweight=self.minweight0)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        self._checkshape(self.outfile,self.imsize[0],self.imsize[1],1,nchan)
        refstats={'blc': numpy.array([0, 0, 0, 0], dtype=numpy.int32),
                  'blcf': '17:31:48.220, +57.36.09.784, I, 1.42038e+09Hz',
                  'max': numpy.array([ 15.64525127]),
                  'maxpos': numpy.array([57, 20,  0, 20], dtype=numpy.int32),
                  'maxposf': '17:10:17.816, +58.38.11.961, I, 1.42087e+09Hz',
                  'mean': numpy.array([ 0.66039867]),
                  'min': numpy.array([-0.42533547]),
                  'minpos': numpy.array([68, 63,  0, 38], dtype=numpy.int32),
                  'minposf': '17:05:16.976, +60.45.51.215, I, 1.42131e+09Hz',
                  'npts': numpy.array([ 225000.]),
                  'rms': numpy.array([ 1.38517249]),
                  'sigma': numpy.array([ 1.21761365]),
                  'sum': numpy.array([ 148589.70138012]),
                  'sumsq': numpy.array([ 431708.13145918]),
                  'trc': numpy.array([74, 74,  0, 39], dtype=numpy.int32),
                  'trcf': '17:02:33.828, +61.17.52.040, I, 1.42133e+09Hz'}
        self._checkstats(self.outfile,refstats,ignoremask=True)

    def test110(self):
        """Test 110: setting minweight=70."""
        nchan=40
        res=tsdimaging(infiles=self.rawfile,mode=self.mode,outfile=self.outfile,cell=self.cell,imsize=self.imsize,phasecenter=self.phasecenter,gridfunction='GJINC',nchan=nchan,start=400,width=10,minweight=70.)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        self._checkshape(self.outfile,self.imsize[0],self.imsize[1],1,nchan)
        refstats={'blc':numpy.array([0, 0, 0, 0], dtype=numpy.int32),
                  'blcf': '17:32:18.690, +57.37.28.536, I, 1.42038e+09Hz',
                  'max':numpy.array([ 15.31498909]),
                  'maxpos':numpy.array([58, 21,  0, 20], dtype=numpy.int32),
                  'maxposf': '17:10:23.737, +58.42.25.413, I, 1.42087e+09Hz',
                  'mean': numpy.array([ 0.96643395]),
                  'min': numpy.array([-0.01385191]),
                  'minpos': numpy.array([19, 63,  0, 33], dtype=numpy.int32),
                  'minposf': '17:25:51.974, +60.48.38.410, I, 1.42119e+09Hz',
                  'npts': numpy.array([ 143920.]),
                  'rms': numpy.array([ 1.66819704]),
                  'sigma': numpy.array([ 1.35974246]),
                  'sum': numpy.array([ 139089.17359187]),
                  'sumsq': numpy.array([ 400512.27532199]),
                  'trc':numpy.array([74, 74,  0, 39], dtype=numpy.int32),
                  'trcf': '17:03:03.151, +61.19.10.757, I, 1.42133e+09Hz'}
        self._checkstats(self.outfile,refstats)
        


###
# Test frequency imaging
###
class sdimaging_test2(sdimaging_unittest_base,unittest.TestCase):
    """
    Test frequency imaging

       - integrated image
       - selected frequency image
       
    """
    # Input and output names
    prefix=sdimaging_unittest_base.taskname+'Test2'
    outfile=prefix+sdimaging_unittest_base.postfix
    unit='GHz'
    mode = "frequency"

    def setUp(self):
        if os.path.exists(self.rawfile):
            shutil.rmtree(self.rawfile)
        shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(tsdimaging)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test200(self):
        """Test 200: Integrated image"""
        nchan = 1
        ms.open(self.rawfile)
        spwinfo =  ms.getspectralwindowinfo()
        ms.close()
        spwid0 = spwinfo.keys()[0]
        start = '%fHz' % (spwinfo[spwid0]['Chan1Freq']+0.5*(spwinfo[spwid0]['TotalWidth']-spwinfo[spwid0]['ChanWidth'])) 
        width = '%fHz' % (spwinfo[spwid0]['TotalWidth'])
        res=tsdimaging(infiles=self.rawfile,mode=self.mode,outfile=self.outfile,cell=self.cell,imsize=self.imsize,phasecenter=self.phasecenter,gridfunction=self.gridfunction,nchan=nchan,start=start,width=width,minweight=self.minweight0)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        self._checkshape(self.outfile,self.imsize[0],self.imsize[1],1,1)
        refstats=self.statsinteg
        self._checkstats(self.outfile,refstats,ignoremask=True)
        
    def test201(self):
        """Test 201: Full channel image (mode='frequency', nchan = -1)"""
        nchan = -1
        start = ''
        width = ''
        res=tsdimaging(infiles=self.rawfile,mode=self.mode,outfile=self.outfile,cell=self.cell,imsize=self.imsize,phasecenter=self.phasecenter,gridfunction=self.gridfunction,nchan=nchan,start=start,width=width,minweight=self.minweight0)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        self._checkshape(self.outfile,self.imsize[0],self.imsize[1],1,self.ms_nchan)
        refstats={'blc': numpy.array([0, 0, 0, 0], dtype=numpy.int32),
                  'blcf': '17:32:18.690, +57.37.28.536, I, 1.419395e+09Hz',
                  'max': numpy.array([ 24.77152824]),
                  'maxpos': numpy.array([ 59,  21,   0, 605], dtype=numpy.int32),
                  'maxposf': '17:10:00.642, +58.42.19.808, I, 1.420872e+09Hz',
                  'mean': numpy.array([ 0.39542111]),
                  'min': numpy.array([-1.84636593]),
                  'minpos': numpy.array([  73,    6,    0, 1023], dtype=numpy.int32),
                  'minposf': '17:04:54.966, +57.55.36.907, I, 1.421893e+09Hz',
                  'npts': numpy.array([ 5760000.]),
                  'rms': numpy.array([ 1.01357317]),
                  'sigma': numpy.array([ 0.93325921]),
                  'sum': numpy.array([ 2277625.60731485]),
                  'sumsq': numpy.array([ 5917423.42281288]),
                  'trc': numpy.array([  74,   74,    0, 1023], dtype=numpy.int32),
                  'trcf': '17:03:03.151, +61.19.10.757, I, 1.421893e+09Hz'}
        self._checkstats(self.outfile,refstats,ignoremask=True)

    def test202(self):
        """Test 202: Selected frequency image"""
        nchan = 100
        start = "%f%s" % (1.4202, self.unit)
        width = "%f%s" % (1.0e-5, self.unit)
        res=tsdimaging(infiles=self.rawfile,mode=self.mode,outfile=self.outfile,cell=self.cell,imsize=self.imsize,phasecenter=self.phasecenter,gridfunction=self.gridfunction,nchan=nchan,start=start,width=width,minweight=self.minweight0)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        self._checkshape(self.outfile,self.imsize[0],self.imsize[1],1,nchan)
        refstats={'blc': numpy.array([0, 0, 0, 0], dtype=numpy.int32),
                  'blcf': '17:32:18.690, +57.37.28.536, I, 1.4202e+09Hz',
                  'max': numpy.array([ 21.55560875]),
                  'maxpos': numpy.array([59, 21,  0, 67], dtype=numpy.int32),
                  'maxposf': '17:10:00.642, +58.42.19.808, I, 1.42087e+09Hz',
                  'mean': numpy.array([ 0.80467233]),
                  'min': numpy.array([-0.27736959]),
                  'minpos': numpy.array([58, 71,  0, 10], dtype=numpy.int32),
                  'minposf': '17:09:45.684, +61.12.21.875, I, 1.4203e+09Hz',
                  'npts': numpy.array([ 562500.]),
                  'rms': numpy.array([ 1.56429076]),
                  'sigma': numpy.array([ 1.3414586]),
                  'sum': numpy.array([ 452628.18628213]),
                  'sumsq': numpy.array([ 1376440.6075593]),
                  'trc': numpy.array([74, 74,  0, 99], dtype=numpy.int32),
                  'trcf': '17:03:03.151, +61.19.10.757, I, 1.42119e+09Hz'}
        self._checkstats(self.outfile,refstats,ignoremask=True)
        
    def test203(self):
        """Test 203: Selected frequency image with other frequency unit"""
        nchan=100
        loc_unit='MHz'
        start = "%f%s" % (1420.2, loc_unit)
        width = "%f%s" % (0.01, loc_unit)
        res=tsdimaging(infiles=self.rawfile,mode=self.mode,outfile=self.outfile,cell=self.cell,imsize=self.imsize,phasecenter=self.phasecenter,gridfunction=self.gridfunction,nchan=nchan,start=start,width=width,minweight=self.minweight0)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        self._checkshape(self.outfile,self.imsize[0],self.imsize[1],1,nchan)
        refstats={'blc': numpy.array([0, 0, 0, 0], dtype=numpy.int32),
                  'blcf': '17:32:18.690, +57.37.28.536, I, 1.4202e+09Hz',
                  'max': numpy.array([ 21.55560875]),
                  'maxpos': numpy.array([59, 21,  0, 67], dtype=numpy.int32),
                  'maxposf': '17:10:00.642, +58.42.19.808, I, 1.42087e+09Hz',
                  'mean': numpy.array([ 0.80467233]),
                  'min': numpy.array([-0.27736959]),
                  'minpos': numpy.array([58, 71,  0, 10], dtype=numpy.int32),
                  'minposf': '17:09:45.684, +61.12.21.875, I, 1.4203e+09Hz',
                  'npts': numpy.array([ 562500.]),
                  'rms': numpy.array([ 1.56429076]),
                  'sigma': numpy.array([ 1.3414586]),
                  'sum': numpy.array([ 452628.18628213]),
                  'sumsq': numpy.array([ 1376440.6075593]),
                  'trc': numpy.array([74, 74,  0, 99], dtype=numpy.int32),
                  'trcf': '17:03:03.151, +61.19.10.757, I, 1.42119e+09Hz'}
        self._checkstats(self.outfile,refstats,ignoremask=True)
        
###
# Test velocity imaging
###
class sdimaging_test3(sdimaging_unittest_base,unittest.TestCase):
    """
    Test velocity imaging

       - integrated image
       - selected velocity image
       
    """
    # Input and output names
    prefix=sdimaging_unittest_base.taskname+'Test3'
    outfile=prefix+sdimaging_unittest_base.postfix
    unit='km/s'
    mode = "velocity"

    def setUp(self):
        if os.path.exists(self.rawfile):
            shutil.rmtree(self.rawfile)
        shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(tsdimaging)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test300(self):
        """Test 300: Integrated image"""
        spwid = '0'
        nchan = 1
        restfreq = '1420405800.0Hz'
        ms.open(self.rawfile)
        spwinfo =  ms.getspectralwindowinfo()
        ms.close()
        chan0_freq = spwinfo[spwid]['Chan1Freq']
        bandwidth = spwinfo[spwid]['TotalWidth']
        chanwidth = spwinfo[spwid]['ChanWidth']
        cent_freq = me.frequency(spwinfo[spwid]['Frame'],
                                 qa.quantity(chan0_freq+0.5*(bandwidth-chanwidth),'Hz'))
        cent_vel = me.todoppler('radio', cent_freq, restfreq)
        # band-edge frequencies
        start_freq = me.frequency(spwinfo[spwid]['Frame'],
                                  qa.quantity(chan0_freq-0.5*chanwidth,'Hz'))
        start_vel = me.todoppler('radio', start_freq, restfreq)
        end_freq = me.frequency(spwinfo[spwid]['Frame'],
                                qa.add(start_freq['m0'],
                                       qa.quantity(bandwidth,'Hz')))
        end_vel = me.todoppler('radio', end_freq, restfreq)
        start = qa.tos(cent_vel['m0'])
        width = qa.tos(qa.sub(start_vel['m0'],end_vel['m0']))
        
        res=tsdimaging(infiles=self.rawfile,mode=self.mode,restfreq=restfreq,spw=spwid,outfile=self.outfile,cell=self.cell,imsize=self.imsize,phasecenter=self.phasecenter,gridfunction=self.gridfunction,nchan=nchan,start=start,width=width,minweight=self.minweight0)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        self._checkshape(self.outfile,self.imsize[0],self.imsize[1],1,1)
        refstats=self.statsinteg
        self._checkstats(self.outfile,refstats,ignoremask=True)
        
    def test301(self):
        """Test 301: Selected velocity image"""
        nchan=100
        start = "%f%s" % (-200.0, self.unit)
        width = "%f%s" % (2.0, self.unit)
        res=tsdimaging(infiles=self.rawfile,mode=self.mode,outfile=self.outfile,cell=self.cell,imsize=self.imsize,phasecenter=self.phasecenter,gridfunction=self.gridfunction,nchan=nchan,start=start,width=width,minweight=self.minweight0)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        self._checkshape(self.outfile,self.imsize[0],self.imsize[1],1,nchan)
        refstats={'blc': numpy.array([0, 0, 0, 0], dtype=numpy.int32),
                  'blcf': '17:32:18.690, +57.37.28.536, I, 1.421353e+09Hz',
                  'max': numpy.array([ 21.97223091]),
                  'maxpos': numpy.array([ 4,  5,  0, 50], dtype=numpy.int32),
                  'maxposf': '17:30:54.243, +57.53.03.440, I, 1.42088e+09Hz',
                  'mean': numpy.array([ 0.84673187]),
                  'min': numpy.array([-0.27300295]),
                  'minpos': numpy.array([61, 71,  0, 16], dtype=numpy.int32),
                  'minposf': '17:08:30.980, +61.12.02.893, I, 1.421202e+09Hz',
                  'npts': numpy.array([ 562500.]),
                  'rms': numpy.array([ 1.6305207]),
                  'sigma': numpy.array([ 1.3934297]),
                  'sum': numpy.array([ 476286.67594505]),
                  'sumsq': numpy.array([ 1495461.22406453]),
                  'trc': numpy.array([74, 74,  0, 99], dtype=numpy.int32),
                  'trcf': '17:03:03.151, +61.19.10.757, I, 1.420415e+09Hz'}
        self._checkstats(self.outfile,refstats,ignoremask=True)

    def test302(self):
        """Test 302: Selected velocity image (different rest frequency)"""
        nchan = 100
        start = "%f%s" % (-100.0, self.unit)
        width = "%f%s" % (2.0, self.unit)
        res=tsdimaging(infiles=self.rawfile,mode=self.mode,restfreq='1.420GHz',outfile=self.outfile,cell=self.cell,imsize=self.imsize,phasecenter=self.phasecenter,gridfunction=self.gridfunction,nchan=nchan,start=start,width=width,minweight=self.minweight0)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        self._checkshape(self.outfile,self.imsize[0],self.imsize[1],1,nchan)
        refstats={'blc': numpy.array([0, 0, 0, 0], dtype=numpy.int32),
                  'blcf': '17:32:18.690, +57.37.28.536, I, 1.420474e+09Hz',
                  'max': numpy.array([ 1.61916351]),
                  'maxpos': numpy.array([ 4, 52,  0, 33], dtype=numpy.int32),
                  'maxposf': '17:31:47.043, +60.13.54.473, I, 1.420161e+09Hz',
                  'mean': numpy.array([ 0.12395606]),
                  'min': numpy.array([-0.41655564]),
                  'minpos': numpy.array([60, 71,  0, 93], dtype=numpy.int32),
                  'minposf': '17:08:55.879, +61.12.09.501, I, 1.419593e+09Hz',
                  'npts': numpy.array([ 562500.]),
                  'rms': numpy.array([ 0.19268371]),
                  'sigma': numpy.array([ 0.14751931]),
                  'sum': numpy.array([ 69725.28195545]),
                  'sumsq': numpy.array([ 20883.94443161]),
                  'trc': numpy.array([74, 74,  0, 99], dtype=numpy.int32),
                  'trcf': '17:03:03.151, +61.19.10.757, I, 1.419536e+09Hz'}
        self._checkstats(self.outfile,refstats,ignoremask=True)

###
# Test auto-resolution of spatial gridding parameters
###
class sdimaging_test4(sdimaging_unittest_base,unittest.TestCase):
    """
    Test auto-resolution of spatial gridding parameters

       - manual setting
       - all
       - phasecenter
       - cell (get rest freq from table)
       - imsize
    """
    prefix=sdimaging_unittest_base.taskname+'Test4'
    outfile=prefix+sdimaging_unittest_base.postfix
    # auto calculation result of imsize
    cell_auto = "162.545308arcsec"
    imsize_auto = [73, 68]
    phasecenter_auto = "J2000 17:17:59.03 59.30.04.104"
    # manual setup
    imsize = [40, 35]
    cell = ["320arcsec", "350arcsec"]
    phasecenter = "J2000 17:18:05 59.30.05"

    def setUp(self):
        if os.path.exists(self.rawfile):
            shutil.rmtree(self.rawfile)
        shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if os.path.exists(self.outfile):
            shutil.rmtree(self.outfile)

        default(tsdimaging)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )


    def test401(self):
        """test 401: Set phasecenter, cell, and imsize manually"""
        res=tsdimaging(infiles=self.rawfile,outfile=self.outfile,cell=self.cell,imsize=self.imsize,phasecenter=self.phasecenter,nchan=1,start=0,width=1024,minweight=self.minweight0)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        self._checkshape(self.outfile,self.imsize[0],self.imsize[1],1,1)
        self._checkdirax(self.outfile,self.phasecenter,self.cell,self.imsize)
    def test402(self):
        """test 402: Automatic resolution of phasecenter, cell, and imsize"""
        res=tsdimaging(infiles=self.rawfile,outfile=self.outfile,cell="",imsize=[],phasecenter="",nchan=1,start=0,width=1024,minweight=self.minweight0)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        self._checkshape(self.outfile,self.imsize_auto[0],self.imsize_auto[1],1,1)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.imsize_auto)
    def test403(self):
        """test 403: Resolve phasecenter"""
        res=tsdimaging(infiles=self.rawfile,outfile=self.outfile,cell=self.cell,imsize=self.imsize,phasecenter="",nchan=1,start=0,width=1024,minweight=self.minweight0)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        self._checkshape(self.outfile,self.imsize[0],self.imsize[1],1,1)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell, self.imsize)

    def test404(self):
        """test 404: Resolve cell"""
        res=tsdimaging(infiles=self.rawfile,outfile=self.outfile,cell="",imsize=self.imsize,phasecenter=self.phasecenter,nchan=1,start=0,width=1024,minweight=self.minweight0)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        self._checkshape(self.outfile,self.imsize[0],self.imsize[1],1,1)
        self._checkdirax(self.outfile,self.phasecenter,self.cell_auto,self.imsize)

    def test405(self):
        """test 405: Resolve imsize"""
        ref_imsize = [38, 32]
        res=tsdimaging(infiles=self.rawfile,outfile=self.outfile,cell=self.cell,imsize=[],phasecenter=self.phasecenter,nchan=1,start=0,width=1024,minweight=self.minweight0)
        self.assertEqual(res,None,
                         msg='Any error occurred during imaging')
        self._checkshape(self.outfile,ref_imsize[0],ref_imsize[1],1,1)
        self._checkdirax(self.outfile,self.phasecenter,self.cell,ref_imsize)


###
# Test data selection
###
class sdimaging_test_selection(selection_syntax.SelectionSyntaxTest,
                               sdimaging_unittest_base,unittest.TestCase):
    """
    Test selection syntax. Selection parameters to test are:
    field, spw (with selection), scan, stokes, and antenna
    """

    prefix = sdimaging_unittest_base.taskname+'TestSel'
    outfile = prefix+sdimaging_unittest_base.postfix
    # input MS names
    miscsel_ms = "selection_misc.ms"
    spwsel_ms = "selection_spw.ms"
    unifreq_ms = "selection_spw_unifreq.ms"
    rawfiles = [miscsel_ms, spwsel_ms, unifreq_ms]
    # default task parameters
    mode_def = "channel"
    kernel = "BOX"
    # 
    # auto calculation result of imsize
    cell_auto = "6.7275953729549656arcsec"
    imsize_auto = [21, 21]
    phasecenter_auto = "J2000 00:00:00.0 00.00.00.00"
    blc_auto = [0, 0, 0, 0]
    trc_auto = [20, 20, 0, 0]
    blcf_auto = '00:00:04.485, -00.01.07.276, I, 3e+11Hz'
    trcf_auto = '23:59:55.515, +00.01.07.276, I, 3e+11Hz'
    # Reference Statistics
    # blcf and trcf => qa.formxxx(+-qa.mul(cell_auto, 10.), "hms"/"dms", prec=3)
    # --- for "selection_misc.ms"
    unif_flux = 25.
    stat_common = {'blc': blc_auto,'trc': trc_auto,
                   'blcf': blcf_auto, 'trcf': trcf_auto}
    region_all = {'blc': blc_auto, 'trc': trc_auto}
    region_bottom = {'blc': [0, 0, 0, 0], 'trc': [20, 11, 0, 0]}
    region_top = {'blc': [0, 9, 0, 0], 'trc': [20, 20, 0, 0]}
    region_left = {'blc': [0, 0, 0, 0], 'trc': [11, 20, 0, 0]}
    region_right = {'blc': [9, 0, 0, 0], 'trc': [20, 20, 0, 0]}
    region_topleft = {'blc': [0, 9, 0, 0], 'trc': [11, 20, 0, 0]}
    region_topright = {'blc': [9, 9, 0, 0], 'trc': [20, 20, 0, 0]}
    region_bottomleft = {'blc': [0, 0, 0, 0], 'trc': [11, 11, 0, 0]}
    region_bottomright = {'blc': [9, 0, 0, 0], 'trc': [20, 11, 0, 0]}
    # --- for "selection_spw_unifreq.ms" and "selection_spw.ms"
    # flux taken from ms.statistics((column='CORRECTED_DATA', complex_value='amp', spw=idx)['mean']
    spw_flux_unifreq = [3.0008814930915833, 5.0014331340789795, 6.001709461212158]
    spw_flux = [5.001473307609558, 5.982952607795596, 3.011193051868015]  #NOTE spw=1 and 2 has relatively large O(10^-4) dispersion in intensity.
    spw_imsize_auto = [12, 12]
    spw_nchan = 10
    spw_blc_auto = [0, 0, 0, 0]
    spw_trc_auto = [11, 11, 0, 9]
    # blcf and trcf => qa.formxxx(+-qa.mul(cell_auto, 6.), "hms"/"dms", prec=3)
    spw_stat_common = {'blc': spw_blc_auto,'trc': spw_trc_auto}
    spw_region_all = {'blc': [1,1,0,0], 'trc': [11,11,0,9]}
    # select channels 2 - 7
    spw_region_chan1 = {'blc': [1,1,0,2], 'trc': [11,11,0,7]}

    @property
    def task(self):
        return tsdimaging
    
    @property
    def spw_channel_selection(self):
        return True

    def setUp(self):
        for name in self.rawfiles:
            if os.path.exists(name):
                shutil.rmtree(name)
            shutil.copytree(self.datapath+name, name)
        if os.path.exists(self.outfile):
            shutil.rmtree(self.outfile)

        default(tsdimaging)
        os.system( 'rm -rf '+self.prefix+'*' )

    def tearDown(self):
        for name in self.rawfiles:
            if (os.path.exists(name)):
                shutil.rmtree(name)
        #os.system( 'rm -rf '+self.prefix+'*' )

    ####################
    # Additional tests
    ####################
    #N/A Stokes & antenna selection

    ####################
    # scan
    ####################
    def test_scan_id_default(self):
        """test scan selection (scan='')"""
        scan = ''
        region =  self.region_all
        infile = self.miscsel_ms
        self.res=self.run_task(infiles=infile,scan=scan,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        refstats = self._merge_dict(self.stat_common, self._construct_refstat_uniform(self.unif_flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.imsize_auto[0],self.imsize_auto[1],1,1)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)

    def test_scan_id_exact(self):
        """test scan selection (scan='16')"""
        scan = '16'
        region =  self.region_topright
        infile = self.miscsel_ms
        self.res=self.run_task(infiles=infile,scan=scan,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        refstats = self._merge_dict(self.stat_common, self._construct_refstat_uniform(self.unif_flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.imsize_auto[0],self.imsize_auto[1],1,1)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-5)

    def test_scan_id_lt(self):
        """test scan selection (scan='<16')"""
        scan = '<16'
        region =  self.region_left
        infile = self.miscsel_ms
        self.res=self.run_task(infiles=infile,scan=scan,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        refstats = self._merge_dict(self.stat_common, self._construct_refstat_uniform(self.unif_flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.imsize_auto[0],self.imsize_auto[1],1,1)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-5)

    def test_scan_id_gt(self):
        """test scan selection (scan='>16')"""
        scan = '>16'
        region =  self.region_bottomright
        infile = self.miscsel_ms
        self.res=self.run_task(infiles=infile,scan=scan,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        refstats = self._merge_dict(self.stat_common, self._construct_refstat_uniform(self.unif_flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.imsize_auto[0],self.imsize_auto[1],1,1)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-5)

    def test_scan_id_range(self):
        """test scan selection (scan='16~17')"""
        scan = '16~17'
        region =  self.region_right
        infile = self.miscsel_ms
        self.res=self.run_task(infiles=infile,scan=scan,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        refstats = self._merge_dict(self.stat_common, self._construct_refstat_uniform(self.unif_flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.imsize_auto[0],self.imsize_auto[1],1,1)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-5)

    def test_scan_id_list(self):
        """test scan selection (scan='16,17')"""
        scan = '16,17'
        region =  self.region_right
        infile = self.miscsel_ms
        self.res=self.run_task(infiles=infile,scan=scan,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        refstats = self._merge_dict(self.stat_common, self._construct_refstat_uniform(self.unif_flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.imsize_auto[0],self.imsize_auto[1],1,1)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-5)

    def test_scan_id_exprlist(self):
        """test scan selection (scan='16,>16')"""
        scan = '16,>16'
        region =  self.region_right
        infile = self.miscsel_ms
        self.res=self.run_task(infiles=infile,scan=scan,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        refstats = self._merge_dict(self.stat_common, self._construct_refstat_uniform(self.unif_flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.imsize_auto[0],self.imsize_auto[1],1,1)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-5)

    ####################
    # field
    ####################
    def test_field_value_default(self):
        """test field selection (field='')"""
        field = ''
        region =  self.region_all
        infile = self.miscsel_ms
        self.res=self.run_task(infiles=infile,field=field,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        refstats = self._merge_dict(self.stat_common, self._construct_refstat_uniform(self.unif_flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.imsize_auto[0],self.imsize_auto[1],1,1)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)

    def test_field_id_exact(self):
        """test field selection (field='6')"""
        field = '6'
        region =  self.region_bottomleft
        infile = self.miscsel_ms
        self.res=self.run_task(infiles=infile,field=field,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        refstats = self._merge_dict(self.stat_common, self._construct_refstat_uniform(self.unif_flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.imsize_auto[0],self.imsize_auto[1],1,1)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-5)

    def test_field_id_lt(self):
        """test field selection (field='<7')"""
        field = '<7'
        region =  self.region_bottom
        infile = self.miscsel_ms
        self.res=self.run_task(infiles=infile,field=field,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        refstats = self._merge_dict(self.stat_common, self._construct_refstat_uniform(self.unif_flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.imsize_auto[0],self.imsize_auto[1],1,1)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-5)

    def test_field_id_gt(self):
        """test field selection (field='>6')"""
        field = '>6'
        region =  self.region_top
        infile = self.miscsel_ms
        self.res=self.run_task(infiles=infile,field=field,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        refstats = self._merge_dict(self.stat_common, self._construct_refstat_uniform(self.unif_flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.imsize_auto[0],self.imsize_auto[1],1,1)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-5)

    def test_field_id_range(self):
        """test field selection (field='7~8')"""
        field = '7~8'
        region =  self.region_top
        infile = self.miscsel_ms
        self.res=self.run_task(infiles=infile,field=field,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        refstats = self._merge_dict(self.stat_common, self._construct_refstat_uniform(self.unif_flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.imsize_auto[0],self.imsize_auto[1],1,1)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-5)

    def test_field_id_list(self):
        """test field selection (field='5,7')"""
        field = '5,7'
        region =  self.region_right
        infile = self.miscsel_ms
        self.res=self.run_task(infiles=infile,field=field,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        refstats = self._merge_dict(self.stat_common, self._construct_refstat_uniform(self.unif_flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.imsize_auto[0],self.imsize_auto[1],1,1)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-5)

    def test_field_id_exprlist(self):
        """test field selection (field='7,>7')"""
        field = '7,>7'
        region =  self.region_top
        infile = self.miscsel_ms
        self.res=self.run_task(infiles=infile,field=field,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        refstats = self._merge_dict(self.stat_common, self._construct_refstat_uniform(self.unif_flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.imsize_auto[0],self.imsize_auto[1],1,1)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-5)

    def test_field_value_exact(self):
        """test field selection (field='bottom')"""
        field = 'bottom'
        region =  self.region_bottom
        infile = self.miscsel_ms
        self.res=self.run_task(infiles=infile,field=field,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        refstats = self._merge_dict(self.stat_common, self._construct_refstat_uniform(self.unif_flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.imsize_auto[0],self.imsize_auto[1],1,1)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-5)

    def test_field_value_pattern(self):
        """test field selection (field='top*')"""
        field = 'top*'
        region =  self.region_top
        infile = self.miscsel_ms
        self.res=self.run_task(infiles=infile,field=field,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        refstats = self._merge_dict(self.stat_common, self._construct_refstat_uniform(self.unif_flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.imsize_auto[0],self.imsize_auto[1],1,1)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-5)

    def test_field_value_list(self):
        """test field selection (field='topright,topleft')"""
        field = 'topright,topleft'
        region =  self.region_top
        infile = self.miscsel_ms
        self.res=self.run_task(infiles=infile,field=field,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        refstats = self._merge_dict(self.stat_common, self._construct_refstat_uniform(self.unif_flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.imsize_auto[0],self.imsize_auto[1],1,1)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-5)

    def test_field_mix_exprlist(self):
        """test field selection (field='topr*,>7')"""
        field = 'topr*,>7'
        region =  self.region_top
        infile = self.miscsel_ms
        self.res=self.run_task(infiles=infile,field=field,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        refstats = self._merge_dict(self.stat_common, self._construct_refstat_uniform(self.unif_flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.imsize_auto[0],self.imsize_auto[1],1,1)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-5)

    ####################
    # spw 
    ####################
    def test_spw_id_default(self):
        """test spw selection (spw='')"""
        spw = ''
        infile = self.unifreq_ms
        flux_list = self.__get_flux_value(infile)
        selspw = range(len(flux_list))
        region =  self.spw_region_all
        self.res=self.run_task(infiles=infile,spw=spw,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        flux = sum([flux_list[idx] for idx in selspw])/float(len(selspw))
        refstats = self._merge_dict(self.spw_stat_common, self._construct_refstat_uniform(flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.spw_imsize_auto[0],self.spw_imsize_auto[1],1,self.spw_nchan)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.spw_imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)

    def test_spw_id_exact(self):
        """test spw selection (spw='1')"""
        spw = '1'
        selspw = [1]
        region =  self.spw_region_all
        infile = self.unifreq_ms
        flux_list = self.__get_flux_value(infile)
        self.res=self.run_task(infiles=infile,spw=spw,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        flux = sum([flux_list[idx] for idx in selspw])/float(len(selspw))
        refstats = self._merge_dict(self.spw_stat_common, self._construct_refstat_uniform(flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.spw_imsize_auto[0],self.spw_imsize_auto[1],1,self.spw_nchan)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.spw_imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)

    def test_spw_id_lt(self):
        """test spw selection (spw='<2')"""
        spw = '<2'
        selspw = [0,1]
        region =  self.spw_region_all
        infile = self.unifreq_ms
        flux_list = self.__get_flux_value(infile)
        self.res=self.run_task(infiles=infile,spw=spw,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        flux = sum([flux_list[idx] for idx in selspw])/float(len(selspw))
        refstats = self._merge_dict(self.spw_stat_common, self._construct_refstat_uniform(flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.spw_imsize_auto[0],self.spw_imsize_auto[1],1,self.spw_nchan)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.spw_imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)

    def test_spw_id_gt(self):
        """test spw selection (spw='>0')"""
        spw = '>0'
        selspw = [1,2]
        region =  self.spw_region_all
        infile = self.unifreq_ms
        flux_list = self.__get_flux_value(infile)
        self.res=self.run_task(infiles=infile,spw=spw,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        flux = sum([flux_list[idx] for idx in selspw])/float(len(selspw))
        refstats = self._merge_dict(self.spw_stat_common, self._construct_refstat_uniform(flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.spw_imsize_auto[0],self.spw_imsize_auto[1],1,self.spw_nchan)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.spw_imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)

    def test_spw_id_range(self):
        """test spw selection (spw='1~2')"""
        spw = '1~2'
        selspw = [1,2]
        region =  self.spw_region_all
        infile = self.unifreq_ms
        flux_list = self.__get_flux_value(infile)
        self.res=self.run_task(infiles=infile,spw=spw,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        flux = sum([flux_list[idx] for idx in selspw])/float(len(selspw))
        refstats = self._merge_dict(self.spw_stat_common, self._construct_refstat_uniform(flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.spw_imsize_auto[0],self.spw_imsize_auto[1],1,self.spw_nchan)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.spw_imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)

    def test_spw_id_list(self):
        """test spw selection (spw='0,2')"""
        spw = '0,2'
        selspw = [0,2]
        region =  self.spw_region_all
        infile = self.unifreq_ms
        flux_list = self.__get_flux_value(infile)
        self.res=self.run_task(infiles=infile,spw=spw,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        flux = sum([flux_list[idx] for idx in selspw])/float(len(selspw))
        refstats = self._merge_dict(self.spw_stat_common, self._construct_refstat_uniform(flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.spw_imsize_auto[0],self.spw_imsize_auto[1],1,self.spw_nchan)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.spw_imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)

    def test_spw_id_exprlist(self):
        """test spw selection (spw='0,>1')"""
        spw = '0,>1'
        selspw = [0,2]
        region =  self.spw_region_all
        infile = self.unifreq_ms
        flux_list = self.__get_flux_value(infile)
        self.res=self.run_task(infiles=infile,spw=spw,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        flux = sum([flux_list[idx] for idx in selspw])/float(len(selspw))
        refstats = self._merge_dict(self.spw_stat_common, self._construct_refstat_uniform(flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.spw_imsize_auto[0],self.spw_imsize_auto[1],1,self.spw_nchan)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.spw_imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)

    def test_spw_id_pattern(self):
        """test spw selection (spw='*')"""
        spw = '*'
        region =  self.spw_region_all
        infile = self.unifreq_ms
        flux_list = self.__get_flux_value(infile)
        selspw = range(len(flux_list))
        self.res=self.run_task(infiles=infile,spw=spw,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        flux = sum([flux_list[idx] for idx in selspw])/float(len(selspw))
        refstats = self._merge_dict(self.spw_stat_common, self._construct_refstat_uniform(flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.spw_imsize_auto[0],self.spw_imsize_auto[1],1,self.spw_nchan)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.spw_imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)

    @unittest.expectedFailure
    def test_spw_value_frequency(self):
        """test spw selection (spw='299.4~299.6GHz')"""
        spw = '299.4~299.6GHz'
        selspw = [0]
        infile = self.spwsel_ms
        flux_list = self.__get_flux_value(infile)
        region =  self.spw_region_all
        self.res=self.run_task(infiles=infile,spw=spw,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        flux = sum([flux_list[idx] for idx in selspw])/float(len(selspw))
        refstats = self._merge_dict(self.spw_stat_common, self._construct_refstat_uniform(flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.spw_imsize_auto[0],self.spw_imsize_auto[1],1,self.spw_nchan)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.spw_imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)

    @unittest.expectedFailure
    def test_spw_value_velocity(self):
        """test spw selection (spw='-550~-450km/s') NOT SUPPORTED YET"""
        self._default_test()

    @unittest.expectedFailure
    def test_spw_mix_exprlist(self):
        """test spw selection (spw='300~300.01GHz,0')"""
        spw = '300~300.01GHz,0'
        selspw = [0,1]
        region =  self.spw_region_all
        infile = self.unifreq_ms
        flux_list = self.spw_flux_unifreq
        self.res=self.run_task(infiles=infile,spw=spw,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        flux = sum([flux_list[idx] for idx in selspw])/float(len(selspw))
        refstats = self._merge_dict(self.spw_stat_common, self._construct_refstat_uniform(flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.spw_imsize_auto[0],self.spw_imsize_auto[1],1,self.spw_nchan)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.spw_imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)

    #########################
    # spw with channel range
    #########################
    @unittest.expectedFailure
    def test_spw_id_default_channel(self):
        """test spw selection w/ channel selection (spw=':2~7')"""
        spw = ':2~7'   #chan=2-7 in all spws should be selected
        region =  self.spw_region_chan1
        infile = self.unifreq_ms
        flux_list = self.__get_flux_value(infile)
        selspw = range(len(flux_list))
        self.res=self.run_task(infiles=infile,spw=spw,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        flux = sum([flux_list[idx] for idx in selspw])/float(len(selspw))
        refstats = self._merge_dict(self.spw_stat_common, self._construct_refstat_uniform(flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.spw_imsize_auto[0],self.spw_imsize_auto[1],1,self.spw_nchan)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.spw_imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-5)
    
    @unittest.expectedFailure
    def test_spw_id_default_frequency(self):
        """test spw selection w/ channel selection (spw=':300.4749~300.5251GHz')"""
        spw = ':300.4749~300.5251GHz'   #chan=2-7 in spw=1 should be selected
        selspw = [1]
        region =  self.spw_region_chan1
        infile = self.spwsel_ms
        flux_list = self.__get_flux_value(infile)
        self.res=self.run_task(infiles=infile,spw=spw,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        flux = sum([flux_list[idx] for idx in selspw])/float(len(selspw))
        refstats = self._merge_dict(self.spw_stat_common, self._construct_refstat_uniform(flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.spw_imsize_auto[0],self.spw_imsize_auto[1],1,self.spw_nchan)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.spw_imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-3,rtol=1.e-3)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-3,rtol=1.e-3)
       
    @unittest.expectedFailure
    def test_spw_id_default_velocity(self):
        """test spw selection w/ channel selection (spw='X~Ykm/s') NOT SUPPORTED YET"""
        self._default_test()
        
    @unittest.expectedFailure
    def test_spw_id_default_list(self):
        """test spw selection w/ channel selection (spw=':6~7;2~5')"""
        spw = ':6~7;2~5'   #chan=2-7 in all spws should be selected
        region =  self.spw_region_chan1
        infile = self.unifreq_ms
        flux_list = self.__get_flux_value(infile)
        selspw = range(len(flux_list))
        self.res=self.run_task(infiles=infile,spw=spw,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        flux = sum([flux_list[idx] for idx in selspw])/float(len(selspw))
        refstats = self._merge_dict(self.spw_stat_common, self._construct_refstat_uniform(flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.spw_imsize_auto[0],self.spw_imsize_auto[1],1,self.spw_nchan)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.spw_imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-5)
        
    def test_spw_id_exact_channel(self):
        """test spw selection w/ channel selection (spw='2:2~7')"""
        spw = '2:2~7'   #chan=2-7 of spw=2 should be selected
        selspw = [2]
        region =  self.spw_region_chan1
        infile = self.spwsel_ms
        flux_list = self.__get_flux_value(infile)
        self.res=self.run_task(infiles=infile,spw=spw,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        flux = sum([flux_list[idx] for idx in selspw])/float(len(selspw))
        refstats = self._merge_dict(self.spw_stat_common, self._construct_refstat_uniform(flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.spw_imsize_auto[0],self.spw_imsize_auto[1],1,self.spw_nchan)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.spw_imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-3,rtol=1.e-3)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-3,rtol=1.e-3)
        
    def test_spw_id_exact_frequency(self):
        """test spw selection w/ channel selection (spw='1:300.4749~300.5251GHz')"""
        spw = '1:300.4749~300.5251GHz'   #chan=2-7 of spw=1 should be selected
        selspw = [1]
        region =  self.spw_region_chan1
        infile = self.spwsel_ms
        flux_list = self.__get_flux_value(infile)
        self.res=self.run_task(infiles=infile,spw=spw,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        flux = sum([flux_list[idx] for idx in selspw])/float(len(selspw))
        refstats = self._merge_dict(self.spw_stat_common, self._construct_refstat_uniform(flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.spw_imsize_auto[0],self.spw_imsize_auto[1],1,self.spw_nchan)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.spw_imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-3,rtol=1.e-3)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-3,rtol=1.e-3)
        
    @unittest.expectedFailure
    def test_spw_id_exact_velocity(self):
        """test spw selection w/ channel selection (spw='0:X~Ykm/s') NOT SUPPORTED YET"""
        self._default_test()
        
    def test_spw_id_exact_list(self):
        """test spw selection w/ channel selection (spw='2:6~7;2~5')"""
        spw = '2:6~7;2~5'   #chan=2-7 of spw=2 should be selected
        selspw = [2]
        region =  self.spw_region_chan1
        infile = self.spwsel_ms
        flux_list = self.__get_flux_value(infile)
        self.res=self.run_task(infiles=infile,spw=spw,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        flux = sum([flux_list[idx] for idx in selspw])/float(len(selspw))
        refstats = self._merge_dict(self.spw_stat_common, self._construct_refstat_uniform(flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.spw_imsize_auto[0],self.spw_imsize_auto[1],1,self.spw_nchan)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.spw_imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-3,rtol=1.e-3)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-3,rtol=1.e-3)
        
    def test_spw_id_pattern_channel(self):
        """test spw selection w/ channel selection (spw='*:2~7')"""
        spw = '*:2~7'
        region =  self.spw_region_chan1
        infile = self.unifreq_ms
        flux_list = self.__get_flux_value(infile)
        selspw = range(len(flux_list))
        self.res=self.run_task(infiles=infile,spw=spw,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        flux = sum([flux_list[idx] for idx in selspw])/float(len(selspw))
        refstats = self._merge_dict(self.spw_stat_common, self._construct_refstat_uniform(flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.spw_imsize_auto[0],self.spw_imsize_auto[1],1,self.spw_nchan)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.spw_imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-5)

    def test_spw_id_pattern_frequency(self):
        """test spw selection w/ channel selection (spw='*:300.4749~300.5251GHz')"""
        #spw = '*:300.4749~300.5251GHz'   #chan=2-7 of spw=1 should be selected
        #selspw = [1]
        region =  self.spw_region_chan1
        #infile = self.spwsel_ms
        ##### TEMPORARY CHANGING INPUT DATA due to seg fault in sdimaging caused by a bug in ms.msseltoindex() #####
        infile = self.unifreq_ms
        spw = '*:299.9749~300.0251GHz'   #chan=2-7 of spw=1 should be selected
        flux_list = self.__get_flux_value(infile)
        selspw = range(len(flux_list))
        #
        self.res=self.run_task(infiles=infile,spw=spw,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        flux = sum([flux_list[idx] for idx in selspw])/float(len(selspw))
        refstats = self._merge_dict(self.spw_stat_common, self._construct_refstat_uniform(flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.spw_imsize_auto[0],self.spw_imsize_auto[1],1,self.spw_nchan)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.spw_imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-3,rtol=1.e-3)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-3,rtol=1.e-3)
        
    @unittest.expectedFailure
    def test_spw_id_pattern_velocity(self):
        """test spw selection w/ channel selection (spw='*:X~Ykm/s') NOT SUPPORTED YET"""
        self._default_test()
        
    def test_spw_id_pattern_list(self):
        """test spw selection w/ channel selection (spw='*:6~7;2~5')"""
        spw = '*:6~7;2~5'
        region =  self.spw_region_chan1
        infile = self.unifreq_ms
        flux_list = self.__get_flux_value(infile)
        selspw = range(len(flux_list))
        self.res=self.run_task(infiles=infile,spw=spw,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        flux = sum([flux_list[idx] for idx in selspw])/float(len(selspw))
        refstats = self._merge_dict(self.spw_stat_common, self._construct_refstat_uniform(flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.spw_imsize_auto[0],self.spw_imsize_auto[1],1,self.spw_nchan)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.spw_imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-5)
        
    @unittest.expectedFailure
    def test_spw_value_frequency_channel(self):
        """test spw selection w/ channel selection (spw='300.4~300.5GHz:2~7')"""
        spw = '300.4~300.5GHz:2~7'
        selspw = [1]
        region =  self.spw_region_chan1
        infile = self.spwsel_ms
        flux_list = self.__get_flux_value(infile)
        self.res=self.run_task(infiles=infile,spw=spw,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        flux = sum([flux_list[idx] for idx in selspw])/float(len(selspw))
        refstats = self._merge_dict(self.spw_stat_common, self._construct_refstat_uniform(flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.spw_imsize_auto[0],self.spw_imsize_auto[1],1,self.spw_nchan)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.spw_imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-3,rtol=1.e-3)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-3,rtol=1.e-3)
        
    @unittest.expectedFailure
    def test_spw_value_frequency_frequency(self):
        """test spw selection w/ channel selection (spw='300.4~300.5GHz:300.4749~300.5251GHz')"""
        spw = '300.4~300.5GHz:300.4749~300.5251GHz'   #chan=2-7 of spw=1 should be selected'
        selspw = [1]
        region =  self.spw_region_chan1
        infile = self.spwsel_ms
        flux_list = self.__get_flux_value(infile)
        self.res=self.run_task(infiles=infile,spw=spw,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        flux = sum([flux_list[idx] for idx in selspw])/float(len(selspw))
        refstats = self._merge_dict(self.spw_stat_common, self._construct_refstat_uniform(flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.spw_imsize_auto[0],self.spw_imsize_auto[1],1,self.spw_nchan)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.spw_imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-3,rtol=1.e-3)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-3,rtol=1.e-3)
        
    @unittest.expectedFailure
    def test_spw_value_frequency_velocity(self):
        """test spw selection w/ channel selection (spw='A~BHz:X~Ykm/s') NOT SUPPORTED YET"""
        self._default_test()
        
    @unittest.expectedFailure
    def test_spw_value_frequency_list(self):
        """test spw selection w/ channel selection (spw='300~300.1GHz:6~7;2~5')"""
        spw = '300~300.1GHz:6~7;2~5'
        selspw = [0]
        region =  self.spw_region_chan1
        infile = self.spwsel_ms
        flux_list = self.__get_flux_value(infile)
        self.res=self.run_task(infiles=infile,spw=spw,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        flux = sum([flux_list[idx] for idx in selspw])/float(len(selspw))
        refstats = self._merge_dict(self.spw_stat_common, self._construct_refstat_uniform(flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.spw_imsize_auto[0],self.spw_imsize_auto[1],1,self.spw_nchan)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.spw_imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-5)
        
    @unittest.expectedFailure
    def test_spw_value_velocity_channel(self):
        """test spw selection w/ channel selection (spw='X~Ykm/s:A~B') NOT SUPPORTED YET"""
        self._default_test()
        
    @unittest.expectedFailure
    def test_spw_value_velocity_frequency(self):
        """test spw selection w/ channel selection (spw='X~Ykm/s:A~BHz') NOT SUPPORTED YET"""
        self._default_test()
        
    @unittest.expectedFailure
    def test_spw_value_velocity_velocity(self):
        """test spw selection w/ channel selection (spw='X~Ykm/s:Z~Wkm/s') NOT SUPPORTED YET"""
        self._default_test()
        
    @unittest.expectedFailure
    def test_spw_value_velocity_list(self):
        """test spw selection w/ channel selection (spw='X~Ykm/s:A~B;C~D') NOT SUPPORTED YET"""
        self._default_test()
        
    def test_spw_id_list_channel(self):
        """test spw selection w/ channel selection (spw='1:2~7,2:2~7')"""
        spw = '0:2~7,2:2~7'
        selspw = [0, 2]
        region =  self.spw_region_chan1
        infile = self.unifreq_ms
        flux_list = self.__get_flux_value(infile)
        self.res=self.run_task(infiles=infile,spw=spw,mode=self.mode_def,gridfunction=self.kernel,outfile=self.outfile)
        # Tests
        flux = sum([flux_list[idx] for idx in selspw])/float(len(selspw))
        refstats = self._merge_dict(self.spw_stat_common, self._construct_refstat_uniform(flux, region['blc'], region['trc']) )
        self._checkshape(self.outfile,self.spw_imsize_auto[0],self.spw_imsize_auto[1],1,self.spw_nchan)
        self._checkdirax(self.outfile,self.phasecenter_auto,self.cell_auto,self.spw_imsize_auto)
        self._checkstats_new(self.outfile,refstats,atol=1.e-5)
        self._checkstats_box(self.outfile,refstats,box=region,atol=1.e-5)
        
    ####################
    # Helper functions
    ####################
    def _merge_dict(self, d1, d2):
        """
        Out of place merge of two dictionaries.
        If both dictionary has the same keys, value of the second
        dictionary is adopted.
        """
        if type(d1) != dict or type(d2) != dict:
            raise ValueError, "Internal error. inputs should be dictionaries."
        d12 = d1.copy()
        d12.update(d2)
        return d12
    
    def _checkstats_new(self,name, ref, compstats=None, atol=1.e-8, rtol=1.e-5, region=None, ignoremask=False):
        """
        A test function to compare statistics of an image with reference
        values.
        Arguments:
            name  :  name of an image to test statistics
            ref   :  a record (dictionary) of the reference statistic values
            compstats : a list of names of statistis to compare. By default,
                        the list is taken from all keys in ref
            atol  : absolute tolerance (see help in numpy.allclose)
            rtol  : relative tolerance (see help in numpy.allclose)
            region : region of image to calculate statistics. a CASA region
                     record should be specified (see help of , e.g., rg.box).
                     default is whole image.
            ignoremask : when True, mask in image is ignored and statistics
                         are calculated from whole pixels in image. default
                         is False (take image mask into account).
        """
        self._checkfile(name)
        if compstats is None: compstats = ref.keys()
        if region is None: region = {}
        ia.open(name)
        if ignoremask:
            def_mask = ia.maskhandler('default')
            ia.calcmask('T')
        stats=ia.statistics(region=region, list=True, verbose=True)
        if ignoremask:
            ia.maskhandler('set',def_mask)
        ia.close()
        #for key in stats.keys():
        for key in compstats:
            message='statistics \'%s\' does not match: %s (expected: %s)' % ( key, str(stats[key]), str(ref[key]) )
            if type(stats[key])==str:
                self.assertEqual(stats[key],ref[key],
                                 msg=message)
            else:
                #print stats[key]-ref[key]
                ret=numpy.allclose(stats[key],ref[key], atol=atol, rtol=rtol)
                self.assertEqual(ret,True,
                                 msg=message)

    def _checkstats_box(self,name, ref, compstats=None, atol=1.e-8, rtol=1.e-5, box=None, ignoremask=False):
        """
        A test function to compare statistics of a box region of an image
        with reference values.
        Arguments:
            name  :  name of an image to test statistics
            ref   :  a record (dictionary) of the reference statistic values
            compstats : a list of names of statistis to compare. By default,
                        the list is taken from all keys in ref
            atol  : absolute tolerance (see help in numpy.allclose)
            rtol  : relative tolerance (see help in numpy.allclose)
            box   : a dictionary that specifies a box region of image to
                    calculate statistics. it should be a dictionary with keys,
                    'blc' and 'trc' in pixel unit.
            ignoremask : when True, mask in image is ignored and statistics
                         are calculated from whole pixels in image. default
                         is False (take image mask into account).
        """
        boxreg = rg.box(**box) if box is not None else None
        refstats = ref.copy()
        refstats.update(box)
        for stats in ['blcf', 'trcf']:
            if refstats.has_key(stats): refstats.pop(stats)
        self._checkstats_new(name,refstats,region=boxreg,
                             compstats=compstats,atol=atol,rtol=rtol,
                             ignoremask=ignoremask)

    def _construct_refstat_uniform(self, fluxval, blc_data, trc_data):
        """
        Return a dictionary of analytic reference statistics of uniform image
        Arguments:
            fluxval  : the uniform flux of the image
            blc_data : blc of un-masked pixel (e.g., [0,0,0,0] for whole image)
            trc_data : trc of un-masked pixel
        Returns:
            a dictionary of statistics, 'min', 'max', 'rms', 'sigma', 'mean',
            'npts', 'sum', and 'sumsq'
        """
        # the number of valid (unmasked) pixels
        ndim = len(blc_data)
        nvalid = 1
        for idim in range(ndim):
            nvalid *= abs(trc_data[idim]-blc_data[idim]+1)
        retstat = {'min': [fluxval], 'max': [fluxval], 'rms': [fluxval],
                   'sigma': [0.], 'mean': [fluxval], 'npts': [nvalid],
                   'sum': [fluxval*nvalid], 'sumsq': [fluxval**2*nvalid]}
        return retstat

    def __get_flux_value(self, infile):
        """ returns proper flux list """
        if infile == self.miscsel_ms:
            return self.unif_flux
        elif infile == self.spwsel_ms:
            return self.spw_flux
        elif infile == self.unifreq_ms:
            return self.spw_flux_unifreq
        else: raise Exception, "Internal error: invalid input file to get flux value."


def suite():
    return [sdimaging_test0,sdimaging_test1,
            sdimaging_test2,sdimaging_test3,
            sdimaging_test4,sdimaging_test_selection]
