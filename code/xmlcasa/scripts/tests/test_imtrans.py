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
# Test suite for the CASA task imtrans
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
#   <li> <linkto class="task_imtrans.py:description">imtrans</linkto> 
# </ul>
# </prerequisite>
#
# <etymology>
# Test for the imtrans task
# </etymology>
#
# <synopsis>
# Test the imtrans task and the ia.reorder() method upon which it is built.
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_imtrans[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the imtrans task to ensure
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

good_image = "reorder_in.fits"

def run_reorder(imagename, outfile, order):
    ia.open(imagename)
    res = ia.reorder(outfile=outfile, order=order)
    ia.done()
    return res

def run_imtrans(imagename, outfile, order):
    return imtrans(imagename=imagename, outfile=outfile, order=order)


class imtrans_test(unittest.TestCase):
    
    def setUp(self):
        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/imtrans/'
        shutil.copy(datapath + good_image, good_image)
    
    def tearDown(self):
        os.remove(good_image)

    def test_exceptions(self):
        """imtrans: Test various exception cases"""
        
        def testit(imagename, outfile, order):
            for i in [0,1]:
                if (i==0):
                    self.assertRaises(Exception, run_reorder, imagename, outfile, order)
                else:
                    self.assertFalse(run_imtrans(imagename, outfile, order))

        # blank imagename
        testit("", "blah", "012")
        
        # not enough specified axes
        testit(good_image, "blah", "01")
        
        # too many specified axes
        testit(good_image, "blah", "0123")
        
        # Bogus axes specification
        testit(good_image, "blah", "123")
        
    def test_straight_copy(self):
        """No actual transposing"""
        imagename = good_image
        outfile = "straight_copy"
        order = "012"
        ia.open(imagename)
        expecteddata = ia.getchunk()
        expectednames = ia.coordsys().names()
        ia.done()
        for i in [0,1]:
            if (i==0):
                newim = run_reorder(imagename, outfile + str(i), order)
            else:
                newim = run_imtrans(imagename, outfile + str(i), order)
            gotdata = newim.getchunk()
            gotnames = newim.coordsys().names()
            self.assertTrue((expecteddata == gotdata).all())
            self.assertTrue(expectednames == gotnames)

    def test_transpose(self):
        """Test transposing"""
        imagename = good_image
        outfile = "transpose"
        order = "120"
        ia.open(imagename)
        expecteddata = ia.getchunk()
        expectednames = ia.coordsys().names()
        ia.done()
        for i in [0,1]:
            if (i==0):
                newim = run_reorder(imagename, outfile + str(i), order)
            else:
                newim = run_imtrans(imagename, outfile + str(i), order)
            gotdata = newim.getchunk()
            inshape = expecteddata.shape
            for i in range(inshape[0]):
                for j in range(inshape[1]):
                    for k in range(inshape[2]):
                        self.assertTrue(expecteddata[i][j][k] == gotdata[j][k][i])
            gotnames = newim.coordsys().names()
            self.assertTrue(expectednames[0] == gotnames[2])
            self.assertTrue(expectednames[1] == gotnames[0])
            self.assertTrue(expectednames[2] == gotnames[1])

def suite():
    return [imtrans_test]