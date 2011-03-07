import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
from asap_init import * 
import unittest
#
import listing
from numpy import array

asap_init()
from sdbaseline import sdbaseline
from sdstat import sdstat

class sdbaseline_basictest(unittest.TestCase):
    """
    Basic unit test for task sdbaseline. No interactive testing.

    The list of tests:
    test00   --- default parameters (raises an error)
    test01   --- test polynominal baseline with masking = 'auto'
    test02   --- test polynominal baseline with masking = 'list'

    Note: input data is generated from a single dish regression data,
    'OrionS_rawACSmod', as follows:
      default(sdaverage)
      sdaverage(sdfile='OrionS_rawACSmod',scanlist=[20,21,22,23],
                calmode='ps',tau=0.09,outfile='temp.asap')
      default(sdaverage)
      sdaverage(sdfile='temp.asap',timeaverage=True,
                tweight='tintsys',outfile=self.sdfile)
    """
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdbaseline/'
    # Input and output names
    sdfile = 'OrionS_rawACSmod_calave.asap'
    outroot = 'sdbaseline_test'
    blrefroot = datapath+'refblparam'
    strefroot = datapath+'refstats'

    def setUp(self):
        if os.path.exists(self.sdfile):
            shutil.rmtree(self.sdfile)
        shutil.copytree(self.datapath+self.sdfile, self.sdfile)

        default(sdbaseline)

    def tearDown(self):
        if (os.path.exists(self.sdfile)):
            shutil.rmtree(self.sdfile)

    def test00(self):
        """Test 0: Default parameters"""
        result = sdbaseline()
        self.assertFalse(result)

    def testbl01(self):
        """Test 1: masking = 'auto'"""
        tid = "01"
        sdfile = self.sdfile
        outfile = self.outroot+tid+".asap"
        mode = "auto"
        iflist = [0,2]
        pollist=[0]
        result = sdbaseline(sdfile=sdfile,masking=mode,outfile=outfile,
                            blfunc='poly',iflist=iflist,pollist=pollist)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self._compareBLparam(outfile+"_blparam.txt",self.blrefroot+tid)
        reference = {'rms': [0.42423146963119507, 0.19752366840839386],
                     'min': [-19.465526580810547, -2.7562465667724609],
                     'max': [14.881180763244629, 2.0289773941040039],
                     'max_abscissa': {'value': array([   21.,  3045.]),
                                      'unit': 'channel'},
                     'min_abscissa': {'value': array([ 18.,   0.]),
                                      'unit': 'channel'},
                     'sum': [83.604629516601562, 129.33999633789062],
                     'mean': [0.010205643251538277, 0.015788573771715164]}
        self._compareStats(outfile,reference)
        #self._compareStats(outfile,self.strefroot+tid)


    def testbl02(self):
        """Test 2: masking = 'list' and masklist=[] (all channels)"""
        tid = "02"
        sdfile = self.sdfile
        outfile = self.outroot+tid+".asap"
        mode = "list"
        iflist = [2]
        pollist = [1]
        result = sdbaseline(sdfile=sdfile,masking=mode,outfile=outfile,
                            iflist=iflist,pollist=pollist)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self._compareBLparam(outfile+"_blparam.txt",self.blrefroot+tid)
        reference = {'rms': 3.4925737380981445,
                     'min': -226.3941650390625,'max': 129.78572082519531,
                     'max_abscissa': {'value': 8186., 'unit': 'channel'},
                     'min_abscissa': {'value': 8187., 'unit': 'channel'},
                     'sum': -0.0060577392578125,'mean': -7.3947012424468994e-07}
        self._compareStats(outfile,reference)
        #self._compareStats(outfile,self.strefroot+tid)


    def _compareBLparam(self,out,reference):
        # test if baseline parameters are equal to the reference values
        # currently comparing every lines in the files
        # TO DO: compare only "Fitter range" and "Baseline parameters"
        self.assertTrue(os.path.exists(out))
        self.assertTrue(os.path.exists(reference),
                        msg="Reference file doesn't exist: "+reference)
        self.assertTrue(listing.compare(out,reference),
                        'New and reference files are different. %s != %s. '
                        %(out,reference))

    def _compareStats(self,outfile,reference):
        # test if the statistics of baselined spectra are equal to
        # the reference values
        self.assertTrue(os.path.exists(outfile))
        currstat = sdstat(sdfile=outfile)
        self.assertTrue(isinstance(currstat,dict),
                        msg="Failed to calculate statistics.")
        f = open(outfile+'_stats','w')
        f.write(str(currstat))
        f.close()
        del f

        #self.assertTrue(os.path.exists(reference),
        #                msg="Reference file doesn't exist: "+reference)
        refstat = reference

        print "Statistics of baselined spectra:\n",currstat
        print "Reference values:\n",refstat
        # compare statistic values
        compstats = ['max','min','mean','sum','rms']
        allowdiff = 0.01
        self.assertEqual(currstat['max_abscissa']['unit'],
                         refstat['max_abscissa']['unit'],
                         msg="The units of max_abscissa are different")
        self.assertEqual(currstat['min_abscissa']['unit'],
                         refstat['min_abscissa']['unit'],
                         msg="The units of min_abscissa are different")
        if isinstance(refstat['max'],list):
            for i in xrange(len(refstat['max'])):
                for stat in compstats:
                    rdiff = (currstat[stat][i]-refstat[stat][i])/refstat[stat][i]
                    self.assertTrue((abs(rdiff)<allowdiff),
                                    msg="'%s' of spectrum %s are different." % (stat, str(i)))
                self.assertEqual(currstat['max_abscissa']['value'][i],
                                 refstat['max_abscissa']['value'][i],
                                 msg="The max channels/frequencies/velocities of spectrum %s are different" % str(i))
                self.assertEqual(currstat['min_abscissa']['value'][i],
                                 refstat['min_abscissa']['value'][i],
                                 msg="The min channels/frequencies/velocities of spectrum %s are different" % str(i))
        else:
            for stat in compstats:
                rdiff = (currstat[stat]-refstat[stat])/refstat[stat]
                self.assertTrue((abs(rdiff)<allowdiff),
                                msg="'%s' of spectrum %s are different." % (stat, str(0)))
            self.assertEqual(currstat['max_abscissa']['value'],
                             refstat['max_abscissa']['value'],
                             msg="The max channels/frequencies/velocities of spectrum %s are different" % str(0))
            self.assertEqual(currstat['min_abscissa']['value'],
                             refstat['min_abscissa']['value'],
                             msg="The min channels/frequencies/velocities of spectrum %s are different" % str(0))




class sdbaseline_masktest(unittest.TestCase):
    """
    Unit test for task sdbaseline. Test various mask selections.
    Polynominal baselining. No interactive testing.

    The list of tests:
    masktest01-02 --- test masklist (list)
    masktest03-04 --- test masklist (string)
    masktest05-08 --- test specunit='GHz'
    masktest09-12 --- test specunit='km/s'

    Note: input data is generated from a single dish regression data,
    'OrionS_rawACSmod', as follows:
      default(sdaverage)
      sdaverage(sdfile='OrionS_rawACSmod',scanlist=[20,21,22,23],
                calmode='ps',tau=0.09,outfile='temp.asap')
      default(sdaverage)
      sdaverage(sdfile='temp.asap',timeaverage=True,
                tweight='tintsys',outfile=self.sdfile)
    """
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdbaseline/'
    # Input and output names
    sdfile = 'OrionS_rawACSmod_calave.asap'
    outroot = 'sdbaseline_masktest'
    blrefroot = datapath+'refblparam_mask'
    #strefroot = datapath+'refstats_mask'
    tid = None

    # reference values for specunit='channel'
    ref_if02pol0 = {'rms': [0.42596656084060669, 0.20059973001480103],
                    'min': [-19.415897369384766, -2.8195438385009766],
                    'max': [14.930398941040039, 2.0193686485290527],
                    'sum': [18.466083526611328, 190.91133117675781],
                    'mean': [0.0022541605867445469,  0.023304605856537819]}
    ref_if2pol0 = {'rms': 0.20059973001480103, 'min': -2.8195438385009766,
                   'max': 2.0193686485290527, 'sum': 190.91133117675781,
                   'mean': 0.023304605856537819}
    # reference values for specunit='GHz'
    ref_if02pol0f = {'rms': [0.42488649487495422, 0.19945363700389862],
                     'min': [-19.564926147460938, -2.7968206405639648],
                     'max': [14.782135009765625, 2.0308547019958496],
                     'sum': [39.382259368896484, 171.57220458984375],
                     'mean': [0.0048074047081172466, 0.020943872630596161]}
    ref_if2pol0f = {'rms': 0.19945363700389862, 'min': -2.7968206405639648,
                    'max': 2.0308547019958496, 'sum': 171.57220458984375,
                    'mean': 0.020943872630596161}
    # reference values for specunit='GHz'
    ref_if02pol0v = {'rms': [0.42596647143363953, 0.19845835864543915],
                     'min': [-19.415897369384766, -2.7794866561889648],
                     'max': [14.930398941040039, 2.0269484519958496],
                     'sum': [18.462699890136719, 164.74856567382812],
                     'mean': [0.0022537475451827049, 0.020110908895730972]}
    ref_if2pol0v = {'rms': 0.19845835864543915, 'min': -2.7794866561889648,
                    'max': 2.0269484519958496, 'sum': 164.74856567382812,
                    'mean': 0.020110908895730972}
    
    def setUp(self):
        if os.path.exists(self.sdfile):
            shutil.rmtree(self.sdfile)
        shutil.copytree(self.datapath+self.sdfile, self.sdfile)

        default(sdbaseline)


    def tearDown(self):
        self._compareBLparam()
        if (os.path.exists(self.sdfile)):
            shutil.rmtree(self.sdfile)


    def testblmask01(self):
        """Mask test 1: test masklist (list) with masking = 'auto'"""
        self.tid="01"
        sdfile = self.sdfile
        outfile = self.outroot+self.tid+".asap"
        mode = "auto"
        masklist = [[200.,7599]]
        iflist = [0,2]
        pollist=[0]
        result = sdbaseline(sdfile=sdfile,masking=mode,masklist=masklist,
                            outfile=outfile,iflist=iflist,pollist=pollist)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self._compareStats(self.ref_if02pol0)

    def testblmask02(self):
        """Mask test 2: test masklist (list) with masking = 'list'"""
        self.tid="02"
        sdfile = self.sdfile
        outfile = self.outroot+self.tid+".asap"
        mode = "list"
        masklist = [[200,2959],[3120,7599]]
        iflist = [2]
        pollist=[0]
        result = sdbaseline(sdfile=sdfile,masking=mode,masklist=masklist,
                            outfile=outfile,iflist=iflist,pollist=pollist)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self._compareStats(self.ref_if2pol0)

    def testblmask03(self):
        """Mask test 3: test masklist (string) with masking = 'auto'"""
        self.tid="03"
        sdfile = self.sdfile
        outfile = self.outroot+self.tid+".asap"
        mode = "auto"
        masklist = "0~2:200~7599"
        iflist = [0,2]
        pollist=[0]
        result = sdbaseline(sdfile=sdfile,masking=mode,masklist=masklist,
                            outfile=outfile,iflist=iflist,pollist=pollist)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self._compareStats(self.ref_if02pol0)

    def testblmask04(self):
        """Mask test 4: test masklist (string) with masking = 'list'"""
        self.tid="04"
        sdfile = self.sdfile
        outfile = self.outroot+self.tid+".asap"
        mode = "list"
        masklist = "0:200~3979;4152~7599, 2:200~2959;3120~7599"
        iflist = [0,2]
        pollist=[0]
        result = sdbaseline(sdfile=sdfile,masking=mode,masklist=masklist,
                            outfile=outfile,iflist=iflist,pollist=pollist)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self._compareStats(self.ref_if02pol0)

    def testblmask05(self):
        """Mask test 5: test specunit='GHz' with masklist (list) and masking = 'auto'"""
        self.tid="05"
        sdfile = self.sdfile
        outfile = self.outroot+self.tid+".asap"
        mode = "auto"
        specunit = "GHz"
        masklist = [[44.0511472,44.0963125]]
        iflist = [2]
        pollist=[0]
        result = sdbaseline(sdfile=sdfile,masking=mode,masklist=masklist,
                            outfile=outfile,iflist=iflist,pollist=pollist,
                            specunit=specunit)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self._compareStats(self.ref_if2pol0f)

    def testblmask06(self):
        """Mask test 6: test specunit='GHz' with masklist (list) and masking = 'list'"""
        self.tid="06"
        sdfile = self.sdfile
        outfile = self.outroot+self.tid+".asap"
        mode = "list"
        specunit = "GHz"
        masklist = [[44.0511472,44.0679889],[44.0689716,44.0963125]]
        iflist = [2]
        pollist=[0]
        result = sdbaseline(sdfile=sdfile,masking=mode,masklist=masklist,
                            outfile=outfile,iflist=iflist,pollist=pollist,
                            specunit=specunit)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self._compareStats(self.ref_if2pol0f)

    def testblmask07(self):
        """Mask test 7: test specunit='GHz' with masklist (string) and masking = 'auto'"""
        self.tid="07"
        sdfile = self.sdfile
        outfile = self.outroot+self.tid+".asap"
        mode = "auto"
        specunit = "GHz"
        masklist = "0:45.4655714~45.5107367, 2:44.0511472~44.0963125"
        iflist = [0,2]
        pollist=[0]
        result = sdbaseline(sdfile=sdfile,masking=mode,masklist=masklist,
                            outfile=outfile,iflist=iflist,pollist=pollist,
                            specunit=specunit)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self._compareStats(self.ref_if02pol0f)

    def testblmask08(self):
        """Mask test 8: test specunit='GHz' with masklist (string) and masking = 'list'"""
        self.tid="08"
        sdfile = self.sdfile
        outfile = self.outroot+self.tid+".asap"
        mode = "list"
        specunit = "GHz"
        masklist = "0:45.4655714~45.4886394;45.4896954~45.5107367, 2:44.0511472~44.0679889;44.0689716~44.0963125"
        iflist = [0,2]
        pollist=[0]
        result = sdbaseline(sdfile=sdfile,masking=mode,masklist=masklist,
                            outfile=outfile,iflist=iflist,pollist=pollist,
                            specunit=specunit)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self._compareStats(self.ref_if02pol0f)

    def testblmask09(self):
        """Mask test 9: test specunit='km/s' with masklist (list) and masking = 'auto'"""
        self.tid="09"
        sdfile = self.sdfile
        outfile = self.outroot+self.tid+".asap"
        mode = "auto"
        specunit = "km/s"
        masklist = [[9186.458, 9484.109]]
        iflist = [2]
        pollist=[0]
        result = sdbaseline(sdfile=sdfile,masking=mode,masklist=masklist,
                            outfile=outfile,iflist=iflist,pollist=pollist,
                            specunit=specunit)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self._compareStats(self.ref_if2pol0v)

    def testblmask10(self):
        """Mask test 10: test specunit='km/s' with masklist (list) and masking = 'list'"""
        self.tid="10"
        sdfile = self.sdfile
        outfile = self.outroot+self.tid+".asap"
        mode = "list"
        specunit = "km/s"
        masklist = [[9186.458, 9366.642],[9373.118, 9484.109]]
        iflist = [2]
        pollist=[0]
        result = sdbaseline(sdfile=sdfile,masking=mode,masklist=masklist,
                            outfile=outfile,iflist=iflist,pollist=pollist,
                            specunit=specunit)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self._compareStats(self.ref_if2pol0v)

    def testblmask11(self):
        """Mask test 11: test specunit='km/s' with masklist (string) and masking = 'auto'"""
        self.tid="11"
        sdfile = self.sdfile
        outfile = self.outroot+self.tid+".asap"
        mode = "auto"
        specunit = "km/s"
        masklist = "0:-134.960~162.691, 2:9186.458~9484.109"
        iflist = [0,2]
        pollist=[0]
        result = sdbaseline(sdfile=sdfile,masking=mode,masklist=masklist,
                            outfile=outfile,iflist=iflist,pollist=pollist,
                            specunit=specunit)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self._compareStats(self.ref_if02pol0v)

    def testblmask12(self):
        """Mask test 12: test specunit='km/s' with masklist (string) and masking = 'list'"""
        self.tid="12"
        sdfile = self.sdfile
        outfile = self.outroot+self.tid+".asap"
        mode = "list"
        specunit = "km/s"
        masklist = "0:-134.960~3.708;10.667~162.691, 2:9373.118~9484.109;9186.458~9366.642"
        iflist = [0,2]
        pollist=[0]
        result = sdbaseline(sdfile=sdfile,masking=mode,masklist=masklist,
                            outfile=outfile,iflist=iflist,pollist=pollist,
                            specunit=specunit)
        # sdbaseline returns None if it runs successfully
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self._compareStats(self.ref_if02pol0v)


    def _compareBLparam(self):
        # test if baseline parameters are equal to the reference values
        # currently comparing every lines in the files
        # TO DO: compare only "Fitter range" and "Baseline parameters"
        out = self.outroot+self.tid+'.asap_blparam.txt'
        reference = self.blrefroot+self.tid
        self.assertTrue(os.path.exists(out))
        self.assertTrue(os.path.exists(reference),
                        msg="Reference file doesn't exist: "+reference)
        self.assertTrue(listing.compare(out,reference),
                        'New and reference files are different. %s != %s. '
                        %(out,reference))

    def _compareStats(self,reference):
        # test if the statistics of baselined spectra are equal to
        # the reference values
        outfile = self.outroot+self.tid+'.asap'
        self.assertTrue(os.path.exists(outfile))
        currstat = sdstat(sdfile=outfile)
        self.assertTrue(isinstance(currstat,dict),
                        msg="Failed to calculate statistics.")
        f = open(outfile+'_stats','w')
        f.write(str(currstat))
        f.close()
        del f
        
        #self.assertTrue(os.path.exists(reference),
        #                msg="Reference file doesn't exist: "+reference)
        refstat = reference

        print "Statistics of baselined spectra:\n",currstat
        print "Reference values:\n",refstat
        # compare statistic values
        compstats = ['max','min','mean','sum','rms']
        allowdiff = 0.01
        if isinstance(refstat['max'],list):
            for i in xrange(len(refstat['max'])):
                for stat in compstats:
                    rdiff = (currstat[stat][i]-refstat[stat][i])/refstat[stat][i]
                    self.assertTrue((abs(rdiff)<allowdiff),
                                    msg="'%s' of spectrum %s are different." % (stat, str(i)))
        else:
            for stat in compstats:
                rdiff = (currstat[stat]-refstat[stat])/refstat[stat]
                self.assertTrue((abs(rdiff)<allowdiff),
                                msg="'%s' of spectrum %s are different." % (stat, str(0)))


def suite():
    return [sdbaseline_basictest, sdbaseline_masktest]
