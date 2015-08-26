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

datadir = os.environ.get('CASAPATH').split()[0]+'/data/'
fixture = datadir + 'regression/unittest/MSMetaData/MSMetaData.ms'

def near(a, b, epsilon):
    return abs((a-b)/max(a,b)) <= epsilon

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
        
        got = self.md.antennaids()        
        expec = range(self.md.nantennas())
        self.assertTrue((got == expec).all())
        
        got = self.md.antennaids(["DV12", "DA*", "DV1*"])
        expec =  [ 8, 0,  1,  7,  9, 10, 11, 12, 13, 14]
        self.assertTrue((got == expec).all())
        
        got = self.md.antennaids(["DV12", "DA*", "DV1*"], "1m", qa.quantity(15,"m"))
        expec =  [ 8, 0,  1,  7,  9, 10, 11, 12, 13, 14]
        self.assertTrue((got == expec).all())
        
        got = self.md.antennaids(["DV12", "DA*", "DV1*"], "1m", qa.quantity(2,"m"))
        self.assertTrue(len(got) == 0)
        
        got = self.md.antennaids([], mindiameter="25m")
        self.assertTrue(len(got) == 0)
        
    def test_chanavgspws(self):
        """Test chanavgspws()"""
        got = self.md.chanavgspws()
        expec = numpy.array([
            2, 4, 6, 8, 10, 12, 14,
            16, 18, 20, 22, 24
        ])
        self.assertTrue((got == expec).all())

    def test_exposuretime(self):
        """Test exposuretime()"""
        # no DDID for spwid=0, polid=0
        self.assertRaises(Exception,self.md.exposuretime, scan=30, spwid=0, polid=0)
        got = self.md.exposuretime(scan=30, spwid=0, polid=1)
        self.assertTrue(got == qa.quantity("1.152s"))
        got = self.md.exposuretime(scan=30, spwid=0)
        self.assertTrue(got == qa.quantity("1.152s"))
        got = self.md.exposuretime(scan=17, spwid=10, polid=0)
        self.assertTrue(got == qa.quantity("1.008s"))
        got = self.md.exposuretime(scan=17, spwid=10)
        self.assertTrue(got == qa.quantity("1.008s"))
        
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
             self.md.fieldsforintent('*WVR*')
             == numpy.array([0, 1, 2, 3, 4, 5])
            ).all()
        )

    def test_fieldsforname(self):
        """Test fieldforname()"""
        got = self.md.fieldsforname()
        expec = numpy.array([0, 1, 2, 3, 4, 5])
        self.assertTrue((got == expec).all())
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
        self.assertRaises(Exception, self.md.fieldsforscans)
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
            got = self.md.scansforintent(i)
            self.assertTrue((got == expec).all())
        self.assertTrue(
            (
             self.md.scansforintent('*WVR*')
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
            
            got = self.md.spwsforintent('*WVR*')
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
        got = self.md.timesforscan(5, 0, 0, True)
        expec = {}
        expec[0] = [
            4842825782.3999996185, 4842825800.8319997787, 4842825807.7440004349,
            4842825826.1760005951, 4842825844.6079998016, 4842825861.8879995346,
            4842825869.9519996643
        ]
        expec[1] = [
            4842825778.6560001373, 4842825780.6719999313, 4842825782.6879997253,
            4842825784.704000473,  4842825786.720000267,  4842825799.3920001984,
            4842825801.4079999924, 4842825803.4239997864, 4842825805.4400005341,
            4842825807.4560003281, 4842825820.1280002594, 4842825822.1440000534,
            4842825824.1599998474, 4842825826.1760005951, 4842825828.1920003891,
            4842825840.8640003204, 4842825842.8800001144, 4842825844.8959999084,
            4842825846.9119997025, 4842825848.9279994965, 4842825861.6000003815,
            4842825863.6159992218, 4842825865.6319999695, 4842825867.6479997635,
            4842825869.6639995575
        ]
        expec[2] = [
            4842825778.1519994736, 4842825779.1600008011, 4842825780.1679992676,
            4842825781.1760005951, 4842825782.1839990616, 4842825783.1920003891,
            4842825784.1999998093, 4842825785.2080001831, 4842825786.2159996033,
            4842825787.2240009308, 4842825798.8879995346, 4842825799.8960008621,
            4842825800.9039993286, 4842825801.9120006561, 4842825802.9199991226,
            4842825803.9280004501, 4842825804.9359998703, 4842825805.9440002441,
            4842825806.9519996643, 4842825807.9600009918, 4842825819.6239995956,
            4842825820.6320009232, 4842825821.6399993896, 4842825822.6480007172,
            4842825823.6560001373, 4842825824.6640005112, 4842825825.6719999313,
            4842825826.6800003052, 4842825827.6879997253, 4842825828.6960010529,
            4842825840.3599996567, 4842825841.3680009842, 4842825842.3759994507,
            4842825843.3840007782, 4842825844.3920001984, 4842825845.4000005722,
            4842825846.4079990387, 4842825847.4160003662, 4842825848.4239988327,
            4842825849.4320001602, 4842825861.0959997177, 4842825862.1040010452,
            4842825863.1119995117, 4842825864.1200008392, 4842825865.1279993057,
            4842825866.1360006332, 4842825867.1439990997, 4842825868.1520004272,
            4842825869.1599998474, 4842825870.1680002213
        ]
        expec[3] = [
            4842825778.6560001373, 4842825780.6719999313, 4842825782.6879997253,
            4842825784.704000473,  4842825786.720000267,  4842825799.3920001984,
            4842825801.4079999924, 4842825803.4239997864, 4842825805.4400005341,
            4842825807.4560003281, 4842825820.1280002594, 4842825822.1440000534,
            4842825824.1599998474, 4842825826.1760005951, 4842825828.1920003891,
            4842825840.8640003204, 4842825842.8800001144, 4842825844.8959999084,
            4842825846.9119997025, 4842825848.9279994965, 4842825861.6000003815,
            4842825863.6159992218, 4842825865.6319999695, 4842825867.6479997635,
            4842825869.6639995575
        ]
        expec[4] = [
            4842825778.1519994736, 4842825779.1600008011, 4842825780.1679992676,
            4842825781.1760005951, 4842825782.1839990616, 4842825783.1920003891,
            4842825784.1999998093, 4842825785.2080001831, 4842825786.2159996033,
            4842825787.2240009308, 4842825798.8879995346, 4842825799.8960008621,
            4842825800.9039993286, 4842825801.9120006561, 4842825802.9199991226,
            4842825803.9280004501, 4842825804.9359998703, 4842825805.9440002441,
            4842825806.9519996643, 4842825807.9600009918, 4842825819.6239995956,
            4842825820.6320009232, 4842825821.6399993896, 4842825822.6480007172,
            4842825823.6560001373, 4842825824.6640005112, 4842825825.6719999313,
            4842825826.6800003052, 4842825827.6879997253, 4842825828.6960010529,
            4842825840.3599996567, 4842825841.3680009842, 4842825842.3759994507,
            4842825843.3840007782, 4842825844.3920001984, 4842825845.4000005722,
            4842825846.4079990387, 4842825847.4160003662, 4842825848.4239988327,
            4842825849.4320001602, 4842825861.0959997177, 4842825862.1040010452,
            4842825863.1119995117, 4842825864.1200008392, 4842825865.1279993057,
            4842825866.1360006332, 4842825867.1439990997, 4842825868.1520004272,
            4842825869.1599998474, 4842825870.1680002213
        ]
        expec[5] = [
            4842825778.6560001373, 4842825780.6719999313, 4842825782.6879997253,
            4842825784.704000473,  4842825786.720000267,  4842825799.3920001984,
            4842825801.4079999924, 4842825803.4239997864, 4842825805.4400005341,
            4842825807.4560003281, 4842825820.1280002594, 4842825822.1440000534,
            4842825824.1599998474, 4842825826.1760005951, 4842825828.1920003891,
            4842825840.8640003204, 4842825842.8800001144, 4842825844.8959999084,
            4842825846.9119997025, 4842825848.9279994965, 4842825861.6000003815,
            4842825863.6159992218, 4842825865.6319999695, 4842825867.6479997635,
            4842825869.6639995575
        ]
        expec[6] = [
            4842825778.1519994736, 4842825779.1600008011, 4842825780.1679992676,
            4842825781.1760005951, 4842825782.1839990616, 4842825783.1920003891,
            4842825784.1999998093, 4842825785.2080001831, 4842825786.2159996033,
            4842825787.2240009308, 4842825798.8879995346, 4842825799.8960008621,
            4842825800.9039993286, 4842825801.9120006561, 4842825802.9199991226,
            4842825803.9280004501, 4842825804.9359998703, 4842825805.9440002441,
            4842825806.9519996643, 4842825807.9600009918, 4842825819.6239995956,
            4842825820.6320009232, 4842825821.6399993896, 4842825822.6480007172,
            4842825823.6560001373, 4842825824.6640005112, 4842825825.6719999313,
            4842825826.6800003052, 4842825827.6879997253, 4842825828.6960010529,
            4842825840.3599996567, 4842825841.3680009842, 4842825842.3759994507,
            4842825843.3840007782, 4842825844.3920001984, 4842825845.4000005722,
            4842825846.4079990387, 4842825847.4160003662, 4842825848.4239988327,
            4842825849.4320001602, 4842825861.0959997177, 4842825862.1040010452,
            4842825863.1119995117, 4842825864.1200008392, 4842825865.1279993057,
            4842825866.1360006332, 4842825867.1439990997, 4842825868.1520004272,
            4842825869.1599998474, 4842825870.1680002213
        ]
        expec[7] = [
            4842825778.6560001373, 4842825780.6719999313, 4842825782.6879997253,
            4842825784.704000473,  4842825786.720000267,  4842825799.3920001984,
            4842825801.4079999924, 4842825803.4239997864, 4842825805.4400005341,
            4842825807.4560003281, 4842825820.1280002594, 4842825822.1440000534,
            4842825824.1599998474, 4842825826.1760005951, 4842825828.1920003891,
            4842825840.8640003204, 4842825842.8800001144, 4842825844.8959999084,
            4842825846.9119997025, 4842825848.9279994965, 4842825861.6000003815,
            4842825863.6159992218, 4842825865.6319999695, 4842825867.6479997635,
            4842825869.6639995575
        ]
        expec[8] = [
            4842825778.1519994736, 4842825779.1600008011, 4842825780.1679992676,
            4842825781.1760005951, 4842825782.1839990616, 4842825783.1920003891,
            4842825784.1999998093, 4842825785.2080001831, 4842825786.2159996033,
            4842825787.2240009308, 4842825798.8879995346, 4842825799.8960008621,
            4842825800.9039993286, 4842825801.9120006561, 4842825802.9199991226,
            4842825803.9280004501, 4842825804.9359998703, 4842825805.9440002441,
            4842825806.9519996643, 4842825807.9600009918, 4842825819.6239995956,
            4842825820.6320009232, 4842825821.6399993896, 4842825822.6480007172,
            4842825823.6560001373, 4842825824.6640005112, 4842825825.6719999313,
            4842825826.6800003052, 4842825827.6879997253, 4842825828.6960010529,
            4842825840.3599996567, 4842825841.3680009842, 4842825842.3759994507,
            4842825843.3840007782, 4842825844.3920001984, 4842825845.4000005722,
            4842825846.4079990387, 4842825847.4160003662, 4842825848.4239988327,
            4842825849.4320001602, 4842825861.0959997177, 4842825862.1040010452,
            4842825863.1119995117, 4842825864.1200008392, 4842825865.1279993057,
            4842825866.1360006332, 4842825867.1439990997, 4842825868.1520004272,
            4842825869.1599998474, 4842825870.1680002213
        ]
        self.assertTrue(len(got.keys()) == 9)
        for i in range(9):
            k = str(i)
            e = numpy.array(expec[i])
            g = got[k]
            self.assertTrue(len(g) == len(e))
            d = g - e
            self.assertTrue(numpy.max(numpy.abs(d)) < 0.1)
            
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
        got = self.md.wvrspws(complement=False)
        self.assertTrue((got == expec).all())
        got = self.md.wvrspws(complement=True)
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
        
    def test_namesforspws(self):
        """Test namesforspws()"""
        md = self.md
        got = md.namesforspws()
        i = 0
        for name in got:
            if i == 3:
                expec = "BB_1#SQLD"
            else:
                expec = ""
            self.assertTrue(name == expec)
            i += 1
        got = md.namesforspws([4, 3])
        self.assertTrue((got == numpy.array(["", "BB_1#SQLD"])).all())
        got = md.namesforspws(3)
        self.assertTrue((got == numpy.array(["BB_1#SQLD"])).all())
        
        self.assertRaises(Exception, md.namesforspws, -2)
        self.assertRaises(Exception, md.namesforspws, [0,-2])
        self.assertRaises(Exception, md.namesforspws, 85)
        self.assertRaises(Exception, md.namesforspws, [0,85])

    def test_fieldsforsource(self):
        """Test fieldsforsource()"""
        md = self.md
        names = [
            "3C279", "J1337-129", "Titan",
            "J1625-254", "V866 Sco", "RNO 90"
        ]
        for i in range(7):
            res = md.fieldsforsource(i, False)
            if i == 6:
                self.assertTrue(len(res) == 0)
            else:
                self.assertTrue(len(res) == 1 and res[0] == i)
            res2 = md.fieldsforsource(i, True)
            if i == 6:
                self.assertTrue(len(res2) == 0)
            else:
                self.assertTrue(len(res2) == 1 and res2[0] == names[i])

    def test_pointingdirection(self):
        """Test pointingdirection(), CAS-5878"""
        md = self.md
        ret = md.pointingdirection(500)
        self.assertTrue(ret['antenna1']['id'] == 7)
        self.assertTrue(ret['antenna2']['id'] == 11)
        eps = 1e-10
        self.assertTrue(near(ret['time'], 4842824902.632, eps))
        p1 = ret['antenna1']['pointingdirection']
        p2 = ret['antenna2']['pointingdirection']
        self.assertTrue(near(p1['m0']['value'], -1.231522504164003, eps))
        self.assertTrue(near(p1['m1']['value'], 0.8713643131745025, eps))
        self.assertTrue(near(p2['m0']['value'], -1.2315042783587336, eps))
        self.assertTrue(near(p2['m1']['value'], 0.8713175514123461, eps))
    
    def test_name(self):
        """Test name(), CAS-6817"""
        md = self.md
        name = md.name()
        self.assertTrue(name == os.path.abspath(fixture))   
        
    def test_timesforintent(self):
        """Test timesforintent(), CAS-6919"""
        md = self.md
        intents = md.intents()
        for intent in intents:
            times = md.timesforintent(intent)
            ntimes = len(times)
            expec = 0
            if intent == "CALIBRATE_AMPLI#ON_SOURCE":
                expec = 234
            elif intent == "CALIBRATE_ATMOSPHERE#OFF_SOURCE":
                expec = 46
            elif intent == "CALIBRATE_ATMOSPHERE#ON_SOURCE":
                expec = 93
            elif intent == "CALIBRATE_BANDPASS#ON_SOURCE":
                expec = 623
            elif intent == "CALIBRATE_PHASE#ON_SOURCE":
                expec = 1128
            elif intent == "CALIBRATE_POINTING#ON_SOURCE":
                expec = 244
            elif (
                intent == "CALIBRATE_SIDEBAND_RATIO#OFF_SOURCE"
                or intent == "CALIBRATE_SIDEBAND_RATIO#ON_SOURCE"
            ):
                expec = 49
            elif intent == "CALIBRATE_WVR#OFF_SOURCE":
                expec = 95
            elif intent == "CALIBRATE_WVR#ON_SOURCE":
                expec = 1514
            elif intent == "OBSERVE_TARGET#ON_SOURCE":
                expec = 1868
            self.assertTrue(ntimes == expec)
        
    def test_CAS7463(self):
        md = self.md
        self.assertTrue((md.chanwidths(0) == [1.5e9, 2.5e9, 2e9, 1.5e9]).all())
        self.assertTrue((md.chanwidths(0, "GHz") == [1.5, 2.5, 2, 1.5]).all())
        self.assertRaises(Exception, md.chanwidths, 0, "km/s")

    def test_sideband(self):
        md = self.md
        expec = [
            -1, -1, -1, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1,
            -1, -1, -1, -1, 1,  1,  1,  1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
        ]
        expec = numpy.array(expec)
        got = []
        for i in range(md.nspw()):
            got.append(md.sideband(i))
        get = numpy.array(got)
        self.assertTrue((got == expec).all())       
        
    def test_fieldnames(self):
        md = self.md
        got = md.fieldnames()
        expec = ['3C279', 'J1337-129', 'Titan', 'J1625-254', 'V866 Sco', 'RNO 90']
        self.assertTrue(got == expec)
        
    def test_projects(self):
        """Test msmd.projects()"""
        md = self.md
        projects = md.projects()
        self.assertTrue(len(projects) == 1)
        self.assertTrue(projects[0] == "T.B.D.")
        
    def test_observers(self):
        """Test msmd.observers()"""
        md = self.md
        observers = md.observers()
        self.assertTrue(len(observers) == 1)
        self.assertTrue(observers[0] == "csalyk")
        
    def test_schedule(self):
        """Test msmd.schedule()"""
        md = self.md
        schedule = md.schedule(0)
        self.assertTrue(len(schedule) == 2)
        self.assertTrue(schedule[0] == "SchedulingBlock uid://A002/X391d0b/X5e")
        self.assertTrue(schedule[1] == "ExecBlock uid://A002/X3f6a86/X5da")
        
    def test_timerforobs(self):
        """Test msmd.timerforobs()"""
        md = self.md
        timer = md.timerangeforobs(0)
        self.assertTrue(
            near(
                qa.convert(me.getvalue(timer['begin'])['m0'],"s")['value'],
                4842824633.472, 1e-10
            )
        )
        self.assertTrue(
            near(
                qa.convert(me.getvalue(timer['end'])['m0'],"s")['value'],
                4842830031.632, 1e-10
            )
        )
        
    def test_reffreq(self):
        """Test msmd.reffreq"""
        md = self.md
        nspw = md.nspw()
        expec = [
                 1.83300000e+11, 2.15250000e+11, 2.15250000e+11,
                 2.17250000e+11, 2.17250000e+11, 2.29250000e+11,
                 2.29250000e+11, 2.31250000e+11, 2.31250000e+11,
                 2.30471730e+11, 2.30471730e+11, 2.32352270e+11,
                 2.32352270e+11, 2.20465062e+11, 2.20465062e+11,
                 2.19610562e+11, 2.19610562e+11, 2.30471730e+11,
                 2.30471730e+11, 2.32352270e+11, 2.32352270e+11,
                 2.20465062e+11, 2.20465062e+11, 2.19610562e+11,
                 2.19610562e+11, 1.83310000e+11, 1.83320000e+11,
                 1.83330000e+11, 1.83340000e+11, 1.83350000e+11,
                 1.83360000e+11, 1.83370000e+11, 1.83380000e+11,
                 1.83390000e+11, 1.83400000e+11, 1.83410000e+11,
                 1.83420000e+11, 1.83430000e+11, 1.83440000e+11,
                 1.83450000e+11
                ]
        for i in range(nspw):
            freq = md.reffreq(i)
            self.assertTrue(me.getref(freq) == 'TOPO')
            v = me.getvalue(freq)['m0']
            self.assertTrue(qa.getunit(v) == "Hz")
            got = qa.getvalue(v) 
            self.assertTrue(abs((got - expec[i])/expec[i]) < 1e-8)
            
    def test_antennadiamter(self):
        """Test msmd.antennadiameter"""
        md = self.md
        nants = md.nantennas()
        for i in range(nants):
            diam = md.antennadiameter(i)
            self.assertTrue(qa.getvalue(diam) == 12)
            self.assertTrue(qa.getunit(diam) == 'm')
    
    def test_spwfordatadesc(self):
        """Test msmd.spwfordatadesc()"""
        md = self.md
        for i in range(25):
            self.assertTrue(md.spwfordatadesc(i) == i)
        spws = md.spwfordatadesc(-1)
        expec = numpy.array(range(25))
        self.assertTrue((spws == expec).all())
        
    def test_polidfordatadesc(self):
        """Test msmd.polidfordatadesc()"""
        md = self.md
        for i in range(25):
            polid = md.polidfordatadesc(i)
            if i == 0:
                self.assertTrue(polid == 1)
            else:
                self.assertTrue(polid == 0)
        polids = md.polidfordatadesc(-1)
        expec = numpy.zeros([25])
        expec[0] = 1
        self.assertTrue((polids == expec).all())
    
    def test_ncorrforpol(self):
        """Test msmd.ncorrforpol()"""
        md = self.md
        for i in [0, 1]:
            ncorr = md.ncorrforpol(i)
            if i == 0:
                expec = 2
            else:
                expec = 1
            self.assertTrue(ncorr == expec)
        ncorrs = md.ncorrforpol(-1)
        self.assertTrue(len(ncorrs) == 2)
        print "ncorrs", ncorrs
        self.assertTrue(ncorrs[0] == 2 and ncorrs[1] == 1)
        
    def test_corrtypesforpol(self):
        """Test msmd.corrtypesforpol()"""
        md = self.md
        for i in [-1, 0, 1, 2]:
            if i == -1 or i == 2:
                self.assertRaises(Exception, md.corrtypesforpol, i)
            elif i == 0:
                ct = md.corrtypesforpol(i)
                self.assertTrue(len(ct) == 2)
                self.assertTrue(ct[0] == 9 and ct[1] == 12)
            else:
                ct = md.corrtypesforpol(i)
                self.assertTrue(len(ct) == 1 and ct[0] == 1)
                
    def test_corrprodsforpol(self):
        """Test msmd.corrprodssforpol()"""
        md = self.md
        for i in [-1, 0, 1, 2]:
            if i == -1 or i == 2:
                self.assertRaises(Exception, md.corrprodsforpol, i)
            elif i == 0:
                ct = md.corrprodsforpol(i)
                print "got", ct
                self.assertTrue(ct.size == 4)
                self.assertTrue(ct[0][0] == 0 and ct[1][1] == 1)
                self.assertTrue(ct[0][1] == 1 and ct[1][0] == 0)
            else:
                ct = md.corrprodsforpol(i)
                self.assertTrue(ct.size == 2 and (ct == 0).all())
                
    def test_sourceidforfield(self):
        """Test msmd.sourceidforfield()"""
        md = self.md
        for i in range(6):
            self.assertTrue(md.sourceidforfield(i) == i)
        self.assertRaises(Exception, md.sourceidforfield, -1)
        self.assertRaises(Exception, md.sourceidforfield, 6)

    def test_antennasforscan(self):
        """Test msmd.antennasforscan()"""
        md = self.md
        expec = numpy.array([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13])
        expec9 = numpy.array([0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 13])
        for i in range(1, 33):
            got = md.antennasforscan(i, 0, 0)
            myexpec = expec
            if i == 9:
                myexpec = expec9
            self.assertTrue((got == myexpec).all())
        self.assertRaises(Exception, md.antennasforscan, 0)
        self.assertRaises(Exception, md.antennasforscan, 33)

    def test_sourceidsfromsourcetable(self):
        """Test msmd.sourceidsfromsourcetable()"""
        md = self.md
        expec = numpy.array([
             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
             1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
             2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
             1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2,
             2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4,
             4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3,
             3, 3, 3, 3, 3, 3, 3, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
             4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
        ])
        self.assertTrue((md.sourceidsfromsourcetable() == expec).all())

    def test_sourcenames(self):
        """Test msmd.sourcenames()"""
        md = self.md
        expec = numpy.array([
            "3C279", "3C279", "3C279", "3C279", "3C279", "3C279", "3C279",
            "3C279", "3C279", "3C279", "3C279", "3C279", "3C279", "3C279",
            "3C279", "3C279", "3C279", "3C279", "3C279", "3C279", "3C279",
            "3C279", "3C279", "3C279", "3C279", "3C279", "3C279", "3C279",
            "3C279", "3C279", "3C279", "3C279", "3C279", "3C279", "3C279",
            "3C279", "3C279", "3C279", "3C279", "3C279", "J1337-129",
            "J1337-129", "J1337-129", "J1337-129", "J1337-129", "J1337-129",
            "J1337-129", "J1337-129", "J1337-129", "J1337-129", "J1337-129",
            "J1337-129", "J1337-129", "J1337-129", "J1337-129", "J1337-129",
            "J1337-129", "J1337-129", "J1337-129", "J1337-129", "J1337-129",
            "J1337-129", "J1337-129", "J1337-129", "Titan", "Titan", "Titan",
            "Titan", "Titan", "Titan", "Titan", "Titan", "Titan", "Titan",
            "Titan", "Titan", "Titan", "Titan", "Titan", "Titan", "Titan",
            "Titan", "Titan", "Titan", "Titan", "Titan", "Titan", "Titan",
            "Titan", "Titan", "Titan", "Titan", "Titan", "Titan", "Titan",
            "Titan", "J1625-254", "J1625-254", "J1625-254", "J1625-254",
            "J1625-254", "J1625-254", "J1625-254", "J1625-254", "J1625-254",
            "J1625-254", "J1625-254", "J1625-254", "J1625-254", "J1625-254",
            "J1625-254", "J1625-254", "J1625-254", "J1625-254", "J1625-254",
            "J1625-254", "J1625-254", "J1625-254", "J1625-254", "J1625-254",
            "J1625-254", "J1625-254", "J1625-254", "J1625-254", "J1625-254",
            "J1625-254", "J1625-254", "J1625-254", "J1625-254", "J1625-254",
            "J1625-254", "J1625-254", "J1625-254", "J1625-254", "J1625-254",
            "J1625-254", "V866 Sco", "V866 Sco", "V866 Sco", "V866 Sco",
            "V866 Sco", "V866 Sco", "V866 Sco", "V866 Sco", "V866 Sco",
            "V866 Sco", "V866 Sco", "V866 Sco", "V866 Sco", "V866 Sco",
            "V866 Sco", "V866 Sco", "V866 Sco", "V866 Sco", "V866 Sco",
            "V866 Sco", "V866 Sco", "V866 Sco", "V866 Sco", "V866 Sco",
            "V866 Sco", "V866 Sco", "V866 Sco", "V866 Sco", "V866 Sco",
            "V866 Sco", "V866 Sco", "V866 Sco", "RNO 90", "RNO 90", "RNO 90",
            "RNO 90", "RNO 90", "RNO 90", "RNO 90", "RNO 90", "RNO 90",
            "RNO 90", "RNO 90", "RNO 90", "RNO 90", "RNO 90", "RNO 90",
            "RNO 90", "RNO 90", "RNO 90", "RNO 90", "RNO 90", "RNO 90",
            "RNO 90", "RNO 90", "RNO 90", "RNO 90", "RNO 90", "RNO 90",
            "RNO 90", "RNO 90", "RNO 90", "RNO 90", "RNO 90"
        ])
        self.assertTrue((md.sourcenames() == expec).all())
        
    def test_sourcedirs(self):
        """Test msmd.sourcedirs()"""
        md = self.md;
        elong = [
            -2.8964345 , -2.8964345 , -2.8964345 , -2.8964345 , -2.8964345 ,
            -2.8964345 , -2.8964345 , -2.8964345 , -2.8964345 , -2.8964345 ,
            -2.8964345 , -2.8964345 , -2.8964345 , -2.8964345 , -2.8964345 ,
            -2.8964345 , -2.8964345 , -2.8964345 , -2.8964345 , -2.8964345 ,
            -2.8964345 , -2.8964345 , -2.8964345 , -2.8964345 , -2.8964345 ,
            -2.8964345 , -2.8964345 , -2.8964345 , -2.8964345 , -2.8964345 ,
            -2.8964345 , -2.8964345 , -2.8964345 , -2.8964345 , -2.8964345 ,
            -2.8964345 , -2.8964345 , -2.8964345 , -2.8964345 , -2.8964345 ,
            -2.71545722, -2.71545722, -2.71545722, -2.71545722, -2.71545722,
            -2.71545722, -2.71545722, -2.71545722, -2.71545722, -2.71545722,
            -2.71545722, -2.71545722, -2.71545722, -2.71545722, -2.71545722,
            -2.71545722, -2.71545722, -2.71545722, -2.71545722, -2.71545722,
            -2.71545722, -2.71545722, -2.71545722, -2.71545722, -2.72554329,
            -2.72554329, -2.72554329, -2.72554329, -2.72554329, -2.72554329,
            -2.72554329, -2.72554329, -2.72554329, -2.72554329, -2.72554329,
            -2.72554329, -2.72554329, -2.72554329, -2.72554329, -2.72554329,
            -2.72554329, -2.72554329, -2.72554329, -2.72554329, -2.72554329,
            -2.72554329, -2.72554329, -2.72554329, -2.72554329, -2.72554329,
            -2.72554329, -2.72554329, -2.72554329, -2.72554329, -2.72554329,
            -2.72554329, -1.98190197, -1.98190197, -1.98190197, -1.98190197,
            -1.98190197, -1.98190197, -1.98190197, -1.98190197, -1.98190197,
            -1.98190197, -1.98190197, -1.98190197, -1.98190197, -1.98190197,
            -1.98190197, -1.98190197, -1.98190197, -1.98190197, -1.98190197,
            -1.98190197, -1.98190197, -1.98190197, -1.98190197, -1.98190197,
            -1.98190197, -1.98190197, -1.98190197, -1.98190197, -1.98190197,
            -1.98190197, -1.98190197, -1.98190197, -1.98190197, -1.98190197,
            -1.98190197, -1.98190197, -1.98190197, -1.98190197, -1.98190197,
            -1.98190197, -2.04411602, -2.04411602, -2.04411602, -2.04411602,
            -2.04411602, -2.04411602, -2.04411602, -2.04411602, -2.04411602,
            -2.04411602, -2.04411602, -2.04411602, -2.04411602, -2.04411602,
            -2.04411602, -2.04411602, -2.04411602, -2.04411602, -2.04411602,
            -2.04411602, -2.04411602, -2.04411602, -2.04411602, -2.04411602,
            -2.04411602, -2.04411602, -2.04411602, -2.04411602, -2.04411602,
            -2.04411602, -2.04411602, -2.04411602, -1.94537525, -1.94537525,
            -1.94537525, -1.94537525, -1.94537525, -1.94537525, -1.94537525,
            -1.94537525, -1.94537525, -1.94537525, -1.94537525, -1.94537525,
            -1.94537525, -1.94537525, -1.94537525, -1.94537525, -1.94537525,
            -1.94537525, -1.94537525, -1.94537525, -1.94537525, -1.94537525,
            -1.94537525, -1.94537525, -1.94537525, -1.94537525, -1.94537525,
            -1.94537525, -1.94537525, -1.94537525, -1.94537525, -1.94537525
        ]
        elat = [
            -0.10104256, -0.10104256, -0.10104256, -0.10104256, -0.10104256,
            -0.10104256, -0.10104256, -0.10104256, -0.10104256, -0.10104256,
            -0.10104256, -0.10104256, -0.10104256, -0.10104256, -0.10104256,
            -0.10104256, -0.10104256, -0.10104256, -0.10104256, -0.10104256,
            -0.10104256, -0.10104256, -0.10104256, -0.10104256, -0.10104256,
            -0.10104256, -0.10104256, -0.10104256, -0.10104256, -0.10104256,
            -0.10104256, -0.10104256, -0.10104256, -0.10104256, -0.10104256,
            -0.10104256, -0.10104256, -0.10104256, -0.10104256, -0.10104256,
            -0.22613985, -0.22613985, -0.22613985, -0.22613985, -0.22613985,
            -0.22613985, -0.22613985, -0.22613985, -0.22613985, -0.22613985,
            -0.22613985, -0.22613985, -0.22613985, -0.22613985, -0.22613985,
            -0.22613985, -0.22613985, -0.22613985, -0.22613985, -0.22613985,
            -0.22613985, -0.22613985, -0.22613985, -0.22613985, -0.1219181 ,
            -0.1219181 , -0.1219181 , -0.1219181 , -0.1219181 , -0.1219181 ,
            -0.1219181 , -0.1219181 , -0.1219181 , -0.1219181 , -0.1219181 ,
            -0.1219181 , -0.1219181 , -0.1219181 , -0.1219181 , -0.1219181 ,
            -0.1219181 , -0.1219181 , -0.1219181 , -0.1219181 , -0.1219181 ,
            -0.1219181 , -0.1219181 , -0.1219181 , -0.1219181 , -0.1219181 ,
            -0.1219181 , -0.1219181 , -0.1219181 , -0.1219181 , -0.1219181 ,
            -0.1219181 , -0.44437211, -0.44437211, -0.44437211, -0.44437211,
            -0.44437211, -0.44437211, -0.44437211, -0.44437211, -0.44437211,
            -0.44437211, -0.44437211, -0.44437211, -0.44437211, -0.44437211,
            -0.44437211, -0.44437211, -0.44437211, -0.44437211, -0.44437211,
            -0.44437211, -0.44437211, -0.44437211, -0.44437211, -0.44437211,
            -0.44437211, -0.44437211, -0.44437211, -0.44437211, -0.44437211,
            -0.44437211, -0.44437211, -0.44437211, -0.44437211, -0.44437211,
            -0.44437211, -0.44437211, -0.44437211, -0.44437211, -0.44437211,
            -0.44437211, -0.32533384, -0.32533384, -0.32533384, -0.32533384,
            -0.32533384, -0.32533384, -0.32533384, -0.32533384, -0.32533384,
            -0.32533384, -0.32533384, -0.32533384, -0.32533384, -0.32533384,
            -0.32533384, -0.32533384, -0.32533384, -0.32533384, -0.32533384,
            -0.32533384, -0.32533384, -0.32533384, -0.32533384, -0.32533384,
            -0.32533384, -0.32533384, -0.32533384, -0.32533384, -0.32533384,
            -0.32533384, -0.32533384, -0.32533384, -0.27584353, -0.27584353,
            -0.27584353, -0.27584353, -0.27584353, -0.27584353, -0.27584353,
            -0.27584353, -0.27584353, -0.27584353, -0.27584353, -0.27584353,
            -0.27584353, -0.27584353, -0.27584353, -0.27584353, -0.27584353,
            -0.27584353, -0.27584353, -0.27584353, -0.27584353, -0.27584353,
            -0.27584353, -0.27584353, -0.27584353, -0.27584353, -0.27584353,
            -0.27584353, -0.27584353, -0.27584353, -0.27584353, -0.27584353
        ]
        dirs = md.sourcedirs()
        n = len(dirs.keys())
        for i in range(n):
            d = dirs[str(i)]
            self.assertTrue(me.getref(d) == "J2000")
            v = me.getvalue(d)
            ra = v['m0']
            dec = v['m1']
            self.assertTrue(qa.getunit(ra) == "rad")
            self.assertTrue(qa.getunit(dec) == "rad")
            self.assertTrue(near(qa.getvalue(ra), elong[i], 1e-7))
            self.assertTrue(near(qa.getvalue(dec), elat[i], 1e-7))

    def test_propermotions(self):
        """Test msmd.propermotions()"""
        md = self.md;
        mu = md.propermotions()
        self.assertTrue(len(mu.keys()) == 200)
        for i in range(200):
            mymu = mu[str(i)]
            lon = mymu['longitude']
            lat = mymu['latitude']
            self.assertTrue(qa.getvalue(lon) == 0)
            self.assertTrue(qa.getunit(lon) == "rad/s")
            self.assertTrue(qa.getvalue(lat) == 0)
            self.assertTrue(qa.getunit(lat) == "rad/s")
            
    def test_nsources(self):
        md = self.md
        self.assertTrue(md.nsources() == 6)

    def test_refdir(self):
        """Test msmd.refdir()"""
        md = self.md
        epsilon = 1e-6 
        for i in range(md.nfields()):
            res = md.refdir(i)
            if i == 0:
                self.assertTrue(abs(1 - res['m0']['value']/-2.8964345) < epsilon)
                self.assertTrue(abs(1 - res['m1']['value']/-0.10104256) < epsilon)
            elif i == 1:
                self.assertTrue(abs(1 - res['m0']['value']/-2.71545722) < epsilon)
                self.assertTrue(abs(1 - res['m1']['value']/-0.22613985) < epsilon)
            elif i == 2:
                self.assertTrue(abs(1 - res['m0']['value']/-2.72554329) < epsilon)
                self.assertTrue(abs(1 - res['m1']['value']/-0.1219181) < epsilon)
            elif i == 3:
                self.assertTrue(abs(1 - res['m0']['value']/-1.98190197) < epsilon)
                self.assertTrue(abs(1 - res['m1']['value']/-0.44437211) < epsilon)
            elif i == 4:
                self.assertTrue(abs(1 - res['m0']['value']/-2.04411602) < epsilon)
                self.assertTrue(abs(1 - res['m1']['value']/-0.32533384) < epsilon)
            elif i == 5:
                self.assertTrue(abs(1 - res['m0']['value']/-1.94537525) < epsilon)
                self.assertTrue(abs(1 - res['m1']['value']/-0.27584353) < epsilon)

    def test_CAS7837(self):
        """Test corner case with no intents to make sure it doesn't segfault"""
        md = self.md
        importuvfits(datadir + 'regression/cvel/input/W3OH_MC.UVFITS', 'lala.ms')
        md.open('lala.ms')
        self.assertTrue((md.fieldsforintent('*') == numpy.array([0])).all())
            
def suite():
    return [msmd_test]
