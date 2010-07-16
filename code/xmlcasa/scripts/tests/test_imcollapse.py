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
# Test suite for the CASA task imcollapse
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
#   <li> <linkto class="task_imcollapse.py:description">imcollapse</linkto> 
# </ul>
# </prerequisite>
#
# <etymology>
# Test for the imcollapse task
# </etymology>
#
# <synopsis>
# Test the imcollapse task and the ia.collapse() method upon which it is built.
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_imcollapse[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the imcollapse task to ensure
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

good_image = "collapse_in.fits"
datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/imcollapse/'


def run_collapse(
    imagename, function, axis, outfile, region, box, chans,
    stokes, mask, overwrite
):
    ia.open(imagename)
    res = ia.collapse(
        function=function, axis=axis, outfile=outfile,
        region=region, box=box, chans=chans, stokes=stokes,
        mask=mask, overwrite=overwrite
    )
    return res

def run_imcollapse(
    imagename, function, axis, outfile, region, box, chans,
    stokes, mask, overwrite, wantreturn
):
    return imcollapse(
        imagename=imagename, function=function, axis=axis,
        outfile=outfile, region=region, box=box, chans=chans,
        stokes=stokes, mask=mask, overwrite=overwrite,
        wantreturn=wantreturn
    )

class imcollapse_test(unittest.TestCase):
    
    def setUp(self):
        shutil.copy(datapath + good_image, good_image)
    
    def tearDown(self):
        os.remove(good_image)

    def checkImage(self, gotImage, expectedName):
        expected = iatool.create()                                
        expected.open(expectedName)
        got = iatool.create()
        if type(gotImage) == str:
            got.open(gotImage)
        else:
            got = gotImage
        self.assertTrue(got.shape() == expected.shape())
        diffData = got.getchunk() - expected.getchunk()
        self.assertTrue(abs(diffData).max() == 0)
        gotCsys = got.coordsys()
        expectedCsys = expected.coordsys()
        diffPixels = gotCsys.referencepixel()['numeric'] - expectedCsys.referencepixel()['numeric']
        self.assertTrue(abs(diffPixels).max() == 0)
        fracDiffRef = (
            gotCsys.referencevalue()['numeric'] - expectedCsys.referencevalue()['numeric']
        )/expectedCsys.referencevalue()['numeric'];
        self.assertTrue(abs(fracDiffRef).max() <= 1e-13)
        got.done()
        expected.done()

    def test_exceptions(self):
        """imcollapse: Test various exception cases"""
        
        def testit(
            imagename, function, axis, outfile, region,
            box, chans, stokes, mask, overwrite, wantreturn
        ):
            for i in [0,1]:
                if (i==0):
                    self.assertRaises(
                        Exception, run_collapse, imagename,
                        function, axis, outfile, region, box,
                        chans, stokes, mask, overwrite
                    )
                else:
                    self.assertFalse(
                        run_imcollapse(
                            imagename, function, axis,
                            outfile, region, box, chans,
                            stokes, mask, overwrite,
                            wantreturn
                        )
                    )
        # no image name given
        testit("", "mean", 0, "", "", "", "", "", "", False, True)
        # bad image name given
        testit("mybogus.im", "mean", 0, "", "", "", "", "", "", False, True)
        # no function given
        testit(good_image, "", 0, "", "", "", "", "", "", False, True)
        # bogus function given
        testit(good_image, "bogus function", 0, "", "", "", "", "", "", False, True)
        # bogus region given
        testit(good_image, "mean", 0, "", "bogus_region", "", "", "", "", False, True)
        #bogus box
        testit(good_image, "mean", 0, "", "", "abc", "", "", "", False, True)
        # another bogus box
        testit(good_image, "mean", 0, "", "", "0,0,1000,1000", "", "", "", False, True)
        # no axis
        testit(good_image, "mean", "", "", "", "", "", "", "", False, True)
        # bogus axis
        testit(good_image, "mean", 10, "", "", "", "", "", "", False, True)

    def test_1(self):
        """imcollapse: average full image collapse along axis 0"""
        expected = "collapse_avg_0.fits"
        shutil.copy(datapath + expected, expected)
        for i in [0, 1]:
            outname = "test_1_" + str(i) + ".im"
            if i == 0:
                mytool = run_collapse(
                    good_image, "mean", 0, outname, "", "",
                    "", "", "", False
                )
                self.assertTrue(type(mytool) == type(ia))
                self.checkImage(mytool, expected)
                self.checkImage(outname, expected)
            else:
                for wantreturn in [True, False]:
                    outname = outname + str(wantreturn)
                    mytool = run_imcollapse(
                        good_image, "mean", 0, outname, "", "",
                        "", "", "", False, wantreturn
                    )
                    if (wantreturn):
                        self.assertTrue(type(mytool) == type(ia))
                        self.checkImage(mytool, expected)
                    else:
                        self.assertTrue(mytool == None)
                    self.checkImage(outname, expected)
            shutil.rmtree(outname)

    def test_2(self):
        """imcollapse: average full image collapse along axis 2"""
        expected = "collapse_avg_2.fits"
        shutil.copy(datapath + expected, expected)
        for i in [0, 1]:
            outname = "test_2_" + str(i) + ".im"
            if i == 0:
                mytool = run_collapse(
                    good_image, "mean", 2, outname, "", "",
                    "", "", "", False
                )
                self.assertTrue(type(mytool) == type(ia))
                self.checkImage(mytool, expected)
                self.checkImage(outname, expected)
            else:
                for wantreturn in [True, False]:
                    outname = outname + str(wantreturn)
                    mytool = run_imcollapse(
                        good_image, "mean", 2, outname, "", "",
                        "", "", "", False, wantreturn
                    )
                    if (wantreturn):
                        self.assertTrue(type(mytool) == type(ia))
                        self.checkImage(mytool, expected)
                    else:
                        self.assertTrue(mytool == None)
                    self.checkImage(outname, expected)
            shutil.rmtree(outname)

    def test_3(self):
        """imcollapse: average full image collapse along axis 2 and check output overwritability"""
        expected = "collapse_sum_1.fits"
        shutil.copy(datapath + expected, expected)
        box = "1,1,2,2"
        chans = "1-2"
        stokes = "qu"
        for i in [0, 1]:
            outname = "test_3_" + str(i) + ".im"
            if i == 0:
                mytool = run_collapse(
                    good_image, "sum", 1, outname, "", box,
                    chans, stokes, "", False
                )
                # check that can overwrite previous output. Then check output image
                mytool = run_collapse(
                    good_image, "sum", 1, outname, "", box,
                    chans, stokes, "", True
                )
                self.assertTrue(type(mytool) == type(ia))
                self.checkImage(mytool, expected)
                self.checkImage(outname, expected)
            else:
                for wantreturn in [True, False]:
                    outname = outname + str(wantreturn)
                    # check that can overwrite previous output. Then check output image
                    mytool = run_imcollapse(
                        good_image, "sum", 1, outname, "", box,
                        chans, stokes, "", False, wantreturn
                    )
                    mytool = run_imcollapse(
                        good_image, "sum", 1, outname, "", box,
                        chans, stokes, "", True, wantreturn
                    )
                    if (wantreturn):
                        self.assertTrue(type(mytool) == type(ia))
                        self.checkImage(mytool, expected)
                    else:
                        self.assertTrue(mytool == None)
                    self.checkImage(outname, expected)
            shutil.rmtree(outname)

    def test_4(self):
        """imcollapse: not specifying an output image is ok"""
        expected = "collapse_avg_2.fits"
        shutil.copy(datapath + expected, expected)
        for i in [0, 1]:
            if i == 0:
                mytool = run_collapse(
                    good_image, "mean", 2, "", "", "",
                    "", "", "", False
                )
                self.assertTrue(type(mytool) == type(ia))
                self.checkImage(mytool, expected)
            else:
                mytool = run_imcollapse(
                    good_image, "mean", 2, "", "", "",
                    "", "", "", False, True
                )
                self.assertTrue(type(mytool) == type(ia))
                self.checkImage(mytool, expected)

    def test_5(self):
        """imcollapse: not wanting a tool returned results in None being returned"""
        expected = "collapse_avg_2.fits"
        shutil.copy(datapath + expected, expected)
        mytool = run_imcollapse(
            good_image, "mean", 2, "", "", "",
            "", "", "", False, False
        )
        self.assertTrue(mytool == None)

def suite():
    return [imcollapse_test]