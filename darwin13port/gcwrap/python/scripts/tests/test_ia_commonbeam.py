###########################################################################
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
# Test suite for the CASA task ia.commonbeam()
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
#   <li> <linkto class="task_imcollapse.py:description">ia.commonbeam()</linkto> 
# </ul>
# </prerequisite>
#
# <etymology>
# Test for the ia.commonbeam() method
# </etymology>
#
# <synopsis>
# Test the ia.commonbeam() method.
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_ia_commonbeam[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the ia.commonbeam() method to ensure
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





class ia_commonbeam_test(unittest.TestCase):
    
    def setUp(self):
        pass
    
    def tearDown(self):
        pass
    
    def test_nobeam(self):
        """ test having no beam throws an exception"""
        ia.fromshape("", [2,2,2])
        self.assertRaises(Exception, ia.commonbeam)
        ia.done()
        
    def test_enclosingbeam(self):
        """ test case where one beam in the set encloses the others"""
        ia.fromshape("", [2,2,2])
        major = {'value': 4.0, 'unit': 'arcsec'}
        minor = {'value': 2.0, 'unit': 'arcsec'}
        pa = {'value': 0.0, 'unit': 'deg'}
        ia.setrestoringbeam(major=major, minor=minor, pa=pa, polarization=0)
        ia.setrestoringbeam(major="1arcsec", minor="1arcsec", pa="0deg", polarization=1)
        x = ia.commonbeam()
        print str(x)
        self.assertTrue(x['major'] == major)
        self.assertTrue(x['minor'] == minor)
        self.assertTrue(x['pa'] == pa)
        ia.done()
        
    def test_onebeam(self):
        """ test global beam case"""
        ia.fromshape("", [2,2,2])
        major = {'value': 4.0, 'unit': 'arcsec'}
        minor = {'value': 2.0, 'unit': 'arcsec'}
        pa = {'value': 0.0, 'unit': 'deg'}
        ia.setrestoringbeam(major=major, minor=minor, pa=pa)
        x = ia.commonbeam()
        self.assertTrue(x['major'] == major)
        self.assertTrue(x['minor'] == minor)
        self.assertTrue(x['pa'] == pa)
        ia.done()
        
    def test_overlappingbeams(self):
        """ test case where one beam does not enclose the other"""
        ia.fromshape("", [2,2,2])
        major = {'value': 4.0, 'unit': 'arcsec'}
        minor = {'value': 2.0, 'unit': 'arcsec'}
        ia.setrestoringbeam(major=major, minor=minor, pa="0deg", polarization=0)
        ia.setrestoringbeam(major=major, minor=minor, pa="60deg", polarization=1)
        x = ia.commonbeam()
        print str(x)
        self.assertTrue(x['major']['value'] < 4.486)
        self.assertTrue(x['minor']['value'] < 3.292)
        self.assertTrue(abs(x['pa']['value'] - 30) < 1e-7 )
        ia.done()
        
        
def suite():
    return [ia_commonbeam_test]
