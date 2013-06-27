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
# Test suite for the CASA tool method ia.subimage
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
#   <li> <linkto class="ia.subimage:description">ia.subimage</linkto> 
# </ul>
# </prerequisite>
#
# <etymology>
# Test for the ia subimage tool method
# </etymology>
#
# <synopsis>
# Test the ia.subimage tool method
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_ia_subimage[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the ia.subimage tool method to ensure
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


datapath = os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/imsubimage/'

class ia_subimage_test(unittest.TestCase):
    
    def setUp(self):
        self.myia = iatool()
    
    def tearDown(self):
        self.myia.done()
        # FIXME need to figure out why this table is left open when test_stretch throws
        # reasonable exception (CAS-4890)
        cache = tb.showcache()
        if len(cache) > 0:
            for t in cache:
                self.assertTrue(t.find("mask3.im") >= 0)
            

    def test_stretch(self):
        """Test the stretch parameter"""
        myia = self.myia
        myia.fromshape("mask1.im", [20, 30, 4, 10])
        myia.fromshape("mask2.im", [20, 30, 4, 1])
        myia.fromshape("mask3.im", [20, 30, 4, 2])
        myia.done()

        imname = "xx.im"
        myia.fromshape(imname, [20,30,4,10])
        mask1 = "mask1.im > 10"
        mm = myia.subimage("", mask=mask1)
        self.assertTrue(mm)
        mm.done()
        res = imsubimage(imagename=imname, outfile="stretch1", mask=mask1)
        self.assertTrue(res)
        myia.done()
        self.assertTrue(len(tb.showcache()) == 0)
        mask2 = "mask2.im > 10"
        self.assertRaises(Exception, myia.subimage, "", mask=mask2, stretch=False)
        self.assertFalse(imsubimage(imname, "stretch4", mask=mask2, stretch=False))
        myia.open(imname)
        mm = myia.subimage("", mask=mask2, stretch=True)
        myia.done()
        print "*** mychache 1 ****************************" + str(tb.showcache())
        mm.done()
        print "*** mychache 2 *****************************" + str(tb.showcache())
        self.assertTrue(len(tb.showcache()) == 0)

        self.assertTrue(imsubimage(imname, outfile="stretch2", mask=mask2, stretch=True))
        mask3 = "mask3.im > 10"
        zz = None
        try:
            myia.open(imname)
            zz = myia.subimage("", mask=mask3, stretch=True)
            zz.done()
            self.asertTrue(False)
        except:
            pass
#        self.assertRaises(Exception, myia.subimage, "", mask=mask3, stretch=True)
        myia.done()
        self.assertFalse(imsubimage(imname, "", mask=mask3, stretch=True))

    def test_beams(self):
        """ Test per plane beams """
        myia = self.myia

        # simple copy
        myia.fromshape("", [10, 10, 10, 4])
        myia.setrestoringbeam(
            "4arcsec", "2arcsec", "5deg", channel=0, polarization=0
        )
        for i in range(10):
            for j in range(4):
                myia.setrestoringbeam(
                    qa.quantity(i + j + 2, "arcsec"),
                    qa.quantity(i + j + 1, "arcsec"),
                    qa.quantity("5deg"),
                    channel=i, polarization=j
                )
        box = rg.box([2, 2, 2, 2], [5, 5, 5, 3])
        subim = myia.subimage("", region=box)
        for i in range(subim.shape()[2]):
            for j in range(subim.shape()[3]):
                self.assertTrue(
                    subim.restoringbeam(channel=i, polarization=j)
                    == myia.restoringbeam(channel=i+2, polarization=j+2)
                )
        box = rg.box([2, 2, 2, 2], [5, 5, 5, 2])
        subim = myia.subimage("", region=box, dropdeg=T)
        for i in range(subim.shape()[2]):
            self.assertTrue(
                subim.restoringbeam(channel=i, polarization=-1)
                == myia.restoringbeam(channel=i+2, polarization=2)
            )
        box = rg.box([2, 2, 6, 1], [5, 5, 6, 3])
        subim = myia.subimage("", region=box, dropdeg=T)
        for i in range(subim.shape()[2]):
            self.assertTrue(
                subim.restoringbeam(channel=-1, polarization=i)
                == myia.restoringbeam(channel=6, polarization=i+1)
            )
        subim.done()
        myia.done()
        
        # CAS-5282
        
        imagename = datapath + "50beams.im"
        outfile = "test_beams1.im"
        imsubimage(
            imagename=imagename, outfile=outfile, box="",
            region="", chans="18~29",stokes="I",mask="",
            dropdeg=False,overwrite=None, verbose=True,
            stretch=False
        )
        myia.open(outfile)
        beams = myia.restoringbeam()
        self.assertTrue(len(beams['beams']) == 12)
        


def suite():
    return [ia_subimage_test]
