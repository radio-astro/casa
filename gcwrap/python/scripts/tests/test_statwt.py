import os
import sys
import shutil
import commands
import numpy
import numpy.ma as ma
import random
from __main__ import default
from tasks import *
from taskinit import *
from cleanhelper import *
import unittest

'''
Unit tests for statwt
'''
#
# ToDo:
# add more tests
# once more independent tests (e.g. comparison
# the AIPS REWAY results) add reference mses
# and do tests against them
# 
class statwt_test(unittest.TestCase):

    # Input and output names
    msfile = 'ngc5921.ms'
    res = False

    def setUp(self):
        if (os.path.exists(self.msfile)):
            os.system('rm -rf ' + self.msfile)

        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/visstat/'
        shutil.copytree(datapath+self.msfile, self.msfile)

    def tearDown(self):
        if (os.path.exists(self.msfile)):
        #    os.system('rm -rf ' + self.msfile)
             pass

    def calcVariance(self,specData):
        """
        calculate variance of a single row of complex vis data
        input: specData - numpy masked array
        """
        dev2 = 0.0
        dmean = specData.mean()
        nchan=len(specData)
        for n in xrange(nchan):
          dev = specData[n] - dmean
          dev2 += dev*dev.conjugate()  
        var = (1./(nchan-1))*dev2
        return var.real
        
    def calcwt(self,selrow,selcorr,datcol,flagcol):
        """
        calc weight,sigma from the data
        """
        dsel = datcol[selcorr]
        fsel = flagcol[selcorr]
        dsel = dsel.transpose()
        fsel = fsel.transpose()
        dspec = dsel[selrow]
        flagc = fsel[selrow]
        mdspec = ma.masked_array(dspec,flagc)
        dmean = mdspec.mean()
        var = self.calcVariance(mdspec)
        sig = numpy.sqrt(var)
        rms = numpy.sqrt(dmean*dmean.conjugate()+var) 
        return (var,sig,rms)
    
    def test_default(self):
        """
        test default case
        """
        tol = 1.e-5
        self.res=statwt(vis=self.msfile)
        # need to have better checking....
        self.assertTrue(self.res)

        tb.open(self.msfile)
        datc=tb.getcol('DATA')
        wt = tb.getcol('WEIGHT')
        sg = tb.getcol('SIGMA')
        fg = tb.getcol('FLAG')
        nr = tb.nrows()
        tb.close()
       
        # Compare 10 randomly selected rows of weight and 
        # sigma columns with calculated statistics from the data
        random.seed()
        randomRowList=random.sample(xrange(nr),10)
        for i in random.sample(xrange(nr),10):
            icorr = random.randint(0,1)
            (v,s,r) = self.calcwt(i,icorr,datc,fg)
            diffwt = 1/v - wt[icorr][i]
            diffsg = s - sg[icorr][i]
            #print "w=",1/v, " wt[",icorr,"][",i,"]=",wt[icorr][i]
            #print "diffwt=",diffwt," diffwt/w=",abs(diffwt)*1/v
            self.assertTrue(abs(diffwt)/(1/v) < tol)
            self.assertTrue(abs(diffsg)/s < tol)

def suite():
    return [statwt_test]

if __name__ == '__main__':
    testSuite = []
    for testClass in suite():
        testSuite.append(unittest.makeSuite(testClass,'test'))
    allTests = unittest.TestSuite(testSuite)
    unittest.TextTestRunner(verbosity=2).run(allTests)
