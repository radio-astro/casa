import os
import string
import sys
import shutil
import unittest
from __main__ import default
from tasks import *
from taskinit import *

'''
Unit tests for task plotweather. It tests the following parameters:
    vis:                wrong and correct values
    seasonal_weight:    default (0.5) and other values
    doPlot:             default (True) and False
    plotName:           if output is created; test formats

    return value:       [opacity] (type='list')
'''

class plotweather_test(unittest.TestCase):
    # Input MS, must have WEATHER table
    datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/listobs/'
    msfile = datapath + 'nep2-shrunk.ms'
    msNoWeatherfile = datapath + 'ngc5921_ut.ms'
    # output plots
    fig = '/tmp/plotweathertest.png'
    defaultFig = msfile + ".plotweather.png"

    def setUp(self):
        default(plotweather)
    
    def tearDown(self):
        if os.path.exists(self.fig):
            os.remove(self.fig)
        if os.path.exists(self.defaultFig):
            os.remove(self.defaultFig)
        
    def test0(self):
        '''Test 0: Default parameters'''
        opac = plotweather()
        self.assertIsNone(opac)
       
    def test1(self):
        '''Test 1: Bad input file'''
        badmsfile = 'badfile.ms'
        opac = plotweather(vis=badmsfile)
        self.assertIsNone(opac)
        
    def test2(self):
        '''Test 2: ms with no weather, no plot '''
        opac = plotweather(vis=self.msNoWeatherfile, plotName=self.fig)
        self.assertIsNotNone(opac)
        self.assertAlmostEqual(opac[0], 0.0055838)
        self.assertFalse(os.path.exists(self.fig))
        
    def test2(self):
        '''Test 2: Good input file and output exists'''
        res = plotweather(vis=self.msfile, plotName=self.fig)
        self.assertIsNotNone(res)
        opac = res[0]/1e55
        self.assertAlmostEqual(opac, 1.3724789)
        self.assertTrue(os.path.exists(self.fig))

    def test3(self):
        '''Test 3: Good input file and no output exists'''
        res = plotweather(vis=self.msfile, doPlot=False)
        self.assertIsNotNone(res)
        opac = res[0]/1e55
        self.assertAlmostEqual(opac, 1.3724789)
        defaultFig = self.msfile + ".plotweather.png"
        self.assertFalse(os.path.exists(defaultFig))

    def test4(self):
        '''Test 4: seasonal_weight'''
        res = plotweather(vis=self.msfile, seasonal_weight=0.75, plotName=self.fig)
        self.assertIsNotNone(res)
        opac = res[0]/1e54
        self.assertAlmostEqual(opac, 6.8623946)
        self.assertTrue(os.path.exists(self.fig))

    def test5(self):
        '''Test 5: pdf output format'''
        plot = '/tmp/plotweathertest.pdf'
        opac = plotweather(vis=self.msfile, plotName=plot)
        self.assertTrue(os.path.exists(plot))
        os.remove(plot)

    def test6(self):
        '''Test 6: ps output format'''
        plot = '/tmp/plotweathertest.ps'
        opac = plotweather(vis=self.msfile, plotName=plot)
        self.assertTrue(os.path.exists(plot))
        os.remove(plot)

    def test7(self):
        '''Test 7: eps output format'''
        plot = '/tmp/plotweathertest.eps'
        opac = plotweather(vis=self.msfile, plotName=plot)
        self.assertTrue(os.path.exists(plot))
        os.remove(plot)
        
    def test8(self):
        '''Test 8: svg output format'''
        plot = '/tmp/plotweathertest.svg'
        opac = plotweather(vis=self.msfile, plotName=plot)
        self.assertTrue(os.path.exists(plot))
        os.remove(plot)
        
def suite():
    return [plotweather_test]

        
        
        
        
        

