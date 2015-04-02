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
class FileReader( object ):
    def __init__( self, filename ):
        self.__filename = filename
        self.__data = None
        self.__nline = None

    def read( self ):
        if self.__data is None:
            f = open(self.__filename, 'r')
            self.__data = f.readlines()
            f.close()
            self.__nline = len( self.__data )
        return

    def nline( self ):
        self.read()
        return self.__nline

    def index( self, txt, start ):
        return self.__data[start:].index( txt ) + 1 + start

    def getline( self, idx ):
        return self.__data[idx]

class BlparamFileParser( FileReader ):
    def __init__( self, blfile ):
        FileReader.__init__( self, blfile )
        self.__nrow = None
        self.__coeff = None
        self.__rms = None
        self.__ctxt = 'Baseline parameters\n'
        self.__rtxt = 'Results of baseline fit\n'

    def nrow( self ):
        self.read()
        if self.__nrow is None:
            return self._nrow()
        else:
            return self.__nrow

    def coeff( self ):
        self.read()
        if self.__coeff is None:
            self.parseCoeff()
        return self.__coeff

    def rms( self ):
        self.read()
        if self.__rms is None:
            self.parseRms()
        return self.__rms

    def _nrow( self ):
        self.__nrow = 0
        for i in xrange(self.nline()):
            if self.getline( i ) == self.__ctxt:
                self.__nrow += 1
        return self.__nrow

    def parse( self ):
        self.read()
        self.parseCoeff()
        self.parseRms()
        return
        
    def parseCoeff( self ):
        self.__coeff = []
        nrow = self.nrow()
        idx = 0
        while ( len(self.__coeff) < nrow ):
            try:
                idx = self.index( self.__ctxt, idx )
                coeffs = []
                while( self.getline( idx ) != self.__rtxt ):
                    coeff = self.__parseCoeff( idx )
                    coeffs += coeff
                    idx += 1
                self.__coeff.append( coeffs )
            except:
                break
        return

    def parseRms( self ):
        self.__rms = []
        nrow = self.nrow()
        idx = 0
        while ( len(self.__rms) < nrow ):
            try:
                idx = self.index( self.__rtxt, idx )
                self.__rms.append( self.__parseRms( idx ) )
            except:
                break   
        return

    def __parseCoeff( self, idx ):
        return parseCoeff( self.getline( idx ) )

    def __parseRms( self, idx ):
        return parseRms( self.getline( idx ) )

def parseCoeff( txt ):
    clist = txt.rstrip( '\n' ).split(',')
    ret = []
    for c in clist:
        ret.append( float( c.split('=')[1] ) )
    return ret
    
def parseRms( txt ):
    t = txt.lstrip().rstrip( '\n' )[6:]
    return float( t )

class tsdbaseline_unittest_base:
    """
    Base class for tsdbaseline unit test
    """
    # Data path of input/output
    datapath = os.environ.get('CASAPATH').split()[0] + \
              '/data/regression/unittest/tsdbaseline/'
    taskname = "tsdbaseline"

    #complist = ['max','min','rms','median','stddev']

    ### helper functions for tests ###
    def _checkfile( self, name ):
        isthere=os.path.exists(name)
        self.assertTrue(isthere,
                         msg='Could not find, %s'%(name))

    """
    def _getStats( self, filename, spw=None ):
        if not spw:
            spw=''
        self._checkfile(filename)
        sd.rcParams['scantable.storage'] = 'memory'
        retstat = sdstat(filename, spw=str(spw))
        return retstat
    """
    def _getUniqList(self, val):
        if not isinstance(val, list):
            raise Exception('_getUniqList: input value must be a list.')
        return list(set(val))

    def _getListSelection(self, val):
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
        res = []
        for i in range(len(data_list)):
            if data_list[i] in sel_list:
                idx = sel_list.index(data_list[i])
                res.append(i)
        return self._getUniqList(res)
    
    def _getEffective(self, spec, mask):
        res = []
        for i in range(len(mask)):
            for j in range(mask[i][0], mask[i][1]):
                res.append(spec[j])
        return numpy.array(res)

    def _getStats(self, filename=None, spw=None, pol=None, colname=None, mask=None):
        if not spw: spw = ''
        select_spw = (spw != '') and (spw != '*')
        if select_spw: spw_sel = self._getListSelection(spw)
        if not pol: pol = ''
        select_pol = (pol != '') and (pol != '*')
        if select_pol: pol_sel = self._getListSelection(pol)
        if not colname: colname='FLOAT_DATA'
        self._checkfile(filename)
        with tbmanager(filename) as tb:
            data = tb.getcol(colname)
            ddid = tb.getcol('DATA_DESC_ID')
        with tbmanager(filename+'/DATA_DESCRIPTION') as tb:
            spwid = tb.getcol('SPECTRAL_WINDOW_ID').tolist()
        if not select_spw: spw_sel = spwid
        dd_sel = self._getListSelectedRowID(spwid, spw_sel)
        row_sel = self._getListSelectedRowID(ddid, dd_sel)
        if not select_spw: row_sel = range(len(ddid))
        if not select_pol: pol_sel = range(len(data))

        res = []
        for irow in row_sel:
            for ipol in pol_sel:
                res_elem = {}
                res_elem['row'] = irow
                res_elem['pol'] = ipol

                spec = data[ipol,:,irow]
                if mask is not None: spec = self._getEffective(spec, mask)
                res_elem['rms'] = numpy.sqrt(numpy.var(spec))
                res_elem['min'] = numpy.min(spec)
                res_elem['max'] = numpy.max(spec)
                spec_mea = numpy.mean(spec)
                res_elem['median'] = numpy.median(spec)

                val_mean = 0.0
                val_meansq = 0.0
                nchan = len(spec)
                for ichan in range(nchan):
                    val_mean = val_mean + spec[ichan]
                    val_meansq = val_meansq + spec[ichan] * spec[ichan]
                val_mean = val_mean / nchan
                val_meansq = val_meansq / nchan
                res_elem['stddev'] = numpy.sqrt(val_meansq - val_mean * val_mean)

                res.append(res_elem)

        return res


    def _compareStats( self, currstat, refstat, reltol=1.0e-2, complist=None ):
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
            for i in range(len(currval)):
                if isinstance(refval[i],str):
                    self.assertTrue(currval[i]==refval[i],\
                                    msg="%s[%d] differs: %s (expected: %s) " % \
                                    (key, i, str(currval[i]), str(refval[i])))
                else:
                    self.assertTrue(self._isInAllowedRange(currval[i],refval[i],reltol),\
                                    msg="%s[%d] differs: %s (expected: %s) " % \
                                    (key, i, str(currval[i]), str(refval[i])))
            del currval, refval

            
    def _isInAllowedRange( self, testval, refval, reltol=1.e-2 ):
        """
        Check if a test value is within permissive relative difference from refval.
        Returns a boolean.
        testval & refval : two numerical values to compare
        reltol           : allowed relative difference to consider the two
                           values to be equal. (default 0.01)
        """
        denom = refval
        if refval == 0:
            if testval == 0:
                return True
            else:
                denom = testval
        rdiff = (testval-refval)/denom
        del denom,testval,refval
        return (abs(rdiff) <= reltol)

    def _to_list( self, input ):
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


    def _compareBLparam( self, out, reference ):
        # test if baseline parameters are equal to the reference values
        # currently comparing every lines in the files
        # TO DO: compare only "Fitter range" and "Baseline parameters"
        self._checkfile(out)
        self._checkfile(reference)
        
        blparse_out = BlparamFileParser( out )
        blparse_out.parse()
        coeffs_out = blparse_out.coeff()
        rms_out = blparse_out.rms()
        blparse_ref = BlparamFileParser( reference )
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
            rdiff = ( r1 - r0 ) / r0
            self.assertTrue((abs(rdiff)<allowdiff),
                            msg='row %s: rms is different'%(irow))
            c0 = coeffs_ref[irow]
            c1 = coeffs_out[irow]
            for ic in xrange(len(c1)):
                rdiff = ( c1[ic] - c0[ic] ) / c0[ic]
                self.assertTrue((abs(rdiff)<allowdiff),
                                msg='row %s: coefficient for order %s is different'%(irow,ic))
        print ''
#         self.assertTrue(listing.compare(out,reference),
#                         'New and reference files are different. %s != %s. '
#                         %(out,reference))


class tsdbaseline_basicTest( tsdbaseline_unittest_base, unittest.TestCase ):
    """
    Basic unit tests for task tsdbaseline. No interactive testing.

    The list of tests:
    test000 --- test polynominal baselining with no mask (maskmode = 'list')
    test001 --- test cubic spline baselining with no mask (maskmode = 'list')
    test050 --- test existing file as outfile with overwrite=False (raises an exception)
    test051 --- test no data after selection (raises an exception)

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

    def setUp( self ):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)

        default(tsdbaseline)

    def tearDown( self ):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)
        #os.system('rm -rf '+self.outroot+'*')

    def test000( self ):
        """Basic Test 000: simple successful case: blfunc = 'poly', maskmode = 'list' and masklist=[] (no mask)"""
        tid = '000'
        infile = self.infile
        outfile = self.outroot+tid+'.ms'
        datacolumn = 'float_data'
        maskmode = 'list'
        blfunc = 'poly'
        spw = '2'
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

        reference = {'rms': 3.4925737380981445,
                     'min': -226.3941650390625,
                     'max': 129.78572082519531,
                     #'max_abscissa': {'value': 8186.0, 'unit': 'channel'},
                     'median': -0.025681495666503906,
                     'stddev': 3.4927871227264404,
                     #'min_abscissa': {'value': 8187.0, 'unit': 'channel'}
                     }

        self._compareStats(theresult, reference)

    def test001( self ):
        """Basic Test 001: simple successful case: blfunc = 'cspline', maskmode = 'list' and masklist=[] (no mask)"""
        print ""

        tid = '001'
        infile = self.infile
        outfile = self.outroot+tid+'.ms'
        datacolumn = 'float_data'  
        maskmode = 'list'
        blfunc = 'cspline'
        overwrite = True
        npiece = 3
        spw='3'
        pol='1'
        scan='0'
        result = tsdbaseline(infile=infile, datacolumn=datacolumn,
                             maskmode=maskmode, blfunc=blfunc, npiece=npiece,
                             spw=spw, 
                             pol=pol,
                             scan=scan,
                             outfile=outfile,overwrite=overwrite)
        #self.assertEqual(result, None,
        #                    msg="The task returned '"+str(result)+"' instead of None")
        

        #***
        #*** check if baseline is correctlt fit and subtracted ***
        #***

        #r1:antenna1=0, data_disc_id=0
        #r2:antenna1=0, data_disc_id=1
        #r3:antenna1=0, data_disc_id=2
        #r4:antenna1=0, data_disc_id=3
        
        ch=10
        
        pol0=0
        pol1=1
        
        orig_pol0_value=[]
        orig_pol1_value=[]

        # open the original MS
        tb.open(infile)
        for i in range(ch):
            orig_pol0_value.append(tb.getcell('FLOAT_DATA', int(spw))[pol0][i])
            orig_pol1_value.append(tb.getcell('FLOAT_DATA', int(spw))[pol1][i])
        tb.close()

        pol0_value=[]
        pol1_value=[]


        #open the MS after tsdbaseline
        tb.open(outfile)
        for i in range(ch):
            pol0_value.append(tb.getcell('FLOAT_DATA', 0)[pol0][i])
            pol1_value.append(tb.getcell('FLOAT_DATA', 0)[pol1][i])
        tb.close()

        
        #when pol selection is 1, baseline subtraction is not performed for pol0
        if(pol=='1'):
            if((numpy.array(pol0_value) == numpy.array(orig_pol0_value)).all()):
                print 'pol0: baseline subtraction is NOT performed. OK'
            if((numpy.array(pol1_value) < numpy.array(orig_pol1_value)).all()):
                print 'pol1: baseline subtraction is performed.OK'





        all_rows=['r1']
        #all_rows=['r1','r2','r3','r4']
        
        #ch=8192 #8192
        ch=1280
        for row in all_rows:
            # after cspline
            tb.open(outfile)
            sum_pol0=0.0
            sum_pol1=0.0
            _sum_pol0=0.0
            _sum_pol1=0.0
        
            for i in range(ch):
                sum_pol0  += tb.getvarcol('FLOAT_DATA')[row][0][i][0]
                sum_pol1  += tb.getvarcol('FLOAT_DATA')[row][1][i][0]
                _sum_pol0 += (tb.getvarcol('FLOAT_DATA')[row][0][i][0])**2
                _sum_pol1 += (tb.getvarcol('FLOAT_DATA')[row][1][i][0])**2
           
            average_sum_pol0 = sum_pol0/float(ch)
            average_sum_pol1 = sum_pol1/float(ch)

            _average_sum_pol0 = _sum_pol0/float(ch)
            _average_sum_pol1 = _sum_pol1/float(ch)

            sigma_pol0= (_average_sum_pol0 - average_sum_pol0**2.0)**0.5
            sigma_pol1= (_average_sum_pol1 - average_sum_pol1**2.0)**0.5
            tb.close()

            # before cspline
            tb.open(infile)
            sum_orig_pol0=0.0
            sum_orig_pol1=0.0
            _sum_orig_pol0=0.0 
            _sum_orig_pol1=0.0 
        
            for i in range(ch):
                sum_orig_pol0  += tb.getvarcol('FLOAT_DATA')[row][0][i][0]
                sum_orig_pol1  += tb.getvarcol('FLOAT_DATA')[row][1][i][0]
                _sum_orig_pol0 += (tb.getvarcol('FLOAT_DATA')[row][0][i][0])**2.0
                _sum_orig_pol1 += (tb.getvarcol('FLOAT_DATA')[row][1][i][0])**2.0
                 
            average_sum_orig_pol0 = sum_orig_pol0/float(ch)
            average_sum_orig_pol1 = sum_orig_pol1/float(ch)

            _average_sum_orig_pol0 = _sum_orig_pol0/float(ch)
            _average_sum_orig_pol1 = _sum_orig_pol1/float(ch)
            
            sigma_orig_pol0= (_average_sum_orig_pol0 - average_sum_orig_pol0**2.0)**0.5
            sigma_orig_pol1= (_average_sum_orig_pol1 - average_sum_orig_pol1**2.0)**0.5
            
            tb.close()

            print ''
            print row, 'npiece: ', npiece
            #print 'average before cspline (pol0)',average_sum_orig_pol0
            #print '_average before cspline(pol0)', _average_sum_orig_pol0
            print '1sigma before cspline (pol0)', sigma_orig_pol0
            print '1sigma after cspline (pol0)', sigma_pol0
            print '1sigma before cspline (pol1)', sigma_orig_pol1
            print '1sigma after cspline (pol1) ', sigma_pol1




    def test050( self ):
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

    def test051( self ):
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


class tsdbaseline_maskTest( tsdbaseline_unittest_base, unittest.TestCase ):
    """
    Tests for various mask selections. No interactive testing.

    The list of tests:
    test101 --- test masklist

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
     
    def setUp( self ):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)
        default(tsdbaseline)

    def tearDown( self ):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)
        os.system('rm -rf '+self.outroot+'*')

    def test101( self ):
        """Mask Test 101: test if user-given mask is correctly applied"""
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

    def _get_range_in_string( self, valrange ):
        if isinstance(valrange, list) or isinstance(valrange, tuple):
            return str(valrange[0])+'~'+str(valrange[1])
        else:
            return False


class tsdbaseline_multi_IF_test( tsdbaseline_unittest_base, unittest.TestCase ):
    """
    Unit tests for task tsdbaseline. No interactive testing.

    This test intends to check whether tsdbaseline task works fine
    for data that has multiple IFs whose nchan differ each other. 

    The list of tests:
    test200 --- test multi IF data input
    """
    # Input and output names
    infile = 'testMultiIF.asap'
    blparamfile_suffix = '_blparam.txt'
    outroot = tsdbaseline_unittest_base.taskname+'_multi'
    refblparamfile = 'refblparam_multiIF'

    def setUp( self ):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)
        default(tsdbaseline)

    def tearDown( self ):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        os.system('rm -rf '+self.outroot+'*')

    def test200( self ):
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


def suite():
    return [tsdbaseline_basicTest, 
            tsdbaseline_maskTest
            #tsdbaseline_multi_IF_test
            ]
