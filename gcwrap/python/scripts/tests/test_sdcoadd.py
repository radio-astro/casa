import os
import sys
import shutil
import numpy
from __main__ import default
from tasks import *
from taskinit import *
import unittest
#

from sdcoadd import sdcoadd
from sdutil import tbmanager
import asap as sd
from asap.scantable import is_scantable, is_ms


class sdcoadd_unittest_base:
    """
    Base class for sdcoadd unit test
    """
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + \
              '/data/regression/unittest/sdcoadd/'
    taskname = "sdcoadd"

    mergeids = {"FOCUS_ID": "FOCUS", "FREQ_ID": "FREQUENCIES", \
                "MOLECULE_ID": "MOLECULES"}
    addedtabs = []#"HISTORY"] #, "FIT", "TCAL", "WEATHER"]


    def _get_scantable_params( self, scanname ):
        """
        Returns a dictionary which contains row numbers and a set of
        data in scantable for later verifications
        """
        if not os.path.exists(scanname):
            raise Exception, "A scantable '%s' does not exists." % scanname
        if not is_scantable(scanname):
            raise Exception, "Input file '%s' is not a scantable." % scanname
    
        res = {}
        for tab in self.addedtabs + self.mergeids.values():
            tb.open(scanname+"/"+tab)
            res["n"+tab] = tb.nrows()
            tb.close()

        # open main table
        tb.open(scanname)
        res["nMAIN"] = tb.nrows()
        for col in ["SCANNO"] + self.mergeids.keys():
            res[col+"S"] = list(set(tb.getcol(col)))
        tb.close()
        return res

    def _test_merged_scantable( self, res, ref ):
        reltol = 1.0e-5
        self._compareDictVal(res,ref,reltol=reltol)

    def _compareDictVal( self, testdict, refdict, reltol=1.0e-5, complist=None ):
        self.assertTrue(isinstance(testdict,dict) and \
                        isinstance(refdict, dict),\
                        "Need to specify two dictionaries to compare")
        if complist:
            keylist = complist
        else:
            keylist = refdict.keys()
        
        for key in keylist:
            self.assertTrue(testdict.has_key(key),\
                            msg="%s is not defined in the current results."\
                            % key)
            self.assertTrue(refdict.has_key(key),\
                            msg="%s is not defined in the reference data."\
                            % key)
            testval = self._to_list(testdict[key])
            refval = self._to_list(refdict[key])
            casalog.post('%s: testval=%s, refval=%s'%(key,testval,refval))
            self.assertTrue(len(testval)==len(refval),"Number of elemnets differs.")
            for i in range(len(testval)):
                if isinstance(refval[i],str):
                    self.assertTrue(testval[i]==refval[i],\
                                    msg="%s[%d] differs: %s (expected: %s) " % \
                                    (key, i, str(testval[i]), str(refval[i])))
                else:
                    self.assertTrue(self._isInAllowedRange(testval[i],refval[i],reltol),\
                                    msg="%s[%d] differs: %s (expected: %s) " % \
                                    (key, i, str(testval[i]), str(refval[i])))
            del testval, refval
            

    def _isInAllowedRange( self, testval, refval, reltol=1.0e-5 ):
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


class sdcoadd_basicTest( sdcoadd_unittest_base, unittest.TestCase ):
    """
    Basic unit tests for task sdcoadd. No averaging and interactive testing.

    The list of tests:
    test00   --- default parameters (raises an error)
    test01   --- valid infiles (scantable) WITHOUT outfile
    test02   --- valid infiles (scantable) with outfile
    test03   --- outform='MS'
    test04   --- overwrite=False
    test05   --- overwrite=True
    test06   --- merge 3 scantables
    test07   --- specify only a scantables (raises an error)
    """
    # Input and output names
    inlist = ['orions_calSave_21.asap','orions_calSave_25.asap','orions_calSave_23.asap']
    outname = "sdcoadd_out.asap"
    # Reference data of merged scantable
    # merge result of scan 21 and 25 (no overlap in IF and MOL_ID data
    # but the same IDs are assigned ... requires proper addition of IDs by
    # checking of subtable rows)
    ref2125 = {"nMAIN": 16, "nFOCUS": 1, "nFREQUENCIES": 8, "nMOLECULES": 2,\
               "SCANNOS": range(2), "FOCUS_IDS": range(1),\
               "FREQ_IDS": range(8),"MOLECULE_IDS": range(2)}
    # merge result of scan 21, 25, and 23
    # - scan 21 & 25: no overlap in IF and MOL_ID but the same IDs
    #   are assigned ... requires proper addition of IDs by checking of
    #   subtable rows)
    # - scan 21 & 23: perfect overlap in IF and MOL_ID ... requires no
    #   adding of existing IDs by checking subtable rows.
    ref212523 = {"nMAIN": 24, "nFOCUS": 1, "nFREQUENCIES": 8, "nMOLECULES": 2,\
                 "SCANNOS": range(3), "FOCUS_IDS": range(1),\
                 "FREQ_IDS": range(8),"MOLECULE_IDS": range(2)}
    
    def setUp( self ):
        # copy input scantables
        for infile in self.inlist:
            if os.path.exists(infile):
                shutil.rmtree(infile)
            shutil.copytree(self.datapath+infile, infile)

        default(sdcoadd)

    def tearDown( self ):
        for thefile in self.inlist + [self.outname]:
            if (os.path.exists(thefile)):
                shutil.rmtree(thefile)

    def test00( self ):
        """Test 0: Default parameters (raises an error)"""
        try:
            result = sdcoadd()
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Need at least two data file names')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test01( self ):
        """Test 1: valid infiles (scantables) WITHOUT outfile"""
        infiles = self.inlist[0:2]
        result = sdcoadd(infiles=infiles)

        self.assertEqual(result,None)
        # test merged scantable
        outname = self.inlist[0]+"_coadd"
        print outname
        self.assertTrue(os.path.exists(outname),msg="No output written")
        merged = self._get_scantable_params(outname)
        self._test_merged_scantable(merged, self.ref2125)

    def test02( self ):
        """Test 2: valid infiles (scantable) with outfile"""
        infiles = self.inlist[0:2]
        outfile = self.outname
        result = sdcoadd(infiles=infiles,outfile=outfile)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        # test merged scantable
        merged = self._get_scantable_params(outfile)
        self._test_merged_scantable(merged, self.ref2125)

    def test03( self ):
        """Test 3: outform='MS'"""
        infiles = self.inlist[0:2]
        outfile = self.outname.rstrip('.asap')+'.ms'
        outform = 'MS2'
        result = sdcoadd(infiles=infiles,outfile=outfile,outform=outform)
        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")

        self.assertTrue(is_ms(outfile),msg="Output file is not an MS")

    def test04( self ):
        """Test 4: overwrite=False (raises an error)"""
        infiles = self.inlist[1:3]
        outfile = self.outname
        result = sdcoadd(infiles=infiles,outfile=outfile)
        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")

        infiles = self.inlist[0:2]
        outfile = self.outname
        try:
            result2 = sdcoadd(infiles=infiles,outfile=outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Output file \'%s\' exists.'%(outfile))
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test05( self ):
        """Test 5: overwrite=True"""
        infiles = self.inlist[1:3]
        outfile = self.outname
        result = sdcoadd(infiles=infiles,outfile=outfile)
        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")

        infiles = self.inlist[0:2]
        overwrite = True
        result2 = sdcoadd(infiles=infiles,outfile=outfile,overwrite=overwrite)

        self.assertEqual(result2,None)
        # test merged scantable
        merged = self._get_scantable_params(outfile)
        self._test_merged_scantable(merged, self.ref2125)

    def test06( self ):
        """Test 6: Merge 3 scantables"""
        infiles = self.inlist
        outfile = self.outname
        result = sdcoadd(infiles=infiles,outfile=outfile)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        # test merged scantable
        merged = self._get_scantable_params(outfile)
        self._test_merged_scantable(merged, self.ref212523)

    def test07( self ):
        """Test 7: specify only a scantables (raises an error)"""
        infiles = self.inlist[0]
        outfile = self.outname
        try:
            result = sdcoadd(infiles=infiles,outfile=outfile)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Need at least two data file names')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))


class sdcoadd_mergeTest( sdcoadd_unittest_base, unittest.TestCase ):
    """
    Test capabilities of sd.merge(). No averaging and interactive testing.

    The list of tests:
    testmerge01   --- test merge of HISTORY subtables
    testmerge02   --- test proper merge of FREQUENCIES subtables
    testmerge03   --- test proper merge of MOLECULE subtables
    """
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdcoadd/'
    # Input and output names
    # orions_calSave_21if03.asap: nROW=4, IF=[0,3], scan=[21], MOL=[0], FOC=[0]
    # orions_calSave_23.asap: nROW=8, IF=[0-3], scan=[23], MOL=[0], FOC=[0]
    # orions_calSave_2327.asap: nROW=16, IF=[0-7], scan=[23,27], MOL=[0], FOC=[0]
    inlist = []
    outname = "sdcoadd_out.asap"
    
    def setUp( self ):
        default(sdcoadd)

    def tearDown( self ):
        for thefile in self.inlist + [self.outname]:
            if (os.path.exists(thefile)):
                shutil.rmtree(thefile)

    def _copy_inputs( self ):
        # copy input scantables
        for infile in self.inlist:
            if os.path.exists(infile):
                shutil.rmtree(infile)
            shutil.copytree(self.datapath+infile, infile)

    def testmerge01( self ):
        """Merge Test 1: merge of HISTORY subtables"""
        self.inlist = ['orions_calSave_21if03.asap','orions_calSave_23.asap']
        self._copy_inputs()

        infiles = self.inlist
        innhist = 0
        htab="/HISTORY"
        for tab in infiles:
            tb.open(tab+htab)
            innhist += tb.nrows()
            tb.close()
        outfile = self.outname
        result = sdcoadd(infiles=infiles,outfile=outfile)
        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")

        tb.open(outfile+htab)
        outnhist = tb.nrows()
        tb.close()
        self.assertTrue(outnhist >= innhist,\
                        msg="nHIST = %d (expected: >= %d)" % (outnhist, innhist))
    def testmerge02( self ):
        """Merge Test 2: proper merge of FREQUENCIES subtables"""
        self.inlist = ['orions_calSave_21if03.asap','orions_calSave_23.asap']
        self._copy_inputs()
        
        infiles = self.inlist
        outfile = self.outname
        result = sdcoadd(infiles=infiles,outfile=outfile)
        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")

        merged = self._get_scantable_params(outfile)
        # check FREQ_ID
        self.assertEqual(merged["nFREQUENCIES"],4,msg="nFREQUENCIES is not correct")
        self.assertTrue(merged["FREQ_IDS"]==range(4),\
                        msg="FREQ_IDS = %s (expected: %s)" % \
                        (str(merged["FREQ_IDS"]), str(range(4))))
        # check main row number
        self.assertEqual(merged["nMAIN"],12,msg="nMAIN is not correct")


    def testmerge03( self ):
        """Merge Test 3: proper merge of MOLECULES subtables"""
        self.inlist = ['orions_calSave_21if03.asap','orions_calSave_2327.asap']
        self._copy_inputs()

        infiles = self.inlist
        outfile = self.outname
        result = sdcoadd(infiles=infiles,outfile=outfile)
        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")

        merged = self._get_scantable_params(outfile)
        # check MOLECULE subtables
        self.assertEqual(merged["nMOLECULES"],2,msg="nMOLECULES is wrong")
        self.assertTrue(merged["MOLECULE_IDS"]==range(2),\
                        msg="MOLECULE_IDS = %s (expected: %s)" % \
                        (str(merged["MOLECULE_IDS"]), str(range(2))))
        # check main row number
        self.assertEqual(merged["nMAIN"],20,msg="nMAIN is wrong")
        # check FREQ_ID
        refFID = range(8)
        self.assertEqual(merged["nFREQUENCIES"],len(refFID),\
                         msg="nFREQUENCIES is wrong")
        self.assertTrue(merged["FREQ_IDS"]==refFID,\
                        msg="FREQ_IDS = %s (expected: %s)" % \
                        (str(merged["FREQ_IDS"]), str(refFID)))


class sdcoadd_freqtolTest( sdcoadd_unittest_base, unittest.TestCase ):
    """
    Test capabilities of sd.merge() with freq_tol='1kHz'.

    The list of tests:
    test_freqtol00: check if same freq[0], increment, and nchan is merged
    test_freqtol01: check if different freq[0] (within 1kHz) is merged
    test_freqtol02: check if different freq[0] (equal to 1kHz) is merged
    test_freqtol03: check if different freq[0] (out of 1kHz) is NOT merged
    test_freqtol04: check if different increment (within 1kHz) is merged
    test_freqtol05: check if different increment (equal to 1kHz) is merged
    test_freqtol06: check if different increment (out of 1kHz) is NOT merged
    test_freqtol07: check if different refval/refpix but same freq[0] is merged
    test_freqtol08: check if different nchan merges FREQUENCIES and FREQ_ID
                    but does NOT IFNO
    test_freqtol10: check if different BASEFRAME causes exception
    test_freqtol11: check if different FRAME doesn't affect the process
    test_freqtol12: check if any small shift within double accuracy is not
                    allowed by default
    test_freqtol13: check if exactly same FREQUENCIES entries are merged
                    by default
    test_freqtol14: check if numeric tolerance value works as expected
    test_freqtol15: check if single MOLECULE_ID is assigned to each IFNO when
                    there are two MOLECULE_ID's that refers different rest
                    frequencies
    test_freqtol16: check if single MOLECULE_ID is assigned to each IFNO when
                    there are two MOLECULE_ID's that refers same rest frequency
    test_freqtol17: fail if invalid value is given
    """
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdcoadd/'
    # Input and output names
    # orions_calSave_21if03.asap: nROW=4, IF=[0,3], scan=[21], MOL=[0], FOC=[0]
    # orions_calSave_23.asap: nROW=8, IF=[0-3], scan=[23], MOL=[0], FOC=[0]
    # orions_calSave_2327.asap: nROW=16, IF=[0-7], scan=[23,27], MOL=[0], FOC=[0]
    inlist = ['orions_calSave_23.asap','orions_calSave_21if03.asap']
    outfile = "sdcoadd_out.asap"
    freqtol = '1kHz'
    within_tol = 0.99e3
    equal_tol = 1.0e3
    outof_tol = 1.01e3
    
    def setUp( self ):
        default(sdcoadd)
        self._copy_inputs()

    def tearDown( self ):
        for thefile in self.inlist + [self.outfile]:
            if (os.path.exists(thefile)):
                shutil.rmtree(thefile)

    def _copy_inputs( self ):
        # copy input scantables
        for infile in self.inlist:
            if os.path.exists(infile):
                shutil.rmtree(infile)
            shutil.copytree(self.datapath+infile, infile)

    def _verify(self, outfile, expected_main_nrow, expected_hist_nrow, expected_freq_nrow, expected_freqids, expected_ifnos, expected_molids=None):
        # outfile must exist
        self.assertTrue(os.path.exists(self.outfile),msg="No output written")
        
        # test nrow of outfile MAIN
        tb.open(outfile)
        main_nrow = tb.nrows()
        freqids = tb.getcol('FREQ_ID')
        ifnos = tb.getcol('IFNO')
        molids = tb.getcol('MOLECULE_ID')
        tb.close()
        self.assertEqual(expected_main_nrow, main_nrow,
                         msg='number of MAIN rows differ')

        # test nrow of outfile HISTORY
        hist_nrow = self._nrow(outfile, 'HISTORY')
        self.assertTrue(hist_nrow >= expected_hist_nrow,
                        msg="nHIST = %d (expected: >= %d)" % (hist_nrow, expected_hist_nrow))        
        
        # test nrow of outfile FREQUENCIES
        freq_nrow = self._nrow(outfile, 'FREQUENCIES')
        self.assertEqual(expected_freq_nrow, freq_nrow,
                         msg='number of FREQUENCIES rows differ')
        
        # test FREQ_ID entries
        self.assertEqual(main_nrow, len(expected_freqids),
                         msg='invalid expected value for FREQ_ID')
        self.assertTrue(all(freqids == numpy.array(expected_freqids)),
                            msg='FREQ_ID entries differ')
        
        # test IFNO entries
        self.assertEqual(main_nrow, len(expected_ifnos),
                         msg='invalid expected value for IFNO')
        self.assertTrue(all(ifnos == numpy.array(expected_ifnos)),
                            msg='IFNO entries differ')

        # test MOLECULE_ID entries if necessary
        if expected_molids is not None:
            self.assertEqual(main_nrow, len(expected_molids),
                             msg='invalid expected value for MOLECULE_ID')
            self.assertTrue(all(molids == numpy.array(expected_molids)),
                            msg='MOLECULE_ID entries differ')
        

    def _nrow(self, tables, subtable=None):
        nrow = 0
        if subtable is None:
            if isinstance(tables, str):
                tb.open(tables)
                nrow = tb.nrows()
                tb.close()
            else:
                for t in tables:
                    nrow += self._nrow(t)
        else:
            if isinstance(tables, str):
                nrow = self._nrow(os.path.join(tables, subtable))
            else:
                for t in tables:
                    nrow += self._nrow(t, subtable)
        return nrow
        
    def test_freqtol00( self ):
        """test_freqtol00: check if same freq[0], increment, and nchan is merged"""
        infiles = self.inlist
        
        # edit self.inlist[1] so that it suites with testing purpose
        # 1. renumber IFNO in second tables according to FREQ_ID
        tb.open(infiles[1], nomodify=False)
        freqids = tb.getcol('FREQ_ID')
        tb.putcol('IFNO', freqids)
        tb.close()

        # expected nrows for outfile HISTORY (must be evaluated before
        # running sdcoadd)
        expected_hist_nrow = self._nrow(infiles, 'HISTORY')

        # run sdcoadd
        result = sdcoadd(infiles=infiles,outfile=self.outfile,freqtol=self.freqtol)
        self.assertEqual(result,None)

        # verification
        expected_main_nrow = self._nrow(infiles)
        expected_freq_nrow = 4
        expected_freqids = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 3, 3]
        expected_ifnos = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 3, 3]
        self._verify(self.outfile, expected_main_nrow, expected_hist_nrow, expected_freq_nrow, expected_freqids, expected_ifnos)

    def test_freqtol01( self ):
        """test_freqtol01: different freq[0] (within 1kHz) is merged"""
        infiles = self.inlist
        
        # edit self.inlist[1] so that it suites with testing purpose
        # 1. renumber IFNO in second tables according tao FREQ_ID
        # 2. shift REFVAL by an amount within tolerance
        tb.open(infiles[1], nomodify=False)
        freqids = tb.getcol('FREQ_ID')
        tb.putcol('IFNO', freqids)
        tb.close()
        tb.open(os.path.join(infiles[1], 'FREQUENCIES'), nomodify=False)
        refval = tb.getcell('REFVAL', 1)
        refval += self.within_tol
        tb.putcell('REFVAL', 1, refval)
        tb.close()

        # expected nrows for outfile HISTORY 
        expected_hist_nrow = self._nrow(infiles, 'HISTORY')

        # run sdcoadd
        result = sdcoadd(infiles=infiles,outfile=self.outfile,freqtol=self.freqtol)
        self.assertEqual(result,None)

        # verification
        expected_main_nrow = self._nrow(infiles)
        expected_freq_nrow = 4
        expected_freqids = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 3, 3]
        expected_ifnos = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 3, 3]
        self._verify(self.outfile, expected_main_nrow, expected_hist_nrow, expected_freq_nrow, expected_freqids, expected_ifnos)

    def test_freqtol02(self):
        """test_freqtol02: check if different freq[0] (equal to 1kHz) is merged"""
        infiles = self.inlist
        
        # edit self.inlist[1] so that it suites with testing purpose
        # 1. renumber IFNO in second tables according tao FREQ_ID
        # 2. shift REFVAL by an amount equal to tolerance
        tb.open(infiles[1], nomodify=False)
        freqids = tb.getcol('FREQ_ID')
        tb.putcol('IFNO', freqids)
        tb.close()
        tb.open(os.path.join(infiles[1], 'FREQUENCIES'), nomodify=False)
        refval = tb.getcell('REFVAL', 1)
        refval += self.equal_tol
        tb.putcell('REFVAL', 1, refval)
        tb.close()

        # expected nrows for outfile HISTORY 
        expected_hist_nrow = self._nrow(infiles, 'HISTORY')

        # run sdcoadd
        result = sdcoadd(infiles=infiles,outfile=self.outfile,freqtol=self.freqtol)
        self.assertEqual(result,None)

        # verification
        expected_main_nrow = self._nrow(infiles)
        expected_freq_nrow = 4
        expected_freqids = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 3, 3]
        expected_ifnos = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 3, 3]
        self._verify(self.outfile, expected_main_nrow, expected_hist_nrow, expected_freq_nrow, expected_freqids, expected_ifnos)

    def test_freqtol03(self):
        """test_freqtol03: check if different freq[0] (out of 1kHz) is NOT merged"""
        infiles = self.inlist
        
        # edit self.inlist[1] so that it suites with testing purpose
        # 1. renumber IFNO in second tables according tao FREQ_ID
        # 2. shift REFVAL by an amount out of tolerance
        tb.open(infiles[1], nomodify=False)
        freqids = tb.getcol('FREQ_ID')
        tb.putcol('IFNO', freqids)
        tb.close()
        tb.open(os.path.join(infiles[1], 'FREQUENCIES'), nomodify=False)
        refval = tb.getcell('REFVAL', 1)
        refval += self.outof_tol
        tb.putcell('REFVAL', 1, refval)
        tb.close()

        # expected nrows for outfile HISTORY 
        expected_hist_nrow = self._nrow(infiles, 'HISTORY')

        # run sdcoadd
        result = sdcoadd(infiles=infiles,outfile=self.outfile,freqtol=self.freqtol)
        self.assertEqual(result,None)

        # verification
        expected_main_nrow = self._nrow(infiles)
        expected_freq_nrow = 5
        expected_freqids = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 4, 4]
        expected_ifnos = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 4, 4]
        self._verify(self.outfile, expected_main_nrow, expected_hist_nrow, expected_freq_nrow, expected_freqids, expected_ifnos)

    def test_freqtol04(self):
        """test_freqtol04: check if different increment (within 1kHz) is merged"""
        infiles = self.inlist
        
        # edit self.inlist[1] so that it suites with testing purpose
        # 1. renumber IFNO in second tables according tao FREQ_ID
        # 2. shift INCREMENT by an amount within tolerance
        # 3. change REFPIX to 0.0 for FREQUENCIES rows that are supposed
        #    to be merged
        tb.open(infiles[1], nomodify=False)
        freqids = tb.getcol('FREQ_ID')
        tb.putcol('IFNO', freqids)
        tb.close()
        tb.open(os.path.join(infiles[1], 'FREQUENCIES'), nomodify=False)
        increment = tb.getcell('INCREMENT', 1)
        increment += self.within_tol
        tb.putcell('INCREMENT', 1, increment)
        tb.putcell('REFPIX', 1, 0.0)
        tb.close()
        tb.open(os.path.join(infiles[0], 'FREQUENCIES'), nomodify=False)
        tb.putcell('REFPIX', 3, 0.0)
        tb.close()

        # expected nrows for outfile HISTORY 
        expected_hist_nrow = self._nrow(infiles, 'HISTORY')

        # run sdcoadd
        result = sdcoadd(infiles=infiles,outfile=self.outfile,freqtol=self.freqtol)
        self.assertEqual(result,None)

        # verification
        expected_main_nrow = self._nrow(infiles)
        expected_freq_nrow = 4
        expected_freqids = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 3, 3]
        expected_ifnos = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 3, 3]
        self._verify(self.outfile, expected_main_nrow, expected_hist_nrow, expected_freq_nrow, expected_freqids, expected_ifnos)

    def test_freqtol05(self):
        """test_freqtol05: check if different increment (equal to 1kHz) is merged"""
        infiles = self.inlist
        
        # edit self.inlist[1] so that it suites with testing purpose
        # 1. renumber IFNO in second tables according tao FREQ_ID
        # 2. shift INCREMENT by an amount equal to tolerance
        # 3. change REFPIX to 0.0 for FREQUENCIES rows that are supposed
        #    to be merged
        tb.open(infiles[1], nomodify=False)
        freqids = tb.getcol('FREQ_ID')
        tb.putcol('IFNO', freqids)
        tb.close()
        tb.open(os.path.join(infiles[1], 'FREQUENCIES'), nomodify=False)
        increment = tb.getcell('INCREMENT', 1)
        increment += self.equal_tol
        tb.putcell('INCREMENT', 1, increment)
        tb.putcell('REFPIX', 1, 0.0)
        tb.close()
        tb.open(os.path.join(infiles[0], 'FREQUENCIES'), nomodify=False)
        tb.putcell('REFPIX', 3, 0.0)
        tb.close()

        # expected nrows for outfile HISTORY 
        expected_hist_nrow = self._nrow(infiles, 'HISTORY')

        # run sdcoadd
        result = sdcoadd(infiles=infiles,outfile=self.outfile,freqtol=self.freqtol)
        self.assertEqual(result,None)

        # verification
        expected_main_nrow = self._nrow(infiles)
        expected_freq_nrow = 4
        expected_freqids = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 3, 3]
        expected_ifnos = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 3, 3]
        self._verify(self.outfile, expected_main_nrow, expected_hist_nrow, expected_freq_nrow, expected_freqids, expected_ifnos)

    def test_freqtol06(self):
        """test_freqtol06: check if different increment (out of 1kHz) is NOT merged"""
        infiles = self.inlist
        
        # edit self.inlist[1] so that it suites with testing purpose
        # 1. renumber IFNO in second tables according tao FREQ_ID
        # 2. shift INCREMENT by an amount out of tolerance
        # 3. change REFPIX to 0.0 for FREQUENCIES rows (including
        #    self.inlist[0]) that are supposed to be merged
        tb.open(infiles[1], nomodify=False)
        freqids = tb.getcol('FREQ_ID')
        tb.putcol('IFNO', freqids)
        tb.close()
        tb.open(os.path.join(infiles[1], 'FREQUENCIES'), nomodify=False)
        increment = tb.getcell('INCREMENT', 1)
        increment += self.outof_tol
        tb.putcell('INCREMENT', 1, increment)
        tb.putcell('REFPIX', 1, 0.0)
        tb.close()
        tb.open(os.path.join(infiles[0], 'FREQUENCIES'), nomodify=False)
        tb.putcell('REFPIX', 3, 0.0)
        tb.close()

        # expected nrows for outfile HISTORY 
        expected_hist_nrow = self._nrow(infiles, 'HISTORY')

        # run sdcoadd
        result = sdcoadd(infiles=infiles,outfile=self.outfile,freqtol=self.freqtol)
        self.assertEqual(result,None)

        # verification
        expected_main_nrow = self._nrow(infiles)
        expected_freq_nrow = 5
        expected_freqids = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 4, 4]
        expected_ifnos = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 4, 4]
        self._verify(self.outfile, expected_main_nrow, expected_hist_nrow, expected_freq_nrow, expected_freqids, expected_ifnos)

    def test_freqtol07(self):
        """test_freqtol07: check if different refval/refpix but same freq[0] is merged"""
        infiles = self.inlist
        
        # edit self.inlist[1] so that it suites with testing purpose
        # 1. renumber IFNO in second tables according tao FREQ_ID
        # 2. shift REFPIX and REFVAL
        tb.open(infiles[1], nomodify=False)
        freqids = tb.getcol('FREQ_ID')
        tb.putcol('IFNO', freqids)
        tb.close()
        tb.open(os.path.join(infiles[1], 'FREQUENCIES'), nomodify=False)
        increment = tb.getcell('INCREMENT', 1)
        npix = 10.0
        refpix = tb.getcell('REFPIX', 1)
        refval = tb.getcell('REFVAL', 1)
        refval += npix * increment
        refpix += npix
        tb.close()

        # expected nrows for outfile HISTORY 
        expected_hist_nrow = self._nrow(infiles, 'HISTORY')

        # run sdcoadd
        result = sdcoadd(infiles=infiles,outfile=self.outfile,freqtol=self.freqtol)
        self.assertEqual(result,None)

        # verification
        expected_main_nrow = self._nrow(infiles)
        expected_freq_nrow = 4
        expected_freqids = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 3, 3]
        expected_ifnos = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 3, 3]
        self._verify(self.outfile, expected_main_nrow, expected_hist_nrow, expected_freq_nrow, expected_freqids, expected_ifnos)

    def test_freqtol08(self):
        """test_freqtol08: check if different nchan merges FREQUENCIES and FREQ_ID but does NOT IFNO"""
        infiles = self.inlist
        
        # edit self.inlist[1] so that it suites with testing purpose
        # 1. renumber IFNO in second tables according tao FREQ_ID
        # 2. change nchan of target FREQ_ID
        tb.open(infiles[1], nomodify=False)
        freqids = tb.getcol('FREQ_ID')
        tb.putcol('IFNO', freqids)
        tsel = tb.query('IFNO==1')
        sp = tsel.getcol('SPECTRA')
        fl = tsel.getcol('FLAGTRA')
        tsel.putcol('SPECTRA', sp[:10,:])
        tsel.putcol('FLAGTRA', fl[:10,:])
        tsel.close()
        tb.close()

        # expected nrows for outfile HISTORY 
        expected_hist_nrow = self._nrow(infiles, 'HISTORY')

        # run sdcoadd
        result = sdcoadd(infiles=infiles,outfile=self.outfile,freqtol=self.freqtol)
        self.assertEqual(result,None)

        # verification
        expected_main_nrow = self._nrow(infiles)
        expected_freq_nrow = 4
        expected_freqids = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 3, 3]
        expected_ifnos = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 4, 4]
        self._verify(self.outfile, expected_main_nrow, expected_hist_nrow, expected_freq_nrow, expected_freqids, expected_ifnos)
    
    def test_freqtol09( self ):
        """test_freqtol09: check if same freq[0], increment, and nchan is merged (order of infiles is reversed)"""
        # reverse an order of infiles
        infiles = self.inlist[::-1]
        
        # edit self.inlist[0] so that it suites with testing purpose
        # 1. renumber IFNO in second tables according to FREQ_ID
        tb.open(infiles[0], nomodify=False)
        freqids = tb.getcol('FREQ_ID')
        tb.putcol('IFNO', freqids)
        tb.close()

        # expected nrows for outfile HISTORY (must be evaluated before
        # running sdcoadd)
        expected_hist_nrow = self._nrow(infiles, 'HISTORY')

        # run sdcoadd
        result = sdcoadd(infiles=infiles,outfile=self.outfile,freqtol=self.freqtol)
        self.assertEqual(result,None)

        # verification
        expected_main_nrow = self._nrow(infiles)
        expected_freq_nrow = 4
        expected_freqids = [0, 0, 1, 1, 0, 0, 2, 2, 3, 3, 1, 1]
        expected_ifnos = [0, 0, 1, 1, 0, 0, 2, 2, 3, 3, 1, 1]
        self._verify(self.outfile, expected_main_nrow, expected_hist_nrow, expected_freq_nrow, expected_freqids, expected_ifnos)

    def test_freqtol10(self):
        """test_freqtol10: check if different BASEFRAME causes exception"""
        infiles = self.inlist
        
        # edit self.inlist[1] so that it suites with testing purpose
        # 1. renumber IFNO in second tables according tao FREQ_ID
        # 2. change BASEFRAME
        tb.open(infiles[1], nomodify=False)
        freqids = tb.getcol('FREQ_ID')
        tb.putcol('IFNO', freqids)
        tb.close()
        tb.open(os.path.join(infiles[1], 'FREQUENCIES'), nomodify=False)
        tb.putkeyword('BASEFRAME', 'TOPO')
        tb.close()

        # expected nrows for outfile HISTORY 
        expected_hist_nrow = self._nrow(infiles, 'HISTORY')

        # run sdcoadd
        try:
            result = sdcoadd(infiles=infiles,outfile=self.outfile,freqtol=self.freqtol)
        except RuntimeError, e:
            self.assertNotEqual(str(e).find('BASEFRAME is not identical'), -1,
                             msg='Unexpected exception is thrown: \'%s\''%(str(e)))
        except Exception, e:
            self.fail('Unexpected exception is thrown: \'%s\''%(str(e)))
            
    
    def test_freqtol11(self):
        """test_freqtol11: check if different FRAME doesn't affect the process"""
        infiles = self.inlist
        
        # edit self.inlist[1] so that it suites with testing purpose
        # 1. renumber IFNO in second tables according tao FREQ_ID
        # 2. change FRAME
        tb.open(infiles[1], nomodify=False)
        freqids = tb.getcol('FREQ_ID')
        tb.putcol('IFNO', freqids)
        tb.close()
        tb.open(os.path.join(infiles[1], 'FREQUENCIES'), nomodify=False)
        tb.putkeyword('FRAME', 'TOPO')
        tb.close()

        # expected nrows for outfile HISTORY 
        expected_hist_nrow = self._nrow(infiles, 'HISTORY')

        # run sdcoadd
        result = sdcoadd(infiles=infiles,outfile=self.outfile,freqtol=self.freqtol)
        self.assertEqual(result,None)

        # verification
        expected_main_nrow = self._nrow(infiles)
        expected_freq_nrow = 4
        expected_freqids = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 3, 3]
        expected_ifnos = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 3, 3]
        self._verify(self.outfile, expected_main_nrow, expected_hist_nrow, expected_freq_nrow, expected_freqids, expected_ifnos)

        # additional check: check FRAME of outfile FREQUENCIES
        tb.open(os.path.join(infiles[0], 'FREQUENCIES'))
        frame_ref = tb.getkeyword('FRAME')
        tb.close()
        tb.open(os.path.join(self.outfile, 'FREQUENCIES'))
        frame_out = tb.getkeyword('FRAME')
        tb.close()
        self.assertEqual(frame_ref, frame_out,
                         msg='FRAME of outfile FREQUENCIES must %s (%s is given)'%(frame_ref, frame_out))

    def test_freqtol12(self):
        """test_freqtol12: check if any small shift within double accuracy is not allowed by default"""
        infiles = self.inlist
        
        # edit self.inlist[1] so that it suites with testing purpose
        # 1. renumber IFNO in second tables according tao FREQ_ID
        # 2. shift REFVAL by an amount out of tolerance
        tb.open(infiles[1], nomodify=False)
        freqids = tb.getcol('FREQ_ID')
        tb.putcol('IFNO', freqids)
        tb.close()
        tb.open(os.path.join(infiles[1], 'FREQUENCIES'), nomodify=False)
        refval = tb.getcell('REFVAL', 1)
        refval += refval * 1.0e-13 # practical value (theoretically should be around 1.0e-15)
        tb.putcell('REFVAL', 1, refval)
        tb.close()

        # expected nrows for outfile HISTORY 
        expected_hist_nrow = self._nrow(infiles, 'HISTORY')

        # run sdcoadd
        result = sdcoadd(infiles=infiles,outfile=self.outfile)
        self.assertEqual(result,None)

        # verification
        expected_main_nrow = self._nrow(infiles)
        expected_freq_nrow = 5
        expected_freqids = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 4, 4]
        expected_ifnos = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 4, 4]
        self._verify(self.outfile, expected_main_nrow, expected_hist_nrow, expected_freq_nrow, expected_freqids, expected_ifnos)
        
    def test_freqtol13( self ):
        """test_freqtol13: check if exactly same FREQUENCIES entries are merged by default"""
        infiles = self.inlist
        
        # edit self.inlist[1] so that it suites with testing purpose
        # 1. renumber IFNO in second tables according to FREQ_ID
        tb.open(infiles[1], nomodify=False)
        freqids = tb.getcol('FREQ_ID')
        tb.putcol('IFNO', freqids)
        tb.close()

        # expected nrows for outfile HISTORY (must be evaluated before
        # running sdcoadd)
        expected_hist_nrow = self._nrow(infiles, 'HISTORY')

        # run sdcoadd
        result = sdcoadd(infiles=infiles,outfile=self.outfile)
        self.assertEqual(result,None)

        # verification
        expected_main_nrow = self._nrow(infiles)
        expected_freq_nrow = 4
        expected_freqids = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 3, 3]
        expected_ifnos = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 3, 3]
        self._verify(self.outfile, expected_main_nrow, expected_hist_nrow, expected_freq_nrow, expected_freqids, expected_ifnos)
    
    def test_freqtol14(self):
        """test_freqtol14: check if numeric tolerance value works as expected"""
        infiles = self.inlist
        
        # edit self.inlist[1] so that it suites with testing purpose
        # 1. renumber IFNO in second tables according tao FREQ_ID
        # 2. shift INCREMENT by an amount equal to tolerance
        # 3. change REFPIX to 0.0 for FREQUENCIES rows that are supposed
        #    to be merged
        tb.open(infiles[1], nomodify=False)
        freqids = tb.getcol('FREQ_ID')
        tb.putcol('IFNO', freqids)
        tb.close()
        tb.open(os.path.join(infiles[1], 'FREQUENCIES'), nomodify=False)
        increment = tb.getcell('INCREMENT', 1)
        increment += self.equal_tol
        tb.putcell('INCREMENT', 1, increment)
        tb.putcell('REFPIX', 1, 0.0)
        tb.close()
        tb.open(os.path.join(infiles[0], 'FREQUENCIES'), nomodify=False)
        tb.putcell('REFPIX', 3, 0.0)
        tb.close()

        # expected nrows for outfile HISTORY 
        expected_hist_nrow = self._nrow(infiles, 'HISTORY')

        # run sdcoadd
        result = sdcoadd(infiles=infiles,outfile=self.outfile,freqtol=self.equal_tol)
        self.assertEqual(result,None)

        # verification
        expected_main_nrow = self._nrow(infiles)
        expected_freq_nrow = 4
        expected_freqids = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 3, 3]
        expected_ifnos = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 3, 3]
        self._verify(self.outfile, expected_main_nrow, expected_hist_nrow, expected_freq_nrow, expected_freqids, expected_ifnos)

    def test_freqtol15( self ):
        """test_freqtol15: check if single MOLECULE_ID is assigned to each IFNO when there are two MOLECULE_ID's that refers different rest frequencies"""
        infiles = self.inlist
        
        # edit self.inlist[1] so that it suites with testing purpose
        # 1. renumber IFNO in second tables according to FREQ_ID
        # 2. renumber MOLECULE_ID and ID in MOLECULES table
        tb.open(infiles[1], nomodify=False)
        freqids = tb.getcol('FREQ_ID')
        tb.putcol('IFNO', freqids)
        tb.putcol('MOLECULE_ID', numpy.ones(tb.nrows()))
        tb.close()
        tb.open(os.path.join(infiles[1], 'MOLECULES'), nomodify=False)
        tb.putcell('ID', 0, 1)
        tb.putcell('RESTFREQUENCY', 0, [1.0e11])
        tb.close()

        # expected nrows for outfile HISTORY (must be evaluated before
        # running sdcoadd)
        expected_hist_nrow = self._nrow(infiles, 'HISTORY')

        # run sdcoadd
        result = sdcoadd(infiles=infiles,outfile=self.outfile,freqtol=self.freqtol)
        self.assertEqual(result,None)

        # verification
        expected_main_nrow = self._nrow(infiles)
        expected_freq_nrow = 4
        expected_freqids = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 3, 3]
        expected_ifnos = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 3, 3]
        expected_molids = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        self._verify(self.outfile, expected_main_nrow, expected_hist_nrow, expected_freq_nrow, expected_freqids, expected_ifnos, expected_molids)

    def test_freqtol16( self ):
        """test_freqtol16: check if single MOLECULE_ID is assigned to each IFNO when there are two MOLECULE_ID's that refers same rest frequency"""
        infiles = self.inlist
        
        # edit self.inlist[1] so that it suites with testing purpose
        # 1. renumber IFNO in second tables according to FREQ_ID
        # 2. renumber MOLECULE_ID and ID in MOLECULES table
        tb.open(infiles[1], nomodify=False)
        freqids = tb.getcol('FREQ_ID')
        tb.putcol('IFNO', freqids)
        tb.putcol('MOLECULE_ID', numpy.ones(tb.nrows()))
        tb.close()
        tb.open(os.path.join(infiles[1], 'MOLECULES'), nomodify=False)
        tb.putcell('ID', 0, 1)
        tb.close()

        # expected nrows for outfile HISTORY (must be evaluated before
        # running sdcoadd)
        expected_hist_nrow = self._nrow(infiles, 'HISTORY')

        # run sdcoadd
        result = sdcoadd(infiles=infiles,outfile=self.outfile,freqtol=self.freqtol)
        self.assertEqual(result,None)

        # verification
        expected_main_nrow = self._nrow(infiles)
        expected_freq_nrow = 4
        expected_freqids = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 3, 3]
        expected_ifnos = [0, 0, 1, 1, 2, 2, 3, 3, 0, 0, 3, 3]
        expected_molids = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
        self._verify(self.outfile, expected_main_nrow, expected_hist_nrow, expected_freq_nrow, expected_freqids, expected_ifnos, expected_molids)

    def test_freqtol17(self):
        """test_freqtol17: fail if invalid value is given"""
        infiles = self.inlist
        
        # run sdcoadd
        try:
            result = sdcoadd(infiles=infiles,outfile=self.outfile,freqtol='None')
        except RuntimeError, e:
            self.assertNotEqual(str(e).find('Failed to convert freqTol string to quantity'), -1,
                             msg='Unexpected exception is thrown: \'%s\''%(str(e)))
        except Exception, e:
            self.fail('Unexpected exception is thrown: \'%s\''%(str(e)))
            
    

        
class sdcoadd_storageTest( sdcoadd_unittest_base, unittest.TestCase ):
    """
    Unit tests for task sdcoadd. Test scantable sotrage and insitu
    parameters

    The list of tests:
    testMT  --- storage = 'memory', insitu = True
    testMF  --- storage = 'memory', insitu = False
    testDT  --- storage = 'disk', insitu = True
    testDF  --- storage = 'disk', insitu = False

    Note on handlings of disk storage:
       Task script restores unit and frame information.
       scantable.convert_flux returns new table.

    Tested items:
       1. Number of rows in (sub-)tables and list of IDs of output scantable.
       2. Units and coordinates of output scantable.
       3. units and coordinates of input scantables before/after run.
    """
    # a list of input scatables to merge
    inlist = ['orions_calSave_21.asap','orions_calSave_25.asap']
    outname = sdcoadd_unittest_base.taskname+".merged"

    # Reference data of merged scantable
    # merge result of scan 21 and 25 (no overlap in IF and MOL_ID data
    # but the same IDs are assigned ... requires proper addition of IDs by
    # checking of subtable rows)
    ref2125 = {"nMAIN": 16, "nFOCUS": 1, "nFREQUENCIES": 8, "nMOLECULES": 2,\
               "SCANNOS": range(2), "FOCUS_IDS": range(1),\
               "FREQ_IDS": range(8),"MOLECULE_IDS": range(2)}

    def setUp( self ):
        # copy input scantables
        for infile in self.inlist:
            if os.path.exists(infile):
                shutil.rmtree(infile)
            shutil.copytree(self.datapath+infile, infile)
        # back up the original settings
        self.storage = sd.rcParams['scantable.storage']
        self.insitu = sd.rcParams['insitu']

        default(sdcoadd)

        self.merge_uc = self._get_unit_coord(self.inlist[0])

    def tearDown( self ):
        # restore settings
        sd.rcParams['scantable.storage'] = self.storage
        sd.rcParams['insitu'] = self.insitu
        for thefile in self.inlist:
            if (os.path.exists(thefile)):
                shutil.rmtree(thefile)

    # helper functions of tests
    def _get_unit_coord( self, scanname ):
        # Returns a dictionary which stores units and coordinates of a
        # scantable, scanname. Returned dictionary stores spectral
        # unit, flux unit, frequency frame, and doppler of scanname.
        self.assertTrue(os.path.exists(scanname),\
                        "'%s' does not exists." % scanname)
        self.assertTrue(is_scantable(scanname),\
                        "Input table is not a scantable: %s" % scanname)
        scan = sd.scantable(scanname, average=False,parallactify=False)
        retdict = {}
        retdict['spunit'] = scan.get_unit()
        retdict['flunit'] = scan.get_fluxunit()
        coord = scan._getcoordinfo()
        retdict['frame'] = coord[1]
        retdict['doppler'] = coord[2]
        return retdict

    def _get_uclist( self, stlist ):
        # Returns a list of dictionaries of units and coordinates of
        # a list of scantables in stlist. This method internally calls
        # _get_unit_coord(scanname).
        retlist = []
        for scanname in stlist:
            retlist.append(self._get_unit_coord(scanname))
        print retlist
        return retlist

    def _comp_unit_coord( self, stlist, before):
        ### stlist: a list of scantable names
        if isinstance(stlist,str):
            stlist = [ stlist ]
        ### before: a return value of _get_uclist() before run
        if isinstance(before, dict):
            before = [ before ]
        if len(stlist) != len(before):
            raise Exception("Number of scantables in list is different from reference data.")
        self.assertTrue(isinstance(before[0],dict),\
                        "Reference data should be (a list of) dictionary")

        after = self._get_uclist(stlist)
        for i in range(len(stlist)):
            print "Comparing units and coordinates of '%s'" %\
                  stlist[i]
            self._compareDictVal(after[i],before[i])

    def _get_ids(self, infiles):
        id_dict = {}
        id_keys = ['SCANNO', 'FREQ_ID', 'FOCUS_ID', 'MOLECULE_ID']
        for f in infiles:
            tb.open(f)
            id_dict[f] = dict([(k,tb.getcol(k)) for k in id_keys])
            tb.close()
        return id_dict

    def _compare_ids(self, out, ref):
        for f in out.keys():
            for k in out[f].keys():
                casalog.post('%s before sdcoadd: %s = %s'%(f,k,ref[f][k]))
                casalog.post('%s after sdcoadd: %s = %s'%(f,k,out[f][k]))
                self.assertTrue(all(out[f][k]==ref[f][k]),
                                    msg='%s of %s may be modified by sdcoadd'%(k,f))
            
    # Actual tests
    def testMT( self ):
        """Storage Test MT: sdcoadd on storage='memory' and insitu=T"""
        tid = "MT"
        infiles = self.inlist
        outfile = self.outname+tid
        fluxunit = self.merge_uc['flunit']
        specunit = self.merge_uc['spunit']
        frame = self.merge_uc['frame']
        doppler = self.merge_uc['doppler']

        # Backup units and coords of input scantable before run.
        initval = self._get_uclist(infiles)
        id_ref = self._get_ids(infiles)

        sd.rcParams['scantable.storage'] = 'memory'
        sd.rcParams['insitu'] = True
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        result = sdcoadd(infiles=infiles,outfile=outfile)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        # Test merged scantable
        merged = self._get_scantable_params(outfile)
        self._test_merged_scantable(merged, self.ref2125)
        self._comp_unit_coord(outfile,self.merge_uc)

        # Compare units and coords of input scantable before/after run
        self._comp_unit_coord(infiles,initval)

        id_out = self._get_ids(infiles)
        self._compare_ids(id_out, id_ref)

    def testMF( self ):
        """Storage Test MF: sdcoadd on storage='memory' and insitu=F"""
        tid = "MF"
        infiles = self.inlist
        outfile = self.outname+tid
        fluxunit = self.merge_uc['flunit']
        specunit = self.merge_uc['spunit']
        frame = self.merge_uc['frame']
        doppler = self.merge_uc['doppler']

        # Backup units and coords of input scantable before run.
        initval = self._get_uclist(infiles)
        id_ref = self._get_ids(infiles)

        sd.rcParams['scantable.storage'] = 'memory'
        sd.rcParams['insitu'] = False
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        result = sdcoadd(infiles=infiles,outfile=outfile)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        # Test merged scantable
        merged = self._get_scantable_params(outfile)
        self._test_merged_scantable(merged, self.ref2125)
        self._comp_unit_coord(outfile,self.merge_uc)

        # Compare units and coords of input scantable before/after run
        self._comp_unit_coord(infiles,initval)

        id_out = self._get_ids(infiles)
        self._compare_ids(id_out, id_ref)

    def testDT( self ):
        """Storage Test DT: sdcoadd on storage='disk' and insitu=T"""
        tid = "DT"
        infiles = self.inlist
        outfile = self.outname+tid
        fluxunit = self.merge_uc['flunit']
        specunit = self.merge_uc['spunit']
        frame = self.merge_uc['frame']
        doppler = self.merge_uc['doppler']

        # Backup units and coords of input scantable before run.
        initval = self._get_uclist(infiles)
        id_ref = self._get_ids(infiles)

        sd.rcParams['scantable.storage'] = 'disk'
        sd.rcParams['insitu'] = True
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        result = sdcoadd(infiles=infiles,outfile=outfile)
        
        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        # Test merged scantable
        merged = self._get_scantable_params(outfile)
        self._test_merged_scantable(merged, self.ref2125)
        self._comp_unit_coord(outfile,self.merge_uc)

        # Compare units and coords of input scantable before/after run
        self._comp_unit_coord(infiles,initval)

        id_out = self._get_ids(infiles)
        self._compare_ids(id_out, id_ref)

    def testDF( self ):
        """Storage Test DF: sdcoadd on storage='disk' and insitu=F"""
        tid = "DF"
        infiles = self.inlist
        outfile = self.outname+tid
        fluxunit = self.merge_uc['flunit']
        specunit = self.merge_uc['spunit']
        frame = self.merge_uc['frame']
        doppler = self.merge_uc['doppler']

        # Backup units and coords of input scantable before run.
        initval = self._get_uclist(infiles)
        id_ref = self._get_ids(infiles)

        sd.rcParams['scantable.storage'] = 'disk'
        sd.rcParams['insitu'] = False
        print "Running test with storage='%s' and insitu=%s" % \
              (sd.rcParams['scantable.storage'], str(sd.rcParams['insitu']))
        result = sdcoadd(infiles=infiles,outfile=outfile)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        # Test merged scantable
        merged = self._get_scantable_params(outfile)
        self._test_merged_scantable(merged, self.ref2125)
        self._comp_unit_coord(outfile,self.merge_uc)

        # Compare units and coords of input scantable before/after run
        self._comp_unit_coord(infiles,initval)

        id_out = self._get_ids(infiles)
        self._compare_ids(id_out, id_ref)

class sdcoadd_flagTest( sdcoadd_unittest_base, unittest.TestCase ):
    """
    Test for flag data handling. actually nothing to do with flagtra and flagrows.
    """
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdcoadd/'
    # Input and output names
    num_repeat = 2
    inlist = ['flagtest.asap'] * num_repeat
    outname = "sdcoadd_out.asap"
    
    def setUp( self ):
        for infile in self.inlist:
            if os.path.exists(infile):
                shutil.rmtree(infile)
            shutil.copytree(self.datapath+infile, infile)
        default(sdcoadd)
        self._getinfo(self.inlist[0], self.num_repeat)

    def tearDown( self ):
        for thefile in self.inlist + [self.outname]:
            if (os.path.exists(thefile)):
                shutil.rmtree(thefile)

    def testflag( self ):
        """Flag Test"""
        result = sdcoadd(infiles=self.inlist,outfile=self.outname)
        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(self.outname),msg="No output written")
        self._verifyflag(self.outname)

    def _getinfo(self, infile, num_repeat):
        with tbmanager(infile) as tb:
            self.nrow_orig = tb.nrows()
            self.nchan_orig = len(tb.getcell('FLAGTRA', 0))
            self.rowid_rflag_orig = numpy.array([])
            rflag = tb.getcol('FLAGROW')
            for i in xrange(self.num_repeat):
                self.rowid_rflag_orig = numpy.r_[self.rowid_rflag_orig, rflag]
            self.rowid_cflag_orig = numpy.array([])
            cfraw = tb.getcol('FLAGTRA').sum(axis=0)
            cflag = numpy.array([cfraw[i] > 0 for i in xrange(len(cfraw))])
            for i in xrange(self.num_repeat):
                self.rowid_cflag_orig = numpy.r_[self.rowid_cflag_orig, cflag]
            self.cflag_orig = tb.getcell('FLAGTRA', numpy.where(self.rowid_cflag_orig)[0][0])

    def _verifyflag(self, outfile):
        with tbmanager(outfile) as tb:
            self.assertEqual(tb.nrows(), self.nrow_orig * self.num_repeat)
            self.assertTrue(all(tb.getcol('FLAGROW')==self.rowid_rflag_orig))
            for i in xrange(tb.nrows()):
                mask = tb.getcell('FLAGTRA', i)
                mask_ref = self.cflag_orig if self.rowid_cflag_orig[i] else numpy.zeros(self.nchan_orig, numpy.int32)
                self.assertTrue(all(mask == mask_ref))

    def get_index(self, value, list):
        try:
            return list.index(value)
        except:
            return -1
    
def suite():
    return [sdcoadd_basicTest, sdcoadd_mergeTest,
            sdcoadd_storageTest, sdcoadd_freqtolTest,
            sdcoadd_flagTest]
