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
# Test suite for the CASA task specfit and tool method ia.fitprofile
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
#   <li> <linkto class="task_specfit.py:description">imcollapse</linkto> 
# </ul>
# </prerequisite>
#
# <etymology>
# Test for the specfit task and ia.fitprofile() tool method.
# </etymology>
#
# <synopsis>
# Test the specfit task and the ia.fitprofile() method upon which it is built.
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_specfit[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the specfit task to ensure
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

twogauss = "specfit_multipix_2gauss.fits"
polyim = "specfit_multipix_poly_2gauss.fits"
gauss_triplet = "gauss_triplet.fits"
two_lorentzians = "two_lorentzians.fits"
invalid_fits = "invalid_fits.im"
solims = [
    "amp", "ampErr", "center", "centerErr",
    "fwhm", "fwhmErr", "integral", "integralErr"
]
birdie = "birdie.im"

nanvalue = 4.53345345

datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/specfit/'

def run_fitprofile (
    imagename, box, region, chans, stokes,
    axis, mask, ngauss, poly, multifit, model="",
    residual="", amp="", amperr="", center="", centererr="",
    fwhm="", fwhmerr="", integral="", integralerr="",
    estimates="", logresults=True, pampest="", pcenterest="", pfwhmest="",
    pfix="", gmncomps=0, gmampcon="", gmcentercon="",
    gmfwhmcon="", gmampest=[0], gmcenterest=[0],
    gmfwhmest=[0], gmfix="", logfile="", pfunc="",
    goodamprange=[0.0], goodcenterrange=[0.0], goodfwhmrange=[0.0],
    sigma="", outsigma=""
):
    myia = iatool()
    myia.open(imagename)
    if (not myia.isopen()):
        myia.done()
        raise Exception
    res = myia.fitprofile(
        box=box, region=region, chans=chans,
        stokes=stokes, axis=axis, mask=mask,
        ngauss=ngauss, poly=poly, estimates=estimates,
        multifit=multifit,
        model=model, residual=residual, amp=amp,
        amperr=amperr, center=center, centererr=centererr,
        fwhm=fwhm, fwhmerr=fwhmerr, integral=integral,
        integralerr=integralerr, logresults=logresults, pampest=pampest,
        pcenterest=pcenterest, pfwhmest=pfwhmest, pfix=pfix,
        gmncomps=gmncomps, gmampcon=gmampcon,
        gmcentercon=gmcentercon, gmfwhmcon=gmfwhmcon,
        gmampest=gmampest, gmcenterest=gmcenterest,
        gmfwhmest=gmfwhmest, gmfix=gmfix, logfile=logfile,
        pfunc=pfunc, goodamprange=goodamprange,
        goodcenterrange=goodcenterrange,
        goodfwhmrange=goodfwhmrange, sigma=sigma, outsigma=outsigma    )
    myia.close()
    myia.done()
    return res

def run_specfit(
    imagename, box, region, chans, stokes,
    axis, mask, ngauss, poly, multifit, model="",
    residual="", amp="", amperr="", center="", centererr="",
    fwhm="", fwhmerr="", integral="", integralerr="",
    wantreturn=True, estimates="", logresults=None,
    pampest="", pcenterest="", pfwhmest="", pfix="",
    gmncomps=0, gmampcon="", gmcentercon="",
    gmfwhmcon="", gmampest=[0], gmcenterest=[0],
    gmfwhmest=[0], gmfix="", logfile="", pfunc="",
    goodamprange=[0.0], goodcenterrange=[0.0], goodfwhmrange=[0.0],
    sigma="", outsigma=""
):
    return specfit(
        imagename=imagename, box=box, region=region,
        chans=chans, stokes=stokes, axis=axis, mask=mask,
        ngauss=ngauss, poly=poly, estimates=estimates,
        multifit=multifit,
        model=model, residual=residual, amp=amp,
        amperr=amperr, center=center, centererr=centererr,
        fwhm=fwhm, fwhmerr=fwhmerr, integral=integral,
        integralerr=integralerr,
        wantreturn=wantreturn, logresults=logresults, pampest=pampest,
        pcenterest=pcenterest, pfwhmest=pfwhmest, pfix=pfix,
        gmncomps=gmncomps, gmampcon=gmampcon,
        gmcentercon=gmcentercon, gmfwhmcon=gmfwhmcon,
        gmampest=gmampest, gmcenterest=gmcenterest,
        gmfwhmest=gmfwhmest, gmfix=gmfix, logfile=logfile,
        pfunc=pfunc,
        goodamprange=goodamprange,
        goodcenterrange=goodcenterrange,
        goodfwhmrange=goodfwhmrange, sigma=sigma, outsigma=outsigma
    )

class specfit_test(unittest.TestCase):
    
    def setUp(self):
        shutil.copy(datapath + twogauss, twogauss)
        shutil.copy(datapath + polyim, polyim)

    def tearDown(self):
        os.remove(twogauss)
        os.remove(polyim)

    def checkImage(self, gotImage, expectedName):
        expected = iatool()                                
        expected.open(expectedName)
        got = iatool()
        if type(gotImage) == str:
            got.open(gotImage)
        else:
            got = gotImage
        self.assertTrue((got.shape() == expected.shape()).all())
        gotchunk = got.getchunk()
        expchunk = expected.getchunk()
        if (numpy.isnan(gotchunk).any()):
            gotchunk = gotchunk.ravel()
            for i in range(len(gotchunk)):
                if isnan(gotchunk[i]):
                    gotchunk[i] = nanvalue
        if (numpy.isnan(expchunk).any()):
            expchunk = expchunk.ravel()
            for i in range(len(expchunk)):
                if isnan(expchunk[i]):
                    expchunk[i] = nanvalue
        diffData = gotchunk - expchunk
        self.assertTrue(abs(diffData).max() < 2e-11)
        self.assertTrue(
            (
                got.getchunk(getmask=T) == expected.getchunk(getmask=T)
            ).all()
        )
        gotCsys = got.coordsys()
        expectedCsys = expected.coordsys()
        diffPixels = gotCsys.referencepixel()['numeric'] - expectedCsys.referencepixel()['numeric']
        self.assertTrue(abs(diffPixels).max() == 0)
        
        diffRef = gotCsys.referencevalue()['numeric'] - expectedCsys.referencevalue()['numeric']
        # fracDiffRef = (diffRef)/expectedCsys.referencevalue()['numeric'];
        self.assertTrue(abs(diffRef).max() == 0)
        got.close()
        got.done()
        expected.close()
        expected.done()

    def test_exceptions(self):
        """specfit: Test various exception cases"""
        
        def testit(
            imagename, box, region, chans, stokes,
            axis, mask, ngauss, poly, multifit, model,
            residual
        ):
            for i in [0,1]:
                if (i==0):
                    self.assertRaises(
                        Exception, run_fitprofile, imagename,
                        box, region, chans, stokes, axis, mask,
                        ngauss, poly, multifit, model, residual
                    )
                else:
                    self.assertFalse(
                        run_specfit(
                            imagename, box, region, chans,
                            stokes, axis, mask, ngauss, poly,
                            multifit, model, residual
                        )
                    )
        # Exception if no image name given",
        testit(
            "", "", "", "", "", 2, "", False, 1, -1, "", ""
        )
        # Exception if bogus image name given
        testit(
            "my bad", "", "", "", "", 2, "", 1, -1, False, "", ""
        )
        # Exception if given axis is out of range
        testit(
            twogauss, "", "", "", "", 5, "", 1, -1, False, "", ""
        )
        # Exception if bogus box string given #1
        testit(
            twogauss, "abc", "", "", "", 2, "", 1, -1, False, "", ""
        )
        # Exception if bogus box string given #2
        testit(
            twogauss, "0,0,1000,1000", "", "", "", 2, "", 1, -1, False, "", ""
        )
        # Exception if bogus chans string given #1
        testit(
            twogauss, "", "", "abc", "", 2, "", 1, -1, False, "", ""
        )
        # Exception if bogus chans string given #2
        testit(
            twogauss, "", "", "0-200", "", 2, "", 1, -1, False, "", ""
        )        
        # Exception if bogus stokes string given #1   
        testit(
            twogauss, "", "", "", "abc", 2, "", 1, -1, False, "", ""
        )       
        # Exception if bogus stokes string given #2 
        testit(
            twogauss, "", "", "", "v", 2, "", 1, -1, False, "", ""
        )       
        # Exception if no gaussians and no polynomial specified
        testit(
            twogauss, "", "", "", "", 2, "", 0, -1, False, "", ""
        )         
        
    def test_1(self):
        """Tests of averaging over a region and then fitting"""
        imagename = twogauss
        box = ""
        region = ""
        chans = ""
        stokes = ""
        axis = 2
        mask = ""
        ngauss = 2
        poly = -1
        multifit = False
        model = ""
        residual = ""
        for code in [run_fitprofile, run_specfit]:
            res = code(
                imagename, box, region, chans,
                stokes, axis, mask, ngauss, poly,
                multifit, model, residual
            )
            self.assertTrue(len(res["converged"]) == 1)
            self.assertTrue(res["converged"][0,0,0,0])
            # even though two components given, only one is fit
            self.assertTrue(res["ncomps"][0,0,0,0] == 1)
            # the fit component is a gaussian
            self.assertTrue(res["type"][0,0,0,0,0] == "GAUSSIAN")
            gs = res["gs"]
            self.assertAlmostEqual(gs["amp"][0,0,0,0,0], 49.7, 1, "amplitude determination failure")
            self.assertAlmostEqual(gs["ampErr"][0,0,0,0,0], 4.0, 1, "amplitude error determination failure")
            self.assertAlmostEqual(gs["center"][0,0,0,0,0], -237.7, 1, "center determination failure")
            self.assertAlmostEqual(gs["centerErr"][0,0,0,0,0], 1.7, 1, "center error determination failure")
            self.assertAlmostEqual(gs["fwhm"][0,0,0,0,0], 42.4, 1, "fwhm determination failure")
            self.assertAlmostEqual(gs["fwhmErr"][0,0,0,0,0], 4.0, 1, "fwhm error determination failure")

            self.assertTrue(res["xUnit"] == "km/s")
            self.assertTrue(res["yUnit"] == "Jy")
 
    def test_2(self):
        """ multipixel, two gaussian fit"""
        imagename = twogauss
        box = ""
        region = ""
        chans = ""
        stokes = ""
        axis = 2
        mask = ""
        ngauss = 2
        poly = -1
        multifit = True
        model = ""
        residual = ""
        for code in [run_fitprofile, run_specfit]:
            res = code(
                imagename, box, region, chans,
                stokes, axis, mask, ngauss, poly,
                multifit, model, residual
            )
            self.assertTrue(len(res["converged"].ravel()) == 81)
            self.assertTrue(res["converged"].all())
            self.assertTrue(res["ncomps"][0, 0, 0, 0] == 1)
            self.assertTrue((res["ncomps"][:, 1:, 0, 0] == 2).all())
            self.assertTrue((res["ncomps"][1:, 0, 0, 0] == 2).all())
            self.assertTrue((res["type"][:,:,:,:,0] == "GAUSSIAN").all())
            self.assertTrue(res["type"][0, 0, 0, 0, 1] == "UNDEF")
            self.assertTrue((res["type"][:, 1:, 0, 0, 1] == "GAUSSIAN").all())
            self.assertTrue((res["type"][1:, 0, 0, 0, 1] == "GAUSSIAN").all())

            self.assertTrue(res["xUnit"] == "km/s")
            self.assertTrue(res["yUnit"] == "Jy")
            
    def test_3(self):
        """ Test two gaussian + one polynomial image"""
        imagename = polyim
        box = ""
        region = ""
        chans = ""
        stokes = ""
        axis = 2
        mask = ""
        ngauss = 2
        poly = 3
        multifit = True
        model = ""
        residual = ""
        for code in [run_fitprofile, run_specfit]:
            res = code(
                imagename, box, region, chans,
                stokes, axis, mask, ngauss, poly,
                multifit, model, residual
            )
            self.assertTrue(len(res["converged"].ravel()) == 81)
            # fit #72 did not converge
            self.assertTrue(res["converged"][:, :7, 0, 0].all())
            self.assertTrue(res["converged"][1:,8,0,0].all())
            self.assertFalse(res["converged"][0, 8, 0, 0])

            self.assertTrue(res["xUnit"] == "km/s")
            self.assertTrue(res["yUnit"] == "Jy")

    def test_4(self):
        """writing solution images for multipixel, two gaussian fit"""
        imagename = twogauss
        box = ""
        region = ""
        chans = ""
        stokes = ""
        axis = 2
        mask = ""
        ngauss = 2
        poly = -1
        multifit = True
        model = ""
        residual = ""
        [
            amp, amperr, center, centererr,
            fwhm, fwhmerr, integral, integralerr
        ] = solims
        for code in [run_fitprofile, run_specfit]:
            res = code(
                imagename, box, region, chans,
                stokes, axis, mask, ngauss, poly,
                multifit, model, residual, amp,
                amperr, center, centererr, fwhm, fwhmerr,
                integral, integralerr
            )
            for im in solims:
                self.checkImage(im, datapath + im)
                shutil.rmtree(im)
                
    def test_4_5(self):
        """writing solution images for multipixel, two gaussian fit with mask - CAS-6134"""
        imagename = twogauss
        outfile = "CAS6134_in.im"
        myia = iatool()
        myia.open(imagename)
        subim = myia.subimage(outfile=outfile)
        myia.done()
        cc = subim.getchunk()
        cc[5,5,:,:] = 1e9
        subim.putchunk(cc)
        # so we have a mask in the output
        subim.calcmask(outfile + "<1e8")
        subim.done()
        imagename = outfile
        box = ""
        region = ""
        chans = ""
        stokes = ""
        axis = 2
        mask = ""
        ngauss = 2
        poly = -1
        multifit = True
        model = ""
        residual = ""
        [
            amp, amperr, center, centererr,
            fwhm, fwhmerr, integral, integralerr
        ] = solims
        for code in [run_fitprofile, run_specfit]:
            res = code(
                imagename, box, region, chans,
                stokes, axis, mask, ngauss, poly,
                multifit, model, residual, amp,
                amperr, center, centererr, fwhm, fwhmerr,
                integral, integralerr
            )
            # running successfully validates that the fix worked
            
    def test_5(self):
        """test results of multi-pixel one gaussian fit with estimates file"""
        imagename = twogauss
        ngauss=10
        box = ""
        region = ""
        chans = ""
        stokes = ""
        axis = 2
        mask = ""
        poly = -1
        estimates = datapath + "goodProfileEstimatesFormat_2.txt"
        multifit = True
        model = ""
        residual = ""
        for code in [run_fitprofile, run_specfit]:
            res = code(
                imagename, box, region, chans,
                stokes, axis, mask, ngauss, poly,
                multifit, model, residual, 
                estimates=estimates
            )
        # no tests yet, just confirm it runs to completion

    def test_6(self):
        """test results of non-multi-pixel one gaussian fit with estimates file"""
        imagename = twogauss
        ngauss=10
        box = ""
        region = ""
        chans = ""
        stokes = ""
        axis = 2
        mask = ""
        poly = -1
        estimates = datapath + "goodProfileEstimatesFormat_2.txt"
        multifit = False
        model = ""
        residual = ""
        for code in [run_fitprofile, run_specfit]:
            res = code(
                imagename, box, region, chans,
                stokes, axis, mask, ngauss, poly,
                multifit, model, residual, 
                estimates=estimates
            )
            self.assertTrue(res['converged'] == 1)
            self.assertTrue(res['converged'][0][0][0][0])
            self.assertTrue(res['ncomps'][0][0][0][0] == 1)
            self.assertTrue(res["type"][0,0,0,0,0] == "GAUSSIAN")
            gs = res["gs"]
            self.assertAlmostEqual(gs["amp"][0,0,0,0,0], 49.7, 1, "amplitude determination failure")
            self.assertAlmostEqual(gs["ampErr"][0,0,0,0,0], 4.0, 1, "amplitude error determination failure")
            self.assertAlmostEqual(gs["center"][0,0,0,0,0], -237.7, 1, "center determination failure")
            self.assertAlmostEqual(gs["centerErr"][0,0,0,0,0], 1.7, 1, "center error determination failure")
            self.assertAlmostEqual(gs["fwhm"][0,0,0,0,0], 42.4, 1, "fwhm determination failure")
            self.assertAlmostEqual(gs["fwhmErr"][0,0,0,0,0], 4.0, 1, "fwhm error determination failure")
            self.assertTrue(res["xUnit"] == "km/s")
            self.assertTrue(res["yUnit"] == "Jy")

    def test_7(self):
        """test results of non-multi-pixel one gaussian fit with estimates file keeping peak fixed"""
        imagename = twogauss
        ngauss=10
        box = ""
        region = ""
        chans = ""
        stokes = ""
        axis = 2
        mask = ""
        poly = -1
        estimates = datapath + "goodProfileEstimatesFormat_3.txt"
        multifit = False
        model = ""
        residual = ""
        for code in [run_fitprofile, run_specfit]:
            res = code(
                imagename, box, region, chans,
                stokes, axis, mask, ngauss, poly,
                multifit, model, residual, 
                estimates=estimates
            )
            self.assertTrue(res['converged'] == 1)
            self.assertTrue(res['converged'][0][0][0][0])
            self.assertTrue(res['ncomps'][0][0][0][0] == 1)
            self.assertTrue(res["type"][0,0,0,0,0] == "GAUSSIAN")
            self.assertAlmostEqual(res["gs"]["amp"][0,0,0,0,0], 45, 1, "amplitude determination failure")
            self.assertAlmostEqual(res["gs"]["ampErr"][0,0,0,0,0], 0, 1, "amplitude error determination failure")
            self.assertAlmostEqual(res["gs"]["center"][0,0,0,0,0], -237.7, 1, "center determination failure")
            self.assertAlmostEqual(res["gs"]["centerErr"][0,0,0,0,0], 1.9, 1, "center error determination failure")
            self.assertAlmostEqual(res["gs"]["fwhm"][0,0,0,0,0], 45.6, 1, "fwhm determination failure")
            self.assertAlmostEqual(res["gs"]["fwhmErr"][0,0,0,0,0], 3.8, 1, "fwhm error determination failure")
            self.assertTrue(res["xUnit"] == "km/s")
            self.assertTrue(res["yUnit"] == "Jy")

            
    def test_stretch(self):
        """specfit : test mask stretch"""
        imagename = twogauss
        yy = iatool()
        yy.open(imagename)
        mycsys = yy.coordsys().torecord()
        yy.done()
        mymask = "maskim"
        yy.fromshape(mymask, [9, 9, 1, 1])
        yy.setcoordsys(mycsys)
        yy.addnoise()
        yy.done()
        yy.open(imagename)
        self.assertRaises(
            Exception, yy.fitprofile,
            ngauss=2, mask=mymask + ">-100",
            stretch=False
        )
        zz = specfit(
            imagename, ngauss=2, mask=mymask + ">-100",
            stretch=False
        )
        self.assertTrue(zz == None)
        zz = yy.fitprofile(
            ngauss=2, mask=mymask + ">-100",
            stretch=True
        )
        self.assertTrue(len(zz.keys()) > 0)
        yy.done()
        zz = specfit(
            imagename, ngauss=2, mask=mymask + ">-100",
            stretch=True
        )
        self.assertTrue(len(zz.keys()) > 0)

    def test_8(self):
        """ Test two gaussian + one polynomial image with estimates"""

        estimates = datapath + "poly+2gauss_estimates.txt"
        
        for code in [run_fitprofile, run_specfit]:
            res = code(
                imagename=polyim, box="", region="", chans="",
                stokes="", axis=2, mask="", ngauss=0, poly=3,
                multifit=True, model="", residual="", estimates=estimates
            )
            self.assertTrue(len(res["converged"].ravel()) == 81)
            # fit #0 did not converge
            self.assertTrue(res["converged"][1:, :, 0, 0].all())
            self.assertTrue(res["converged"][0,1:,0,0].all())
            self.assertFalse(res["converged"][0, 0, 0, 0])

            self.assertTrue(res["xUnit"] == "km/s")
            self.assertTrue(res["yUnit"] == "Jy")
        gs = res["gs"]
        center = gs["center"]
        center[0,0,0,0,0] = nanvalue
        center[0,0,0,0,1] = nanvalue

        amp = gs["amp"]
        amp[0,0,0,0,0] = nanvalue
        amp[0,0,0,0,1] = nanvalue

        fwhm = gs["fwhm"]
        fwhm[0,0,0,0,0] = nanvalue
        fwhm[0,0,0,0,1] = nanvalue

        for code in [run_fitprofile, run_specfit]:
            res = code(
                imagename=polyim, box="", region="", chans="",
                stokes="", axis=2, mask="", ngauss=0, poly=3,
                multifit=True, model="", residual="", estimates="",
                pampest=[50, 10], pcenterest=[90, 30], pfwhmest=[10, 7]
            )
            self.assertTrue(len(res["converged"].ravel()) == 81)
            # fit #0 did not converge
            self.assertTrue(res["converged"][1:, :, 0, 0].all())
            self.assertTrue(res["converged"][0,1:,0,0].all())
            self.assertFalse(res["converged"][0, 0, 0, 0])

            self.assertTrue(res["xUnit"] == "km/s")
            self.assertTrue(res["yUnit"] == "Jy")
            self.assertTrue(len(res["direction"].ravel()) == 81)
            gs = res["gs"]
            gs["center"][0,0,0,0,0] = nanvalue
            gs["center"][0,0,0,0,1] = nanvalue
            self.assertTrue((gs["center"] == center).all())
            
            gs["amp"][0,0,0,0,0] = nanvalue
            gs["amp"][0,0,0,0,1] = nanvalue
            self.assertTrue((abs(gs["amp"] - amp) < 1e-13).all())
            
            gs["fwhm"][0,0,0,0,0] = nanvalue
            gs["fwhm"][0,0,0,0,1] = nanvalue
            self.assertTrue((abs(gs["fwhm"] - fwhm) < 1e-13).all())

    def test_9(self):
        """Polynomial fitting, moved from imagetest_regression.py"""
        shape = [16,16,128]
        imname = 'ia.fromshape.image'
        myim = ia.newimagefromshape(shape=shape)
        myim.set(pixels='1.0')        #
        residname = 'ia.fromshape.resid'
        fitname = 'ia.fromshape.fit'
        res = myim.fitprofile (multifit=True, residual=residname, model=fitname, poly=0, ngauss=0, axis=2)
        myim.done()
        myim.open(residname)
        pixels = myim.getchunk()
        self.assertTrue(pixels.shape == tuple(shape))
        self.assertTrue((pixels == 0).all())
        myim.done()
        
    def test_10(self):
        """test results of non-multi-fit gaussian triplet"""
        imagename=datapath+gauss_triplet
        gmampcon = [0.7, 0.55]
        gmcentercon = [52, 0]
        for code in [run_fitprofile, run_specfit]:
            res = code(
                imagename=imagename, box="", region="", chans="",
                stokes="", axis=2, mask="", ngauss=0, poly=-1,
                multifit=False, model="", residual="", estimates="",
                gmncomps=3, gmampest=[1.2, 0.8, 0.6], 
                gmcenterest=[20, 72, 100], gmfwhmest=[4, 4, 4],
                gmampcon=gmampcon, gmcentercon=gmcentercon
            )
            self.assertTrue(res["type"].ravel() == "GAUSSIAN MULTIPLET")
            gm0 = res["gm0"]
            exp = [4.15849, 2.91095, 2.28717]
            got = gm0["amp"].ravel()
            for i in [0, 1, 2]:
                self.assertAlmostEqual(got[i], exp[i], 5)
            exp = [1149.73, 1138.76, 1133.66]
            got = gm0["center"].ravel()
            for i in [0, 1, 2]:
                self.assertAlmostEqual(got[i], exp[i], 2)
            exp = [5.75308, 4.09405, 3.93497]
            got = gm0["fwhm"].ravel()
            for i in [0, 1, 2]:
                self.assertAlmostEqual(got[i], exp[i], 5)
            exp = [0.0301945, 0.0211362, 0.016607]
            got = gm0["ampErr"].ravel()
            for i in [0, 1, 2]:
                self.assertAlmostEqual(got[i], exp[i], 7)
            exp = [0.0221435, 0.0221435, 0.0475916]
            got = gm0["centerErr"].ravel()
            for i in [0, 1, 2]:
                self.assertAlmostEqual(got[i], exp[i], 7)
            exp = [0.0556099, 0.085414, 0.0987483]
            got = gm0["fwhmErr"].ravel()
            for i in [0, 1, 2]:
                self.assertAlmostEqual(got[i], exp[i], 7)
            self.assertAlmostEqual(
                gm0["amp"].ravel()[0]*gmampcon[0],
                gm0["amp"].ravel()[1], 7
            )
            self.assertAlmostEqual(
                gm0["ampErr"].ravel()[0]*gmampcon[0],
                gm0["ampErr"].ravel()[1], 7
            )
            self.assertAlmostEqual(
                gm0["amp"].ravel()[0]*gmampcon[1],
                gm0["amp"].ravel()[2], 7
            )
            self.assertAlmostEqual(
                gm0["ampErr"].ravel()[0]*gmampcon[1],
                gm0["ampErr"].ravel()[2], 7
            )
            ia.open(imagename)
            mc = ia.coordsys()
            ia.done()
            restfreq = mc.restfrequency()["value"][0]
            dv = mc.increment()["numeric"][2]
            increment = -dv/restfreq*299797
            self.assertAlmostEqual(
                gm0["center"].ravel()[0] + gmcentercon[0]*increment,
                gm0["center"].ravel()[1], 3
            )
            self.assertAlmostEqual(
                gm0["centerErr"].ravel()[0],
                gm0["centerErr"].ravel()[1], 7
            )

    def test_11(self):
        """test results of multi-fit gaussian triplet"""
        imagename=datapath+gauss_triplet
        gmampcon = [0.7, 0.55]
        gmcentercon = [52, 0]
        logfile = "mylog.txt"
        i = 1
        for code in [run_fitprofile, run_specfit]:
            res = code(
                imagename=imagename, box="", region="", chans="",
                stokes="", axis=2, mask="", ngauss=0, poly=-1,
                multifit=True, center="center",
                centererr="centerErr", fwhm="fwhm",
                fwhmerr="fwhmErr", amp="amp", amperr="ampErr",
                integral="integral", integralerr="integralErr",
                gmncomps=3, gmampest=[1.2, 0.1, 0.1], 
                gmcenterest=[20, 0, 100], gmfwhmest=[4, 4, 4],
                gmampcon=gmampcon, gmcentercon=gmcentercon,
                logfile=logfile
            )
            for image in (
                "center", "centerErr", "fwhm", "fwhmErr", "amp",
                "ampErr", "integral", "integralErr"
            ):
                self.checkImage(
                    image + "_gm", datapath + image + "_gm"
                )
            # appending, second time through size should double
            self.assertTrue(os.path.getsize(logfile) > 3e4*i)
            i = i+1

    def test_12(self):
        """test results of lorentzian fitting"""
        imagename=datapath+two_lorentzians
        pamp = [1, 7]
        pcen = [30, 111]
        pfwhm = [4, 4]
        pfunc = ["l", "l"]
        
        logfile = "two_lorentzian_fit.log"
        i = 1
        for code in [run_fitprofile, run_specfit]:
            res = code(
                imagename=imagename, box="", region="", chans="",
                stokes="", axis=2, mask="", ngauss=0, poly=-1,
                multifit=True, center="center",
                centererr="centerErr", fwhm="fwhm",
                fwhmerr="fwhmErr", amp="amp", amperr="ampErr",
                integral="integral", integralerr="integralErr",
                pampest=pamp, pcenterest=pcen, pfwhmest=pfwhm,
                logfile=logfile, pfunc=pfunc
            )
            for image in (
                "center", "centerErr", "fwhm", "fwhmErr", "amp",
                "ampErr", "integral", "integralErr"
            ):
                self.checkImage(
                    image + "_ls", datapath + image + "_ls"
                )
            # appending, second time through size should double
            self.assertTrue(os.path.getsize(logfile) > 2e4*i)
            i = i+1
            
    def test_13(self):
        """test setting solution parameter validities """
        imagename = datapath + invalid_fits
        i = 0
        goodfwhmrange= [[0], [2,20]]
        for gfr in goodfwhmrange:
            for code in [run_fitprofile, run_specfit]:
                res = code(
                    imagename=imagename, box="", region="", chans="",
                    stokes="", axis=3, mask="", ngauss=2, poly=-1,
                    multifit=True, logresults=False, goodfwhmrange=gfr
                )
                if i == 0:
                    exp = 16
                else:
                    exp = 6
                    #print "valid " + str(res["valid"])
                    #print "*** fwhm " + str(res["gs"]["fwhm"])
                self.assertTrue(res["valid"].sum() == exp)
            i = i+1
          
    def test_14(self):
        imagename = datapath + birdie
        sigmaimage = "sigma.im"
        myia = iatool()
        myia.open(imagename)
        bb = myia.getchunk()
        myia.done()
        fullsigma = bb
        fullsigma[:] = 1
        s = []
        s.append(fullsigma[:])
        s.append(fullsigma[0, 0, :, 0].reshape([1, 1, 100, 1]))
        s.append(fullsigma[0, 0, :, 0])
        outsigma = "outsigma.im"
        expfwhmtrue = {
            "0": [ 61.77550675, 41.74020775],
            "0.1" : [82.55515563, 36.4484157],
            "1": [ 61.89796905, 41.69283371],
            "100": [ 61.77551883, 41.74020306]
        }
        expfwhmerrtrue = {
            "0": [  2.88886469e-07, 4.18990637e-07],
            "0.1": [ 1.96169836, 2.99212108],
            "1": [ 0.30788521, 0.44558165],
            "100": [ 0.00307781, 0.00446394]
        }
        expfwhmfalse = {
            "0": 42.43941294,
            "0.1": 42.43941294,
            "1": 42.43941294,
            "100":  42.43941294
        }
        expfwhmerrfalse = {
            "0": 4.0707913,
            "0.1": 6.09143172,
            "1": 4.07523664,
            "100": 4.04975604
        }
        for sigma in (s):
            for birdiesigma in [0, 0.1, 1, 100]:
                fullsigma[:, :, 50, :] = birdiesigma
                mymax = max(1, birdiesigma)
                for i in range(2):
                    sig = sigma
                    if (sig.ndim == 1):
                        sig[50] = birdiesigma
                    else:
                        sig[:,:,50,:] = birdiesigma
                    if i == 1:
                        myia.fromarray(sigmaimage, sig, overwrite=T)
                        myia.done()
                        sig = sigmaimage
                    for code in [run_fitprofile, run_specfit]:
                        for multifit in [True, False]:
                            res = code(
                                imagename=imagename, box="", region="", chans="",
                                stokes="", axis=2, mask=imagename + "<1000", ngauss=2, poly=-1,
                                multifit=multifit, logresults=False, sigma=sig,
                                outsigma=outsigma
                            )
                            if multifit:
                                self.assertTrue((abs(res["gs"]["fwhm"][8,8,0,0,:] - expfwhmtrue[str(birdiesigma)]) < 1e-7).all())
                                self.assertTrue((abs(res["gs"]["fwhmErr"][8,8,0,0,:] - expfwhmerrtrue[str(birdiesigma)]) < 1e-7).all())
                            else:
                                self.assertTrue(abs(res["gs"]["fwhm"][0,0,0,0,0] - expfwhmfalse[str(birdiesigma)]) < 1e-7)
                                print "*** got " + str(abs(res["gs"]["fwhmErr"][0,0,0,0,0] - expfwhmerrfalse[str(birdiesigma)]))
                                self.assertTrue(abs(res["gs"]["fwhmErr"][0,0,0,0,0] - expfwhmerrfalse[str(birdiesigma)]) < 1e-5)
                            myia.open(outsigma)
                            if (birdiesigma == 0 or birdiesigma == 1):
                                self.assertTrue((mymax*myia.getchunk() == fullsigma).all())
                            else:
                                self.assertTrue(((mymax*myia.getchunk() - fullsigma)/fullsigma < 1e-7).all())
                            myia.remove()
                
        


def suite():
    return [specfit_test]
