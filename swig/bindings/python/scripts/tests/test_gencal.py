import os
import sys
import shutil
import commands
import numpy
import numpy.ma as ma
from __main__ import default
from tasks import *
from taskinit import *
import unittest

'''
Unit tests for gencal 
'''
#
# ToDo:
# add more tests
# once more independent tests (e.g. comparison
# the AIPS REWAY results) add reference mses
# and do tests against them
# 
class gencal_antpostest(unittest.TestCase):

    # Input and output names
    msfile = 'tdem0003gencal.ms'
    caltable = 'anpos.cal'
    res = False

    def setUp(self):
        if (os.path.exists(self.msfile)):
            #os.system('rm -rf ' + self.msfile)
            shutil.rmtree(self.msfile)

        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/gencal/'
        #datapath='/export/home/tengu/alma/casatest/gencal_antoffset/testdata/'
        shutil.copytree(datapath+self.msfile, self.msfile)

    def tearDown(self):
        if (os.path.exists(self.msfile)):
            shutil.rmtree(self.msfile)
        shutil.rmtree(self.caltable,ignore_errors=True)

    def test_antpos_manual(self):
        """
        test manual antenna position correction 
        """
        gencal(vis=self.msfile,
               caltable=self.caltable, 
               caltype='antpos',
               antenna='ea12,ea22',
               parameter=[-0.0072,0.0045,-0.0017, -0.0220,0.0040,-0.0190])

        self.assertTrue(os.path.exists(self.caltable))

        # ToDo:check generated caltable. Wait for new caltable


    def test_antpos_auto(self):
        """
        test automated antenna position correction
        """
        # check if the URL is reachable
        import urllib2
        # current EVLA baseline correction URL
        evlabslncorrURL="http://www.vla.nrao.edu/cgi-bin/evlais_blines.cgi?Year="
        try: 
          urlaccess=urllib2.urlopen(evlabslncorrURL+"2010")
          gencal(vis=self.msfile,
                 caltable=self.caltable,
                 caltype='antpos',
                 antenna='',
                 parameter='')

          self.assertTrue(os.path.exists(self.caltable))
          
          # ToDo: check for generated caltable

        except urllib2.URLError, err:
          print "Cannot access %s , skip this test" % evlabslncorrURL
          self.res=True

def suite():
    return [gencal_antpostest]
