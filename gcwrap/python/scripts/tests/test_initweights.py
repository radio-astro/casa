import os
import sys
import shutil
import re
import numpy
import math

from __main__ import default
from tasks import *
from taskinit import *
import unittest

from initweights import initweights

# to rethrow exception 
import inspect
g = sys._getframe(len(inspect.stack())-1).f_globals
exception_stat = g['__rethrow_casa_exceptions'] if g.has_key('__rethrow_casa_exceptions') else False

class initweights_common(unittest.TestCase):
    """
    A base test class for initweights task
    """
    datapath = os.environ.get('CASAPATH').split()[0] + \
        '/data/regression/unittest/initweights/'
    inputms = "tsysweight_ave.ms"
    tsystable = "tsysweight_ave.tsys.cal"
    """
    Note:
    tsys spws in 'tsysweight_ave.tsys.cal' are 1,3,5,7
    spw maps in tsysweight_ave.ms are 1->1,9, 3->3,11, 5->5,13, 7->7,15
    All data and weight/sigma columns are intialized to 1.0
    It does NOT have WEIGHT_SPECTRUM and SIGMA_SPECTRUM columns at first.
    # Tsys spectra in the first Tsys measurements (the second one has +10 offset)
    # spw 1: Tsys[ichan] = 50.
    # spw 3: Tsys[ichan] = 45. + 10*ichan/nchan
    # spw 5: Tsys[ichan] = 50. + 10*(ichan/nchan)^2
    # spw 7: Tsys[ichan] = 60.
    """
    # the list of tables to copy and clear up in each test.
    templist = [inputms, tsystable]
    spwmap = [0, 1, 2, 3, 4, 5, 6, 7, 8, 1, 10, 3, 12, 5, 14, 7]
    nchan = -1 # need to override this
    verbose = False

    def setUp(self):
        default(initweights)
        for name in self.templist:
            # remove old ones (if exists)
            if (os.path.exists(name)):
                shutil.rmtree(name)
            # copy a new ones
            shutil.copytree(self.datapath+name, name)
    
    def tearDown(self):
        # remove list of files
        for name in self.templist:
           if (os.path.exists(name)):
               shutil.rmtree(name)

    def _run_local_tests(self, *args, **kwargs):
        """Additional tests to run in the class (the default is nothing)"""
        pass

    def _get_interpolated_wtsp(self, *args, **kwargs):
        """
        A method to get interpolated weight spectrumn for a given
        spw and row id
        Should be implemented in child class
        """
        raise NotImplementedError

    def _check_file(self, name):
        """Tests the existance of a file/directory"""
        self.assertTrue(os.path.exists(name), "Could not find table %s." % name)

    def _column_exists(self, tbname, colname):
        """Returns True if the column exists in the table"""
        self._check_file(tbname)
        tb.open(tbname)
        cols = tb.colnames()
        tb.close()
        return (colname in cols)

    def _generate_poly_array(self, nchan, coeff=[]):
        """
        Generates a polynomial array of length nchan.
        The polynomial coefficients should be given in ascending order, i.e.,
        when
        coeff = [1.0, 2.0, 3.0]
        elements of the return array will be
        polyarr[ichan] = 1.0 + 2.0*ichan + 3.0*ichan**2 (ichan=0~nchan-1)
        """
        if nchan < 0:
            raise ValueError, "nchan should be >=0"
        if len(coeff)==0:
            if nchan ==0: return []
            else: raise ValueError, "No valid coefficient given."
        polyarr = numpy.zeros(nchan)
        for iorder in range(len(coeff)):
            polyarr += coeff[iorder]*numpy.array(xrange(nchan))**iorder
        return polyarr

    def _compare_arrays(self, data, reference, atol=1.e-5, rtol=1.e-5):
        """
        Compares two arrays and returns True if they are within a tolerance.
        - checks shapes
        - checks if values are within permissive range (atol: absolute
          tolerance, rtol: relative tolerance)
        """
        if not (data.shape==reference.shape): return False
        ret=numpy.allclose(data,reference, atol=atol, rtol=rtol)
        return ret

    # def _list_to_str(idlist):
    #     """
    #     converts a list of indices to a string
    #     [0,1,2] => '0,1,2'
    #     """
    #     return str(',').join(map(str, idlist))
    def interpolation_to_list(self, interpolation):
        """
        Convert interpolation string to a list of interpolations
        in time (should be defined) and frequency (default is 'linear')
        E.g.
        'linear,cspline' -> ['linear', 'cpline']
        'nearest' -> ['nearest', 'linear' (using the default)]
        """
        interplist = interpolation.split(',')
        if len(interplist) == 0:
            interplist = ['linear', 'linear']
        elif len(interplist) == 1:
            interplist += ['linear']
        return interplist[0:2]

    def _runTest(self, wtmode, dowtsp, testspw, interpolation="", spwmap=[],
                 atol=1.e-5, rtol=1.e-5):
        """Common function to run intiweights and test results"""
        had_wtsp = self._column_exists(self.inputms, "WEIGHT_SPECTRUM")
        had_sigsp = self._column_exists(self.inputms, "SIGMA_SPECTRUM")
        initweights(vis=self.inputms,wtmode=wtmode,
                    tsystable=self.tsystable,
                    interp=interpolation,spwmap=spwmap, dowtsp=dowtsp)
        # Test existance of MS and columns
        if self.verbose: print("Test if MS exists.")
        self._check_file(self.inputms)
        # WEIGHT_SPECTRUM should exist when dowtsp=True or it pre-exists in MS
        if (dowtsp or had_wtsp) and not wtmode == "delwtsp":
            if self.verbose: print("Verify WEIGHT_SPECTRUM exists in MS after operation")
            self.assertTrue(self._column_exists(self.inputms, "WEIGHT_SPECTRUM"),
                            "WEIGHT_SPECTRUM does not exists even though dowtsp=True")
        else:
            if self.verbose: print("Verify WEIGHT_SPECTRUM does NOT exist in MS after operation")
            self.assertFalse(self._column_exists(self.inputms, "WEIGHT_SPECTRUM"),
                            "WEIGHT_SPECTRUM exists when it shouldn't")
        # test if SIGMA_SPECTRUM column exists
        # The column should exist if
        # (a) dowtsp = True AND wtmode='tsys' or 'tinttsys', OR
        # (b) SIGMA_SPECTRUM pre-exists and wtmode='delwtsp'
        # otherwise, the column will be removed from MS if exists
        sigsp_should_exists = (dowtsp and wtmode.find('tsys') > -1) or \
            (had_sigsp and wtmode=='delwtsp')
        if sigsp_should_exists:
            if self.verbose: print("Verify SIGMA_SPECTRUM exists in MS after operation")
            self.assertTrue(self._column_exists(self.inputms, "SIGMA_SPECTRUM"),
                            "SIGMA_SPECTRUM does not exist")
        else:
            if self.verbose: print("Verify SIGMA_SPECTRUM does NOT exist in MS after operation")
            self.assertFalse(self._column_exists(self.inputms, "SIGMA_SPECTRUM"),
                            "SIGMA_SPECTRUM exists when it shouldn't")
        # more tests
        self._test_results(wtmode, dowtsp, testspw, interpolation, atol, rtol)

    def _test_results(self, mode, dowtsp, spwlist, interpolation, atol, rtol):
        interplist = self.interpolation_to_list(interpolation)
        # any tests specific to class
        self._run_local_tests(mode, dowtsp, spwlist, interplist, atol, rtol)
        # common tests
        # calculate results for each time
        self._check_file(self.inputms)
        has_wtsp = self._column_exists(self.inputms, "WEIGHT_SPECTRUM")
        has_sigsp = self._column_exists(self.inputms, "SIGMA_SPECTRUM")
        if self.verbose: print("Test of values in MS after operation")
        for spw in spwlist:
            if self.verbose: print("SPW %d" % spw)
            nchan = -1
            data_found = False
            tb.open(self.inputms)
            try:
                subtb = tb.query("DATA_DESC_ID==%d" % spw)
                for irow in range(subtb.nrows()):
                    # make sure DATA columns are not touched
                    for dataname in ["FLOAT_DATA", "DATA", "CORRECTED_DATA"]:
                        if dataname in subtb.colnames():
                            data_found = True
                            if nchan<0:
                                nchan = subtb.getcell(dataname, irow).shape[1]
                                refdata = self._generate_poly_array(nchan, [1.0])
                            self._testCell(subtb.getcell(dataname, irow), refdata,
                                           rtol=rtol, atol=atol)
                    self.assertTrue(data_found, "Could not fine valid data column")
                    self.assertTrue(nchan>0, "Invalid number of channel in spw=%d" % spw)
                    # test WEIGHT, SIGMA, WEIGHT_SPECTRUM and SIGMA_SPECTRUM columns
                    refwtsp = self._get_interpolated_wtsp(mode, spw, nchan, interplist, irow, has_wtsp)
                    if has_wtsp:
                        if self.verbose: print("WEIGHT_SPECTRUM")
                        self._testCell(subtb.getcell("WEIGHT_SPECTRUM",irow),
                                       refwtsp, rtol=rtol, atol=atol)
                    if has_sigsp:
                        if self.verbose: print("SIGMA_SPECTRUM")
                        self._testCell(subtb.getcell("SIGMA_SPECTRUM",irow),
                                       self.sigmasp_from_weightsp(refwtsp),
                                       rtol=rtol, atol=atol)
                    refwt = self. weight_from_weightsp(refwtsp, takeEvenMean=False)
                    if self.verbose: print("WEIGHT")
                    self._testCell(subtb.getcell("WEIGHT",irow), refwt,
                                   rtol=rtol, atol=atol)
                    refsig = self.sigma_from_weightsp(refwtsp, takeEvenMean=False)
                    if self.verbose: print("SIGMA")
                    self._testCell(subtb.getcell("SIGMA",irow), refsig,
                                   rtol=rtol, atol=atol)
            finally:
                if subtb: subtb.close()
                tb.close()

    def _testCell(self, cell, reference, atol=1.e-5, rtol=1.e-5):
        """
        Array comparison. Duplicate reference for pol if necessary, i.e.,
        If cell.shape==reference.shape, this method comares cell and reference directly
        if cell.shape!=reference.shape
        (e.g., cell.shape=[npol, nchan] while reference.shape=[nchan]),
        this method compares cell[ipol,:] = reference[:] for all ipol=0~npol-1 assuming
        axis=0 is duplicated one
        """
        cellarr = numpy.array(cell)
        refarr = numpy.array(reference)
        if cellarr.ndim != refarr.ndim:
            # pol loop
            for ipol in range(cellarr.shape[0]):
                testarr = cellarr[ipol]
                self._testCell(testarr, refarr)
        else:
            if self.verbose and refarr.size < 130:
                print("Reference = %s" % str(refarr))
                print("Data = %s" % str(cellarr))
            self.assertEqual(cellarr.shape,refarr.shape)
            self.assertTrue(self._compare_arrays(cellarr, refarr,
                                                 rtol=rtol, atol=atol))

    def tsysweightsp_from_tsysarr(self, in_arr):
        """
        returns an array of 1./in_arr^2
        This corresponds to WEIGHT_SPECTRUM by 1./Tsys^2 in case
        input is Tsys spectrum
        """
        return 1./(numpy.array(in_arr)**2)

    def weight_from_meantsys(self, in_arr):
        """
        returns 1./mean(in_arr)^2
        This corresponds to WEIGHT by 1./Tsys^2
        in case WEIGH_SPECTRUM does not exists.
        """
        return 1./(numpy.mean(in_arr)**2)

    def weight_from_weightsp(self, in_arr, takeEvenMean=False):
        """returns median of input array"""
        return self._median(numpy.array(in_arr), takeEvenMean)

    def sigma_from_weightsp(self, in_arr, takeEvenMean=False):
        """returns a value, 1./sqrt(median(in_array))"""
        sigsp = self.sigmasp_from_weightsp(in_arr)
        return self._median(numpy.array(sigsp), takeEvenMean)

    def sigmasp_from_weightsp(self, in_arr):
        """returns an array of 1./sqrt(in_array)"""
        return 1./numpy.sqrt(numpy.array(in_arr))

    def _median(self, in_arr, takeEvenMean):
        """
        Returns a median value of an array.
        if takeEvenMean, middle two values are average if the number of elements in in_array is even.
        if not sort in_array in ascending order and returns an (n-1)/2-th element.
        """
        if takeEvenMean:
            return numpy.median(in_arr)
        else:
            return numpy.sort(in_arr, axis=None)[(in_arr.size-1)/2]


class initweights_tsys_base(initweights_common):
    """
    Tests of mode ='tsys' and 'tinittsys' without spw mapping.
    The class tests various interpolations.
    The tests include proper generation of SIGMA_SPECTRUM when dowtsp=T.
    """
    # Polynomial coefficients of interpolated Tsys spectra in each Tsys spw, 1,3,5&7.
    # The time stamp in MS is identical to the first Tsys measurements in caltable.
    # Interpolation is not effective in this case.
    tsys_funcs = {1: [50.0],
                  3: [45.0, 0.07874015748031496],
                  5: [50.0, 0.0, 0.00062000124000248],
                  7: [60.0],
                  9: None, 11: None, 13: None, 15: None} #None means untouched spw
    exposure = 34.56 #sec (common)
    chw = 1.5625e7   #Hz (common)

    def _get_interpolated_wtsp(self, mode, spw, nchan, interplist, irow, dowtsp):
        if mode=='tsys':
            factor = self.chw
        elif mode=='tinttsys':
            factor = self.chw*self.exposure
        else:
            raise ValueError, "invalid mode for tests"
        if self.tsys_funcs[spw]:
            tsys = self._generate_poly_array(nchan, self.tsys_funcs[spw])
            if not dowtsp:
                #use mean Tsys instead of spectral Tsys
                meantsys = numpy.mean(tsys)
                tsys = self._generate_poly_array(nchan, [meantsys])
            wtsp = self.tsysweightsp_from_tsysarr(tsys)
            wtsp *= factor
        else: #None
            wtsp = self._generate_poly_array(nchan, [1.0])
        return  wtsp

    def testTsysNN(self):
        """Test wtmode='tsys', interp='nearest,nearest'"""
        self._runTest('tsys', False, self.tsys_funcs.keys(), 'nearest,nearest')

    def testTsysLL(self):
        """Test wtmode='tsys', interp='linear,linear'"""
        self._runTest('tsys', False, self.tsys_funcs.keys(), 'linear,linear')

    def testTsysLC(self):
        """Test wtmode='tsys', interp='linear,cspline'"""
        self._runTest('tsys', False, self.tsys_funcs.keys(), 'linear,cspline')
        
    def testTinttsysNN(self):
        """Test wtmode='tinttsys', interp='nearest,nearest'"""
        self._runTest('tinttsys', False, self.tsys_funcs.keys(), 'nearest,nearest')

    def testTinttsysLL(self):
        """Test wtmode='tinttsys', interp='linear,linear'"""
        self._runTest('tinttsys', False, self.tsys_funcs.keys(), 'nearest,nearest')

    def testTinttsysLC(self):
        """Test wtmode='tinttsys', interp='linear,cspline'"""
        self._runTest('tinttsys', False, self.tsys_funcs.keys(), 'nearest,nearest')
        
    def testTsysNNSp(self):
        """Test wtmode='tsys', interp='nearest,nearest', dowtsp=True"""
        self._runTest('tsys', True, self.tsys_funcs.keys(), 'nearest,nearest')

    def testTsysLLSp(self):
        """Test wtmode='tsys', interp='linear,linear', dowtsp=True"""
        self._runTest('tsys', True, self.tsys_funcs.keys(), 'linear,linear')

    def testTsysLCSp(self):
        """Test wtmode='tsys', interp='linear,cspline', dowtsp=True"""
        self._runTest('tsys', True, self.tsys_funcs.keys(), 'linear,cspline')
        
    def testTinttsysNNSp(self):
        """Test wtmode='tinttsys', interp='nearest,nearest', dowtsp=True"""
        self._runTest('tinttsys', True, self.tsys_funcs.keys(), 'nearest,nearest')

    def testTinttsysLLSp(self):
        """Test wtmode='tinttsys', interp='linear,linear', dowtsp=True"""
        self._runTest('tinttsys', True, self.tsys_funcs.keys(), 'nearest,nearest')

    def testTinttsysLCSp(self):
        """Test wtmode='tinttsys', interp='linear,cspline', dowtsp=True"""
        self._runTest('tinttsys', True, self.tsys_funcs.keys(), 'nearest,nearest')
        
class initweights_tsys_map(initweights_common):
    """
    Tests of mode ='tsys' and 'tinittsys' with spw mapping.
    The class tests various interpolations.
    The tests include proper generation of SIGMA_SPECTRUM when dowtsp=T.
    """
    # Polynomial coefficients of interpolated Tsys spectra in each rows of spw
    # obs sequence: Tsys1 - Science1 - Tsys2 - Science2
    # interpolation for Science2 is always nearest.
    tsys_nearest = {1: [[50.0]],
                    3: [[45.0, 0.07874015748031496]],
                    5: [[50.0, 0.0, 0.00062000124000248]],
                    7: [[60.0]],
                    9: [[50.0], [60.0]],
                    11: [[45.0, 2.451581269919098e-3], [55.0, 2.451581269919098e-3]],
                    13: [[50.0, 0.0, 6.010250723018137e-07], [60.0, 0.0, 6.010250723018137e-07]],
                    15: [[60.0], [70.0]]}
    tsys_linear = {1: [[50.0]],
                   3: [[45.0, 0.07874015748031496]],
                   5: [[50.0, 0.0, 0.00062000124000248]],
                   7: [[60.0]],
                   9: [[55.0],[60.0]],
                   11: [[50.0, 2.451581269919098e-3], [55.0, 2.451581269919098e-3]],
                   13: [[55.0, 0.0, 6.010250723018137e-07], [60.0, 0.0, 6.010250723018137e-07]],
                   15: [[65.0],[70.0]]}
    exposure = {'tsys': [34.56], 'sci': [424.368, 160.272]} 
    chw = {'tsys': 1.5625e7, 'sci': 486486.}

    def _get_interpolated_wtsp(self, mode, spw, nchan, interplist, irow, dowtsp):
        if interplist[0].startswith('near'):
            tsys_funcs = self.tsys_nearest
        elif interplist[0].startswith('lin'):
            tsys_funcs = self.tsys_linear
        else:
            raise ValueError, "got unexpected time interpolation"
        if spw not in tsys_funcs.keys():
            raise ValueError, "Testing unexpected spw %d" % spw
        spwintent = 'tsys' if spw in [1,3,5,7] else 'sci'
        if mode=='tsys':
            factor = self.chw[spwintent]
        elif mode=='tinttsys':
            factor = self.chw[spwintent]*self.exposure[spwintent][irow]
        else:
            raise ValueError, "invalid mode for tests"
        if tsys_funcs[spw]:
            tsys = self._generate_poly_array(nchan, tsys_funcs[spw][irow])
            if not dowtsp:
                #use mean Tsys instead of spectral Tsys
                meantsys = numpy.mean(tsys)
                tsys = self._generate_poly_array(nchan, [meantsys])
            wtsp = self.tsysweightsp_from_tsysarr(tsys)
            wtsp *= factor
        else: #None
            wtsp = self._generate_poly_array(nchan, [1.0])
        return  wtsp

    def testTsysMapNN(self):
        """Test spwmap wtmode='tsys', interp='nearest,nearest'"""
        self._runTest('tsys', False, [1,3,5,7,9,15], 'nearest,nearest',self.spwmap)

    def testTsysMapLL(self):
        """Test spwmap wtmode='tsys', interp='linear,linear'"""
        self._runTest('tsys', False, [1,3,5,7,9,11,15], 'linear,linear',self.spwmap)

    def testTsysMapLC(self):
        """Test spwmap wtmode='tsys', interp='linear,cspline'"""
        self._runTest('tsys', False, [1,3,5,7,9,11,13,15], 'linear,cspline',self.spwmap)
        
    def testTinttsysMapNN(self):
        """Test spwmap wtmode='tinttsys', interp='nearest,nearest'"""
        self._runTest('tinttsys', False, [1,3,5,7,15], 'nearest,nearest',self.spwmap)

    def testTinttsysMapLL(self):
        """Test spwmap wtmode='tinttsys', interp='linear,linear'"""
        self._runTest('tinttsys', False, [1,3,5,7,9,11,15], 'linear,linear',self.spwmap)

    def testTinttsysMapLC(self):
        """Test spwmap wtmode='tinttsys', interp='linear,cspline'"""
        self._runTest('tinttsys', False, [1,3,5,7,9,11,13,15], 'linear,cspline',self.spwmap)
        
    def testTsysMapNNSp(self):
        """Test spwmap wtmode='tsys', interp='nearest,nearest', dowtsp=True"""
        self._runTest('tsys', True, [1,3,5,7,9,15], 'nearest,nearest',self.spwmap)

    def testTsysMapLLSp(self):
        """Test spwmap wtmode='tsys', interp='linear,linear', dowtsp=True"""
        self._runTest('tsys', True, [1,3,5,7,9,11,15], 'linear,linear',self.spwmap)

    def testTsysMapLCSp(self):
        """Test spwmap wtmode='tsys', interp='linear,cspline', dowtsp=True"""
        self._runTest('tsys', True, [1,3,5,7,9,11,13,15], 'linear,cspline',self.spwmap)
        
    def testTinttsysMapNNSp(self):
        """Test spwmap wtmode='tinttsys', interp='nearest,nearest', dowtsp=True"""
        self._runTest('tinttsys', True, [1,3,5,7,9,15], 'nearest,nearest',self.spwmap)

    def testTinttsysMapLLSp(self):
        """Test spwmap wtmode='tinttsys', interp='linear,linear', dowtsp=True"""
        self._runTest('tinttsys', True, [1,3,5,7,9,11,15], 'linear,linear',self.spwmap)

    def testTinttsysMapLCSp(self):
        """Test spwmap wtmode='tinttsys', interp='linear,cspline', dowtsp=True"""
        self._runTest('tinttsys', True, [1,3,5,7,9,11,13,15], 'linear,cspline',self.spwmap)

class initweights_base(initweights_common):
    """
    Tests of mode ='nyq', 'ones', and 'sigma'
    The class tests dowtsp=T/F
    NOTE the input MS has inconsistent values in WEIGHT and SIGMA only for testing purpose.
    SIGMA=2.0, WEIGHT=9.0
    The MS does NOT have WEIGHT_SPECTRUM but HAS SIGMA_SPECTRUM, or CORRECTED_DATA columns at first.
    The class tests proper removal of SIGMA_SPECTRUM in the modes.
    """
    inputms = 'weight_inconsistent.ms'
    templist = [ inputms ]
    exposure = {0: [34.56], 1: [424.368, 160.272]} 
    chw = {0: 1.5625e7, 1: 486486.}
    valid_spw = [0,1]
    sigma = 2.0
    weight = 9.0

    def _get_interpolated_wtsp(self, mode, spw, nchan, interplist, irow, dowtsp):
        if spw not in self.valid_spw:
            raise ValueError, "Testing unexpected spw %d" % spw
        wt = -1.0
        if mode=='nyq':
            wt = self.chw[spw]*self.exposure[spw][irow]
        elif mode=='ones':
            wt = 1.0
        elif mode=='sigma':
            wt = 1./self.sigma**2
        elif mode=='weight':
            wt = self.weight
        else:
            raise ValueError, "invalid mode for tests"

        return self._generate_poly_array(nchan, [wt])

    # Just not to raise error at verification stage.
    def _make_consistent(self):
        tb.open(self.inputms,nomodify=False)
        try:
            for irow in xrange(tb.nrows()):
                tb.putcell("SIGMA", irow, 1./numpy.sqrt(tb.getcell("WEIGHT", irow)))
        except:
            raise RuntimeError, "Failed to manually make SIGMA and WEIGHT consistent."
        finally:
            tb.close()

    def testOnes(self):
        """Test wtmode='ones', dowtsp=F"""
        self._runTest('ones', False, self.valid_spw)

    def testOnesSp(self):
        """Test wtmode='ones', dowtsp=T"""
        self._runTest('ones', True, self.valid_spw)

    def testNyq(self):
        """Test wtmode='nyq', dowtsp=F"""
        self._runTest('nyq', False, self.valid_spw)

    def testNyqSp(self):
        """Test wtmode='nyq', dowtsp=T"""
        self._runTest('nyq', True, self.valid_spw)

    def testSigma(self):
        """Test wtmode='sigma', dowtsp=F"""
        self._runTest('sigma', False, self.valid_spw)

    def testSigmaSp(self):
        """Test wtmode='sigma', dowtsp=T"""
        self._runTest('sigma', True, self.valid_spw)

    # def testWeight(self):
    #     """Test wtmode='weight', dowtsp=F (shoud Fail)"""
    #     self._make_consistent()
    #     try:
    #         g['__rethrow_casa_exceptions'] = True
    #         ret = self._runTest('weight', False, self.valid_spw)
    #         #self.fail("The task should raise error")
    #     except Exception, e:
    #         pos=str(e).find("Specified wtmode requires dowtsp=T")
    #         self.assertNotEqual(pos, -1, "Unexpected exception was thorown: %s" % str(e))
    #     finally:
    #         g['__rethrow_casa_exceptions'] = exception_stat

    def testWeightSp(self):
        """Test wtmode='weight', dowtsp=T"""
        self._make_consistent()
        self._runTest('weight', True, self.valid_spw)

class initweights_delspcol(initweights_common):
    """
    Tests of mode ='delwtsp' and 'delsigsp'
    The class tests dowtsp=T/F
    The input MS has WEIGHT_SPECTRUM, SIGMA_SPECTRUM, CORRECTED_DATA columns at first.
    NOTE the values of WEIGHT[_SPECTRUM] = 4.0, while SIGMA[_SPECTRUM] = 0.5
    """
    inputms = "weight_speccols.ms"
    templist = [inputms]
    valid_spw = [0,1]

    def setUp(self):
        super(initweights_delspcol,self).setUp()
        if self.verbose: "Test WEIGHT_SPECTRUM exists in MS before operation. "
        self.assertTrue(self._column_exists(self.inputms, "WEIGHT_SPECTRUM"))
        if self.verbose: "Test SIGMA_SPECTRUM exists in MS before operation. "
        self.assertTrue(self._column_exists(self.inputms, "SIGMA_SPECTRUM"))

    def _get_interpolated_wtsp(self, mode, spw, nchan, interplist, irow, dowtsp):
        return self._generate_poly_array(nchan, [4.0])

    def testDelwtsp(self):
        """Test wtmode='delwtsp', dowtsp=F"""
        self._runTest('delwtsp', False, self.valid_spw)

    def testDelwtspSp(self):
        """Test wtmode='delwtsp', dowtsp=T"""
        self._runTest('delwtsp', True, self.valid_spw)

    def testDelsigsp(self):
        """Test wtmode='delsigsp', dowtsp=F"""
        self._runTest('delsigsp', False, self.valid_spw)

    def testDelsigspSp(self):
        """Test wtmode='delsigsp', dowtsp=T"""
        self._runTest('delsigsp', True, self.valid_spw)

#TODO: dowtsp=F and MS with "WEIGHT_SPECTRUM" (should forced to dowtsp=T)
#TODO: removal of SIGMA_SPECTRUM in wtmode='tsys', 'tinttsys' with dowtsp=F

def suite():
    return [initweights_tsys_base, initweights_tsys_map,
            initweights_base, initweights_delspcol]
