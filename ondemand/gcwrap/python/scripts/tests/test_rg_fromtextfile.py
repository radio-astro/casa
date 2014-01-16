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
# Test suite for the CASA method rg.fromtextfile
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
#   <li> <linkto class="task_rg_fromtextfile.py:description">rg.fromtextfile</linkto> 
# </ul>
# </prerequisite>
#
# <etymology>
# Test for the rg.fromtextile method
# </etymology>
#
# <synopsis>
# Test the rg.fromtextfile method
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_rg_fromtextfile[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the rg.fromtextfile method to ensure
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

image = "imregion.fits"
text1 = "goodfile1.txt"
res1 = "res1.rgn"
cas_3258t = "CAS-3258.txt"
cas_3258r = "CAS-3258.rgn"
cas_3259t = "CAS-3259.txt"
cas_3259r = "CAS-3259.rgn"
cas_3260t = "CAS-3260.txt"
cas_3260r = "CAS-3260.rgn"

def deep_equality(a, b):
    if (type(a) != type(b)):
        print "types don't match, a is a " + str(type(a)) + " b is a " + str(type(b))
        return False
    if (type(a) == dict):
        if (a.keys() != b.keys()):
            print "keys don't match, a is " + str(a.keys()) + " b is " + str(b.keys())
            return False
        for k in a.keys():
            if (
                k == "telescope" or k == "observer"
                or k == "telescopeposition"
            ):
                continue
            elif (not deep_equality(a[k], b[k])):
                print "dictionary member inequality a[" + str(k) \
                    + "] is " + str(a[k]) + " b[" + str(k) + "] is " + str(b[k])
                return False
        return True
    if (type(a) == float):
        if not (a == b or abs((a-b)/a) <= 1e-6):
            print "float mismatch, a is " + str(a) + ", b is " + str(b)
        return a == b or abs((a-b)/a) <= 1e-6
    if (type(a) == numpy.ndarray):
        if (a.shape != b.shape):
            print "shape mismatch a is " + str(a.shape) + " b is " + str(b.shape)
            return False
        x = a.tolist()
        y = b.tolist()
        for i in range(len(x)):
            if (not deep_equality(x[i], y[i])):
                print "array element mismatch, x is " + str(x[i]) + " y is " + str(y[i])
                return False
        return True
    return a == b

datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/rg.fromtextfile/'


class rg_fromtextfile_test(unittest.TestCase):
    
    _fixtures = [
        image, text1, res1, cas_3258t, cas_3258r, cas_3259t, cas_3259r,
        cas_3260t, cas_3260r
    ]
    
    def setUp(self):
        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/rg.fromtextfile/'
        for im in self._fixtures:
            shutil.copy(datapath + im, im)
        self.ia = iatool()
        self.rg = rgtool()
    
    def tearDown(self):
        for im in self._fixtures:
            os.remove(im)
        self.ia.done()
        del self.ia
        self.rg.done()
        del self.rg

    def _testit(self, text, rgn):
        csys = self.ia.coordsys().torecord()
        shape = self.ia.shape()
        got = self.rg.fromtextfile(text, shape, csys)
        expected = self.rg.fromfiletorecord(rgn)
        expected['comment'] = ""
        self.assertTrue(deep_equality(got, expected))
        
        f = open(text, 'r')
        text = f.read()
        got = self.rg.fromtext(text, shape, csys)
        self.assertTrue(deep_equality(got, expected))

    def test_exceptions(self):
        """test exception cases"""

        # bad file
        self.assertRaises(Exception, self.rg.fromtextfile, "blah", {}, [1,1])
        # coordsys not set
        self.assertRaises(Exception, self.rg.fromtextfile, text1, {}, [1,1])

    def test_read(self):
        """Read test"""
        self.ia.open(image)
        self._testit(text1, res1)
        
    def test_CAS_3258(self):
        """Verify fix to CAS-3258"""
        self.ia.fromshape("", [250,250])
        self._testit(cas_3258t, cas_3258r)
        
    def test_CAS_3259(self):
        """Verify fix to CAS-3259"""
        self.ia.fromshape("", [250,250])
        self._testit(cas_3259t, cas_3259r)
        
    def test_CAS_3260(self):
        """Verify fix to CAS-3260"""
        self.ia.fromshape("", [250,250])
        self._testit(cas_3260t, cas_3260r)
        
    def test_CAS_4415(self):
        """Verify CAS-4415 (parser did not properly handle frquency decreasing with pixel number)"""
        shape = [50, 50, 10]
        self.ia.fromshape("", shape)
        csys = self.ia.coordsys()
        increment = csys.increment()["numeric"]
        increment[2] = -increment[2]
        csys.setincrement(increment)
        self.ia.setcoordsys(csys.torecord())
        zz = rg.fromtext(
            "circle[[20pix,20pix],6pix],range=[1pix,3pix]",
            shape, csys.torecord()
        )
        self.assertTrue(len(zz.keys()) > 0)

    def test_CAS_4425(self):
        """ Verify CAS-4425 (pixel units now accounted for in range and no units throws exception)"""
        shape = [100, 100, 80]
        self.ia.fromshape("", shape)
        csys = self.ia.coordsys()
        zz = rg.fromtext("box[[30pix, 30pix], [39pix, 39pix]], range=[55pix,59pix]", shape, csys.torecord())
        self.assertTrue(self.ia.statistics(region=zz)["npts"] == 500)
        zz = rg.fromtext("box[[30pix, 30pix], [39pix, 39pix]], range=[59pix,55pix]", shape, csys.torecord())
        self.assertTrue(self.ia.statistics(region=zz)["npts"] == 500)
        self.assertRaises(
            Exception, rg.fromtext, "box[[30pix, 30pix], [39pix, 39pix]], range=[59,55]",
            shape, csys.torecord()
        )
         
    def test_rectangle_rotation(self):
        """Test rectangle region is preserved under coordinate frame switch"""
        self.ia.fromshape("",[200, 200])
        csys = self.ia.coordsys()
        # rectangular box
        xx = rg.fromtext(
            "box[[5834.23813221arcmin, -3676.92506701arcmin],[5729.75600494arcmin, -3545.36602909arcmin]] coord=GALACTIC",
            csys=csys.torecord(), shape=self.ia.shape()
        )
        zz = self.ia.subimage("", region=xx)
        got = zz.getchunk(getmask=True)
        self.ia.open(datapath + "rect_rot.im")
        expec = self.ia.getchunk(getmask=True)
        self.assertTrue((got == expec).all())
        zz.done()
        
        #center box
        self.ia.fromshape("",[200, 200])
        csys = self.ia.coordsys()
        yval = "-3611.1455480499999arcmin"
        xwidth = qa.tos(qa.mul(qa.quantity("104.48212727000009arcmin"),qa.cos(yval)))
        xx = rg.fromtext(
            "centerbox[[5781.9970685749995arcmin, " + yval + "],[" + xwidth + ", 131.55903791999981arcmin]] coord=GALACTIC",
            csys=csys.torecord(), shape=self.ia.shape()
        )
        zz = self.ia.subimage("", region=xx)
        got = zz.getchunk(getmask=True)
        self.ia.open(datapath + "rect_rot2.im")
        expec = self.ia.getchunk(getmask=True)
        self.assertTrue((got == expec).all())
        
        zz.done()
        self.ia.done()
        
    def test_rotbox(self):
        """Test rotbox when specified in pixels (CAS-5723)"""
        self.ia.fromshape("",[200,200])
        reg = rg.fromtext(
            "rotbox [ [ 60pix , 50pix ] , [ 30pix , 30pix ] , 30deg ]",
            csys=self.ia.coordsys().torecord(),shape=self.ia.shape()
        )
        self.assertTrue(self.ia.statistics(region=reg)['npts'] == 901)
        csys = self.ia.coordsys()
        csys.setreferencevalue([800,70*60])
        self.ia.setcoordsys(csys.torecord())
        reg = rg.fromtext(
            "rotbox [ [ 60pix , 50pix ] , [ 30pix , 30pix ] , 30deg ]",
            csys=self.ia.coordsys().torecord(),shape=self.ia.shape()
        )
        self.assertTrue(self.ia.statistics(region=reg)['npts'] == 901)

def suite():
    return [rg_fromtextfile_test]
