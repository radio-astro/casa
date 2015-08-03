#import os
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

    Note: input data 'gaussian.ms' is based on a single dish
    regression data, 'OrionS_rawACSmod', but the spectra values
    are modified to have the following features:
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

      the spectra of the row (irow=3) have two Gaussian lines, while
      those of the other rows have single Gaussian line.
      spw value is identical to irow, and number of channels is 8192.
    """
    # Input and output names
    infile = 'gaussian.ms'
    outroot = tsdfit_unittest_base.taskname+'_basictest'
    blrefroot = tsdfit_unittest_base.datapath+'refblparam'
    tid = None

    def setUp(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)
        default(tsdfit)

    def tearDown(self):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)
        os.system('rm -rf '+self.outroot+'*')

    def test000(self):
        """Basic Test 000: default values for all parameters (nfit=[0] : no fitting)"""
        tid = '000'
        infile = self.infile
        outfile = self.outroot+tid+'.ms'
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
        """Basic Test 001: fitting with single Gaussian (spw='0,1,2', nfit=[1])"""
        tid = '001'
        infile = self.infile
        outfile = self.outroot+tid+'.ms'
        datacolumn = 'float_data'
        spw = '0,1,2'
        nfit = [1]
        result = tsdfit(infile=infile, datacolumn=datacolumn, spw=spw, nfit=nfit)

        npol = 2
        nrow = len(spw.split(','))

        answer = {'cent': [[4000.0], [5000.0], [3000.0], [2000.0], [4500.0], [5500.0]],
                  'peak': [[10.0], [10.0], [10.0], [10.0], [10.0], [10.0]],
                  'fwhm': [[500.0], [100.0], [20.0], [4.0], [100.0], [20.0]]
                  }
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
                        #print "lower("+str(result_lower)+") - answer("+str(answer[key][i][j])+") - upper("+str(result_upper) +")"
                        self.assertTrue(((result_lower <= answer[key][i][j]) and (answer[key][i][j] <= result_upper)),
                                        msg="row%s, comp%s result inconsistent with answer"%(i, j))

    def test002(self):
        """Basic Test 002: fitting with double fitrange (spw='3', nfit=[1,1])"""
        tid = '002'
        infile = self.infile
        outfile = self.outroot+tid+'.ms'
        datacolumn = 'float_data'
        spw = '3:0~4000;4001~8191'
        nfit = [1,1]
        result = tsdfit(infile=infile, datacolumn=datacolumn, spw=spw, nfit=nfit)

        npol = 2
        nrow = 1

        answer = {'cent': [[2000.0, 6000.0], [3900.0, 4100.0]],
                  'peak': [[10.0, 10.0], [10.0, 5.0]],
                  'fwhm': [[100.0, 100.0], [20.0, 100.0]]
                  }
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
                        #print "lower("+str(result_lower)+") - answer("+str(answer[key][i][j])+") - upper("+str(result_upper) +")"
                        self.assertTrue(((result_lower <= answer[key][i][j]) and (answer[key][i][j] <= result_upper)),
                                        msg="row%s, comp%s result inconsistent with answer"%(i, j))

    def test003(self):
        """Basic Test 003: fitting with double Gaussian (spw='3', nfit=[2])"""
        tid = '003'
        infile = self.infile
        outfile = self.outroot+tid+'.ms'
        datacolumn = 'float_data'
        spw = '3'
        nfit = [2]
        result = tsdfit(infile=infile, datacolumn=datacolumn, spw=spw, nfit=nfit)

        npol = 2
        nrow = 1

        answer = {'cent': [[2000.0, 6000.0], [3900.0, 4100.0]],
                  'peak': [[10.0, 10.0], [10.0, 5.0]],
                  'fwhm': [[100.0, 100.0], [20.0, 100.0]]
                  }
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
                        #print "lower("+str(result_lower)+") - answer("+str(answer[key][i][j])+") - upper("+str(result_upper) +")"
                        self.assertTrue(((result_lower <= answer[key][i][j]) and (answer[key][i][j] <= result_upper)),
                                        msg="row%s, comp%s result inconsistent with answer"%(i, j))


def suite():
    return [tsdfit_basicTest]
