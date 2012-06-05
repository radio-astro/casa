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
from sdgrid import sdgrid
import asap as sd

#
# Unit test of sdgrid task.
# 

###
# Base class for sdimaging unit test
###
class sdgrid_unittest_base(object):
    """
    """
    taskname='sdgrid'
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdgrid/'
    data=None
    tolerance=0.01
    outfile='sdgrid.asap.grid'

    def getdata(self):
        tb.open(self.outfile)
        self.data = tb.getcol('SPECTRA')
        tb.close()

    def check(self,ref,val):
        diff=abs((val-ref)/ref)
        #print 'diff=',diff
        self.assertTrue(diff < self.tolerance,
                        msg='grid result differ: ref %s, val %s'%(ref,val))

    def nonzero(self,ref,index):
        refpix=ref[1]
        resultpix=index[1]
        self.assertEqual(len(refpix),len(resultpix),
                         msg='There are nonzero pixels that should be zero')
        for i in xrange(len(refpix)):
            self.assertEqual(refpix[i],resultpix[i],
                             msg='There are nonzero pixels that should be zero')

    def generateNonzeroPix(self,npol,npix,width):
        index=[]
        start=(npix-1)/2-(width-1)
        end=(npix-1)/2+(width-1)
        #print 'start=',start,',end=',end
        for i in xrange(start,end+1):
            tweak=npol if (width>=4 and (i==start or i==end)) else 0
            ifrom=npol*npix*i+npol*start+tweak
            ito=ifrom+npol*2*(width-1)-2*tweak
            index+=range(ifrom,ito+npol)
        #print 'index=',index
        nonzeropix_ref=(numpy.zeros(len(index),int),numpy.array(index))
        return nonzeropix_ref

    def addrow(self,val):
        tb.open(self.datapath+'/'+self.rawfile)
        tb.copyrows(self.rawfile,0,-1,1)
        tb.close()
        tb.open(self.rawfile,nomodify=False)
        tb.putcell('SPECTRA',tb.nrows()-1,val)
        tb.flush()
        tb.close()

###
# Test on bad parameter settings
###
class sdgrid_failure_case(sdgrid_unittest_base,unittest.TestCase):
    """
    Test on bad parameter setting
    """
    # Input and output names
    prefix=sdgrid_unittest_base.taskname+'Test0'
    badid=99
    rawfile='testimage1chan.1point.asap'

    def setUp(self):
        if os.path.exists(self.rawfile):
            shutil.rmtree(self.rawfile)
        shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(sdgrid)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.outfile)):
            shutil.rmtree(self.outfile)

    def test000(self):
        """Test 000: Default parameters"""
        res=sdgrid()
        self.assertFalse(res)

    def test001(self):
        """Test001: Invalid IFNO"""
        res=sdgrid(infiles=self.rawfile,ifno=self.badid,npix=16,cell='20arcsec',outfile=self.outfile)
        self.assertFalse(res)

    def test002(self):
        """Test002: Invalid POLNO"""
        res=sdgrid(infiles=self.rawfile,pollist=self.badid,npix=16,cell='20arcsec',outfile=self.outfile)
        self.assertFalse(res)

    def test003(self):
        """Test003: Invalid gridfunction"""
        res=sdgrid(infiles=self.rawfile,gridfunction='NONE',npix=16,cell='20arcsec',outfile=self.outfile)
        self.assertFalse(res)

    def test004(self):
        """Test004: Invalid weight type"""
        res=sdgrid(infiles=self.rawfile,weight='NONE',npix=16,cell='20arcsec',outfile=self.outfile)
        self.assertFalse(res)

    def test005(self):
        """Test005: Check overwrite option"""
        shutil.copytree(self.rawfile,self.outfile)
        res=sdgrid(infiles=self.rawfile,npix=16,cell='20arcsec',outfile=self.outfile,overwrite=False)
        self.assertFalse(res)

    def test006(self):
        """Test006: Invalid npix"""
        res=sdgrid(infiles=self.rawfile,npix=-99,cell='20arcsec',outfile=self.outfile)
        self.assertFalse(res)


    def test007(self):
        """Test007: Invalid unit for cell"""
        res=sdgrid(infiles=self.rawfile,npix=16,cell='20none',outfile=self.outfile)
        self.assertFalse(res)


    def test008(self):
        """Test008: Invalid format for center coordinate"""
        res=sdgrid(infiles=self.rawfile,npix=16,cell='20arcsec',outfile=self.outfile,center='Invalid format')
        self.assertFalse(res)

        

###
# Test simple gridding
###
class sdgrid_single_integ(sdgrid_unittest_base,unittest.TestCase):
    """
    Test simple gridding using data containing only one integration.
    """
    # Input and output names
    rawfile='testimage1chan.1point.asap'
    #prefix=sdgrid_unittest_base.taskname+'Test1'
    #outfile=prefix+'.asap'

    def setUp(self):
        if os.path.exists(self.rawfile):
            shutil.rmtree(self.rawfile)
        shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(sdgrid)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.outfile)):
            shutil.rmtree(self.outfile)

    def test100(self):
        """Test 100: Box kernel"""
        npix=17
        res=sdgrid(infiles=self.rawfile,gridfunction='BOX',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False)
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        self.getdata()

        # center is only nonzero pixel
        npol=2
        width=1
        nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        nonzeropix=self.data.nonzero()
        self.nonzero(nonzeropix_ref,nonzeropix)

        # pol0 must be 10.0
        pol0=self.data[nonzeropix[0][0],nonzeropix[1][0]]
        self.check(10.0,pol0)

        # pol1 must be 1.0
        pol1=self.data[nonzeropix[0][1],nonzeropix[1][1]]
        self.check(1.0,pol1)
        

    def test101(self):
        """Test101: SF kernel"""
        npix=17
        res=sdgrid(infiles=self.rawfile,gridfunction='SF',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False)
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        self.getdata()

        # default width for SF is 3
        width=3
        npol=2
        nonzeropix=self.data.nonzero()
        nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        self.nonzero(nonzeropix_ref,nonzeropix)

        # pol0 must be 10.0 while pol1 must be 1.0
        for i in xrange(0,len(nonzeropix[1]),npol):
            pol0=self.data[nonzeropix[0][i],nonzeropix[1][i]]
            self.check(10.0,pol0)
            pol1=self.data[nonzeropix[0][i+1],nonzeropix[1][i+1]]
            self.check(1.0,pol1)
        

    def test102(self):
        """Test102: Gaussian kernel"""
        npix=17
        res=sdgrid(infiles=self.rawfile,gridfunction='GAUSS',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False)
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        self.getdata()

        # default width for GAUSS is 4
        width=4
        npol=2
        nonzeropix=self.data.nonzero()
        nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        self.nonzero(nonzeropix_ref,nonzeropix)
        
        # pol0 must be 10.0 while pol1 must be 1.0
        for i in xrange(0,len(nonzeropix[1]),npol):
            pol0=self.data[nonzeropix[0][i],nonzeropix[1][i]]
            self.check(10.0,pol0)
            pol1=self.data[nonzeropix[0][i+1],nonzeropix[1][i+1]]
            self.check(1.0,pol1)

###
# Test clipminmax
###
class sdgrid_clipping(sdgrid_unittest_base,unittest.TestCase):
    """
    Test clipminmax
    """
    rawfile='testimage1chan.1point.asap'
    
    def setUp(self):
        if os.path.exists(self.rawfile):
            shutil.rmtree(self.rawfile)
        shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(sdgrid)

        # modification of input file along with this test
        #   - add row to enable clipping
        #   - all polno set to 0
        self.addrow([0.1])
        tb.open(self.rawfile,nomodify=False)
        polno=tb.getcol('POLNO')
        polno[:]=0
        tb.putcol('POLNO',polno)
        tb.close()

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.outfile)):
            shutil.rmtree(self.outfile)

    def test200(self):
        """Test 200: test clipping"""
        npix=17
        res=sdgrid(infiles=self.rawfile,gridfunction='BOX',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False,clipminmax=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        self.getdata()
        
        # center is only nonzero pixel
        npol=1
        width=1
        nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        nonzeropix=self.data.nonzero()
        #print nonzeropix
        self.nonzero(nonzeropix_ref,nonzeropix)

        # pol0 must be 1.0
        pol0=self.data[nonzeropix[0][0],nonzeropix[1][0]]
        self.check(1.0,pol0)
        

###
# Test for flag
###
class sdgrid_flagging(sdgrid_unittest_base,unittest.TestCase):
    """
    Test for flag
    """
    rawfile='testimage1chan.1point.asap'
    
    def setUp(self):
        if os.path.exists(self.rawfile):
            shutil.rmtree(self.rawfile)
        shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(sdgrid)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.outfile)):
            shutil.rmtree(self.outfile)

    def test300(self):
        """Test 300: Test channel flagging"""
        # channel flag pol0 data
        tb.open(self.rawfile,nomodify=False)
        fl=tb.getcell('FLAGTRA',0)
        fl[:]=1
        tb.putcell('FLAGTRA',0,fl)
        tb.close()

        # exec task
        npix=17
        res=sdgrid(infiles=self.rawfile,gridfunction='BOX',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False)
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')

        # test result
        self._testresult(npix)

    def test301(self):
        """Test 301: Test row flagging"""
        # row flag pol0 data
        tb.open(self.rawfile,nomodify=False)
        fl=tb.getcell('FLAGROW',0)
        fl=1
        tb.putcell('FLAGROW',0,fl)
        tb.close()

        # exec task
        npix=17
        res=sdgrid(infiles=self.rawfile,gridfunction='BOX',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False)
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')

        # test result
        self._testresult(npix)

    def _testresult(self,npix):
        self.getdata()
        
        # center is only nonzero pixel
        npol=2
        width=1
        nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        # pol0 is flagged so that data must be zero
        nonzeropix_ref2=(numpy.array([nonzeropix_ref[0][1]]),
                         numpy.array([nonzeropix_ref[1][1]]))
        nonzeropix=self.data.nonzero()
        #print nonzeropix
        self.nonzero(nonzeropix_ref2,nonzeropix)

        # pol1 must be 1.0
        pol1=self.data[nonzeropix[0][0],nonzeropix[1][0]]
        self.check(1.0,pol1)
        

###
# Test various weighting
###
class sdgrid_weighting(sdgrid_unittest_base,unittest.TestCase):
    """
    Test various weighting: UNIFORM, TSYS, TINTSYS
    """
    rawfile='testimage1chan.1point.asap'
    
    def setUp(self):
        if os.path.exists(self.rawfile):
            shutil.rmtree(self.rawfile)
        shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(sdgrid)

        # modification of input file along with this test
        #   - all polno set to 0
        tb.open(self.rawfile,nomodify=False)
        polno=tb.getcol('POLNO')
        polno[:]=0
        tb.putcol('POLNO',polno)
        tb.close()

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.outfile)):
            shutil.rmtree(self.outfile)

    def test400(self):
        """Test 400: test UNIFORM weighting"""
        npix=17
        res=sdgrid(infiles=self.rawfile,gridfunction='BOX',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False,weight='UNIFORM')
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        self.getdata()

        # center is only nonzero pixel
        npol=1
        width=1
        nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        nonzeropix=self.data.nonzero()
        #print nonzeropix
        self.nonzero(nonzeropix_ref,nonzeropix)

        # pol0 must be 5.5 (={10.0+1.0}/{1.0+1.0}
        pol0=self.data[nonzeropix[0][0],nonzeropix[1][0]]
        self.check(5.5,pol0)
        
    def test401(self):
        """Test 401: test TINT weighting"""
        # modify INTERVAL
        tb.open(self.rawfile,nomodify=False)
        integ=tb.getcol('INTERVAL')
        integ[0]=0.5
        integ[1]=1.0
        tb.putcol('INTERVAL',integ)
        tb.close()

        # exec task
        npix=17
        res=sdgrid(infiles=self.rawfile,gridfunction='BOX',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False,weight='TINT')
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        self.getdata()

        # center is only nonzero pixel
        npol=1
        width=1
        nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        nonzeropix=self.data.nonzero()
        #print nonzeropix
        self.nonzero(nonzeropix_ref,nonzeropix)

        # pol0 must be 4.0 (={10.0*0.5+1.0*1.0}/{0.5+1.0})
        pol0=self.data[nonzeropix[0][0],nonzeropix[1][0]]
        self.check(4.0,pol0)
        

    def test402(self):
        """Test402: test TSYS weighting"""
        # modify TSYS
        tb.open(self.rawfile,nomodify=False)
        tsys=tb.getcol('TSYS')
        tsys[:,0]=numpy.sqrt(2.0)
        tsys[:,1]=1.0
        tb.putcol('TSYS',tsys)
        tb.close()

        # exec task
        npix=17
        res=sdgrid(infiles=self.rawfile,gridfunction='BOX',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False,weight='TSYS')
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        self.getdata()

        # center is only nonzero pixel
        npol=1
        width=1
        nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        nonzeropix=self.data.nonzero()
        #print nonzeropix
        self.nonzero(nonzeropix_ref,nonzeropix)

        # pol0 must be 4.0 (={10.0*0.5+1.0*1.0}/{0.5+1.0})
        pol0=self.data[nonzeropix[0][0],nonzeropix[1][0]]
        self.check(4.0,pol0)
        

    def test403(self):
        """Test403: test TINTSYS weighting"""
        # modify TSYS and INTERVAL
        tb.open(self.rawfile,nomodify=False)
        tsys=tb.getcol('TSYS')
        tsys[:,0]=numpy.sqrt(2.0)
        tsys[:,1]=1.0
        tb.putcol('TSYS',tsys)
        integ=tb.getcol('INTERVAL')
        integ[0]=0.5
        integ[1]=1.0
        tb.putcol('INTERVAL',integ)
        tb.close()

        # exec task
        npix=17
        res=sdgrid(infiles=self.rawfile,gridfunction='BOX',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False,weight='TINTSYS')
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        self.getdata()

        # center is only nonzero pixel
        npol=1
        width=1
        nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        nonzeropix=self.data.nonzero()
        #print nonzeropix
        self.nonzero(nonzeropix_ref,nonzeropix)

        # pol0 must be 2.8 (={10.0*0.5*0.5+1.0*1.0*1.0}/{0.5*0.5+1.0*1.0})
        pol0=self.data[nonzeropix[0][0],nonzeropix[1][0]]
        self.check(2.8,pol0)
        
###
# Test grid map data
###
class sdgrid_map(sdgrid_unittest_base,unittest.TestCase):
    """
    Test grid map data
    """
    rawfile='testimage1chan.map.asap'

    def setUp(self):
        if os.path.exists(self.rawfile):
            shutil.rmtree(self.rawfile)
        shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(sdgrid)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.outfile)):
            shutil.rmtree(self.outfile)

    def test500(self):
        """Test BOX gridding for map data"""
        npix=17
        res=sdgrid(infiles=self.rawfile,gridfunction='BOX',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False)
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        self.getdata()

        # center is only nonzero pixel
        npol=2
        width=1
        nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        nonzeropix=self.data.nonzero()
        self.nonzero(nonzeropix_ref,nonzeropix)

        # pol0 must be 0.625
        pol0=self.data[nonzeropix[0][0],nonzeropix[1][0]]
        self.check(0.625,pol0)
        
        # pol1 must be 0.0625
        pol1=self.data[nonzeropix[0][1],nonzeropix[1][1]]
        self.check(0.0625,pol1)

    def test501(self):
        """Test SF gridding for map data"""
        npix=17
        res=sdgrid(infiles=self.rawfile,gridfunction='SF',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False)
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        self.getdata()

        # default width for SF is 3
        width=3
        npol=2
        nonzeropix=self.data.nonzero()
        nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        self.nonzero(nonzeropix_ref,nonzeropix)

        # check nonzero values
        refdata=[  1.54954410e-04,   1.54954414e-05,   4.63147834e-03,
                   4.63147851e-04,   9.89488605e-03,   9.89488559e-04,
                   4.63147834e-03,   4.63147851e-04,   1.54954410e-04,
                   1.54954414e-05,   4.63147834e-03,   4.63147851e-04,
                   3.81659232e-02,   3.81659227e-03,   6.86512142e-02,
                   6.86512096e-03,   3.81659232e-02,   3.81659227e-03,
                   4.63147834e-03,   4.63147851e-04,   9.89488605e-03,
                   9.89488559e-04,   6.86512142e-02,   6.86512096e-03,
                   1.19758800e-01,   1.19758807e-02,   6.86512142e-02,
                   6.86512096e-03,   9.89488605e-03,   9.89488559e-04,
                   4.63147834e-03,   4.63147851e-04,   3.81659232e-02,
                   3.81659227e-03,   6.86512142e-02,   6.86512096e-03,
                   3.81659232e-02,   3.81659227e-03,   4.63147834e-03,
                   4.63147851e-04,   1.54954410e-04,   1.54954414e-05,
                   4.63147834e-03,   4.63147851e-04,   9.89488605e-03,
                   9.89488559e-04,   4.63147834e-03,   4.63147851e-04,
                   1.54954410e-04,   1.54954414e-05]
        nonzerodata=numpy.take(self.data,nonzeropix[1],axis=1).squeeze()
        for i in xrange(len(nonzerodata)):
            self.check(refdata[i],nonzerodata[i])

    def test502(self):
        """Test GAUSS gridding for map data"""
        npix=17
        res=sdgrid(infiles=self.rawfile,gridfunction='GAUSS',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False)
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        self.getdata()
        
        # default width for GAUSS is 4
        width=4
        npol=2
        nonzeropix=self.data.nonzero()
        nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        self.nonzero(nonzeropix_ref,nonzeropix)

        refdata=[  1.77304173e-05,   1.77304173e-06,   1.39215190e-04,
                   1.39215190e-05,   2.75663275e-04,   2.75663278e-05,
                   1.39218566e-04,   1.39218564e-05,   1.77580732e-05,
                   1.77580728e-06,   1.77304137e-05,   1.77304128e-06,
                   5.55176579e-04,   5.55176593e-05,   4.37455671e-03,
                   4.37455688e-04,   8.58624838e-03,   8.58624873e-04,
                   4.37466195e-03,   4.37466166e-04,   5.56042010e-04,
                   5.56042032e-05,   1.81926825e-05,   1.81926828e-06,
                   1.39215146e-04,   1.39215144e-05,   4.37455624e-03,
                   4.37455630e-04,   3.45965475e-02,   3.45965475e-03,
                   6.86242804e-02,   6.86242757e-03,   3.45973745e-02,
                   3.45973740e-03,   4.38137632e-03,   4.38137620e-04,
                   1.42844816e-04,   1.42844810e-05,   2.75663246e-04,
                   2.75663260e-05,   8.58624838e-03,   8.58624873e-04,
                   6.86242804e-02,   6.86242757e-03,   1.37247697e-01,
                   1.37247695e-02,   6.86259195e-02,   6.86259149e-03,
                   8.59963335e-03,   8.59963300e-04,   2.82850349e-04,
                   2.82850360e-05,   1.39218551e-04,   1.39218546e-05,
                   4.37466241e-03,   4.37466224e-04,   3.45973782e-02,
                   3.45973787e-03,   6.86259270e-02,   6.86259242e-03,
                   3.45982052e-02,   3.45982029e-03,   4.38148156e-03,
                   4.38148156e-04,   1.42848279e-04,   1.42848285e-05,
                   1.77580678e-05,   1.77580671e-06,   5.56042127e-04,
                   5.56042141e-05,   4.38137632e-03,   4.38137620e-04,
                   8.59963335e-03,   8.59963300e-04,   4.38148109e-03,
                   4.38148127e-04,   5.56909014e-04,   5.56909035e-05,
                   1.82210515e-05,   1.82210522e-06,   1.81926716e-05,
                   1.81926725e-06,   1.42844699e-04,   1.42844701e-05,
                   2.82850146e-04,   2.82850142e-05,   1.42848148e-04,
                   1.42848157e-05,   1.82210370e-05,   1.82210374e-06]
        nonzerodata=numpy.take(self.data,nonzeropix[1],axis=1).squeeze()
        for i in xrange(len(nonzerodata)):
            self.check(refdata[i],nonzerodata[i])

###
# Test DEC correction
###
class sdgrid_dec_correction(sdgrid_unittest_base,unittest.TestCase):
    """
    Test DEC correction factor for horizontal (R.A.) auto grid setting.
    """
    rawfile='testimage1chan.1point.asap'
    
    def setUp(self):
        if os.path.exists(self.rawfile):
            shutil.rmtree(self.rawfile)
        shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(sdgrid)

        # modification of input file along with this test
        #   - declination set to 60deg
        tb.open(self.rawfile,nomodify=False)
        dir=tb.getcol('DIRECTION')
        dir[1,:]=60.0*numpy.pi/180.0
        tb.putcol('DIRECTION',dir)
        tb.close()

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.outfile)):
            shutil.rmtree(self.outfile)

    def test600(self):
        """Test 600: Test DEC correction factor"""
        npix=17
        res=sdgrid(infiles=self.rawfile,gridfunction='BOX',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False)
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')

        # check horizontal and vertical grid
        tb.open(self.outfile)
        dir0=tb.getcell('DIRECTION',0)
        dir1=tb.getcell('DIRECTION',2)
        dir2=tb.getcell('DIRECTION',npix*2)
        dx=dir1[0]-dir0[0]
        dy=dir2[1]-dir0[1]
        #print 'dx=',dx,',dy=',dy
        diff=abs((0.5*dx-dy)/dy)
        self.assertTrue(diff<0.01,
                        msg='DEC correction is not correct.')


###
# Test to change center for gridding
###
class sdgrid_grid_center(sdgrid_unittest_base,unittest.TestCase):
    """
    Test to change center for gridding
    """
    rawfile='testimage1chan.1point.asap'
    
    def setUp(self):
        if os.path.exists(self.rawfile):
            shutil.rmtree(self.rawfile)
        shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(sdgrid)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.outfile)):
            shutil.rmtree(self.outfile)

    def test700(self):
        """Test 700: Test to change center for gridding"""
        tb.open(self.rawfile)
        dir=tb.getcell('DIRECTION',0)
        tb.close()

        #shift center 3 pixels upward
        nshift=3
        pix=20.0
        dir[1]+=nshift*pix/3600.0*numpy.pi/180.0

        npix=17
        res=sdgrid(infiles=self.rawfile,gridfunction='BOX',npix=npix,cell='%sarcsec'%(pix),outfile=self.outfile,plot=False,center=dir)
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')

        self.getdata()
        
        # center is only nonzero pixel
        npol=2
        width=1
        nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        #print nonzeropix_ref
        # shift 3 pixels downward
        nonzeropix_ref[1][0]-=nshift*npol*npix
        nonzeropix_ref[1][1]-=nshift*npol*npix
        nonzeropix=self.data.nonzero()
        #print nonzeropix_ref
        #print nonzeropix
        self.nonzero(nonzeropix_ref,nonzeropix)


def suite():
    return [sdgrid_failure_case, sdgrid_single_integ,
            sdgrid_clipping, sdgrid_flagging,
            sdgrid_weighting, sdgrid_map,
            sdgrid_dec_correction, sdgrid_grid_center]
