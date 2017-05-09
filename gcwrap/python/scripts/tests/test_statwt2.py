import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest
import math
import numpy
import numbers

src = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/simplecluster/ngc5921.split.ms'
refdir = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/statwt/'

def _get_dst_cols(dst):
    mytb = tbtool()
    mytb.open(dst)
    wt = mytb.getcol("WEIGHT")
    wtsp = mytb.getcol("WEIGHT_SPECTRUM")
    flag = mytb.getcol("FLAG")
    frow = mytb.getcol("FLAG_ROW")
    data = mytb.getcol("CORRECTED_DATA")
    mytb.close()
    return [wt, wtsp, flag, frow, data]

def _get_table_cols(mytb):
    times = mytb.getcol("TIME")
    wt = mytb.getcol("WEIGHT")
    wtsp = mytb.getcol("WEIGHT_SPECTRUM")
    flag = mytb.getcol("FLAG")
    frow = mytb.getcol("FLAG_ROW")
    data = mytb.getcol("CORRECTED_DATA")
    return [times, wt, wtsp, flag, frow, data]

class statwt2_test(unittest.TestCase):

    def test_algorithm(self):
        """ Test the algorithm"""
        mytb = tbtool()
        mytb.open(src)
        expflag = mytb.getcol("FLAG")
        expfrow = mytb.getcol("FLAG_ROW")
        mytb.done()
        dst = "ngc5921.split.ms"
        for i in [0,1]:
            shutil.copytree(src, dst) 
            myms = mstool()
            if i == 0:
                myms.open(dst, nomodify=False)
                myms.statwt2()
                myms.done()
            else:
                statwt2(dst)
            [wt, wtsp, flag, frow, data] = _get_dst_cols(dst)
            dr = numpy.real(data)
            di = numpy.imag(data)
            myshape = wtsp.shape
            ncorr = myshape[0]
            nrow = myshape[2]
            rtol = 1e-7
            for row in range(nrow):
                fr = numpy.extract(numpy.logical_not(flag[:,:,row]), dr[:,:,row])
                fi = numpy.extract(numpy.logical_not(flag[:,:,row]), di[:,:,row])
                if len(fr) <= 1:
                    expec = 0
                else:
                    vr = numpy.var(fr, ddof=1)
                    vi = numpy.var(fi, ddof=1)
                    expec = 2/(vr + vi)
                self.assertTrue(
                    numpy.all(numpy.isclose(wt[:, row], expec, rtol=rtol)),
                    "WEIGHT fail at row" + str(row) + ". got: " + str(wt[:, row]) + " expec " + str(expec)
                )
                self.assertTrue(len(numpy.unique(wtsp[:,:,row])) == 1, "Weight values are not the same")
                self.assertTrue(numpy.all(numpy.isclose(wtsp[:,:,row], expec, rtol)), "Incorrect weights")
                if expec == 0:
                    self.assertTrue(numpy.all(flag[:,:,row]), "Not all flags are true")
                    self.assertTrue(frow[row], "FLAG_ROW is not true")
                else:
                    self.assertTrue(numpy.all(flag[:,:,row] == expflag[:,:,row]), "FLAGs don't match")
                    self.assertTrue(frow[row] == expfrow[row], "FLAG_ROW doesn't match")
            shutil.rmtree(dst) 
           
    def test_timebin(self):
        """ Test time binning"""
        dst = "ngc5921.split.timebin.ms"
        ref = refdir + "ngc5921.timebin300s.ms.ref"
        [refwt, refwtsp, refflag, reffrow, refdata] = _get_dst_cols(ref)
        rtol = 1e-7
        for timebin in ["300s", 10]:
            for i in [0, 1]:
                shutil.copytree(src, dst) 
                myms = mstool()
                if i == 0:
                    myms.open(dst, nomodify=False)
                    myms.statwt2(timebin=timebin)
                    myms.done()
                else:
                    statwt2(dst, timebin=timebin)
                [tstwt, tstwtsp, tstflag, tstfrow, tstdata] = _get_dst_cols(dst)
                self.assertTrue(numpy.all(tstflag == refflag), "FLAGs don't match")
                self.assertTrue(numpy.all(tstfrow == reffrow), "FLAG_ROWs don't match")
                self.assertTrue(numpy.all(numpy.isclose(tstwt, refwt, rtol)), "WEIGHTs don't match")
                self.assertTrue(numpy.all(numpy.isclose(tstwtsp, refwtsp, rtol)), "WEIGHT_SPECTRUMs don't match")
                shutil.rmtree(dst)

    def test_chanbin(self):
        """Test channel binning"""
        mytb = tbtool()
        mytb.open(src)
        expflag = mytb.getcol("FLAG")
        expfrow = mytb.getcol("FLAG_ROW")
        mytb.done()
        dst = "ngc5921.split.ms"
        for i in [0,1]:
            for chanbin in ["195.312kHz", 8]:
                shutil.copytree(src, dst) 
                myms = mstool()
                if i == 0:
                    myms.open(dst, nomodify=False)
                    myms.statwt2(chanbin=chanbin)
                    myms.done()
                else:
                    statwt2(dst, chanbin=chanbin)
                [wt, wtsp, flag, frow, data] = _get_dst_cols(dst)
                dr = numpy.real(data)
                di = numpy.imag(data)
                myshape = wtsp.shape
                ncorr = myshape[0]
                nrow = myshape[2]
                rtol = 1e-7
                for row in range(nrow):
                    for start in (0, 8, 16, 24, 32, 40, 48, 56):
                        end = start + 8
                        fr = numpy.extract(numpy.logical_not(flag[:,start:end,row]), dr[:,start:end,row])
                        fi = numpy.extract(numpy.logical_not(flag[:,start:end,row]), di[:,start:end,row])
                        if len(fr) <= 1:
                            expec = 0
                        else:
                            vr = numpy.var(fr, ddof=1)
                            vi = numpy.var(fi, ddof=1)
                            expec = 2/(vr + vi)
                        self.assertTrue(numpy.all(numpy.isclose(wtsp[:,start:end,row], expec, rtol)), "Incorrect weight spectrum")
                        if expec == 0:
                            self.assertTrue(numpy.all(flag[:,start:end,row]), "Not all flags are true")
                        else:
                            self.assertTrue(numpy.all(flag[:,start:end,row] == expflag[:,start:end,row]), "FLAGs don't match")
                    wts = numpy.extract(numpy.logical_not(flag[:,:,row]), wtsp[:,:,row])
                    if len(wts) > 0:
                        expwt = numpy.median(wts)
                    else:
                        expwt = 0
                    self.assertTrue(
                        numpy.all(numpy.isclose(wt[:, row], expwt, rtol=rtol)),
                        "WEIGHT fail at row" + str(row) + ". got: " + str(wt[:, row]) + " expec " + str(expec)
                    )
                    if expec == 0:
                        self.assertTrue(frow[row], "FLAG_ROW is not true")
                    else:
                        self.assertFalse(frow[row], "FLAG_ROW is not false")
                shutil.rmtree(dst)

def suite():
    return [statwt2_test]

