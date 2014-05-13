#############################################
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
import numpy

good_image = "collapse_in.fits"
masked_image = "im_w_mask.im"
datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/imcollapse/'


def run_collapse(
    imagename, function, axes, outfile, region, box, chans,
    stokes, mask, overwrite, stretch=False
):
    myia = iatool()
    myia.open(imagename)
    res = myia.collapse(
        function=function, axes=axes, outfile=outfile,
        region=region, box=box, chans=chans, stokes=stokes,
        mask=mask, overwrite=overwrite, stretch=stretch
    )
    myia.close()
    myia.done()
    return res

def run_imcollapse(
    imagename, function, axes, outfile, region, box, chans,
    stokes, mask, overwrite, stretch=False
):
    return imcollapse(
        imagename=imagename, function=function, axes=axes,
        outfile=outfile, region=region, box=box, chans=chans,
        stokes=stokes, mask=mask, overwrite=overwrite,
        stretch=stretch
    )

class imcollapse_test(unittest.TestCase):
    
    def setUp(self):
        shutil.copy(datapath + good_image, good_image)
        self.tabular_spectral_image = datapath + "longZax"

    def tearDown(self):
        os.remove(good_image)
        self.assertTrue(len(tb.showcache()) == 0)

    def checkImage(self, gotImage, expectedName):
        expected = iatool()                                
        expected.open(expectedName)
        got = iatool()
        if type(gotImage) == str:
            got.open(gotImage)
        else:
            got = gotImage
        self.assertTrue(all(got.shape() == expected.shape()))
        diffData = got.getchunk() - expected.getchunk()
        self.assertTrue(abs(diffData).max() == 0)
        gotCsys = got.coordsys()
        expectedCsys = expected.coordsys()
        diffPixels = gotCsys.referencepixel()['numeric'] - expectedCsys.referencepixel()['numeric']
        self.assertTrue(abs(diffPixels).max() == 0)
        fracDiffRef = (
            gotCsys.referencevalue()['numeric'] - expectedCsys.referencevalue()['numeric']
        )/expectedCsys.referencevalue()['numeric'];
        self.assertTrue(abs(fracDiffRef).max() <= 1.5e-6)
        beam = got.restoringbeam()
        self.assertTrue(len(beam) == 3)
        self.assertTrue(abs(beam["major"]["value"] - 1) < 1.5e-6)
        self.assertTrue(abs(beam["minor"]["value"] - 1) < 1.5e-6)
        self.assertTrue(abs(beam["positionangle"]["value"] - 40) < 1.5e-6)
        got.close()
        got.done()
        expected.close()
        expected.done()

    def test_exceptions(self):
        """imcollapse: Test various exception cases"""
        
        bogus = "mybogus.im"
        def testit(
            imagename, function, axes, outfile, region,
            box, chans, stokes, mask, overwrite, wantreturn
        ):
            for i in [0,1]:
                if (i==0 and len(imagename) > 0 and imagename != bogus):
                    self.assertRaises(
                        Exception, run_collapse, imagename,
                        function, axes, outfile, region, box,
                        chans, stokes, mask, overwrite
                    )
                else:
                    self.assertFalse(
                        run_imcollapse(
                            imagename, function, axes,
                            outfile, region, box, chans,
                            stokes, mask, overwrite
                        )
                    )
        # no image name given
        testit("", "mean", 0, "", "", "", "", "", "", False, True)
        # bad image name given
        testit(bogus, "mean", 0, "", "", "", "", "", "", False, True)
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
        # no axes
        testit(good_image, "mean", "", "", "", "", "", "", "", False, True)
        # bogus axes
        testit(good_image, "mean", 10, "", "", "", "", "", "", False, True)

    def test_1(self):
        """imcollapse: average full image collapse along axis 0"""
        expected = "collapse_avg_0.fits"
        shutil.copy(datapath + expected, expected)
        for i in [0, 1]:
            for axis in (0 ,"r", "right"):
                outname = "test_1_" + str(i) + "_" + str(axis) + ".im"
                if i == 0:
                    mytool = run_collapse(
                        good_image, "mean", axis, outname, "", "",
                        "", "", "", False
                    )
                    self.assertTrue(type(mytool) == type(ia))
                    self.checkImage(mytool, expected)
                    self.checkImage(outname, expected)
                else:
                    for wantreturn in [True, False]:
                        outname = outname + str(wantreturn)
                        res = run_imcollapse(
                            good_image, "mean", 0, outname, "", "",
                            "", "", "", False
                        )
                        self.assertTrue(res)
                        self.checkImage(outname, expected)
                shutil.rmtree(outname)

    def test_2(self):
        """imcollapse: average full image collapse along axis 2"""
        expected = "collapse_avg_2.fits"
        shutil.copy(datapath + expected, expected)
        for i in [0, 1]:
            for axis in (2, "f", "freq"):
                outname = "test_2_" + str(i) + str(axis) + ".im"
                if i == 0:
                    mytool = run_collapse(
                        good_image, "mean", axis, outname, "", "",
                        "", "", "", False
                    )
                    self.assertTrue(type(mytool) == type(ia))
                    self.checkImage(mytool, expected)
                    self.checkImage(outname, expected)
                else:
                    outname = outname + "imcollapse"
                    res = run_imcollapse(
                    good_image, "mean", 2, outname, "", "",
                        "", "", "", False
                    )
                    self.assertTrue(res)
                    self.checkImage(outname, expected)
                shutil.rmtree(outname)

    def test_3(self):
        """imcollapse: average full image collapse along axis 2 and check output overwritability"""
        expected = "collapse_sum_1.fits"
        shutil.copy(datapath + expected, expected)
        box = "1,1,2,2"
        chans = "1~2"
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
               outname = outname + "imcollapse"
               # check that can overwrite previous output. Then check output image
               res = run_imcollapse(
                    good_image, "sum", 1, outname, "", box,
                    chans, stokes, "", False
               )
               res = run_imcollapse(
                    good_image, "sum", 1, outname, "", box,
                    chans, stokes, "", True
               )
               self.assertTrue(res)
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
                self.assertFalse(
                    run_imcollapse(
                        good_image, "mean", 2, "", "", "",
                        "", "", "", False
                    )
                )
        
    def test_6(self):
        """imcollapse: memory only images can be collapsed"""
        mytool = run_collapse(
            good_image, "mean", 2, "", "", "",
            "", "", "", False
        )
        mytool2 = mytool.collapse("mean", 3)
        expected = [3, 3, 1, 1]
        self.assertTrue(all(mytool2.shape() == expected))
 
    def test_7(self):
        """imcollapse: verify collapsing along multiple axes works"""
        expected = "collapse_avg_0_1.fits"
        shutil.copy(datapath + expected, expected)
        for i in [0, 1]:
            for axes in ([0, 1], ["r", "d"], ["right", "dec"]):
                if i == 0:
                    mytool = run_collapse(
                        good_image, "mean", axes, "", "", "",
                        "", "", "", False
                    )
                    self.assertTrue(type(mytool) == type(ia))
                    self.checkImage(mytool, expected)
                else:
                    outfile = "test_7.out"
                    res = run_imcollapse(
                        good_image, "mean", [0, 1], outfile, "", "",
                        "", "", "", overwrite=True
                    )
                    self.assertTrue(res)
                    self.checkImage(outfile, expected)

    def test_8(self):
        """imcollapse: test both OTF and permanent masking works"""
        xx = iatool()
        good_image_im = "collapse_in.im"
        xx.fromfits(good_image_im, good_image)
        xx.calcmask(good_image_im + "<78")
        xx.close()
        xx.done()
        mytool = F
        axes = 3
        for i in [0,1]:
            for j in [0, 1, 2]:
                mask = good_image_im + ">7"
                if j == 0:
                    xx.open(good_image_im)
                    xx.maskhandler("set", "")
                    xx.close()
                    xx.done()
                if j == 1:
                    mask = ""
                    xx.open(good_image_im)
                    xx.maskhandler("set", "mask0")
                    xx.close()
                    xx.done()
                for func in ["mean", "median"]:
                    for outfile in ["", "test_8_"+str(i) + str(j) + func]:
                        if i == 0:
                            mytool = run_collapse(
                                good_image_im, func, axes, outfile, "", "",
                                "", "", mask, False
                            )
                            self.assertTrue(type(mytool) == type(ia))
                        else:
                            outfile = "test8.im"
                            res = run_imcollapse(
                                good_image_im, func, axes, outfile, "", "",
                                "", "", mask, True
                            )
                            self.assertTrue(type(res))
                            mytool.open(outfile)
                        npts = mytool.statistics()["npts"]
                        mytool.close()
                        mytool.done()
                        if (j == 0):
                            self.assertTrue(npts == 25)
                        elif (j == 1):
                            self.assertTrue(npts == 26)
                        else:
                            self.assertTrue(npts == 24)

    def test_CAS_3418(self):
        """imcollapse: Test separate code for median due to performance issues"""
        for i in range(0,4):
            xx = iatool()
            xx.open(good_image)
            exp = xx.statistics(robust=T, axes=i)["median"]
            xx.done()
            mytool = run_collapse(
                good_image, "median", i, "", "", "",
                "", "", "", False
            )
            zz = mytool.subimage("", dropdeg=T)
            got = zz.getchunk()
            self.assertTrue((got == exp).all())
            outfile = "test_CAS_3418.im"
            res = run_imcollapse(
                good_image, "median", i, outfile, "", "",
                "", "", "", overwrite=True
            )
            mytool.open(outfile)
            zz = mytool.subimage("", dropdeg=T)
            got = zz.getchunk()
            self.assertTrue((got == exp).all())
            mytool.done()
            zz.done()
            
    def test_region(self):
        """ imcollapse: Test region"""
        myia = iatool()
        myia.fromshape("", [10, 10, 10])
        bb = myia.getchunk()
        for i in range(10):
            bb[i,5,:] = i
            bb[i,0:5,:] = i+1
            bb[i,6:10,:] = i+2
        myia.putchunk(bb)
        res = myia.collapse("mean", 1, box="0,4,9,6")
        expec = myia.makearray(0, [10, 1, 10])
        for i in range(10):
            expec[i, 0, :] = i+1
        got = res.getchunk()
        self.assertTrue((expec == got).all())
        
    def test_stretch(self):
        """ imcollapse: Test stretch parameter"""
        yy = iatool()
        yy.open(good_image)
        mycs = yy.coordsys().torecord()
        yy.done()
        maskim = "ymask"
        yy.fromshape(maskim,[3,3,1,1])
        yy.addnoise()
        yy.setcoordsys(mycs)
        yy.done()
        for i in [0,1]:
            if i == 1:
                yy = run_collapse(
                    good_image, "mean", 0, "", "", "", "",
                    "", maskim + ">0", False, stretch=True
                )
                self.assertTrue(type(yy) == type(ia))
                yy.done()
            else:
                outfile = "test_stretch.im"
                res = run_imcollapse(
                    good_image, "mean", 0, outfile, "", "", "",
                    "", maskim + ">0", False, stretch=True
                )
                self.assertTrue(res)
            
    def test_CAS3737(self):
        """ imcollapse: test tabular spectral axis has correct collapsed reference value """
        image = self.tabular_spectral_image
        for chans in ["2445~2555", "range=[2445pix,2555pix]"]:
            mytool = run_collapse(
                image, "mean", 2, "", "", "",
                chans, "", "", False
            )
            expected = 98318505973583.641
            got = mytool.toworld([0,0,0])["numeric"][2]
            mytool.done()
            frac = got/expected - 1
            self.assertTrue(frac < 1e-6 and frac > -1e-6)
            outfile = "test_CAS3737"
            res = run_imcollapse(
                image, "mean", 2, outfile, "", "",
                chans, "", "", False
            )
            mytool.open(outfile)
            got = mytool.toworld([0,0,0])["numeric"][2]
            mytool.done()
            frac = got/expected - 1
            self.assertTrue(frac < 1e-6 and frac > -1e-6)
        
    def test_beams(self):
        """test per plane beams"""
        myia = iatool()
        myia.fromshape("", [10, 10, 10, 4])
        myia.setrestoringbeam(
            major="4arcsec", minor="3arcsec",
            pa="20deg", channel=1, polarization=1
        )
        for i in range (myia.shape()[2]):
            for j in range(myia.shape()[3]):
                major = qa.quantity(4 + i + j, "arcsec")
                minor = qa.quantity(2 + i + 0.5*j, "arcsec")
                pa = qa.quantity(10*i + j, "deg")
                myia.setrestoringbeam(
                    major=major, minor=minor, pa=pa,
                    channel=i, polarization=j
                )
        reg = rg.box(blc=[1,1,1,1], trc=[2,2,2,2])
        collapsed = myia.collapse(function="mean", axes=2, outfile="", region=reg)
        beam = collapsed.restoringbeam()
        self.assertTrue(len(beam) == 3)
        self.assertTrue(beam["major"] == qa.quantity(6, "arcsec"))
        self.assertTrue(beam["minor"] == qa.quantity(3.5, "arcsec"))
        self.assertTrue(beam["positionangle"] == qa.quantity(11, "deg"))
        myia.done()
        collapsed.done()

    def test_complex(self):
        """Test support for complex valued images"""
        myia = iatool()
        
        myia.fromshape("", [2, 2, 2], type='c')
        bb = myia.getchunk()
        counter = 0
        for i in [0, 1]:
            for j in [0, 1]:
                for k in [0, 1]:
                    bb[i, j, k] = counter*(1-1j)
                    counter += 1
        myia.putchunk(bb)
        col = myia.collapse("min", [2])
        got = col.subimage(dropdeg=T).getchunk()
        exp = numpy.min(bb, 2)
        self.assertTrue((got == exp).all())
        
        col = myia.collapse("mean", [2])
        got = col.subimage(dropdeg=T).getchunk()
        exp = numpy.average(bb, 2)
        self.assertTrue((got == exp).all())

        myia.done()
        col.done()
        
    def test_flux(self):
        """Test flux function"""
        myia = iatool()
        imagename = "flux_test.im"
        myia.fromshape(imagename, [10, 10, 10])
        bb = myia.getchunk()
        bb[:] = 1
        bb[0,0,0] = 0
        myia.putchunk(bb)
        self.assertRaises(Exception, myia.collapse, axes=[0,1], function="flux")
        myia.setrestoringbeam(major="3arcmin", minor="3arcmin", pa="0deg")
        myia.setbrightnessunit("Jy/beam")
        col = myia.collapse(axes=[0,1], function="flux", mask=imagename + "> 0")
        self.assertTrue((col.shape() == [1, 1, 10]).all())
        bb = col.getchunk()
        for i in range(10):
            if i == 0:
                self.assertTrue(abs(bb[0,0,i] - 9.707966) < 1e-5)
            else:
                self.assertTrue(abs(bb[0,0,i] - 9.806027) < 1e-5)
        col.done()
        myia.done()
        
    def test_sqrtsum(self):
        """Test sqrtsum function"""
        myia = iatool()
        myia.fromshape("",[2,2,2])
        bb = myia.getchunk()
        bb[:, :, 0] = 1
        bb[:, :, 1] = 2
        myia.putchunk(bb)
        zz = myia.collapse(axes=[0,1], function="sqrtsum")
        bb = zz.getchunk()
        self.assertTrue(bb[0, 0, 0] == 2)
        self.assertTrue(abs(bb[0, 0, 1] - 2*sqrt(2)) < 1e-6)
        bb = myia.getchunk()
        bb[:, :, 0] = -1
        myia.putchunk(bb)
        zz = myia.collapse(axes=[0,1], function="sqrtsum")
        bb = zz.getchunk()
        self.assertTrue(bb[0, 0, 0] == 0)
        self.assertTrue(abs(bb[0, 0, 1] - 2*sqrt(2)) < 1e-6)
        
    def test_sqrtsum_npix(self):
        """Test sqrtsum function"""
        myia = iatool()
        myia.fromshape("",[2,2,2])
        bb = myia.getchunk()
        bb[:, :, 0] = 1
        bb[:, :, 1] = 2
        myia.putchunk(bb)
        zz = myia.collapse(axes=[0,1], function="sqrtsum_npix")
        bb = zz.getchunk()
        self.assertTrue(bb[0, 0, 0] == 0.5)
        self.assertTrue(abs(bb[0, 0, 1] - 0.5*sqrt(2)) < 1e-6)
        bb = myia.getchunk()
        bb[:, :, 0] = -1
        myia.putchunk(bb)
        zz = myia.collapse(axes=[0,1], function="sqrtsum_npix")
        bb = zz.getchunk()
        self.assertTrue(bb[0, 0, 0] == 0)
        self.assertTrue(abs(bb[0, 0, 1] - 0.5*sqrt(2)) < 1e-6)
        
    def test_sqrtsum_npix_beam(self):
        """Test sqrtsum function"""
        myia = iatool()
        myia.fromshape("",[2,2,2])
        myia.setrestoringbeam(major="3arcmin", minor="3arcmin", pa="0deg")
        bb = myia.getchunk()
        bb[:, :, 0] = 1
        bb[:, :, 1] = 2
        myia.putchunk(bb)
        zz = myia.collapse(axes=[0,1], function="sqrtsum_npix_beam")
        bb = zz.getchunk()
        self.assertTrue(abs(bb[0, 0, 0] - 0.19612053) < 1e-6)
        self.assertTrue(abs(bb[0, 0, 1] - 0.27735632) < 1e-6)
        bb = myia.getchunk()
        bb[:, :, 0] = -1
        myia.putchunk(bb)
        zz = myia.collapse(axes=[0,1], function="sqrtsum_npix_beam")
        bb = zz.getchunk()
        self.assertTrue(bb[0, 0, 0] == 0)
        self.assertTrue(abs(bb[0, 0, 1] - 0.27735632) < 1e-6)
        
def suite():
    return [imcollapse_test]
