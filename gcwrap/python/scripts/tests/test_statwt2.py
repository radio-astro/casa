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

class statwt2_test(unittest.TestCase):

    def test_algorithm(self):
        """ Test the algorithm"""
        mytb = tbtool()
        mytb.open(src)
        expflag = mytb.getcol("FLAG")
        expfrow = mytb.getcol("FLAG_ROW")
        mytb.done
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
            mytb.open(dst)
            wt = mytb.getcol("WEIGHT")
            wtsp = mytb.getcol("WEIGHT_SPECTRUM")
            flag = mytb.getcol("FLAG")
            frow = mytb.getcol("FLAG_ROW")
            data = mytb.getcol("CORRECTED_DATA")
            mytb.close()
            dr = numpy.real(data)
            di = numpy.imag(data)
            myshape = wtsp.shape
            ncorr = myshape[0]
            nrow = myshape[2]
            rtol = 1e-7
            for row in range(nrow):
                for corr in range(ncorr):
                    fr = numpy.extract(numpy.logical_not(flag[corr,:,row]), dr[corr,:,row])
                    fi = numpy.extract(numpy.logical_not(flag[corr,:,row]), di[corr,:,row])
                    if len(fr) <= 1:
                        expec = 0
                    else:
                        vr = numpy.var(fr, ddof=1)
                        vi = numpy.var(fi, ddof=1)
                        expec = 2/(vr + vi)
                    self.assertTrue(numpy.all(numpy.isclose(wt[corr, row], expec, rtol=rtol)), "WEIGHT fail")
                    self.assertTrue(len(numpy.unique(wtsp[corr,:,row])) == 1, "Weight values are not the same")
                    self.assertTrue(numpy.all(numpy.isclose(wtsp[corr,:,row], expec, rtol)), "Incorrect weights")
                    if expec == 0:
                        self.assertTrue(numpy.all(flag[corr,:,row]), "Not all flags are true")
                        self.assertTrue(frow[row], "FLAG_ROW is not true")
                    else:
                        self.assertTrue(numpy.all(flag[corr,:,row] == expflag[corr,:,row]), "FLAGs don't match")
                        self.assertTrue(frow[row] == expfrow[row], "FLAG_ROW doesn't match")
            shutil.rmtree(dst) 
           
def suite():
    return [statwt2_test]

