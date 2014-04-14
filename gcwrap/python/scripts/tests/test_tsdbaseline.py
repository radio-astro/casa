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

import asap as sd
from tsdbaseline import tsdbaseline
from sdstat import sdstat

try:
    import selection_syntax
except:
    import tests.selection_syntax as selection_syntax


class tsdbaseline_unittest_base:
    """
    Base class for tsdbaseline unit test
    """
    # Data path of input/output
    datapath = os.environ.get('CASAPATH').split()[0] + \
              '/data/regression/unittest/sdbaseline/'
    taskname = "tsdbaseline"

    #complist = ['max','min','rms','median','stddev']

    ### helper functions for tests ###
    def _checkfile( self, name ):
        isthere=os.path.exists(name)
        self.assertTrue(isthere,
                         msg='Could not find, %s'%(name))

    def _getStats( self, filename, ifno=None ):
        if not ifno:
            ifno=[]
        self._checkfile(filename)
        sd.rcParams['scantable.storage'] = 'memory'
        retstat = sdstat(filename, iflist=ifno)
        return retstat

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
    test01   --- test polynominal baseline with maskmode = 'auto'
    test02   --- test polynominal baseline with maskmode = 'list'
    testwp00   --- test existing file as outfile with overwrite=False (raises an exception)
    testwp01   --- test no data after selection (raises an exception)

    Note: input data is generated from a single dish regression data,
    'OrionS_rawACSmod', as follows:
      default(sdcal)
      sdcal(infile='OrionS_rawACSmod',scanlist=[20,21,22,23],
                calmode='ps',tau=0.09,outfile='temp.asap')
      default(sdcal)
      sdcal(infile='temp.asap',timeaverage=True,
                tweight='tintsys',outfile=self.infile)
    """
    # Input and output names
    #infile = 'OrionS_rawACSmod_calTave.asap'
    infile = 'OrionS_rawACSmod_calave.asap'
    outroot = tsdbaseline_unittest_base.taskname+'_test'
    blrefroot = tsdbaseline_unittest_base.datapath+'refblparam'
    strefroot = tsdbaseline_unittest_base.datapath+'refstats'

    def setUp( self ):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)

        default(tsdbaseline)

    def tearDown( self ):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)

    def test01( self ):
        """Test 0: maskmode = 'auto'"""
        tid = "01"
        infile = self.infile
        outfile = self.outroot+tid+".asap"
        mode = "auto"
        spw = '0,2'
        pol = '0'
        result = tsdbaseline(infile=infile,maskmode=mode,outfile=outfile,
                            blfunc='poly',spw=spw,pol=pol)
        # tsdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self._compareBLparam(outfile+"_blparam.txt",self.blrefroot+tid)
        reference = {'rms': [0.42423143982887268, 0.19752366840839386],
                     'min': [-19.465526580810547, -2.7562465667724609],
                     'max': [14.881179809570312, 2.0289769172668457],
                     'max_abscissa': {'value': array([   21.,  3045.]),
                                      'unit': 'channel'},
                     'median': [0.005268096923828125, 0.0032062530517578125],
                     'min_abscissa': {'value': array([ 18.,   0.]),
                                      'unit': 'channel'},
                     'stddev': [0.42413413524627686, 0.19690337777137756]}
        self._compareStats(outfile,reference)
        #self._compareStats(outfile,self.strefroot+tid)

    def test02( self ):
        """Test 1: maskmode = 'list' and masklist=[] (all channels)"""
        tid = "02"
        infile = self.infile
        outfile = self.outroot+tid+".asap"
        mode = "list"
        spw = '2'
        pol = '1'
        result = tsdbaseline(infile=infile,maskmode=mode,outfile=outfile,
                            spw=spw,pol=pol)
        # tsdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self._compareBLparam(outfile+"_blparam.txt",self.blrefroot+tid)
        reference = {'rms': 3.4925737380981445,
                     'min': -226.3941650390625,
                     'max': 129.78572082519531,
                     'max_abscissa': {'value': 8186.0, 'unit': 'channel'},
                     'median': -0.025681495666503906,
                     'stddev': 3.4927871227264404,
                     'min_abscissa': {'value': 8187.0, 'unit': 'channel'}}
        self._compareStats(outfile,reference)
        #self._compareStats(outfile,self.strefroot+tid)

    def testwp00( self ):
        """Test wp00: Test existing file as outfile with overwrite=False"""
        infile = self.infile
        outfile = "Dummy_Empty.asap"
        mode = "list"
        os.mkdir(outfile)
        try:
            result = tsdbaseline(infile=infile, outfile=outfile, overwrite=False, maskmode=mode)
        except Exception, e:
            pos = str(e).find("Output file 'Dummy_Empty.asap' exists.")
            self.assertNotEqual(pos, -1, msg='Unexpected exception was thrown: %s'%(str(e)))
        finally:
            shutil.rmtree(outfile)

    def testwp01( self ):
        """Test wp01: Test no data after selection"""
        tid = "wp01"
        infile = self.infile
        outfile = self.outroot+tid+".asap"
        spw = '10' # non-existent IF value
        mode = "list"
        try:
            result = tsdbaseline(infile=infile, outfile=outfile, spw=spw, maskmode=mode)
        except Exception, e:
            #pos = str(e).find('Invalid spectral window selection. Selection contains no data.')
            pos = str(e).find('Invalid IF value.')
            self.assertNotEqual(pos, -1, msg='Unexpected exception was thrown: %s'%(str(e)))




class tsdbaseline_maskTest( tsdbaseline_unittest_base, unittest.TestCase ):
    """
    Unit tests for task tsdbaseline. Test various mask selections.
    Polynominal baselining. No interactive testing.

    The list of tests:
    masktest01-02 --- test masklist (list)
    masktest03-04 --- test masklist (string)
    masktest05-08 --- test specunit='GHz'
    masktest09-12 --- test specunit='km/s'

    Note: input data is generated from a single dish regression data,
    'OrionS_rawACSmod', as follows:
      default(sdcal)
      sdcal(infile='OrionS_rawACSmod',scanlist=[20,21,22,23],
                calmode='ps',tau=0.09,outfile='temp.asap')
      default(sdcal)
      sdcal(infile='temp.asap',timeaverage=True,
                tweight='tintsys',outfile=self.infile)
    """
    # Input and output names
    #infile = 'OrionS_rawACSmod_calTave.asap'
    infile = 'OrionS_rawACSmod_calave.asap'
    outroot = tsdbaseline_unittest_base.taskname+'_masktest'
    blrefroot = tsdbaseline_unittest_base.datapath+'refblparam_mask'
    #strefroot = tsdbaseline_unittest_base.datapath+'refstats_mask'
    tid = None

    # Channel range excluding bad edge
    search = [[200,7599]]
    # Baseline channels. should be identical to one selected by 'auto' mode
    blchan0 = [[200,3979],[4152,7599]]
    blchan2 = [[200,2959],[3120,7599]]

    # reference values for specunit='channel'
    ref_pol0if0 =  {'linemaxpos': 4102.0, 'linesum': 103.81604766845703,
                    'linemax': 1.6280698776245117,
                    'baserms': 0.15021507441997528,
                    'basestd': 0.15022546052932739}
    ref_pol0if2 = {'linemaxpos': 3045.0, 'linesum': 127.79755401611328,
                   'linemax': 2.0193681716918945,
                   'baserms': 0.13134850561618805,
                   'basestd': 0.1313575953245163}
    # reference values for specunit='GHz'
    ref_pol0if0f = {'linemaxpos': 4102.0, 'linesum': 103.81603241,
                    'linemax': 1.6132903099060059,
                    'baserms': 0.150215059519, 
                    'basestd': 0.150225445628}
    ref_pol0if2f = {'linemaxpos': 3045.0, 'linesum': 127.797515869,
                    'linemax': 2.0308537483215332,
                    'baserms': 0.13158561289310455,
                    'basestd': 0.13159450888633728}

    # reference values for specunit='km/s'
    ref_pol0if0v = {'linemaxpos': 4102.0, 'linesum': 103.81607055664062,
                    'linemax': 1.6280698776245117,
                    'baserms': 0.15021507441997528,
                    'basestd': 0.15022547543048859}
    ref_pol0if2v = {'linemaxpos': 3045.0, 'linesum': 128.9298095703125,
                    'linemax': 2.0264592170715332,
                    'baserms': 0.13150280714035034,
                    'basestd': 0.13151165843009949}
     
    def setUp( self ):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)

        default(tsdbaseline)


    def tearDown( self ):
#        self._compareBLparam(self.outroot+self.tid+'.asap_blparam.txt',\
#                             self.blrefroot+self.tid)
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)


    def testblmask01( self ):
        """Mask test 1: test masklist (list) with maskmode = 'auto'"""
        self.tid="01"
        infile = self.infile
        outfile = self.outroot+self.tid+".asap"
        mode = "auto"
        masklist = self._get_range_in_string(self.search[0])
        spw = '0:%s,2:%s'%(masklist,masklist)
        pol = '0'

        print "spw =", spw

        result = tsdbaseline(infile=infile,maskmode=mode,
                            outfile=outfile,spw=spw,pol=pol)
        # tsdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF0
        testval = self._getStats(outfile,self.blchan0,0)
        self._compareStats(testval,self.ref_pol0if0)
        # Compare IF2
        testval = self._getStats(outfile,self.blchan2,2)
        self._compareStats(testval,self.ref_pol0if2)
        self._compareBLparam(self.outroot+self.tid+'.asap_blparam.txt',\
                             self.blrefroot+self.tid)

    def testblmask02( self ):
        """Mask test 2: test masklist (list) with maskmode = 'list'"""
        self.tid="02"
        infile = self.infile
        outfile = self.outroot+self.tid+".asap"
        mode = "list"
        spw = '2:%s'%(';'.join(map(self._get_range_in_string,self.blchan2)))
        pol = '0'

        print "spw =", spw

        result = tsdbaseline(infile=infile,maskmode=mode,
                            outfile=outfile,spw=spw,pol=pol)
        # tsdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF2
        testval = self._getStats(outfile,self.blchan2,2)
        self._compareStats(testval,self.ref_pol0if2)
        self._compareBLparam(self.outroot+self.tid+'.asap_blparam.txt',\
                             self.blrefroot+self.tid)

    def testblmask03( self ):
        """Mask test 3: test masklist (string) with maskmode = 'auto'"""
        self.tid="03"
        infile = self.infile
        outfile = self.outroot+self.tid+".asap"
        mode = "auto"
        #masklist = "0~2:200~7599"
        range_in_string = self._get_range_in_string(self.search[0])
        spw = '0:%s,2:%s'%(range_in_string,range_in_string)
        pol = '0'

        print "spw =", spw

        result = tsdbaseline(infile=infile,maskmode=mode,
                            outfile=outfile,spw=spw,pol=pol)
        # tsdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF0
        testval = self._getStats(outfile,self.blchan0,0)
        self._compareStats(testval,self.ref_pol0if0)
        # Compare IF2
        testval = self._getStats(outfile,self.blchan2,2)
        self._compareStats(testval,self.ref_pol0if2)
        self._compareBLparam(self.outroot+self.tid+'.asap_blparam.txt',\
                             self.blrefroot+self.tid)

    def testblmask04( self ):
        """Mask test 4: test masklist (string) with maskmode = 'list'"""
        self.tid="04"
        infile = self.infile
        outfile = self.outroot+self.tid+".asap"
        mode = "list"
        #masklist = "0:200~3979;4152~7599, 2:200~2959;3120~7599"
        spw = ','.join(['0:%s'%(';'.join(map(self._get_range_in_string,self.blchan0))), '2:%s'%(';'.join(map(self._get_range_in_string,self.blchan2)))])

        print "spw =", spw
        
        pol = '0'

        result = tsdbaseline(infile=infile,maskmode=mode,
                            outfile=outfile,spw=spw,pol=pol)
        # tsdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF0
        testval = self._getStats(outfile,self.blchan0,0)
        self._compareStats(testval,self.ref_pol0if0)
        # Compare IF2
        testval = self._getStats(outfile,self.blchan2,2)
        self._compareStats(testval,self.ref_pol0if2)
        self._compareBLparam(self.outroot+self.tid+'.asap_blparam.txt',\
                             self.blrefroot+self.tid)

    def testblmask05( self ):
        """Mask test 5: test specunit='GHz' with masklist (list) and maskmode = 'auto'"""
        self.tid="05"
        infile = self.infile
        outfile = self.outroot+self.tid+".asap"
        mode = "auto"
        specunit = "GHz"
        #masklist = [[44.0511472,44.0963125]]
        iflist = [2]

        masklist = self._get_chanval(infile,self.search,specunit,spw=iflist[0],addedge=True)
        print "masklist =", masklist
        spw = '2:%s%s'%(self._get_range_in_string(masklist[0]),specunit)
        print "spw =", spw

        pol = '0'
        result = tsdbaseline(infile=infile,maskmode=mode,
                            outfile=outfile,spw=spw,pol=pol)
        # tsdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF2
        testval = self._getStats(outfile,self.blchan2,2)
        self._compareStats(testval,self.ref_pol0if2f)

    def testblmask06( self ):
        """Mask test 6: test specunit='GHz' with masklist (list) and maskmode = 'list'"""
        self.tid="06"
        infile = self.infile
        outfile = self.outroot+self.tid+".asap"
        mode = "list"
        specunit = "GHz"
        #masklist = [[44.0511472,44.0679889],[44.0689716,44.0963125]]
        iflist = [2]
        pollist=[0]

        masklist = self._get_chanval(infile,self.blchan2,specunit,spw=iflist[0],addedge=True)
        print "masklist =", masklist

        spw = '2:%s'%(';'.join(map(lambda x: x + specunit, [self._get_range_in_string(m) for m in masklist])))
        pol = '0'
        print "spw =", spw
        
        result = tsdbaseline(infile=infile,maskmode=mode,
                            outfile=outfile,spw=spw,pol=pol)
        # tsdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF2
        testval = self._getStats(outfile,self.blchan2,2)
        self._compareStats(testval,self.ref_pol0if2f)

    def testblmask07( self ):
        """Mask test 7: test specunit='GHz' with masklist (string) and maskmode = 'auto'"""
        self.tid="07"
        infile = self.infile
        outfile = self.outroot+self.tid+".asap"
        mode = "auto"
        specunit = "GHz"
        #masklist = "0:45.4655714~45.5107367, 2:44.0511472~44.0963125"
        iflist = [0,2]
        pollist=[0]

        sblrange=[]
        chanlist = (self.search, self.search)
        for i in xrange(len(iflist)):
            sblrange.append("")
            mlist = self._get_chanval(infile,chanlist[i],specunit,spw=iflist[i],addedge=True)
            for valrange in mlist:
                if len(sblrange[i]): sblrange[i] += ";"
                sblrange[i] += self._get_range_in_string(valrange)
        masklist = "0:"+sblrange[0]+", 2:"+sblrange[1]
        print "masklist =", masklist

        pol = '0'
        spw = ','.join(map(lambda x: x + specunit, masklist.split(',')))
        print "spw = ", spw
        
        result = tsdbaseline(infile=infile,maskmode=mode,
                            outfile=outfile,spw=spw,pol=pol)
        # tsdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF0
        testval = self._getStats(outfile,self.blchan0,0)
        self._compareStats(testval,self.ref_pol0if0f)
        # Compare IF2
        testval = self._getStats(outfile,self.blchan2,2)
        self._compareStats(testval,self.ref_pol0if2f)

    def testblmask08( self ):
        """Mask test 8: test specunit='GHz' with masklist (string) and maskmode = 'list'"""
        self.tid="08"
        infile = self.infile
        outfile = self.outroot+self.tid+".asap"
        mode = "list"
        specunit = "GHz"
        #masklist = "0:45.4655714~45.4886394;45.4896954~45.5107367, 2:44.0511472~44.0679889;44.0689716~44.0963125"
        iflist = [0,2]
        pollist=[0]

        sblrange=[]
        chanlist = (self.blchan0, self.blchan2)
        for i in xrange(len(iflist)):
            sblrange.append("")
            mlist = self._get_chanval(infile,chanlist[i],specunit,spw=iflist[i],addedge=True)
            for valrange in mlist:
                if len(sblrange[i]): sblrange[i] += ";"
                sblrange[i] += self._get_range_in_string(valrange)
        masklist = "0:"+sblrange[0]+", 2:"+sblrange[1]
        print "masklist =", masklist

        pol = '0'
        spw = ','.join([';'.join(map(lambda x: x + specunit, m.split(';'))) for m in masklist.split(',')])
        print "spw = ", spw

        result = tsdbaseline(infile=infile,maskmode=mode,
                            outfile=outfile,spw=spw,pol=pol)
        # tsdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF0
        testval = self._getStats(outfile,self.blchan0,0)
        self._compareStats(testval,self.ref_pol0if0f)
        # Compare IF2
        testval = self._getStats(outfile,self.blchan2,2)
        self._compareStats(testval,self.ref_pol0if2f)

    def testblmask09( self ):
        """Mask test 9: test specunit='km/s' with masklist (list) and maskmode = 'auto'"""
        self.tid="09"
        infile = self.infile
        outfile = self.outroot+self.tid+".asap"
        mode = "auto"
        specunit = "km/s"
        #masklist = [[9186.458, 9484.109]]
        iflist = [2]
        pollist=[0]

        masklist = self._get_chanval(infile,self.search,specunit,spw=iflist[0],addedge=True)
        print "masklist =", masklist
        spw = '2:%s'%(';'.join(map(lambda x: x + specunit, [self._get_range_in_string(m) for m in masklist])))
        pol = '0'
        
        print "spw =", spw
        
        result = tsdbaseline(infile=infile,maskmode=mode,
                            outfile=outfile,spw=spw,pol=pol)
        # tsdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF2
        testval = self._getStats(outfile,self.blchan2,2)
        self._compareStats(testval,self.ref_pol0if2v)

    def testblmask10( self ):
        """Mask test 10: test specunit='km/s' with masklist (list) and maskmode = 'list'"""
        self.tid="10"
        infile = self.infile
        outfile = self.outroot+self.tid+".asap"
        mode = "list"
        specunit = "km/s"
        #masklist = [[9186.458, 9366.642],[9373.118, 9484.109]]
        iflist = [2]
        pollist=[0]

        masklist = self._get_chanval(infile,self.blchan2,specunit,spw=iflist[0],addedge=True)
        print "masklist =", masklist

        masklist.reverse()
        spw = '2:%s'%(';'.join(map(lambda x: x + specunit,[self._get_range_in_string(m) for m in masklist])))
        pol = '0'

        print "spw =", spw
        
        result = tsdbaseline(infile=infile,maskmode=mode,
                            outfile=outfile,spw=spw,pol=pol)
        # tsdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF2
        testval = self._getStats(outfile,self.blchan2,2)
        self._compareStats(testval,self.ref_pol0if2v)

    def testblmask11( self ):
        """Mask test 11: test specunit='km/s' with masklist (string) and maskmode = 'auto'"""
        self.tid="11"
        infile = self.infile
        outfile = self.outroot+self.tid+".asap"
        mode = "auto"
        specunit = "km/s"
        #masklist = "0:-134.960~162.691, 2:9186.458~9484.109"
        iflist = [0,2]
        pollist=[0]

        sblrange=[]
        chanlist = (self.search, self.search)
        for i in xrange(len(iflist)):
            sblrange.append("")
            mlist = self._get_chanval(infile,chanlist[i],specunit,spw=iflist[i],addedge=True)
            for valrange in mlist:
                if len(sblrange[i]): sblrange[i] += ";"
                sblrange[i] += self._get_range_in_string(valrange)
        masklist = "0:"+sblrange[0]+", 2:"+sblrange[1]
        print "masklist =", masklist

        pol = '0'
        spw = ','.join(map(lambda x: x + specunit, masklist.split(',')))
        print "spw =", spw

        result = tsdbaseline(infile=infile,maskmode=mode,
                            outfile=outfile,spw=spw,pol=pol)
        # tsdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF0
        testval = self._getStats(outfile,self.blchan0,0)
        self._compareStats(testval,self.ref_pol0if0v)
        # Compare IF2
        testval = self._getStats(outfile,self.blchan2,2)
        self._compareStats(testval,self.ref_pol0if2v)

    def testblmask12( self ):
        """Mask test 12: test specunit='km/s' with masklist (string) and maskmode = 'list'"""
        self.tid="12"
        infile = self.infile
        outfile = self.outroot+self.tid+".asap"
        mode = "list"
        specunit = "km/s"
        #masklist = "0:-134.960~3.708;10.667~162.691, 2:9373.118~9484.109;9186.458~9366.642"
        iflist = [0,2]
        pollist=[0]

        sblrange=[]
        chanlist = (self.blchan0, self.blchan2)
        for i in xrange(len(iflist)):
            sblrange.append("")
            mlist = self._get_chanval(infile,chanlist[i],specunit,spw=iflist[i],addedge=True)
            for valrange in mlist:
                if len(sblrange[i]): sblrange[i] += ";"
                sblrange[i] += self._get_range_in_string(valrange)
        masklist = "0:"+sblrange[0]+", 2:"+sblrange[1]
        print "masklist =", masklist

        pol = '0'
        spw = ','.join([';'.join(map(lambda x: x + specunit, m.split(';'))) for m in masklist.split(',')])
        print "spw = ", spw

        result = tsdbaseline(infile=infile,maskmode=mode,
                            outfile=outfile,spw=spw,pol=pol)
        # tsdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        # Compare IF0
        testval = self._getStats(outfile,self.blchan0,0)
        self._compareStats(testval,self.ref_pol0if0v)
        # Compare IF2
        testval = self._getStats(outfile,self.blchan2,2)
        self._compareStats(testval,self.ref_pol0if2v)


    def _get_range_in_string( self, valrange ):
        if isinstance(valrange, list) or isinstance(valrange, tuple):
            return str(valrange[0])+"~"+str(valrange[1])
        else:
            return False

    def _get_chanval( self,file, chanrange, unit, spw=0, addedge=False ):
        mylist = []
        scan = sd.scantable(file, average=False)
        scan.set_unit(unit)
        scan.set_selection(ifs=[spw])
        chanval = scan._getabcissa(0)
        edge = 0
        if addedge:
            # add 1/2 chan to both edges
            nchan = len(chanval)
            #edge = 0.0
            edge = 0.5*abs(chanval[nchan-1]-chanval[0])/float(nchan-1)
        for schan, echan in chanrange:
            lval = max(chanval[schan],chanval[echan])
            sval = min(chanval[schan],chanval[echan])
            mylist.append([sval-edge,lval+edge])
        del scan, nchan, edge, lval, sval
        return mylist

    def _getStats( self, filename, basechan, ispw ):
        self.assertTrue(os.path.exists(filename),
                        msg=("Output file '%s' doesn't exist" % filename))
        linechan = [basechan[0][1]+1,basechan[1][0]-1]
        scan = sd.scantable(filename,average=False)
        scan.set_selection(ifs=[ispw])
        scan.set_unit('channel')
        linmsk = scan.create_mask(linechan)
        blmsk = scan.create_mask(basechan)
        # only the fist row is returned
        linmax = scan.stats('max',mask=linmsk)[0]
        linmaxpos = scan.stats('max_abc',mask=linmsk)[0]
        linesum = scan.stats('sum',mask=linmsk)[0]
        blrms = scan.stats('rms',mask=blmsk)[0]
        blstd = scan.stats('stddev',mask=blmsk)[0]
        del scan, linmsk, blmsk
        retdic = {'linemax': linmax, 'linemaxpos': linmaxpos,
                  'linesum': linesum, 'baserms': blrms, 'basestd': blstd}
        del linmax, linmaxpos, linesum, blrms, blstd
        print 'Current run (IF',ispw,'):',retdic
        return retdic
 

class tsdbaseline_funcTest( unittest.TestCase ):
    """
    Unit tests for task tsdbaseline. No interactive testing.

    The list of tests:
    testCSpline01  --- test cubic spline fitting with maskmode = 'list'
    testCSpline02  --- test cubic spline fitting with maskmode = 'auto'
    testSinusoid01 --- test sinusoidal fitting with maskmode = 'list'
    testSinusoid02 --- test sinusoidal fitting with maskmode = 'auto'

    Note: (1) the rms noise of input data is 1.0.
          (2) the max_rms value comes from +3sigma level of Chi-square
              distribution with freedom of (2048-38) for cspline, (2048-27)
              for sinusoid, respectively.

    created 19/04/2011 by Wataru Kawasaki
    """
    # Data path of input/output
    datapath = os.environ.get('CASAPATH').split()[0] + \
              '/data/regression/unittest/sdbaseline/'    
    # Input and output names
    infile_cspline  = 'Artificial_CubicSpline.asap'
    infile_sinusoid = 'Artificial_Sinusoid.asap'
    blparamfile_suffix = '_blparam.txt'
    outroot = 'tsdbaseline_test'
    tid = None

    def setUp( self ):
        if os.path.exists(self.infile_cspline):
            shutil.rmtree(self.infile_cspline)
        shutil.copytree(self.datapath+self.infile_cspline, self.infile_cspline)
        if os.path.exists(self.infile_sinusoid):
            shutil.rmtree(self.infile_sinusoid)
        shutil.copytree(self.datapath+self.infile_sinusoid, self.infile_sinusoid)

        default(tsdbaseline)

    def tearDown( self ):
        if os.path.exists(self.infile_cspline):
            shutil.rmtree(self.infile_cspline)
        if os.path.exists(self.infile_sinusoid):
            shutil.rmtree(self.infile_sinusoid)

    def testCSpline01( self ):
        """Test CSpline01: Cubic spline fitting with maskmode = 'list'"""
        self.tid = "CSpline01"
        infile = self.infile_cspline
        mode = "list"
        outfile = self.outroot+self.tid+".asap"
        blparamfile = outfile+self.blparamfile_suffix
        
        result = tsdbaseline(infile=infile,maskmode=mode,outfile=outfile,blfunc='cspline',npiece=35)
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")
        self.checkRms(blparamfile, 1.038696)   #the actual rms should be 1.02407 though

    def testCSpline02( self ):
        """Test CSpline02: Cubic spline fitting with maskmode = 'auto'"""
        self.tid = "CSpline02"
        infile = self.infile_cspline
        mode = "auto"
        outfile = self.outroot+self.tid+".asap"
        blparamfile = outfile+self.blparamfile_suffix
        
        result = tsdbaseline(infile=infile,maskmode=mode,outfile=outfile,blfunc='cspline',npiece=35)
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")
        self.checkRms(blparamfile, 1.038696)   #the actual rms should be 1.02407 though

    def testSinusoid01( self ):
        """Test Sinusoid01: Sinusoidal fitting with maskmode = 'list'"""
        self.tid = "Sinusoid01"
        infile = self.infile_sinusoid
        mode = "list"
        outfile = self.outroot+self.tid+".asap"
        blparamfile = outfile+self.blparamfile_suffix
        
        result = tsdbaseline(infile=infile,maskmode=mode,outfile=outfile,blfunc='sinusoid')
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")
        self.checkRms(blparamfile, 1.10)   #the actual rms should be 1.09705 though

    def testSinusoid02( self ):
        """Test Sinusoid02: Sinusoidal fitting with maskmode = 'auto'"""
        self.tid = "Sinusoid02"
        infile = self.infile_sinusoid
        mode = "auto"
        outfile = self.outroot+self.tid+".asap"
        blparamfile = outfile+self.blparamfile_suffix
        
        result = tsdbaseline(infile=infile,maskmode=mode,outfile=outfile,blfunc='sinusoid')
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")
        self.checkRms(blparamfile, 1.10)   #the actual rms should be 1.09705 though

    def checkRms( self, blparamfile, max_rms ):
        rms = 10000.0
        for line in open(blparamfile,'r'):
            if line.strip() != "":
                items = line.split()
                if len(items) >= 3:
                    if (items[0] == 'rms') and (items[1] == '='):
                        rms = float(items[2])
                        break

        self.assertTrue((rms <= max_rms), msg = "CSpline fitting failed.")


class tsdbaseline_multi_IF_test( tsdbaseline_unittest_base, unittest.TestCase ):
    """
    Unit tests for task tsdbaseline. No interactive testing.

    This test intends to check whether tsdbaseline task works fine
    for data that has multiple IFs whose nchan differ each other. 

    The list of tests:
    test0 --- test multi IF data input

    created 24/02/2012 by Takeshi Nakazato
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

    def test01multi( self ):
        """test01multi: Test the task works with multi IF data"""
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
            currstat = self._getStats(outfile,[ifno])
            self._compareStats(currstat,reference[ifno])

class tsdbaseline_storageTest( tsdbaseline_unittest_base, unittest.TestCase ):
    """
    Unit tests for task tsdbaseline. Test scantable sotrage and insitu
    parameters

    The list of tests:
    testMT   --- storage = 'memory', insitu = True
    testMF   --- storage = 'memory', insitu = False
    testDT   --- storage = 'disk', insitu = True
    testDF   --- storage = 'disk', insitu = False
    testDTow --- infile=outfile on storage = 'disk', insitu = True

    Note on handlings of disk storage:
       Task script copies scantable after setting selection when storage='disk'
    """
    # Input and output names
    infile = 'OrionS_rawACSmod_calave.asap'
    outroot = tsdbaseline_unittest_base.taskname+'_store'
    mode = "list"
    #iflist = [2]
    #pollist = [1]
    spw = '2'
    pol = '1'
    
    blparamfile_suffix = '_blparam.txt'
    blreffile = tsdbaseline_unittest_base.datapath+'refblparam02'

    # Reference statistic values of IF=2, POL=1
    refstat =  {'rms': 3.4925737380981445,
                'min': -226.3941650390625,
                'max': 129.78572082519531,
                'max_abscissa': {'value': 8186.0, 'unit': 'channel'},
                'median': -0.025681495666503906,
                'stddev': 3.4927871227264404,
                'min_abscissa': {'value': 8187.0, 'unit': 'channel'}}

    def setUp( self ):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)
        default(tsdbaseline)

    def tearDown( self ):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)

    def testMT( self ):
        """Storage Test MT: storage='memory' and insitu=T"""
        tid = "MT"
        outfile = self.outroot+tid+".asap"

        initstat = self._getStats(self.infile)

        sd.rcParams['scantable.storage'] = 'memory'
        sd.rcParams['insitu'] = True
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        result = tsdbaseline(infile=self.infile,maskmode=self.mode,outfile=outfile,
                            spw=self.spw,pol=self.pol)

        # tsdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        print "Testing OUTPUT statistics and baseline parameters"
        self._compareBLparam(outfile+self.blparamfile_suffix,self.blreffile)
        self._compareStats(outfile,self.refstat)
        # Test input data
        newinstat = self._getStats(self.infile)
        print "Comparing INPUT statistics before/after calculations"
        self._compareStats(newinstat,initstat)

    def testMF( self ):
        """Storage Test MF: storage='memory' and insitu=F"""
        tid = "MF"
        outfile = self.outroot+tid+".asap"

        initstat = self._getStats(self.infile)

        sd.rcParams['scantable.storage'] = 'memory'
        sd.rcParams['insitu'] = False
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        result = tsdbaseline(infile=self.infile,maskmode=self.mode,outfile=outfile,
                            spw=self.spw,pol=self.pol)

        # tsdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        print "Testing OUTPUT statistics and baseline parameters"
        self._compareBLparam(outfile+self.blparamfile_suffix,self.blreffile)
        self._compareStats(outfile,self.refstat)
        # Test input data
        newinstat = self._getStats(self.infile)
        print "Comparing INPUT statistics before/after calculations"
        self._compareStats(newinstat,initstat)

    def testDT( self ):
        """Storage Test DT: storage='disk' and insitu=T"""
        tid = "DT"
        outfile = self.outroot+tid+".asap"

        initstat = self._getStats(self.infile)

        sd.rcParams['scantable.storage'] = 'disk'
        sd.rcParams['insitu'] = True
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        result = tsdbaseline(infile=self.infile,maskmode=self.mode,outfile=outfile,
                            spw=self.spw,pol=self.pol)

        # tsdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        print "Testing OUTPUT statistics and baseline parameters"
        self._compareBLparam(outfile+self.blparamfile_suffix,self.blreffile)
        self._compareStats(outfile,self.refstat)
        # Test input data
        newinstat = self._getStats(self.infile)
        print "Comparing INPUT statistics before/after calculations"
        self._compareStats(newinstat,initstat)

    def testDF( self ):
        """Storage Test DF: storage='disk' and insitu=F"""
        tid = "DF"
        outfile = self.outroot+tid+".asap"

        initstat = self._getStats(self.infile)

        sd.rcParams['scantable.storage'] = 'disk'
        sd.rcParams['insitu'] = False
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        result = tsdbaseline(infile=self.infile,maskmode=self.mode,outfile=outfile,
                            spw=self.spw,pol=self.pol)

        # tsdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        print "Testing OUTPUT statistics and baseline parameters"
        self._compareBLparam(outfile+self.blparamfile_suffix,self.blreffile)
        self._compareStats(outfile,self.refstat)
        # Test input data
        newinstat = self._getStats(self.infile)
        print "Comparing INPUT statistics before/after calculations"
        self._compareStats(newinstat,initstat)


    def testDTow( self ):
        """Storage Test DTow:  infile=outfile on storage='disk' and insitu=T"""
        tid = "DTow"
        infile = self.infile
        outfile = self.infile
        overwrite = True

        sd.rcParams['scantable.storage'] = 'disk'
        sd.rcParams['insitu'] = True
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        result = tsdbaseline(infile=infile,maskmode=self.mode,outfile=outfile,
                            spw=self.spw,pol=self.pol,overwrite=True)

        # tsdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        print "Testing OUTPUT statistics and baseline parameters"
        self._compareBLparam(outfile+self.blparamfile_suffix,self.blreffile)
        self._compareStats(outfile,self.refstat)

class tsdbaseline_selection_syntax(selection_syntax.SelectionSyntaxTest):
    
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/singledish/'
    # Input and output names
    infile = 'sd_analytic_type1-3.cal.asap'
    #workfile = 'sd_analytic_type1-3.cal.asap_work'
    outfile = 'tsdbaseline_selection_syntax.asap'
    blparamfile = outfile + '_blparam.txt'
    order = 5

    # line information
    # | row | line channel | intensity |
    # | 0   | 20           | 5         |
    # | 1   | 40           | 10        |
    # | 2   | 60           | 20        |
    # | 3   | 80           | 30        |
    line_location = [20, 40, 60, 80]

    # reference values for baseline fit
    fit_ref = {(15,11,23,0): [1.0],
               (16,12,25,1): [0.2, 0.02],
               (16,13,21,0): [2.44, -0.048, 0.0004],
               (17,13,23,1): [-3.096, 0.1536, -0.00192, 8.0e-6]}

    # tolerance
    tol_partial = 1.0e-3
    tol_full = 1.0e-5
    tol_coeff = 1.0e-6
    
    @property
    def task(self):
        return tsdbaseline

    @property
    def spw_channel_selection(self):
        return True

    def setUp(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)
        default(tsdbaseline)

    def tearDown(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        #if os.path.exists(self.workfile):
        #    shutil.rmtree(self.workfile)
        if os.path.exists(self.outfile):
            shutil.rmtree(self.outfile)
        if os.path.exists(self.blparamfile):
            os.remove(self.blparamfile)

    def _flag_lines(self):
        s = sd.scantable(self.infile, average=False)
        nrow = s.nrow()
        
        # flag lines
        self.assertEqual(nrow, len(self.line_location))
        for i in xrange(nrow):
            line = self.line_location[i]
            line_mask = s.create_mask([line,line], row=i)
            s.flag(line_mask, row=i)
        s.save(self.infile, format='ASAP', overwrite=True)

        del s

        # check if data is edited as expected
        s = sd.scantable(self.infile, average=False)
        self.assertEqual(nrow, s.nrow())
        for i in xrange(nrow):
            idx = numpy.where(numpy.array(s.get_mask(i)) == False)[0]
            self.assertEqual(1, len(idx))
            self.assertEqual(self.line_location[i], idx[0])
        del s

    def __test_result(self, spw):
        # basic check
        #   - check if self.outfile exists
        #   - check if self.outfile is a directory
        #   - check if self.blparamfile exists
        #   - check if self.blparamfile is a regular file
        self.assertTrue(os.path.exists(self.outfile))
        self.assertTrue(os.path.isdir(self.outfile))
        self.assertTrue(os.path.exists(self.blparamfile))
        self.assertTrue(os.path.isfile(self.blparamfile))
        
        s = sd.scantable(self.outfile, average=False)
        nrow = s.nrow()
        spw_selection = s.parse_spw_selection(spw)

        # length check
        #   - check if length of refids is equal to nrow of self.outfile
        #self.assertEqual(nrow, len(refids))

        # result check
        #   - check if resulting spectral data have small value
        #   - check if fit coefficients are equal to reference values
        blparam_list = list(self.__read_blparamfile())
        for i in xrange(nrow):
            mask = numpy.logical_not(s.get_mask(i))
            sp = numpy.ma.masked_array(s._getspectrum(i),mask)
            ifno = s.getif(i)
            nchan = s.nchan(ifno)
            chrange = [map(int, l) for l in spw_selection[ifno]]
            if len(chrange) == 1 and chrange[0] == 0 \
                    and chrange[1] == nchan - 1:
                tol = self.tol_full
            else:
                tol = self.tol_partial
            for _chrange in chrange:
                start = _chrange[0]
                end = _chrange[1] + 1
                #self.assertTrue(numpy.all(abs(sp[start:end]) < tol))
                maxdiff = abs(sp[start:end]).max()
                casalog.post('maxdiff = %s (tol %s)'%(maxdiff,tol))
                self.assertLess(maxdiff, tol)

            (key, coeffs) = blparam_list[i]
            self.assertTrue(self.fit_ref.has_key(key))
            self.assertEqual(self.order+1, len(coeffs))
            ref_coeff = self.fit_ref[key]
            ncoeff = len(ref_coeff)
            for j in xrange(ncoeff):
                ref = ref_coeff[j]
                val = coeffs[j]
                diff = abs((val - ref)/ref)
                casalog.post('ref: %s, val: %s, diff: %s'%(ref, val, diff))
                self.assertLess(diff, tol)
            casalog.post('coeffs=%s'%(coeffs))
            for j in xrange(ncoeff, self.order+1):
                casalog.post('coeffs[%s]: %s (%s)'%(j,coeffs[j],abs(coeffs[j])))
                self.assertLess(abs(coeffs[j]), self.tol_coeff)

        del s

    def __read_blparamfile(self):
        with open(self.blparamfile, 'r') as f:
            for line in f:
                s = line.split(',')
                scanno = int(s[0])
                beamno = int(s[1])
                ifno = int(s[2])
                polno = int(s[3])
                coeffs = map(float, s[6:-2])
                yield (scanno, beamno, ifno, polno), coeffs
        
    def __exec_complex_test(self, params, exprs, values, columns, expected_nrow, regular_test=True):
        num_param = len(params)
        test_name = self._get_test_name(regular_test)
        #outfile = '.'.join([self.prefix, test_name])
        outfile = self.outfile
        #print 'outfile=%s'%(outfile)
        casalog.post('%s: %s'%(test_name, ','.join(['%s = \'%s\''%(params[i],exprs[i]) for i in xrange(num_param)])))
        kwargs = {'infile': self.infile,
                  'outfile': self.outfile,
                  'outform': 'ASAP',
                  'overwrite': True,
                  'blfunc': 'poly',
                  'order': self.order,
                  'maskmode': 'list',
                  'bloutput': True,
                  'blformat': 'csv'}
        for i in xrange(num_param):
            kwargs[params[i]] = exprs[i]

        if regular_test:
            self.run_task(**kwargs)
        else:
            tsdbaseline(**kwargs)

        tb.open(outfile)
        cols = [tb.getcol(columns[i]) for i in xrange(num_param)]
        nrow = tb.nrows()
        tb.close()
        casalog.post('expected nrow = %s, actual nrow = %s'%(expected_nrow, nrow))
        self.assertEqual(expected_nrow, nrow)
        for i in xrange(num_param):
            casalog.post('expected values = %s, actual values = %s'%(set(values[i]), set(cols[i])))
            self.assertEqual(set(values[i]), set(cols[i]))

        # channel range selection should be taken into account
        if 'spw' in params:
            spw = exprs[params.index('spw')]
        else:
            spw = ''
        self.__test_result(spw)
                          
        return outfile

    def __exec_simple_test(self, param, expr, value_list, column, expected_nrow, regular_test=True):
        return self.__exec_complex_test([param], [expr], [value_list], [column],
                                        expected_nrow, regular_test)
    
    def prepare(func):
        import functools
        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            # first argument is 'self'
            obj = args[0]
            obj._flag_lines()
            return func(*args, **kwargs)
        return wrapper

    ### field selection syntax test ###
    @prepare
    def test_field_value_default(self):
        """test_field_value_default: Test default value for field"""
        field = ''
        expected_nrow = 4
        fieldlist = ['M100__5', 'M100__6', 'M30__7', '3C273__8']

        self.__exec_simple_test('field', field, fieldlist, 'FIELDNAME', expected_nrow)
        
    @prepare
    def test_field_id_exact(self):
        """test_field_id_exact: Test field selection by id"""
        field = '5'
        expected_nrow = 1
        fieldlist = ['M100__5']

        self.__exec_simple_test('field', field, fieldlist, 'FIELDNAME', expected_nrow)
        
    @prepare
    def test_field_id_lt(self):
        """test_field_id_lt: Test field selection by id (<N)"""
        field = '<7'
        expected_nrow = 2
        fieldlist = ['M100__5', 'M100__6']

        self.__exec_simple_test('field', field, fieldlist, 'FIELDNAME', expected_nrow)

    @prepare
    def test_field_id_gt(self):
        """test_field_id_gt: Test field selection by id (>N)"""
        field = '>6'
        expected_nrow = 2
        fieldlist = ['M30__7', '3C273__8']

        self.__exec_simple_test('field', field, fieldlist, 'FIELDNAME', expected_nrow)

    @prepare
    def test_field_id_range(self):
        """test_field_id_range: Test field selection by id ('N~M')"""
        field = '6~7'
        expected_nrow = 2
        fieldlist = ['M100__6', 'M30__7']

        self.__exec_simple_test('field', field, fieldlist, 'FIELDNAME', expected_nrow)

    @prepare
    def test_field_id_list(self):
        """test_field_id_list: Test field selection by id ('N,M')"""
        field = '5,8'
        expected_nrow = 2
        fieldlist = ['M100__5', '3C273__8']

        self.__exec_simple_test('field', field, fieldlist, 'FIELDNAME', expected_nrow)

    @prepare
    def test_field_id_exprlist(self):
        """test_field_id_exprlist: Test field selection by id ('EXPR0,EXPR1')"""
        field = '<6,7~8'
        expected_nrow = 3
        fieldlist = ['M100__5', 'M30__7', '3C273__8']

        self.__exec_simple_test('field', field, fieldlist, 'FIELDNAME', expected_nrow)

    @prepare
    def test_field_value_exact(self):
        """test_field_value_exact: Test field selection by name"""
        field = 'M100'
        expected_nrow = 2
        fieldlist = ['M100__5', 'M100__6']

        self.__exec_simple_test('field', field, fieldlist, 'FIELDNAME', expected_nrow)

    @prepare
    def test_field_value_pattern(self):
        """test_field_value_pattern: Test field selection by pattern match"""
        field = 'M*'
        expected_nrow = 3
        fieldlist = ['M100__5', 'M100__6', 'M30__7']

        self.__exec_simple_test('field', field, fieldlist, 'FIELDNAME', expected_nrow)

    @prepare
    def test_field_value_list(self):
        """test_field_value_list: Test field selection by name list"""
        field = 'M30,3C273'
        expected_nrow = 2
        fieldlist = ['M30__7', '3C273__8']

        self.__exec_simple_test('field', field, fieldlist, 'FIELDNAME', expected_nrow)

    @prepare
    def test_field_mix_exprlist(self):
        """test_field_mix_list: Test field selection by name and id"""
        field = 'M1*,7~8'
        expected_nrow = 4
        fieldlist = ['M100__5', 'M100__6', 'M30__7', '3C273__8']

        self.__exec_simple_test('field', field, fieldlist, 'FIELDNAME', expected_nrow)

    ### spw selection syntax test ###
    @prepare
    def test_spw_id_default(self):
        """test_spw_id_default: Test default value for spw"""
        spw = ''
        expected_nrow = 4
        spwlist = [21, 23, 25]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)
        
    @prepare
    def test_spw_id_exact(self):
        """test_spw_id_exact: Test spw selection by id ('N')"""
        spw = '23'
        expected_nrow = 2
        spwlist = [23]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)
        
    @prepare
    def test_spw_id_lt(self):
        """test_spw_id_lt: Test spw selection by id ('<N')"""
        spw = '<23'
        expected_nrow = 1
        spwlist = [21]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    @prepare
    def test_spw_id_gt(self):
        """test_spw_id_lt: Test spw selection by id ('>N')"""
        spw = '>23'
        expected_nrow = 1
        spwlist = [25]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    @prepare
    def test_spw_id_range(self):
        """test_spw_id_range: Test spw selection by id ('N~M')"""
        spw = '22~24'
        expected_nrow = 2
        spwlist = [23]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    @prepare
    def test_spw_id_list(self):
        """test_spw_id_list: Test spw selection by id ('N,M')"""
        spw = '21,25'
        expected_nrow = 2
        spwlist = [21, 25]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    @prepare
    def test_spw_id_exprlist(self):
        """test_spw_id_exprlist: Test spw selection by id ('EXP0,EXP1')"""
        spw = '24~26,<23'
        expected_nrow = 2
        spwlist = [21, 25]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    @prepare
    def test_spw_id_pattern(self):
        """test_spw_id_pattern: Test spw selection by wildcard"""
        spw = '*'
        expected_nrow = 4
        spwlist = [21, 23, 25]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    @prepare
    def test_spw_value_frequency(self):
        """test_spw_value_frequency: Test spw selection by frequency range ('FREQ0~FREQ1')"""
        spw = '299~300GHz'
        expected_nrow = 3
        spwlist = [21, 23]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    @prepare
    def test_spw_value_velocity(self):
        """test_spw_value_velocity: Test spw selection by velocity range ('VEL0~VEL1')"""
        spw = '-100~100km/s'
        expected_nrow = 2
        spwlist = [23]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    @prepare
    def test_spw_mix_exprlist(self):
        """test_spw_mix_exprlist: Test spw selection by id and frequency/velocity range"""
        spw = '<23,-100~100km/s'
        expected_nrow = 3
        spwlist = [21, 23]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    ### spw (channel) selection syntax test ###
    def test_spw_id_default_channel(self):
        """test_spw_id_default_channel: Test spw selection with channel range (':CH0~CH1')"""
        spw = ':0~19'
        expected_nrow = 4
        spwlist = [21, 23, 25]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    def test_spw_id_default_frequency(self):
        """test_spw_id_default_frequency: Test spw selection with channel range (':FREQ0~FREQ1')"""
        spw = ':299.45~299.509GHz'
        expected_nrow = 1
        spwlist = [21]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    def test_spw_id_default_velocity(self):
        """test_spw_id_default_velocity: Test spw selection with channel range (':VEL0~VEL1')"""
        spw = ':-28.98~28.98km/s'
        expected_nrow = 2
        spwlist = [23]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    def test_spw_id_default_list(self):
        """test_spw_id_default_list: Test spw selection with multiple channel range (':CH0~CH1;CH2~CH3')"""
        spw = ':0~19;81~100'
        expected_nrow = 4
        spwlist = [21, 23, 25]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    def test_spw_id_exact_channel(self):
        """test_spw_id_exact_channel: Test spw selection with channel range ('N:CH0~CH1')"""
        spw = '21:0~59'
        expected_nrow = 1
        spwlist = [21]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    def test_spw_id_exact_frequency(self):
        """test_spw_id_exact_frequency: Test spw selection with channel range ('N:FREQ0~FREQ1')"""
        spw = '21:299.45~299.509GHz'
        expected_nrow = 1
        spwlist = [21]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    def test_spw_id_exact_velocity(self):
        """test_spw_id_exact_velocity: Test spw selection with channel range ('N:VEL0~VEL1')"""
        spw = '23:-28.98~28.98km/s'
        expected_nrow = 2
        spwlist = [23]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    def test_spw_id_exact_list(self):
        """test_spw_id_exact_list: Test spw selection with channel range ('N:CH0~CH1;CH2~CH3')"""
        spw = '21:0~59;61~100'
        expected_nrow = 1
        spwlist = [21]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    def test_spw_id_pattern_channel(self):
        """test_spw_id_pattern_channel: Test spw selection with channel range ('*:CH0~CH1')"""
        spw = '*:0~19'
        expected_nrow = 4
        spwlist = [21, 23, 25]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    def test_spw_id_pattern_frequency(self):
        """test_spw_id_pattern_frequency: Test spw selection with channel range ('*:FREQ0~FREQ1')"""
        spw = '*:299.45~299.509GHz'
        expected_nrow = 1
        spwlist = [21]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    def test_spw_id_pattern_velocity(self):
        """test_spw_id_pattern_velocity: Test spw selection with channel range ('*:VEL0~VEL1')"""
        spw = '*:-28.98~28.98km/s'
        expected_nrow = 2
        spwlist = [23]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    def test_spw_id_pattern_list(self):
        """test_spw_id_pattern_list: Test spw selection with channel range ('*:CH0~CH1;CH2~CH3')"""
        spw = '*:0~19;81~100'
        expected_nrow = 4
        spwlist = [21, 23, 25]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    def test_spw_value_frequency_channel(self):
        """test_spw_value_frequency_channel: Test spw selection with channel range ('FREQ0~FREQ1:CH0~CH1')"""
        spw = '299.4~299.8GHz:0~59'
        expected_nrow = 1
        spwlist = [21]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    def test_spw_value_frequency_frequency(self):
        """test_spw_value_frequency_frequency: Test spw selection with channel range ('FREQ0~FREQ1:FREQ2~FREQ3')"""
        spw = '299.8~300.0GHz:299.971~300.029GHz'
        expected_nrow = 2
        spwlist = [23]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    def test_spw_value_frequency_velocity(self):
        """test_spw_value_frequency_velocity: Test spw selection with channel range ('FREQ0~FREQ1:VEL0~VEL1')"""
        spw = '299.8~300.0GHz:-28.98~28.98km/s'
        expected_nrow = 2
        spwlist = [23]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    def test_spw_value_frequency_list(self):
        """test_spw_value_frequency_list: Test spw selection with channel range ('FREQ0~FREQ1:CH0~CH1;CH2~CH3')"""
        spw = '299.4~299.8GHz:0~19;81~100'
        expected_nrow = 1
        spwlist = [21]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    def test_spw_value_velocity_channel(self):
        """test_spw_value_velocity_channel: Test spw selection with channel range ('VEL0~VEL1:CH0~CH1')"""
        spw = '-50~50km/s:21~79'
        expected_nrow = 2
        spwlist = [23]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    def test_spw_value_velocity_frequency(self):
        """test_spw_value_velocity_frequency: Test spw selection with channel range ('VEL0~VEL1:FREQ0~FREQ1')"""
        spw = '-50~50km/s:299.971~300.029GHz'
        expected_nrow = 2
        spwlist = [23]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    def test_spw_value_velocity_velocity(self):
        """test_spw_value_velocity_velocity: Test spw selection with channel range ('VEL0~VEL1:VEL2~VEL3')"""
        spw = '-50~50km/s:-28.98~28.98km/s'
        expected_nrow = 2
        spwlist = [23]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    def test_spw_value_velocity_list(self):
        """test_spw_value_velocity_list: Test spw selection with channel range ('VEL0~VEL1:CH0~CH1;CH2~CH3')"""
        spw = '-50~50km/s:0~19;21~79;81~100'
        expected_nrow = 2
        spwlist = [23]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)

    def test_spw_id_list_channel(self):
        """test_spw_id_list_channel: Test spw selection with channnel range ('ID0:CH0~CH1,ID1:CH2~CH3')"""
        spw = '21:0~59;61~100,25:0~39;41~100'
        expected_nrow = 2
        spwlist = [21,25]

        self.__exec_simple_test('spw', spw, spwlist, 'IFNO', expected_nrow)
        
    ### timerange selection syntax test ###
    @prepare
    def test_timerange_value_default(self):
        """test_timerange_value_default: Test default value for timerange"""
        timerange = ''
        expected_nrow = 4
        timelist = [55876.10559574073, 55876.10629018517, 55876.106984629616]

        self.__exec_simple_test('timerange', timerange, timelist, 'TIME', expected_nrow)

    @prepare
    def test_timerange_value_exact(self):
        """test_timerange_value_exact: Test timerange selection by syntax 'T0'"""
        timerange = '2011/11/11/02:32:03.5'
        expected_nrow = 1
        timelist = [55876.10559574073]

        self.__exec_simple_test('timerange', timerange, timelist, 'TIME', expected_nrow)

    @prepare
    def test_timerange_value_range(self):
        """test_timerange_value_range: Test timerange selection by syntax 'T0~T1'"""
        timerange = '2011/11/11/02:32:00~02:34:00'
        expected_nrow = 3
        timelist = [55876.10559574073, 55876.10629018517]

        self.__exec_simple_test('timerange', timerange, timelist, 'TIME', expected_nrow)

    @prepare
    def test_timerange_value_lt(self):
        """test_timerange_value_lt: Test timerange selection by syntax '<T0'"""
        timerange = '<2011/11/11/02:33:00'
        expected_nrow = 1
        timelist = [55876.10559574073]

        self.__exec_simple_test('timerange', timerange, timelist, 'TIME', expected_nrow)

    @prepare
    def test_timerange_value_gt(self):
        """test_timerange_value_gt: Test timerange selection by syntax '>T0'"""
        timerange = '>2011/11/11/02:33:00'
        expected_nrow = 3
        timelist = [55876.10629018517, 55876.106984629616]

        self.__exec_simple_test('timerange', timerange, timelist, 'TIME', expected_nrow)

    @prepare
    def test_timerange_value_interval(self):
        """test_timerange_value_interval: Test timerange selection by syntax 'T0+dT'"""
        timerange = '2011/11/11/02:33:00+10:00'
        expected_nrow = 3
        timelist = [55876.10629018517, 55876.106984629616]

        self.__exec_simple_test('timerange', timerange, timelist, 'TIME', expected_nrow)

    ### scan selection syntax test ###
    @prepare
    def test_scan_id_default(self):
        """test_scan_id_default: Test default value for scan"""
        scan = ''
        expected_nrow = 4
        scanlist = [15, 16, 17]

        self.__exec_simple_test('scan', scan, scanlist, 'SCANNO', expected_nrow)

    @prepare
    def test_scan_id_exact(self):
        """test_scan_id_exact: Test scan selection by id ('N')"""
        scan = '15'
        expected_nrow = 1
        scanlist = [15]

        self.__exec_simple_test('scan', scan, scanlist, 'SCANNO', expected_nrow)

    @prepare
    def test_scan_id_lt(self):
        """test_scan_id_lt: Test scan selection by id ('<N')"""
        scan = '<17'
        expected_nrow = 3
        scanlist = [15,16]

        self.__exec_simple_test('scan', scan, scanlist, 'SCANNO', expected_nrow)

    @prepare
    def test_scan_id_gt(self):
        """test_scan_id_gt: Test scan selection by id ('>N')"""
        scan = '>16'
        expected_nrow = 1
        scanlist = [17]

        self.__exec_simple_test('scan', scan, scanlist, 'SCANNO', expected_nrow)

    @prepare
    def test_scan_id_range(self):
        """test_scan_id_range: Test scan selection by id ('N~M')"""
        scan = '16~17'
        expected_nrow = 3
        scanlist = [16,17]

        self.__exec_simple_test('scan', scan, scanlist, 'SCANNO', expected_nrow)

    @prepare
    def test_scan_id_list(self):
        """test_scan_id_list: Test scan selection by id ('N,M')"""
        scan = '15,17'
        expected_nrow = 2
        scanlist = [15,17]

        self.__exec_simple_test('scan', scan, scanlist, 'SCANNO', expected_nrow)

    @prepare
    def test_scan_id_exprlist(self):
        """test_scan_id_exprlist: Test scan selection by id ('EXP0,EXP1')"""
        scan = '<16,>16'
        expected_nrow = 2
        scanlist = [15,17]

        self.__exec_simple_test('scan', scan, scanlist, 'SCANNO', expected_nrow)

    ### pol selection syntax test ###
    @prepare
    def test_pol_id_default(self):
        """test_pol_id_default: Test default value for pol"""
        pol = ''
        expected_nrow = 4
        pollist = [0,1]

        self.__exec_simple_test('pol', pol, pollist, 'POLNO', expected_nrow)
        
    @prepare
    def test_pol_id_exact(self):
        """test_pol_id_exact: Test pol selection by id ('N')"""
        pol = '1'
        expected_nrow = 2
        pollist = [1]

        self.__exec_simple_test('pol', pol, pollist, 'POLNO', expected_nrow)
        
    @prepare
    def test_pol_id_lt(self):
        """test_pol_id_lt: Test pol selection by id ('<N')"""
        pol = '<1'
        expected_nrow = 2
        pollist = [0]

        self.__exec_simple_test('pol', pol, pollist, 'POLNO', expected_nrow)

    @prepare
    def test_pol_id_gt(self):
        """test_pol_id_gt: Test pol selection by id ('>N')"""
        pol = '>0'
        expected_nrow = 2
        pollist = [1]

        self.__exec_simple_test('pol', pol, pollist, 'POLNO', expected_nrow)

    @prepare
    def test_pol_id_range(self):
        """test_pol_id_range: Test pol selection by id ('N~M')"""
        pol = '0~1'
        expected_nrow = 4
        pollist = [0,1]

        self.__exec_simple_test('pol', pol, pollist, 'POLNO', expected_nrow)

    @prepare
    def test_pol_id_list(self):
        """test_pol_id_list: Test pol selection by id ('N,M')"""
        pol = '0,1'
        expected_nrow = 4
        pollist = [0,1]

        self.__exec_simple_test('pol', pol, pollist, 'POLNO', expected_nrow)

    @prepare
    def test_pol_id_exprlist(self):
        """test_pol_id_exprlist: Test pol selection by id ('EXP0,EXP1')"""
        pol = '>0,<1'
        expected_nrow = 4
        pollist = [0,1]

        self.__exec_simple_test('pol', pol, pollist, 'POLNO', expected_nrow)

def suite():
    return [tsdbaseline_basicTest, tsdbaseline_maskTest, tsdbaseline_funcTest,
            tsdbaseline_multi_IF_test, tsdbaseline_storageTest,
            tsdbaseline_selection_syntax]

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

