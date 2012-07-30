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
# Test suite for the CASA ia.imageconcat() tool method
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
#   <li> <linkto class="image:description">ia.imageconcat()</linkto> 
# </ul>
# </prerequisite>
#
# <etymology>
# ia_imageconcat_test stands for ia.imageconcat test
# </etymology>
#
# <synopsis>
# test_ia_imageconcat.py is a Python script that tests the correctness
# of the ia.imageconcat() tool method.
# </synopsis> 
#
# <example>
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_ia_imageconcat[test1,test2,...]
# </example>
#
# <motivation>
# To provide a test standard to the ia.imageconcat() method to ensure
# coding changes do not break the associated bits 
# </motivation>
#

###########################################################################
import os
import casac
from tasks import *
from taskinit import *
import hashlib
import shutil
from __main__ import *
import unittest

class ia_imageconcat_test(unittest.TestCase):
    
    def setUp(self):
        self._myia = iatool()

    def tearDown(self):
        self._myia.done()


    def test_multibeam(self):
        """Test concatenating images with different beams"""
        myia = self._myia
        shape = [4, 4, 20]
        myia.fromshape("", shape)
        print "*** here 1"
        blc1=[0, 0, 0]
        trc1=[shape[0]-1, shape[1]-1, shape[2]/2-1]
        rg1 = rg.box(blc=blc1, trc=trc1)
        im1 = "image1.im"
        sub1 = myia.subimage(im1, region=rg1)
        im2 = "image2.im"
        blc2 = [0, 0, trc1[2]+1]
        trc2 = [shape[0]-1, shape[1]-1, shape[2]-1]
        rg2 = rg.box(blc=blc2, trc=trc2)
        sub2 = myia.subimage(im2, region=rg2)
        major = qa.quantity("3arcmin")
        minor = qa.quantity("2arcmin")
        pa = qa.quantity("0deg")
        major2 = qa.quantity("4arcmin")
        minor2 = qa.quantity("3arcmin")
        pa2 = qa.quantity("10deg")
        major3 = qa.quantity("5arcmin")
        minor3 = qa.quantity("4arcmin")
        pa3 = qa.quantity("20deg")
        print "*** here 2"

        # first image has no beam while second does
        sub1.setbrightnessunit("Jy/pixel") 
        sub2.setrestoringbeam(major=major, minor=minor, pa=pa)
        sub2.setbrightnessunit("Jy/beam")
        self.assertRaises(Exception, myia.imageconcat, "", [im1, im2])
        concat = myia.imageconcat("", [im1, im2], relax=True)
        self.assertTrue((concat.shape() == shape).all())
        
        # first image has a single beam, second has per plane beams
        sub1.setbrightnessunit("Jy/beam")
        sub1.setrestoringbeam(major=major, minor=minor, pa=pa)
        sub2.setbrightnessunit("Jy/beam")
        sub2.setrestoringbeam(remove=True)
        sub2.setrestoringbeam(
            major=major2, minor=minor2, pa=pa2,
            channel=0, polarization=-1
        )
        
        sub2.setrestoringbeam(
            major=major3, minor=minor3, pa=pa3,
            channel=5, polarization=-1
        )
        concat = myia.imageconcat("", [im1, im2])
        for i in range(concat.shape()[2]):
            beam = concat.restoringbeam(channel=i)
            if i < sub1.shape()[2]:
                self.assertTrue(qa.eq(qa.quantity(beam["major"]), major))
                self.assertTrue(qa.eq(qa.quantity(beam["minor"]), minor))
                self.assertTrue(qa.eq(qa.quantity(beam["positionangle"]), pa))
            elif i == sub1.shape()[2] + 5:
                self.assertTrue(qa.eq(qa.quantity(beam["major"]), major3))
                self.assertTrue(qa.eq(qa.quantity(beam["minor"]), minor3))
                self.assertTrue(qa.eq(qa.quantity(beam["positionangle"]), pa3))
            else:
                self.assertTrue(qa.eq(qa.quantity(beam["major"]), major2))
                self.assertTrue(qa.eq(qa.quantity(beam["minor"]), minor2))
                self.assertTrue(qa.eq(qa.quantity(beam["positionangle"]), pa2))
                
        # both images have a single beam which is the same
        sub1.setbrightnessunit("Jy/beam")
        sub1.setrestoringbeam(major=major, minor=minor, pa=pa)
        sub2.setrestoringbeam(remove=True)
        sub2.setbrightnessunit("Jy/beam")
        sub2.setrestoringbeam(major=major, minor=minor, pa=pa)
        concat = myia.imageconcat("", [im1, im2])
        beam = concat.restoringbeam()
        self.assertTrue(qa.eq(qa.quantity(beam["major"]), major))
        self.assertTrue(qa.eq(qa.quantity(beam["minor"]), minor))
        self.assertTrue(qa.eq(qa.quantity(beam["positionangle"]), pa))
        
        # both images have single, unequal beams
        sub1.setbrightnessunit("Jy/beam")
        sub1.setrestoringbeam(major=major, minor=minor, pa=pa)
        sub2.setrestoringbeam(remove=True)
        sub2.setbrightnessunit("Jy/beam")
        sub2.setrestoringbeam(major=major2, minor=minor2, pa=pa2)
        concat = myia.imageconcat("", [im1, im2])
        for i in range(concat.shape()[2]):
            beam = concat.restoringbeam(channel=i)
            if i < sub1.shape()[2]:
                self.assertTrue(qa.eq(qa.quantity(beam["major"]), major))
                self.assertTrue(qa.eq(qa.quantity(beam["minor"]), minor))
                self.assertTrue(qa.eq(qa.quantity(beam["positionangle"]), pa))
            else:
                self.assertTrue(qa.eq(qa.quantity(beam["major"]), major2))
                self.assertTrue(qa.eq(qa.quantity(beam["minor"]), minor2))
                self.assertTrue(qa.eq(qa.quantity(beam["positionangle"]), pa2))
        
        # first image has per plane beams, second has single beam
        sub2.setbrightnessunit("Jy/beam")
        sub2.setrestoringbeam(remove=True)
        sub2.setrestoringbeam(major=major, minor=minor, pa=pa)
        sub1.setbrightnessunit("Jy/beam")
        sub1.setrestoringbeam(remove=True)
        sub1.setrestoringbeam(
            major=major2, minor=minor2, pa=pa2,
            channel=0, polarization=-1
        )
        sub1.setrestoringbeam(
            major=major3, minor=minor3, pa=pa3,
            channel=5, polarization=-1
        )
        concat = myia.imageconcat("", [im1, im2])
        for i in range(concat.shape()[2]):
            beam = concat.restoringbeam(channel=i)
            if i == 5:
                self.assertTrue(qa.eq(qa.quantity(beam["major"]), major3))
                self.assertTrue(qa.eq(qa.quantity(beam["minor"]), minor3))
                self.assertTrue(qa.eq(qa.quantity(beam["positionangle"]), pa3))
            elif i < sub1.shape()[2]:
                self.assertTrue(qa.eq(qa.quantity(beam["major"]), major2))
                self.assertTrue(qa.eq(qa.quantity(beam["minor"]), minor2))
                self.assertTrue(qa.eq(qa.quantity(beam["positionangle"]), pa2))
            else:
                self.assertTrue(qa.eq(qa.quantity(beam["major"]), major))
                self.assertTrue(qa.eq(qa.quantity(beam["minor"]), minor))
                self.assertTrue(qa.eq(qa.quantity(beam["positionangle"]), pa))
                
        # both images have a single beam which is the same
        sub1.setbrightnessunit("Jy/beam")
        sub1.setrestoringbeam(remove=True)
        sub1.setrestoringbeam(major=major, minor=minor, pa=pa)
        sub2.setrestoringbeam(remove=True)
        sub2.setbrightnessunit("Jy/beam")
        sub2.setrestoringbeam(major=major, minor=minor, pa=pa)
        concat = myia.imageconcat("", [im1, im2])
        beam = concat.restoringbeam()
        self.assertTrue(qa.eq(qa.quantity(beam["major"]), major))
        self.assertTrue(qa.eq(qa.quantity(beam["minor"]), minor))
        self.assertTrue(qa.eq(qa.quantity(beam["positionangle"]), pa))
        

def suite():
    return [ia_imageconcat_test]
