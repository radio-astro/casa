##########################################################################
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
# Test suite for the CASA tool method ia.pv()
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
# Test for the ia.pv() tool method
# </etymology>
#
# <synopsis>
# Test the ia.pv() tool method
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_ia_pv[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the ia.pv() tool method to ensure
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

datapath = os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/imageanalysis/ImageAnalysis/'

def run_ia_pv(
    imagename, outfile, start, end, width,
    center=[], length=[], pa=[]
):
    myia = iatool()
    myia.open(imagename)
    if (not myia.isopen()):
        myia.done()
        raise Exception
    res = myia.pv(
        outfile=outfile, start=start, end=end, width=width,
        center=center, length=length, pa=pa
    )
    myia.done()
    return res

def run_impv(
    imagename, outfile, start, end, width,
    center=[], length=[], pa=[], mode="coords"
):
    return impv(
        imagename=imagename, outfile=outfile, start=start,
        end=end, width=width, center=center, length=length,
        mode=mode, pa=pa
    )


class ia_pv_test(unittest.TestCase):
    
    def setUp(self):
        self.ia = iatool()
    
    def tearDown(self):
        #pass
        self.assertTrue(len(tb.showcache()) == 0)
    
    def test_pv(self):
        """ ia.pv(): Test pv()"""
        myia = self.ia
        imagename = "zxye.im"
        myia.fromshape(imagename, [10, 10, 10])
        bb = myia.getchunk()
        # basic sanity test, no rotation involved
        for i in range(10):
            bb[i,5,:] = i
            bb[i,0:5,:] = i+1
            bb[i,6:10,:] = i+2
        myia.putchunk(bb)
        expeccoord = myia.toworld([1,5,0])['numeric'][2]
        mycsys = myia.coordsys()
        units = mycsys.units()
        expinc = mycsys.increment()["numeric"]
        expinc = [
            abs(
                qa.convert(
                    qa.quantity(expinc[0], units[0]), "arcsec"
                )["value"]
            ),
            expinc[2]
        ]
        myia.done()
        self.assertTrue(len(tb.showcache())== 0)
        pv = iatool()
        for code in [0, 1]:
            # no width
            for i in range(7):
                if i == 0:
                    start = [2, 5]
                    end = [7, 5]
                    mode = "coords"
                elif i == 1:
                    start = ["3.00000038arcmin", "0'"]
                    end = ["2.15980000e+04'", "0arcmin"]
                    mode = "coords"
                if i == 2:
                    start = ["0h0m12s", "0d0m0s" ]
                    end = ["23:59:52", "0.0.0"]
                    mode = "coords"
                if i == 3:
                    center = [4.5, 5]
                    length = 5
                    pa = "90deg"
                    mode = "length"
                if i == 4:
                    center = ["0:0:02", "0.0.0"]
                    length = 5
                    pa = "90deg"
                    mode = "length"
                if i == 5:
                    center = ["0:0:02", "0.0.0"]
                    length = "5arcmin"
                    pa = "90deg"
                    mode = "length"
                if i == 6:
                    center = [4.5, 5]
                    length = "5arcmin"
                    pa = "90deg"
                    mode = "length"
                outfile = "test_pv_" + str(code) + str(i)
                if i <= 2:
                    if code == 0:
                        xx = run_ia_pv(
                            imagename=imagename, outfile=outfile, start=start,
                            end=end, width=1
                        )
                    elif code == 1:
                        xx = run_impv(
                            imagename=imagename, outfile=outfile, start=start,
                            end=end, width=1, mode=mode
                        )
                else:
                    if code == 0:
                        xx = run_ia_pv(
                            imagename=imagename, outfile=outfile, start=[],
                            end=[], width=1, center=center, length=length,
                            pa=pa
                        )
                    elif code == 1:
                        print "*** mode ", mode
                        xx = run_impv(
                            imagename=imagename, outfile=outfile, start=[],
                            end=[], width=1, center=center, length=length,
                            pa=pa, mode=mode
                        )
                if (type(xx) == type(ia)):
                    xx.done()
                self.assertTrue(len(tb.showcache())== 0)
                pv.open(outfile)
                expec = [6, 10]
                got = pv.shape()
                self.assertTrue((got == expec).all())
                expec = numpy.zeros(got)
                for i in range(10):
                    expec[:,i] = range(2,8)
                got = pv.getchunk()
                self.assertTrue((got == expec).all())
                self.assertTrue(pv.getchunk(getmask=T).all())
                got = pv.toworld([0,0,0])['numeric'][1]
                self.assertTrue(abs(got - expeccoord) < 1e-6)
                gotinc = pv.coordsys().increment()["numeric"]
                # the position offset axis always has units of arcsec, the units
                # in the input image were arcmin
                self.assertTrue((abs(gotinc - expinc) < 1e-5).all())
                pv.done()
            
            
            # width > 1
            for i in range(4):
                outfile = "test_pv_1_" + str(code) + str(i)
                if i == 0:
                    width = 3;
                elif i == 1:
                    width = "3arcmin"
                elif i == 2:
                    width = "1.1arcmin"
                elif i == 3:
                    width = qa.quantity("1.2arcmin")
                if code == 0:
                    xx = run_ia_pv(
                        imagename=imagename, outfile=outfile, start=[2, 5],
                        end=[7, 5], width=width
                    )
                elif code == 1:
                    xx = run_impv(
                        imagename=imagename, outfile=outfile, start=[2, 5],
                        end=[7, 5], width=width, mode="coords"
                    )
                if (type(xx) == type(ia)):
                    xx.done()
                pv.open(outfile)
                expec = [6, 10]
                got = pv.shape()
                self.assertTrue((got == expec).all())
                expec = numpy.zeros(got)
                for i in range(10):
                    expec[:,i] = range(3,9)
                got = pv.getchunk()
                self.assertTrue((got == expec).all())
                self.assertTrue(pv.getchunk(getmask=T).all())
                pv.done()
        
    def test_stretch(self):
        """ia.pv(): Test stretch parameter"""
        yy = iatool()
        mymask = "maskim"
        yy.fromshape(mymask, [200, 200, 1, 1])
        yy.addnoise()
        yy.done()
        shape = [200,200,1,20]
        yy.fromshape("kk", shape)
        yy.addnoise()
        self.assertRaises(
            Exception,
            yy.pv, start=[2,2], end=[20,2],
            mask=mymask + ">0", stretch=False
        )
        zz = yy.pv(
            start=[2,2], end=[20,2], mask=mymask + ">0", stretch=True
        )
        mytype = type(yy)
        self.assertTrue(zz and type(zz) == mytype)
        yy.done()
        zz.done()
        self.assertFalse(
            impv(
                 imagename="kk", outfile="x1.im", start=[2,2], end=[20,2],
                 mask=mymask + ">0", stretch=False
            )
        )
        self.assertTrue(
            impv(
                 imagename="kk", outfile="xyz", start=[2,2], end=[20,2],
                 mask=mymask + ">0", stretch=True
            )
        )
    
    def test_CAS_2996(self):
        """ia.pv(): Test issues raised in CAS-2996"""
        # the only tests necessary here are to ensure ia.pv() runs 
        # successfully for the provided inputs
        # calculate stats to make sure region determination code doesn't segfault (CAS-4881)
        print "*** aa"
        myia = self.ia
        myia.open(datapath + "pv1.im")
        xx = myia.pv(start = [30, 30], end = [250, 250])
        xx.statistics()
        xx = myia.pv(start = [30, 250], end = [250, 30])
        xx.statistics()
        xx = myia.pv(start = [250, 250], end = [30, 30])
        xx.statistics()
        xx = myia.pv(start = [250, 30], end = [30, 250])
        xx.statistics()
        print "*** ab"

        myia.open(datapath + "pv2.im")
        x1 = 264.865854
        x2 = 166.329268
        y1 = 142.914634
        y2 = 232.670732
        # success or failure should not depend on the end pixel's location relative to the start pixel.
        xx = myia.pv(start=[x1, y1], end=[x2, y2])
        # test units while we're at it
        self.assertTrue(xx.coordsys().units()[0] == "arcsec")
        xx = myia.pv(start=[x2, y1], end=[x1, y2])
        xx = myia.pv(start=[x2, y2], end=[x1, y1])
        xx = myia.pv(start=[x1, y2], end=[x2, y1], unit="rad")
        self.assertTrue(xx.coordsys().units()[0] == "rad")

        myia.done()
        xx.done()
        # test units from task level
        outfile = "unittest.im"
        unit="arcmin"
        print "*** ac"

        impv(
             imagename=datapath + "pv1.im", unit=unit,
             outfile="unittest.im", start=[3,3], end=[6,6]
        )
        myia.open(outfile)
        self.assertTrue(myia.coordsys().units()[0] == unit)
        myia.done()
        
    def test_fits(self):
        """ia.pv(): Test exporting and importing to/from FITS"""
        myia = self.ia
        myia.open(datapath + "pv1.im")
        xx = myia.pv(start = [30, 30], end = [250, 250])
        expec = ["OFFSET", "Frequency", "Stokes"]
        outfile = "zz.fits"
        xx.tofits(outfile)
        myia.open(outfile)
        got = myia.coordsys().names()
        print "got " + str(got)
        self.assertTrue(got == expec)
        xx.tofits(outfile, velocity=True, overwrite=True)
        xx.done()
        myia.open(outfile)
        got = myia.coordsys().names()
        myia.done()
        self.assertTrue(got == expec)
        
    def test_mask(self):
        """Verify fix of mask defect in CAS-5520"""
        myia = self.ia
        outfile = "mask_test_got.im"
        impv(
            imagename=datapath + "pv_mask_test.im", outfile=outfile,
            overwrite=True, start=[343,42],end=[343,660],width=425,unit='arcsec'
        )
        myia.open(datapath + "pv_mask_exp.im")
        expec = myia.getchunk(getmask=T)
        myia.open(outfile)
        got = myia.getchunk(getmask=T)
        myia.done()
        self.assertTrue((got == expec).all())

    def test_refpix_far_outside_image(self):
        """Test refpix far outside image doesn't lead to malloc error, CAS-5251"""
        myia = self.ia
        myia.fromshape("",[50,50,1000])
        csys = myia.coordsys()
        csys.setreferencepixel([2000, 2000, 500])
        myia.setcoordsys(csys.torecord())
        pv = myia.pv(start=[5,5], end=[10,10])

    def test_machine_precision_fix(self):
        """Test fix for finite machine precision issue, CAS-6043"""
        self.assertTrue(
            impv(
                imagename=datapath + 'CAS-6043.im', outfile="CAS-6043.out.im",
                start=[187,348], end=[228,383]
            )
        ) 
        
    def test_pa(self):
        """Test that when pa is given, the start of the slice is at pa and end is at pa-180deg"""
        myia = self.ia
        myia.open(datapath + "pv_patest_exp.im")
        expec = myia.getchunk()
        myia.done()
        imagename = datapath + "pv_patest.im"
        
        for length in [19, "19arcmin"]:
            for center in [
                [9,9], ["00h00m4s", "-0d1m"], "00:00:04 -0d1m",
                "GALACTIC +096.21.17.792 -060.12.37.929"
            ]:
                pa = "45deg"
                if type(center)==str and center.startswith("G"):
                    # pa = "68.46450771415163deg"
                    pa = "68.464508deg"
                outfile = "pv_patest_got" + str(length) + str(center) + ".im"
                impv(
                     imagename=imagename, outfile=outfile,
                     center=center, length=length, pa=pa,
                     mode="length"
                )
                myia.open(outfile)
                got = myia.getchunk()
                myia.done()
                self.assertTrue(abs(got/expec - 1).max() < 1e-6)

def suite():
    return [ia_pv_test]
