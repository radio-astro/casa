##########################################################################
# imfit_test.py
#
# Copyright (C) 2008, 2009
# Associated Universities, Inc. Washington DC, USA.
#
# This script is free software; you can redistribute it and/or modify it
# under the terms of the GNU Library General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
# License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with this library; if not, write to the Free Software Foundation,
# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
#
# Correspondence concerning AIPS++ should be adressed as follows:
#        Internet email: aips2-request@nrao.edu.
#        Postal address: AIPS++ Project Office
#                        National Radio Astronomy Observatory
#                        520 Edgemont Road
#                        Charlottesville, VA 22903-2475 USA
#
# <author>
# Dave Mehringer
# </author>
#
# <summary>
# Test suite for the CASA tool method ia.boxcar()
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
# </ul>
# </prerequisite>
#
# <etymology>
# Test for the ia.boxcar() tool method
# </etymology>
#
# <synopsis>
# Test the ia.boxcar() tool method
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_ia_boxcar[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the ia.boxcar() tool method to ensure
# coding changes do not break the associated bits 
# </motivation>
#

###########################################################################
import shutil
import casac
from tasks import *
from taskinit import *
from __main__ import *
import unittest

class ia_boxcar_test(unittest.TestCase):
    
    def setUp(self):
        pass
    
    def tearDown(self):
        self.assertTrue(len(tb.showcache()) == 0)
    
    def test_stretch(self):
        """ ia.boxcar(): Test stretch parameter"""
        yy = iatool()
        mymask = "maskim"
        yy.fromshape(mymask, [200, 200, 1, 1])
        yy.addnoise()
        yy.done()
        shape = [200,200,1,20]
        yy.fromshape("", shape)
        yy.addnoise()
        self.assertRaises(
            Exception,
            yy.boxcar, mask=mymask + ">0", stretch=False
        )
        zz = yy.boxcar(
            mask=mymask + ">0", stretch=True
        )
        self.assertTrue(type(zz) == type(yy))
        yy.done()
        zz.done()
    

    def test_general(self):
        """Test general behavior"""
        myia = iatool()
        length = 13
        imagename = "test_gen.im"
        myia.fromshape(imagename, [1, 1, length])
        bb = myia.getchunk()
        for i in range(length):
            bb[0, 0, i] = i*i + 1
        gg = bb.ravel()
        myia.putchunk(bb)
        for i in range(length):
            reg = rg.box([0, 0, 0], [0, 0, i])
            for width in [3, 4]:
                outfile = "out" + str(i) + str(width) + ".im"
                if (i < width-1):
                    self.assertRaises(
                        Exception, myia.boxcar, region=reg, axis=2, width=width
                    )
                    self.assertFalse(
                        specsmooth(
                            imagename=imagename, outfile=outfile, region=reg, axis=2,
                            function="b", width=width
                        )
                    )
                else:
                    undec = []
                    if width==3:
                        for j in range(i-1):
                            undec.append((gg[j] + gg[j+1] + gg[j+2])/3.0)
                    elif width==4:
                        for j in range(i-2):
                            undec.append((gg[j] + gg[j+1] + gg[j+2] + gg[j+3])/4.0)
                    for drop in [False, True]:
                        if drop:
                            for dmethod in ("c", "m"):
                                outfile = "out" + str(i) + str(width) + str(drop) + dmethod + ".im"
                                runit = dmethod != "m" or i >= 2*width - 1
                                expec = []
                                kk = 0
                                if dmethod == "c":
                                    while kk < len(undec):
                                        expec.append(undec[kk])
                                        kk += width
                                elif dmethod == "m":
                                    if not runit:
                                        self.assertRaises(
                                            Exception, myia.boxcar, region=reg, axis=2,
                                            width=width, dmethod=dmethod
                                        )
                                        self.assertFalse(
                                            specsmooth(
                                                imagename=imagename, outfile=outfile, function="b",
                                                region=reg, axis=2, width=width, dmethod=dmethod
                                            )
                                        )
                                    while kk < len(undec)/width*width:
                                        sum = 0
                                        npoints = 0
                                        for jj in range(width):
                                            if kk + jj == len(undec):
                                                break
                                            else:
                                                sum += undec[kk + jj]
                                                npoints += 1
                                        expec.append(sum/float(npoints))
                                        kk += width
                                if runit:
                                    for mm in [0, 1]:
                                        if mm == 0:
                                            boxcar = myia.boxcar(
                                                region=reg, axis=2, drop=drop, dmethod=dmethod,
                                                width=width
                                            )
                                        elif mm == 1:
                                            specsmooth(
                                                imagename=imagename, outfile=outfile,
                                                region=reg, axis=2, function="b",
                                                dmethod=dmethod, width=width
                                            )
                                            boxcar.open(outfile)
                                        got = boxcar.getchunk().ravel()
                                        self.assertTrue((abs(got/expec - 1) < 1e-6).all())
                                        boxcar.done()
                        else:
                            dmethod = "c"
                            expec = undec
                            for mm in [0, 1]:
                                if mm == 0:
                                    boxcar = myia.boxcar(
                                        region=reg, axis=2, drop=drop, dmethod=dmethod,
                                        width=width
                                    )
                                elif mm == 1:
                                    specsmooth(
                                        imagename=imagename, outfile=outfile,
                                        region=reg, axis=2, function="b",
                                        dmethod="", width=width
                                    )
                                    boxcar.open(outfile)
                                got = boxcar.getchunk().ravel()
                                self.assertTrue((abs(got/expec - 1) < 1e-6).all())
                                boxcar.done()
        myia.done()
    
def suite():
    return [ia_boxcar_test]
