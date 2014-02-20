import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest
#
#import listing
#from numpy import array

import asap as sd
from tsdreduce import tsdreduce
#from sdstat import sdstat

class sdreduce_test(unittest.TestCase):
    """
    Basic unit tests for task sdreduce. No interactive testing.

    The list of tests:
    test00    --- default parameters (raises an errror)
    test01    --- Default parameters + valid input filename
    test02    --- operate all 3 steps (mostly with default parameters)
    test03    --- explicitly specify all parameters
    test04-07 --- do one of calibration, average, baseline, or smooth
    test08-10 --- skip one of of calibration and average, baseline, or smooth
    
    Note: input data (infile0) is generated from a single dish regression data,
    'OrionS_rawACSmod', as follows:
      default(sdsave)
      sdsave(infile='OrionS_rawACSmod',outfile=self.infile0,outform='ASAP')
    -> Just converted to scantable to eliminate errors by data conversion.
    """
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdreduce/'
    # Input and output names
    # uncalibrated data
    infile0 = 'OrionS_rawACSmod.asap'
    # uncalibrated data
    infile1 = 'OrionS_rawACSmod_cal2123.asap'
    infiles = [infile0, infile1]
    outroot = 'sdreduce_test'

    def setUp(self):
        for file in self.infiles:
            if os.path.exists(file):
                shutil.rmtree(file)
            shutil.copytree(self.datapath+file, file)
        default(tsdreduce)

    def tearDown(self):
        for file in self.infiles:
            if (os.path.exists(file)):
                shutil.rmtree(file)

    def _row0_stats(self,file):
        scan = sd.scantable(file,average=False)
        stats=["max","min","sum","rms","stddev","max_abc","min_abc"]
        edge = 500
        chanrange = [edge, scan.nchan()-edge-1]
        mask = scan.create_mask(chanrange)
        statdict = {}
        for stat in stats:
            statdict[stat] = scan.stats(stat,mask=mask,row=0)[0]
        del scan
        print "\nCurrent run: "+str(statdict)
        return statdict

    def _teststats0(self,teststat,refstat):
        for stat, refval in refstat.iteritems():
            self.assertTrue(teststat.has_key(stat),
                            msg = "'%s' is not defined in the current run" % stat)
            allowdiff = 0.01
            reldiff = (teststat[stat]-refval)/refval
            #print "Comparing '%s': %f (current run), %f (reference)" % \
            #      (stat,teststat[stat],refval)
            self.assertTrue(abs(reldiff) < allowdiff,\
                            msg="'%s' differs: %f (ref) != %f" % \
                            (stat, refval, teststat[stat]))

    def test00(self):
        """Test 0: Default parameters (raises an errror)"""
        #print blfunc
        result = tsdreduce()
        self.assertFalse(result)

    def test01(self):
        """Test 1: Default parameters + valid input filename (do nothing)"""
        self.tid="01"
        infile = self.infile0
        outfile = self.outroot+self.tid+'.asap'

        result = tsdreduce(infile=infile,outfile=outfile)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")
        refstat = {'rms': 0.55389463901519775, 'min': 0.26541909575462341,
                   'max_abc': 773.0, 'max': 0.91243284940719604,
                   'sum': 3802.1259765625, 'stddev': 0.16529126465320587,
                   'min_abc': 7356.0}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)

    def test02(self):
        """
        Test 2: operate all steps (mostly with default parameters)
        testing if default parameter values are changed
        """
        # Don't average GBT ps data at the same time of calibration.
        self.tid="02"
        infile = self.infile0
        outfile = self.outroot+self.tid+'.asap'
        calmode = 'ps'
        kernel = 'hanning'
        blfunc='poly'


        result = tsdreduce(infile=infile,outfile=outfile,calmode=calmode,
                       kernel=kernel,blfunc=blfunc)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")

        refstat = {'rms': 0.21985267102718353, 'min': -0.7019438743591309,
                   'max_abc': 4093.0, 'max': 0.9684028029441833,
                   'sum': 5.486020088195801, 'stddev': 0.21986639499664307,
                   'min_abc': 7623.0}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)
        

    def test03(self):
        """
        Test 3:  explicitly specify all parameters
        testing if parameter names are changed
        """
        self.tid="03"
        infile = self.infile0
        outfile = self.outroot+self.tid+'.asap'

        result = tsdreduce(infile=infile,
                          antenna=0,
                          fluxunit='K',
                          telescopeparam='',
                          #specunit='GHz',
                          field='OrionS*',
                          spw='0',
                          frame='',
                          doppler='',
                          timerange='',
                          scan='20~23',
                          pol='',
                          calmode='ps',
                          fraction='10%',
                          noff=-1,
                          width=0.5,
                          elongated=False,
                          markonly=False,
                          plotpointings=False,
                          tau=0.0,
                          average=False,
                          scanaverage=False,
                          timeaverage=False,
                          tweight='tintsys',
                          averageall=False,
                          polaverage=False,
                          pweight='tsys',
                          kernel='hanning',
                          kwidth=5,
                          chanwidth='5',
                          maskmode='auto',
                          thresh=5.0,
                          avg_limit=4,
                          edge=[0],
                          blfunc='poly',
                          order=5,
                          npiece=2,
                          applyfft=True,
                          fftmethod='fft',
                          fftthresh=3.0,
                          addwn=[0.],
                          rejwn=[],
                          clipthresh=3.0,
                          clipniter=1,
                          verifycal=False,
                          verifysm=False,
                          verifybl=False,
                          verbosebl=True,
                          bloutput=True,
                          blformat='',
                          showprogress=True,
                          minnrow=1000,
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
        infile = self.infile0
        outfile = self.outroot+self.tid+'.asap'
        calmode = 'ps'

        result = tsdreduce(infile=infile,outfile=outfile,calmode=calmode)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")
        refstat = {'rms': 2.1299083232879639, 'min': 1.2246102094650269,
                   'max_abc': 4093.0, 'max': 3.1902554035186768,
                   'sum': 15209.119140625, 'stddev': 0.25390961766242981,
                   'min_abc': 7434.0}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)

    def test05(self):
        """ Test 5:  operate only averaging step """
        self.tid="05"
        infile = self.infile1
        outfile = self.outroot+self.tid+'.asap'
        average = True
        # need to run one of average
        timeaverage = True
        tweight = 'tintsys'
        scanaverage = True

        result = tsdreduce(infile=infile,outfile=outfile,
                          average=average,timeaverage=timeaverage,
                          tweight = 'tintsys',scanaverage=scanaverage)

        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")
        refstat = {'rms': 4.128033638000488, 'min': 3.2304768562316895,
                   'max_abc': 4093.0, 'max': 5.676198482513428,
                   'sum': 29638.3515625, 'stddev': 0.24048194289207458,
                   'min_abc': 2452.0}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)

    def test06(self):
        """ Test 6:  operate only smoothing step """
        self.tid="06"
        infile = self.infile1
        outfile = self.outroot+self.tid+'.asap'
        kernel = 'hanning'

        result = tsdreduce(infile=infile,outfile=outfile,kernel=kernel)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")
        refstat = {'rms': 3.5979659557342529, 'min': 2.3542881011962891,
                   'max_abc': 4093.0, 'max': 5.2421674728393555,
                   'sum': 25737.166015625, 'stddev': 0.37295544147491455,
                   'min_abc': 6472.0}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)

    def test07(self):
        """ Test 7:  operate only baseline step """
        self.tid="07"
        infile = self.infile1
        outfile = self.outroot+self.tid+'.asap'
        blfunc = 'poly'

        result = tsdreduce(infile=infile,outfile=outfile,blfunc=blfunc)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")
        refstat = {'rms': 0.4292968213558197, 'min': -1.4878684282302856,
                   'max_abc': 4093.0, 'max': 1.8000496625900269,
                   'sum': 6.965803146362305, 'stddev': 0.42932620644569397,
                   'min_abc': 7434.0}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)

    def test08(self):
        """ Test 8:  skip calibration and averaging """
        self.tid="08"
        infile = self.infile1
        outfile = self.outroot+self.tid+'.asap'
        calmode = 'none'
        average = False
        kernel = 'hanning'
        blfunc = 'poly'

        result = tsdreduce(infile=infile,outfile=outfile,calmode=calmode,
                       average=average,kernel=kernel,blfunc=blfunc)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")
        refstat = {'rms': 0.37204012274742126, 'min': -1.1878470182418823,
                   'max_abc': 4093.0, 'max': 1.638755202293396,
                   'sum': 9.283474922180176, 'stddev': 0.37206435203552246,
                   'min_abc': 7623.0}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)

    def test09(self):
        """ Test 9:  skip smoothing"""
        self.tid="09"
        infile = self.infile0
        outfile = self.outroot+self.tid+'.asap'
        calmode = 'ps'
        kernel = 'none'
        blfunc = 'poly'

        result = tsdreduce(infile=infile,outfile=outfile,calmode=calmode,
                       kernel=kernel,blfunc=blfunc)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")

        refstat = {'rms': 0.2536879777908325, 'min': -0.8792380690574646,
                   'max_abc': 4093.0, 'max': 1.0637180805206299,
                   'sum': 4.116354942321777, 'stddev': 0.2537047564983368,
                   'min_abc': 7434.0}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)

    def test10(self):
        """ Test 10:  skip baseline"""
        self.tid="10"
        infile = self.infile0
        outfile = self.outroot+self.tid+'.asap'
        calmode = 'ps'
        kernel = 'hanning'
        blfunc = 'none'

        result = tsdreduce(infile=infile,outfile=outfile,calmode=calmode,
                       kernel=kernel,blfunc=blfunc)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                         msg="Output file '"+str(outfile)+"' doesn't exists")
        refstat = {'rms': 2.126171350479126, 'min': 1.3912382125854492,
                   'max_abc': 4093.0, 'max': 3.0977959632873535,
                   'sum': 15209.0869140625, 'stddev': 0.2203933447599411,
                   'min_abc': 6472.0}
        teststat = self._row0_stats(outfile)
        self._teststats0(teststat,refstat)

def suite():
    return [sdreduce_test]
