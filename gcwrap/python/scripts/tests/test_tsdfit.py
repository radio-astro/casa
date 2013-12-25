import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest
#
import listing
from numpy import array

import asap as sd
from tsdfit import tsdfit

class tsdfit_test(unittest.TestCase):
    """
    Unit tests for task tsdfit. No interactive testing.

    The list of tests:
    testGaussian00 --- test fitting a broad Gaussian profile (centre=4000, fwhm=1000, ampl=10)
    testGaussian01 --- test fitting a broad Gaussian profile (centre= 500, fwhm=1000, ampl=10) : on spectral edge
    testGaussian02 --- test fitting a narrow Gaussian profile (centre=4000, fwhm=100, ampl=10)
    testGaussian03 --- test fitting a combination of broad and narrow Gaussian profiles
                       (cen1=3000,fwhm1=1000,ampl1=10,cen2=6000,fwhm2=100,ampl2=10) : separated
    testGaussian04 --- test fitting a combination of broad and narrow Gaussian profiles
                       (cen1=4000,fwhm1=1000,ampl1=10,cen2=4700,fwhm2=100,ampl2=10) : overlapped
    testLorentzian00 --- test fitting a broad Lorentzian profile (centre=4000, fwhm=1000, ampl=10)
    testLorentzian01 --- test fitting a broad Lorentzian profile (centre= 500, fwhm=1000, ampl=10) : on spectral edge
    testLorentzian02 --- test fitting a narrow Lorentzian profile (centre=4000, fwhm=100, ampl=10)
    testLorentzian03 --- test fitting a combination of broad and narrow Lorentzian profiles
                       (cen1=3000,fwhm1=1000,ampl1=10,cen2=6000,fwhm2=100,ampl2=10) : separated
    testLorentzian04 --- test fitting a combination of broad and narrow Lorentzian profiles
                       (cen1=4000,fwhm1=1000,ampl1=10,cen2=4700,fwhm2=100,ampl2=10) : overlapped
    
    Note: (1) the rms noise is 1.0 for all data.

    created 21/04/2011 by Wataru Kawasaki
    """
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdfit/'
    # Input and output names
    infile_gaussian   = 'Artificial_Gaussian.asap'
    infile_lorentzian = 'Artificial_Lorentzian.asap'

    def setUp(self):
        if os.path.exists(self.infile_gaussian):
            shutil.rmtree(self.infile_gaussian)
        shutil.copytree(self.datapath+self.infile_gaussian, self.infile_gaussian)
        if os.path.exists(self.infile_lorentzian):
            shutil.rmtree(self.infile_lorentzian)
        shutil.copytree(self.datapath+self.infile_lorentzian, self.infile_lorentzian)

        default(tsdfit)

    def tearDown(self):
        if os.path.exists(self.infile_gaussian):
            shutil.rmtree(self.infile_gaussian)
        if os.path.exists(self.infile_lorentzian):
            shutil.rmtree(self.infile_lorentzian)

    def testGaussian00(self):
        """Test Gaussian00: single broad profile """
        infile = self.infile_gaussian
        scan = '0'
        fitfunc = "gauss"
        fitmode = "list"
        nfit = 1
        
        res = tsdfit(infile=infile,scan=scan,fitfunc=fitfunc,fitmode=fitmode,nfit=nfit)
        self.assertNotEqual(res, None, msg="The task returned None. Fit failed.")

        ans = {'cent': [[4000.0]],
               'fwhm': [[1000.0]],
               'peak': [[10.0]]}

        """ the result (RHEL5 64bit)
        ref = {'cent': [[[3997.420166015625, 2.2180848121643066]]],
               'fwhm': [[[1006.1046142578125, 5.2231903076171875]]],
               'nfit': [1],
               'peak': [[[9.9329404830932617, 0.044658195227384567]]]}
        """

        self.checkResult(res, ans)

    def testGaussian01(self):
        """Test Gaussian01: single broad profile on spectral edge"""
        infile = self.infile_gaussian
        scan = '1'
        spw = '0:0~2000'
        fitfunc = "gauss"
        fitmode = "list"
        #maskline = [0,2000]
        nfit = 1
        
        res = tsdfit(infile=infile,spw=spw,scan=scan,fitfunc=fitfunc,fitmode=fitmode,nfit=nfit)
        self.assertNotEqual(res, None, msg="The task returned None. Fit failed.")

        ans = {'cent': [[500.0]],
               'fwhm': [[1000.0]],
               'peak': [[10.0]]}

        """ the result (RHEL5 64bit)
        ref = {'cent': [[[504.638427734375, 2.7173392772674561]]],
               'fwhm': [[[998.78643798828125, 7.1386871337890625]]],
               'nfit': [1],
               'peak': [[[10.030097961425781, 0.047238241881132126]]]}
        """

        self.checkResult(res, ans)

    def testGaussian02(self):
        """Test Gaussian02: single narrow profile """
        infile = self.infile_gaussian
        scan = '2'
        fitfunc = "gauss"
        fitmode = "list"
        nfit = 1
        
        res = tsdfit(infile=infile,scan=scan,fitfunc=fitfunc,fitmode=fitmode,nfit=nfit)
        self.assertNotEqual(res, None, msg="The task returned None. Fit failed.")

        ans = {'cent': [[4000.0]],
               'fwhm': [[100.0]],
               'peak': [[10.0]]}

        """ the result (RHEL5 64bit)
        ref = {'cent': [[[3999.159912109375, 0.68400073051452637]]],
               'fwhm': [[[98.87506103515625, 1.6106985807418823]]],
               'nfit': [1],
               'peak': [[[9.9385099411010742, 0.14021013677120209]]]}
        """

        self.checkResult(res, ans)

    def testGaussian03(self):
        """Test Gaussian03: broad/narrow combination : separated """
        infile = self.infile_gaussian
        scan = '3'
        spw = '0:2000~4000;5500~6500'
        fitfunc = "gauss"
        fitmode = "list"
        #maskline = [[2000,4000],[5500,6500]]
        nfit = [1,1]
        
        res = tsdfit(infile=infile,spw=spw,scan=scan,fitfunc=fitfunc,fitmode=fitmode,nfit=nfit)
        self.assertNotEqual(res, None, msg="The task returned None. Fit failed.")

        ans = {'cent': [[3000.0, 6000.0]],
               'fwhm': [[1000.0, 100.0]],
               'peak': [[10.0, 10.0]]}

        """ the result (RHEL5 64bit)
        ref = {'cent': [[[2996.004638671875, 2.2644386291503906],
                         [5999.11181640625, 0.70802927017211914]]],
               'fwhm': [[[1001.549560546875, 5.4809303283691406],
                         [99.259437561035156, 1.6672815084457397]]],
               'nfit': [2],
               'peak': [[[9.899937629699707, 0.04574853926897049],
                         [9.9107418060302734, 0.14416992664337158]]]}
        """

        self.checkResult(res, ans)

    def testGaussian04(self):
        """Test Gaussian04: broad/narrow combination : overlapped """
        infile = self.infile_gaussian
        scan = '4'
        spw = '0:3000~4400;4500~5000'
        fitfunc = "gauss"
        fitmode = "list"
        #maskline = [[3000,4400],[4500,5000]]
        nfit = [1,1]
        
        res = tsdfit(infile=infile,spw=spw,scan=scan,fitfunc=fitfunc,fitmode=fitmode,nfit=nfit)
        self.assertNotEqual(res, None, msg="The task returned None. Fit failed.")

        ans = {'cent': [[4000.0, 4700.0]],
               'fwhm': [[1000.0, 100.0]],
               'peak': [[10.0, 10.0]]}

        """ the result (RHEL5 64bit)
        ref = {'cent': [[[4001.522216796875, 2.6332762241363525],
                         [4699.75732421875, 0.6802678108215332]]],
               'fwhm': [[[999.63507080078125, 6.4683256149291992],
                         [97.721427917480469, 1.7482517957687378]]],
               'nfit': [2],
               'peak': [[[9.9929990768432617, 0.04641139879822731],
                         [10.233022689819336, 0.15014420449733734]]]}
        """

        self.checkResult(res, ans)

    def testLorentzian00(self):
        """Test Lorentzian00: single broad profile """
        infile = self.infile_lorentzian
        scan = '0'
        fitfunc = "lorentz"
        fitmode = "list"
        nfit = 1
        
        res = tsdfit(infile=infile,scan=scan,fitfunc=fitfunc,fitmode=fitmode,nfit=nfit)
        self.assertNotEqual(res, None, msg="The task returned None. Fit failed.")

        ans = {'cent': [[4000.0]],
               'fwhm': [[1000.0]],
               'peak': [[10.0]]}

        """ the result (RHEL5 64bit)
        ref = {'cent': [[[3997.696044921875, 2.5651662349700928]]],
               'fwhm': [[[1010.3181762695312, 7.2803301811218262]]],
               'nfit': [1],
               'peak': [[[9.9210958480834961, 0.05041566863656044]]]}
        """

        self.checkResult(res, ans)

    def testLorentzian01(self):
        """Test Lorentzian01: single broad profile on spectral edge"""
        infile = self.infile_lorentzian
        scan = '1'
        spw = '0:0~2000'
        fitfunc = "lorentz"
        fitmode = "list"
        #maskline = [0,2000]
        nfit = 1
        
        res = tsdfit(infile=infile,spw=spw,scan=scan,fitfunc=fitfunc,fitmode=fitmode,nfit=nfit)
        self.assertNotEqual(res, None, msg="The task returned None. Fit failed.")

        ans = {'cent': [[500.0]],
               'fwhm': [[1000.0]],
               'peak': [[10.0]]}
        
        """ the result (RHEL5 64bit)
        ref = {'cent': [[[500.99105834960938, 2.8661653995513916]]],
               'fwhm': [[[995.85455322265625, 9.5194911956787109]]],
               'nfit': [1],
               'peak': [[[10.041034698486328, 0.053434751927852631]]]}
        """

        self.checkResult(res, ans)

    def testLorentzian02(self):
        """Test Lorentzian02: single narrow profile """
        infile = self.infile_lorentzian
        scan = '2'
        fitfunc = "lorentz"
        fitmode = "list"
        nfit = 1
        
        res = tsdfit(infile=infile,scan=scan,fitfunc=fitfunc,fitmode=fitmode,nfit=nfit)
        self.assertNotEqual(res, None, msg="The task returned None. Fit failed.")

        ans = {'cent': [[4000.0]],
               'fwhm': [[100.0]],
               'peak': [[10.0]]}

        """ the result (RHEL5 64bit)
        ref = {'cent': [[[3999.230224609375, 0.79903918504714966]]],
               'fwhm': [[[102.48796081542969, 2.2600326538085938]]],
               'nfit': [1],
               'peak': [[[9.9708395004272461, 0.1554737389087677]]]}
        """

        self.checkResult(res, ans)

    def testLorentzian03(self):
        """Test Lorentzian03: broad/narrow combination : separated """
        infile = self.infile_lorentzian
        scan = '3'
        spw = '0:2000~4000;5500~6500'
        fitfunc = "lorentz"
        fitmode = "list"
        #maskline = [[2000,4000],[5500,6500]]
        nfit = [1,1]
        
        res = tsdfit(infile=infile,spw=spw,scan=scan,fitfunc=fitfunc,fitmode=fitmode,nfit=nfit)
        self.assertNotEqual(res, None, msg="The task returned None. Fit failed.")

        ans = {'cent': [[3000.0, 6000.0]],
               'fwhm': [[1000.0, 100.0]],
               'peak': [[10.0, 10.0]]}

        """ the result (RHEL5 64bit)
        ref = {'cent': [[[3001.23876953125, 2.5231325626373291],
                         [5999.01953125, 0.82874661684036255]]],
               'fwhm': [[[990.19671630859375, 8.1528301239013672],
                         [102.10212707519531, 2.3521277904510498]]],
               'nfit': [2],
               'peak': [[[9.9958734512329102, 0.051685664802789688],
                         [9.6133279800415039, 0.1561257392168045]]]}
        """

        self.checkResult(res, ans)

    def testLorentzian04(self):
        """Test Lorentzian04: broad/narrow combination : overlapped """
        infile = self.infile_lorentzian
        scan = '4'
        spw = '0:3000~4400;4500~5000'
        fitfunc = "lorentz"
        fitmode = "list"
        #maskline = [[3000,4400],[4500,5000]]
        nfit = [1,1]
        
        res = tsdfit(infile=infile,spw=spw,scan=scan,fitfunc=fitfunc,fitmode=fitmode,nfit=nfit)
        self.assertNotEqual(res, None, msg="The task returned None. Fit failed.")

        ans = {'cent': [[4000.0, 4700.0]],
               'fwhm': [[1000.0, 100.0]],
               'peak': [[10.0, 10.0]]}
        
        """ the result (RHEL5 64bit)
        ref = {'cent': [[[3995.85693359375, 3.0016641616821289],
                         [4699.53271484375, 0.82658475637435913]]],
               'fwhm': [[[972.22833251953125, 10.149419784545898],
                         [104.71010589599609, 2.7239837646484375]]],
               'nfit': [2],
               'peak': [[[10.013784408569336, 0.053735069930553436],
                         [9.9273672103881836, 0.15813499689102173]]]}
        """

        self.checkResult(res, ans)

    def checkResult(self, result, answer):
        for key in ['cent', 'fwhm', 'peak']:
            for i in range(len(result[key][0])):
                val = result[key][0][i][0]
                err = result[key][0][i][1]
                ans = answer[key][0][i]

                #check if result is consistent with answer in 3-sigma level
                threshold = 3.0
                
                within_errorrange = (abs(ans - val) <= abs(err * threshold))
                self.assertTrue(within_errorrange)

class tsdfit_test_exceptions(unittest.TestCase):
    """
    test the case when tsdfit throws exception.
    """
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdfit/'
    # Input and output names
    infile_gaussian   = 'Artificial_Gaussian.asap'

    def setUp(self):
        if os.path.exists(self.infile_gaussian):
            shutil.rmtree(self.infile_gaussian)
        shutil.copytree(self.datapath+self.infile_gaussian, self.infile_gaussian)
        default(tsdfit)

    def tearDown(self):
        if os.path.exists(self.infile_gaussian):
            shutil.rmtree(self.infile_gaussian)

    def testNoData(self):
        try:
            res = tsdfit(infile=self.infile_gaussian,spw='99')
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Selection contains no data. Not applying it.')
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

def suite():
    return [tsdfit_test, tsdfit_test_exceptions]
