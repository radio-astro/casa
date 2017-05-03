import os
import sys
import shutil
import re
import numpy as np
from __main__ import default
from tasks import importvla
from taskinit import tbtool
from taskinit import qatool
import unittest

# local copy of the table tool
tblocal = tbtool()
qalocal = qatool()

# these tests use existing regression test data
regressionpath = os.environ.get('CASAPATH').split()[0] + "/data/regression/"

def checkms(msname, expectedNrows, antnamescheme='new', autocorrRows = 0):
    '''Check on the named MS.  Each table in expectedNrows dict should have
       the indicated nrows, "MAIN" refers to the main table.  If antnamescheme is
       'new' then the ANTENNA names should all be "VAxx" where xx is a digit, else
       for antnamescheme of 'old' the names are all just digits "xx".  autocorrRows
       gives the number of rows with ANTENNA1==ANTENNA2

       returns (T/F, status string) status string is non-empty when the return value is F'''
    
    # check the MAIN table first
    if not os.path.isdir(msname):
        status = "%s does not exist or is not a directory" % msname
        return (False,status)

    tblocal.open(msname)
    if tblocal.nrows() != expectedNrows["MAIN"]:
        status = "%s has an unexpected number of rows.  %d != %d" % (msname, tblocal.nrows(), expectedNrows["MAIN"])
        tblocal.close()
        return (False,status)

    # expected data columns exist
    mainCols = tblocal.colnames()
    for colName in ['DATA','MODEL_DATA','CORRECTED_DATA']:
        if colName not in mainCols:
            status = "%s is missing the %s column" % (msname, colName)
            tblocal.close()
            return (False,status)

    # expected number of auto correlation rows exist
    autoCorrMask = tblocal.getcol('ANTENNA1') == tblocal.getcol('ANTENNA2')
    if autoCorrMask.sum() != autocorrRows:
        status = "%s has an unexpected number of autocorrelation rows. %d != %d" % (msname, autoCorrMask.sum(), autocorrRows)
        tblocal.close()
        return (False,status)

    tblocal.close()

    for tabName in expectedNrows:
        if tabName != "MAIN":
            tabPath = msname + "/" + tabName
            if not os.path.isdir(tabPath):
                status = "%s is missing the %s table : %s" % (msname, tabName, tabPath)
                return (False,status)
            tblocal.open(tabPath)
            if tblocal.nrows() != expectedNrows[tabName]:
                status = "The %s table in %s has an unexpected number of rows. %d != %d" % (tabName, msname, tblocal.nrows(), expectedNrows[tabName])
                tblocal.close()
                return (False,status)
            if tabName == "ANTENNA":
                antNames = tblocal.getcol('NAME')
                if antnamescheme == "new":
                    antPat = '^VA\d+$'
                else:
                    antPat = '^\d+$'
                r = re.compile(antPat)
                vmatch = np.vectorize(lambda x:bool(r.match(x)))
                check = vmatch(antNames)
                if check.sum() != len(antNames):
                    status = "One or more of the ANTENNA names in %s does not follow the expected naming scheme (%s)." % (msname, antnamescheme)
                    tblocal.close()
                    return (False,status)
            tblocal.close()

    return (True,"")

def rmMS(msPath):
    '''Removes the name MS directory and all contents if present.  Also removes the
       related flagversions directory tree, <msPath>.flagversions, if present.'''
    if os.path.exists(msPath):
        shutil.rmtree(msPath)
    msFlagvers = msPath + ".flagversions"
    if os.path.exists(msFlagvers):
        shutil.rmtree(msFlagvers)

class importvla_test_1(unittest.TestCase):

    # these tests all use the 3C129 regression data
    datapath = regressionpath + "3C129/"
    msfile = "impvla_3c129_test.ms"

    def setUp(self):
        default(importvla)
        rmMS(self.msfile)

    def tearDown(self):
        rmMS(self.msfile)

    def test_defaults(self):
        '''Test defaults: default values, checks for expected tables with expected sizes,
           DATA, MODEL_DATA, and CORRECTED_DATA columns all exist in MAIN,
           The SCAN column contains the expected set of scans,
           the ANTENNA NAME values follow the "new" scheme: VAxx where xx are digits'''

        # use AT166_1 and AT166_3 but not AT166_2 so that multiple files are used
        # but the resulting MS is smaller and the test runs faster than it would
        # if all 3 are used.
        importvla(archivefiles=[self.datapath+'AT166_1',
                                self.datapath+'AT166_3'],
                  vis=self.msfile)

        expectedRows = {"MAIN":1509300,'SOURCE':8,'ANTENNA':47,'DATA_DESCRIPTION':8,'FEED':47,
                        'FLAG_CMD':0,'FIELD':8,'HISTORY':14,'OBSERVATION':2,'POINTING':0,
                        'POLARIZATION':1,'PROCESSOR':0,'SPECTRAL_WINDOW':8,'STATE':0,
                        'DOPPLER':8}

        (msOK, status) = checkms(self.msfile,expectedRows)
        self.assertTrue(msOK,'Check of MS failed : '+status)

    def test_timeTsys(self):
        '''test_timeTsys: time selection, 'old' antnamescheme, tests applytsys=False'''

        # use all 3 files as inputs, only a small number of rows are actually selected.
        importvla(archivefiles=[self.datapath+'AT166_1',
                                self.datapath+'AT166_2',
                                self.datapath+'AT166_3'],
                  vis=self.msfile, 
                  starttime='1994/7/25/07:34:00',
                  stoptime='1994/7/25/07:40:00',
                  antnamescheme='old')

        expectedRows = {"MAIN":25272,'SOURCE':1,'ANTENNA':27,'DATA_DESCRIPTION':4,'FEED':27,
                        'FLAG_CMD':0,'FIELD':1,'HISTORY':14,'OBSERVATION':1,'POINTING':0,
                        'POLARIZATION':1,'PROCESSOR':0,'SPECTRAL_WINDOW':4,'STATE':0,
                        'DOPPLER':4}

        (msOK, status) = checkms(self.msfile,expectedRows,antnamescheme='old')
        self.assertTrue(msOK,'Check of MS failed : ' + status)

        # check that the time column is within the expected limits
        tstartSeconds = qalocal.getvalue(qalocal.convert(qalocal.quantity('1994/07/25/07:34:00'),'s'))[0]
        tstopSeconds = qalocal.getvalue(qalocal.convert(qalocal.quantity('1994/07/25/07:40:00'),'s'))[0]

        tblocal.open(self.msfile)
        times = tblocal.getcol('TIME')
        # also get the DATA column for later comparison - should be safe for this small selection
        dataColWithTsysCorr = tblocal.getcol('DATA')
        tblocal.close()
        self.assertTrue((max(times) <= tstopSeconds) and (min(times) >= tstartSeconds), 'TIME values are outside of selected range')

        # refill with applytsys off
        rmMS(self.msfile)
        importvla(archivefiles=[self.datapath+'AT166_1',
                                self.datapath+'AT166_2',
                                self.datapath+'AT166_3'],
                  vis=self.msfile, 
                  starttime='1994/7/25/07:34:00',
                  stoptime='1994/7/25/07:40:00',
                  antnamescheme='old',applytsys=False)
        tblocal.open(self.msfile)
        dataColNoTsysCorr = tblocal.getcol('DATA')
        tblocal.close()
        dataDiff = dataColWithTsysCorr - dataColNoTsysCorr
        # only tests that they're different, not that they're correct
        self.assertTrue(dataDiff.sum() != 0j)

    def test_bandSel(self):
        '''test_bandSel: test bandname selection, U data from the 3C129 regression test data'''
        # use all 3 files as inputs, only a small number of rows are actually selected.
        importvla(archivefiles=[self.datapath+'AT166_1',
                                self.datapath+'AT166_2',
                                self.datapath+'AT166_3'],
                  vis=self.msfile, bandname='U')

        expectedRows = {"MAIN":490698,'SOURCE':5,'ANTENNA':27,'DATA_DESCRIPTION':2,'FEED':27,
                        'FLAG_CMD':0,'FIELD':5,'HISTORY':14,'OBSERVATION':2,'POINTING':0,
                        'POLARIZATION':1,'PROCESSOR':0,'SPECTRAL_WINDOW':2,'STATE':0,
                        'DOPPLER':2}

        (msOK, status) = checkms(self.msfile,expectedRows)
        self.assertTrue(msOK,('Check of MS failed : ' + status))

        # check that the REF_FREQUENCY is in the expected range
        tblocal.open(self.msfile+'/SPECTRAL_WINDOW')
        fref = tblocal.getcol('REF_FREQUENCY')
        tblocal.close()
        self.assertTrue(((fref >= 13.5e9) & (fref <= 16.3e9)).all(),'REF_FREQUENCY is outside of expected range (13.5-16.3 GHz) in %s.' % (self.msfile))
        
    def test_checkms(self):
        '''Internal test of checkms function, this should never fail.'''

        # use AT166_1 because it's the smallest data set, the fill isn't the important step here.
        # these are the true expected values, tweaked for the tests as necessary
        expectedRows = {"MAIN":49842,'SOURCE':1,'ANTENNA':27,'DATA_DESCRIPTION':8,'FEED':27,
                        'FLAG_CMD':0,'FIELD':1,'HISTORY':14,'OBSERVATION':1,'POINTING':0,
                        'POLARIZATION':1,'PROCESSOR':0,'SPECTRAL_WINDOW':8,'STATE':0,
                        'DOPPLER':8}
        # fill an ms to use in subsequent tests
        importvla(archivefiles=[self.datapath+'AT166_1'], vis=self.msfile)
        (msOK, status) = checkms(self.msfile,expectedRows)
        self.assertTrue(msOK,"initial fill failed "+status)

        print "internal checkms test, the following printed status messages are expected here and are not test errors."

        # bad msname
        (msOK, status) = checkms("__dummy__.ms",expectedRows)
        self.assertTrue(not msOK,'checkms test of bad msname failed to report the MS as bad')
        print status

        # bad number of rows in MAIN
        trueRows = expectedRows["MAIN"]
        expectedRows["MAIN"] = trueRows+1
        (msOK, status) = checkms(self.msfile,expectedRows)
        self.assertTrue(not msOK,'checkms test of wrong MAIN rows failed to report the MS as bad')
        expectedRows["MAIN"] = trueRows
        print status

        # bad autocorrRows value
        (msOK, status) = checkms(self.msfile,expectedRows,autocorrRows=1)
        self.assertTrue(not msOK,'checkms test of wrong autocorrRows value failed to report the MS as bad')
        print status

        # check for a table that doesn't exit
        badExpectedRows = dict(expectedRows)
        badExpectedRows["NotATable"] = 1
        (msOK, status) = checkms(self.msfile,badExpectedRows)
        self.assertTrue(not msOK,'checkms test for non-existant subtable failed to report the MS as bad')
        print status

        # check that the ANTENNA follows the "old" scheme when the "new" was actually used
        (msOK, status) = checkms(self.msfile,expectedRows,antnamescheme="old")
        self.assertTrue(not msOK,'checkms test for old antnamescheme failed to report the MS as bad')
        print status

        # check using an incorrect number of rows for one of the subtables
        badExpectedRows = dict(expectedRows)
        # intentionally picked a subtable that should have 0 rows
        badExpectedRows["FLAG_CMD"] = 1
        (msOK, status) = checkms(self.msfile,badExpectedRows)
        self.assertTrue(not msOK,'checkms test for bad number of FLAG_CMD rows failed to report the MS as bad')
        print status

class importvla_test_2(unittest.TestCase):

    # these tests all use the ATST1/G192 regression data
    datapath = regressionpath + "ATST1/G192/"
    msfile = "impvla_g192_test.ms"

    def setUp(self):
        default(importvla)
        rmMS(self.msfile)

    def tearDown(self):
        rmMS(self.msfile)

    def test_freqtol(self):
        '''test_freqtol: test frequency tolerance parameter'''

        # only use the xp5 data to limit size, but provide enough variation to test this parameter
        # first pass, wide frequencytol value that results in a single spectral window
        importvla(archivefiles=[self.datapath+'AS758_C030426.xp5'],
                  vis=self.msfile,bandname='K',frequencytol=10000000.0)

        expectedRows = {"MAIN":214825,'SOURCE':4,'ANTENNA':26,'DATA_DESCRIPTION':1,'FEED':26,
                        'FLAG_CMD':0,'FIELD':4,'HISTORY':14,'OBSERVATION':1,'POINTING':0,
                        'POLARIZATION':1,'PROCESSOR':0,'SPECTRAL_WINDOW':1,'STATE':0,
                        'DOPPLER':1}

        (msOK, status) = checkms(self.msfile,expectedRows)
        self.assertTrue(msOK,('Check of initial, wide frequencytol, MS failed : ' + status))
        
        # second pass, default frequencytol results in 2 SWs
        rmMS(self.msfile)
        importvla(archivefiles=[self.datapath+'AS758_C030426.xp5'],
                  vis=self.msfile,bandname='K')

        expectedRows = {"MAIN":214825,'SOURCE':4,'ANTENNA':26,'DATA_DESCRIPTION':2,'FEED':26,
                        'FLAG_CMD':0,'FIELD':4,'HISTORY':14,'OBSERVATION':1,'POINTING':0,
                        'POLARIZATION':1,'PROCESSOR':0,'SPECTRAL_WINDOW':2,'STATE':0,
                        'DOPPLER':2}

        (msOK, status) = checkms(self.msfile,expectedRows)
        self.assertTrue(msOK,('Check of default frequencytol, MS failed : ' + status))

    def test_autocorr(self):
        '''test_autocorr: test autocorr parameter'''

        # only use the xp1 data to limit size
        # first pass, default (no autocorrlation data)
        importvla(archivefiles=[self.datapath+'AS758_C030425.xp1'],
                  vis=self.msfile,bandname='K',frequencytol=10000000.0)

        expectedRows = {"MAIN":44928,'SOURCE':3,'ANTENNA':27,'DATA_DESCRIPTION':1,'FEED':27,
                        'FLAG_CMD':0,'FIELD':3,'HISTORY':14,'OBSERVATION':1,'POINTING':0,
                        'POLARIZATION':1,'PROCESSOR':0,'SPECTRAL_WINDOW':1,'STATE':0,
                        'DOPPLER':1}

        (msOK, status) = checkms(self.msfile,expectedRows)
        self.assertTrue(msOK,('Check of default autocorr, MS failed : ' + status))
        
        # second pass, autocorr=True
        rmMS(self.msfile)
        importvla(archivefiles=[self.datapath+'AS758_C030425.xp1'],
                  vis=self.msfile,bandname='K',frequencytol=10000000.0, autocorr=True)

        expectedRows = {"MAIN":48384,'SOURCE':3,'ANTENNA':27,'DATA_DESCRIPTION':1,'FEED':27,
                        'FLAG_CMD':0,'FIELD':3,'HISTORY':14,'OBSERVATION':1,'POINTING':0,
                        'POLARIZATION':1,'PROCESSOR':0,'SPECTRAL_WINDOW':1,'STATE':0,
                        'DOPPLER':1}

        # note that autocorrRows == new rows added to MAIN by setting autocorr=True
        (msOK, status) = checkms(self.msfile,expectedRows,autocorrRows=3456)
        self.assertTrue(msOK,('Check of default frequencytol, MS failed : ' + status))

def suite():
    return [importvla_test_1, importvla_test_2]
