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
# Test suite for the CASA tool method ia.setrestoringbeam(), ia.restoringbeam()
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
# Test for the ia.restoringbeam() and ia.setrestoringbeam() tool methods
# </etymology>
#
# <synopsis>
# Test for the ia.restoringbeam() and ia.setrestoringbeam() tool methods
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_ia_restoringbeam[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the ia.restoringbeam and ia.setrestoringbeam() tool methods to ensure
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

class ia_restoringbeam_test(unittest.TestCase):
    
    def setUp(self):
        self._myia = iatool()
    
    def tearDown(self):
        self._myia.done()
    
    def test_global_beam(self):
        """Test adding, deleting, and setting beams"""
        myia = self._myia
        myia.fromshape(shape=[10, 10, 4, 10])
        self.assertFalse(bool(myia.restoringbeam()))
        major = "4arcsec"
        minor = "3arcsec"
        pa = "10deg"
        myia.setrestoringbeam(major=major, minor=minor, pa=pa)
        beam = myia.restoringbeam()
        self.assertTrue(beam["major"] == qa.quantity(major))
        self.assertTrue(beam["minor"] == qa.quantity(minor))
        self.assertTrue(beam["positionangle"] == qa.quantity(pa))
        for channel in [-1, 0, 1, 2]:
            for polarization in [-1, 0, 1, 2]:
                beam = myia.restoringbeam(channel=channel, polarization=polarization)
                self.assertTrue(beam["major"] == qa.quantity(major))
                self.assertTrue(beam["minor"] == qa.quantity(minor))
                self.assertTrue(beam["positionangle"] == qa.quantity(pa))
        myia.setrestoringbeam(remove=T)
        self.assertFalse(bool(myia.restoringbeam()))

    def test_per_plane_beams(self):
        myia = self._myia
        nchan = 10
        npol = 4
        for t in ['f', 'c']:
            myia.fromshape(shape=[10, 10, npol, nchan], type=t)
            self.assertFalse(bool(myia.restoringbeam()))
            major = "4arcsec"
            minor = "3arcsec"
            pa = "10deg"
            myia.setrestoringbeam(major=major, minor=minor, pa=pa, channel=20, polarization=2)
            nmajor = "10arcsec"
            nminor = "5arcsec"
            npa = "40deg"
            myia.setrestoringbeam(
                major=nmajor, minor=nminor, pa=npa,
                channel=2, polarization=1
            )
            beams = myia.restoringbeam()
            self.assertTrue(beams["nChannels"] == nchan)
            self.assertTrue(beams["nStokes"] == npol)
            self.assertTrue(len(beams["beams"]) == nchan)
            for chan in range(nchan):
                rec = beams["beams"]["*" + str(chan)]
                for pol in range(npol):
                    bmaj = major
                    bmin = minor
                    bpa = pa
                    if chan == 2 and pol == 1:
                        bmaj = nmajor
                        bmin = nminor
                        bpa = npa
                    beam = rec["*" + str(pol)]
                    self.assertTrue(beam["major"] == qa.quantity(bmaj))
                    self.assertTrue(beam["minor"] == qa.quantity(bmin))
                    self.assertTrue(beam["positionangle"] == qa.quantity(bpa))
                    beam = myia.restoringbeam(channel=chan, polarization=pol)
                    self.assertTrue(beam["major"] == qa.quantity(bmaj))
                    self.assertTrue(beam["minor"] == qa.quantity(bmin))
                    self.assertTrue(beam["positionangle"] == qa.quantity(bpa))
            for chan in [-1, 10]:
                for pol in [-1, 10]:
                    if chan != -1 or pol != -1:
                        self.assertRaises(
                            Exception, myia.restoringbeam,
                            channel=chan, polarization=pol
                        )
                        
    def test_copy_beams(self):
        """Test copy beamset option - CAS-5435"""
        myia = self._myia
        imagename = "source.im"
        nchan = 10
        nstokes = 4
        myia.fromshape(imagename, shape=[5,5, nchan, nstokes])
        myia.setrestoringbeam(major="4arcsec", minor="2arcsec", pa="0deg", channel=0, polarization=0)
        myia.setrestoringbeam(major="8arcsec", minor="4arcsec", pa="0deg", channel=2, polarization=2)
        myia.done()
        myia.fromshape("", shape=[10,10, nchan, nstokes])
        self.assertRaises(
            Exception, myia.setrestoringbeam,
            major="4arcsec", minor="2arcsec", pa="0deg",
            imagename=imagename
        )
        self.assertRaises(
            Exception, myia.setrestoringbeam,
            remove=True,
            imagename=imagename
        )
        myia.setrestoringbeam(imagename=imagename)
        self._compareBeams(myia, imagename)
        # test overwriting
        myia.setrestoringbeam(imagename=imagename)
        self._compareBeams(myia, imagename)
        myia.done()
        # swap axes
        myia.fromshape("", shape=[10,10, nstokes, nchan])
        myia.setrestoringbeam(imagename=imagename)
        self._compareBeams(myia, imagename)
        source = iatool()
        source.fromshape(
            imagename, shape=[5,5, nchan-1, nstokes-1],
            overwrite=True
        )
        source.done()
        # source has no beam
        self.assertRaises(
            Exception, myia.setrestoringbeam, imagename=imagename
        )
        source.open(imagename)
        source.setrestoringbeam(
            major="4arcsec", minor="2arcsec", pa="0deg",
            channel=0, polarization=0
        )
        source.setrestoringbeam(
            major="8arcsec", minor="8arcsec", pa="0deg",
            channel=0, polarization=0
        )
        # incompatible beam matrices
        self.assertRaises(
            Exception, myia.setrestoringbeam, imagename=imagename
        )
        source.fromshape(
            imagename, shape=[5,5, nstokes],
            overwrite=True
        )
        source.setrestoringbeam(
            major="4arcsec", minor="2arcsec", pa="0deg",
            channel=0, polarization=0
        )
        source.setrestoringbeam(
            major="8arcsec", minor="4arcsec", pa="0deg",
            channel=0, polarization=0
        )
        source.done()
        myia.setrestoringbeam(imagename=imagename)
        self._compareBeams(myia, imagename)
        
        source.fromshape(
            imagename, shape=[5,5, nstokes, 1],
            overwrite=True
        )
        source.setrestoringbeam(
            major="4arcsec", minor="2arcsec", pa="0deg",
            channel=0, polarization=0
        )
        source.setrestoringbeam(
            major="8arcsec", minor="4arcsec", pa="0deg",
            channel=0, polarization=0
        )
        source.done()
        myia.setrestoringbeam(imagename=imagename)
        self._compareBeams(myia, imagename)
        
        source.fromshape(
            imagename, shape=[5,5, nchan],
            overwrite=True
        )
        source.setrestoringbeam(
            major="4arcsec", minor="2arcsec", pa="0deg",
            channel=0, polarization=0
        )
        source.setrestoringbeam(
            major="8arcsec", minor="4arcsec", pa="0deg",
            channel=0, polarization=0
        )
        source.done()
        myia.setrestoringbeam(imagename=imagename)
        self._compareBeams(myia, imagename)
        
        source.fromshape(
            imagename, shape=[5,5, nchan, 1],
            overwrite=True
        )
        source.setrestoringbeam(
            major="4arcsec", minor="2arcsec", pa="0deg",
            channel=0, polarization=0
        )
        source.setrestoringbeam(
            major="8arcsec", minor="4arcsec", pa="0deg",
            channel=0, polarization=0
        )
        source.done()
        myia.setrestoringbeam(imagename=imagename)
        self._compareBeams(myia, imagename)
        
        myia.done()
        
        
    def _compareBeams(self, target, imagename):
        source = iatool()
        source.open(imagename)
        tshape = target.shape()
        sshape = source.shape()
        tnchan = tshape[target.coordsys().findaxisbyname("spectral")]
        tnstokes = tshape[target.coordsys().findaxisbyname("stokes")]
        try:
            snchan = sshape[source.coordsys().findaxisbyname("spectral")]
        except:
            snchan = 0
        try:
            snstokes = sshape[source.coordsys().findaxisbyname("stokes")]
        except:
            snstokes = 0
        self.assertTrue(
            (tnchan == snchan or snchan <= 1)
            and (tnstokes == snstokes or snstokes <= 1)
        )
        for c in range(tnchan):
            for p in range(tnstokes):
                got = target.restoringbeam(channel=c, polarization=p)
                sc = c
                if (snchan == 0):
                    sn = -1;
                expec = source.restoringbeam(channel=sc, polarization=p)
                self.assertTrue(got == expec)
        source.done()

def suite():
    return [ia_restoringbeam_test]
