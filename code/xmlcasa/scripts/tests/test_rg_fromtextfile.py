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
        return False
    if (type(a) == dict):
        if (a.keys() != b.keys()):
            return False
        for k in a.keys():
            return deep_equality(a[k], b[k])
    if (type(a) == float):
        if(abs((a-b)/a > 1e-6)):
            return False
    if (type(a) == numpy.ndarray):
        if (a.shape() != b.shape()):
            return False
        x = a.tolist()
        y = b.tolist()
        for i in range(len(x)):
            return deep_equality(x[i], y[i])
    if (a != b):
        return False
    return True

class rg_fromtextfile_test(unittest.TestCase):
    
    _fixtures = [
        image, text1, res1, cas_3258t, cas_3258r, cas_3259t, cas_3259r,
        cas_3260t, cas_3260r
    ]
    
    def setUp(self):
        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/rg.fromtextfile/'
        for im in self._fixtures:
            shutil.copy(datapath + im, im)
        self.ia = iatool.create()
        self.rg = rgtool.create()
    
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
        self.ia.maketestimage()
        self._testit(cas_3258t, cas_3258r)
        
    def test_CAS_3259(self):
        """Verify fix to CAS-3259"""
        self.ia.maketestimage()
        self._testit(cas_3259t, cas_3259r)
        
    def test_CAS_3260(self):
        """Verify fix to CAS-3260"""
        self.ia.maketestimage()
        self._testit(cas_3260t, cas_3260r)

def suite():
    return [rg_fromtextfile_test]
