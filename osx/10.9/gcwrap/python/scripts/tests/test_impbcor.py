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
# Test suite for the CASA task impbcor
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
#   <li> <linkto class="task_imcollapse.py:description">impbcor</linkto> 
# </ul>
# </prerequisite>
#
# <etymology>
# Test for the impbcor task
# </etymology>
#
# <synopsis>
# Test the impbcor task and the ia.pbcor() method upon which it is built.
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_impbcor[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the impbcor task to ensure
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

im1 = "pbtest1_im.fits"
pb1 = "pbtest1_pb.fits"
co1_1 = "pbtest1_co1.fits"
co1_2 = "pbtest1_co2.im"

im2 = "pb2_im.fits"
pb2 = "pb2_pb.fits"  
co2 = "pb2_co.im"  

pb4 = "CAS_5096template.im"

data = [im1, pb1, co1_1, co1_2, im2, pb2, co2, pb4]

datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/imageanalysis/ImageAnalysis/'


def run_pbcor(
    imagename, pbimage, outfile, overwrite, region, box, chans,
    stokes, mask, mode, cutoff
):
    myia = iatool()
    myia.open(imagename)
    res = myia.pbcor(
        pbimage=pbimage, outfile=outfile, overwrite=overwrite,
        region=region, box=box, chans=chans, stokes=stokes,
        mask=mask, mode=mode, cutoff=cutoff
    )
    myia.close()
    myia.done()
    return res

def run_impbcor(
    imagename, pbimage, outfile, overwrite, region, box, chans,
    stokes, mask, mode, cutoff
):
    return impbcor(
        imagename=imagename, pbimage=pbimage, outfile=outfile,
        overwrite=overwrite, region=region, box=box, chans=chans,
        stokes=stokes, mask=mask, mode=mode, cutoff=cutoff
    )

class impbcor_test(unittest.TestCase):
    
    def setUp(self):
        
        for f in data:
            if os.path.isdir(datapath + f):
                shutil.copytree(datapath + f, f)
            else:
                shutil.copy(datapath + f, f)
    
    def tearDown(self):
        for f in data:
            if os.path.isdir(f):
                shutil.rmtree(f)
            else:
                os.remove(f)
            self.assertTrue(len(tb.showcache()) == 0)

    def checkImage(self, gotImage, expectedName):
        expected = iatool()                                
        expected.open(expectedName)
        got = iatool()
        if type(gotImage) == str:
            got.open(gotImage)
        else:
            got = gotImage
        self.assertTrue((got.shape() == expected.shape()).all())
        diffData = got.getchunk() - expected.getchunk()
        self.assertTrue(abs(diffData).max() == 0)
        gotCsys = got.coordsys()
        expectedCsys = expected.coordsys()
        diffPixels = gotCsys.referencepixel()['numeric'] - expectedCsys.referencepixel()['numeric']
        self.assertTrue(abs(diffPixels).max() == 0)
        denom = expectedCsys.referencevalue()['numeric']
        for i in range(len(denom)):
            if (denom[i] == 0):
                denom[i] = 1
        fracDiffRef = (
            gotCsys.referencevalue()['numeric'] - expectedCsys.referencevalue()['numeric']
        )/denom;
        print "*** " + str(abs(fracDiffRef).max())
        self.assertTrue(abs(fracDiffRef).max() <= 1.5e-6)
        gotnpts = got.statistics()['npts']
        expnpts = expected.statistics()['npts']
        self.assertTrue(gotnpts == expnpts)

        got.close()
        got.done()
        expected.close()
        expected.done()

    def test_exceptions(self):
        """impbcor: Test various exception cases"""
        
        def testit(
            imagename, pbimage, outfile, overwrite, region,
            box, chans, stokes, mask, mode, cutoff, wantreturn
        ):
            for i in [0,1]:
                if (i==0):
                    self.assertRaises(
                        Exception, run_pbcor, imagename=imagename,
                        pbimage=pbimage, outfile=outfile,
                        overwrite=overwrite, region=region, box=box,
                        chans=chans, stokes=stokes, mask=mask,
                        mode=mode, cutoff=cutoff
                    )
                else:
                    self.assertFalse(
                        run_impbcor(
                            imagename=imagename, pbimage=pbimage,
                            outfile=outfile, overwrite=overwrite,
                            region=region, box=box, chans=chans,
                            stokes=stokes, mask=mask, mode=mode,
                            cutoff=cutoff
                        )
                    )
        # no image name given
        testit(
            imagename="", pbimage=pb1, outfile="",
            overwrite=False, region="", box="",
            chans="", stokes="", mask="", mode="d", cutoff=-1.0,
            wantreturn=True
        )
        # bad image name given
        testit(
            imagename="totally_bogus", pbimage=pb1, outfile="",
            overwrite=False, region="", box="",
            chans="", stokes="", mask="", mode="d", cutoff=-1.0,
            wantreturn=True
        )
        # no pbimage name given
        testit(
            imagename=im1, pbimage="", outfile="",
            overwrite=False, region="", box="",
            chans="", stokes="", mask="", mode="d", cutoff=-1.0,
            wantreturn=True
        )
        # bad pbimage name given
        testit(
            imagename=im1, pbimage="totally_bogus2", outfile="",
            overwrite=False, region="", box="",
            chans="", stokes="", mask="", mode="d", cutoff=-1.0,
            wantreturn=True
        )
        # unwritable outfile
        testit(
            imagename=im1, pbimage=pb1, outfile="/bogusplace/bogusimage",
            overwrite=False, region="", box="",
            chans="", stokes="", mask="", mode="d", cutoff=-1.0,
            wantreturn=True
        )
        # bogus region
        testit(
            imagename=im1, pbimage=pb1, outfile="",
            overwrite=False, region="bogus_region", box="",
            chans="", stokes="", mask="", mode="d", cutoff=-1.0,
            wantreturn=True
        )
        # bad mode
        testit(
            imagename=im1, pbimage=pb1, outfile="",
            overwrite=False, region="", box="",
            chans="", stokes="", mask="", mode="zz", cutoff=-1.0,
            wantreturn=True
        )
        # incompatible image and pb
        testit(
            imagename=im1, pbimage=pb2, outfile="",
            overwrite=False, region="", box="",
            chans="", stokes="", mask="", mode="d", cutoff=-1.0,
            wantreturn=True
        )

    def _testit(
        self, expected, imagename, pbimage, overwrite, region, box,
        chans, stokes, mask, mode, cutoff
    ):
        myia = iatool()
        myia.open(pbimage)
        pbpix = myia.getchunk()
        myia.done()
        del myia
        for j in [0, 1]:
            for i in [0, 1]:
                outfile = str(i) + ".im"
                if j == 1:
                    pbimage = pbpix
                if i == 0:
                    mytool = run_pbcor(
                        imagename=imagename, pbimage=pbimage,
                        outfile=outfile, overwrite=overwrite,
                        region=region, box=box, chans=chans,
                        stokes=stokes, mask=mask, mode=mode,
                        cutoff=cutoff
                    )
                            
                    self.assertTrue(type(mytool) == type(ia))
                    self.checkImage(mytool, expected)
                    self.checkImage(outfile, expected)
                    shutil.rmtree(outfile)
                else:
                    outfile = outfile
                    res = run_impbcor(
                        imagename=imagename, pbimage=pbimage,
                        outfile=outfile, overwrite=overwrite,
                        region=region, box=box, chans=chans,
                        stokes=stokes, mask=mask, mode=mode,
                        cutoff=cutoff
                    )
                    self.assertTrue(res)
                    self.checkImage(outfile, expected)
                    shutil.rmtree(outfile)

    def test_1(self):
        """impbcor: Test full image divide"""
        self._testit(
            expected=co1_1, imagename=im1, pbimage=pb1,
            overwrite=False, region="", box="",
            chans="", stokes="", mask="", mode="d",
            cutoff=-1.0
        )

    def test_2(self):
        """impbcor: Test full image divide with cutoff"""
        self._testit(
            expected=co1_2, imagename=im1, pbimage=pb1,
            overwrite=False, region="", box="",
            chans="", stokes="", mask="", mode="d",
            cutoff=0.001
        )
    
    def test_3(self):
        """impbcor: Test full image divide with cutoff. Primary beam is 2 D, image is 4 D"""
        self._testit(
            expected=co2, imagename=im2, pbimage=pb2,
            overwrite=False, region="", box="",
            chans="", stokes="", mask="", mode="d",
            cutoff=0.001
        )

    def test_stretch(self):
        """ ia.pbcor(): Test stretch parameter"""
        yy = iatool()
        mymask = "maskim"
        yy.fromshape("", [113, 76, 1, 1])
        yy.addnoise()
        xx = yy.transpose(mymask, "0132")
        yy.done()
        xx.done()
        for i in [0,1]:
            if i == 0:
                yy.open(im2)
                self.assertRaises(
                    Exception,
                    yy.pbcor, pbimage=pb2,
                    mask=mymask + ">0", stretch=False
                )
                zz = yy.pbcor(
                    pbimage=pb2, mask=mymask + ">0", stretch=True
                )
                self.assertTrue(type(yy) == type(zz))
                yy.done()
                zz.done()
            else:
                zz = impbcor(
                    imagename=im2, pbimage=pb2,
                    mask=mymask + ">0", stretch=False
                )
                self.assertFalse(zz)
                zz = impbcor(
                    imagename=im2, pbimage=pb2, outfile="blahblah", mask=mymask + ">0", stretch=True
                )
                self.assertTrue(zz)
        
    def test_diff_spectral_coordinate(self):
        """Verify fix that a different spectral coordinates in target and template don't matter, CAS-5096"""
        imagename = datapath + "CAS_5096target.im"
        template = pb4
        outfile = "mypb.im"
        impbcor(
            imagename=imagename, pbimage=template,
            outfile=outfile, mask='"' + template + '">0.21'
        )
        self.assertTrue(os.path.exists(outfile))

def suite():
    return [impbcor_test]
