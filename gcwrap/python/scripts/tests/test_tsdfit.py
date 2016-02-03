#import os
import copy
import glob
import sys
import shutil
import numpy
from __main__ import default
from tasks import *
from taskinit import *
import unittest
#
import listing
from numpy import array

from tsdfit import tsdfit
from sdutil import tbmanager
from sdstat import sdstat


try:
    import selection_syntax
except:
    import tests.selection_syntax as selection_syntax


### Utilities for reading blparam file
class FileReader(object):
    def __init__(self, filename):
        self.__filename = filename
        self.__data = None
        self.__nline = None

    def read(self):
        if self.__data is None:
            f = open(self.__filename, 'r')
            self.__data = f.readlines()
            f.close()
            self.__nline = len(self.__data)
        return

    def nline(self):
        self.read()
        return self.__nline

    def index(self, txt, start):
        return self.__data[start:].index(txt) + 1 + start

    def getline(self, idx):
        return self.__data[idx]

class BlparamFileParser(FileReader):
    def __init__(self, blfile):
        FileReader.__init__(self, blfile)
        self.__nrow = None
        self.__coeff = None
        self.__rms = None
        self.__ctxt = 'Baseline parameters\n'
        self.__rtxt = 'Results of baseline fit\n'

    def nrow(self):
        self.read()
        if self.__nrow is None:
            return self._nrow()
        else:
            return self.__nrow

    def coeff(self):
        self.read()
        if self.__coeff is None:
            self.parseCoeff()
        return self.__coeff

    def rms(self):
        self.read()
        if self.__rms is None:
            self.parseRms()
        return self.__rms

    def _nrow(self):
        self.__nrow = 0
        for i in xrange(self.nline()):
            if self.getline(i) == self.__ctxt:
                self.__nrow += 1
        return self.__nrow

    def parse(self):
        self.read()
        self.parseCoeff()
        self.parseRms()
        return
        
    def parseCoeff(self):
        self.__coeff = []
        nrow = self.nrow()
        idx = 0
        while (len(self.__coeff) < nrow):
            try:
                idx = self.index(self.__ctxt, idx)
                coeffs = []
                while(self.getline(idx) != self.__rtxt):
                    coeff = self.__parseCoeff(idx)
                    coeffs += coeff
                    idx += 1
                self.__coeff.append(coeffs)
            except:
                break
        return

    def parseRms(self):
        self.__rms = []
        nrow = self.nrow()
        idx = 0
        while (len(self.__rms) < nrow):
            try:
                idx = self.index(self.__rtxt, idx)
                self.__rms.append(self.__parseRms(idx))
            except:
                break   
        return

    def __parseCoeff(self, idx):
        return parseCoeff(self.getline(idx))

    def __parseRms(self, idx):
        return parseRms(self.getline(idx))

def parseCoeff(txt):
    clist = txt.rstrip('\n').split(',')
    ret = []
    for c in clist:
        ret.append(float(c.split('=')[1]))
    return ret
    
def parseRms(txt):
    t = txt.lstrip().rstrip('\n')[6:]
    return float(t)

class tsdfit_unittest_base(unittest.TestCase):
    """
    Base class for tsdfit unit test
    """
    # Data path of input/output
    datapath = os.environ.get('CASAPATH').split()[0] + \
              '/data/regression/unittest/tsdfit/'
    taskname = "tsdfit"
    verboselog = False

    #complist = ['max','min','rms','median','stddev']

    blparam_order = ['row', 'pol', 'mask', 'nclip', 'cthre',
                     'uself', 'lthre', 'ledge', 'redge', 'chavg',
                     'btype', 'order', 'npiec', 'nwave']
    blparam_dic = {}
    blparam_dic['row']   = [0, 0, 1, 1, 2, 2, 3, 3]
    blparam_dic['pol']   = [0, 1, 0, 1, 0, 1, 0, 1]
    #blparam_dic['mask']  = ['0~4000;6000~8000']*3 + ['']*5
    blparam_dic['mask']  = ['500~2500;5000~7500']*8
    blparam_dic['nclip'] = [0]*8
    blparam_dic['cthre'] = ['3.']*8
    blparam_dic['uself'] = ['false']*4 + ['true'] + ['false']*3
    blparam_dic['lthre'] = ['0.']*4 + ['3.', '', '', '0.']
    blparam_dic['ledge'] = [0]*4 + [10, 50, '', 0]
    blparam_dic['redge'] = [0]*4 + [10, 50, '', 0]
    blparam_dic['chavg'] = [0]*4 + [4, '', '', 0]
    blparam_dic['btype'] = ['poly'] + ['chebyshev']*2 + ['poly', 'chebyshev', 'poly'] + ['cspline']*2
    blparam_dic['order'] = [0, 0, 1, 1, 2, 2, '', '']
    blparam_dic['npiec'] = [0]*6 + [1]*2
    blparam_dic['nwave'] = [[]]*3 + ['']*2 + [[]]*3

    ### helper functions for tests ###
    def _createBlparamFile(self, file, param_order, val, option=''):
        nspec = 8
        f = open(file, 'w')
        assert(len(param_order) == len(val.keys()))
        for key in val.keys():
            assert(len(val[key]) == nspec)
        for i in range(nspec):
            do_write = True
            s = ''
            for key in param_order:
                v = val[key][i]
                if key == 'nwave':
                    if v != '':
                        s += ','
                        s += str(v)
                else:
                    s += str(v)
                    if key != 'npiec': s += ','
            s += '\n'
            if (option == 'r2p1less') and (val['row'][i] == 2) and (val['pol'][i] == 1):
                do_write = False
            if (option == 'r2p1cout') and (val['row'][i] == 2) and (val['pol'][i] == 1):
                s = '#' + s
            if do_write:
                f.write(s)
        f.close()

    def _checkfile(self, name, fail=True):
        """
        Check if the file exists.
        name : the path and file name to test
        fail : if True, Error if the file does not exists.
               if False, return if the file exists
        """
        isthere=os.path.exists(name)
        if fail:
            self.assertTrue(isthere,
                            msg='Could not find, %s'%(name))
        else: return isthere

    def _remove(self, names):
        """
        Remove a list of files and directories from disk
        """
        for name in names:
            if os.path.exists(name):
                if os.path.isdir(name):
                    shutil.rmtree(name)
                else:
                    os.remove(name)

    def _copy(self, names, from_dir=None, dest_dir=None):
        """
        Copy a list of files and directories from a directory (from_dir) to
        another (dest_dir) in the same name.
        
        names : a list of files and directories to copy
        from_dir : a path to directory from which search and copy files
                   and directories (the default is the current path)
        to_dir   : a path to directory to which copy files and directories
                   (the default is the current path)
        NOTE: it is not allowed to specify 
        """
        # Check for paths
        if from_dir==None and dest_dir==None:
            raise ValueError, "Can not copy files to exactly the same path."
        from_path = os.path.abspath("." if from_dir==None else from_dir.rstrip("/"))
        to_path = os.path.abspath("." if dest_dir==None else dest_dir.rstrip("/"))
        if from_path == to_path:
            raise ValueError, "Can not copy files to exactly the same path."
        # Copy a list of files and directories
        for name in names:
            from_name = from_path + "/" + name
            to_name = to_path + "/" + name
            if os.path.exists(from_name):
                if os.path.isdir(from_name):
                    shutil.copytree(from_name, to_name)
                else:
                    shutil.copyfile(from_name, to_name)
                if self.verboselog:
                    casalog.post("Copying '%s' FROM %s TO %s" % (name, from_path, to_path))
            else:
                casalog.post("Could not find '%s'...skipping copy" % from_name, 'WARN')
    
    """
    def _getStats(self, filename, spw=None):
        if not spw:
            spw=''
        self._checkfile(filename)
        sd.rcParams['scantable.storage'] = 'memory'
        retstat = sdstat(filename, spw=str(spw))
        return retstat
    """

    def _getUniqList(self, val):
        """Accepts a python list and returns a list of unique values"""
        if not isinstance(val, list):
            raise Exception('_getUniqList: input value must be a list.')
        return list(set(val))

    def _getListSelection(self, val):
        """
        Converts input to a list of unique integers
        Input: Either comma separated string of IDs, an integer, or a list of values.
        Output: a list of unique integers in input arguments for string and integer input.
                In case the input is a list of values, output will be a list of unique values.
        """
        if isinstance(val, str):
            val_split = val.split(',')
            val_sel = []
            for j in range(len(val_split)):
                val_sel.append(int(val_split[j]))
        elif isinstance(val, int):
            val_sel = [val]
        elif isinstance(val, list) or isinstance(val, tuple):
            val_sel = val.copy()
        else:
            raise Exception('_getListSelection: wrong value ' + str(val) + ' for selection.')
        return self._getUniqList(val_sel)
    
    def _getListSelectedRowID(self, data_list, sel_list):
        """
        Returns IDs of data_list that contains values equal to one in
        sel_list.
        The function is used to get row IDs that corresponds to a
        selected IDs. In that use case, data_list is typically a list
        of values in a column of an MS (e.g., SCAN_NUMBER) and sel_list is
        a list of selected (scan) IDs.

        data_list : a list to test and get IDs from
        sel_list  : a list of values to look for existance in data_list
        """
        res = []
        for i in range(len(data_list)):
            if data_list[i] in sel_list:
                #idx = sel_list.index(data_list[i])
                res.append(i)
        return self._getUniqList(res)
    
    def _getEffective(self, spec, mask):
        """
        Returns an array made by selected elements in spec array.
        Only the elements in the ID range in mask are returned.

        spec : a data array
        mask : a mask list of the channel ranges to use. The format is
               [[start_idx0, end_idx0], [start_idx1, end_idx1], ...]
        """
        res = []
        for i in range(len(mask)):
            for j in range(mask[i][0], mask[i][1]):
                res.append(spec[j])
        return numpy.array(res)

    def _getStats(self, filename=None, spw=None, pol=None, colname=None, mask=None):
        """
        Returns a list of statistics dictionary of selected rows in an MS.

        filename : the name of MS
        spw      : spw ID selection (default: all spws in MS)
        pol      : pol ID selection (default: all pols in MS)
        colname  : the name of data column (default: 'FLOAT_DATA')
        mask     : a mask list of the channel ranges to use. The format is
                   [[start_idx0, end_idx0], [start_idx1, end_idx1], ...]
        
        The order of output list is in the ascending order of selected row IDs.
        The dictionary in output list has keys:
        'row' (row ID in MS), 'pol' (pol ID), 'rms', 'min', 'max', 'median',
        and 'stddev'
        """
        # Get selected row and pol IDs in MS. Also get spectrumn in the MS
        if not spw: spw = ''
        select_spw = (spw not in ['', '*'])
        if select_spw: spw_sel = self._getListSelection(spw)
        if not pol: pol = ''
        select_pol = (pol not in ['', '*'])
        if select_pol: pol_sel = self._getListSelection(pol)
        if not colname: colname='FLOAT_DATA'
        self._checkfile(filename)
        with tbmanager(filename) as tb:
            data = tb.getcol(colname)
            ddid = tb.getcol('DATA_DESC_ID')
        with tbmanager(filename+'/DATA_DESCRIPTION') as tb:
            spwid = tb.getcol('SPECTRAL_WINDOW_ID').tolist()
        if not select_spw: spw_sel = spwid
        # get the selected DD IDs from selected SPW IDs.
        dd_sel = self._getListSelectedRowID(spwid, spw_sel)
        # get the selected row IDs from selected DD IDs
        row_sel = self._getListSelectedRowID(ddid, dd_sel)
        if not select_spw: row_sel = range(len(ddid))
        if not select_pol: pol_sel = range(len(data))

        res = []
        for irow in row_sel:
            for ipol in pol_sel:
                spec = data[ipol,:,irow]
                res_elem = self._calc_stats_of_array(spec, mask=mask)
                res_elem['row'] = irow
                res_elem['pol'] = ipol
                
                res.append(res_elem)

        return res

    def _calc_stats_of_array(self, data, mask=None):
        """
        """
        if mask is not None:
            spec = self._getEffective(data, mask)
        else:
            spec = numpy.array(data)
        res_elem = {}
        res_elem['rms'] = numpy.sqrt(numpy.var(spec))
        res_elem['min'] = numpy.min(spec)
        res_elem['max'] = numpy.max(spec)
        spec_mea = numpy.mean(spec)
        res_elem['median'] = numpy.median(spec)
        res_elem['stddev'] = numpy.std(spec)
        return res_elem
        

    def _convert_statslist_to_dict(self, stat_list):
        """
        Returns a disctionary of statistics of selected rows in an MS.

        stat_list: a list of stats dictionary (e.g., return value of _getStats)

        The output dictionary is in form:
        {'max': [max0, max1, max2, ...], 'min': [min0, min1,...], ...}
        The order of elements are in ascending order of row and pol IDs pair, i.e.,
        (row0, pol0), (row0, pol1), (row1, pol0), ....
        """
        #if len(stat_list)==0: raise Exception, "No row selected in MS"
        keys=stat_list[0].keys()
        stat_dict={}
        for key in keys:
            stat_dict[key] = []
        for stat in stat_list:
            for key in keys:
                stat_dict[key].append(stat[key])
        return stat_dict

    def _compareStats(self, currstat, refstat, rtol=1.0e-2, atol=1.0e-5, complist=None):
        """
        Compare statistics results (dictionaries) and test if the values are within
        an allowed tolerance.

        currstat : the statistic values to test (either an MS name or
                   a dictionary)
        refstat  : the reference statistics values (a dictionary)
        rtol   : tolerance of relative difference
        atol   : tolerance of absolute difference
        complist : statistics to compare (default: keys in refstat)
        """
        # test if the statistics of baselined spectra are equal to
        # the reference values
        printstat = False #True
        # In case currstat is filename
        if isinstance(currstat, str) and os.path.exists(currstat):
            #print "calculating statistics from '%s'" % currstat
            currstat = self._getStats(currstat)

        self.assertTrue(isinstance(currstat,dict) and \
                        isinstance(refstat, dict),\
                        "Need to specify two dictionaries to compare")
        if complist:
            keylist = complist
        else:
            keylist = refstat.keys()
            #keylist = self.complist
        
        for key in keylist:
            self.assertTrue(currstat.has_key(key),\
                            msg="%s is not defined in the current results."\
                            % key)
            self.assertTrue(refstat.has_key(key),\
                            msg="%s is not defined in the reference data."\
                            % key)
            refval = refstat[key]
            currval = currstat[key]
            # Quantum values
            if isinstance(refval,dict):
                if refval.has_key('unit') and currval.has_key('unit'):
                    if printstat:
                        print "Comparing unit of '%s': %s (current run), %s (reference)" %\
                              (key,currval['unit'],refval['unit'])
                    self.assertEqual(refval['unit'],currval['unit'],\
                                     "The units of '%s' differs: %s (expected: %s)" % \
                                     (key, currval['unit'], refval['unit']))
                    refval = refval['value']
                    currval = currval['value']
                else:
                    raise Exception("Invalid quantum values. %s (current run) %s (reference)" %\
                                    (str(currval),str(refval)))
            currval = self._to_list(currval)
            refval = self._to_list(refval)
            if printstat:
                print "Comparing '%s': %s (current run), %s (reference)" %\
                      (key,str(currval),str(refval))
            self.assertTrue(len(currval)==len(refval),"Number of elemnets in '%s' differs." % key)
            if isinstance(refval[0],str):
                for i in range(len(currval)):
                    if isinstance(refval[i],str):
                        self.assertTrue(currval[i]==refval[i],\
                                        msg="%s[%d] differs: %s (expected: %s) " % \
                                        (key, i, str(currval[i]), str(refval[i])))
            else:
                # numpy.allclose handles almost zero case more properly.
                self.assertTrue(numpy.allclose(currval, refval, rtol=rtol, atol=atol),
                                msg="%s differs: %s" % (key, str(currval)))
            del currval, refval

            
#     def _isInAllowedRange(self, testval, refval, reltol=1.e-2):
#         """
#         Check if a test value is within permissive relative difference from refval.
#         Returns a boolean.
#         testval & refval : two numerical values to compare
#         reltol           : allowed relative difference to consider the two
#                            values to be equal. (default 0.01)
#         """
#         denom = refval
#         if refval == 0:
#             if testval == 0:
#                 return True
#             else:
#                 denom = testval
#         rdiff = (testval-refval)/denom
#         del denom,testval,refval
#         return (abs(rdiff) <= reltol)

    def _to_list(self, input):
        """
        Convert input to a list
        If input is None, this method simply returns None.
        """
        import numpy
        listtypes = (list, tuple, numpy.ndarray)
        if input == None:
            return None
        elif type(input) in listtypes:
            return list(input)
        else:
            return [input]


    def _compareBLparam(self, out, reference):
        # test if baseline parameters are equal to the reference values
        # currently comparing every lines in the files
        # TO DO: compare only "Fitter range" and "Baseline parameters"
        self._checkfile(out)
        self._checkfile(reference)
        
        blparse_out = BlparamFileParser(out)
        blparse_out.parse()
        coeffs_out = blparse_out.coeff()
        rms_out = blparse_out.rms()
        blparse_ref = BlparamFileParser(reference)
        blparse_ref.parse()
        coeffs_ref = blparse_ref.coeff()
        rms_ref = blparse_ref.rms()
        allowdiff = 0.01
        print 'Check baseline parameters:'
        for irow in xrange(len(rms_out)):
            print 'Row %s:'%(irow)
            print '   Reference rms  = %s'%(rms_ref[irow])
            print '   Calculated rms = %s'%(rms_out[irow])
            print '   Reference coeffs  = %s'%(coeffs_ref[irow])
            print '   Calculated coeffs = %s'%(coeffs_out[irow])
            r0 = rms_ref[irow]
            r1 = rms_out[irow]
            rdiff = (r1 - r0) / r0
            self.assertTrue((abs(rdiff)<allowdiff),
                            msg='row %s: rms is different'%(irow))
            c0 = coeffs_ref[irow]
            c1 = coeffs_out[irow]
            for ic in xrange(len(c1)):
                rdiff = (c1[ic] - c0[ic]) / c0[ic]
                self.assertTrue((abs(rdiff)<allowdiff),
                                msg='row %s: coefficient for order %s is different'%(irow,ic))
        print ''
#         self.assertTrue(listing.compare(out,reference),
#                         'New and reference files are different. %s != %s. '
#                         %(out,reference))


class tsdfit_basicTest(tsdfit_unittest_base):
    """
    Basic unit tests for task tsdfit. No interactive testing.

    List of tests:
    test000 --- default values for all parameters (nfit=[0] : no fitting)
    test001 --- fitting with single Gaussian (spw='0,1,2', nfit=[1])
    test002 --- fitting with double fitrange (spw='3', nfit=[1,1])
    test003 --- fitting with double Gaussian (spw='3', nfit=[2])

    Note: input data 'gaussian.ms' and 'lorentzian.ms' are based
    on a single dish regression data, 'OrionS_rawACSmod', but the
    spectra values are modified to have the following features:
      irow ipol num_lines line_id peak_height center fwhm sigma_noise
      -------------------------------------------------------
         0    0     1        0        10       4000   500    0.1
         0    1     1        0        10       5000   100    0.1
         1    0     1        0        10       3000    20    0.1
         1    1     1        0        10       2000     4    0.1
         2    0     1        0        10       4500   100    1.0
         2    1     1        0        10       5500    20    1.0
         3    0     2        0        10       2000   100    0.1
         3    0     2        1        10       6000   100    0.1
         3    1     2        0        10       3900    20    0.1
         3    1     2        1         5       4100   100    0.1
      -------------------------------------------------------

      the spectra of the row (irow=3) have two lines, while those 
      of the other rows have single line.
      spw value is identical to irow, and number of channels is 8192.
    """
    # Input and output names
    infiles = ['gaussian.ms', 'lorentzian.ms']
    outroot = tsdfit_unittest_base.taskname+'_basictest'
    blrefroot = tsdfit_unittest_base.datapath+'refblparam'
    tid = None

    answer012 = {'cent': [[4000.0], [5000.0], [3000.0], [2000.0], [4500.0], [5500.0]],
                 'peak': [[10.0], [10.0], [10.0], [10.0], [10.0], [10.0]],
                 'fwhm': [[500.0], [100.0], [20.0], [4.0], [100.0], [20.0]]
                 }
    answer3 = {'cent': [[2000.0, 6000.0], [3900.0, 4100.0]],
               'peak': [[10.0, 10.0], [10.0, 5.0]],
               'fwhm': [[100.0, 100.0], [20.0, 100.0]]
               }

    def _generateMSWithNegativeProfiles(self, infile, negative_file):
        # negative_file contains data copied from infile, but a factor of -1 is multiplied.
        shutil.copytree(infile, negative_file)
        tb.open(tablename=negative_file, nomodify=False)
        spec_orig = tb.getcol('FLOAT_DATA')
        spec = spec_orig * -1.0
        tb.putcol('FLOAT_DATA', spec)
        tb.close()

    def _generateAnswerForNegativeProfiles(self, answer):
        for i in range(len(answer['peak'])):
            for j in range(len(answer['peak'][i])):
                answer['peak'][i][j] *= -1.0
        
    def setUp(self):
        for infile in self.infiles:
            if os.path.exists(infile):
                shutil.rmtree(infile)
            shutil.copytree(self.datapath+infile, infile)
        default(tsdfit)

    def tearDown(self):
        for infile in self.infiles:
            if (os.path.exists(infile)):
                shutil.rmtree(infile)
        os.system('rm -rf '+self.outroot+'*')

    def test000(self):
        """Basic Test 000: default values for all parameters (nfit=[0] : no fitting)"""
        tid = '000'
        for infile in self.infiles:
            datacolumn = 'float_data'
            result = tsdfit(infile=infile, datacolumn=datacolumn)

            npol = 2
            with tbmanager(infile) as tb:
                nrow = tb.nrows()

            for key in result.keys():
                self.assertEqual(len(result[key]), nrow*npol, 
                                 msg="The result data has wrong data length")
                for i in range(len(result[key])):
                    if (key == "nfit"):
                        self.assertEqual(result[key][i], 0, msg="%s has wrong value."%(key))
                    else:
                        self.assertEqual(result[key][i], [], msg="%s has wrong value."%(key))

    def test001(self):
        """Basic Test 001: fitting with single line profile (spw='0,1,2', nfit=[1])"""
        tid = '001'
        for infile in self.infiles:
            datacolumn = 'float_data'
            spw = '0,1,2'
            nfit = [1]
            fitfunc = infile.split('.')[0]
            print "testing " + fitfunc + " profile..."
            result = tsdfit(infile=infile, datacolumn=datacolumn, spw=spw, nfit=nfit, fitfunc=fitfunc)
            npol = 2
            nrow = len(spw.split(','))
            answer = self.answer012
        
            for key in result.keys():
                self.assertEqual(len(result[key]), nrow*npol, 
                                 msg="The result data has wrong data length")
                for i in range(len(result[key])):
                    if (key == "nfit"):
                        self.assertEqual(result[key][i], nfit[0], msg="%s has wrong value."%(key))
                    else:
                        self.assertEqual(len(result[key][i]), nfit[0], msg="%s element has wrong length."%(key))
                        for j in range(len(result[key][i])):
                            result_lower = result[key][i][j][0] - 3.0 * result[key][i][j][1]
                            result_upper = result[key][i][j][0] + 3.0 * result[key][i][j][1]
                            #print "infile="+infile+" --- "+"lower("+str(result_lower)+") - answer("+str(answer[key][i][j])+") - upper("+str(result_upper) +")"
                            self.assertTrue(((result_lower <= answer[key][i][j]) and (answer[key][i][j] <= result_upper)),
                                            msg="row%s, comp%s result inconsistent with answer"%(i, j))

    def test002(self):
        """Basic Test 002: fitting with double fitrange (spw='3', nfit=[1,1])"""
        tid = '002'
        for infile in self.infiles:
            datacolumn = 'float_data'
            spw = '3:0~4000;4001~8191'
            nfit = [1,1]
            fitfunc = infile.split('.')[0]
            print "testing " + fitfunc + " profile..."
            result = tsdfit(infile=infile, datacolumn=datacolumn, spw=spw, nfit=nfit, fitfunc=fitfunc)
            npol = 2
            nrow = 1
            answer = self.answer3

            for key in result.keys():
                self.assertEqual(len(result[key]), nrow*npol, 
                                 msg="The result data has wrong data length")
                for i in range(len(result[key])):
                    if (key == "nfit"):
                        self.assertEqual(result[key][i], sum(nfit), msg="%s has wrong value."%(key))
                    else:
                        self.assertEqual(len(result[key][i]), sum(nfit), msg="%s element has wrong length."%(key))
                        for j in range(len(result[key][i])):
                            thres = 3.0
                            if (key == "fwhm") and (i == 1) and (j == 0): thres = 18.0
                            result_lower = result[key][i][j][0] - thres * result[key][i][j][1]
                            result_upper = result[key][i][j][0] + thres * result[key][i][j][1]
                            #print "infile="+infile+" --- "+"lower("+str(result_lower)+") - answer("+str(answer[key][i][j])+") - upper("+str(result_upper) +")"
                            self.assertTrue(((result_lower <= answer[key][i][j]) and (answer[key][i][j] <= result_upper)),
                                            msg="row%s, comp%s result inconsistent with answer"%(i, j))

    def test003(self):
        """Basic Test 003: fitting with double lines (spw='3', nfit=[2])"""
        tid = '003'
        for infile in self.infiles:
            datacolumn = 'float_data'
            spw = '3'
            nfit = [2]
            fitfunc = infile.split('.')[0]
            print "testing " + fitfunc + " profile..."
            result = tsdfit(infile=infile, datacolumn=datacolumn, spw=spw, nfit=nfit, fitfunc=fitfunc)
            npol = 2
            nrow = 1
            answer = self.answer3

            for key in result.keys():
                self.assertEqual(len(result[key]), nrow*npol, 
                                 msg="The result data has wrong data length")
                for i in range(len(result[key])):
                    if (key == "nfit"):
                        self.assertEqual(result[key][i], sum(nfit), msg="%s has wrong value."%(key))
                    else:
                        self.assertEqual(len(result[key][i]), sum(nfit), msg="%s element has wrong length."%(key))
                        for j in range(len(result[key][i])):
                            result_lower = result[key][i][j][0] - 3.0 * result[key][i][j][1]
                            result_upper = result[key][i][j][0] + 3.0 * result[key][i][j][1]
                            #print "infile="+infile+" --- "+"lower("+str(result_lower)+") - answer("+str(answer[key][i][j])+") - upper("+str(result_upper) +")"
                            self.assertTrue(((result_lower <= answer[key][i][j]) and (answer[key][i][j] <= result_upper)),
                                            msg="row%s, comp%s result inconsistent with answer"%(i, j))

    def test004(self):
        """Basic Test 004: fitting negative line profile with single line model (spw='0,1,2', nfit=[1])"""
        tid = '004'
        for infile in self.infiles:
            infile_negative = self.outroot+tid+'.negative.ms'
            self._generateMSWithNegativeProfiles(infile, infile_negative)
            datacolumn = 'float_data'
            spw = '0,1,2'
            nfit = [1]
            fitfunc = infile.split('.')[0]
            print "testing " + fitfunc + " profile..."
            result = tsdfit(infile=infile_negative, datacolumn=datacolumn, spw=spw, nfit=nfit, fitfunc=fitfunc)
            shutil.rmtree(infile_negative)
            npol = 2
            nrow = len(spw.split(','))
            answer = copy.deepcopy(self.answer012)
            self._generateAnswerForNegativeProfiles(answer)
            
            for key in result.keys():
                self.assertEqual(len(result[key]), nrow*npol, 
                                 msg="The result data has wrong data length")
                for i in range(len(result[key])):
                    if (key == "nfit"):
                        self.assertEqual(result[key][i], nfit[0], msg="%s has wrong value."%(key))
                    else:
                        self.assertEqual(len(result[key][i]), nfit[0], msg="%s element has wrong length."%(key))
                        for j in range(len(result[key][i])):
                            result_lower = result[key][i][j][0] - 3.0 * result[key][i][j][1]
                            result_upper = result[key][i][j][0] + 3.0 * result[key][i][j][1]
                            #print infile="+infile+" --- "+"lower("+str(result_lower)+") - answer("+str(answer[key][i][j])+") - upper("+str(result_upper) +")"
                            self.assertTrue(((result_lower <= answer[key][i][j]) and (answer[key][i][j] <= result_upper)),
                                            msg="row%s, comp%s result inconsistent with answer"%(i, j))

    def test005(self):
        """Basic Test 005: fitting with double fitrange (spw='3', nfit=[1,1])"""
        tid = '005'
        for infile in self.infiles:
            infile_negative = self.outroot+tid+'.negative.ms'
            self._generateMSWithNegativeProfiles(infile, infile_negative)
            datacolumn = 'float_data'
            spw = '3:0~4000;4001~8191'
            nfit = [1,1]
            fitfunc = infile.split('.')[0]
            print "testing " + fitfunc + " profile..."
            result = tsdfit(infile=infile_negative, datacolumn=datacolumn, spw=spw, nfit=nfit, fitfunc=fitfunc)
            shutil.rmtree(infile_negative)
            npol = 2
            nrow = 1
            answer = copy.deepcopy(self.answer3)
            self._generateAnswerForNegativeProfiles(answer)
            
            for key in result.keys():
                self.assertEqual(len(result[key]), nrow*npol, 
                                 msg="The result data has wrong data length")
                for i in range(len(result[key])):
                    if (key == "nfit"):
                        self.assertEqual(result[key][i], sum(nfit), msg="%s has wrong value."%(key))
                    else:
                        self.assertEqual(len(result[key][i]), sum(nfit), msg="%s element has wrong length."%(key))
                        for j in range(len(result[key][i])):
                            thres = 3.0
                            if (key == "fwhm") and (i == 1) and (j == 0): thres = 18.0
                            result_lower = result[key][i][j][0] - thres * result[key][i][j][1]
                            result_upper = result[key][i][j][0] + thres * result[key][i][j][1]
                            #print infile="+infile+" --- "+"lower("+str(result_lower)+") - answer("+str(answer[key][i][j])+") - upper("+str(result_upper) +")"
                            self.assertTrue(((result_lower <= answer[key][i][j]) and (answer[key][i][j] <= result_upper)),
                                            msg="row%s, comp%s result inconsistent with answer"%(i, j))

    def test006(self):
        """Basic Test 006: fitting with double line profiles (spw='3', nfit=[2])"""
        tid = '006'
        for infile in self.infiles:
            infile_negative = self.outroot+tid+'.negative.ms'
            self._generateMSWithNegativeProfiles(infile, infile_negative)
            datacolumn = 'float_data'
            spw = '3'
            nfit = [2]
            fitfunc = infile.split('.')[0]
            print "testing " + fitfunc + " profile..."
            result = tsdfit(infile=infile_negative, datacolumn=datacolumn, spw=spw, nfit=nfit, fitfunc=fitfunc)
            shutil.rmtree(infile_negative)
            npol = 2
            nrow = 1
            answer = copy.deepcopy(self.answer3)
            self._generateAnswerForNegativeProfiles(answer)
            
            for key in result.keys():
                self.assertEqual(len(result[key]), nrow*npol, 
                                 msg="The result data has wrong data length")
                for i in range(len(result[key])):
                    if (key == "nfit"):
                        self.assertEqual(result[key][i], sum(nfit), msg="%s has wrong value."%(key))
                    else:
                        self.assertEqual(len(result[key][i]), sum(nfit), msg="%s element has wrong length."%(key))
                        for j in range(len(result[key][i])):
                            thres = 3.0
                            if (key == "fwhm") and (i == 1) and (j == 0): thres = 18.0
                            result_lower = result[key][i][j][0] - thres * result[key][i][j][1]
                            result_upper = result[key][i][j][0] + thres * result[key][i][j][1]
                            #print infile="+infile+" --- "+"lower("+str(result_lower)+") - answer("+str(answer[key][i][j])+") - upper("+str(result_upper) +")"
                            self.assertTrue(((result_lower <= answer[key][i][j]) and (answer[key][i][j] <= result_upper)),
                                            msg="row%s, comp%s result inconsistent with answer"%(i, j))

class tsdfit_selection(unittest.TestCase):
    datapath = os.environ.get('CASAPATH').split()[0] + \
        '/data/regression/unittest/tsdfit/'
    infile = "analytic_type1.fit.ms"
    common_param = dict(infile=infile, outfile='',
                        fitfunc='gaussian',nfit=[1],fitmode='list')
    selections=dict(intent=("CALIBRATE_ATMOSPHERE#*", [1]),
                    antenna=("DA99", [1]),
                    field=("M1*", [0]),
                    spw=(">6", [1]),
                    timerange=("2013/4/28/4:13:21",[1]),
                    scan=("0~8", [0]),
                    pol=("YY", [1]))
    verbose = False
 
    reference = {'float_data': {'cent': [50, 50, 60, 60],
                                'peak': [5, 10, 15, 20],
                                'fwhm': [40, 30, 20, 10]},
                 'corrected': {'cent': [70, 70, 80, 80],
                               'peak': [25, 30, 35, 40],
                               'fwhm': [20, 30, 40, 10]} }
    templist = [infile]

    def _clearup(self):
        for name in self.templist:
            if os.path.isdir(name):
                shutil.rmtree(name)
            elif os.path.exists(name):
                os.remove(name)

    def setUp(self):
        self._clearup()
        shutil.copytree(self.datapath+self.infile, self.infile)
        default(tsdbaseline)

    def tearDown(self):
        self._clearup()

    def _get_selection_string(self, key):
        if key not in self.selections.keys():
            raise ValueError, "Invalid selection parameter %s" % key
        return {key: self.selections[key][0]}

    def _get_selected_row_and_pol(self, key):
        if key not in self.selections.keys():
            raise ValueError, "Invalid selection parameter %s" % key
        pols = [0,1]
        rows = [0,1]
        if key == 'pol':  #self.selection stores pol ids
            pols = self.selections[key][1]
        else: #self.selection stores row ids
            rows = self.selections[key][1]
        return (rows, pols)

    def _get_reference(self, row_offset, pol_offset, datacol):
        ref_list = self.reference[datacol]
        idx = row_offset*2+pol_offset
        retval = {}
        for key in ref_list.keys():
            retval[key] = ref_list[key][idx]
        if self.verbose: print("reference=%s" % str(retval))
        return retval

    def _get_gauss_param_from_return(self, params, keys):
        """returns a dictionary that stores a list of cent, fwhm, and peak """
        retval = {}
        for key in keys:
            self.assertTrue(key in params.keys(),
                            "Return value does not have key '%s'" % key)
            retval[key] = [ params[key][irow][0][0] for irow in range(len(params[key])) ]
        return retval

    def run_test(self, sel_param, datacolumn):
        inparams = self._get_selection_string(sel_param)
        inparams.update(self.common_param)
        fit_val = tsdfit(datacolumn=datacolumn, **inparams)
        self._test_result(fit_val, sel_param, datacolumn)
        
    def _test_result(self, fit_val, sel_param, dcol, atol=1.e-5, rtol=1.e-5):
        # Make sure output MS exists
        self.assertTrue(os.path.exists(self.infile), "Could not find input MS")
        tb.open(self.infile)
        nrow = tb.nrows()
        tb.close()
        # Compare fitting parameters with reference
        (rowids, polids) = self._get_selected_row_and_pol(sel_param)
        self.assertEqual(nrow, 2, "Row number changed in input MS")
        test_keys = self.reference[dcol].keys()
        # format return values and make a list of line parameters
        test_value = self._get_gauss_param_from_return(fit_val, test_keys)
        idx = 0
        for out_row in range(len(rowids)):
            in_row = rowids[out_row]
            for out_pol in range(len(polids)):
                in_pol = polids[out_pol]
                reference = self._get_reference(in_row, in_pol, dcol)
                for key in reference.keys():
                    self.assertTrue(numpy.allclose([test_value[key][idx]],
                                                   [reference[key]],
                                                   atol=atol, rtol=rtol),
                                    "Fitting result '%s' in row=%d, pol=%d differs: %f (expected: %f)" % (key, in_row, in_pol, test_value[key][idx], reference[key]))
                #Next spectrum
                idx += 1

    def testIntentF(self):
        """Test selection by intent (float_data)"""
        self.run_test("intent", "float_data")

    def testIntentC(self):
        """Test selection by intent (corrected)"""
        self.run_test("intent", "corrected")

    # def testAntennaF(self):
    #     """Test selection by antenna (float_data)"""
    #     self.run_test("antenna", "float_data")

    # def testAntennaC(self):
    #     """Test selection by antenna (corrected)"""
    #     self.run_test("antenna", "corrected")

    def testFieldF(self):
        """Test selection by field (float_data)"""
        self.run_test("field", "float_data")

    def testFieldC(self):
        """Test selection by field (corrected)"""
        self.run_test("field", "corrected")

    def testSpwF(self):
        """Test selection by spw (float_data)"""
        self.run_test("spw", "float_data")

    def testSpwC(self):
        """Test selection by spw (corrected)"""
        self.run_test("spw", "corrected")

    def testTimerangeF(self):
        """Test selection by timerange (float_data)"""
        self.run_test("timerange", "float_data")

    def testTimerangeC(self):
        """Test selection by timerange (corrected)"""
        self.run_test("timerange", "corrected")

    def testScanF(self):
        """Test selection by scan (float_data)"""
        self.run_test("scan", "float_data")

    def testScanC(self):
        """Test selection by scan (corrected)"""
        self.run_test("scan", "corrected")

    # def testPolF(self):
    #     """Test selection by pol (float_data)"""
    #     self.run_test("pol", "float_data")

    # def testPolC(self):
    #     """Test selection by pol (corrected)"""
    #     self.run_test("pol", "corrected")

def suite():
    return [tsdfit_basicTest, tsdfit_selection]
