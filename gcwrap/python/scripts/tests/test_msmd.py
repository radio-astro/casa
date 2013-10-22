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
# Test suite for the CASA tool msmd
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
#   <li> <linkto class="msmd:description">msmd</linkto> 
# </ul>
# </prerequisite>
#
# <etymology>
# Test for the msmd tool
# </etymology>
#
# <synopsis>
# Test the msmd tool
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_msmd[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the ia.subimage tool method to ensure
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

fixture = os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/MSMetaData/MSMetaData.ms'

class msmd_test(unittest.TestCase):
    
    def setUp(self):
        self.md = msmdtool()
        self.md.open(fixture)
    
    def tearDown(self):
        self.md.done()
        self.assertTrue(len(tb.showcache()) == 0)
        
    def test_antennanames_and_antennaids(self):
        """Test antennanames() and antennaids()"""
        names = [
            "DA43", "DA44", "DV02", "DV03", "DV05",
            "DV07", "DV08", "DV10", "DV12", "DV13",
            "DV14", "DV15", "DV16", "DV17", "DV18"
        ]
        for i in range(self.md.nantennas()):
            got = self.md.antennanames(i)
            self.assertTrue(got == [names[i]])
            got = self.md.antennaids(names[i])
            self.assertTrue(got == [i])
        expec = ["DV07", "DV02"]
        got = self.md.antennanames([5, 2])
        self.assertTrue(got == expec)
        expec = [4, 0, 7]
        got = self.md.antennaids([names[4], names[0], names[7]])
        self.assertTrue((got == expec).all())
        
    def test_chanavgspws(self):
        """Test chanavgspws()"""
        got = self.md.chanavgspws()
        expec = numpy.array([
            2, 4, 6, 8, 10, 12, 14,
            16, 18, 20, 22, 24
        ])
        self.assertTrue((got == expec).all())

        
    def test_fdmspws(self):
        """Test fdmspws()"""
        got = self.md.fdmspws()
        expec = numpy.array([17, 19, 21, 23])
        self.assertTrue((got == expec).all())

    def test_fieldsforintent(self):
        """Test fieldsforintent()"""
        for intent in self.md.intents():
            if intent=="CALIBRATE_AMPLI#ON_SOURCE":
                expec = numpy.array([2])
            elif [
                "CALIBRATE_BANDPASS#ON_SOURCE",
                "CALIBRATE_SIDEBAND_RATIO#OFF_SOURCE",
                "CALIBRATE_SIDEBAND_RATIO#ON_SOURCE"
            ].count(intent) > 0:
                expec = numpy.array([0])
            elif [
                "CALIBRATE_ATMOSPHERE#OFF_SOURCE",
                "CALIBRATE_ATMOSPHERE#ON_SOURCE",
                "CALIBRATE_WVR#OFF_SOURCE"
            ].count(intent) > 0:
                expec = numpy.array([0, 2, 3, 4, 5])
            elif intent=="CALIBRATE_PHASE#ON_SOURCE":
                expec = numpy.array([0, 2, 3])
            elif intent=="CALIBRATE_POINTING#ON_SOURCE":
                expec = numpy.array([0, 1, 3])
            elif intent=="CALIBRATE_WVR#ON_SOURCE":
                expec = numpy.array([0, 1, 2, 3, 4, 5])
            else:
                expec = numpy.array([4, 5])
            got = self.md.fieldsforintent(intent)
            self.assertTrue((got == expec).all())
        self.assertTrue(
            (
             self.md.fieldsforintent('.*WVR.*')
             == numpy.array([0, 1, 2, 3, 4, 5])
            ).all()
        )

    def test_fieldsforname(self):
        """Test fieldforname()"""
        names = ["3C279", "J1337-129", "Titan", "J1625-254", "V866 Sco", "RNO 90"]
        for i in range(self.md.nfields()):
            expec = numpy.array([i])
            got = self.md.fieldsforname(names[i])
            self.assertTrue((got==expec).all())
    
    def test_fieldsforscan(self):
        """Test fieldsforscan() and fieldsforscans()"""
        expec2 = numpy.array([], dtype="int")
        scans = numpy.array([], dtype="int")
        names = numpy.array([
            "3C279", "J1337-129", "Titan",
            "J1625-254", "V866 Sco", "RNO 90"
        ])
        for scan in self.md.scannumbers():
            if scan <= 4:
                expec = numpy.array([0])
            elif scan == 5:
                expec = numpy.array([1])
            elif scan <= 7:
                expec = numpy.array([2])
            elif [8, 9, 10, 13, 14, 17, 18, 21, 24, 25, 28, 31, 32].count(scan) > 0:
                expec = numpy.array([3])
            elif [11, 12, 19, 20, 26, 27].count(scan) > 0:
                expec = numpy.array([4])
            else:
                expec = numpy.array([5])
            expec2 = numpy.unique(numpy.append(expec2, expec))
            got = self.md.fieldsforscan(scan, False)
            self.assertTrue((got==expec).all())
            got = self.md.fieldsforscan(scan, True)
            scans = numpy.append(scans, scan)
            self.assertTrue((got==names[expec]).all())
            got = self.md.fieldsforscans(scans, False)
            self.assertTrue((got==expec2).all())
            got = self.md.fieldsforscans(scans, True)
            self.assertTrue((got==names[expec2]).all())
            
    def test_fieldsforspw(self):
        """Test fieldsforspw()"""
        for i in range(self.md.nspw()):
            if (i==0):
                expids = [0, 1, 2, 3, 4, 5]
                expnames = [
                    "3C279", "J1337-129", "Titan", "J1625-254",
                    "V866 Sco", "RNO 90"
                ]
            elif (i<9):
                expids = [0, 1, 3]
                expnames = ["3C279", "J1337-129", "J1625-254"]
            elif (i<25):
                expids = [0, 2, 3, 4, 5]
                expnames = [
                    "3C279", "Titan", "J1625-254",
                    "V866 Sco", "RNO 90"
                ]
            else:
                expids = []
                expnames = []
        if i < 25:
            got = self.md.fieldsforspw(i, false)
            self.assertTrue((got == expids).all())
            got = self.md.fieldsforspw(i, true)
            self.assertTrue((got == expnames).all())
        else:
            got = self.md.fieldsforspw(i, false)
            self.assertTrue(len(got) == 0)
            got = self.md.fieldsforspw(i, true)
            self.assertTrue(len(got) == 0)

    def test_fieldsfortimes(self):
        """Test fieldsfortimes()"""
        got = self.md.fieldsfortimes(4842824746, 10)
        expec = numpy.array([0])
        self.assertTrue((got == expec).all())
        got = self.md.fieldsfortimes(4842824746, 10000)
        expec = numpy.array([0, 1, 2, 3, 4, 5])
        self.assertTrue((got == expec).all())
        
    def test_intents(self):
        """Test intents()"""
        got = self.md.intents()
        expec = numpy.array(
            [
                "CALIBRATE_AMPLI#ON_SOURCE",
                "CALIBRATE_ATMOSPHERE#OFF_SOURCE",
                "CALIBRATE_ATMOSPHERE#ON_SOURCE",
                "CALIBRATE_BANDPASS#ON_SOURCE",
                "CALIBRATE_PHASE#ON_SOURCE",
                "CALIBRATE_POINTING#ON_SOURCE",
                "CALIBRATE_SIDEBAND_RATIO#OFF_SOURCE",
                "CALIBRATE_SIDEBAND_RATIO#ON_SOURCE",
                "CALIBRATE_WVR#OFF_SOURCE",
                "CALIBRATE_WVR#ON_SOURCE",
                "OBSERVE_TARGET#ON_SOURCE"
            ]
        )
        self.assertTrue((got == expec).all())
    
    def test_intentsforfield(self):
        """Test intentsforfield()"""
        for field in range(self.md.nfields()):
            for i in [0, 1]:
                if i == 0:
                    f = field
                else:
                    f = self.md.namesforfields(field)[0]
                got = self.md.intentsforfield(f)
                if field == 0:
                    expec = numpy.array([
                        "CALIBRATE_ATMOSPHERE#OFF_SOURCE", "CALIBRATE_ATMOSPHERE#ON_SOURCE",
                        "CALIBRATE_BANDPASS#ON_SOURCE", "CALIBRATE_PHASE#ON_SOURCE",
                        "CALIBRATE_POINTING#ON_SOURCE", "CALIBRATE_SIDEBAND_RATIO#OFF_SOURCE",
                        "CALIBRATE_SIDEBAND_RATIO#ON_SOURCE", "CALIBRATE_WVR#OFF_SOURCE",
                        "CALIBRATE_WVR#ON_SOURCE"
                    ])
                if field == 1:
                    expec = numpy.array([
                        "CALIBRATE_POINTING#ON_SOURCE", "CALIBRATE_WVR#ON_SOURCE"
                    ])
                if field == 2:
                    expec = numpy.array([
                        "CALIBRATE_AMPLI#ON_SOURCE", "CALIBRATE_ATMOSPHERE#OFF_SOURCE",
                        "CALIBRATE_ATMOSPHERE#ON_SOURCE", "CALIBRATE_PHASE#ON_SOURCE",
                        "CALIBRATE_WVR#OFF_SOURCE", "CALIBRATE_WVR#ON_SOURCE"
                    ])
                if field == 3:
                    expec = numpy.array([
                        "CALIBRATE_ATMOSPHERE#OFF_SOURCE", "CALIBRATE_ATMOSPHERE#ON_SOURCE",
                        "CALIBRATE_PHASE#ON_SOURCE", "CALIBRATE_POINTING#ON_SOURCE",
                        "CALIBRATE_WVR#OFF_SOURCE", "CALIBRATE_WVR#ON_SOURCE"
                    ])
                if field == 4:
                    expec = numpy.array([
                        "CALIBRATE_ATMOSPHERE#OFF_SOURCE", "CALIBRATE_ATMOSPHERE#ON_SOURCE",
                        "CALIBRATE_WVR#OFF_SOURCE", "CALIBRATE_WVR#ON_SOURCE",
                        "OBSERVE_TARGET#ON_SOURCE"
                    ])
                if field == 5:
                    expec = numpy.array([
                        "CALIBRATE_ATMOSPHERE#OFF_SOURCE", "CALIBRATE_ATMOSPHERE#ON_SOURCE",
                        "CALIBRATE_WVR#OFF_SOURCE", "CALIBRATE_WVR#ON_SOURCE",
                        "OBSERVE_TARGET#ON_SOURCE"
                    ])
                self.assertTrue((got == expec).all())
    
    def test_intentsforscan(self):
        """Test intentsforscan()"""
        for scan in self.md.scannumbers():
            got = self.md.intentsforscan(scan);
            if [1, 5, 8].count(scan) > 0:
                expec = numpy.array(
                    ["CALIBRATE_POINTING#ON_SOURCE", "CALIBRATE_WVR#ON_SOURCE"]
                )
            elif scan==2:
                expec = numpy.array(
                    [
                        "CALIBRATE_SIDEBAND_RATIO#OFF_SOURCE",
                        "CALIBRATE_SIDEBAND_RATIO#ON_SOURCE",
                        "CALIBRATE_WVR#OFF_SOURCE",
                        "CALIBRATE_WVR#ON_SOURCE"
                    ]
                )   
            elif [3, 6, 9, 11, 13, 15, 17, 19, 22, 24, 26, 29, 31].count(scan) > 0:
                expec = numpy.array(
                    [
                        "CALIBRATE_ATMOSPHERE#OFF_SOURCE",
                        "CALIBRATE_ATMOSPHERE#ON_SOURCE",
                        "CALIBRATE_WVR#OFF_SOURCE",
                        "CALIBRATE_WVR#ON_SOURCE"
                    ]
                )
            elif scan == 4:
                expec = numpy.array(
                    [
                        "CALIBRATE_BANDPASS#ON_SOURCE",
                        "CALIBRATE_PHASE#ON_SOURCE",
                        "CALIBRATE_WVR#ON_SOURCE"
                    ]
                )
            elif scan==7:
                expec = numpy.array(
                    [
                        "CALIBRATE_AMPLI#ON_SOURCE",
                        "CALIBRATE_PHASE#ON_SOURCE",
                        "CALIBRATE_WVR#ON_SOURCE"
                    ]
                )
            elif [10, 14, 18, 21, 25, 28, 32].count(scan) > 0:
                expec = numpy.array(
                    [
                        "CALIBRATE_PHASE#ON_SOURCE",
                        "CALIBRATE_WVR#ON_SOURCE"
                    ]
                )
            elif [12, 16, 20, 23, 27, 30].count(scan) > 0:
                expec = numpy.array(["OBSERVE_TARGET#ON_SOURCE"])
            self.assertTrue((got == expec).all())
        
    def test_intentsforspw(self):
        """Test intentsforspw()"""
        for spw in range(self.md.nspw()):
            got = self.md.intentsforspw(spw)
            if spw == 0:
                expec = numpy.array(
                    [
                        "CALIBRATE_AMPLI#ON_SOURCE",
                        "CALIBRATE_ATMOSPHERE#OFF_SOURCE",
                        "CALIBRATE_ATMOSPHERE#ON_SOURCE",
                        "CALIBRATE_BANDPASS#ON_SOURCE",
                        "CALIBRATE_PHASE#ON_SOURCE",
                        "CALIBRATE_POINTING#ON_SOURCE",
                        "CALIBRATE_SIDEBAND_RATIO#OFF_SOURCE",
                        "CALIBRATE_SIDEBAND_RATIO#ON_SOURCE",
                        "CALIBRATE_WVR#OFF_SOURCE",
                        "CALIBRATE_WVR#ON_SOURCE",
                        "OBSERVE_TARGET#ON_SOURCE"
                    ]
                )
            elif spw < 9:
                expec = numpy.array(
                    [
                        "CALIBRATE_POINTING#ON_SOURCE",
                        "CALIBRATE_WVR#ON_SOURCE"
                    ]
                )
            elif (spw < 17):
                expec = numpy.array(
                    [
                        "CALIBRATE_ATMOSPHERE#OFF_SOURCE",
                        "CALIBRATE_ATMOSPHERE#ON_SOURCE",
                        "CALIBRATE_SIDEBAND_RATIO#OFF_SOURCE",
                        "CALIBRATE_SIDEBAND_RATIO#ON_SOURCE",
                        "CALIBRATE_WVR#OFF_SOURCE",
                        "CALIBRATE_WVR#ON_SOURCE"
                   ]
                )
            elif (spw < 25):
                expec = numpy.array(
                    [
                        "CALIBRATE_AMPLI#ON_SOURCE",
                        "CALIBRATE_BANDPASS#ON_SOURCE",
                        "CALIBRATE_PHASE#ON_SOURCE",
                        "CALIBRATE_WVR#ON_SOURCE",
                        "OBSERVE_TARGET#ON_SOURCE"
                    ]
                )
            else:
                expec = numpy.array([])
            self.assertTrue((got == expec).all())

    def test_namesforfields(self):
        """Test namesforfields()"""
        names = [
            "3C279", "J1337-129", "Titan",
            "J1625-254", "V866 Sco", "RNO 90"
        ]
        for i in range(self.md.nfields()):
            got = self.md.namesforfields(i)
            self.assertTrue(got == [names[i]])
        self.assertTrue(self.md.namesforfields() == names)
        got = self.md.namesforfields([4, 0, 2])
        print "*** got " + str(got)
        self.assertTrue(got == ["V866 Sco", "3C279", "Titan"])

    def test_nantennas(self):
        """ Test nantennas()"""
        self.assertTrue(self.md.nantennas() == 15)
     
    def test_nfields(self):
        """ Test nfields()"""
        self.assertTrue(self.md.nfields() == 6)
     
    def test_nobservations(self):
        """ Test nfields()"""
        self.assertTrue(self.md.nobservations() == 1)
     
    def test_nscans(self):
        """ Test nscans()"""
        self.assertTrue(self.md.nscans() == 32)

    def test_nspw(self):
        """ Test nspw()"""
        self.assertTrue(self.md.nspw() == 40)
        
    def test_nstates(self):
        """ Test nstates()"""
        self.assertTrue(self.md.nstates() == 43)
        
    def test_nvis(self):
        """ Test nvis()"""
        self.assertTrue(self.md.nrows() == 15363)
        
    def test_scannumbers(self):
        """ Test scannumbers()"""
        expec = numpy.array(range(1, 33))
        self.assertTrue((self.md.scannumbers()==expec).all())
        
    def test_scansforfield(self):
        """Test scansforfield()"""
        names = ["3C279", "J1337-129", "Titan", "J1625-254", "V866 Sco", "RNO 90"]
        for i in range(self.md.nfields()):
            if i == 0:
                expec = numpy.array([1, 2, 3, 4])
            elif i == 1:
                expec = numpy.array([5])
            elif i == 2:
                expec = numpy.array([6, 7])
            elif i == 3:
                expec = numpy.array([
                    8, 9, 10, 13, 14, 17, 18,
                    21, 24, 25, 28, 31, 32
                ])
            elif i == 4:
                expec = numpy.array([11, 12, 19, 20, 26, 27])
            elif i == 5:
                expec = numpy.array([15, 16, 22, 23, 29, 30])
            got = self.md.scansforfield(i)
            self.assertTrue((got==expec).all())
            got = self.md.scansforfield(names[i])
            self.assertTrue((got==expec).all())
    
    def test_scansforintent(self):
        """Test scansforintent()"""
        for i in self.md.intents():
            if i=="CALIBRATE_AMPLI#ON_SOURCE":
                expec = numpy.array([7])
            elif ["CALIBRATE_ATMOSPHERE#OFF_SOURCE", "CALIBRATE_ATMOSPHERE#ON_SOURCE"].count(i) > 0:
                expec = numpy.array([
                    3, 6, 9, 11, 13, 15, 17,
                    19, 22, 24, 26, 29, 31
                ])
            elif i=="CALIBRATE_BANDPASS#ON_SOURCE":
                expec = numpy.array([4])
            elif i=="CALIBRATE_PHASE#ON_SOURCE":
                expec = numpy.array([4, 7, 10, 14, 18, 21, 25, 28, 32])
            elif i=="CALIBRATE_POINTING#ON_SOURCE":
                expec = numpy.array([1, 5, 8])
            elif ["CALIBRATE_SIDEBAND_RATIO#OFF_SOURCE", "CALIBRATE_SIDEBAND_RATIO#ON_SOURCE"].count(i) > 0:
                expec = numpy.array([2])
            elif i == "CALIBRATE_WVR#OFF_SOURCE":
                expec = numpy.array([
                    2, 3, 6, 9, 11, 13, 15, 17,
                    19, 22, 24, 26, 29, 31
                ])
            elif i=="CALIBRATE_WVR#ON_SOURCE":
                expec = numpy.array([
                    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                    11, 13, 14, 15, 17, 18, 19, 21,
                    22, 24, 25, 26, 28, 29, 31, 32
                ])
            else:
                expec = numpy.array([12, 16, 20, 23, 27, 30])
            got = self.md.scansforintent(i, regex=False)
            self.assertTrue((got == expec).all())
        self.assertTrue(
            (
             self.md.scansforintent('.*WVR.*', regex=True)
             == numpy.array([
                    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                    11, 13, 14, 15, 17, 18, 19, 21,
                    22, 24, 25, 26, 28, 29, 31, 32
                ])
            ).all()
        )
        
    def test_scansforspw(self):
        """Test scansforspw()"""
        for i in range(self.md.nspw()):
            if (i==0):
                expec = numpy.array([
                    1,  2,  3,  4,  5,  6,  7,  8,  9,
                    10, 11, 12, 13, 14, 15, 16, 17, 18,
                    19, 20, 21, 22, 23, 24, 25, 26, 27,
                    28, 29, 30, 31, 32
                ])
            elif (i<9):
                expec = numpy.array([1, 5, 8])
            elif (i<17):
                expec = numpy.array([
                    2,  3,  6,  9, 11, 13, 15,
                    17, 19, 22, 24, 26, 29, 31
                ])
            elif (i<25):
                expec = numpy.array([
                    4,  7, 10, 12, 14, 16, 18,
                    20, 21, 23, 25, 27, 28, 30, 32
                ])
            got = self.md.scansforspw(i)
            if (i<25):
                self.assertTrue((got == expec).all())
            else:
                self.assertTrue(len(got) == 0)

    def test_scansforstate(self):
        """ Test scansforstate()"""
        for state in range(self.md.nstates()):
            got = self.md.scansforstate(state)
            if (state < 5):
                expec = numpy.array([1, 5, 8])
            elif (state < 7):
                expec = numpy.array([2])
            elif (state < 10):
                expec = numpy.array(
                    [3, 6, 9, 11, 13, 15, 17, 19, 22, 24, 26, 29, 31]
                )
            elif (state < 26):
                expec = numpy.array([4])
            elif (state < 32):
                expec = numpy.array([7])
            elif (state < 33):
                expec = numpy.array([10, 14, 18, 21, 25, 28, 32])
            elif (state < 37):
                expec = numpy.array([12, 16, 20, 23, 27, 30])
            else:
                expec = numpy.array([12, 16, 20, 23])
            self.assertTrue((got == expec).all())

    def test_scansfortimes(self):
        """Test scansfortimes()"""
        expec = numpy.array([27])
        got = self.md.scansfortimes(4.84282937e+09,20)
        self.assertTrue((got == expec).all())
        expec = numpy.array([24, 25, 26, 27, 28])
        got = self.md.scansfortimes(4.84282937e+09,200)
        self.assertTrue((got == expec).all())

    def test_spwsforbasebands(self):
        """Test spwsforbasebands()"""
        for mode in ("i", "e", "o"):
            got = self.md.spwsforbaseband(sqldmode=mode)
            print "*** got " + str(got)
            self.assertTrue(len(got) == 5)
            if mode == "o":
                self.assertTrue(len(got['0']) == 0)
                self.assertTrue(len(got['1']) == 0)
                self.assertTrue(got['2'] == [3])
                self.assertTrue(len(got['3']) == 0)
                self.assertTrue(len(got['4']) == 0)
            else:
                self.assertTrue(
                    (
                        got['0']
                        == [
                            0, 25, 26, 27, 28, 29,
                            30, 31, 32, 33, 34, 35,
                            36, 37, 38, 39
                        ]
                     ).all()
                )
                self.assertTrue(
                    (got['1'] == [1, 2, 9, 10, 17, 18]).all()
                )
                if mode == "i":
                    self.assertTrue(
                        (got['2'] == [3, 4, 11, 12, 19, 20]).all()
                    )
                else:
                    self.assertTrue(
                        (got['2'] == [4, 11, 12, 19, 20]).all()
                )
                self.assertTrue(
                    (got['3'] == [5, 6, 13, 14, 21, 22]).all()
                )
                self.assertTrue(
                    (got['4'] == [7, 8, 15, 16, 23, 24]).all()
                )
            for i in range(4):
                self.assertTrue(
                    (got[str(i)] == self.md.spwsforbaseband(i, sqldmode=mode)).all()
                )

    def test_spwsforfield(self):
        """Test spwsforfield()"""
        names = [
            "3C279", "J1337-129", "Titan",
            "J1625-254", "V866 Sco", "RNO 90"
        ]
        for i in range(self.md.nfields()):
            if (i==0 or i==3):
                expec = numpy.array(
                    [
                        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                        10, 11, 12, 13, 14, 15, 16, 17,
                        18, 19, 20, 21, 22, 23, 24
                    ]
                )
            elif (i == 1):
                expec = numpy.array([0, 1, 2, 3, 4, 5, 6, 7, 8])
            elif (i==2 or i==4 or i==5):
                expec = numpy.array(
                    [
                        0, 9, 10, 11, 12, 13, 14, 15, 16,
                        17, 18, 19, 20, 21, 22, 23, 24
                    ]
                )         
            got = self.md.spwsforfield(i)
            self.assertTrue((got == expec).all())
            got = self.md.spwsforfield(names[i])
            self.assertTrue((got == expec).all())

    def test_spwsforintent(self):
        """Test spwsforintent()"""
        for intent in self.md.intents():
            got = self.md.spwsforintent(intent)
            if [
                "CALIBRATE_AMPLI#ON_SOURCE",
                "CALIBRATE_BANDPASS#ON_SOURCE",
                "CALIBRATE_PHASE#ON_SOURCE",
                "OBSERVE_TARGET#ON_SOURCE"
            ].count(intent) > 0:
                
                expec = numpy.array([0, 17, 18, 19, 20, 21, 22, 23, 24])
            elif [
                "CALIBRATE_ATMOSPHERE#OFF_SOURCE",
                "CALIBRATE_ATMOSPHERE#ON_SOURCE",
                "CALIBRATE_SIDEBAND_RATIO#OFF_SOURCE",
                "CALIBRATE_SIDEBAND_RATIO#ON_SOURCE",
                "CALIBRATE_WVR#OFF_SOURCE"
            ].count(intent) > 0:
                expec = numpy.array([0, 9, 10, 11, 12, 13, 14, 15, 16])
            elif intent == "CALIBRATE_POINTING#ON_SOURCE":
                expec = numpy.array([0, 1, 2, 3, 4, 5, 6, 7, 8])
            elif intent == "CALIBRATE_WVR#ON_SOURCE":
                expec = numpy.array(
                    [
                        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                        10, 11, 12, 13, 14, 15, 16,
                        17, 18, 19, 20, 21, 22, 23, 24
                    ]
                )
            self.assertTrue((got == expec).all())
            
            got = self.md.spwsforintent('.*WVR.*', regex=True)
            expec = numpy.array(
                [
                    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                    10, 11, 12, 13, 14, 15, 16,
                    17, 18, 19, 20, 21, 22, 23, 24
                 ]
            )
            self.assertTrue((got == expec).all())

    def test_spwsforscan(self):
        """Test spwsforscan()"""
        for i in self.md.scannumbers():
            if [1, 5, 8].count(i) > 0:
                expec = numpy.array([0, 1, 2, 3, 4, 5, 6, 7, 8])
            elif [2, 3, 6, 9, 11, 13, 15, 17, 19, 22, 24, 26, 29, 31].count(i) > 0:
                expec = numpy.array([0, 9, 10, 11, 12, 13, 14, 15, 16])
            elif [4, 7, 10, 12, 14, 16, 18, 20, 21, 23, 25, 27, 28, 30, 32].count(i) > 0:
                expec = numpy.array([0, 17, 18, 19, 20, 21, 22, 23, 24])
            got = self.md.spwsforscan(i)
            self.assertTrue((got == expec).all())
            
    def test_statesforscan(self):
        """Test statesforscan()"""
        for i in self.md.scannumbers():
            if [1, 5, 8].count(i) > 0:
                expec = numpy.array([0, 1, 2, 3, 4])
            elif i == 2:
                expec = numpy.array([5, 6])
            elif [3, 6, 9, 11, 13, 15, 17, 19, 22, 24, 26, 29, 31].count(i) > 0:
                expec = numpy.array([7, 8, 9])
            elif i == 4:
                expec = numpy.array([
                    10, 11, 12, 13, 14, 15, 16, 17, 18,
                    19, 20, 21, 22, 23, 24, 25
                ])
            elif i == 7:
                expec = numpy.array([26, 27, 28, 29, 30, 31])
            elif [10, 14, 18, 21, 25, 28, 32].count(i) > 0:
                expec = numpy.array([32])
            elif [12, 16, 20, 23].count(i) > 0:
                expec = numpy.array([33, 34, 35, 36, 37, 38, 39, 40, 41, 42])
            else:
                expec = numpy.array([33, 34, 35, 36])
            got = self.md.statesforscan(i)
            self.assertTrue((got == expec).all())

    def test_telescopenames(self):
        """ Test observatorynames()"""
        got = self.md.observatorynames()
        expec = numpy.array(["ALMA"])
        self.assertTrue((got == expec).all())
        
    def test_tdmspws(self):
        """Test tdmspws()"""
        got = self.md.tdmspws()
        expec = numpy.array([1, 3, 5, 7, 9, 11, 13, 15])
        self.assertTrue((got == expec).all())

    def test_timesforfield(self):
        """Test timesforfield()"""
        for i in range(self.md.nfields()):
            if i == 0:
                expec = 818
            if i == 1:
                expec = 81
            if i == 2:
                expec = 248
            if i == 3:
                expec = 402
            if i == 4:
                expec = 963
            if i == 5:
                expec = 965
            got = len(self.md.timesforfield(i))
            self.assertTrue(got == expec)

    def test_timesforscan(self):
        """Test timesforscan()"""
        expec = numpy.array([
       4842825003.6,
       4842825004.0, 4842825004.5,
       4842825004.8, 4842825005.0,
       4842825016.3, 4842825016.6,
       4842825017.1, 4842825017.5,
       4842825017.6, 4842825029.0,
       4842825029.3, 4842825029.8,
       4842825030.1, 4842825030.3
        ])
        expec.sort()
        got = self.md.timesforscans(3)
        self.assertTrue((abs(got - expec)).all() < 0.1)
                 

    def test_timesforscans(self):
        """Test timesforscans()"""
        expec = numpy.array([
            4842825928.7, 4842825929.5,
            4842825930.0,
            4842825930.6, 4842825941.4,
            4842825942.2, 4842825942.5,
            4842825942.7, 4842825943.2,
            4842825954.0, 4842825954.9,
            4842825955.2, 4842825955.4,
            4842825955.9, 4842825003.6,
            4842825004.0, 4842825004.5,
            4842825004.8, 4842825005.0,
            4842825016.3, 4842825016.6,
            4842825017.1, 4842825017.5,
            4842825017.6, 4842825029.0,
            4842825029.3, 4842825029.8,
            4842825030.1, 4842825030.3
        ])
        expec.sort()
        got = self.md.timesforscans([3, 6])
        self.assertTrue((abs(got - expec)).all() < 0.1)
                        
    def test_wvrspws(self):
        """Test wvrspws()"""
        got = self.md.wvrspws()
        expec = numpy.array([
            0, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39
        ])
        self.assertTrue((got == expec).all())
        got = self.md.wvrspws(False)
        self.assertTrue((got == expec).all())
        got = self.md.wvrspws(True)
        expec = range(1, 25)
        self.assertTrue((got == expec).all())

    def test_almaspws(self):
        """Test almaspws()"""
        got = self.md.almaspws()
        self.assertTrue(len(got) == 0)
        got = self.md.almaspws(complement=True)
        self.assertTrue(len(got) == 40)
        got = self.md.almaspws(sqld=True)
        self.assertTrue(len(got) == 1 and got[0] == 3)
        got = self.md.almaspws(sqld=True, complement=True)
        expec = range(40)
        expec.remove(3)
        self.assertTrue((got == expec).all())

        got = self.md.almaspws(chavg=True)
        expec = numpy.array([
            2, 4, 6, 8, 10, 12, 14,
            16, 18, 20, 22, 24
        ])
        self.assertTrue((got == expec).all())
        got = self.md.almaspws(chavg=True, complement=True)
        jj = range(40)
        for i in expec:
            jj.remove(i)
        expec = jj
        self.assertTrue((got == expec).all())
        
        got = self.md.almaspws(fdm=True)
        expec = [17, 19, 21, 23]
        self.assertTrue((got == expec).all())
        got = self.md.almaspws(fdm=True, complement=True)
        jj = range(40)
        for i in expec:
            jj.remove(i)
        expec = jj
        self.assertTrue((got == expec).all())
        
        got = self.md.almaspws(tdm=True)
        expec = [1, 3, 5, 7, 9, 11, 13, 15]
        self.assertTrue((got == expec).all())
        got = self.md.almaspws(tdm=True, complement=True)
        jj = range(40)
        for i in expec:
            jj.remove(i)
        expec = jj
        self.assertTrue((got == expec).all())
        
        got = self.md.almaspws(wvr=True)
        expec = numpy.array([
            0, 25, 26, 27, 28, 29, 30, 31,
            32, 33, 34, 35, 36, 37, 38, 39
        ])
        self.assertTrue((got == expec).all())
        got = self.md.almaspws(wvr=True, complement=True)
        expec = range(1, 25)
        self.assertTrue((got == expec).all())


    def test_bandwidths(self):
        """Test bandwidths()"""
        got = self.md.bandwidths()
        expec = [
            7.5e+09,      2e+09,        1.796875e+09,
            2e+09,        1.796875e+09, 2e+09,
            1.796875e+09, 2e+09,        1.796875e+09,
            2e+09,        1.796875e+09, 2e+09,
            1.796875e+09,   2e+09,   1.796875e+09,
            2e+09,   1.796875e+09,   1.171875e+08,
            1.171875e+08,   1.171875e+08,   1.171875e+08,
            1.171875e+08,   1.171875e+08,   1.171875e+08,
            1.171875e+08,   7.5e+09,   7.5e+09,
            7.5e+09,   7.5e+09,   7.5e+09,
            7.5e+09,   7.5e+09,   7.5e+09,
            7.5e+09,   7.5e+09,   7.5e+09,
            7.5e+09,   7.5e+09,   7.5e+09,
            7.5e+09
        ]
        self.assertTrue((got == expec).all())
        got = self.md.bandwidths(-1)
        self.assertTrue((got == expec).all())
        for i in range(len(expec)):
            self.assertTrue(self.md.bandwidths(i) == expec[i])
            self.assertTrue(self.md.bandwidths([i]) == [expec[i]])
        self.assertTrue((self.md.bandwidths([4, 10, 5]) == [expec[4], expec[10], expec[5]]).all)
        self.assertRaises(Exception, self.md.bandwidths, 50)
        self.assertRaises(Exception, self.md.bandwidths, [4, 50])
        self.assertRaises(Exception, self.md.bandwidths, [4, -1])
        
    def test_chanwidths(self):
        """Test chanwidths()"""
        got = self.md.chanwidths(0)
        expec = numpy.array([1.5e9, 2.5e9, 2e9, 1.5e9])
        self.assertTrue((got == expec).all())
        got = self.md.chanwidths(0, "MHz")
        self.assertTrue((got == expec/1e6).all())
        self.assertRaises(Exception, self.md.chanwidths, 50);
        self.assertRaises(Exception, self.md.chanwidths, -2);
        
        self.md.close()
        self.assertRaises(Exception, self.md.chanwidths, 1)
        self.assertRaises(Exception, self.md.chanfreqs, 1)
        self.assertRaises(Exception, self.md.meanfreq, 1)
        self.assertRaises(Exception, self.md.sideband, 1)
        self.assertRaises(Exception, self.md.effexposuretime)

    def test_datadescids(self):
        """Test datadescids()"""
        md = self.md;
        got = md.datadescids()
        self.assertTrue((got == range(25)).all())
        got = md.datadescids(-1, -1)
        self.assertTrue((got == range(25)).all())
        for i in range(25):
            got = md.datadescids(i, -1)
            self.assertTrue(got == [i])
        got = md.datadescids(pol=1)
        self.assertTrue(got == [0])
        got = md.datadescids(pol=0)
        self.assertTrue((got == range(1, 25)).all())
        got = md.datadescids(spw=10, pol=1)
        self.assertTrue(len(got) == 0)
        
    def test_antennastations(self):
        """Test antennastations()"""
        md = self.md
        got = md.antennastations()
        expec = numpy.array([
            'A075', 'A068', 'A077', 'A137', 'A082', 'A076', 'A021', 'A071',
            'A011', 'A072', 'A025', 'A074', 'A069', 'A138', 'A053'
        ])
        self.assertTrue((got == expec).all())
        got = md.antennastations(-1)
        self.assertTrue((got == expec).all())
        got = md.antennastations([-1])
        self.assertTrue((got == expec).all())
        got = md.antennastations([])
        self.assertTrue((got == expec).all())
        got = md.antennastations(2)
        self.assertTrue((got == numpy.array(["A077"])).all())
        self.assertRaises(Exception, md.antennastations, [2, -1])
        got = md.antennastations([4, 2])
        expec = numpy.array(['A082', 'A077'])
        self.assertTrue((got == expec).all())
        self.assertRaises(Exception, md.antennastations, [1, 20])
        self.assertRaises(Exception, md.antennastations, 20)
        got = md.antennastations('DV13')
        expec = numpy.array(["A072"])
        self.assertTrue((got == expec).all())
        expec = numpy.array(["A072", "A075"])
        got = md.antennastations(['DV13', 'DA43'])
        self.assertTrue((got == expec).all())

def suite():
    return [msmd_test]
