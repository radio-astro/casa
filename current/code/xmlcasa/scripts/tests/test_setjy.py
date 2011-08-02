import numpy
import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
from tests.test_split import check_eq, datapath, SplitChecker
import unittest

'''
Unit tests for task setjy.

Features tested:
  1. Does setjy(modimage=modelimu, fluxdensity=0) NOT scale the model image's
     flux density?
  2. Does setjy(modimage=modelimu) scale the model image's flux density?
  3. Solar system (Uranus) flux density calibration.
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
        for use_standard in [False, True]:
            self.__class__.records[use_standard] = self.run_setjy(use_standard)
        self.__class__.records['fluxdens'] = self.run_setjy(False, 1234.0)
        self.__class__.records['spix'] = self.run_setjy(False,
                                                        1234.0 * (43.42064/35.0)**0.7,
                                                        -0.7,
                                                        "35.0GHz")
        shutil.rmtree(self.inpms)
        # setjy returns None :-P
        #return self.__class__.records[False]['setjyran'] and \
        #       self.__class__.records[True]['setjyran']
        return True

    def run_setjy(self, use_standard, fluxdens=0, spix=0, reffreq="1GHz"):
        record = {'setjyran': False}
        try:
            if use_standard:
                record['setjyran'] = setjy(vis=self.inpms, field=self.field,
                                           modimage=self.modelim,
                                           standard='Perley-Taylor 99',
                                           async=False)
            else:
                record['setjyran'] = setjy(vis=self.inpms, field=self.field,
                                           modimage=self.modelim,
                                           fluxdensity=fluxdens,
                                           spix=spix, reffreq=reffreq,
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
            check_eq(self.records[True]['short'], 0.911185, 0.025)
            check_eq(self.records[True]['long'],  0.808885, 0.025)
        except Exception, e:
            print "results with scaling:", self.records[True]
            raise e
        
    def test_fluxdens(self):
        """modimage != '' and fluxdensity > 0"""
        try:
            check_eq(self.records['fluxdens']['short'], 1233.7, 0.05)
            check_eq(self.records['fluxdens']['long'],  1095.2, 0.05)
        except Exception, e:
            print "results with modimage and fluxdensity", self.records['fluxdens']
            raise e    

    def test_spix(self):
        """modimage != '', fluxdensity > 0, and spix = -0.7"""
        try:
            check_eq(self.records['spix']['short'], 1233.7, 0.5)
            check_eq(self.records['spix']['long'],  1095.2, 0.5)
        except Exception, e:
            print "results with modimage and fluxdensity", self.records['fluxdens']
            raise e    

class Uranus(SplitChecker):
    need_to_initialize = True
    inpms = 'unittest/setjy/2528.ms'
    corrsels = ['']
    records = {}

    def do_split(self, corrsel):
        """
        Doesn't really run split; just setjy.
        """
        record = {}

        # Paranoia: check that inpms doesn't already have MODEL_DATA.
        # Otherwise, we could mistake old results for new ones.  That could be
        # fixed by splitting out DATA, but inpms is not supposed to require
        # that.
        tb.open(self.inpms)
        cols = tb.colnames()
        tb.close()
        if 'MODEL_DATA' in cols:
            raise ValueError, "The input MS, " + inpms + " already has a MODEL_DATA col"

        try:
            print "\nRunning setjy(field='Uranus')."
            sjran = setjy(self.inpms, field='Uranus', spw='', modimage='',
                          scalebychan=False, fluxdensity=-1,
                          standard='Butler-JPL-Horizons 2010', async=False)
        except Exception, e:
            print "Error running setjy(field='Uranus')"
            raise e
        try:
            tb.open(self.inpms)
            cols = tb.colnames()
            if 'MODEL_DATA' not in cols:
                raise AssertionError, "setjy(field='Uranus') did not add a MODEL_DATA column"
        except AssertionError, e:
            raise e
        else:
            record['wvr'] = tb.getcell('MODEL_DATA', 0)
            record['auto3'] = tb.getcell('MODEL_DATA', 10)
            record['long3'] = tb.getcell('MODEL_DATA', 11)
            record['auto4'] = tb.getcell('MODEL_DATA', 2)
            record['med4'] = tb.getcell('MODEL_DATA', 4)
            record['long4'] = tb.getcell('MODEL_DATA', 3)
        finally:
            tb.close()
        self.__class__.records[corrsel] = record
        return sjran

    def test_wvr(self):
        """WVR spw"""
        check_eq(self.records['']['wvr'], numpy.array([[26.42447472+0.j,
                                                        26.42447472+0.j]]),
                 0.0001)
    def test_auto3(self):
        """Zero spacing of spw 3"""
        check_eq(self.records['']['auto3'], numpy.array([[65.85110474+0.j],
                                                         [65.85110474+0.j]]),
                 0.0001)
    def test_long3(self):
        """Long spacing of spw 3"""
        check_eq(self.records['']['long3'], numpy.array([[4.74397755+0.j],
                                                         [4.74397755+0.j]]),
                 0.0001)
    def test_auto4(self):
        """Zero spacing of spw 4"""
        check_eq(self.records['']['auto4'], numpy.array([[ 69.381073+0.j],
                                                         [ 69.381073+0.j]]),
                 0.0001)
    def test_med4(self):
        """Medium spacing of spw 4"""
        check_eq(self.records['']['med4'], numpy.array([[38.01920319+0.j],
                                                        [38.01920319+0.j]]),
                 0.0001)
    def test_long4(self):
        """Long spacing of spw 4"""
        check_eq(self.records['']['long4'], numpy.array([[ 2.82068014+0.j],
                                                         [ 2.82068014+0.j]]),
                 0.0001)

class ScaleUranusByChan(SplitChecker):
    need_to_initialize = True
    inpms = 'unittest/setjy/2528.ms'  # Uranus again
    corrsels = ['']
    records = {}

    def do_split(self, corrsel):
        """
        Doesn't really run split; just setjy.
        """
        record = {}

        # Paranoia: check that inpms doesn't already have MODEL_DATA.
        # Otherwise, we could mistake old results for new ones.  That could be
        # fixed by splitting out DATA, but inpms is not supposed to require
        # that.
        tb.open(self.inpms)
        cols = tb.colnames()
        tb.close()
        if 'MODEL_DATA' in cols:
            raise ValueError, "The input MS, " + inpms + " already has a MODEL_DATA col"

        try:
            print "\nRunning setjy(field='Uranus')."
            sjran = setjy(self.inpms, field='Uranus', spw='', modimage='',
                          scalebychan=True, fluxdensity=-1,
                          standard='Butler-JPL-Horizons 2010', async=False)
        except Exception, e:
            print "Error running setjy(field='Uranus')"
            raise e
        try:
            tb.open(self.inpms)
            cols = tb.colnames()
            if 'MODEL_DATA' not in cols:
                raise AssertionError, "setjy(field='Uranus') did not add a MODEL_DATA column"
        except AssertionError, e:
            raise e
        else:
            record['wvr'] = tb.getcell('MODEL_DATA', 0)
            record['auto1'] = tb.getcell('MODEL_DATA', 18)
            record['long1'] = tb.getcell('MODEL_DATA', 19)
            record['auto4'] = tb.getcell('MODEL_DATA', 2)
            record['long4'] = tb.getcell('MODEL_DATA', 3)
        finally:
            tb.close()
        self.__class__.records[corrsel] = record
        return sjran

    def test_wvr(self):
        """WVR spw with scalebychan"""
        check_eq(self.records['']['wvr'], numpy.array([[25.951+0.j,
                                                        26.901+0.j]]),
                 0.003)
    def test_auto1(self):
        """Zero spacing of spw 1 with scalebychan"""
        # 8 (decreasing freq!) chans, XX & YY.
        check_eq(self.records['']['auto1'],
                 numpy.array([[65.53865814+0.j, 65.46551514+0.j,
                               65.39238739+0.j, 65.31927490+0.j,
                               65.24617004+0.j, 65.17308807+0.j,
                               65.10002136+0.j, 65.02696228+0.j],
                              [65.53865814+0.j, 65.46551514+0.j,
                               65.39238739+0.j, 65.31927490+0.j,
                               65.24617004+0.j, 65.17308807+0.j,
                               65.10002136+0.j, 65.02696228+0.j]]),
                 0.0001)
    def test_long1(self):
        """Long spacing of spw 1 with scalebychan"""
        check_eq(self.records['']['long1'],
                 numpy.array([[4.91202736+0.j, 4.95130348+0.j,
                               4.99054718+0.j, 5.02975798+0.j,
                               5.06893539+0.j, 5.10807943+0.j,
                               5.14718962+0.j, 5.18626642+0.j],
                              [4.91202736+0.j, 4.95130348+0.j,
                               4.99054718+0.j, 5.02975798+0.j,
                               5.06893539+0.j, 5.10807943+0.j,
                               5.14718962+0.j, 5.18626642+0.j]]),
                 0.0001)
    # spw 4 only has 1 chan, so it should be the same as without scalebychan.
    def test_auto4(self):
        """Zero spacing of spw 4 with scalebychan"""
        check_eq(self.records['']['auto4'], numpy.array([[ 69.381073+0.j],
                                                         [ 69.381073+0.j]]),
                 0.0001)
    def test_long4(self):
        """Long spacing of spw 4 with scalebychan"""
        check_eq(self.records['']['long4'], numpy.array([[ 2.82068014+0.j],
                                                         [ 2.82068014+0.j]]),
                 0.0001)
            
def suite():
    return [setjy_test_modimage, Uranus, ScaleUranusByChan]
