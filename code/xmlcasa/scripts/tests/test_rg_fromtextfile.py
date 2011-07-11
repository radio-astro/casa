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

def deep_equality(a, b):
    print "types " + str(type(a)) + " " + str(type(b))
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
        print "string values " + str(a) + " " + str(b)
        return False
    return True
            
        
        



class rg_fromtextfile_test(unittest.TestCase):
    
    _fixtures = [image, text1, res1]
    
    def setUp(self):
        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/rg.fromtextfile/'
        for im in self._fixtures:
            shutil.copy(datapath + im, im)
    
    def tearDown(self):
        for im in self._fixtures:
            os.remove(im)

    def test_exceptions(self):
        """test exception cases"""

        myrg = rgtool.create()
        # bad file
        self.assertRaises(Exception, myrg.fromtextfile, "blah", {}, [1,1])
        # coordsys not set
        self.assertRaises(Exception, myrg.fromtextfile, text1, {}, [1,1])

    def test_read(self):
        """Read test"""
        myia = iatool.create()
        ia.open(image)
        csys = ia.coordsys().torecord()
        shape = ia.shape()
        myrg = rgtool.create()
        got = rg.fromtextfile(text1, shape, csys)
        expected = rg.fromfiletorecord(res1)
        expected['comment'] = ""
        self.assertTrue(deep_equality(got, expected))


        

def suite():
    return [rg_fromtextfile_test]
