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

from msmoments import msmoments

#
# Unit test of msmoments task.
# 

###
# Base class for msmoments unit test
###
class msmoments_unittest_base:
    """
    Base class for msmoments unit test
    """
    taskname='msmoments'
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/msmoments/'
    place=5
    
    def _compare(self,tab,ref,unit):
        cols=tab.getcolkeywords('FLOAT_DATA')
        if cols.has_key('QuantumUnits'):
            cunit=cols['QuantumUnits']
        else:
            cunit=cols['UNIT']
        self.assertEqual(cunit,unit,
                         msg='Unit %s is wrong (should be %s)'%(cunit,unit))
        sp=tab.getcell('FLOAT_DATA',0)
        self.assertEqual(len(sp[0]),1,
                         msg='moment must be 1 per RR sectrum')
        if sp[0][0] > 10.0:
            while(sp[0][0]>10.0):
                sp[0][0] /= 10.0
                ref[0] /= 10.0
        elif sp[0][0] < 1.0:
            while(sp[0][0]<1.0):
                sp[0][0] *= 10.0
                ref[0] *= 10.0
        #print sp[0][0], ref[0]
        self.assertAlmostEqual(sp[0][0],ref[0],self.place,
                               msg='RR value differ.')
        if sp[1][0] > 10.0:
            while(sp[1][0]>10.0):
                sp[1][0] /= 10.0
                ref[1] /= 10.0
        elif sp[1][0] < 1.0:
            while(sp[1][0]<1.0):
                sp[1][0] *= 10.0
                ref[1] *= 10.0
        #print sp[1][0], ref[1]
        self.assertEqual(len(sp[1]),1,
                         msg='moment must be 1 per LL sectrum')
        self.assertAlmostEqual(sp[1][0],ref[1],self.place,
                               msg='LL value differ.')
        tab.close()
        
                
###
# Test on bad parameter settings
###
class msmoments_test0(unittest.TestCase,msmoments_unittest_base):
    """
    Test on bad parameter setting

    """
    # Input and output names
    rawfile='msmoments0.ms'
    prefix=msmoments_unittest_base.taskname+'Test0'
    outfile=prefix+'.ms'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(msmoments)

    def tearDown(self):
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test000(self):
        """Test 000: Default parameters"""
        res=msmoments()
        self.assertFalse(res)

    def test001(self):
        """Test 001: Undefined moment"""
        moments=[-2]
        try:
            msmoments(infile=self.rawfile,moments=moments,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except StandardError, e:
            pos=str(e).find('Illegal moment requested')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))
        except Exception, e:
            self.assertTrue(False,
                            msg='Unexpected exception was thrown: %s'%(str(e)))        

    def test002(self):
        """Test 002: Illegal antenna id"""
        antenna=-2
        #def call():
        #    msmoments(infile=self.rawfile,antenna=antnna,outfile=self.outfile)
        #self.assertRaises(StandardError, call)
        try:
            msmoments(infile=self.rawfile,antenna=antenna,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except StandardError, e:
            pos=str(e).find('Antenna Expression: Parse error')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))
        except Exception, e:
            self.assertTrue(False,
                            msg='Unexpected exception was thrown: %s'%(str(e)))
        
    def test003(self):
        """Test 003: Illegal field name"""
        field='SOMEWHARE'
        #def call():
        #    msmoments(infile=self.rawfile,field=field,outfile=self.outfile)
        #self.assertRaises(StandardError, call)
        try:
            msmoments(infile=self.rawfile,field=field,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except StandardError, e:
            pos=str(e).find('Field Expression: No match found')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))
        except Exception, e:
            self.assertTrue(False,
                            msg='Unexpected exception was thrown: %s'%(str(e)))

    def test004(self):
        """Test 004: Illegal spw id"""
        id=99
        #def call():
        #    msmoments(infile=self.rawfile,spw=id,outfile=self.outfile)
        #self.assertRaises(StandardError, call)
        try:
            msmoments(infile=self.rawfile,spw=id,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except StandardError, e:
            pos=str(e).find('No Spw ID(s) matched')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))
        except Exception, e:
            self.assertTrue(False,
                            msg='Unexpected exception was thrown: %s'%(str(e)))

    def test005(self):
        """Test 005: Existing output file"""
        shutil.copytree(self.datapath+self.rawfile, self.outfile)
        try:
            msmoments(infile=self.rawfile,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            self.assertEqual(str(e),'%s exists.'%(self.outfile),
                             msg='Unexpected exception was thrown.')

###
# Test to calculate moments
###
class msmoments_test1(unittest.TestCase,msmoments_unittest_base):
    """
    Test to calculate moments.

    Original data is OrionS_rawACSmod.
    The data is calibrated and averaged to get good example
    for moment calculation.

    """
    # Input and output names
    rawfile='msmoments0.ms'
    prefix=msmoments_unittest_base.taskname+'Test1'
    outfile=prefix+'.ms'
    _tb = None

    def setUp(self):
        self.res=None
        self._tb = tbtool()
        if (not os.path.exists(self.rawfile)):
            shutil.copytree(self.datapath+self.rawfile, self.rawfile)

        default(msmoments)

    def tearDown(self):
        self._tb.close()
        del self._tb
        if (os.path.exists(self.rawfile)):
            shutil.rmtree(self.rawfile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test100(self):
        """Test 100: Mean"""
        moments=[-1]
        postfix='.average'
        refval=[2.8974850177764893,2.6505289077758789]
        if (os.path.exists(self.outfile)):
            shutil.rmtree(self.outfile)
        res=msmoments(infile=self.rawfile,moments=moments,outfile=self.outfile)
        # the task doesn't return table anymore
        self.assertTrue(os.path.exists(self.outfile),
                        msg='Out put table does not exist.')
        self.assertTrue(self._tb.open(self.outfile),
                        msg='Failed to open output table')
        self._compare(self._tb,refval,'K')
#         # the task must return table object
#         self.assertEqual(type(res),type(tb),
#                          msg='Any error occurred during task execution')
#         self.assertEqual(res.name().split('/')[-1],self.outfile,
#                             msg='Returned table is wrong.')
#         self._compare(res,refval,'K')

    def test101(self):
        """Test 101: Integrated intensity"""
        moments=[0]
        postfix='.integrated'
        refval=[954.87066650390625,873.48583984375]
        res=msmoments(infile=self.rawfile,moments=moments,outfile=self.outfile)
        # the task doesn't return table anymore
        self.assertTrue(os.path.exists(self.outfile),
                        msg='Out put table does not exist.')
        self.assertTrue(self._tb.open(self.outfile),
                        msg='Failed to open output table')
        self._compare(self._tb,refval,'K.km/s')

    def test102(self):
        """Test 102: Weighted coordinate (velocity field)"""
        moments=[1]
        postfix='.weighted_coord'
        refval=[6.8671870231628418,7.6601519584655762]
        res=msmoments(infile=self.rawfile,moments=moments,outfile=self.outfile)
        # the task doesn't return table anymore
        self.assertTrue(os.path.exists(self.outfile),
                        msg='Out put table does not exist.')
        self.assertTrue(self._tb.open(self.outfile),
                        msg='Failed to open output table')
        self._compare(self._tb,refval,'km/s')

    def test103(self):
        """Test 103: Weighted dispersion of coordinate (velocity dispersion)"""
        moments=[2]
        postfix='.weighted_dispersion_coord'
        refval=[95.011787414550781,95.257194519042969]
        res=msmoments(infile=self.rawfile,moments=moments,outfile=self.outfile)
        # the task doesn't return table anymore
        self.assertTrue(os.path.exists(self.outfile),
                        msg='Out put table does not exist.')
        self.assertTrue(self._tb.open(self.outfile),
                        msg='Failed to open output table')
        self._compare(self._tb,refval,'km/s')

    def test104(self):
        """Test 104: Median"""
        moments=[3]
        postfix='.median'
        refval=[2.8934342861175537,2.6451926231384277]
        res=msmoments(infile=self.rawfile,moments=moments,outfile=self.outfile)
        # the task doesn't return table anymore
        self.assertTrue(os.path.exists(self.outfile),
                        msg='Out put table does not exist.')
        self.assertTrue(self._tb.open(self.outfile),
                        msg='Failed to open output table')
        self._compare(self._tb,refval,'K')

    def test105(self):
        """Test 105: Median coordinate (fail at the moment)"""
        moments=[4]
        postfix='.median_coordinate'
        refval=[]
        try:
            res=msmoments(infile=self.rawfile,spw=id,moments=moments,outfile=self.outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except StandardError, e:
            pos=str(e).find('moment not allowed to calculate')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))
        except Exception, e:
            self.assertTrue(False,
                            msg='Unexpected exception was thrown: %s'%(str(e)))


    def test106(self):
        """Test 106: Standard deviation about the mean"""
        moments=[5]
        postfix='.standard_deviation'
        refval=[0.13466399908065796,0.13539622724056244]
        res=msmoments(infile=self.rawfile,moments=moments,outfile=self.outfile)
        # the task doesn't return table anymore
        self.assertTrue(os.path.exists(self.outfile),
                        msg='Out put table does not exist.')
        self.assertTrue(self._tb.open(self.outfile),
                        msg='Failed to open output table')
        self._compare(self._tb,refval,'K')

    def test107(self):
        """Test 107: Rms"""
        moments=[6]
        postfix='.rms'
        refval=[2.9006123542785645, 2.6539843082427979]
        res=msmoments(infile=self.rawfile,moments=moments,outfile=self.outfile)
        # the task doesn't return table anymore
        self.assertTrue(os.path.exists(self.outfile),
                        msg='Out put table does not exist.')
        self.assertTrue(self._tb.open(self.outfile),
                        msg='Failed to open output table')
        self._compare(self._tb,refval,'K')

    def test108(self):
        """Test 108: Absolute mean deviation"""
        moments=[7]
        postfix='.abs_mean_dev'
        refval=[0.09538549929857254, 0.10068970918655396]
        res=msmoments(infile=self.rawfile,moments=moments,outfile=self.outfile)
        # the task doesn't return table anymore
        self.assertTrue(os.path.exists(self.outfile),
                        msg='Out put table does not exist.')
        self.assertTrue(self._tb.open(self.outfile),
                        msg='Failed to open output table')
        self._compare(self._tb,refval,'K')

    def test109(self):
        """Test 109: Maximum value"""
        moments=[8]
        postfix='.maximum'
        refval=[3.9766585826873779, 3.7296187877655029]
        res=msmoments(infile=self.rawfile,moments=moments,outfile=self.outfile)
        # the task doesn't return table anymore
        self.assertTrue(os.path.exists(self.outfile),
                        msg='Out put table does not exist.')
        self.assertTrue(self._tb.open(self.outfile),
                        msg='Failed to open output table')
        self._compare(self._tb,refval,'K')

    def test110(self):
        """Test 110: Coordinate of maximum value"""
        moments=[9]
        postfix='.maximum_Coord'
        refval=[45489389568.0, 45489410048.0]
        res=msmoments(infile=self.rawfile,moments=moments,outfile=self.outfile)
        # the task doesn't return table anymore
        self.assertTrue(os.path.exists(self.outfile),
                        msg='Out put table does not exist.')
        self.assertTrue(tb.open(self.outfile),
                        msg='Failed to open output table')
        self._compare(tb,refval,'km/s') #Hz?

    def test111(self):
        """Test 111: Minimum value"""
        moments=[10]
        postfix='.minimum'
        refval=[1.7176277637481689, 1.4913345575332642]
        res=msmoments(infile=self.rawfile,moments=moments,outfile=self.outfile)
        # the task doesn't return table anymore
        self.assertTrue(os.path.exists(self.outfile),
                        msg='Out put table does not exist.')
        self.assertTrue(self._tb.open(self.outfile),
                        msg='Failed to open output table')
        self._compare(self._tb,refval,'K')

    def test112(self):
        """Test 112: Coordinate of minimum value"""
        moments=[11]
        postfix='.minimum_coord'
        refval=[45514190848.0, 45464350720.0]
        res=msmoments(infile=self.rawfile,moments=moments,outfile=self.outfile)
        # the task doesn't return table anymore
        self.assertTrue(os.path.exists(self.outfile),
                        msg='Out put table does not exist.')
        self.assertTrue(self._tb.open(self.outfile),
                        msg='Failed to open output table')
        self._compare(self._tb,refval,'km/s')# Hz?

    def test113(self):
        """Test 113: Multiple moments (mean + velocity field)"""
        moments=[-1,1]
        postfix=['.average','.weighted_coord']
        refval0=[2.8974850177764893,2.6505289077758789]
        refval1=[6.8671870231628418,7.6601519584655762]
        res=msmoments(infile=self.rawfile,moments=moments,outfile=self.outfile)
        # the task doesn't return table anymore
        # file existence check
        for imom in range(len(moments)):
            name='./'+self.outfile+postfix[imom]
            self.assertEqual(os.path.exists(name),True,
                             msg='%s must exist'%(name))
        # check moments[0] table
        self.assertTrue(self._tb.open(self.outfile+postfix[0]),
                        msg='Failed to open %s table' % postfix[0])
        self._compare(self._tb,refval0,'K')
        # check second moment
        self.assertTrue(self._tb.open(self.outfile+postfix[1]),
                        msg='Failed to open %s table' %  postfix[1])
        self._compare(self._tb,refval1,'km/s')

def suite():
    return [msmoments_test0,msmoments_test1]
