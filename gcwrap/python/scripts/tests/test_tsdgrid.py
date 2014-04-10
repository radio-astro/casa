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
from tsdgrid import tsdgrid
import asap as sd

#
# Unit test of tsdgrid task.
# 

###
# Base class for sdimaging unit test
###
class tsdgrid_unittest_base(object):
    """
    """
    taskname='tsdgrid'
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdgrid/'
    data=None
    tolerance=0.01
    outfile='sdgrid.asap.grid'

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
        #refpix=ref[1]
        #resultpix=index[1]
        refpix = ref
        resultpix = index
        msglt = 'There are nonzero pixels that should be zero'
        msggt = 'There are zero pixels that should be nonzero'
        self.assertEqual(len(refpix),len(resultpix),
                         msg=(msglt if len(refpix) < len(resultpix) else msggt))
        for i in xrange(len(refpix)):
            self.assertEqual(refpix[i],resultpix[i],
                             msg='Index doesn\'t match: ref %s, result %s'%(refpix[i],resultpix[i]))

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
        #nonzeropix_ref=(numpy.zeros(len(index),int),numpy.array(index))
        nonzeropix_ref=numpy.array(index)
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
class tsdgrid_failure_case(tsdgrid_unittest_base,unittest.TestCase):
    """
    Test on bad parameter setting
    """
    # Input and output names
    #prefix=sdgrid_unittest_base.taskname+'Test0'
    prefix='sdgridTest0'
    badid='99'
    rawfile='testimage1chan.1point.asap'

    def setUp(self):
        if os.path.exists(self.rawfile):
            shutil.rmtree(self.rawfile)
        shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(tsdgrid)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.outfile)):
            shutil.rmtree(self.outfile)

    def test000(self):
        """Test 000: Default parameters"""
        # argument verification error
        res=tsdgrid()
        self.assertFalse(res)

    def test001(self):
        """Test001: Invalid SPW"""
        try:
            res=tsdgrid(infiles=self.rawfile,spw=self.badid,npix=16,cell='20arcsec',outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            #pos=str(e).find('No corresponding rows for given selection: SPW %s'%(self.badid))
            pos=str(e).find('No valid spw')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test002(self):
        """Test002: Invalid POLNO"""
        try:
            res=tsdgrid(infiles=self.rawfile,pol=self.badid,npix=16,cell='20arcsec',outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Empty pol')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test003(self):
        """Test003: Invalid gridfunction"""
        # argument verification error
        res=tsdgrid(infiles=self.rawfile,gridfunction='NONE',npix=16,cell='20arcsec',outfile=self.outfile)
        self.assertFalse(res)

    def test004(self):
        """Test004: Invalid weight type"""
        # argument verification error
        res=tsdgrid(infiles=self.rawfile,weight='NONE',npix=16,cell='20arcsec',outfile=self.outfile)
        self.assertFalse(res)

    def test005(self):
        """Test005: Check overwrite option"""
        shutil.copytree(self.rawfile,self.outfile)
        try:
            res=tsdgrid(infiles=self.rawfile,npix=16,cell='20arcsec',outfile=self.outfile,overwrite=False)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Output file \'%s\' exists.'%(self.outfile))
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

# Those two tests are meaningless
#    def test006(self):
#        """Test006: Invalid npix"""
#        res=tsdgrid(infiles=self.rawfile,npix=-99,cell='',outfile=self.outfile)
#        self.assertFalse(res)
#
#    def test007(self):
#        """Test007: Invalid unit for cell"""
#        res=tsdgrid(infiles=self.rawfile,npix=16,cell='20none',outfile=self.outfile)
#        self.assertFalse(res)

    def test008(self):
        """Test008: Invalid format for center coordinate"""
        try:
            res=tsdgrid(infiles=self.rawfile,npix=16,cell='20arcsec',outfile=self.outfile,center='Invalid format')
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Empty QuantumHolder argument for asQuantumDouble')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

        

###
# Test simple gridding
###
class tsdgrid_single_integ(tsdgrid_unittest_base,unittest.TestCase):
    """
    Test simple gridding using data containing only one integration.
    """
    # Input and output names
    rawfile='testimage1chan.1point.asap'
    #prefix=tsdgrid_unittest_base.taskname+'Test1'
    #outfile=prefix+'.asap'

    def setUp(self):
        if os.path.exists(self.rawfile):
            shutil.rmtree(self.rawfile)
        shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(tsdgrid)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.outfile)):
            shutil.rmtree(self.outfile)

    def test100(self):
        """Test 100: Box kernel"""
        npix=17
        res=tsdgrid(infiles=self.rawfile,gridfunction='BOX',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False)
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        self.getdata()

        # center is only nonzero pixel
        npol=2
        width=1
        nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        nonzeropix=self.data.nonzero()[1]
        self.nonzero(nonzeropix_ref,nonzeropix)

        # pol0 must be 10.0
        pol0=self.data[0,nonzeropix[0]]
        self.check(10.0,pol0)

        # pol1 must be 1.0
        pol1=self.data[0,nonzeropix[1]]
        self.check(1.0,pol1)
        

    def test101(self):
        """Test101: SF kernel"""
        npix=17
        res=tsdgrid(infiles=self.rawfile,gridfunction='SF',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False)
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        self.getdata()

        # default width for SF is 3
        width=3
        npol=2
        nonzeropix=self.data.nonzero()[1]
        nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        self.nonzero(nonzeropix_ref,nonzeropix)

        # pol0 must be 10.0 while pol1 must be 1.0
        for i in xrange(0,len(nonzeropix),npol):
            pol0=self.data[0,nonzeropix[i]]
            self.check(10.0,pol0)
            pol1=self.data[0,nonzeropix[i+1]]
            self.check(1.0,pol1)
        

    def test102(self):
        """Test102: Gaussian kernel"""
        npix=17
        res=tsdgrid(infiles=self.rawfile,gridfunction='GAUSS',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False)
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        self.getdata()

        # default width for GAUSS is 4
        width=2
        npol=2
        nonzeropix=self.data.nonzero()[1]
        nonzeropix_ref=numpy.array([218, 219, 220, 221, 222, 223, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 354, 355, 356, 357, 358, 359])
        #nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        self.nonzero(nonzeropix_ref,nonzeropix)
        
        # pol0 must be 10.0 while pol1 must be 1.0
        for i in xrange(0,len(nonzeropix),npol):
            pol0=self.data[0,nonzeropix[i]]
            self.check(10.0,pol0)
            pol1=self.data[0,nonzeropix[i+1]]
            self.check(1.0,pol1)

    def test103(self):
        """Test103: Gaussian*Jinc kernel"""
        npix=17
        res=tsdgrid(infiles=self.rawfile,gridfunction='GJINC',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False)
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        self.getdata()

        # default width for GAUSS is 4
        width=2
        npol=2
        nonzeropix=self.data.nonzero()[1]
        nonzeropix_ref=numpy.array([252, 253, 254, 255, 256, 257, 286, 287, 288, 289, 290, 291, 320, 321, 322, 323, 324, 325])
        #nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        self.nonzero(nonzeropix_ref,nonzeropix)
        
        # pol0 must be 10.0 while pol1 must be 1.0
        for i in xrange(0,len(nonzeropix),npol):
            pol0=self.data[0,nonzeropix[i]]
            self.check(10.0,pol0)
            pol1=self.data[0,nonzeropix[i+1]]
            self.check(1.0,pol1)
        

###
# Test clipminmax
###
class tsdgrid_clipping(tsdgrid_unittest_base,unittest.TestCase):
    """
    Test clipminmax
    """
    rawfile='testimage1chan.1point.asap'
    
    def setUp(self):
        if os.path.exists(self.rawfile):
            shutil.rmtree(self.rawfile)
        shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(tsdgrid)

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
        res=tsdgrid(infiles=self.rawfile,gridfunction='BOX',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False,clipminmax=True)
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        self.getdata()
        
        # center is only nonzero pixel
        npol=1
        width=1
        nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        nonzeropix=self.data.nonzero()[1]
        #print nonzeropix
        self.nonzero(nonzeropix_ref,nonzeropix)

        # pol0 must be 1.0
        pol0=self.data[0,nonzeropix[0]]
        self.check(1.0,pol0)
        

###
# Test for flag
###
class tsdgrid_flagging(tsdgrid_unittest_base,unittest.TestCase):
    """
    Test for flag
    """
    rawfile='testimage1chan.1point.asap'
    
    def setUp(self):
        if os.path.exists(self.rawfile):
            shutil.rmtree(self.rawfile)
        shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(tsdgrid)

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
        res=tsdgrid(infiles=self.rawfile,gridfunction='BOX',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False)
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
        res=tsdgrid(infiles=self.rawfile,gridfunction='BOX',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False)
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
        #nonzeropix_ref2=(numpy.array([nonzeropix_ref[0][1]]),
        #                 numpy.array([nonzeropix_ref[1][1]]))
        nonzeropix_ref2=numpy.array([nonzeropix_ref[1]])
        nonzeropix=self.data.nonzero()[1]
        #print nonzeropix
        self.nonzero(nonzeropix_ref2,nonzeropix)

        # pol1 must be 1.0
        pol1=self.data[0,nonzeropix[0]]
        self.check(1.0,pol1)
        

###
# Test various weighting
###
class tsdgrid_weighting(tsdgrid_unittest_base,unittest.TestCase):
    """
    Test various weighting: UNIFORM, TSYS, TINTSYS
    """
    rawfile='testimage1chan.1point.asap'
    
    def setUp(self):
        if os.path.exists(self.rawfile):
            shutil.rmtree(self.rawfile)
        shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(tsdgrid)

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
        res=tsdgrid(infiles=self.rawfile,gridfunction='BOX',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False,weight='UNIFORM')
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        self.getdata()

        # center is only nonzero pixel
        npol=1
        width=1
        nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        nonzeropix=self.data.nonzero()[1]
        #print nonzeropix
        self.nonzero(nonzeropix_ref,nonzeropix)

        # pol0 must be 5.5 (={10.0+1.0}/{1.0+1.0}
        pol0=self.data[0,nonzeropix[0]]
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
        res=tsdgrid(infiles=self.rawfile,gridfunction='BOX',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False,weight='TINT')
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        self.getdata()

        # center is only nonzero pixel
        npol=1
        width=1
        nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        nonzeropix=self.data.nonzero()[1]
        #print nonzeropix
        self.nonzero(nonzeropix_ref,nonzeropix)

        # pol0 must be 4.0 (={10.0*0.5+1.0*1.0}/{0.5+1.0})
        pol0=self.data[0,nonzeropix[0]]
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
        res=tsdgrid(infiles=self.rawfile,gridfunction='BOX',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False,weight='TSYS')
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        self.getdata()

        # center is only nonzero pixel
        npol=1
        width=1
        nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        nonzeropix=self.data.nonzero()[1]
        #print nonzeropix
        self.nonzero(nonzeropix_ref,nonzeropix)

        # pol0 must be 4.0 (={10.0*0.5+1.0*1.0}/{0.5+1.0})
        pol0=self.data[0,nonzeropix[0]]
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
        res=tsdgrid(infiles=self.rawfile,gridfunction='BOX',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False,weight='TINTSYS')
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        self.getdata()

        # center is only nonzero pixel
        npol=1
        width=1
        nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        nonzeropix=self.data.nonzero()[1]
        #print nonzeropix
        self.nonzero(nonzeropix_ref,nonzeropix)

        # pol0 must be 2.8 (={10.0*0.5*0.5+1.0*1.0*1.0}/{0.5*0.5+1.0*1.0})
        pol0=self.data[0,nonzeropix[0]]
        self.check(2.8,pol0)
        
###
# Test grid map data
###
class tsdgrid_map(tsdgrid_unittest_base,unittest.TestCase):
    """
    Test grid map data
    """
    rawfile='testimage1chan.map.asap'

    def setUp(self):
        if os.path.exists(self.rawfile):
            shutil.rmtree(self.rawfile)
        shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(tsdgrid)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.outfile)):
            shutil.rmtree(self.outfile)

    def test500(self):
        """Test BOX gridding for map data"""
        npix=17
        res=tsdgrid(infiles=self.rawfile,gridfunction='BOX',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False)
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        self.getdata()

        # center is only nonzero pixel
        npol=2
        width=1
        nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        nonzeropix=self.data.nonzero()[1]
        self.nonzero(nonzeropix_ref,nonzeropix)

        pol0=self.data[0,nonzeropix[0]]
        #self.check(0.625,pol0)
        #self.check(0.5,pol0)
        self.check(0.6666666667,pol0)
        
        pol1=self.data[0,nonzeropix[1]]
        #self.check(0.0625,pol1)
        #self.check(0.05,pol1)
        self.check(0.06666666667,pol1)

    def test501(self):
        """Test SF gridding for map data"""
        npix=17
        res=tsdgrid(infiles=self.rawfile,gridfunction='SF',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False)
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        self.getdata()

        # default width for SF is 3
        width=3
        npol=2
        nonzeropix=self.data.nonzero()[1]
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
        nonzerodata=numpy.take(self.data,nonzeropix,axis=1).squeeze()
        for i in xrange(len(nonzerodata)):
            self.check(refdata[i],nonzerodata[i])

    def test502(self):
        """Test GAUSS gridding for map data"""
        npix=17
        res=tsdgrid(infiles=self.rawfile,gridfunction='GAUSS',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False)
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        self.getdata()
        
        # default width for GAUSS is 4
        width=3
        npol=2
        nonzeropix=self.data.nonzero()[1]
        nonzeropix_ref = numpy.array([218, 219, 220, 221, 222, 223, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 354, 355, 356, 357, 358, 359])
        #nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        self.nonzero(nonzeropix_ref,nonzeropix)

        refdata = [1.37290766e-03,   1.37290757e-04,   3.63217224e-03,
         3.63217230e-04,   1.37290766e-03,   1.37290757e-04,
         1.37290766e-03,   1.37290757e-04,   2.71596070e-02,
         2.71596084e-03,   7.29541257e-02,   7.29541294e-03,
         2.71596070e-02,   2.71596084e-03,   1.37290766e-03,
         1.37290757e-04,   3.63217224e-03,   3.63217230e-04,
         7.29541257e-02,   7.29541294e-03,   1.98309869e-01,
         1.98309869e-02,   7.29541257e-02,   7.29541294e-03,
         3.63217224e-03,   3.63217230e-04,   1.37290766e-03,
         1.37290757e-04,   2.71596070e-02,   2.71596084e-03,
         7.29541257e-02,   7.29541294e-03,   2.71596070e-02,
         2.71596084e-03,   1.37290766e-03,   1.37290757e-04,
         1.37290766e-03,   1.37290757e-04,   3.63217224e-03,
         3.63217230e-04,   1.37290766e-03,   1.37290757e-04]
        nonzerodata=numpy.take(self.data,nonzeropix,axis=1).squeeze()
        for i in xrange(len(nonzerodata)):
            self.check(refdata[i],nonzerodata[i])

    def test503(self):
        """Test GJINC gridding for map data"""
        npix=17
        res=tsdgrid(infiles=self.rawfile,gridfunction='GJINC',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False)
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        self.getdata()
        
        # default width for GAUSS is 4
        width=3
        npol=2
        nonzeropix=self.data.nonzero()[1]
        nonzeropix_ref = numpy.array([252, 253, 254, 255, 256, 257, 286, 287, 288, 289, 290, 291, 320, 321, 322, 323, 324, 325])
        #nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        self.nonzero(nonzeropix_ref,nonzeropix)

        refdata = [0.0337296, 0.00337296, 0.0818698, 0.00818698, 0.0337296,
                   0.00337296, 0.0818698, 0.00818698, 0.16894495, 0.0168945,
                   0.0818698, 0.00818698, 0.0337296, 0.00337296, 0.0818698,
                   0.00818698, 0.0337296, 0.00337296]
        nonzerodata=numpy.take(self.data,nonzeropix,axis=1).squeeze()
        for i in xrange(len(nonzerodata)):
            self.check(refdata[i],nonzerodata[i])

###
# Test DEC correction
###
class tsdgrid_dec_correction(tsdgrid_unittest_base,unittest.TestCase):
    """
    Test DEC correction factor for horizontal (R.A.) auto grid setting.
    """
    rawfile='testimage1chan.1point.asap'
    
    def setUp(self):
        if os.path.exists(self.rawfile):
            shutil.rmtree(self.rawfile)
        shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(tsdgrid)

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
        res=tsdgrid(infiles=self.rawfile,gridfunction='BOX',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False)
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')

        # check horizontal and vertical grid
        tb.open(self.outfile)
        dir0=tb.getcell('DIRECTION',0)
        dir1=tb.getcell('DIRECTION',2)
        dir2=tb.getcell('DIRECTION',npix*2)
        tb.close()
        dx=dir0[0]-dir1[0]
        dy=dir2[1]-dir0[1]
        #print 'dx=',dx,',dy=',dy
        diff=abs((0.5*dx-dy)/dy)
        self.assertTrue(diff<0.01,
                        msg='DEC correction is not correct.')


###
# Test to change center for gridding
###
class tsdgrid_grid_center(tsdgrid_unittest_base,unittest.TestCase):
    """
    Test to change center for gridding
    """
    rawfile='testimage1chan.1point.asap'
    
    def setUp(self):
        if os.path.exists(self.rawfile):
            shutil.rmtree(self.rawfile)
        shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(tsdgrid)

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
        res=tsdgrid(infiles=self.rawfile,gridfunction='BOX',npix=npix,cell='%sarcsec'%(pix),outfile=self.outfile,plot=False,center=dir)
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')

        self.getdata()
        
        # center is only nonzero pixel
        npol=2
        width=1
        nonzeropix_ref=self.generateNonzeroPix(npol,npix,width)
        #print nonzeropix_ref
        # shift 3 pixels downward
        nonzeropix_ref[0]-=nshift*npol*npix
        nonzeropix_ref[1]-=nshift*npol*npix
        nonzeropix=self.data.nonzero()[1]
        #print nonzeropix_ref
        #print nonzeropix
        self.nonzero(nonzeropix_ref,nonzeropix)

class tsdgrid_flagging2(tsdgrid_unittest_base,unittest.TestCase):
    """
    This is test suite for handling flag information in tsdgrid.
    """
    rawfile='testimage1chan.map.asap'
    modified_file=rawfile+'.mod'
    def setUp(self):
        if os.path.exists(self.rawfile):
            shutil.rmtree(self.rawfile)
        shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        table = gentools(['tb'])[0]
        table.open(self.rawfile, nomodify=False)
        spectra = table.getcol('SPECTRA')
        flagtra = table.getcol('FLAGTRA')
        spectra_new = numpy.concatenate([spectra, spectra])
        flagtra_new = numpy.concatenate([flagtra, flagtra])
        table.putcol('SPECTRA', spectra_new)
        table.putcol('FLAGTRA', flagtra_new)
        table.close()
        
        default(tsdgrid)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        if (os.path.exists(self.outfile)):
            shutil.rmtree(self.outfile)

    def test_channelflag(self):
        """test_channelflag: test specific channels are flagged"""
        # flag all channels
        scan = sd.scantable(self.rawfile, average=False)
        mask = scan.create_mask([0,0])
        scan.flag(mask)
        scan.save(self.modified_file, overwrite=True)

        # run tsdgrid
        npix=17
        res=tsdgrid(infiles=self.modified_file,gridfunction='BOX',npix=npix,cell='20arcsec',outfile=self.outfile,plot=False)
        self.assertEqual(res,None,
                         msg='Any error occurred during gridding')
        
        # check result
        table = gentools(['tb'])[0]
        table.open(self.outfile)
        flagtra = table.getcol('FLAGTRA')
        table.close()
        self.assertTrue(all(flagtra[0] != 0),
                        msg='Channel 0 should be flagged for all spectra')
        self.assertTrue(all(flagtra[1] == 0),
                        msg='Channel 1 should not be flagged')
        
class tsdgrid_selection(selection_syntax.SelectionSyntaxTest,
                        tsdgrid_unittest_base,unittest.TestCase):
    """
    Test selection syntax. Selection parameters to test are:
    field, spw (no channel selection), scan, pol
    
    Data used for this test are sd_analytic_type1-3.asap (raw data)
    and sdgrid_selection.asap.ref (reference data).
    The reference data are generated using the following expr parameter:
    expr='V1*V1+1.0' where V1='sd_analytic_type1-3.asap'.

    """
    # Input and output names
    rawfile='sd_analytic_type1-3.asap'
    reffile='sdgrid_test_selection.asap.ref'
    prefix=tsdgrid_unittest_base.taskname+'TestSel'
    postfix='.grid.asap'
    field_prefix = 'M100__'

    @property
    def task(self):
        return tsdgrid
    
    @property
    def spw_channel_selection(self):
        return False

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)
        if (not os.path.exists(self.reffile)):
            shutil.copytree(self.datapath+self.reffile, self.reffile)

        default(tsdgrid)

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
        infiles=[self.rawfile]
        outname=self.prefix+self.postfix
        scan=''
        self.res=tsdgrid(scan=scan,infiles=infiles,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, self.reffile)
    
    def test_scan_id_exact(self):
        """ test scan selection (scan='15')"""
        infiles=[self.rawfile]
        outname=self.prefix+self.postfix
        reffile0='sdgrid_test_selection_scan15.asap.ref'
        if (not os.path.exists(reffile0)):
            shutil.copytree(self.datapath+reffile0, reffile0)
        scan = '15'
        self.res=tsdgrid(scan=scan,infiles=infiles,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, reffile0)
        if (os.path.exists(reffile0)):
            shutil.rmtree(reffile0)

    def test_scan_id_lt(self):
        """ test scan selection (scan='<16')"""
        infiles=[self.rawfile]
        outname=self.prefix+self.postfix
        reffile0='sdgrid_test_selection_scan15.asap.ref'
        if (not os.path.exists(reffile0)):
            shutil.copytree(self.datapath+reffile0, reffile0)
        scan = '<16'
        self.res=tsdgrid(scan=scan,infiles=infiles,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, reffile0)
        if (os.path.exists(reffile0)):
            shutil.rmtree(reffile0)

    def test_scan_id_gt(self):
        """ test scan selection (scan='>15')"""
        infiles=[self.rawfile]
        outname=self.prefix+self.postfix
        reffile0='sdgrid_test_selection_scangt15.asap.ref'
        if (not os.path.exists(reffile0)):
            shutil.copytree(self.datapath+reffile0, reffile0)
        scan = '>15'
        self.res=tsdgrid(scan=scan,infiles=infiles,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, reffile0)
        if (os.path.exists(reffile0)):
            shutil.rmtree(reffile0)
    
    def test_scan_id_range(self):
        """ test scan selection (scan='15~16')"""
        infiles=[self.rawfile]
        outname=self.prefix+self.postfix
        reffile0='sdgrid_test_selection_scan15.asap.ref'
        if (not os.path.exists(reffile0)):
            shutil.copytree(self.datapath+reffile0, reffile0)
        scan = '15~16'
        self.res=tsdgrid(scan=scan,infiles=infiles,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, reffile0)
        if (os.path.exists(reffile0)):
            shutil.rmtree(reffile0)
    
    def test_scan_id_list(self):
        """ test scan selection (scan='15,17')"""
        infiles=[self.rawfile]
        outname=self.prefix+self.postfix
        scan = '15,17'
        self.res=tsdgrid(scan=scan,infiles=infiles,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, self.reffile)
    
    def test_scan_id_exprlist(self):
        """ test scan selection (scan='<16, 17')"""
        infiles=[self.rawfile]
        outname=self.prefix+self.postfix
        scan = '<16, 17'
        self.res=tsdgrid(scan=scan,infiles=infiles,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, self.reffile)

    ####################
    # pol
    ####################
    def test_pol_id_default(self):
        """test pol selection (pol='')"""
        infiles=[self.rawfile]
        outname=self.prefix+self.postfix
        pol=''
        self.res=tsdgrid(pol=pol,infiles=infiles,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, self.reffile)

    @unittest.expectedFailure
    def test_pol_id_exact(self):
        """ test pol selection (pol='1')"""
#        infiles=[self.rawfile]
#        outname=self.prefix+self.postfix
#        pol = '1'
#        self.res=tsdgrid(pol=pol,infiles=infiles,outfile=outname)
#        tbsel = {'POLNO': [1]}
#        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
#        self._comparecal_with_selection(outname, tbsel)
        self._default_test()

    def test_pol_id_lt(self):
        """ test pol selection (pol='<1')"""
        infiles=[self.rawfile]
        outname=self.prefix+self.postfix
        pol = '<1'
        self.res=tsdgrid(pol=pol,infiles=infiles,outfile=outname)
        tbsel = {'POLNO': [0]}
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._comparecal_with_selection(outname, tbsel)

    @unittest.expectedFailure
    def test_pol_id_gt(self):
        """ test pol selection (pol='>0')"""
#        infiles=[self.rawfile]
#        outname=self.prefix+self.postfix
#        pol = '>0'
#        self.res=tsdgrid(pol=pol,infiles=infiles,outfile=outname)
#        tbsel = {'POLNO': [1]}
#        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
#        self._comparecal_with_selection(outname, tbsel)
        self._default_test()
    
    def test_pol_id_range(self):
        """ test pol selection (pol='0~1')"""
        infiles=[self.rawfile]
        outname=self.prefix+self.postfix
        pol = '0~1'
        self.res=tsdgrid(pol=pol,infiles=infiles,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, self.reffile)
    
    def test_pol_id_list(self):
        """ test pol selection (pol='0,1')"""
        infiles=[self.rawfile]
        outname=self.prefix+self.postfix
        pol = '0,1'
        self.res=tsdgrid(pol=pol,infiles=infiles,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, self.reffile)
    
    @unittest.expectedFailure
    def test_pol_id_exprlist(self):
        """test pol selection (pol='')"""
        self._default_test()
    
    ####################
    # spw 
    ####################
    def test_spw_id_default(self):
        """test spw selection (spw='')"""
        infiles=[self.rawfile]
        outname=self.prefix+self.postfix
        reffile0='sdgrid_test_selection_spwnullstring.asap.ref'
        if (not os.path.exists(reffile0)):
            shutil.copytree(self.datapath+reffile0, reffile0)
        spw=''
        self.res=tsdgrid(spw=spw,infiles=infiles,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, reffile0)
        if (os.path.exists(reffile0)):
            shutil.rmtree(reffile0)

    def test_spw_id_exact(self):
        """ test spw selection (spw='22')"""
        infiles=[self.rawfile]
        outname=self.prefix+self.postfix
        reffile0='sdgrid_test_selection_spw22.asap.ref'
        if (not os.path.exists(reffile0)):
            shutil.copytree(self.datapath+reffile0, reffile0)
        spw='22'
        self.res=tsdgrid(spw=spw,infiles=infiles,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, reffile0)
        if (os.path.exists(reffile0)):
            shutil.rmtree(reffile0)
    
    def test_spw_id_lt(self):
        """ test spw selection (spw='<25')"""
        infiles=[self.rawfile]
        outname=self.prefix+self.postfix
        reffile0='sdgrid_test_selection_spwlt25.asap.ref'
        if (not os.path.exists(reffile0)):
            shutil.copytree(self.datapath+reffile0, reffile0)
        spw='<25'
        self.res=tsdgrid(spw=spw,infiles=infiles,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, reffile0)
        if (os.path.exists(reffile0)):
            shutil.rmtree(reffile0)
    
    def test_spw_id_gt(self):
        """ test spw selection (spw='>21')"""
        infiles=[self.rawfile]
        outname=self.prefix+self.postfix
        reffile0='sdgrid_test_selection_spwgt21.asap.ref'
        if (not os.path.exists(reffile0)):
            shutil.copytree(self.datapath+reffile0, reffile0)
        spw='>21'
        self.res=tsdgrid(spw=spw,infiles=infiles,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, reffile0)
        if (os.path.exists(reffile0)):
            shutil.rmtree(reffile0)
    
    def test_spw_id_range(self):
        """ test spw selection (spw='21~24')"""
        infiles=[self.rawfile]
        outname=self.prefix+self.postfix
        reffile0='sdgrid_test_selection_spwlt25.asap.ref'
        if (not os.path.exists(reffile0)):
            shutil.copytree(self.datapath+reffile0, reffile0)
        spw='<25'
        self.res=tsdgrid(spw=spw,infiles=infiles,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, reffile0)
        if (os.path.exists(reffile0)):
            shutil.rmtree(reffile0)
    
    def test_spw_id_list(self):
        """ test spw selection (spw='21,22,23,25')"""
        infiles=[self.rawfile]
        outname=self.prefix+self.postfix
        reffile0='sdgrid_test_selection_spw21222325.asap.ref'
        if (not os.path.exists(reffile0)):
            shutil.copytree(self.datapath+reffile0, reffile0)
        spw='21,22,23,25'
        self.res=tsdgrid(spw=spw,infiles=infiles,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, reffile0)
        if (os.path.exists(reffile0)):
            shutil.rmtree(reffile0)
    
    def test_spw_id_exprlist(self):
        """ test spw selection (spw='<22,>24')"""
        infiles=[self.rawfile]
        outname=self.prefix+self.postfix
        reffile0='sdgrid_test_selection_spw21222325.asap.ref'
        if (not os.path.exists(reffile0)):
            shutil.copytree(self.datapath+reffile0, reffile0)
        spw='<22,>24'
        self.res=tsdgrid(spw=spw,infiles=infiles,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, reffile0)
        if (os.path.exists(reffile0)):
            shutil.rmtree(reffile0)
    
    def test_spw_id_pattern(self):
        """test spw selection (spw='*')"""
        infiles=[self.rawfile]
        outname=self.prefix+self.postfix
        reffile0='sdgrid_test_selection_spwnullstring.asap.ref'
        if (not os.path.exists(reffile0)):
            shutil.copytree(self.datapath+reffile0, reffile0)
        spw='*'
        self.res=tsdgrid(spw=spw,infiles=infiles,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, reffile0)
        if (os.path.exists(reffile0)):
            shutil.rmtree(reffile0)
    
    def test_spw_value_frequency(self):
        """test spw selection (spw='300~310GHz')"""
        infiles=[self.rawfile]
        outname=self.prefix+self.postfix
        reffile0='sdgrid_test_selection_spw300to310ghz.asap.ref'
        if (not os.path.exists(reffile0)):
            shutil.copytree(self.datapath+reffile0, reffile0)
        spw='300~310GHz'
        self.res=tsdgrid(spw=spw,infiles=infiles,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, reffile0)
        if (os.path.exists(reffile0)):
            shutil.rmtree(reffile0)
    
    def test_spw_value_velocity(self):
        """test spw selection (spw='-50~50km/s')"""
        infiles=[self.rawfile]
        outname=self.prefix+self.postfix
        reffile0='sdgrid_test_selection_spw-50to50kms.asap.ref'
        if (not os.path.exists(reffile0)):
            shutil.copytree(self.datapath+reffile0, reffile0)
        spw='-50~50km/s'
        self.res=tsdgrid(spw=spw,infiles=infiles,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, reffile0)
        if (os.path.exists(reffile0)):
            shutil.rmtree(reffile0)
    
    def test_spw_mix_exprlist(self):
        """test spw selection (spw='150~500km/s,>23')"""
        infiles=[self.rawfile]
        outname=self.prefix+self.postfix
        reffile0='sdgrid_test_selection_spw150to500kmsgt23.asap.ref'
        if (not os.path.exists(reffile0)):
            shutil.copytree(self.datapath+reffile0, reffile0)
        spw='150~500km/s,>23'
        self.res=tsdgrid(spw=spw,infiles=infiles,outfile=outname)
        self.assertEqual(self.res,None, msg='Any error occurred during calibration')
        self._compare(outname, reffile0)
        if (os.path.exists(reffile0)):
            shutil.rmtree(reffile0)
    
    ####################
    # Helper functions
    ####################
    def _comparecal_with_selection( self, name, tbsel={} ):
        self._checkfile(name)
        sp=self._getspectra_selected(name, tbsel)
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

def suite():
    return [#tsdgrid_failure_case, tsdgrid_single_integ,
            #tsdgrid_clipping, tsdgrid_flagging,
            #tsdgrid_weighting, tsdgrid_map,
            #tsdgrid_dec_correction, tsdgrid_grid_center,
            #tsdgrid_flagging2,
            tsdgrid_selection]
