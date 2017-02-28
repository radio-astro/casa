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

_rg = rgtool( )

#run using
# `which casa` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py --mem test_makestatimage
#

'''
Unit tests for task ia.makestatimage().
'''

datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/ia_makestatimage/'

input0 = datapath + "100x100x2.im"
ref0 = datapath + "ref0.im"
ref1 = datapath + "ref1.im"
ref2 = datapath + "ref2.im"
ref3 = datapath + "ref3.im"
ref4 = datapath + "ref4.im"
ref5 = datapath + "ref5.im"
ref6 = datapath + "ref6.im"
ref7 = datapath + "ref7.im"

class ia_makestatimage_test(unittest.TestCase):

    def _compare(self, resold, resnew, helpstr):
        mytype = type(resold)
        self.assertTrue(mytype == type(resnew), helpstr + ": types differ")
        if mytype == dict:
            for k in resold.keys():
                self._compare(resold[k], resnew[k], helpstr)
        elif mytype == numpy.ndarray:
            oldarray = resold.ravel()
            newarray = resnew.ravel()
            self.assertTrue(
                len(oldarray) == len(newarray),
                helpstr + ": array lengths not equal"
            )
            for i in range(len(oldarray)):
                self._compare(oldarray[i], newarray[i], helpstr)
        elif mytype == str:
            self.assertTrue(
                resold == resnew,
                helpstr + ": string inequality, old = " + resold + ", new = " + resnew
            )
        elif isinstance(resold, numbers.Integral) or mytype == numpy.int32:
            self.assertTrue(
                resold == resnew,
                helpstr + ": integral inequality, old = " + str(resold) + ", new = " + str(resnew)
            )
        elif isinstance(resold, numbers.Real):
            self.assertTrue(
                resold == resnew
                or abs(resnew/resold - 1) < 1e-6,
                helpstr + "float inequality: old = " + str(resold)
                + ", new = " + str(resnew)
            )
        else:
            self.assertTrue(False, "Unhandled type " + str(mytype))

    def setUp(self):
        self.res = None
        self._myia = iatool()
    
    def tearDown(self):
        self._myia.done()
        self.assertTrue(len(tb.showcache()) == 0)

    def test001(self):
        """Every pixel is a grid point"""
        self._myia.open(input0)
        zz = self._myia.makestatimage(
            "", grid=[1,1], xlength="4pix", ylength="4pix", stattype="npts",
            interp="cub",anchor=[0,0], statalg="cl"
        )
        self._myia.open(ref0)
        self._compare(self._myia.getchunk(), zz.getchunk(), "test001 compare")
        zz = self._myia.makestatimage(
            "", grid=[1,1], xlength="4pix", ylength="4pix", stattype="sigma",
            interp="cub",anchor=[0,0], statalg="cl"
        )
        self._myia.open(ref1)
        self._myia.done()
        zz.done()

        # now the task
        outname = "out0.im"
        imstatimage(
            input0, outname, grid=[1,1], xlength="4pix", ylength="4pix",
            stattype="npts", interp="cub",anchor=[0,0], statalg="cl"
        )
        self._myia.open(ref0)
        expec = self._myia.getchunk()
        self._myia.open(outname)
        got = self._myia.getchunk()
        self._myia.done()
        self._compare(got, expec, "imstatimage test 1")
        
    def test002(self):
        """Every pixel is a grid point with an offset, so should be the same result
        as test001"""
        anchor = [1,1]
        self._myia.open(input0)
        zz = self._myia.makestatimage(
            "", grid=[1,1], xlength="4pix", ylength="4pix", stattype="npts",
            interp="cub",anchor=anchor, statalg="cl"
        )
        self._myia.open(ref0)
        self._compare(self._myia.getchunk(), zz.getchunk(), "test001 compare")
        zz = self._myia.makestatimage(
            "", grid=[1,1], xlength="4pix", ylength="4pix", stattype="sigma",
            interp="cub",anchor=anchor, statalg="cl"
        )
        self._myia.open(ref1)
        self._myia.done()
        zz.done()
        
    def test003(self):
        """Every third pixel is a grid point"""
        grid = [3,3]
        for anchor in [[0,0], [12, 60]]:
            self._myia.open(input0)
            zz = self._myia.makestatimage(
                "", grid=grid, xlength="4pix", ylength="4pix", stattype="npts",
                interp="cub",anchor=anchor, statalg="cl"
            )
            self._myia.open(ref2)
            self._compare(self._myia.getchunk(), zz.getchunk(), "test001 compare")
            zz = self._myia.makestatimage(
                "", grid=grid, xlength="4pix", ylength="4pix", stattype="sigma",
                interp="cub",anchor=anchor, statalg="cl"
            )
            self._myia.open(ref3)
        self._myia.done()
        zz.done()
        
    def test005(self):
        """Every third pixel is a grid point, using anchor [2,2]"""
        anchor = [2,2]
        grid = [3,3]
        self._myia.open(input0)
        for anchor in [[2,2], [17,11]]:
            zz = self._myia.makestatimage(
                "", grid=grid, xlength="4pix", ylength="4pix", stattype="npts",
                interp="cub", anchor=anchor, statalg="cl"
            )
            self._myia.open(ref4)
            self._compare(self._myia.getchunk(), zz.getchunk(), "test005 compare")
            zz = self._myia.makestatimage(
                "", grid=grid, xlength="4pix", ylength="4pix", stattype="sigma",
                interp="cub", anchor=anchor, statalg="cl"
            )
            self._myia.open(ref5)
        self._myia.done()
        zz.done()

    def test006(self):
        """Test that regions work as expected"""
        self._myia.open(input0)
        myrg = rgtool()
        reg = myrg.box([5,5,0],[85,85,1])
        anchor = [1,1]
        grid = [13,13]
        length = "10.001pix"
        zz = self._myia.makestatimage(
            grid=grid, xlength=length, ylength=length, stattype="sigma",
            interp="cub", anchor=anchor, statalg="cl"
        )
        yy = self._myia.makestatimage(
            grid=grid, xlength=length, ylength=length, stattype="sigma",
            interp="cub", anchor=anchor, statalg="cl", region=reg
        )
        self._myia.done()
        sub0 = zz.subimage(region=myrg.box([27,27,0],[66,66,0]))
        zz.done()
        sub1 = yy.subimage(region=myrg.box([22,22,0],[61,61,0]))
        yy.done()
        self._compare(sub0.getchunk(), sub1.getchunk(), "test006 compare")
        sub0.done()
        sub1.done()

def suite():
    return [ia_makestatimage_test]
