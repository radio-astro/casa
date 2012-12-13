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
# Test suite for the CASA task ia.isconform
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
#   <li> <linkto class="task_ia_isconform.py:description">ia.isconform</linkto> 
# </ul>
# </prerequisite>
#
# <etymology>
# Test for the ia.isconform method
# </etymology>
#
# <synopsis>
# Test the ia.isconform method.
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_ia_isconform[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the ia.isconform method to ensure
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

fits = "jj.fits"


class ia_isconform_test(unittest.TestCase):

    def setUp(self):
        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/ia_isconform/'
        shutil.copy(datapath + fits, fits)
        self._myia = iatool()
        self._myia.maketestimage()

    
    def tearDown(self):
        self._myia.done()
        del self._myia

    def test_unattached(self):
        self._myia.done()
        self.assertRaises(Exception, self._myia.isconform("x"))
        
    def test_trueness(self):
        self.assertTrue(self._myia.isconform(fits))
        
    def test_diffaxes(self):
        _newia = self._myia.adddegaxes(spectral=True)
        self.assertFalse(_newia.isconform(fits))
      
    def test_diffaxes(self):
        _cs = self._myia.coordsys()
        names = _cs.names()
        _cs.setnames([names[1], names[0]])
        self._myia.setcoordsys(_cs.torecord())
        self.assertFalse(self._myia.isconform(fits))
      
    def test_diffincrements(self):
        _cs = self._myia.coordsys()
        _cs.setincrement([0.1,0.1])
        self._myia.setcoordsys(_cs.torecord())
        self.assertFalse(self._myia.isconform(fits))
        
def suite():
    return [ia_isconform_test]
