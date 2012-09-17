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
# Test suite for the CASA tool method ia.tofits()
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
# Test for the ia.tofits() tool method
# </etymology>
#
# <synopsis>
# Test the ia.tofits() tool method
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/gcwrap/python/scripts/regressions/admin/runUnitTest.py test_ia_tofits[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the ia.tofits() tool method to ensure
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

class ia_tofits_test(unittest.TestCase):
    
    def setUp(self):
        self.myia = iatool()
    
    def tearDown(self):
        self.myia.done()
    
    def test_stretch(self):
        """ ia.tofits(): Test stretch parameter"""
        yy = self.myia
        mymask = "maskim"
        yy.fromshape(mymask, [200, 200, 1, 1])
        yy.addnoise()
        yy.done()
        shape = [200,200,1,20]
        yy.fromshape("", shape)
        yy.addnoise()
        self.assertRaises(
            Exception,
            yy.tofits, outfile="blah1.fits",
            mask=mymask + ">0", stretch=False
        )
        zz = yy.tofits(
            outfile="blah2.fits",
            mask=mymask + ">0", stretch=True
        )
        self.assertTrue(zz and type(zz) == type(True))
        yy.done()
        
    def test_CAS3675(self):
        """ test fix for CAS 3675, outfile must be specified """
        name = "my.im"
        yy = self.myia
        yy.fromshape(name, [1,1,1,1])
        self.assertRaises(Exception, yy.tofits, overwrite=T)
        yy.done()
        self.assertFalse(exportfits(imagename=name, overwrite=T))
    
    def test_multibeam(self):
        """Test exporting and importing an image with multiple beams"""
        myia = self.myia
        shape = [10,10,10,4]
        myia.fromshape("", shape)
        bmaj = qa.quantity("10arcsec")
        bmin = qa.quantity("7arcsec")
        bpa = qa.quantity("45deg")
        myia.setrestoringbeam(
            major=bmaj, minor=bmin, pa=bpa,
            channel=0, polarization=0
        )
        cmaj = qa.quantity("12arcsec")
        cmin = qa.quantity("8arcsec")
        cpa = qa.quantity("50deg")
        myia.setrestoringbeam(
            major=cmaj, minor=cmin, pa=cpa,
            channel=6, polarization=3
        )
        myia.addnoise()
        exppix = myia.getchunk()
        fitsname = "myfits.fits"
        myia.tofits(outfile=fitsname)
        myia.done()
        for i in range(4):
            if i == 0:
                myia.fromfits("", fitsname)
            elif i == 1:
                myia.open(fitsname)
            elif i == 2:
                zz = iatool()
                myia = zz.newimagefromfits("", fitsname)
            else:
                zz = iatool()
                myia = zz.newimagefromfile(fitsname)
            ep = 1e-7
            for c in range(shape[2]):
                for p in range(shape[3]):
                    beam = myia.restoringbeam(c, p)
                    majax = qa.convert(beam["major"], "arcsec")["value"]
                    minax = qa.convert(beam["minor"], "arcsec")["value"]
                    pa = qa.convert(beam["positionangle"], "deg")["value"]
                    if c == 6 and p == 3:
                        self.assertTrue(abs(1 - majax/cmaj["value"]) < ep)
                        self.assertTrue(abs(1 - minax/cmin["value"]) < ep)
                        self.assertTrue(abs(1 - pa/cpa["value"]) < ep)
                    else:
                        self.assertTrue(abs(1 - majax/bmaj["value"]) < ep)
                        self.assertTrue(abs(1 - minax/bmin["value"]) < ep)
                        self.assertTrue(abs(1 - pa/bpa["value"]) < ep)
                    # ensure the pixel values were read correctly
                    gotpix = myia.getchunk()
                    self.assertTrue((gotpix == exppix).all())
            myia.done()

def suite():
    return [ia_tofits_test]
