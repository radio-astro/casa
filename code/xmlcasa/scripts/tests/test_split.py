import os
import numpy
import re
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
    """Checks that val matches expval within tol."""
    if type(val) == dict:
        for k in val:
            check_eq(val[k], expval[k], tol)
    else:
        try:
            if tol:
                are_eq = abs(val - expval) < tol
            else:
                are_eq = val == expval
            if hasattr(are_eq, 'all'):
                are_eq = are_eq.all()
            if not are_eq:
                raise ValueError, '!='
        except ValueError:
            raise ValueError, "%r != %r" % (val, expval)
        except Exception, e:
            print "Error comparing", val, "to", expval
            raise e

def slurp_table(tabname):
    """
    Returns a dictionary containing the CASA table tabname.  The dictionary
    is arranged like:

    {'keywords': tb.getkeywords(),
     'cols': {colname0, {'desc': tb.getcoldesc(colname0),
                         'keywords': tb.getcolkeywords(colname0),
                         'data': tb.getcol(colname0)},
              colname1, {'desc': tb.getcoldesc(colname1),
                             'keywords': tb.getcolkeywords(colname1),
                         'data': tb.getcol(colname1)},
              ...}}
    """
    tb.open(tabname)
    retval = {'keywords': tb.getkeywords(),
              'cols': {}}
    cols = tb.colnames()
    for col in cols:
        entry = {'desc': tb.getcoldesc(col),
                 'keywords': tb.getcolkeywords(col)}
        if tb.isvarcol(col):
            entry['data'] = tb.getvarcol(col)
        else:
            entry['data'] = tb.getcol(col)
        retval['cols'][col] = entry
    tb.close()
    return retval
    
def compare_tables(tabname, exptabname, tol=None):
    """
    Raises a ValueError if the contents of tabname are not the same as those
    of exptabname to within tol.
    """
    exptabdict = slurp_table(exptabname)
    tabdict = slurp_table(tabname)

    if set(tabdict['keywords']) != set(exptabdict['keywords']):
        raise ValueError, tabname + ' and ' + exptabname + ' have different keywords'
    if set(tabdict['cols'].keys()) != set(exptabdict['cols'].keys()):
        raise ValueError, tabname + ' and ' + exptabname + ' have different columns'
    for col, tabentry in tabdict['cols'].iteritems():
        if set(tabentry['keywords']) != set(exptabdict['cols'][col]['keywords']):
            raise ValueError, tabname + ' and ' + exptabname + ' have different keywords for column ' + col

        # Check everything in the description except the data manager.
        for thingy in tabentry['desc']:
            if thingy not in ('dataManagerGroup', 'dataManagerType'):
                if tabentry['desc'][thingy] != exptabdict['cols'][col]['desc'][thingy]:
                    raise ValueError, thingy + ' differs in the descriptions of ' + col + ' in ' + tabname + ' and ' + exptabname
                
        check_eq(tabentry['data'], exptabdict['cols'][col]['data'])



class SplitChecker(unittest.TestCase):
    """
    Base class for unit test suites that do multiple tests per split run.
    """
    # Don't setup class variables here - the children would squabble over them.
    #
    # DO define a do_split(corrsel) method in each subclass to do the work and
    # record the results.  Any variables that it sets for use by the tests must
    # be class variables, i.e. prefixed by self.__class__..  The tests,
    # however, will refer to them as instance variables.  Example: usually
    # do_split() will set self.__class__.records, and the tests will use it as
    # self.records.  This quirk is a result of unittest.TestCase's preference
    # for starting from scratch, and tearing down afterwards, for each test.
    # That's exactly what SplitChecker is avoiding.
    
    def setUp(self):
        if self.need_to_initialize:
            self.initialize()

    def tearDown(self):
        """
        Only check_subtables() needs MSes, and it should take care of rming them.
        """
        pass
    
    def initialize(self):
        # The realization that need_to_initialize needs to be
        # a class variable more or less came from
        # http://www.gossamer-threads.com/lists/python/dev/776699
        self.__class__.need_to_initialize = False
                
        inpms = self.inpms
    
        if not os.path.exists(inpms):
            # Copying is technically unnecessary for split,
            # but self.inpms is shared by other tests, so making
            # it readonly might break them.
            shutil.copytree(datapath + inpms, inpms)

        if not os.path.exists(inpms):
            raise EnvironmentError, "Missing input MS: " + datapath + inpms

        for corrsel in self.corrsels:
            self.res = self.do_split(corrsel)

    def check_subtables(self, corrsel, expected):
        oms = self.records[corrsel]['ms']
        assert listshapes(mspat=oms)[oms] == set(expected)
        shutil.rmtree(oms)


class split_test_tav(SplitChecker):
    need_to_initialize = True
    inpms = '0420+417/0420+417.ms'
    corrsels = ['', 'rr, ll', 'rl, lr', 'rr', 'll']
    records = {}
    
    def do_split(self, corrsel):
        outms = 'tav' + re.sub(',\s*', '', corrsel) + '.ms'
        record = {'ms': outms}

        shutil.rmtree(outms, ignore_errors=True)
        try:
            print "\nTime averaging", corrsel
            splitran = split(self.inpms, outms, datacolumn='data',
                             field='', spw='', width=1, antenna='',
                             timebin='20s', timerange='',
                             scan='', array='', uvrange='',
                             correlation=corrsel, async=False)
            tb.open(outms)
            record['data']   = tb.getcell('DATA', 2)
            record['weight'] = tb.getcell('WEIGHT', 5)
            record['sigma']  = tb.getcell('SIGMA', 7)
            tb.close()
        except Exception, e:
            print "Error time averaging and reading", outms
            raise e
        self.__class__.records[corrsel] = record
        return splitran

    def test_sts(self):
        """Subtables, time avg. without correlation selection"""
        self.check_subtables('', [(4, 1)])

    def test_sts_rrll(self):
        """Subtables, time avg. RR, LL"""
        self.check_subtables('rr, ll', [(2, 1)])
        
    def test_sts_rllr(self):
        """Subtables, time avg. RL, LR"""
        self.check_subtables('rl, lr', [(2, 1)])
        
    def test_sts_rr(self):
        """Subtables, time avg. RR"""
        self.check_subtables('rr', [(1, 1)])
        
    def test_sts_ll(self):
        """Subtables, time avg. LL"""
        self.check_subtables('ll', [(1, 1)])

    ## # split does not yet return a success value, and exceptions
    ## # are captured.
    ## # But at least on June 10 it correctly exited with an error
    ## # msg for correlation = 'rr, rl, ll'.
    ## def test_abort_on_rrrlll(self):
    ##     """
    ##     Cannot slice out RR, RL, LL
    ##     """
    ##     self.assertFalse(self.doSplit('rr, rl, ll'))
        
    def test_data(self):
        """DATA[2],   time avg. without correlation selection"""
        check_eq(self.records['']['data'],
                 numpy.array([[ 0.14428490-0.03145669j],
                              [-0.00379944+0.00710297j],
                              [-0.00381106-0.00066403j],
                              [ 0.14404297-0.04763794j]]),
                 0.0001)
        
    def test_data_rrll(self):
        """DATA[2],   time avg. RR, LL"""
        check_eq(self.records['rr, ll']['data'],
                 numpy.array([[ 0.14428490-0.03145669j],
                              [ 0.14404297-0.04763794j]]),
                 0.0001)

    def test_data_rllr(self):
        """DATA[2],   time avg. RL, LR"""
        check_eq(self.records['rl, lr']['data'],
                 numpy.array([[-0.00379944+0.00710297j],
                              [-0.00381106-0.00066403j]]),
                 0.0001)
        
    def test_data_rr(self):
        """DATA[2],   time avg. RR"""
        check_eq(self.records['rr']['data'],
                 numpy.array([[ 0.14428490-0.03145669j]]),
                 0.0001)

    def test_data_ll(self):
        """DATA[2],   time avg. LL"""
        check_eq(self.records['ll']['data'],
                 numpy.array([[ 0.14404297-0.04763794j]]),
                 0.0001)

    def test_wt(self):
        """WEIGHT[5], time avg. without correlation selection"""
        check_eq(self.records['']['weight'],
                 numpy.array([143596.34375, 410221.34375,
                              122627.1640625, 349320.625]),
                 1.0)

    def test_wt_rrll(self):
        """WEIGHT[5], time avg. RR, LL"""
        check_eq(self.records['rr, ll']['weight'],
                 numpy.array([143596.34375, 349320.625]),
                 1.0)

    def test_wt_rllr(self):
        """WEIGHT[5], time avg. RL, LR"""
        check_eq(self.records['rl, lr']['weight'],
                 numpy.array([410221.34375, 122627.1640625]),
                 1.0)

    def test_wt_rr(self):
        """WEIGHT[5], time avg. RR"""
        check_eq(self.records['rr']['weight'],
                 numpy.array([143596.34375]),
                 1.0)

    def test_wt_ll(self):
        """WEIGHT[5], time avg. LL"""
        check_eq(self.records['ll']['weight'],
                 numpy.array([349320.625]),
                 1.0)

    def test_sigma(self):
        """SIGMA[7],  time avg. without correlation selection"""
        check_eq(self.records['']['sigma'],
                 numpy.array([0.00168478, 0.00179394,
                              0.00182574, 0.00194404]),
                 0.0001)
        
    def test_sigma_rrll(self):
        """SIGMA[7],  time avg. RR, LL"""
        check_eq(self.records['rr, ll']['sigma'],
                 numpy.array([0.00168478, 0.00194404]),
                 0.0001)
        
    def test_sigma_rllr(self):
        """SIGMA[7],  time avg. RL, LR"""
        check_eq(self.records['rl, lr']['sigma'],
                 numpy.array([0.00179394, 0.00182574]),
                 0.0001)
        
    def test_sigma_rr(self):
        """SIGMA[7],  time avg. RR"""
        check_eq(self.records['rr']['sigma'],
                 numpy.array([0.00168478]),
                 0.0001)
        
    def test_sigma_ll(self):
        """SIGMA[7],  time avg. LL"""
        check_eq(self.records['ll']['sigma'],
                 numpy.array([0.00194404]),
                 0.0001)


class split_test_cav(SplitChecker):
    need_to_initialize = True
    corrsels = ['', 'rr', 'll']
    inpms = 'viewertest/ctb80-vsm.ms'
    records = {}
    
    def do_split(self, corrsel):
        outms = 'cav' + re.sub(',\s*', '', corrsel) + '.ms'
        record = {'ms': outms}

        shutil.rmtree(outms, ignore_errors=True)
        try:
            print "\nChannel averaging", corrsel
            splitran = split(self.inpms, outms, datacolumn='data',
                             field='', spw='0:5~16', width=3,
                             antenna='',
                             timebin='', timerange='',
                             scan='', array='', uvrange='',
                             correlation=corrsel, async=False)
            tb.open(outms)
            record['data']   = tb.getcell('DATA', 2)
            record['weight'] = tb.getcell('WEIGHT', 5)
            record['sigma']  = tb.getcell('SIGMA', 7)
            tb.close()
        except Exception, e:
            print "Error channel averaging and reading", outms
            raise e
        self.records[corrsel] = record
        return splitran

    def test_sts(self):
        """Subtables, chan avg. without correlation selection"""
        self.check_subtables('', [(2, 4)])

    def test_sts_rr(self):
        """Subtables, chan avg. RR"""
        self.check_subtables('rr', [(1, 4)])
        
    def test_sts_ll(self):
        """Subtables, chan avg. LL"""
        self.check_subtables('ll', [(1, 4)])

    def test_data(self):
        """DATA[2],   chan avg. without correlation selection"""
        check_eq(self.records['']['data'],
                 numpy.array([[16.795681-42.226387j, 20.5655-44.9874j,
                               26.801544-49.595020j, 21.4770-52.0462j],
                              [-2.919122-38.427235j, 13.3042-50.8492j,
                                4.483857-43.986446j, 10.1733-19.4007j]]),
                 0.0005)
        
    def test_data_rr(self):
        """DATA[2],   chan avg. RR"""
        check_eq(self.records['rr']['data'],
                 numpy.array([[16.79568-42.226387j, 20.5655-44.9874j,
                               26.80154-49.595020j, 21.4770-52.0462j]]),
                 0.0001)

    def test_data_ll(self):
        """DATA[2],   chan avg. LL"""
        check_eq(self.records['ll']['data'],
                 numpy.array([[-2.919122-38.427235j, 13.3042-50.8492j,
                                4.483857-43.986446j, 10.1733-19.4007j]]),
                 0.0001)

    def test_wt(self):
        """WEIGHT[5], chan avg. without correlation selection"""
        check_eq(self.records['']['weight'],
                 numpy.array([0.38709676, 0.38709676]),
                 0.001)

    def test_wt_rr(self):
        """WEIGHT[5], chan avg. RR"""
        check_eq(self.records['rr']['weight'],
                 numpy.array([0.38709676]),
                 0.001)

    def test_wt_ll(self):
        """WEIGHT[5], chan avg. LL"""
        check_eq(self.records['ll']['weight'],
                 numpy.array([0.38709676]),
                 0.001)

    def test_sigma(self):
        """SIGMA[7],  chan avg. without correlation selection"""
        check_eq(self.records['']['sigma'],
                 numpy.array([1.60727513, 1.60727513]),
                 0.0001)
        
    def test_sigma_rr(self):
        """SIGMA[7],  chan avg. RR"""
        check_eq(self.records['rr']['sigma'],
                 numpy.array([1.60727513]),
                 0.0001)
        
    def test_sigma_ll(self):
        """SIGMA[7],  chan avg. LL"""
        check_eq(self.records['ll']['sigma'],
                 numpy.array([1.60727513]),
                 0.0001)

class split_test_cst(unittest.TestCase):
    """
    The main thing here is to not segfault even when the SOURCE table
    contains nonsense.
    """    
    inpms = datapath + 'unittest/split/crazySourceTable.ms' # read-only
    outms = 'filteredsrctab.ms'

    def setUp(self):
        shutil.rmtree(self.outms, ignore_errors=True)
        try:
            print "\nSplitting", self.inpms
            splitran = split(self.inpms, self.outms, datacolumn='data',
                             field='', spw='', width=1,
                             antenna='',
                             timebin='', timerange='',
                             scan='', array='', uvrange='',
                             correlation='', async=False)
        except Exception, e:
            print "Error splitting to", self.outms
            raise e

    def tearDown(self):
        shutil.rmtree(self.outms, ignore_errors=True)
        
    def test_cst(self):
        """
        Check that only the good part of a SOURCE subtable with some nonsense made it through
        """
        tb.open(self.outms + '/SOURCE')
        srcids = tb.getcol('SOURCE_ID')
        tb.close()
        check_eq(srcids, numpy.array([0, 0, 0, 0, 1, 1, 1,
                                      1, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0,
                                      0, 0, 0, 0, 0, 0, 0]))
        

class split_test_state(unittest.TestCase):
    """
    Checks a simple copy of the STATE subtable.
    """
    # rename and make readonly when plotxy goes away.
    inpms = 'plotxy/uid___X1eb_Xa30_X1.ms'
    
    outms = 'musthavestate.ms'

    def setUp(self):
        try:
            shutil.rmtree(self.outms, ignore_errors=True)
        
            if not os.path.exists(self.inpms):
                # Copying is technically unnecessary for split,
                # but self.inpms is shared by other tests, so making
                # it readonly might break them.
                shutil.copytree(datapath + self.inpms, self.inpms)
                
                print "\n\tSplitting", self.inpms
                print "\t  ...ignore any warnings about time-varying feeds..."
            splitran = split(self.inpms, self.outms, datacolumn='corrected',
                             field='', spw=[0, 1], width=1,
                             antenna='',
                             timebin='0s', timerange='',
                             scan='', array='', uvrange='',
                             correlation='', async=False)
        except Exception, e:
            print "Error splitting", self.inpms, "to", self.outms
            raise e

    def tearDown(self):
        shutil.rmtree(self.inpms, ignore_errors=True)
        shutil.rmtree(self.outms, ignore_errors=True)

    def test_state(self):
        """
        Was the STATE subtable copied?
        """
        compare_tables(self.outms + '/STATE', self.inpms + '/STATE')

class split_test_genericsubtables(unittest.TestCase):
    """
    Check copying generic subtables
    """
    inpms = datapath + 'unittest/split/2554.ms'
    outms = 'musthavegenericsubtables.ms'

    def setUp(self):
        try:
            shutil.rmtree(self.outms, ignore_errors=True)

            #print "\n\tSplitting", self.inpms
            splitran = split(self.inpms, self.outms, datacolumn='data',
                             field='', spw='0', width=1,
                             antenna='',
                             timebin='0s', timerange='',
                             scan='', array='', uvrange='',
                             correlation='', async=False)
        except Exception, e:
            print "Error splitting", self.inpms, "to", self.outms
            raise e

    def tearDown(self):
        shutil.rmtree(self.outms, ignore_errors=True)

    def test_genericsubtables(self):
        """
        Can we copy generic subtables?
        """
        tb.open(self.outms)
        kws = tb.keywordnames()
        tb.close()
        # Just check a few, and order does not matter.  Include both "generic"
        # and "standard" (mandatory and optional) subtables.
        for subtab in ('ASDM_CALWVR', 'ASDM_CALDELAY', 'DATA_DESCRIPTION',
                       'POINTING', 'SYSCAL'):
            assert subtab in kws
 
class split_test_singchan(unittest.TestCase):
    """
    Check selecting a single channel with the spw:chan syntax
    """
    # rename and make readonly when plotxy goes away.
    inpms = 'viewertest/ctb80-vsm.ms'

    outms = 'musthavesingchan.ms'

    def setUp(self):
        try:
            shutil.rmtree(self.outms, ignore_errors=True)

            if not os.path.exists(self.inpms):
                # Copying is technically unnecessary for split,
                # but self.inpms is shared by other tests, so making
                # it readonly might break them.
                shutil.copytree(datapath + self.inpms, self.inpms)

            print "\n\tSplitting", self.inpms
            splitran = split(self.inpms, self.outms, datacolumn='data',
                             field='', spw='0:25', width=1,
                             antenna='',
                             timebin='0s', timerange='',
                             scan='', array='', uvrange='',
                             correlation='', async=False)
        except Exception, e:
            print "Error splitting", self.inpms, "to", self.outms
            raise e

    def tearDown(self):
        shutil.rmtree(self.inpms, ignore_errors=True)
        shutil.rmtree(self.outms, ignore_errors=True)

    def test_singchan(self):
        """
        Did we get the right channel?
        """
        tb.open(self.inpms)
        data_orig = tb.getcell('DATA', 3)
        tb.close()
        tb.open(self.outms)
        data_sp = tb.getcell('DATA', 3)
        tb.close()
        
        # For all correlations, compare output channel 0 to input channel 25.
        check_eq(data_sp[:,0], data_orig[:,25], 0.0001)

class split_test_unorderedpolspw(SplitChecker):
    """
    Check spw selection from a tricky MS.
    """
    need_to_initialize = True
    inpms = 'unittest/split/unordered_polspw.ms'
    corrsels = ['']
    records = {}

    def do_split(self, corrsel):
        outms = 'pss' + re.sub(',\s*', '', corrsel) + '.ms'
        record = {'ms': outms}

        shutil.rmtree(outms, ignore_errors=True)
        try:
            print "\nSelecting spws 1, 3, and 5."
            splitran = split(self.inpms, outms, datacolumn='data',
                             field='', spw='1,3,5', width=1, antenna='',
                             timebin='0s', timerange='18:32:40~18:33:20',
                             scan='', array='', uvrange='',
                             correlation=corrsel, async=False)
            tb.open(outms)
            record['data'] = tb.getcell('DATA', 2)
            tb.close()
        except Exception, e:
            print "Error selecting spws 1, 3, and 5 from", self.inpms
            raise e
        self.__class__.records[corrsel] = record
        return splitran

    def test_datashape(self):
        """Data shape"""
        assert self.records['']['data'].shape == (2, 128)

    def test_subtables(self):
        """DATA_DESCRIPTION, SPECTRAL_WINDOW, and POLARIZATION shapes"""
        self.check_subtables('', [(2, 128)])

class split_test_tav_then_cvel(SplitChecker):
    need_to_initialize = True
    # doppler01fine-01.ms was altered by
    # make_labelled_ms(vis, vis,
    #                  {'SCAN_NUMBER': 1.0,
    #                   'DATA_DESC_ID': 0.01,
    #                   'chan': complex(0, 1),
    #                   'STATE_ID': complex(0, 0.1),
    #                   'time': 100.0}, ow=True)
    inpms = 'unittest/split/doppler01fine-01.ms'
    corrsels = ['']
    records = {}
    
    def do_split(self, corrsel):
        tavms = 'doppler01fine-01-10s.ms'
        cvms  = 'doppler01fine-01-10s-cvel.ms'
        record = {'tavms': tavms, 'cvms': cvms,
                  'tav': {},      'cv': False}
        self.__class__._cvel_err = False

        shutil.rmtree(tavms, ignore_errors=True)
        shutil.rmtree(cvms, ignore_errors=True)
        try:
            print "\nTime averaging", corrsel
            splitran = split(self.inpms, tavms, datacolumn='data',
                             field='', spw='', width=1, antenna='',
                             timebin='10s', timerange='',
                             scan='', array='', uvrange='',
                             correlation=corrsel, async=False)
            tb.open(tavms)
            for c in ['DATA', 'WEIGHT', 'INTERVAL', 'SCAN_NUMBER', 'STATE_ID']:
                record['tav'][c] = {}
                for r in [0, 4, 5, 6, 7, 90, 91]:
                    record['tav'][c][r] = tb.getcell(c, r)
            tb.close()
        except Exception, e:
            print "Error time averaging and reading", tavms
            raise e
        try:
            print "Running cvel"
            cvelran = cvel(tavms, cvms, passall=False, field='', spw='0~8',
                           selectdata=True, timerange='', scan="", array="",
                           mode="velocity", nchan=-1, start="-4km/s",
                           width="-1.28km/s", interpolation="linear",
                           phasecenter="", restfreq="6035.092MHz",
                           outframe="lsrk", veltype="radio", hanning=False)
        except Exception, e:
            print "Error running cvel:", e
            # Do NOT raise e: that would prevent the tav tests from running.
            # Use test_cv() to register a cvel error.
            self.__class__._cvel_err = True
            
        self.__class__.records = record
        return splitran

    def test_tav_data(self):
        """Time averaged DATA"""
        check_eq(self.records['tav']['DATA'],
                 {0: numpy.array([[ 455.+0.10000001j,  455.+1.10000014j,
                                    455.+2.10000014j,  455.+3.10000014j],
                                  [ 455.+0.10000001j,  455.+1.10000014j,
                                    455.+2.10000014j,  455.+3.10000014j]]),
                  4: numpy.array([[4455.+0.10000001j, 4455.+1.10000014j,
                                   4455.+2.10000014j, 4455.+3.10000014j],
                                  [4455.+0.10000001j, 4455.+1.10000014j,
                                   4455.+2.10000014j, 4455.+3.10000014j]]),
                  5: numpy.array([[5405.+0.10000001j, 5405.+1.10000002j,
                                   5405.+2.10000014j, 5405.+3.10000014j],
                                  [5405.+0.10000001j, 5405.+1.10000002j,
                                   5405.+2.10000014j, 5405.+3.10000014j]]),
                  6: numpy.array([[5906.+0.1j, 5906.+1.10000002j,
                                   5906.+2.0999999j, 5906.+3.0999999j],
                                  [5906.+0.1j, 5906.+1.10000002j,
                                   5906.+2.0999999j, 5906.+3.0999999j]]),
                  7: numpy.array([[6456.+0.10000001j, 6456.+1.10000014j,
                                   6456.+2.10000014j, 6456.+3.10000014j],
                                  [6456.+0.10000001j, 6456.+1.10000014j,
                                   6456.+2.10000014j, 6456.+3.10000014j]]),
                 90: numpy.array([[89256.+0.1j, 89256.+1.10000002j,
                                   89256.+2.10000014j, 89256.+3.10000014j],
                                  [89256.+0.1j, 89256.+1.10000002j,
                                   89256.+2.10000014j, 89256.+3.10000014j]]),
                 91: numpy.array([[162467.015625+0.j, 162467.015625+1.j,
                                   162467.015625+2.j, 162467.015625+3.j],
                                  [162467.015625+0.j, 162467.015625+1.j,
                                   162467.015625+2.j, 162467.015625+3.j]])},
                 0.0001)

    def test_tav_wt(self):
        """Time averaged WEIGHT"""
        check_eq(self.records['tav']['WEIGHT'],
                 {0: numpy.array([ 10.,  10.]),
                  4: numpy.array([ 10.,  10.]),
                  5: numpy.array([ 9.,  9.]),
                  6: numpy.array([ 1.,  1.]),
                  7: numpy.array([ 10.,  10.]),
                  90: numpy.array([ 6.,  6.]),
                  91: numpy.array([ 10.,  10.])}, 0.01)

    def test_tav_int(self):
        """Time averaged INTERVAL"""
        check_eq(self.records['tav']['INTERVAL'],
                 {0: 10.0, 4: 10.0, 5: 9.0, 6: 1.0, 7: 10.0, 90: 6.0, 91: 10.0},
                 0.01)

    def test_tav_state_id(self):
        """Time averaged STATE_ID"""
        check_eq(self.records['tav']['STATE_ID'],
                 {0: 1, 4: 1, 5: 1, 6: 1, 7: 1, 90: 1, 91: 0})

    def test_tav_scan(self):
        """Time averaged SCAN_NUMBER"""
        check_eq(self.records['tav']['SCAN_NUMBER'],
                 {0: 5, 4: 5, 5: 5, 6: 6, 7: 6, 90: 6, 91: 17})

    def test_cv(self):
        """cvel completed"""
        assert self._cvel_err == False and os.path.isdir(self.records['cvms'])
        

def suite():
    return [split_test_tav, split_test_cav, split_test_cst, split_test_state,
            split_test_singchan, split_test_unorderedpolspw,
            split_test_tav_then_cvel, split_test_genericsubtables]
    
