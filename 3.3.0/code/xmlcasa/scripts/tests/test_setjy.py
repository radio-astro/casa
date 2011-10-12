from glob import glob
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

def get_last_history_line(vis, origin='setjy::imager::setjy()', nback=0):
    """
    Finding the right history line is a bit tricky...it helps to filter
    by origin and read from the back to remain unaffected by changes
    elsewhere.

    Returns 'JUNK' on failure.
    """
    retline = ''
    try:
        tb.open(vis + '/HISTORY')
        st = tb.query('ORIGIN == "%s"' % origin, columns='MESSAGE')
        retline = st.getcell('MESSAGE', st.nrows() - 1 - nback)
        st.close()
        tb.close()
    except Exception:
        retline = 'JUNK'
    return retline

def check_history(histline, items):
    isok = True
    for item in items:
        if item not in histline:
            isok = False
            break
    if not isok:
        errmsg = "%s not found in %s.\n" % (items, histline)
        errmsg += "It could be that a change to HISTORY caused the wrong line to be selected."
        raise AssertionError, errmsg
    return isok

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
        things_to_rm = glob(self.inpms + '*')  # incl. flagversions.
        for t in things_to_rm:
            if os.path.isdir(t):
                shutil.rmtree(t)
            else:
                os.remove(t)
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
            record['history'] = get_last_history_line(self.inpms)
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

    def test_history_standard(self):
        """Flux density in HISTORY (standard)?"""
        # Don't bother checking it without scaling - it won't be there and isn't
        # interesting.
        check_history(self.records[True]['history'],
                      ["Scaling spw 1's model image to I ="])
            
    def test_history_fluxdensity(self):
        """Flux density in HISTORY (fluxdensity)?"""
        check_history(self.records['fluxdens']['history'],
                      ["Scaling spw 1's model image to I ="])
            
    def test_history_spix(self):
        """Flux density in HISTORY (spix)?"""
        check_history(self.records['spix']['history'],
                      ["Scaling spw 1's model image to I ="])
            

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
            tb.close()
            raise e
        else:
            record['wvr'] = tb.getcell('MODEL_DATA', 0)
            record['auto3'] = tb.getcell('MODEL_DATA', 10)
            record['long3'] = tb.getcell('MODEL_DATA', 11)
            record['auto4'] = tb.getcell('MODEL_DATA', 2)
            record['med4'] = tb.getcell('MODEL_DATA', 4)
            record['long4'] = tb.getcell('MODEL_DATA', 3)
            tb.close()
            record['history'] = get_last_history_line(self.inpms)
        self.__class__.records[corrsel] = record
        return sjran

    def test_history_Uranus(self):
        """Flux density in HISTORY (Uranus)?"""
        check_history(self.records['']['history'], ["Uranus", "V=0] Jy"])

    def test_wvr(self):
        """WVR spw"""
        check_eq(self.records['']['wvr'], numpy.array([[26.40653229+0.j,
                                                        26.40653229+0.j]]),
                 0.0001)
    def test_auto3(self):
        """Zero spacing of spw 3"""
        check_eq(self.records['']['auto3'], numpy.array([[65.80638885+0.j],
                                                         [65.80638885+0.j]]),
                 0.0001)
    def test_long3(self):
        """Long spacing of spw 3"""
        check_eq(self.records['']['long3'], numpy.array([[4.76111794+0.j],
                                                         [4.76111794+0.j]]),
                 0.0001)
    def test_auto4(self):
        """Zero spacing of spw 4"""
        check_eq(self.records['']['auto4'], numpy.array([[69.33396912+0.j],
                                                         [69.33396912+0.j]]),
                 0.0001)
    def test_med4(self):
        """Medium spacing of spw 4"""
        check_eq(self.records['']['med4'], numpy.array([[38.01076126+0.j],
                                                        [38.01076126+0.j]]),
                 0.0001)
    def test_long4(self):
        """Long spacing of spw 4"""
        check_eq(self.records['']['long4'], numpy.array([[2.83933783+0.j],
                                                         [2.83933783+0.j]]),
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
            tb.close()
            raise e
        else:
            record['wvr'] = tb.getcell('MODEL_DATA', 0)
            record['auto1'] = tb.getcell('MODEL_DATA', 18)
            record['long1'] = tb.getcell('MODEL_DATA', 19)
            record['auto4'] = tb.getcell('MODEL_DATA', 2)
            record['long4'] = tb.getcell('MODEL_DATA', 3)
            tb.close()
            record['history'] = get_last_history_line(self.inpms)
        self.__class__.records[corrsel] = record
        return sjran

    def test_history_scalebychan(self):
        """Flux density in HISTORY (scalebychan)?"""
        check_history(self.records['']['history'], ["Uranus", "V=0] Jy"])

    def test_wvr(self):
        """WVR spw with scalebychan"""
        check_eq(self.records['']['wvr'], numpy.array([[25.93320656+0.j,
                                                        26.88228607+0.j]]),
                 0.003)
    def test_auto1(self):
        """Zero spacing of spw 1 with scalebychan"""
        # 8 (decreasing freq!) chans, XX & YY.
        check_eq(self.records['']['auto1'],
                 numpy.array([[65.49415588+0.j, 65.42105865+0.j,
                               65.34798431+0.j, 65.27491760+0.j,
                               65.20187378+0.j, 65.12883759+0.j,
                               65.05581665+0.j, 64.98281097+0.j],
                              [65.49415588+0.j, 65.42105865+0.j,
                               65.34798431+0.j, 65.27491760+0.j,
                               65.20187378+0.j, 65.12883759+0.j,
                               65.05581665+0.j, 64.98281097+0.j]]),
                 0.0001)
    def test_long1(self):
        """Long spacing of spw 1 with scalebychan"""
        check_eq(self.records['']['long1'],
                 numpy.array([[4.92902184+0.j, 4.96826363+0.j,
                               5.00747252+0.j, 5.04664850+0.j,
                               5.08579159+0.j, 5.12490082+0.j,
                               5.16397619+0.j, 5.20301771+0.j],
                              [4.92902184+0.j, 4.96826363+0.j,
                               5.00747252+0.j, 5.04664850+0.j,
                               5.08579159+0.j, 5.12490082+0.j,
                               5.16397619+0.j, 5.20301771+0.j]]),
                 0.0001)
    # spw 4 only has 1 chan, so it should be the same as without scalebychan.
    def test_auto4(self):
        """Zero spacing of spw 4 with scalebychan"""
        check_eq(self.records['']['auto4'], numpy.array([[69.33396912+0.j],
                                                         [69.33396912+0.j]]),
                 0.0001)
    def test_long4(self):
        """Long spacing of spw 4 with scalebychan"""
        check_eq(self.records['']['long4'], numpy.array([[2.83933783+0.j],
                                                         [2.83933783+0.j]]),
                 0.0001)

class selectobs(SplitChecker):
    """Test CAS-3320"""
    need_to_initialize = True
    inpms = 'unittest/setjy/multiobs.ms'  # 3 concatted observations of Titan
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
            print "\nRunning setjy(field='Titan', observation=1)."
            sjran = setjy(self.inpms, field='Titan', spw='',
                          selectdata=True, observation=1, 
                          modimage='',
                          scalebychan=False, fluxdensity=-1,
                          standard='Butler-JPL-Horizons 2010', async=False)
        except Exception, e:
            print "Error running setjy(field='Titan', observation=1)"
            raise e
        try:
            tb.open(self.inpms)
            cols = tb.colnames()
            if 'MODEL_DATA' not in cols:
                raise AssertionError, "setjy(field='Titan') did not add a MODEL_DATA column"
        except AssertionError, e:
            tb.close()
            raise e
        else:
            record[0] = tb.getcell('MODEL_DATA', 0)[0, 0]
            record[1] = tb.getcell('MODEL_DATA', 666)[0]
            record[2] = tb.getcell('MODEL_DATA', 950)[0, 0]
            tb.close()
        self.__class__.records[corrsel] = record
        return sjran

    def test_obs0(self):
        """Was obsID 0 left alone?"""
        check_eq(self.records[''][0], 1.0+0.0j, 0.003)

    def test_obs1(self):
        """Was obsID 1 set?"""
        check_eq(self.records[''][1],
                 numpy.array([1.40439999+0.j, 1.40436542+0.j,
                              1.40433097+0.j, 1.40429640+0.j]), 0.003)

    def test_obs2(self):
        """Was obsID 2 left alone?"""
        check_eq(self.records[''][2], 1.0+0.0j, 0.003)

            
def suite():
    return [setjy_test_modimage, Uranus, ScaleUranusByChan, selectobs]
