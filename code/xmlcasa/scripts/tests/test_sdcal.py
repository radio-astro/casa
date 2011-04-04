import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
from asap_init import * 
import unittest
#
#import listing
#from numpy import array

asap_init()
import asap as sd
from sdcal import sdcal
#from sdstat import sdstat

class sdcal_test(unittest.TestCase):
    """
    Basic unit tests for task sdcal. No interactive testing.

    The list of tests:
    test00    --- default parameters (raises an errror)
    test01    --- Default parameters + valid input filename
    test02    --- operate all 3 steps (mostly with default parameters)
    test03    --- explicitly specify all parameters
    test04-07 --- do one of calibration, average, baseline, or smooth
    test08-10 --- skip one of of calibration and average, baseline, or smooth
    
    Note: input data (sdfile0) is generated from a single dish regression data,
    'OrionS_rawACSmod', as follows:
      default(sdsave)
      sdsave(sdfile='OrionS_rawACSmod',outfile=self.sdfile0,outform='ASAP')
    -> Just converted to scantable to eliminate errors by data conversion.
    """
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdcal/'
    # Input and output names
    # uncalibrated data
    sdfile0 = 'OrionS_rawACSmod.asap'
    # uncalibrated data
    sdfile1 = 'OrionS_rawACSmod_cal2123.asap'
    sdfiles = [sdfile0, sdfile1]
    outroot = 'sdcal_test'

    def setUp(self):
        for file in self.sdfiles:
            if os.path.exists(file):
                shutil.rmtree(file)
            shutil.copytree(self.datapath+file, file)
        default(sdcal)

    def tearDown(self):
        for file in self.sdfiles:
            if (os.path.exists(file)):
                shutil.rmtree(file)

    def _row0_stats(self,file):
        scan = sd.scantable(file,average=False)
        stats=["max","min","sum","rms","max_abc","min_abc"]
        statdict = {}
        for stat in stats:
            statdict[stat] = scan.stats(stat,row=0)
        del scan
        print "\nCurrent run: "+str(statdict)
        return statdict

    def _teststats0(self,teststat,refstat,places=4):
        for stat, testval in teststat.iteritems():
            self.assertAlmostEqual(testval[0],refstat[stat][0],places=places,
                                   msg = "'%s' is not in allowed range" % stat)

    def test00(self):
        """Test 0: Default parameters (raises an errror)"""
        #print blfunc
        result = sdcal()
        self.assertFalse(result)

    def test01(self):
        """Test 1: Default parameters + valid input filename (do nothing)"""
        self.tid="01"
        sdfile = self.sdfile0
        outfile = self.outroot+self.tid+'.asap'

        result = sdcal(sdfile=sdfile,outfile=outfile)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")
        refstat = {'rms': [0.62064331769943237], 'min': [0.0078176604583859444],
                   'max_abc': [0.0], 'max': [28.165399551391602],
                   'sum': [4133.86181640625], 'min_abc': [8189.0]}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)

    def test02(self):
        """
        Test 2: operate all steps (mostly with default parameters)
        testing if default parameter values are changed
        """
        # Don't average GBT ps data at the same time of calibration.
        self.tid="02"
        sdfile = self.sdfile0
        outfile = self.outroot+self.tid+'.asap'
        calmode = 'ps'
        kernel = 'hanning'
        maskmode='auto'


        result = sdcal(sdfile=sdfile,outfile=outfile,calmode=calmode,
                       kernel=kernel,maskmode=maskmode)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")

        refstat = {'rms': [0.21946178376674652], 'min': [-8.9328231811523438],
                   'max_abc': [22.0], 'max': [6.2358169555664062],
                   'sum': [19.246952056884766], 'min_abc': [18.0]}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)
        

    def test03(self):
        """
        Test 3:  explicitly specify all parameters
        testing if parameter names are changed
        """
        self.tid="03"
        sdfile = self.sdfile0
        outfile = self.outroot+self.tid+'.asap'

        result = sdcal(sdfile=sdfile,
                       antenna=0,
                       fluxunit='K',
                       telescopeparm='',
                       specunit='GHz',
                       frame='',
                       doppler='',
                       calmode='ps',
                       scanlist=[20,21,22,23],
                       field='OrionS*',
                       iflist=[0],
                       pollist=[],
                       channelrange=[],
                       average=False,
                       scanaverage=False,
                       timeaverage=False,
                       tweight='none',
                       averageall=False,
                       polaverage=False,
                       pweight='none',
                       tau=0.0,
                       kernel='hanning',
                       kwidth=5,
                       masklist=[],
                       maskmode='auto',
                       thresh=5.0,
                       avg_limit=4,
                       edge=[0],
                       blfunc='poly',
                       order=5,
                       npiece=2,
                       clipthresh=3.0,
                       clipniter=1,
                       nwave=3,
                       maxwavelength=1.0,
                       verifycal=False,
                       verifysm=False,
                       verifybl=False,
                       verbosebl=True,
                       outfile=outfile,
                       outform='ASAP',
                       overwrite=False,
                       plotlevel=0)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")

    def test04(self):
        """ Test 4:  operate only calibration step """
        self.tid="04"
        sdfile = self.sdfile0
        outfile = self.outroot+self.tid+'.asap'
        calmode = 'ps'

        result = sdcal(sdfile=sdfile,outfile=outfile,calmode=calmode)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")
        refstat = {'rms': [2.1405680179595947], 'min': [-8.1819267272949219],
                   'max_abc': [21.0], 'max': [9.528289794921875],
                   'sum': [17313.1484375], 'min_abc': [18.0]}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)

    def test05(self):
        """ Test 5:  operate only averaging step """
        self.tid="05"
        sdfile = self.sdfile1
        outfile = self.outroot+self.tid+'.asap'
        average = True
        # need to run one of average
        scanaverage = True

        result = sdcal(sdfile=sdfile,outfile=outfile,
                       average=average,scanaverage=scanaverage)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")

        refstat = {'rms': [4.1485457420349121], 'min': [-15.086404800415039],
                   'max_abc': [21.0], 'max': [18.760025024414062],
                   'sum': [33786.92578125], 'min_abc': [18.0]}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)

    def test06(self):
        """ Test 6:  operate only smoothing step """
        self.tid="06"
        sdfile = self.sdfile1
        outfile = self.outroot+self.tid+'.asap'
        kernel = 'hanning'

        result = sdcal(sdfile=sdfile,outfile=outfile,kernel=kernel)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")

        refstat = {'rms': [4.1414732933044434], 'min': [-10.916072845458984],
                   'max_abc': [22.0], 'max': [14.735917091369629],
                   'sum': [33782.40625], 'min_abc': [18.0]}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)

    def test07(self):
        """ Test 7:  operate only baseline step """
        self.tid="07"
        sdfile = self.sdfile1
        outfile = self.outroot+self.tid+'.asap'
        maskmode = 'auto'

        result = sdcal(sdfile=sdfile,outfile=outfile,maskmode=maskmode)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")

        refstat =  {'rms': [0.57502073049545288], 'min': [-17.474369049072266],
                    'max_abc': [21.0], 'max': [12.495612144470215],
                    'sum': [-36.998577117919922], 'min_abc': [18.0]}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)

    def test08(self):
        """ Test 8:  skip calibration and averaging """
        self.tid="08"
        sdfile = self.sdfile1
        outfile = self.outroot+self.tid+'.asap'
        calmode = 'none'
        average = False
        kernel = 'hanning'
        maskmode = 'auto'

        result = sdcal(sdfile=sdfile,outfile=outfile,calmode=calmode,
                       average=average,kernel=kernel,maskmode=maskmode)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")
        refstat = {'rms': [0.37114140391349792], 'min': [-15.10654354095459],
                   'max_abc': [22.0], 'max': [10.545536994934082],
                   'sum': [32.552036285400391], 'min_abc': [18.0]}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)

    def test09(self):
        """ Test 9:  skip smoothing"""
        self.tid="09"
        sdfile = self.sdfile0
        outfile = self.outroot+self.tid+'.asap'
        calmode = 'ps'
        kernel = 'none'
        maskmode = 'auto'

        result = sdcal(sdfile=sdfile,outfile=outfile,calmode=calmode,
                       kernel=kernel,maskmode=maskmode)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")

        refstat = {'rms': [0.33980178833007812], 'min': [-10.326268196105957],
                   'max_abc': [21.0], 'max': [7.384131908416748],
                   'sum': [-21.863819122314453], 'min_abc': [18.0]}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)

    def test10(self):
        """ Test 10:  skip baseline"""
        self.tid="10"
        sdfile = self.sdfile0
        outfile = self.outroot+self.tid+'.asap'
        calmode = 'ps'
        kernel = 'hanning'
        maskmode = 'none'

        result = sdcal(sdfile=sdfile,outfile=outfile,calmode=calmode,
                       kernel=kernel,maskmode=maskmode)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")

        refstat = {'rms': [2.4489636421203613], 'min': [-6.454887866973877],
                   'max_abc': [22.0], 'max': [8.7136993408203125],
                   'sum': [19976.408203125], 'min_abc': [18.0]}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)

def suite():
    return [sdcal_test]
