import os
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

from tsdbaseline import tsdbaseline
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

class tsdbaseline_unittest_base:
    """
    Base class for tsdbaseline unit test
    """
    # Data path of input/output
    datapath = os.environ.get('CASAPATH').split()[0] + \
              '/data/regression/unittest/tsdbaseline/'
    taskname = "tsdbaseline"
    verboselog = False

    #complist = ['max','min','rms','median','stddev']

    ### helper functions for tests ###
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


class tsdbaseline_basicTest(tsdbaseline_unittest_base, unittest.TestCase):
    """
    Basic unit tests for task tsdbaseline. No interactive testing.

    List of tests:
    test000 --- default values for all parameters
    test001 --- polynominal baselining with no mask (maskmode = 'list'). spw and pol specified.
    test002 --- cubic spline baselining with no mask (maskmode = 'list'). spw and pol specified.
    test050 --- existing file as outfile with overwrite=False (raises an exception)
    test051 --- no data after selection (raises an exception)

    Note: input data is generated from a single dish regression data,
    'OrionS_rawACSmod', as follows:
      default(sdcal)
      sdcal(infile='OrionS_rawACSmod',scanlist=[20,21,22,23],
                calmode='ps',tau=0.09,outfile='temp.asap')
      default(sdcal)
      sdcal(infile='temp.asap',timeaverage=True,
                tweight='tintsys',outfile='temp2.asap')
      sdsave(infile='temp2.asap',outformat='MS2',
                outfile='OrionS_rawACSmod_calave.ms')
    """
    # Input and output names
    #infile = 'OrionS_rawACSmod_calTave.asap'
    infile = 'OrionS_rawACSmod_calave.ms'
    outroot = tsdbaseline_unittest_base.taskname+'_basictest'
    blrefroot = tsdbaseline_unittest_base.datapath+'refblparam'
    tid = None

    def setUp(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)

        default(tsdbaseline)

    def tearDown(self):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)
        #os.system('rm -rf '+self.outroot+'*')

    def test000(self):
        """Basic Test 000: default values for all parameters"""
        tid = '000'
        infile = self.infile
        outfile = self.outroot+tid+'.ms'
        datacolumn = 'float_data'
        result = tsdbaseline(infile=infile, datacolumn=datacolumn,
                             outfile=outfile)
        # tsdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # uncomment the next line once blparam file can be output
        #self._compareBLparam(outfile+"_blparam.txt",self.blrefroot+tid)
        row = 3
        pol = 1
        results = self._getStats(outfile, '')
        theresult = None
        for i in range(len(results)):
            if ((results[i]['row'] == int(row)) and (results[i]['pol'] == int(pol))):
                theresult = results[i]
        reference = {'rms': 0.16677055621054496,
                     'min': -2.5817961692810059,
                     'max': 1.3842859268188477,
                     'median': -0.00086212158203125,
                     'stddev': 0.16677055621054496,
                     }
        self._compareStats(theresult, reference)

    def test001(self):
        """Basic Test 001: simple successful case: blfunc = 'poly', maskmode = 'list' and masklist=[] (no mask)"""
        tid = '001'
        infile = self.infile
        outfile = self.outroot+tid+'.ms'
        datacolumn = 'float_data'
        maskmode = 'list'
        blfunc = 'poly'
        spw = '3'
        pol = '1'
        overwrite = True
        result = tsdbaseline(infile=infile, datacolumn=datacolumn,
                             maskmode=maskmode, blfunc=blfunc, 
                             spw=spw, pol=pol, outfile=outfile,
                             overwrite=overwrite)
        # tsdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # uncomment the next line once blparam file can be output
        #self._compareBLparam(outfile+"_blparam.txt",self.blrefroot+tid)
        results = self._getStats(outfile, '', pol)
        print self._getStats(outfile, '', pol)
        theresult = None
        for i in range(len(results)):
            if (results[i]['pol'] == int(pol)):
                theresult = results[i]

        reference = {'rms': 0.16677055621054496,
                     'min': -2.5817961692810059,
                     'max': 1.3842859268188477,
                     'median': -0.00086212158203125,
                     'stddev': 0.16677055621054496,
                     }

        self._compareStats(theresult, reference)

    def test002(self):
        """Basic Test 002: simple successful case: blfunc = 'cspline', maskmode = 'list' and masklist=[] (no mask)"""
        print ""

        tid = '002'
        infile = self.infile
        outfile = self.outroot+tid+'.ms'
        datacolumn = 'float_data'  
        maskmode = 'list'
        blfunc = 'cspline'
        overwrite = True
        npiece = 3
        spw='3'
        pol='1'
        result = tsdbaseline(infile=infile, datacolumn=datacolumn,
                             maskmode=maskmode, blfunc=blfunc, 
                             npiece=npiece,spw=spw, 
                             pol=pol,
                             outfile=outfile,overwrite=overwrite)
        
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,msg="The task returned '"+str(result)+"' instead of None")
        #self._compareBLparam(outfile+"_blparam.txt",self.blrefroot+tid) 
        results = self._getStats(outfile, '', pol)
        print self._getStats(outfile, '', pol)
        
        theresult = None
        for i in range(len(results)):
            if (results[i]['pol'] == int(pol)):
                theresult = results[i]

        reference = {'rms': 0.16685959517745799,
                     'min': -2.5928177833557129,
                     'max': 1.3953156471252441,
                     'median': -0.00089824199676513672,
                     'stddev': 0.16685959517745766,
                    }

        self._compareStats(theresult, reference)

        #***
        #*** check if baseline is subtracted ***
        #***
       
        tb.open(outfile)
        num_ch=len(tb.getcell('FLOAT_DATA', 0)[0])
        tb.close()

        pol0=0
        pol1=1        
        orig_pol0_value=[]
        orig_pol1_value=[]

        sum_pol0=0.0
        sum_pol1=0.0

        sum_square_pol0 = 0.0
        sum_square_pol1 = 0.0

        # open the original MS
        tb.open(infile)
        for i in range(num_ch):
            orig_pol0_value.append(tb.getcell('FLOAT_DATA', int(spw))[pol0][i])
            orig_pol1_value.append(tb.getcell('FLOAT_DATA', int(spw))[pol1][i])
        tb.close()

        variance_orig_pol0 = numpy.var(numpy.array(orig_pol0_value))
        variance_orig_pol1 = numpy.var(numpy.array(orig_pol1_value))
        

        pol0_value=[]
        pol1_value=[]
        
        #open the MS after tsdbaseline
        tb.open(outfile)
        for i in range(num_ch):
            pol0_value.append(tb.getcell('FLOAT_DATA', 0)[pol0][i])
            pol1_value.append(tb.getcell('FLOAT_DATA', 0)[pol1][i])
        tb.close()

      
        variance_pol0 = numpy.var(numpy.array(pol0_value))        
        variance_pol1 = numpy.var(numpy.array(pol1_value))        


        #check if baseline of pol1 is subtracted and if pol0 is not.
        #if(pol=='1'):
        #    if((numpy.array(pol0_value) == numpy.array(orig_pol0_value)).all()):
        #        print 'pol0: baseline subtraction is NOT performed. OK'
        #    if((numpy.array(pol1_value) < numpy.array(orig_pol1_value)).all()):
        #        print 'pol1: baseline subtraction is performed.OK'

        
        #cspline applied to only pol1 => pol0 should be the same before and after applying cspline


        #assert pol1_value < prig_pol1_value
        self.assertLess(pol1_value, orig_pol1_value)
        
        #assert variance of pol1_value < variance of prig_pol1_value
        self.assertLess(variance_pol1**0.5, variance_orig_pol1**0.5)

        #assert pol0_value == orig_pol0_value
        self.assertEqual(pol0_value, orig_pol0_value)
        self.assertEqual(variance_pol0, variance_orig_pol0)


        #print '1sigma before cspline (pol1)', variance_orig_pol1**0.5 
        #print '1sigma after cspline (pol1)',  variance_pol1**0.5 
        
        #print '1sigma before cspline (pol0)', variance_orig_pol0**0.5 
        #print '1sigma after cspline (pol0)',  variance_pol0**0.5 

    def test050(self):
        """Basic Test 050: failure case: existing file as outfile with overwrite=False"""
        infile = self.infile
        outfile = 'Dummy_Empty.ms'
        mode = 'list'
        os.mkdir(outfile)
        try:
            result = tsdbaseline(infile=infile, outfile=outfile, overwrite=False, maskmode=mode)
        except Exception, e:
            pos = str(e).find(outfile+' exists.')
            self.assertNotEqual(pos, -1, msg='Unexpected exception was thrown: %s'%(str(e)))
        finally:
            shutil.rmtree(outfile)

    def test051(self):
        """Basic Test 051: failure case: no data after selection"""
        tid = '051'
        infile = self.infile
        outfile = self.outroot+tid+'.ms'
        spw = '10' # non-existent IF value
        mode = 'list'
        try:
            result = tsdbaseline(infile=infile, outfile=outfile, spw=spw, maskmode=mode)
        except Exception, e:
            pos = str(e).find('No Spw ID(s) matched')
            self.assertNotEqual(pos, -1, msg='Unexpected exception was thrown: %s'%(str(e)))


class tsdbaseline_maskTest(tsdbaseline_unittest_base, unittest.TestCase):
    """
    Tests for various mask selections. No interactive testing.

    List of tests:
    test100 --- with masked ranges at the edges of spectrum. blfunc is cspline.
    test101 --- with masked ranges not touching spectrum edge

    Note: input data is generated from a single dish regression data,
    'OrionS_rawACSmod', as follows:
      default(sdcal)
      sdcal(infile='OrionS_rawACSmod',scanlist=[20,21,22,23],
                calmode='ps',tau=0.09,outfile='temp.asap')
      default(sdcal)
      sdcal(infile='temp.asap',timeaverage=True,
                tweight='tintsys',outfile='temp2.asap')
      sdsave(infile='temp2.asap',outformat='MS2',
                outfile='OrionS_rawACSmod_calave.ms')
    """
    # Input and output names
    infile = 'OrionS_rawACSmod_calave.ms'
    outroot = tsdbaseline_unittest_base.taskname+'_masktest'
    blrefroot = tsdbaseline_unittest_base.datapath+'refblparam_mask'
    tid = None

    # Channel range excluding bad edge
    search = [[200,7599]]
    # Baseline channels. should be identical to one selected by 'auto' mode
    blchan0 = [[200,3979],[4152,7599]]
    blchan2 = [[200,2959],[3120,7599]]

    # reference values
    ref_pol0if0 =  {'linemaxpos': 4102.0, 'linesum': 103.81604766845703,
                    'linemax': 1.6280698776245117,
                    'baserms': 0.15021507441997528,
                    'basestd': 0.15022546052932739}
    ref_pol0if2 = {#'linemaxpos': 3045.0, 'linesum': 127.79755401611328,
                   #'linemax': 2.0193681716918945,
                   #'baserms': 0.13134850561618805,
                   #'basestd': 0.1313575953245163}
                   'rms': 0.13134850561618805,
                   'stddev': 0.1313575953245163}
     
    def setUp(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)
        default(tsdbaseline)

    def tearDown(self):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)
        os.system('rm -rf '+self.outroot+'*')

    def test100(self):
        """Mask Test 100: with masked ranges at the edges of spectrum. blfunc must be cspline."""
        self.tid='100'
        infile = self.infile
        outfile = self.outroot+self.tid+'.ms'
        datacolumn='float_data'
        mode = 'list'
        spw = '2:%s'%(';'.join(map(self._get_range_in_string,self.search)))
        pol = '0'
        blfunc = 'cspline'
        npiece = 4

        result = tsdbaseline(infile=infile,datacolumn=datacolumn,maskmode=mode,
                             spw=spw,pol=pol,blfunc=blfunc,npiece=npiece,
                             outfile=outfile)
        # tsdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF2
        testval = self._getStats(filename=outfile, spw='', pol=pol, mask=self.search)
        ref100 = {'rms': 0.18957555661537034,
                  'min': -0.48668813705444336,
                  'max': 1.9516196250915527,
                  'median': -0.013428688049316406,
                  'stddev': 0.18957555661537034,
                  'row': 0,
                  'pol': 0}
        self._compareStats(testval[0], ref100)

    def test101(self):
        """Mask Test 101: with masked ranges not touching spectrum edge"""
        self.tid='101'
        infile = self.infile
        outfile = self.outroot+self.tid+'.ms'
        datacolumn='float_data'
        mode = 'list'
        spw = '2:%s'%(';'.join(map(self._get_range_in_string,self.blchan2)))
        pol = '0'

        print 'spw =', spw

        result = tsdbaseline(infile=infile,datacolumn=datacolumn,maskmode=mode,
                            outfile=outfile,spw=spw,pol=pol)
        # tsdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF2
        testval = self._getStats(filename=outfile, spw='', pol=pol, mask=self.blchan2)
        self._compareStats(testval[0],self.ref_pol0if2)
        #self._compareBLparam(self.outroot+self.tid+'.asap_blparam.txt',\
        #                     self.blrefroot+self.tid)

    def _get_range_in_string(self, valrange):
        if isinstance(valrange, list) or isinstance(valrange, tuple):
            return str(valrange[0])+'~'+str(valrange[1])
        else:
            return False


class tsdbaseline_multi_IF_test(tsdbaseline_unittest_base, unittest.TestCase):
    """
    Unit tests for task tsdbaseline. No interactive testing.

    This test intends to check whether tsdbaseline task works fine
    for data that has multiple IFs whose nchan differ each other. 

    List of tests:
    test200 --- test multi IF data input
    """
    # Input and output names
    infile = 'testMultiIF.asap'
    blparamfile_suffix = '_blparam.txt'
    outroot = tsdbaseline_unittest_base.taskname+'_multi'
    refblparamfile = 'refblparam_multiIF'

    def setUp(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)
        default(tsdbaseline)

    def tearDown(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        os.system('rm -rf '+self.outroot+'*')

    def test200(self):
        """test200: Test the task works with multi IF data"""
        infile = self.infile
        mode = "list"
        blfunc = "poly"
        order = 1
        outfile = self.outroot+".asap"
        blparamfile = outfile+self.blparamfile_suffix
        
        result = tsdbaseline(infile=infile,maskmode=mode,outfile=outfile,blfunc=blfunc,order=order)
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")
        self._compareBLparam(blparamfile,self.datapath+self.refblparamfile)
        reference = {5: {'rms': 1.4250789880752563,
                         'min': -4.2702846527099609,
                         'max': 5.5566844940185547,
                         'max_abscissa': {'value': 823.0,
                                          'unit': 'channel'},
                         'median': 0.017315864562988281,
                         'min_abscissa': {'value': 520.0,
                                          'unit': 'channel'},
                         'stddev': 1.425775408744812},
                     7: {'rms': 1.4971292018890381,
                         'min': -4.7103700637817383,
                         'max': 5.4820127487182617,
                         'max_abscissa': {'value': 1335.0,
                                          'unit': 'channel'},
                         'median': 0.027227401733398438,
                         'min_abscissa': {'value': 1490.0,
                                          'unit': 'channel'},
                         'stddev': 1.4974949359893799}}
        # sdstat must run each IF separately
        for ifno in [5,7]:
            currstat = self._getStats(outfile,ifno)
            self._compareStats(currstat,reference[ifno])


class tsdbaseline_bltableTest(tsdbaseline_unittest_base, unittest.TestCase):
    """
    Tests for baseline table

    List of tests
    test300 --- blmode='fit', bloutput='', dosubtract=False (no baselining, no bltable output)
    test301 --- blmode='fit', bloutput!='', dosubtract=True, blfunc='poly'/'chebyshev'/'cspline'
                (poly/chebyshev/cspline fit in MS, bltable is written)
    test302 --- blmode='fit', bloutput!='', dosubtract=True, blfunc='variable'
                (variable fit in MS, bltable is written)
                testing 3 cases:
                    (1) blparam contains values for all spectra
                    (2) no values for a spectrum (row=2,pol=1), which is to be skipped
                    (3) values commented out for a spectrum (row=2,pol=1), which is to be skipped

    Note: input data is generated from a single dish regression data,
    'OrionS_rawACSmod', as follows:
      default(sdcal)
      sdcal(infile='OrionS_rawACSmod',scanlist=[20,21,22,23],
                calmode='ps',tau=0.09,outfile='temp.asap')
      default(sdcal)
      sdcal(infile='temp.asap',timeaverage=True,
                tweight='tintsys',outfile='temp2.asap')
      sdsave(infile='temp2.asap',outformat='MS2',
                outfile='OrionS_rawACSmod_calave.ms')
    """
    # Input and output names
    infile = 'OrionS_rawACSmod_calave.ms'
    outroot = tsdbaseline_unittest_base.taskname+'_bltabletest'
    tid = None
    ftype = {'poly': 0, 'chebyshev': 1, 'cspline': 2, 'sinusoid': 3}

    blparam_order = ['row', 'pol', 'mask', 'nclip', 'cthre',
                     'uself', 'lthre', 'ledge', 'redge', 'chavg',
                     'btype', 'order', 'npiec', 'nwave']
    blparam_dic = {}
    blparam_dic['row']   = [0, 0, 1, 1, 2, 2, 3, 3]
    blparam_dic['pol']   = [0, 1, 0, 1, 0, 1, 0, 1]
    blparam_dic['mask']  = ['0~4000;6000~8000']*3 + ['']*5
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

    def setUp(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)
        default(tsdbaseline)

    def tearDown(self):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)
        os.system('rm -rf '+self.outroot+'*')

    def _checkBltableVar(self, outms, bltable, blparam, option):
        npol = 2
        results = [[4.28688335], [3.92898083],
                   [4.32321167, 0.01343461], [3.83523464e+00, -9.37040113e-06],
                   [4.29850674, 0.00465105, -0.00938174], [4.19220352e+00, -1.22489364e-04, 1.47625645e-08],
                   [4.18074942e+00, 4.44418511e-05, -6.81303991e-09, 3.36383428e-13],
                   [3.93746042e+00, 5.36526677e-05, -1.11215614e-08, 7.00922610e-13]
                   ]
        rms = [0.160806953907, 0.172459229827, 0.145005837083, 0.167485550046,
               0.141711354256, 0.874852001667, 0.134670868516, 0.149306803942]
        tb.open(bltable)
        for i in range(npol*tb.nrows()):
            irow = i / npol
            ipol = i % npol
            is_skipped = (option != '') and (irow == 2) and (ipol == 1)

            self.assertEqual(not is_skipped, tb.getcell('APPLY', irow)[ipol][0]);
            self.assertEqual(self.ftype[blparam['btype'][i]], tb.getcell('FUNC_TYPE', irow)[ipol][0]);
            fparam_key = 'order' if (blparam['btype'][i] != 'cspline') else 'npiec'
            fparam = blparam[fparam_key][i]
            if not is_skipped:
                self.assertEqual(fparam, tb.getcell('FUNC_PARAM', irow)[ipol][0])
            if (blparam['btype'][i] == 'cspline'):
                for j in range(blparam['npiec'][i]):
                    self.assertEqual(0.0, tb.getcell('FUNC_FPARAM', irow)[ipol][j])
            else:
                self.assertEqual(0, len(tb.getcell('FUNC_FPARAM', irow)[ipol]))
            for j in range(len(results[i])):
                result = 0.0 if is_skipped else results[i][j]
                self._checkValue(result, tb.getcell('RESULT', irow)[ipol][j], 1.0e-6)
            if not is_skipped:
                self._checkValue(rms[i], tb.getcell('RMS', irow)[ipol][0], 1.0e-6)
                self._checkValue(float(blparam['cthre'][i]), tb.getcell('CLIP_THRESHOLD', irow)[ipol][0], 1.0e-6)
                self.assertEqual(blparam['nclip'][i], tb.getcell('CLIP_ITERATION', irow)[ipol][0])
                uself = (blparam['uself'][i] == 'true')
                self.assertEqual(uself, tb.getcell('USE_LF', irow)[ipol][0])
                lthre = 5.0 if (blparam['lthre'][i] == '') else float(blparam['lthre'][i])
                #self._checkValue(lthre, tb.getcell('LF_THRESHOLD', irow)[ipol][0], 1.0e-6)
                chavg = 0 if (blparam['chavg'][i] == '') else int(blparam['chavg'][i])
                self.assertEqual(chavg, tb.getcell('LF_AVERAGE', irow)[ipol][0])
                ledge = 0 if (blparam['ledge'][i] == '') else int(blparam['ledge'][i])
                #self.assertEqual(ledge, tb.getcell('LF_EDGE', irow)[ipol][0])
                redge = 0 if (blparam['redge'][i] == '') else int(blparam['redge'][i])
                #self.assertEqual(redge, tb.getcell('LF_EDGE', irow)[ipol][1])

        tb.close()
    
    def _checkBltable(self, outms, bltable, blfunc, order, mask):
        tb.open(bltable)
        for irow in range(tb.nrows()):
            for ipol in range(len(tb.getcell('RMS', irow))):
                self.assertEqual(tb.getcell('FUNC_TYPE', irow)[ipol], self.ftype[blfunc])
                self.assertEqual(tb.getcell('FUNC_PARAM', irow)[ipol], order)
                ref = self._getStats(filename=outms, spw=str(irow), pol=str(ipol), mask=mask[irow])
                #tolerance value in the next line is temporarily set a bit large 
                #since rms in bltable is smaller than expected because it is
                #calculated based on masklist currently stored in bltable, which 
                #is after an extra clipping.
                #this bug is already fixed in trunk of Sakura, so once libsakura
                #is updated we can set smaller tolerance value. (2015/4/22 WK)
                self._checkValue(ref[0]['rms'], tb.getcell('RMS', irow)[ipol][0], 2.0e-2)
        tb.close()

    def _checkValue(self, ref, out, tol=1.0e-02):
        if ref != 0.0:
            rel = abs((out - ref)/ref)
        else:
            rel = abs((out - ref)/out)
        #print 'rel=' + str(rel)
        if rel > tol:
            raise Exception, 'result and reference differs!'
        
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

    def test300(self):
        """Mask Test 300: no baselining, no bltable output"""
        self.tid='300'
        infile = self.infile
        outfile = self.outroot+self.tid+'.ms'
        datacolumn='float_data'
        blmode='fit'
        bloutput=''
        dosubtract=False

        result = tsdbaseline(infile=infile,datacolumn=datacolumn,
                             blmode=blmode,bloutput=bloutput,dosubtract=dosubtract,
                             outfile=outfile)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")

        spec_in = []
        tb.open(infile)
        for i in range(tb.nrows()):
            spec_in.append(tb.getcell('FLOAT_DATA', i))
        tb.close()
        spec_out = []
        tb.open(outfile)
        for i in range(tb.nrows()):
            spec_out.append(tb.getcell('FLOAT_DATA', i))
        tb.close()
        for irow in range(len(spec_in)):
            for ipol in range(len(spec_in[0])):
                for ichan in range(len(spec_in[0][0])):
                    self.assertEqual(spec_in[irow][ipol][ichan], spec_out[irow][ipol][ichan],
                                     msg="output spectrum modified at row="+str(irow)+
                                     ",pol="+str(ipol)+",chan="+str(ichan))

    def test301(self):
        """Mask Test 301: poly/chebyshev/cspline baselining, output bltable"""
        self.tid='301'
        infile = self.infile
        datacolumn='float_data'
        spw='0:1000~3500;5000~7500,1:500~7500,2:500~2500;3500~7500'
        mask=[ [[1000,3500],[5000,7500]],
               [[500,7500]],
               [[500,2500],[3500,7500]]
               ]
        blmode='fit'
        blformat='table'
        dosubtract=True
        blfunc=['poly','chebyshev','cspline']
        order=5
        npiece=4
        rms_s0p0_ms = [0.150905484071, 0.150905484071, 0.149185846787]

        for i in range(len(blfunc)):
            print 'testing blfunc='+blfunc[i]+'...'
            outfile = self.outroot+self.tid+blfunc[i]+'.ms'
            bloutput= self.outroot+self.tid+blfunc[i]+'.bltable'
            result = tsdbaseline(infile=infile,datacolumn=datacolumn,
                                 blmode=blmode,blformat=blformat,bloutput=bloutput,
                                 spw=spw,blfunc=blfunc[i],order=order,npiece=npiece,
                                 dosubtract=dosubtract,outfile=outfile)
            self.assertEqual(result,None,
                             msg="The task returned '"+str(result)+"' instead of None")
            msresult = self._getStats(filename=outfile, spw='0', pol='0', mask=mask[0])
            self._checkValue(rms_s0p0_ms[i], msresult[0]['stddev'], 1.0e-6)

            fparam = npiece if blfunc[i] == 'cspline' else order
            self._checkBltable(outfile, bloutput, blfunc[i], fparam, mask)
            print 'OK'

    def test302(self):
        """Mask Test 302: per-spectrum baselining, output bltable"""
        self.tid='302'
        infile = self.infile
        datacolumn='float_data'
        blmode='fit'
        blformat='table'
        blfunc='variable'
        dosubtract=True

        for option in ['', 'r2p1less', 'r2p1cout']:
            bloutput= self.outroot+self.tid+option+'.bltable'
            outfile = self.outroot+self.tid+option+'.ms'
            blparam = self.outroot+self.tid+option+'.blparam'
            self._createBlparamFile(blparam, self.blparam_order, self.blparam_dic, option)
            result = tsdbaseline(infile=infile,datacolumn=datacolumn,
                                 blmode=blmode,blformat=blformat,bloutput=bloutput,
                                 blfunc=blfunc,blparam=blparam,
                                 dosubtract=dosubtract,outfile=outfile)
            self.assertEqual(result,None,
                             msg="The task returned '"+str(result)+"' instead of None")
            self._checkBltableVar(outfile, bloutput, self.blparam_dic, option)


class tsdbaseline_variableTest(tsdbaseline_unittest_base, unittest.TestCase):
    """
    Tests for blfunc='variable'

    List of tests necessary
    00: test baseline subtraction with variable baseline functions and orders
    01: test skipping rows by comment, i.e., lines start with '#' (rows should be flagged)
    02: test skipping rows by non-existent lines in blparam file (rows should be flagged)
    03: test mask selection
    04: test data selection
    05: test clipping
    06: duplicated fitting parameter in blparam file (the last one is adopted)
    NOT IMPLEMENTED YET
    * test dosubtract = False
    * line finder
    * edge flagging
    """
    outfile='variable_bl.ms'
    column='float_data'
    nspec = 4
    refstat0 = {'max': [0.0]*nspec, 'min': [0.0]*nspec,
                'rms': [0.0]*nspec, 'stddev': [0.0]*nspec}
    
    def setUp(self):
        if hasattr(self, 'infile'):
            self.__refetch_files(self.infile)

        default(tsdbaseline)

    def tearDown(self):
        self._remove([self.infile, self.outfile])

    def _refetch_files(self, files, from_dir=None):
        if type(files)==str: files = [files]
        self._remove(files)
        self._copy(files, from_dir)

    def __select_stats(self, stats, idx_list):
        """
        Returns a dictionary with selected elements of statistics
        stats    : a dictionary of statistics
        idx_list : a list of indices to select in stats
        """
        ret_dict = {}
        for key in stats.keys():
            ret_dict[key] = [stats[key][idx] for idx in idx_list]
        return ret_dict

    def _run_test(self, infile, reference, mask=None, rtol=1.e-5, atol=1.e-6, flag_spec=(), **task_param):
        """
        Run tsdbaseline with mode='variable' and test output MS.

        infile    : input ms name
        reference : reference statistic values in form {'key': [value0, value1, ...], ...}
        mask      : list of masklist to calculate statistics of output MS (None=use all)
        rtol, atol: relative and absolute tolerance of comparison.
        flag_spec : a list of rowid and polid pair whose spectrum should be flagged in output MS
        **task_param : additional parameters to invoke task. blfunc and outfile are predefined.
        """
        self.infile = infile
        tsdbaseline(infile=self.infile,blfunc='variable',outfile=self.outfile,**task_param)
        colname = (task_param['datacolumn'] if task_param.has_key('datacolumn') else 'data').upper()

        # calculate statistics of valid spectrum. Test flagged spectrum.
        ivalid_spec = 0
        ispec = 0
        stats_list = []
        valid_idx = []
        with tbmanager(self.outfile) as tb:
            for rowid in range(tb.nrows()):
                data = tb.getcell(colname, rowid)
                flag = tb.getcell('FLAG', rowid)
                npol = len(data)
                for polid in range(npol):
                    if (rowid, polid) in flag_spec:
                        # for flagged rows
                        self.assertTrue(flag[polid].all(),
                                        "row=%d, pol=%d should be flagged" % (rowid, polid))
                    else:
                        spec = data[polid,:]
                        masklist = mask[ivalid_spec] if mask is not None else None
                        stats_list.append(self._calc_stats_of_array(spec, masklist))
                        ivalid_spec += 1
                        valid_idx.append(ispec)
                    ispec += 1
        # shrink reference list if # of processed spectra is smaller than reference (selection)
        if len(stats_list) < len(reference[reference.keys()[0]]):
            self.assertEqual(len(valid_idx), len(stats_list),
                             "Internal error: len(valid_idx)!=len(stats_list)")
            reference = self.__select_stats(reference, valid_idx)

        currstat = self._convert_statslist_to_dict(stats_list)
        #print("cruustat=%s" % str(currstat))
        self._compareStats(currstat, reference, rtol=1.0e-6, atol=1.0e-6)

    def testVariable00(self):
        """Test blfunc='variable' with variable baseline functions and orders"""
        infile='analytic_variable.ms'
        paramfile='analytic_variable_blparam.txt'
        self._refetch_files([infile, paramfile], self.datapath)
        self._run_test(infile,self.refstat0,blparam=paramfile,datacolumn=self.column)

    def testVariable01(self):
        """Test blfunc='variable' with skipping rows by comment ('#') (rows should be flagged)"""
        infile='analytic_variable.ms'
        paramfile='analytic_variable_blparam_comment.txt'
        self._refetch_files([infile, paramfile], self.datapath)
        self._run_test(infile,self.refstat0,flag_spec=[(0,0)],blparam=paramfile,datacolumn=self.column)

    def testVariable02(self):
        """Test blfunc='variable' with non-existent lines in blparam file (rows should be flagged)"""
        infile='analytic_variable.ms'
        paramfile='analytic_variable_blparam_2lines.txt'
        self._refetch_files([infile, paramfile], self.datapath)
        self._run_test(infile,self.refstat0,flag_spec=[(0,0),(1,1)],blparam=paramfile,datacolumn=self.column)

    def testVariable03(self):
        """Test blfunc='variable' with mask selection"""
        infile='analytic_order3_withoffset.ms'
        paramfile='analytic_variable_blparam_mask.txt'
        self._refetch_files([infile, paramfile], self.datapath)
        mask = [[[0,4000],[6000,8000]], [[0,5000],[6000,8000]], [[0,3000],[5000,8000]], None]
        self._run_test(infile,self.refstat0,mask=mask,blparam=paramfile,datacolumn=self.column)

    def testVariable04(self):
        """Test blfunc='variable' with data selection (spw='1')"""
        infile='analytic_variable.ms'
        paramfile='analytic_variable_blparam_spw1.txt'
        self._refetch_files([infile, paramfile], self.datapath)
        self._run_test(infile,self.refstat0,spw='1',blparam=paramfile,datacolumn=self.column)

    def testVariable05(self):
        """Test blfunc='variable' with clipping"""
        infile='analytic_order3_withoffset.ms'
        paramfile='analytic_variable_blparam_clip.txt'
        self._refetch_files([infile, paramfile], self.datapath)
        mask = [[[0,4000],[6000,8000]], [[0,5000],[6000,8000]], [[0,3000],[5000,8000]], None]
        self._run_test(infile,self.refstat0,atol=1.e-5,
                       mask=mask,blparam=paramfile,datacolumn=self.column)

    def testVariable06(self):
        """Test blfunc='variable' with duplicated fitting parameters (the last one is adopted)"""
        infile='analytic_variable.ms'
        paramfile='analytic_variable_blparam_duplicate.txt'
        self._refetch_files([infile, paramfile], self.datapath)
        self._run_test(infile,self.refstat0,blparam=paramfile,datacolumn=self.column)

def suite():
    return [tsdbaseline_basicTest, 
            tsdbaseline_maskTest,
            #tsdbaseline_multi_IF_test,
            tsdbaseline_bltableTest,
            tsdbaseline_variableTest
            ]
