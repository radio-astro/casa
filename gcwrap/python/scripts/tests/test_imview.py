##########################################################################
# test_imview.py
#
# Copyright (C) 2017
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
# Darrell Schiebel
# </author>
#
# <summary>
# Minimal test of CASA viewer
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
# </etymology>
#
# <synopsis>
# </synopsis> 
#
# <example>
# </example>
#
# <motivation>
# Because the viewer has no way to specify specific dimensions and because the size
# of the bitmaps that it produces are dependent on the dimensions of the viewer, it
# is difficult to create good tests to verify that the viewer works properly. As a
# result, this test only verifies that it produces the expected file.
# </motivation>
#

###########################################################################
import os
import shutil
import os.path
import unittest
from taskinit import *
from tasks import imview

class imview_test(unittest.TestCase):
    
    @classmethod
    def setUpClass(self):
        """creating test images"""
        self.linim = "lin"+str(os.getpid())+".im"
        self.outfiles = { }
        for t in ['jpg', 'pdf', 'eps', 'ps', 'png', 'xbm', 'xpm', 'ppm']:
            self.outfiles[t] = "lin"+str(os.getpid())+"."+t
        cs = cstool( )
        ia = iatool( )
        cs.addcoordinate(linear=2)
        ia.fromshape(self.linim,[20,20],csys=cs.torecord( ))
        xx = ia.getchunk( )
        xx[9:19,:] = 1
        ia.putchunk(xx)
        ia.done( )
        cs.done( )

    #@classmethod
    #def tearDownClass(self):
    #    """removing test images"""
    #    shutil.rmtree(self.linim)
        
    def test_xbm(self):
        """Test production of Xbm file"""
        imview(self.linim,out=self.outfiles['xbm'])
        self.assertTrue(os.path.isfile(self.outfiles['xbm']),"viewer failed to produce an Xbm file")

    def test_jpg(self):
        """Test production of JPEG file"""
        imview(self.linim,out=self.outfiles['jpg'])
        self.assertTrue(os.path.isfile(self.outfiles['jpg']),"viewer failed to produce an JPEG file")

    def test_pdf(self):
        """Test production of PDF file"""
        imview(self.linim,out=self.outfiles['pdf'])
        self.assertTrue(os.path.isfile(self.outfiles['pdf']),"viewer failed to produce an PDF file")

    def test_eps(self):
        """Test production of EPS file"""
        imview(self.linim,out=self.outfiles['eps'])
        self.assertTrue(os.path.isfile(self.outfiles['eps']),"viewer failed to produce an EPS file")

    def test_ps(self):
        """Test production of PS file"""
        imview(self.linim,out=self.outfiles['ps'])
        self.assertTrue(os.path.isfile(self.outfiles['ps']),"viewer failed to produce an PS file")

    def test_xpm(self):
        """Test production of XPM file"""
        imview(self.linim,out=self.outfiles['xpm'])
        self.assertTrue(os.path.isfile(self.outfiles['xpm']),"viewer failed to produce an XPM file")

    def test_ppm(self):
        """Test production of PPM file"""
        imview(self.linim,out=self.outfiles['ppm'])
        self.assertTrue(os.path.isfile(self.outfiles['ppm']),"viewer failed to produce an PPM file")

    def test_png(self):
        """Test production of PNG file"""
        imview(self.linim,out=self.outfiles['png'])
        self.assertTrue(os.path.isfile(self.outfiles['png']),"viewer failed to produce an PNG file")

def suite():
    return [imview_test]
