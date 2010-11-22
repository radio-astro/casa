import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
from tests.test_split import check_eq, datapath
import unittest

'''
Unit tests for task setjy.

Features tested:
  1. Does setjy(modimage=modelimu, fluxdensity=0) NOT scale the model image's
     flux density?
  2. Does setjy(modimage=modelimu) scale the model image's flux density?
'''

class CheckAfterImportuvfits(unittest.TestCase):
    """
    Base class for unit test suites that want to convert a UVFITS file to an MS
    once, and then run multiple tests on that MS.
    """
    # Don't setup class variables here - the children would squabble over them.
    #
    # DO define a do_initial_setup() method in each subclass to do the work,
    # record the results for use by the tests, and rm the MS.  Any variables
    # that it sets for use by the tests must be class variables, i.e. prefixed
    # by self.__class__..  The tests, however, will refer to them as instance
    # variables.  Example: often do_initial_setup() will set
    # self.__class__.records, and the tests will use it as self.records.  This
    # quirk is a result of unittest.TestCase's preference for starting from
    # scratch, and tearing down afterwards, for each test.  That's exactly what
    # CheckAfterImportuvfits is avoiding.
    
    def setUp(self):
        if self.need_to_initialize:
            self.initialize()

    def tearDown(self):
        """
        Leave this to do_initial_setup().
        """
        pass
    
    def initialize(self):
        # The realization that need_to_initialize needs to be
        # a class variable more or less came from
        # http://www.gossamer-threads.com/lists/python/dev/776699
        self.__class__.need_to_initialize = False

        if not os.path.exists(self.inpuvf):
            raise EnvironmentError, "Missing input UVFITS file: " + datapath + self.inpuvf

        try:
            print "Importing", self.inpuvf, "to an MS."
            importuvfits(fitsfile=self.inpuvf, vis=self.inpms,
                         antnamescheme="new")
        except Exception, e:
            print "importuvfits error:"
            raise e
        self.res = self.do_initial_setup()

    def check_subtables(self, corrsel, expected):
        oms = self.records[corrsel]['ms']
        assert listshapes(mspat=oms)[oms] == set(expected)
        shutil.rmtree(oms)


class setjy_test_modimage(CheckAfterImportuvfits):
    need_to_initialize = True
    inpuvf = datapath + '/ATST2/NGC1333/N1333_1.UVFITS'
    inpms = 'n1333_1.ms'
    field = '0542+498_1'

    # The MS is in Q band, so deliberately choose the U band model so that the
    # structure is not too far off, but whether or not its flux density is
    # scaled makes a difference.
    modelim = datapath + '/../nrao/VLA/CalModels/3C147_U.im'

    records = {}
    
    def do_initial_setup(self):
        for do_scaling in [False, True]:
            self.__class__.records[do_scaling] = self.run_setjy(do_scaling)
        shutil.rmtree(self.inpms)
        # setjy returns None :-P
        #return self.__class__.records[False]['setjyran'] and \
        #       self.__class__.records[True]['setjyran']
        return True

    def run_setjy(self, do_scaling):
        record = {'setjyran': False}
        try:
            if do_scaling:
                record['setjyran'] = setjy(vis=self.inpms, field=self.field,
                                           modimage=self.modelim,
                                           async=False)
            else:
                record['setjyran'] = setjy(vis=self.inpms, field=self.field,
                                           modimage=self.modelim,
                                           fluxdensity=0,
                                           async=False)
            ms.open(self.inpms)
            record['short'] = ms.statistics(column='MODEL',
                                            complex_value='amp',
                                            field='0542+498_1',
                                            baseline='2&9',
                                            time='2003/05/02/19:53:30.0',
                                            correlation='rr')['MODEL']['mean']
            record['long']  = ms.statistics(column='MODEL',
                                            complex_value='amp',
                                            field='0542+498_1',
                                            baseline='21&24',
                                            time='2003/05/02/19:53:30.0',
                                            correlation='ll')['MODEL']['mean']
            ms.close()
        except Exception, e:
            print "Error from setjy or ms.statistics()"
            raise e
        return record

    def test_no_scaling(self):
        """modimage != '' and fluxdensity == 0 -> no scaling?"""
        try:
            check_eq(self.records[False]['short'], 2.712631, 0.05)
            check_eq(self.records[False]['long'],  2.4080808, 0.05)
        except Exception, e:
            print "results without scaling:", self.records[False]
            raise e
    
    def test_scaling(self):
        """modimage != '' and default fluxdensity -> scaling?"""
        try:
            check_eq(self.records[True]['short'], 0.9111846, 0.025)
            check_eq(self.records[True]['long'],  0.808885,  0.025)
        except Exception, e:
            print "results with scaling:", self.records[True]
            raise e
    

def suite():
    return [setjy_test_modimage]
