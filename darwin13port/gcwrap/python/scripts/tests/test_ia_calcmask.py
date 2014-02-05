#############################################
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
# Test suite for the CASA method ia.calcmask()
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
# Test for the ia.calcmask() method
# </etymology>
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_ia_calcmask[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the ia.calcmask() task to ensure
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

class ia_calcmask_test(unittest.TestCase):
    
    def setUp(self):
       pass
    def tearDown(self):
        self.assertTrue(len(tb.showcache()) == 0)

    def test_basic(self):
        """Test basic functionality of ia.calcmask()"""
        myia = iatool()
        im1 = "myfloatmask.im"
        myia.fromshape(im1, [2, 2], type='f')
        bb = myia.getchunk()
        bb[0, 0] = 1
        myia.putchunk(bb)
        myia.done()
        im2 = "myfloat.im"
        myia.fromshape(im2, [2, 2], type='f')
        myia.calcmask(im1 + "<= 0")
        mask = myia.getchunk(getmask = True)
        myia.done()
        for i in [0, 1]:
            for j in [0, 1]:
                if i == 0 and j == 0:
                    self.assertFalse(mask[i, j])
                else:
                    self.assertTrue(mask[i, j])
                    
        im1 = "mycomplexmask.im"
        myia.fromshape(im1, [2, 2], type='c')
        bb = myia.getchunk()
        bb[0, 0] = 1 + 1j
        myia.putchunk(bb)
        myia.done()
        im2 = "mycomplex.im"
        myia.fromshape(im2, [2, 2], type='c')
        myia.calcmask("real(" + im1 + ") <= 0")
        mask = myia.getchunk(getmask = True)
        myia.done()
        for i in [0, 1]:
            for j in [0, 1]:
                if i == 0 and j == 0:
                    self.assertFalse(mask[i, j])
                else:
                    self.assertTrue(mask[i, j])
        
        
        

def suite():
    return [ia_calcmask_test]
