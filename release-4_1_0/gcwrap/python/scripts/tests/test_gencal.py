import os
import sys
import shutil
import commands
import numpy
import numpy.ma as ma
import testhelper as th
from __main__ import default
from tasks import gencal
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

datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/gencal/'

# Pick up alternative data directory to run tests on MMSs
testmms = False
if os.environ.has_key('TEST_DATADIR'):   
    DATADIR = str(os.environ.get('TEST_DATADIR'))+'/gencal/'
    if os.path.isdir(DATADIR):
        testmms = True
        datapath = DATADIR
    else:
        print 'WARN: directory '+DATADIR+' does not exist'

print 'gencal tests will use data from '+datapath         


class gencal_antpostest(unittest.TestCase):

    # Input and output names
    msfile = 'tdem0003gencal.ms'
    if testmms:
        msfile = 'tdem0003gencal.mms'
    caltable = 'anpos.cal'
    reffile1 = datapath+'anpos.manual.cal'
    reffile2 = datapath+'anpos.auto.cal'
    res = False

    def setUp(self):
        if (os.path.exists(self.msfile)):
            shutil.rmtree(self.msfile)

        shutil.copytree(datapath+self.msfile, self.msfile, symlinks=True)

    def tearDown(self):
        if (os.path.exists(self.msfile)):
            shutil.rmtree(self.msfile)
            
        shutil.rmtree(self.caltable,ignore_errors=True)

    def test_antpos_manual(self):
        """
        gencal: test manual antenna position correction 
        """
        gencal(vis=self.msfile,
               caltable=self.caltable, 
               caltype='antpos',
               antenna='ea12,ea22',
               parameter=[-0.0072,0.0045,-0.0017, -0.0220,0.0040,-0.0190])

        self.assertTrue(os.path.exists(self.caltable))

        # ToDo:check generated caltable. Wait for new caltable
        
        # Compare with reference file from the repository
        reference = self.reffile1
        self.assertTrue(th.compTables(self.caltable, reference, ['WEIGHT','OBSERVATION_ID']))

    def test_antpos_auto(self):
        """
        gencal: test automated antenna position correction
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
          
          # Compare with reference file from the repository
          reference = self.reffile2
          self.assertTrue(th.compTables(self.caltable, reference, ['WEIGHT','OBSERVATION_ID']))

        except urllib2.URLError, err:
          print "Cannot access %s , skip this test" % evlabslncorrURL
          self.res=True

def suite():
    return [gencal_antpostest]
