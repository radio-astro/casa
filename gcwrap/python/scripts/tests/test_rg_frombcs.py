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
# Test suite for the CASA method rg.frombcs
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
#   <li> <linkto class="task_rg_frombcs.py:description">imtrans</linkto> 
# </ul>
# </prerequisite>
#
# <etymology>
# Test for the rg.frombcs method
# </etymology>
#
# <synopsis>
# Test the rg.frombcs method
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_rg_frombcs[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the rg.frombcs method to ensure
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

image = "imregion.fits"
image_nospec = "imregion_nospec.fits"
image_dironly = "imregion_dironly.fits"

box1 = 1.24795026
box2 = 0.782552901
box3 = 1.24794616
box4 = 0.782555814
box5 = 1.24794206
box6 = 0.782558727
box7 = 1.24793797
box8 = 0.782561641
box9 = 1.2479338718038551
box10 = 0.78256455381109313
box11 = 1.2479297756405987
box12 = 0.78256746696533663
chan0 = 4.73510000e+09
chan4 = 6.33510000e+09
chan15 = 1.07351000e+10
chan19 = 1.23351000e+10

def run_frombcs(imagename, box, chans, stokes, stokes_control, region=""):
    myia = iatool()
    myia.open(imagename)
    mycsys = myia.coordsys()
    myrg = rgtool()
    res = rg.frombcs(
        mycsys.torecord(), myia.shape(), box, chans,
        stokes, stokes_control, region
    )
    myia.close()
    del myia
    del myrg
    return res

def recToList(rec):
    mylist = []
    mykeys = rec.keys()
    mykeys.sort()
    for k in mykeys:
        mylist.append(rec[k]['value'])
    return mylist 

class rg_frombcs_test(unittest.TestCase):
    
    def setUp(self):
        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/rg.frombcs/'
        for im in [image, image_nospec, image_dironly]:
            shutil.copy(datapath + im, im)
    
    def tearDown(self):
        for im in [image, image_nospec, image_dironly]:
            os.remove(im)

    def compLists(self, got, exp):
        epsilon = 1e-8
        print "got " + str(got)
        print "exp " + str(exp)
        for i in range(len(got)):
            fracDiff = abs((got[i]-exp[i])/exp[i]);
            self.assertTrue(fracDiff < epsilon)

    def test_full_image(self):
        """Test default gives region of entire image"""

        stokes = ""
        chans = ""
        stokes_control = "a"
        box = ""
        myreg = run_frombcs(
            image, box, chans, stokes, stokes_control
        )
        gotblc = recToList(myreg["blc"])
        expblc = [1.24795230, 0.782549990, chan0, 1.0]
        self.compLists(gotblc, expblc);
        gottrc = recToList(myreg["trc"])
        exptrc = [1.24791339, 0.782577665, chan19, 4.0]
        self.compLists(gottrc, exptrc)
        
    def test_single_stokes(self):
        """Test setting a single stokes"""

        stokes = "Q"
        chans = ""
        stokes_control = "a"
        box = ""
        myreg = run_frombcs(
            image, box, chans, stokes, stokes_control
        )
        gotblc = recToList(myreg["blc"])
        expblc = [1.24795230, 0.782549990, chan0, 2.0]
        self.compLists(gotblc, expblc);
        gottrc = recToList(myreg["trc"])
        exptrc = [1.24791339, 0.782577665, chan19, 2.0]

    def test_continguous_stokes(self):
        """Test setting a contiguous stokes"""
        
        box = ""
        chans = ""
        stokes = "QU"
        stokes_control = "a"
        myreg = run_frombcs(
            image, box, chans, stokes, stokes_control
        )
        gotblc = recToList(myreg["blc"])
        expblc = [1.24795230, 0.782549990, chan0, 2.0]
        self.compLists(gotblc, expblc);
        gottrc = recToList(myreg["trc"])
        exptrc = [1.24791339, 0.782577665, chan19, 3.0]

    def test_single_channel(self):
        """Test setting a single channel"""
        box = ""
        chans = "4"
        stokes = ""
        stokes_control = "a"
        myreg = run_frombcs(
            image, box, chans, stokes, stokes_control
        )
        gotblc = recToList(myreg["blc"])
        expblc = [1.24795230, 0.782549990, chan4, 1.0]
        self.compLists(gotblc, expblc);
        gottrc = recToList(myreg["trc"])
        exptrc = [1.24791339, 0.782577665, chan4, 4.0]
        
    def test_contiguous_channels(self):
        """Test setting multiple continuous channels"""
        
        box = ""
        chans = "0~4"
        stokes = ""
        stokes_control = "a"
        myreg = run_frombcs(
            image, box, chans, stokes, stokes_control
        )
        gotblc = recToList(myreg["blc"])
        expblc = [1.24795230, 0.782549990, chan0, 1.0]
        self.compLists(gotblc, expblc);
        gottrc = recToList(myreg["trc"])
        exptrc = [1.24791339, 0.782577665, chan4, 4.0]

    def test_single_box(self):
        """Test setting single box"""
        
        box = "1,2,3,4"
        chans = ""
        stokes = ""
        stokes_control = "a"
        myreg = run_frombcs(
            image, box, chans, stokes, stokes_control
        )
        gotblc = recToList(myreg["blc"])
        expblc = [box1, box2, chan0, 1.0]
        self.compLists(gotblc, expblc);
        gottrc = recToList(myreg["trc"])
        exptrc = [box3, box4, chan19, 4.0]
        
    def test_region_record(self):
        """Test setting region record"""
        
        box = ""
        chans = ""
        stokes = ""
        stokes_control = "a"
        blahia = iatool()
        blahia.open(image)
        mycsys = blahia.coordsys()
        blahia.done()
        mybox = rg.wbox(
            ["1pix", "2pix", "0pix", "0pix"],
            ["3pix", "4pix", "19pix", "3pix"],
            csys=mycsys.torecord()
        )
        myreg = run_frombcs(
            image, box, chans, stokes, stokes_control, mybox
        )
        gotblc = recToList(myreg["blc"])
        for i in range(len(gotblc)):
            gotblc[i] = gotblc[i] - 1 
        gotblc = mycsys.toworld(gotblc)['numeric']
        expblc = [box1, box2, chan0, 1.0]
        self.compLists(gotblc, expblc);
        gottrc = recToList(myreg["trc"])
        for i in range(len(gottrc)):
            gottrc[i] = gottrc[i] - 1 
        gottrc = mycsys.toworld(gottrc)['numeric']
        exptrc = [box3, box4, chan19, 4.0]
        self.compLists(gottrc, exptrc);

        
    def test_first_stokes(self):
        """Test setting first stokes"""
        
        box = ""
        chans = ""
        stokes = ""
        stokes_control = "f"
        myreg = run_frombcs(
            image, box, chans, stokes, stokes_control
        )
        gotblc = recToList(myreg["blc"])
        expblc = [1.24795230, 0.782549990, chan0, 1.0]
        self.compLists(gotblc, expblc);
        gottrc = recToList(myreg["trc"])
        exptrc = [1.24791339, 0.782577665, chan19, 1.0]
        
        
    def test_multiple_boxes(self):
        """Test setting multiple boxes"""
        
        stokes = ""
        chans = ""
        stokesControl = "a"
        box = "1,2,3,4,5,6,7,8,9,10,11,12"
        stokes_control = "a"
        myreg = run_frombcs(
            image, box, chans, stokes, stokes_control
        )

        gotblc = recToList(myreg["regions"]["*2"]["blc"])
        expblc = [1.24793387, 0.782564554, chan0, 1.0]
        self.compLists(gotblc, expblc);
        gottrc = recToList(myreg["regions"]["*2"]["trc"])
        exptrc = [1.24792978, 0.782567467, chan19, 4.0]
        self.compLists(gottrc, exptrc)

        gotblc = recToList(myreg["regions"]["*1"]["regions"]["*1"]["blc"])
        expblc = [box1, box2, chan0, 1.0]
        self.compLists(gotblc, expblc);
        gottrc = recToList(myreg["regions"]["*1"]["regions"]["*1"]["trc"])
        exptrc = [box3, box4, chan19, 4.0]
        self.compLists(gottrc, exptrc);
         
        gotblc = recToList(myreg["regions"]["*1"]["regions"]["*2"]["blc"])
        expblc = [box5, box6, chan0, 1.0]
        self.compLists(gotblc, expblc);
        gottrc = recToList(myreg["regions"]["*1"]["regions"]["*2"]["trc"])
        exptrc = [box7, box8, chan19, 4.0]
        self.compLists(gottrc, exptrc);
        
    def test_set_multiple_stokes_ranges(self):
        """Test setting multiple stokes ranges"""
        
        stokes = "IUV"
        chans = ""
        stokesControl = "a"
        box = ""
        stokes_control = "a"
        myreg = run_frombcs(
            image, box, chans, stokes, stokes_control
        )

        gotblc = recToList(myreg["regions"]["*1"]["blc"])
        expblc = [1.24795230, 0.782549990, chan0, 1.0]
        self.compLists(gotblc, expblc);
        gottrc = recToList(myreg["regions"]["*1"]["trc"])
        exptrc = [1.24791339, 0.782577665, chan19, 1.0]
        self.compLists(gottrc, exptrc)
        
        gotblc = recToList(myreg["regions"]["*2"]["blc"])
        expblc = [1.24795230, 0.782549990, chan0, 3.0]
        self.compLists(gotblc, expblc);
        gottrc = recToList(myreg["regions"]["*2"]["trc"])
        exptrc = [1.24791339, 0.782577665, chan19, 4.0]
        self.compLists(gottrc, exptrc)
        
    def test_multiple_channel_ranges(self):
        """Test multiple channel ranges"""
        
        stokes = ""
        chans = "<5,>=15"
        stokesControl = "a"
        box = ""
        stokes_control = "a"
        myreg = run_frombcs(
            image, box, chans, stokes, stokes_control
        )

        gotblc = recToList(myreg["regions"]["*1"]["blc"])
        expblc = [1.24795230, 0.782549990, chan0, 1.0]
        self.compLists(gotblc, expblc);
        gottrc = recToList(myreg["regions"]["*1"]["trc"])
        exptrc = [1.24791339, 0.782577665, chan4, 4.0]
        self.compLists(gottrc, exptrc)
        
        gotblc = recToList(myreg["regions"]["*2"]["blc"])
        expblc = [1.24795230, 0.782549990, chan15, 1.0]
        self.compLists(gotblc, expblc);
        gottrc = recToList(myreg["regions"]["*2"]["trc"])
        exptrc = [1.24791339, 0.782577665, chan19, 4.0]
        self.compLists(gottrc, exptrc)

    def test_multiple_boxes_channel_ranges_stokes_ranges(self):
        """Test multiple channel ranges, multiple stokes ranges, and multiple boxes"""
        
        stokes = "IQV"
        chans = "<5,>=15"
        stokesControl = "a"
        box = "1,2,3,4,5,6,7,8"
        stokes_control = "a"
        myreg = run_frombcs(
            image, box, chans, stokes, stokes_control
        )

        # box="5,6,7,8", chans="15~19", stokes="V"
        gotblc = recToList(myreg["regions"]["*2"]["blc"])
        expblc = [box5, box6, chan15, 4.0]
        self.compLists(gotblc, expblc)
        gottrc = recToList(myreg["regions"]["*2"]["trc"])
        exptrc = [box7, box8, chan19, 4.0]
        self.compLists(gottrc, exptrc)

        # box="5,6,7,8", chans="0~4", stokes="V"
        gotblc = recToList(myreg["regions"]["*1"]["regions"]["*2"]["blc"])
        expblc = [box5, box6, chan0, 4.0]
        self.compLists(gotblc, expblc)
        gottrc = recToList(myreg["regions"]["*1"]["regions"]["*2"]["trc"])
        exptrc = [box7, box8, chan4, 4.0]
        self.compLists(gottrc, exptrc)

        # box="5,6,7,8", chans="15-19", stokes="IQ" 
        gotblc = recToList(myreg["regions"]["*1"]["regions"]["*1"]["regions"]["*2"]["blc"])
        expblc = [box5, box6, chan15, 1.0]
        self.compLists(gotblc, expblc)
        gottrc = recToList(myreg["regions"]["*1"]["regions"]["*1"]["regions"]["*2"]["trc"])
        exptrc = [box7, box8, chan19, 2.0]
        self.compLists(gottrc, exptrc)

        # box="5,6,7,8", chans="0~4", stokes="IQ" 
        gotblc = recToList(myreg["regions"]["*1"]["regions"]["*1"]["regions"]["*1"]["regions"]["*2"]["blc"])
        expblc = [box5, box6, chan0, 1.0]
        self.compLists(gotblc, expblc)
        gottrc = recToList(myreg["regions"]["*1"]["regions"]["*1"]["regions"]["*1"]["regions"]["*2"]["trc"])
        exptrc = [box7, box8, chan4, 2.0]
        self.compLists(gottrc, exptrc)

        # box="1,2,3,4", chans="15-19", stokes="V" 
        gotblc = recToList(
            myreg["regions"]["*1"]["regions"]["*1"]["regions"]["*1"]["regions"]["*1"]
            ["regions"]["*2"]["blc"]
        )
        expblc = [box1, box2, chan15, 4.0]
        self.compLists(gotblc, expblc)
        gottrc = recToList(
            myreg["regions"]["*1"]["regions"]["*1"]["regions"]["*1"]["regions"]["*1"]
            ["regions"]["*2"]["trc"]
        )
        exptrc = [box3, box4, chan19, 4.0]
        self.compLists(gottrc, exptrc)

        # box="1,2,3,4", chans="0-4", stokes="V" 
        gotblc = recToList(
            myreg["regions"]["*1"]["regions"]["*1"]["regions"]["*1"]["regions"]["*1"]
                ["regions"]["*1"]["regions"]["*2"]["blc"]
        )
        expblc = [box1, box2, chan0, 4.0]
        self.compLists(gotblc, expblc)
        gottrc = recToList(
            myreg["regions"]["*1"]["regions"]["*1"]["regions"]["*1"]["regions"]["*1"]
                ["regions"]["*1"]["regions"]["*2"]["trc"]
        )
        exptrc = [box3, box4, chan4, 4.0]
        self.compLists(gottrc, exptrc)

        # box="1,2,3,4", chans="15-19", stokes="IQ" 
        gotblc = recToList(
            myreg["regions"]["*1"]["regions"]["*1"]["regions"]["*1"]["regions"]["*1"]
                ["regions"]["*1"]["regions"]["*1"]["regions"]["*2"]["blc"]
        )
        expblc = [box1, box2, chan15, 1.0]
        self.compLists(gotblc, expblc)
        gottrc = recToList(
            myreg["regions"]["*1"]["regions"]["*1"]["regions"]["*1"]["regions"]["*1"]
                ["regions"]["*1"]["regions"]["*1"]["regions"]["*2"]["trc"]
        )
        exptrc = [box3, box4, chan19, 2.0]
        self.compLists(gottrc, exptrc)

         # box="1,2,3,4", chans="0~4", stokes="IQ" 
        gotblc = recToList(
            myreg["regions"]["*1"]["regions"]["*1"]["regions"]["*1"]["regions"]["*1"]
                ["regions"]["*1"]["regions"]["*1"]["regions"]["*1"]["blc"]
        )
        expblc = [box1, box2, chan0, 1.0]
        self.compLists(gotblc, expblc)
        gottrc = recToList(
            myreg["regions"]["*1"]["regions"]["*1"]["regions"]["*1"]["regions"]["*1"]
                ["regions"]["*1"]["regions"]["*1"]["regions"]["*1"]["trc"]
        )
        exptrc = [box3, box4, chan4, 2.0]
        self.compLists(gottrc, exptrc)

    def test_multiple_boxes_multiple_stokes_no_spectral_axis(self):
        """Test multiple stokes ranges, and multiple boxes on image with no spectral axis"""

        stokes = "IQV"
        chans = ""
        stokesControl = "a"
        box = "1,2,3,4,5,6,7,8"
        stokes_control = "a"
        myreg = run_frombcs(
            image_nospec, box, chans, stokes, stokes_control
        )

        # box="5,6,7,8", stokes="V"
        gotblc = recToList(myreg["regions"]["*2"]["blc"])
        expblc = [box5, box6, 4.0]
        self.compLists(gotblc, expblc)
        gottrc = recToList(myreg["regions"]["*2"]["trc"])
        exptrc = [box7, box8, 4.0]
        self.compLists(gottrc, exptrc)

        # box="5,6,7,8", stokes="IQ"
        gotblc = recToList(myreg["regions"]["*1"]["regions"]["*2"]["blc"])
        expblc = [box5, box6, 1.0]
        self.compLists(gotblc, expblc)
        gottrc = recToList(myreg["regions"]["*1"]["regions"]["*2"]["trc"])
        exptrc = [box7, box8, 2.0]
        self.compLists(gottrc, exptrc)

        # box="1,2,3,4", stokes="V"
        gotblc = recToList(myreg["regions"]["*1"]["regions"]["*1"]["regions"]["*2"]["blc"])
        expblc = [box1, box2, 4.0]
        self.compLists(gotblc, expblc)
        gottrc = recToList(myreg["regions"]["*1"]["regions"]["*1"]["regions"]["*2"]["trc"])
        exptrc = [box3, box4, 4.0]
        self.compLists(gottrc, exptrc)

        # box="1,2,3,4", stokes="IQ"
        gotblc = recToList(myreg["regions"]["*1"]["regions"]["*1"]["regions"]["*1"]["blc"])
        expblc = [box1, box2, 1.0]
        self.compLists(gotblc, expblc)
        gottrc = recToList(myreg["regions"]["*1"]["regions"]["*1"]["regions"]["*1"]["trc"])
        exptrc = [box3, box4, 2.0]
        self.compLists(gottrc, exptrc)

    def test_multiple_boxes_image_with_direction_coordinate_only(self):
        """Test multiple boxes on image with direction coordinate only"""                        

        stokes = ""
        chans = ""
        stokesControl = "a"
        box = "1,2,3,4,5,6,7,8"
        stokes_control = "a"
        myreg = run_frombcs(
            image_dironly, box, chans, stokes, stokes_control
        )

        # box="5,6,7,8", stokes="V"
        gotblc = recToList(myreg["regions"]["*2"]["blc"])
        expblc = [box5, box6]
        self.compLists(gotblc, expblc)
        gottrc = recToList(myreg["regions"]["*2"]["trc"])
        exptrc = [box7, box8]
        self.compLists(gottrc, exptrc)
 
        # box="1,2,3,4", stokes="IQ"
        gotblc = recToList(myreg["regions"]["*1"]["blc"])
        expblc = [box1, box2]
        self.compLists(gotblc, expblc)
        gottrc = recToList(myreg["regions"]["*1"]["trc"])
        exptrc = [box3, box4]
        self.compLists(gottrc, exptrc)

    def test_region_text_string(self):
        """Test setting a region text string"""
        
        region = "box[[1pix,2pix],[3pix,4pix]]\nbox[[5pix,6pix],[7pix,8pix]]\nbox[[9pix,10pix],[11pix,12pix]]"
        myreg = run_frombcs(
            image, "", "", "", "a", region
        )
        myia = iatool()
        myia.open(image)
        subi = myia.subimage("xxyy.im", region=myreg)
        myia.done()
        self.assertTrue((subi.shape()[0:2] == [11, 11]).all())
        got = subi.toworld([0,0])['numeric'][0:2]
        expec = [box1, box2]
        self.compLists(got, expec)
        expec = [box11, box12]
        got = subi.toworld([10,10])['numeric'][0:2]
        self.compLists(got, expec)
        gotmask = subi.getchunk(getmask=T)[:,:,0,0]
        subi.done()
        expmask = gotmask.copy()
        expmask[:] = False
        expmask[0:3, 0:3] = True
        expmask[4:7, 4:7] = True
        expmask[8:11, 8:11] = True
        self.assertTrue((gotmask == expmask).all())

def suite():
    return [rg_frombcs_test]
