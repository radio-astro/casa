import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest
import numpy

import asap as sd
from sdflag2old import sdflag2old

class sdflag2old_test(unittest.TestCase):
    """
    Basic unit tests for task sdflag2old.

    The list of tests:
    test00   --- test channel flagging/unflagging
    test01   --- test row flagging/unflagging
    test02   --- test clipping flagging/unflagging
    test03   --- test no data after selection (raises an exception)

    ***NOTE*** These tests are for Scantable only. Tests for the other formats
               which ASAP supports, including MS and SDFITS, are to be made later.

    NOTE2: 'Artificial_Flat.asap' has 6 flat spectra with rms of 1.0.

    """
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdflag/'
    # Input and output names
    infile = 'Artificial_Flat.asap'

    def setUp(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)

        default(sdflag2old)

    def tearDown(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)

    def test00(self):
        """Test 0: channel flagging"""
        infile = self.infile
        mode = 'manual'
        maskflag = [[1,3],[10,15]]

        #flag
        result = sdflag2old(infile=infile, mode=mode, maskflag=maskflag)

        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")

        scan = sd.scantable(filename=infile, average=False)
        ansmask = scan.create_mask(maskflag, invert=True)
        for i in range (scan.nrow()):
            mask = scan.get_mask(i)
            res = True
            for j in range(len(mask)):
                if mask[j] != ansmask[j]:
                    res = False
                    break
            self.assertTrue(res)
        del scan

        #unflag
        result = sdflag2old(infile=infile, mode=mode, maskflag=maskflag,
                         unflag=True)

        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")

        scan = sd.scantable(filename=infile, average=False)
        for i in range (scan.nrow()):
            mask = scan.get_mask(i)
            res = True
            for j in range(len(mask)):
                if mask[j] != True:
                    res = False
                    break
            self.assertTrue(res)
        del scan

    def test01(self):
        """Test 1: row flagging"""
        infile = self.infile
        mode = 'rowid'
        flagrow = [2,4]

        #flag
        result = sdflag2old(infile=infile, mode=mode, rows=flagrow)

        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")

        scan = sd.scantable(filename=infile, average=False)
        for i in range (scan.nrow()):
            resrflag = scan._getflagrow(i)
            ansrflag = False
            for j in flagrow:
                if i == j:
                    ansrflag = True
                    break
            res = (resrflag == ansrflag)
            self.assertTrue(res)
        del scan

        #unflag
        result = sdflag2old(infile=infile, mode=mode, rows=flagrow,
                         unflag=True)

        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")

        scan = sd.scantable(filename=infile, average=False)
        for i in range (scan.nrow()):
            resrflag = scan._getflagrow(i)
            res = (resrflag == False)
            self.assertTrue(res)
        del scan

    def test02(self):
        """Test 2: clipping"""
        infile = self.infile
        mode = 'clip'
        #clip = True
        clipminmax = [-3.5, 3.5] #clip at 3.5-sigma level, i.e., flag channels at which abs(value) exceeds 3.5.

        #flag
        result = sdflag2old(infile=infile, mode=mode, clipminmax=clipminmax)

        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")

        ansmlist = [
            [[0,301],[303,8191]],
            [[0,3326],[3328,7753],[7755,8191]],
            [[0,6545],[6547,8191]],
            [[0,3996],[3998,7920],[7922,8191]],
            [[0,3274],[3276,8191]],
            [[0,1708],[1710,7710],[7712,7799],[7801,8191]]
            ]

        scan = sd.scantable(filename=infile, average=False)
        for i in range (scan.nrow()):
            mask = scan.get_mask(i)
            ansmask = scan.create_mask(ansmlist[i])
            res = True
            for j in range(len(mask)):
                if mask[j] != ansmask[j]:
                    res = False
                    break
            self.assertTrue(res)
        del scan

        #unflag
        result = sdflag2old(infile=infile, mode=mode, clipminmax=clipminmax,
                         unflag=True)

        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")

        scan = sd.scantable(filename=infile, average=False)
        for i in range (scan.nrow()):
            mask = scan.get_mask(i)
            res = True
            for j in range(len(mask)):
                if mask[j] != True:
                    res = False
                    break
            self.assertTrue(res)
        del scan

    def test03(self):
        """Test 3: Failure case (No data to flag)"""
        #test for the default parameters (raises an exception)
        infile = self.infile
        iflist = [10] # non-existent IF value
        maskflag = [0,3]
        mode = 'manual'
        try:
            result = sdflag2old(infile=infile, mode=mode, ifs=iflist,
                             maskflag=maskflag)
        except Exception, e:
            pos = str(e).find('Selection contains no data. Not applying it.')
            self.assertNotEqual(pos, -1, msg='Unexpected exception was thrown: %s'%(str(e)))

class sdflag2old_test_timerange(unittest.TestCase):
    """
    Basic unit tests for task sdflag2old.

    The list of tests:
    test01   --- test row flagging with selection by timerange 'T0~T1'
    test02   --- test row flagging with selection by timerange 'T0'
    test03   --- test row flagging with selection by timerange 'T0+dT'
    test04   --- test row flagging with selection by timerange '>T0'
    test05   --- test row flagging with selection by timerange '<T0'
    test06   --- test row flagging with selection by timerange '>T0'
                 and scan number
    test07   --- test row flagging with selection by timerange '>T0'
                 and field name
    test08   --- test row flagging with selection by timerange 'T0~T1'
                 where T0 and T1 are incomplete time strings
    test09   --- test row flagging with selection by timerange '>T0'
                 where T0 is incomplete time string
    test10   --- test row flagging with selection by timerange 'T0~T1'
                 where only T1 is incomplete time strings
    test11   --- test selection by timerange 'T0~T1' is inclusive

    ***NOTE*** These tests are for Scantable only. Tests for the other formats
               which ASAP supports, including MS and SDFITS, are to be made later.

    NOTE2: 'Artificial_Flat.asap' has 6 flat spectra with rms of 1.0.

    """
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdflag/'
    # Input and output names
    infile = 'Artificial_Flat.asap'

    def setUp(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)

        # edit TIME column for testing
        table = gentools(['tb'])[0]
        table.open(self.infile, nomodify=False)
        time_column = table.getcol('TIME')
        for irow in xrange(table.nrows()):
            time_column[irow] += irow * 1000.0 / 86400.0
        table.putcol('TIME', time_column)
        table.close()

        default(sdflag2old)

    def tearDown(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)

    def verify(self, data, flag_row_expected):
        scan = sd.scantable(self.infile, average=False)
        flag_row = numpy.array([scan._getflagrow(i) for i in xrange(scan.nrow())], dtype=bool)
        del scan
        self.assertTrue(all(flag_row_expected==flag_row),
                        msg='FLAGROW is different from expected value: %s (expected: %s)'%(flag_row, flag_row_expected))
        
    def test01(self):
        """timerange test01: test row flagging with selection by timerange 'T0~T1'"""
        # first two rows should be flagged
        timerange = '2006/01/19/01:50:00~2006/01/19/02:24:00'
        flag_row_expected = numpy.array([True, True, False, False, False, False], dtype=bool)
        sdflag2old(infile=self.infile, mode='manual', timerange=timerange)

        # verification
        self.verify(self.infile, flag_row_expected)
        
    def test02(self):
        """timerange test02: test row flagging with selection by timerange 'T0'"""
        # only second row should be flagged
        timerange = '2006/01/19/02:16:45.0'
        flag_row_expected = numpy.array([False, True, False, False, False, False], dtype=bool)
        sdflag2old(infile=self.infile, mode='manual', timerange=timerange)

        # verification
        self.verify(self.infile, flag_row_expected)

    def test03(self):
        """timerange test03: test row flagging with selection by timerange 'T0+dT'"""
        # second and third rows should be flagged
        timerange = '2006/01/19/02:08:00.0+0:30:00'
        flag_row_expected = numpy.array([False, True, True, False, False, False], dtype=bool)
        sdflag2old(infile=self.infile, mode='manual', timerange=timerange)

        # verification
        self.verify(self.infile, flag_row_expected)

    def test04(self):
        """timerange test04: test row flagging with selection by timerange '>T0'"""
        # fourth and subsequent rows should be flagged
        timerange = '>2006/01/19/02:42:00'
        flag_row_expected = numpy.array([False, False, False, True, True, True], dtype=bool)
        sdflag2old(infile=self.infile, mode='manual', timerange=timerange)

        # verification
        self.verify(self.infile, flag_row_expected)

    def test05(self):
        """timerange test05: test row flagging with selection by timerange '<T0'"""
        # only first row should be flagged
        timerange = '<2006/01/19/02:00:00'
        flag_row_expected = numpy.array([True, False, False, False, False, False], dtype=bool)
        sdflag2old(infile=self.infile, mode='manual', timerange=timerange)

        # verification
        self.verify(self.infile, flag_row_expected)

    def test06(self):
        """timerange test06: test row flagging with selection by timerange '>T0' and scan number"""
        # only fifth row should be flagged
        timerange = '>2006/01/19/02:42:00'
        flag_row_expected = numpy.array([False, False, False, False, True, False], dtype=bool)
        scans = [4]
        sdflag2old(infile=self.infile, mode='manual', scans=scans, timerange=timerange)

        # verification
        self.verify(self.infile, flag_row_expected)

    def test07(self):
        """timerange test06: test row flagging with selection by timerange '>T0' and field name"""
        # edit infile
        table = gentools(['tb'])[0]
        table.open(self.infile, nomodify=False)
        table.putcell('SRCNAME', 3, 'SDFLAG2_TEST')
        table.close()
        
        # only fourth row should be flagged
        timerange = '>2006/01/19/02:42:00'
        field = 'SDFLAG2*'
        flag_row_expected = numpy.array([False, False, False, True, False, False], dtype=bool)
        scans = [4]
        sdflag2old(infile=self.infile, mode='manual', field=field, timerange=timerange)

        # verification
        self.verify(self.infile, flag_row_expected)

    def test08(self):
        """timerange test08: test row flagging with selection by timerange 'T0~T1' where T0 and T1 are incomplete time string"""
        # first two rows should be flagged
        timerange = '01:50:00~02:24:00'
        flag_row_expected = numpy.array([True, True, False, False, False, False], dtype=bool)
        sdflag2old(infile=self.infile, mode='manual', timerange=timerange)

        # verification
        self.verify(self.infile, flag_row_expected)
        
    def test09(self):
        """timerange test09: test row flagging with selection by timerange '>T0'"""
        # fourth and subsequent rows should be flagged
        timerange = '>02:42:00'
        flag_row_expected = numpy.array([False, False, False, True, True, True], dtype=bool)
        sdflag2old(infile=self.infile, mode='manual', timerange=timerange)

        # verification
        self.verify(self.infile, flag_row_expected)

    def test10(self):
        """timerange test10: test row flagging with selection by timerange 'T0~T1' where only T1 is incomplete time string"""
        # first two rows should be flagged
        timerange = '2006/01/19/01:50:00~02:24:00'
        flag_row_expected = numpy.array([True, True, False, False, False, False], dtype=bool)
        sdflag2old(infile=self.infile, mode='manual', timerange=timerange)

        # verification
        self.verify(self.infile, flag_row_expected)

    def test11(self):
        """test11: test selection by timerange 'T0~T1' is inclusive"""
        # edit infile
        T0 = '2006/01/19/01:52:05'
        T1 = '2006/01/19/02:08:45'
        qa = qatool()
        mjd0 = qa.totime(T0)['value']
        mjd1 = qa.totime(T1)['value']
        table = gentools(['tb'])[0]
        table.open(self.infile, nomodify=False)
        table.putcell('TIME', 0, mjd0)
        table.putcell('TIME', 1, mjd1)
        table.close()
        
        # first two rows should be flagged
        timerange = '%s~%s'%(T0,T1)
        flag_row_expected = numpy.array([True, True, False, False, False, False], dtype=bool)
        sdflag2old(infile=self.infile, mode='manual', timerange=timerange)

        # verification
        self.verify(self.infile, flag_row_expected)

    def test12(self):
        """test12: test with the data that all rows are flagged"""
        # edit infile
        table = gentools(['tb'])[0]
        table.open(self.infile, nomodify=False)
        flagrow = table.getcol('FLAGROW')
        flagrow[:] = 1
        table.putcol('FLAGROW', flagrow)
        table.close()
        
        # all rows are flagged
        T0 = '2006/01/19/01:52:05'
        T1 = '2006/01/19/02:08:45'
        timerange = '%s~%s'%(T0,T1)
        flag_row_expected = numpy.array([True, True, True, True, True, True], dtype=bool)
        sdflag2old(infile=self.infile, mode='manual', timerange=timerange)

        # verification
        self.verify(self.infile, flag_row_expected)
        
        
        
def suite():
    return [sdflag2old_test, sdflag2old_test_timerange]
