import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
from asap_init import * 
import unittest
#

asap_init()
from sdcoadd import sdcoadd
import asap as sd
from asap.scantable import is_scantable, is_ms

mergeids = {"FOCUS_ID": "FOCUS", "FREQ_ID": "FREQUENCIES", \
            "MOLECULE_ID": "MOLECULES"}
addedtabs = []#"HISTORY"] #, "FIT", "TCAL", "WEATHER"]


def _get_scantable_params(scanname):
    """
    Returns a dictionary which contains row numbers and a set of
    data in scantable for later verifications
    """
    if not os.path.exists(scanname):
        raise Exception, "A scantable '%s' does not exists." % scanname
    if not is_scantable(scanname):
        raise Exception, "Input file '%s' is not a scantable." % scanname
    
    res = {}
    for tab in addedtabs + mergeids.values():
        tb.open(scanname+"/"+tab)
        res["n"+tab] = tb.nrows()
        tb.close()

    # open main table
    tb.open(scanname)
    res["nMAIN"] = tb.nrows()
    for col in ["SCANNO"] + mergeids.keys():
        res[col+"S"] = list(set(tb.getcol(col)))
    tb.close()
    return res


class sdcoadd_basictest(unittest.TestCase):
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
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdcoadd/'
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
    
    def setUp(self):
        # copy input scantables
        for infile in self.inlist:
            if os.path.exists(infile):
                shutil.rmtree(infile)
            shutil.copytree(self.datapath+infile, infile)

        default(sdcoadd)

    def tearDown(self):
        for thefile in self.inlist + [self.outname]:
            if (os.path.exists(thefile)):
                shutil.rmtree(thefile)

    def test00(self):
        """Test 0: Default parameters (raises an error)"""
        result = sdcoadd()
        self.assertFalse(result)

    def test01(self):
        """Test 1: valid infiles (scantables) WITHOUT outfile"""
        infiles = self.inlist[0:2]
        result = sdcoadd(infiles=infiles)

        self.assertEqual(result,None)
        # test merged scantable
        outname = self.inlist[0]+"_coadd"
        self.assertTrue(os.path.exists(outname),msg="No output written")
        merged = _get_scantable_params(outname)
        self._test_merged_scantable(merged, self.ref2125)

    def test02(self):
        """Test 2: valid infiles (scantable) with outfile"""
        infiles = self.inlist[0:2]
        outfile = self.outname
        result = sdcoadd(infiles=infiles,outfile=outfile)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        # test merged scantable
        merged = _get_scantable_params(outfile)
        self._test_merged_scantable(merged, self.ref2125)

    def test03(self):
        """Test 3: outform='MS'"""
        infiles = self.inlist[0:2]
        outfile = self.outname.rstrip('.asap')+'.ms'
        outform = 'MS2'
        result = sdcoadd(infiles=infiles,outfile=outfile,outform=outform)
        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")

        self.assertTrue(is_ms(outfile),msg="Output file is not an MS")

    def test04(self):
        """Test 4: overwrite=False (raises an error)"""
        infiles = self.inlist[1:3]
        outfile = self.outname
        result = sdcoadd(infiles=infiles,outfile=outfile)
        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")

        infiles = self.inlist[0:2]
        outfile = self.outname
        result2 = sdcoadd(infiles=infiles,outfile=outfile)
        self.assertFalse(result2)


    def test05(self):
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
        merged = _get_scantable_params(outfile)
        self._test_merged_scantable(merged, self.ref2125)

    def test06(self):
        """Test 6: Merge 3 scantables"""
        infiles = self.inlist
        outfile = self.outname
        result = sdcoadd(infiles=infiles,outfile=outfile)

        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")
        # test merged scantable
        merged = _get_scantable_params(outfile)
        self._test_merged_scantable(merged, self.ref212523)

    def test07(self):
        """Test 7: specify only a scantables (raises an error)"""
        infiles = self.inlist[0]
        outfile = self.outname
        result = sdcoadd(infiles=infiles,outfile=outfile)
        self.assertFalse(result)

    def _test_merged_scantable(self, res, ref):
        for key in ref.iterkeys():
            self.assertTrue(res.has_key(key),\
                            msg="%s is not defined in the current results."\
                            % key)
            val = res.pop(key)
            self.assertTrue(val==ref[key],\
                            msg="'%s' = %s (expected: %s) " % \
                            (key, str(val), str(ref[key])))
        self.assertTrue(len(res)==0,\
                        msg="%s are not defined in reference data" % \
                        str(res.keys()))


class sdcoadd_mergetest(unittest.TestCase):
    """
    Test capabilities of sd.merge(). No averaging and interactive testing.

    The list of tests:
    mtest01   --- test merge of HISTORY subtables
    mtest02   --- test proper merge of FREQUENCIES subtables
    mtest03   --- test proper merge of MOLECULE subtables
    """
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdcoadd/'
    # Input and output names
    # orions_calSave_21if03.asap: nROW=4, IF=[0,3], scan=[21], MOL=[0], FOC=[0]
    # orions_calSave_23.asap: nROW=8, IF=[0-3], scan=[23], MOL=[0], FOC=[0]
    # orions_calSave_2327.asap: nROW=16, IF=[0-7], scan=[23,27], MOL=[0], FOC=[0]
    inlist = []
    outname = "sdcoadd_out.asap"
    
    def setUp(self):
        default(sdcoadd)

    def tearDown(self):
        for thefile in self.inlist + [self.outname]:
            if (os.path.exists(thefile)):
                shutil.rmtree(thefile)

    def _copy_inputs(self):
        # copy input scantables
        for infile in self.inlist:
            if os.path.exists(infile):
                shutil.rmtree(infile)
            shutil.copytree(self.datapath+infile, infile)

    def testmerge01(self):
        """Merge Test 1: merge of HISTORY subtables"""
        self.inlist = ['orions_calSave_21if03.asap','orions_calSave_23.asap']
        self._copy_inputs()

        infiles = self.inlist
        outfile = self.outname
        result = sdcoadd(infiles=infiles,outfile=outfile)
        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")

        htab="/HISTORY"
        tb.open(outfile+htab)
        outnhist = tb.nrows()
        tb.close()
        innhist = 0
        for tab in infiles:
            tb.open(tab+htab)
            innhist += tb.nrows()
            tb.close()
        self.assertTrue(outnhist >= innhist,\
                        msg="nHIST = %d (expected: >= %d)" % (outnhist, innhist))
    def testmerge02(self):
        """Merge Test 2: proper merge of FREQUENCIES subtables"""
        self.inlist = ['orions_calSave_21if03.asap','orions_calSave_23.asap']
        self._copy_inputs()
        
        infiles = self.inlist
        outfile = self.outname
        result = sdcoadd(infiles=infiles,outfile=outfile)
        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")

        merged = _get_scantable_params(outfile)
        # check FREQ_ID
        self.assertEqual(merged["nFREQUENCIES"],4,msg="nFREQUENCIES is not correct")
        self.assertTrue(merged["FREQ_IDS"]==range(4),\
                        msg="FREQ_IDS = %s (expected: %s)" % \
                        (str(merged["FREQ_IDS"]), str(range(4))))
        # check main row number
        self.assertEqual(merged["nMAIN"],12,msg="nMAIN is not correct")


    def testmerge03(self):
        """Merge Test 3: proper merge of MOLECULES subtables"""
        self.inlist = ['orions_calSave_21if03.asap','orions_calSave_2327.asap']
        self._copy_inputs()

        infiles = self.inlist
        outfile = self.outname
        result = sdcoadd(infiles=infiles,outfile=outfile)
        self.assertEqual(result,None)
        self.assertTrue(os.path.exists(outfile),msg="No output written")

        merged = _get_scantable_params(outfile)
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

        

def suite():
    return [sdcoadd_basictest, sdcoadd_mergetest]
