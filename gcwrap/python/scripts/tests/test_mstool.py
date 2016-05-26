##########################################################################
# test_mstool.py
#
# Copyright (C) 2016
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
# Test suite for the CASA ms tool,
# using examples from ms.xml
#
###########################################################################

import shutil
import os
import time
import unittest
from numpy import array, testing, where
from math import ceil

from taskinit import mstool

datadir = os.environ.get('CASAPATH').split()[0]+'/data/regression/'
datafile = os.path.join(datadir, "unittest/listobs/ngc5921_ut.ms")
print 'ms tool tests will use data from '+ datafile

class mstool_test_base(unittest.TestCase):
    
    testms = "ngc5921.ms"
    testms2 = "ngc7538.ms"
    testfits = "ngc5921.fits"

    def setUpTest(self, nomodify=True):
        # Despite setUp/tearDown, some tests with writable MS left
        # MS in a changed state and caused later tests to fail.
        # So will copy RO testms to a RW one in those tests
        self.ms = mstool()
        if not os.path.exists(self.testms):
            shutil.copytree(datafile, self.testms)
        self.ms.open(self.testms, nomodify)

    def setUpMs2(self):
        ms2 = os.path.join(datadir, 'unittest/clean/ngc7538_ut.ms')
        if not os.path.exists(self.testms2):
            shutil.copytree(ms2, self.testms2, symlinks=True)
    
    def setUpFits(self):
        fitsfile = os.path.join(datadir, 'ngc5921/ngc5921.fits')
        if not os.path.exists(self.testfits):
            shutil.copyfile(fitsfile, self.testfits)

    def tearDownTest(self):
        self.ms.done()
        self.removeMS(self.testms)
        self.ms = None

    def checkMS(self, msName):
        # MS is a directory
        self.assertTrue(os.path.isdir(msName), "MS check failed")

    def removeMS(self, msName):
        # really any directory, usually an MS in these tests
        try:
            shutil.rmtree(msName)
        except OSError:
            pass

    def checkFile(self, fileName):
        # MS is a directory
        self.assertTrue(os.path.isfile(fileName), "File check failed")

    def removeFile(self, fileName):
        try:
            os.remove(fileName)
        except OSError:
            pass

# ------------------------------------------------------------------------------

class mstool_test_ms(mstool_test_base):
    """ Test basic MS info """

    def setUp(self):
        self.setUpTest()

    def tearDown(self):
        self.tearDownTest()

    def test_msinfo(self): 
        """test ms.iswritable, name, nrow, getreferencedtables"""
        self.assertFalse(self.ms.iswritable())
        self.assertEqual(self.ms.name(), os.path.join(os.getcwd(), self.testms))
        self.assertEqual(self.ms.nrow(), 22653)
        self.assertEqual(self.ms.getreferencedtables(), [self.ms.name()])
        print

    def test_getfielddirmeas(self): 
        """test ms.getfielddirmeas"""
        # Reads row 0 of DELAY_DIR column in FIELD table
        delayDir = self.ms.getfielddirmeas("DELAY_DIR", 0)
        self.assertAlmostEqual(delayDir['m0']['value'], -2.7439276)
        self.assertAlmostEqual(delayDir['m1']['value'], 0.53248521)
        print

    def test_asdmref(self): 
        """test ms.asdmref"""
        # empty string if no ASDM reference
        self.assertEqual(self.ms.asdmref(), "")
        print

# ------------------------------------------------------------------------------

class mstool_test_concat(mstool_test_base):

    def setUp(self):
        self.setUpTest(False) # need write access
        self.setUpMs2() 
        self.outputMS = os.path.join(os.getcwd(), "outputTest.ms")

    def tearDown(self):
        self.tearDownTest()
        self.removeMS(self.outputMS)
        self.removeMS(self.testms2)

    def test_createmultims(self): 
        """test ms.createmultims, ms.getreferencedtables"""
        self.ms.createmultims(self.outputMS, self.testms2, 
            ["ANTENNA", "DATA_DESCRIPTION", "FEED"])
        self.checkMS(self.outputMS)
        self.assertTrue(self.ms.ismultims())
        # For multims returns path to SUBMSS as a list
        refTable = os.path.join(self.outputMS, "SUBMSS/ngc7538.ms")
        self.assertEquals(self.ms.getreferencedtables(), [refTable])
        print

    def test_concatenate(self): 
        """test ms.concatenate"""
        # concatenate testms2 to testms
        self.ms.concatenate(self.testms2, '1GHz', '1arcsec')
        self.ms.close() # to flush changes
        self.ms.open(self.testms)
        # test MS originally had 22653 rows
        self.assertEqual(self.ms.nrow(), 49329)
        print

    def test_testconcatenate(self): 
        """test ms.testconcatenate"""
        # concat without MAIN and POINTING tables
        self.assertEqual(len(self.ms.range(["fields"])['fields']), 3)
        self.ms.testconcatenate(self.testms2, '1GHz', '1arcsec')
        self.assertEqual(len(self.ms.range(["fields"])['fields']), 6)
        print

    def test_virtconcatenate(self): 
        """test ms.virtconcatenate"""
        self.assertEqual(len(self.ms.range(["fields"])['fields']), 3)
        self.ms.virtconcatenate(self.testms2, 'ngcaux.dat', '1GHz', '1arcsec')
        self.assertEqual(len(self.ms.range(["fields"])['fields']), 6)
        self.removeFile('ngcaux.dat')
        print

# ------------------------------------------------------------------------------

class mstool_test_summary(mstool_test_base):

    def setUp(self):
        self.setUpTest()

    def tearDown(self):
        self.tearDownTest()

    def test_summary(self): 
        """test ms.summary"""
        # Returns record (python dict)
        summary = self.ms.summary()
        # just check simple keyword values (also has scan and field dicts)
        self.assertEqual(summary['numrecords'], 22653)
        self.assertEqual(summary['timeref'], 'TAI')
        self.assertEqual(summary['nfields'], 3)
        self.assertAlmostEqual(summary['IntegrationTime'], 5310.000972747803)
        self.assertAlmostEqual(summary['BeginTime'], 49820.38802083069)
        self.assertAlmostEqual(summary['EndTime'], 49820.44947917527)
        print

    def test_getscansummary(self): 
        """test ms.getscansummary"""
        # Returns record (python dict)
        scanInfo = self.ms.getscansummary()
        self.assertEqual(len(scanInfo), 7) # 7 scans in this ms
        scan7 = scanInfo['7']['0']  # we'll just check the details of one scan
        self.assertEqual(scan7['nRow'], 1512)
        self.assertEqual(scan7['StateId'], -1)
        self.assertEqual(scan7['FieldId'], 1)
        self.assertEqual(scan7['IntegrationTime'], 30.0)
        self.assertAlmostEqual(scan7['BeginTime'], 49820.44826388359)
        self.assertAlmostEqual(scan7['EndTime'], 49820.449305564165)
        print

    def test_getspectralwindowinfo(self): 
        """test ms.getspectralwindowinfo"""
        # Returns record (python dict)
        spwInfo = self.ms.getspectralwindowinfo()
        self.assertEqual(len(spwInfo), 1) # 1 spw
        spw0 = spwInfo['0']
        self.assertEqual(spw0['NumChan'], 63)
        self.assertEqual(spw0['NumCorr'], 2)
        self.assertEqual(spw0['Frame'], 'LSRK')
        self.assertEqual(spw0['PolId'], 0)
        self.assertEqual(spw0['SpectralWindowId'], 0)
        self.assertAlmostEqual(spw0['ChanWidth'], 24414.0625)
        self.assertAlmostEqual(spw0['TotalWidth'], 1550196.875)
        self.assertAlmostEqual(spw0['Chan1Freq'], 1412665073.7687755)
        self.assertAlmostEqual(spw0['RefFreq'], 1413428013.2219005)
        print

    def test_listhistory(self): 
        """test ms.listhistory"""
        # Lists history table in logger, returns bool
        self.assertTrue(self.ms.listhistory())
        print

    def test_writehistory(self): 
        """test ms.writehistory"""
        self.assertTrue(self.ms.writehistory('an arbitrary history message'))
        self.assertTrue(self.ms.listhistory())
        print

    def test_statistics(self): 
        """test ms.statistics"""
        # Returns record (python dict)
        stats = self.ms.statistics(column="DATA", complex_value='amp', 
                field="2")["DATA"]
        self.assertEqual(stats['npts'], 1619352.0)
        self.assertAlmostEqual(stats['medabsdevmed'], 0.0122800)
        self.assertAlmostEqual(stats['min'], 2.2130522e-05)
        self.assertAlmostEqual(stats['max'], 73.6559143)
        self.assertAlmostEqual(stats['sum'], 6899544.742657, 6)
        self.assertAlmostEqual(stats['sumsq'], 439740418.530647, 6)
        self.assertAlmostEqual(stats['mean'], 4.2606825)
        self.assertAlmostEqual(stats['median'], 0.0273157)
        self.assertAlmostEqual(stats['rms'], 16.4788742)
        self.assertAlmostEqual(stats['stddev'], 15.9185447)
        self.assertAlmostEqual(stats['var'], 253.4000652)
        self.assertAlmostEqual(stats['quartile'], 0.0254916)
        print

    def test_statistics2(self): 
        """test ms.statistics2"""
        # Returns record (python dict)
        stats = self.ms.statistics2(column="DATA", complex_value='amp', field="2")
        stats = stats['']
        self.assertTrue(stats['isMasked'])
        self.assertFalse(stats['isWeighted'])
        self.assertEqual(stats['minIndex'], 8692)
        self.assertEqual(stats['maxIndex'], 2408)
        self.assertEqual(stats['maxDatasetIndex'], 13)
        self.assertEqual(stats['minDatasetIndex'], 32)
        self.assertEqual(stats['npts'], 1619352.0)
        self.assertAlmostEqual(stats['medabsdevmed'], 0.0122800)
        self.assertAlmostEqual(stats['min'], 2.2130521e-05)
        self.assertAlmostEqual(stats['max'], 73.6559143)
        self.assertAlmostEqual(stats['sum'], 6899544.7427, 4)
        self.assertAlmostEqual(stats['sumsq'], 439740418.5306466)
        self.assertAlmostEqual(stats['mean'], 4.2606825)
        self.assertAlmostEqual(stats['median'], 0.0273157)
        self.assertAlmostEqual(stats['rms'], 16.47887508)
        self.assertAlmostEqual(stats['stddev'], 15.9185447)
        self.assertAlmostEqual(stats['variance'], 253.4000652)
        self.assertAlmostEqual(stats['quartile'], 0.0418118)
        print

    def test_range(self): 
        """test ms.range"""
        # Returns record (python dict)
        ranges = self.ms.range(["time","uvdist","amplitude","antenna1"])
        self.assertEqual(len(ranges['antenna1']), 27)
        self.assertAlmostEqual(ranges['time'][0], 4.30448154e+09, 1)
        self.assertAlmostEqual(ranges['time'][1], 4.30448682e+09, 1)
        self.assertAlmostEqual(ranges['uvdist'][0], 0.)
        self.assertAlmostEqual(ranges['uvdist'][1], 1020.32003137)
        self.assertAlmostEqual(ranges['amplitude'][0], 2.21305218e-05)
        self.assertAlmostEqual(ranges['amplitude'][1], 7.37500000e+01)
        print

    def test_lister(self): 
        """test ms.lister"""
        fname = 'lister.txt'
        self.assertTrue(self.ms.lister(field='2', antenna='1', correlation='RR', 
            scan='3', listfile=fname))
        self.checkFile(fname)
        f = open(fname)
        # we're not going to do any parsing...
        self.assertEqual(len(f.readlines()), 30784)
        f.close()
        self.removeFile('lister.txt')
        print

    def test_metadata(self): 
        """test ms.metadata"""
        # returns msmetadata
        msmd = self.ms.metadata()
        # just check ms.metadata example;
        # other msmetadata functions tested in test_msmd
        self.assertEqual(msmd.nspw(), 1)
        msmd.done()
        print

# ------------------------------------------------------------------------------

class mstool_test_select(mstool_test_base):

    def setUp(self):
        self.setUpTest()

    def tearDown(self):
        self.tearDownTest()

    def test_selectinit(self): 
        """test ms.selectinit"""
        self.assertTrue(self.ms.selectinit())
        # this MS only has one DDID so no selection done
        self.assertEqual(self.ms.nrow(), 22653)
        print

    def test_msselect(self): 
        """test ms.msselect, ms.msselectedindices, ms.reset"""
        staql={'field':'1445+0990*', 'scan':'3~5'}
        self.ms.msselect(staql)
        # fewer rows with selection
        self.assertEqual(self.ms.nrow(), 1890)
        # selected field 1
        self.assertEqual(self.ms.msselectedindices()['field'][0], 1)
        testing.assert_array_equal(self.ms.msselectedindices()['scan'], [3,4,5])
        # reset to orig MS - no selected indices
        self.ms.reset()
        self.assertEqual(self.ms.nrow(), 22653)
        self.assertEqual(self.ms.msselectedindices()['field'].size, 0)
        print

    def test_select(self): 
        """test ms.select"""
        self.assertEqual(len(self.ms.range(["antenna1"])["antenna1"]), 27)
        # select 3 antenna1
        antsel = [1,3,5]
        self.ms.select({'antenna1':antsel})
        # range for antenna is list of sel antennas, not min/max
        testing.assert_array_equal(self.ms.range(['antenna1'])['antenna1'], antsel)
        print

    def test_selecttaql(self): 
        """test ms.selecttaql"""
        self.ms.selectinit()
        self.ms.selecttaql('ANTENNA1==3')
        # selected 1 antenna1
        testing.assert_array_equal(self.ms.range(["antenna1"])['antenna1'], [3])
        print

    def test_selectchannel(self): 
        """test ms.selectchannel"""
        # original ms has 63 channels
        self.ms.selectinit()
        rec = self.ms.getdata(["axis_info"])
        self.assertEqual(len(rec["axis_info"]["freq_axis"]["chan_freq"]), 63)

        # average into 3 channels
        self.assertTrue(self.ms.selectchannel(3,2,5,3))
        rec = self.ms.getdata(["axis_info"])
        chan_freqs = rec["axis_info"]["freq_axis"]["chan_freq"]
        self.assertEqual(len(chan_freqs), 3)
        exp_freqs = array([[1.41276273e+09], [1.41283597e+09], [1.41290921e+09]])
        # test to -1 decimal places, i.e. to the nearest 1s place
        testing.assert_array_almost_equal(chan_freqs, exp_freqs, -1)

        # invalid selection
        print "\nTest invalid channel selection:"
        self.ms.selectinit(reset=True)
        self.assertFalse(self.ms.selectchannel(128,2,5,3))
        print

    def test_selectpolarization(self): 
        """test ms.selectpolarization"""
        # no selection: RR,LL 
        self.ms.selectinit()
        rec = self.ms.getdata(["axis_info"])
        testing.assert_array_equal(rec["axis_info"]["corr_axis"], ["RR", "LL"])
        # select one polarization RR
        self.ms.selectpolarization(["RR"])
        rec = self.ms.getdata(["axis_info"])
        testing.assert_array_equal(rec["axis_info"]["corr_axis"], ["RR"])
        print

    def test_msseltoindex(self): 
        """test ms.msseltoindex"""
        # select field id 2
        rec = self.ms.msseltoindex(self.testms, field="N*")
        self.assertEqual(rec['field'], [2])
        print

# ------------------------------------------------------------------------------

class mstool_test_transform(mstool_test_base):

    def setUp(self):
        self.setUpTest()

    def tearDown(self):
        self.tearDownTest()

    def test_regridspw(self): 
        """test ms.regridspw"""
        # Basic check, tested more fully in regridspw_test.py
        # See pre-regridspw values in test_getspectralwindowinfo
        self.ms.close()
        regridms = 'ngc5921_regrid.ms'
        shutil.copytree(self.testms, regridms)
        self.ms.open(regridms, False)
        self.assertTrue(self.ms.regridspw(mode="chan", center=24, chanwidth=2, bandwidth=32))
        spwinfo = self.ms.getspectralwindowinfo()['0']
        self.ms.close()
        self.removeMS(regridms)
        self.ms.open(self.testms) # prevent SEVERE ms::detached errors in cleanup
        # check values
        self.assertEqual(spwinfo['NumChan'], 16)  # was 63
        self.assertAlmostEqual(spwinfo['ChanWidth'], 48828.125) # was 24414.0625
        self.assertAlmostEqual(spwinfo['TotalWidth'], 781250.0) # was 1550196.875
        print

    def test_cvel(self): 
        """test ms.cvel"""
        self.ms.close()
        cvelms = 'ngc5921_cvel.ms'
        shutil.copytree(self.testms, cvelms)
        self.ms.open(cvelms, False)
        self.assertTrue(self.ms.cvel(mode="channel", width=2))
        spwinfo = self.ms.getspectralwindowinfo()['0']
        self.ms.close()
        self.removeMS(cvelms)
        self.ms.open(self.testms) # prevent SEVERE ms::detached errors in cleanup
        # check values
        self.assertEqual(spwinfo['NumChan'], 31)  # was 63
        self.assertAlmostEqual(spwinfo['ChanWidth'], 48828.125) # was 24414.0625
        self.assertAlmostEqual(spwinfo['TotalWidth'], 1513671.875) # was 1550196.875
        self.assertAlmostEqual(spwinfo['RefFreq'], 1412677280.8000255) # was 1413428013.2219005
        self.assertAlmostEqual(spwinfo['Chan1Freq'], 1412677280.8000255) # was 1412665073.7687755
        print

    def test_cvelfreqs(self): 
        """test ms.cvelfreqs"""
        self.ms.close()
        cvelms = 'ngc5921_cvelfreq.ms'
        shutil.copytree(self.testms, cvelms)
        self.ms.open(cvelms, False)
        centers = self.ms.cvelfreqs(mode="channel", width=2)
        self.ms.close()
        self.removeMS(cvelms)
        self.ms.open(self.testms) # prevent SEVERE ms::detached errors in cleanup
        # check len, first and last values
        self.assertEqual(len(centers), 31)
        self.assertAlmostEqual(centers[0], 1412677280.8000255)
        self.assertAlmostEqual(centers[30], 1414142124.5500255)
        print

    def test_statwt(self): 
        """test ms.statwt"""
        self.ms.close()
        statwtms = 'ngc5921_statwt.ms'
        shutil.copytree(self.testms, statwtms)
        self.ms.open(statwtms, False)
        self.assertTrue(self.ms.statwt(fitspw='0:0~32', field=[0], datacolumn='data'))
        rec = self.ms.getdata(['weight','sigma'])
        self.ms.close()
        self.removeMS(statwtms)
        self.ms.open(self.testms) # prevent SEVERE ms::detached errors in cleanup
        # orig weight=378.0, sigma=0.0514344
        self.assertAlmostEqual(rec['weight'][0][0], .0101575451)
        self.assertAlmostEqual(rec['sigma'][0][0], 9.9221458)
        print

    def test_continuumsub_uvsub(self): 
        """test ms.continuumsub, uvsub, contsub"""
        # Despite setUp/tearDown, tests were leaving MS in a changed state
        # and causing later tests to fail.  So make a unique one:
        self.ms.close()
        subms = 'ngc5921_sub.ms'
        shutil.copytree(self.testms, subms)
        self.ms.open(subms, False)
        # no model or corrected data in orig MS
        self.assertTrue(self.ms.continuumsub(field=0, fitspw='0:3~7', solint=0.0, fitorder=1, mode='sub'))
        rec = self.ms.getdata(['model_data', 'data'])
        self.assertEqual(rec['model_data'].shape, (2,63,22653))
        model0 = rec['model_data'][0][0][0]
        self.assertEqual(model0, (1+0j))
        data0 = rec['data'][0][0][0]
        self.assertAlmostEqual(data0, (3.15738129616+0j))
        # Now that we have model data, call uvsub
        self.assertTrue(self.ms.uvsub())
        rec = self.ms.getdata(['corrected_data'])
        corr0 = rec['corrected_data'][0][0][0]
        self.assertAlmostEqual(corr0, data0-model0)
        # call contsub
        contsubms = "contsub.ms"
        self.assertTrue(self.ms.contsub(contsubms, fitspw='0', 
            fitorder=0, field=[0], combine='spw'))
        self.ms.close()
        self.removeMS(subms)
        self.checkMS(contsubms)
        self.ms.open(contsubms)
        rec = self.ms.getdata(['data'])
        self.assertAlmostEqual(rec['data'][0][0][0], (-59.606513977050781+0j))
        self.ms.close()
        self.removeMS(contsubms)
        self.ms.open(self.testms) # prevent SEVERE ms::detached errors in cleanup
        print

    def test_hanningsmooth(self): 
        """test ms.hanningsmooth"""
        self.ms.close()
        smoothms = 'ngc5921_statwt.ms'
        shutil.copytree(self.testms, smoothms)
        self.ms.open(smoothms, False)
        self.assertTrue(self.ms.hanningsmooth('data'))
        rec = self.ms.getdata(['data'])
        self.ms.close()
        self.removeMS(smoothms)
        self.ms.open(self.testms) # prevent SEVERE ms::detached errors in cleanup
        # orig data was 3.15738129616+0j
        self.assertAlmostEqual(rec['data'][0][0][0], (14.5159635544+0j))
        print

    # No tests for these functions:
    def xtest_moments(self): 
        """test ms.moments"""
        # No test: this function does not exist in ms_cmpt.cc!
        print

    def xtest_addephemeris(self): 
        """test ms.addephemeris"""
        # No test: no ephemeris sources in test MS
        print

# ------------------------------------------------------------------------------

class mstool_test_dataIO(mstool_test_base):

    def setUp(self):
        self.setUpTest()

    def tearDown(self):
        self.tearDownTest()

    def test_getdata_putdata(self): 
        """test ms.getdata, putdata"""
        self.ms.close()
        putms = 'ngc5921_put.ms'
        shutil.copytree(self.testms, putms)
        self.ms.open(putms, False) # need writable MS
        # items list
        rec = self.ms.getdata(['data', 'antenna1', 'scan_number'])
        self.assertEqual(rec['antenna1'][1], 27)
        self.assertEqual(rec['scan_number'][1], 1)
        data0 = rec['data'][0][0][0]
        # double the data and write it into the MS
        doubleData = rec['data'] * 2
        doubleRec = {'data': doubleData}
        self.ms.putdata(doubleRec)
        # make sure it is 2x
        rec = self.ms.getdata(['data'])
        self.assertAlmostEqual(rec['data'][0][0][0], data0 * 2)
        self.ms.close()
        self.removeMS(putms)
        self.ms.open(self.testms) # prevent SEVERE ms::detached errors in cleanup
        print

    def test_getdata_args(self): 
        """test ms.getdata ifraxis, increment, and average"""
        ncorr = 2
        nchan = 63
        nrow = 22653
        nIfr = 378
        nAnt = 60
        # Expected data after averaging
        exp_data1 = [0.18245400-0.00021324j, 1.50447035-0.0020918j, 2.84147143-0.00377467j, 3.58339596-0.00475751j, 4.02027655-0.00478132j]
        exp_data2 = [2.49434376e+00+0.j, -1.52505504e-03-0.00459018j, 1.09913200e-03+0.00773078j, -4.95528942e-03-0.00457142j, 7.29188230e-03+0.00369013j]
        # ifraxis and average False
        rec = self.ms.getdata(['data'], ifraxis=False)
        self.assertEqual(rec['data'].shape, (ncorr, nchan, nrow))
        # ifraxis True
        rec = self.ms.getdata(['data', 'ifr_number'], ifraxis=True)
        self.assertEqual(rec['data'].shape, (ncorr, nchan, nIfr, nAnt))
        # increment 2
        rec = self.ms.getdata(['data'], increment=2)
        self.assertEqual(rec['data'].shape, (ncorr, nchan, nrow/2))
        # average True, ifraxis False - avg over row axis
        rec = self.ms.getdata(['data'], average=True)
        self.assertEqual(rec['data'].shape, (ncorr, nchan))
        testing.assert_array_almost_equal(rec['data'][0][:5], exp_data1, 8)
        # average True, ifraxis True - avg over time axis
        rec = self.ms.getdata(['data'], ifraxis=True, average=True)
        self.assertEqual(rec['data'].shape, (ncorr, nchan, nIfr))
        testing.assert_array_almost_equal(rec['data'][0][0][:5], exp_data2, 8)
        print

    def test_ngetdata(self): 
        """test ms.ngetdata"""
        # 'items' list is only argument implemented
        rec = self.ms.ngetdata(['data', 'flag'])
        self.assertAlmostEqual(rec['data'][0][0][0], (3.15738129616+0j))
        self.assertEqual(rec['flag'][0][0][0], 0)
        print

# ------------------------------------------------------------------------------

class mstool_test_sort(mstool_test_base):

    def setUp(self):
        # For these tests, no newmsname caused exception:
        # Exception Reported: Invalid Table operation: Cannot copy/rename;
        # target table <path>/nosedir/ngc5921.ms is still
        # open (is in the table cache)
        self.setUpTest()

    def tearDown(self):
        self.tearDownTest()

    def test_timesort(self): 
        """test ms.timesort"""
        rec0 = self.ms.getdata(['time'])
        timesortms = 'ngc5921_timesort.ms'
        self.assertTrue(self.ms.timesort(timesortms))
        self.ms.close()
        self.ms.open(timesortms)
        rec1 = self.ms.getdata(['time'])
        self.ms.close()
        self.removeMS(timesortms)
        self.ms.open(self.testms) # prevent SEVERE ms::detached errors in cleanup
        # ms is already time-sorted!
        testing.assert_array_equal(rec0['time'], rec1['time'])
        print

    def test_sort(self): 
        """test ms.sort"""
        rec = self.ms.getdata(['antenna1', 'antenna2'])
        self.assertEqual(rec['antenna1'][1], 27)
        self.assertEqual(rec['antenna2'][2], 7)
        sortms = 'ngc5921_sort.ms'
        self.assertTrue(self.ms.sort(sortms, ['ANTENNA1', 'ANTENNA2']))
        self.ms.close()
        self.ms.open(sortms)
        rec = self.ms.getdata(['antenna1', 'antenna2'])
        self.ms.close()
        self.removeMS(sortms)
        self.ms.open(self.testms) # prevent SEVERE ms::detached errors in cleanup
        # check that antennas were sorted
        self.assertEqual(rec['antenna1'][1], 0)
        self.assertEqual(rec['antenna2'][2], 0)
        print

# ------------------------------------------------------------------------------

class mstool_test_subms(mstool_test_base):

    def setUp(self):
        self.setUpTest()

    def tearDown(self):
        self.tearDownTest()

    def test_split(self): 
        """test ms.split"""
        self.assertEqual(self.ms.nrow(), 22653)
        self.assertTrue(self.ms.split('subms.ms', field=[0], step=[5]))
        self.checkMS('subms.ms')
        self.ms.close()
        self.ms.open('subms.ms')
        # fewer rows with field id 0 only
        self.assertEqual(self.ms.nrow(), 4509)
        rec = self.ms.getdata(['field_id'])
        self.ms.close()
        self.removeMS('subms.ms')
        self.ms.open(self.testms) # for teardown
        # check values
        field0 = rec['field_id']
        self.assertTrue((field0 == 0).all())
        print

    def test_split_averaging(self): 
        """test ms.split time averaging"""
        self.assertTrue(self.ms.split('subms.ms', timebin='120s'))
        self.checkMS('subms.ms')
        self.ms.close()
        self.ms.open('subms.ms')
        self.assertEqual(self.ms.nrow(), 9450)
        self.ms.close()
        self.removeMS('subms.ms')
        self.ms.open(self.testms) # for teardown
        print

    def test_partition(self): 
        """test ms.partition"""
        self.assertTrue(self.ms.partition('partition.ms', field=[0]))
        self.checkMS('partition.ms')
        self.ms.close()
        self.ms.open('partition.ms')
        # fewer rows with field id 0 only
        self.assertEqual(self.ms.nrow(), 4509)
        self.ms.close()
        self.removeMS('partition.ms')
        self.ms.open(self.testms) # for teardown
        print

    def xtest_partition_averaging(self): 
        """test ms.partition time averaging"""
        # TBD: test removed, timebin causes seg fault!
        self.assertTrue(self.ms.partition('partition.ms', timebin='120s'))
        self.checkMS('partition.ms')
        self.ms.close()
        self.ms.open('partition.ms')
        self.assertEqual(self.ms.nrow(), 4509)
        self.ms.close()
        self.removeMS('partition.ms')
        self.ms.open(self.testms) # for teardown
        print

# ------------------------------------------------------------------------------

class mstool_test_iter(mstool_test_base):

    def setUp(self):
        self.setUpTest()

    def tearDown(self):
        self.tearDownTest()

    def test_iteration(self): 
        """test ms.iter"""
        scannum = 1
        # rows per scan
        rows = [4509, 1890, 6048, 756, 1134, 6804, 1512]
        # iterate over scan number first
        # each getdata call should be the next scan
        self.ms.iterinit(["SCAN_NUMBER", "ARRAY_ID", "FIELD_ID", 
            "DATA_DESC_ID", "TIME"])
        self.ms.iterorigin()
        rec = self.ms.getdata(["scan_number", "data"])
        self.assertEqual(rec['scan_number'][0], scannum)
        self.assertEqual(rec["data"].shape[2], rows[scannum-1])
        while self.ms.iternext():
            scannum += 1
            rec = self.ms.getdata(["scan_number", "data"])
            self.assertEqual(rec['scan_number'][0], scannum)
            self.assertEqual(rec["data"].shape[2], rows[scannum-1])
        self.ms.iterend()
        self.assertEqual(scannum, 7)
        print

    def test_iteration_interval(self): 
        """test ms.iter interval parameter"""
        lasttime = 0.0
        interval = 120.0
        nchunk = 1
        # Select a scan to avoid time jumps between scans
        self.ms.msselect({'scan': '3'})
        times = self.ms.range(['time'])['time']
        scantime = times[1]-times[0]
        totalchunks = ceil(scantime/interval)
        self.ms.iterinit(interval=interval)
        self.ms.iterorigin() # chunk1
        while self.ms.iternext(): # next chunk
            nchunk += 1
            rec = self.ms.getdata(["time", "data"])
            thistime = rec["time"][0]
            if lasttime != 0.0:
                self.assertAlmostEqual(thistime - lasttime, interval, 2)
            lasttime = thistime
            self.assertEqual(rec["data"].shape[2], 756)
        self.ms.iterend()
        self.assertEqual(nchunk, totalchunks)
        print

    def test_iteration_rows(self): 
        """test ms.iter rows parameter"""
        maxrows = 256 
        nchunk = 1
        self.ms.msselect({'scan': '2'})
        rec = self.ms.getdata(['data'])
        totalrows = rec['data'].shape[2]
        totalchunks = ceil(float(totalrows)/maxrows)
        self.ms.iterinit(maxrows=maxrows)
        self.ms.iterorigin() # chunk1
        rec = self.ms.getdata(["data"])
        self.assertEqual(rec["data"].shape[2], maxrows)
        while self.ms.iternext(): # next chunk
            nchunk += 1
            # last chunk will be partial
            if nchunk < totalchunks:
                rec = self.ms.getdata(["data"])
                self.assertEqual(rec["data"].shape[2], maxrows)
        self.ms.iterend()
        self.assertEqual(nchunk, totalchunks)
        print

    def test_niteration(self): 
        """test ms.niter"""
        exp_rows = [0, 4509, 6399]
        got_rows = []
        self.ms.niterinit()
        self.ms.niterorigin()
        while not self.ms.niterend():
            rec = self.ms.ngetdata(['rows'])
            got_rows.append(rec['rows'][0])
            self.ms.niternext()
        self.assertEqual(got_rows, exp_rows)
        print

    def test_niteration_interval(self): 
        """test ms.niter interval parameter"""
        # Test interval
        self.ms.msselect({'scan': '3'})
        lasttime = 0.0
        interval = 120.0
        self.ms.niterinit(interval=interval)
        self.ms.niterorigin()
        while not self.ms.niterend():
            rec = self.ms.ngetdata(["time"])
            thistime = rec["time"][0]
            if lasttime != 0.0:
                self.assertAlmostEqual(thistime - lasttime, interval, 2)
            lasttime = thistime
            self.ms.niternext()
        print

    def test_niteration_rows(self): 
        """test ms.niter rows parameter"""
        # Note this does not work quite like old iter,
        # niternext gets next VB not next set of rows 
        # (i.e., there is only one VB so one iteration)
        # Also setting maxrows returns maxrows+1!
        maxrows = 256 
        self.ms.msselect({'scan': '2'})
        self.ms.niterinit(maxrows=maxrows)
        self.ms.niterorigin()
        while not self.ms.niterend():
            rec = self.ms.ngetdata(['rows'])
            self.assertEqual(len(rec['rows']), maxrows+1)
            self.ms.niternext()
        print

# ------------------------------------------------------------------------------
""" DEPRECATED *buffer methods since they depend on MSSelector

class mstool_test_buffer(mstool_test_base):

    def setUp(self):
        self.setUpTest()
        # select a subset of data
        self.ms.select({'antenna1':[3]})

    def tearDown(self):
        self.tearDownTest()

    def test_diff_clipbuffer(self): 
        # test ms.fillbuffer, diffbuffer, clipbuffer
        exp_aad = array([[ 4.37068081], [ 4.31520033]])
        exp_median = array([[ 4.6457572 ], [ 4.51209211]])
        self.assertTrue(self.ms.fillbuffer("DATA"))
        rec = self.ms.diffbuffer("TIME", 15)
        testing.assert_array_almost_equal(rec['aad'], exp_aad, 8)
        testing.assert_array_almost_equal(rec['median'], exp_median, 8)
        rec = self.ms.getbuffer()
        # at first, all flags are False
        self.assertEqual(len(where(rec['flag'][0][0])[0]), 0)
        self.ms.clipbuffer(10,10,10)
        rec = self.ms.getbuffer()
        # clip sets some flags
        self.assertEqual(len(where(rec['flag'][0][0])[0]), 60)
        print

    def test_get_clearbuffer(self): 
        # test ms.getbuffer, clearbuffer
        exp_phase1 = array([-1.694900, -1.604357, -0.766461, -2.658377, -2.548764, -1.703157,  2.388235, -2.540746,  2.701918, -1.691168])
        exp_phase2 = array([0.683892,-1.658363,-1.591494, 2.319796, 2.912622, 3.089270, 2.936095,-2.889212,-2.267109,-2.687929])
        # fill buffer with phase (no ifraxis)
        self.assertTrue(self.ms.fillbuffer("PHASE"))
        rec = self.ms.getbuffer()
        self.assertEqual(rec['phase'].shape, (2, 63, 1439))
        testing.assert_array_almost_equal(rec['phase'][0][0][30:40], exp_phase1, 6)
        # clear buffer
        self.ms.clearbuffer()
        rec = self.ms.getbuffer()  # empty dict, buffer is empty
        self.assertEqual(len(rec), 0)
        # add interferometer axis
        self.assertTrue(self.ms.fillbuffer("PHASE", True))
        rec = self.ms.getbuffer()
        self.assertEqual(rec['phase'].shape, (2, 63, 24, 60))
        testing.assert_array_almost_equal(rec['phase'][0][0][1][30:40], exp_phase2, 6)
        print

    def test_set_writebufferflags(self): 
        # test ms.setbufferflags, writebufferflags
        # Need writable MS for this test
        self.ms.close()
        rwbufferms = 'ngc5921_rw.ms'
        shutil.copytree(self.testms, rwbufferms)
        self.ms.open(rwbufferms, False)
        self.ms.fillbuffer("PHASE")
        rec = self.ms.getbuffer()
        self.assertFalse(rec['flag_row'][17])
        # set this flag row
        rec['flag_row'][17]=True
        self.ms.setbufferflags(rec)
        self.ms.writebufferflags()
        # check that flag set
        self.ms.clearbuffer()
        self.ms.fillbuffer("PHASE")
        rec = self.ms.getbuffer()
        self.assertTrue(rec['flag_row'][17])
        self.ms.close()
        self.removeMS(rwbufferms)
        self.ms.open(self.testms) # prevent SEVERE ms::detached errors in cleanup
        print
"""
# ------------------------------------------------------------------------------

class mstool_test_fits(mstool_test_base):

    def setUp(self):
        self.setUpTest()
        self.setUpFits()

    def tearDown(self):
        self.tearDownTest()
        self.removeFile(self.testfits)

    def test_list_from_tofits(self):
        """test ms.listfits, fromfits, tofits"""
        self.assertTrue(self.ms.listfits(self.testfits))
        # from fits to ms
        fitsms = 'fromfits.ms'
        testfits = "testfile.fits"
        self.assertTrue(self.ms.fromfits(fitsms, self.testfits))
        self.checkMS(fitsms)
        # from ms to fits
        self.ms.close() # self.testms
        self.ms.open(fitsms)
        self.assertTrue(self.ms.tofits(testfits, column="DATA", field=[0]))
        self.checkFile(testfits)
        # clean up
        self.ms.close() # fitsms
        self.removeMS(fitsms)
        self.removeFile(testfits)
        self.ms.open(self.testms) # for teardown
        print

# ------------------------------------------------------------------------------

def suite():
    return [mstool_test_ms,
            mstool_test_concat,
            mstool_test_summary,
            mstool_test_select,
            mstool_test_transform,
            mstool_test_dataIO,
            mstool_test_iter,
            #mstool_test_buffer,
            mstool_test_sort,
            mstool_test_subms,
            mstool_test_fits
            ]

