import os
import numpy
import sys
import shutil
from __main__ import default
from recipes.listshapes import listshapes
from tasks import *
from taskinit import *
import unittest

'''
Unit tests for task split.

Features tested:
  1. Are the POLARIZATION, DATA_DESCRIPTION, and (to some extent) the
     SPECTRAL_WINDOW tables correct with and without correlation selection?
  2. Are the data shapes and values correct with and without correlation
     selection?
  3. Are the WEIGHT and SIGMA shapes and values correct with and without
     correlation selection?

Note: The time_then_chan_avg regression is a more general test of split.
'''

datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/'

def check_eq(val, expval, tol=None):
    """
    Checks that val matches expval within tol.
    """
    if tol:
        are_eq = abs(val - expval) < tol
    else:
        are_eq = val == expval
    if hasattr(are_eq, 'all'):
        are_eq = are_eq.all()
    if not are_eq:
        raise ValueError, "%r != %r" % (val, expval)

class split_test1(unittest.TestCase):
    need_to_initialize = True
    doTearDown        = False
    
    def setUp(self):
        if self.need_to_initialize:
            self.initialize()
        
    def initialize(self):
        # The realization that need_to_initialize needs to be
        # a class variable more or less came from
        # http://www.gossamer-threads.com/lists/python/dev/776699
        self.__class__.need_to_initialize = False
                
        inpms = '0420+417/0420+417.ms'
        outms1 = 'output1.ms'
        self.__class__.inpms = inpms
        self.__class__.outms1 = outms1
    
        if not os.path.exists(inpms):
            # Copying is technically unnecessary for split,
            # but self.inpms is shared by other tests, so making
            # it readonly might break them.
            shutil.copytree(datapath + inpms, inpms)
        for doomedms in (outms1, ):
            shutil.rmtree(doomedms, ignore_errors=True)
        self.res = split(inpms, outms1, datacolumn='data',
                         field='', spw='', width=1, antenna='',
                         timebin='20s', timerange='',
                         scan='', array='', uvrange='',
                         correlation='', async=False)
        tb.open(outms1)
        self.__class__.data   = tb.getcell('DATA', 2)
        self.__class__.weight = tb.getcell('WEIGHT', 5)
        self.__class__.sigma  = tb.getcell('SIGMA', 7)
        tb.close()
    
    def tearDown(self):
        if self.doTearDown:
            #for doomedms in (inpms1, inpms2, self.outms1, self.outms2):
            for doomedms in (self.inpms, self.outms1):
                shutil.rmtree(doomedms, ignore_errors=True)
            self.__class__.doTearDown = False

    def test1(self):
        """
        Subtables, time avg. without correlation selection
        """
        assert listshapes(mspat=self.outms1)[self.outms1] == set([(4, 1)])
        self.__class__.doTearDown = True

    def test2(self):
        """
        DATA[2],   time avg. without correlation selection
        """
        check_eq(self.data,
                 numpy.array([[ 0.14428490-0.03145669j],
                              [-0.00379944+0.00710297j],
                              [-0.00381106-0.00066403j],
                              [ 0.14404297-0.04763794j]]),
                 0.0001)

    def test3(self):
        """
        WEIGHT[5], time avg. without correlation selection
        """
        check_eq(self.weight,
                 numpy.array([143596.34375, 410221.34375,
                              122627.1640625, 349320.625]),
                 1.0)

    def test4(self):
        """
        SIGMA[7],  time avg. without correlation selection
        """
        check_eq(self.sigma,
                 numpy.array([0.00168478, 0.00179394,
                              0.00182574, 0.00194404]),
                 0.0001)
        

def suite():
    return [split_test1]        
        
    
