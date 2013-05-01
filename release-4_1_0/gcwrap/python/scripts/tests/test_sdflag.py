import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest

import asap as sd
from sdflag import sdflag

class sdflag_test(unittest.TestCase):
    """
    Basic unit tests for task sdflag.

    The list of tests:
    test00   --- test channel flagging/unflagging
    test01   --- test row flagging/unflagging
    test02   --- test clipping flagging/unflagging
    test03   --- test no data after selection (raises an exception)

    ***NOTE*** These tests are for Scantable only. Tests for the other formats
               which ASAP supports, including MS and SDFITS, are to be made later.

    NOTE2: 'Artificial_Flat.asap' has 6 flat spectra with rms of 1.0.

    """
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdflag/'
    # Input and output names
    infile = 'Artificial_Flat.asap'

    def setUp(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)

        default(sdflag)

    def tearDown(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)

    def test00(self):
        """Test 0: channel flagging"""
        infile = self.infile
        maskflag = [[1,3],[10,15]]

        #flag
        result = sdflag(infile=infile,maskflag=maskflag)

        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")

        scan = sd.scantable(filename=infile, average=False)
        ansmask = scan.create_mask(maskflag, invert=True)
        for i in range (scan.nrow()):
            mask = scan.get_mask(i)
            res = True
            for j in range(len(mask)):
                if mask[j] != ansmask[j]:
                    res = False
                    break
            self.assertTrue(res)
        del scan

        #unflag
        result = sdflag(infile=infile,maskflag=maskflag,flagmode="unflag")

        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")

        scan = sd.scantable(filename=infile, average=False)
        for i in range (scan.nrow()):
            mask = scan.get_mask(i)
            res = True
            for j in range(len(mask)):
                if mask[j] != True:
                    res = False
                    break
            self.assertTrue(res)
        del scan

    def test01(self):
        """Test 1: row flagging"""
        infile = self.infile
        flagrow = [2,4]

        #flag
        result = sdflag(infile=infile,flagrow=flagrow)

        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")

        scan = sd.scantable(filename=infile, average=False)
        for i in range (scan.nrow()):
            resrflag = scan._getflagrow(i)
            ansrflag = False
            for j in flagrow:
                if i == j:
                    ansrflag = True
                    break
            res = (resrflag == ansrflag)
            self.assertTrue(res)
        del scan

        #unflag
        result = sdflag(infile=infile,flagrow=flagrow,flagmode="unflag")

        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")

        scan = sd.scantable(filename=infile, average=False)
        for i in range (scan.nrow()):
            resrflag = scan._getflagrow(i)
            res = (resrflag == False)
            self.assertTrue(res)
        del scan

    def test02(self):
        """Test 2: clipping"""
        infile = self.infile
        clip = True
        clipminmax = [-3.5, 3.5] #clip at 3.5-sigma level, i.e., flag channels at which abs(value) exceeds 3.5.

        #flag
        result = sdflag(infile=infile,clip=clip,clipminmax=clipminmax)

        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")

        ansmlist = [
            [[0,301],[303,8191]],
            [[0,3326],[3328,7753],[7755,8191]],
            [[0,6545],[6547,8191]],
            [[0,3996],[3998,7920],[7922,8191]],
            [[0,3274],[3276,8191]],
            [[0,1708],[1710,7710],[7712,7799],[7801,8191]]
            ]

        scan = sd.scantable(filename=infile, average=False)
        for i in range (scan.nrow()):
            mask = scan.get_mask(i)
            ansmask = scan.create_mask(ansmlist[i])
            res = True
            for j in range(len(mask)):
                if mask[j] != ansmask[j]:
                    res = False
                    break
            self.assertTrue(res)
        del scan

        #unflag
        result = sdflag(infile=infile,clip=clip,clipminmax=clipminmax,flagmode="unflag")

        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")

        scan = sd.scantable(filename=infile, average=False)
        for i in range (scan.nrow()):
            mask = scan.get_mask(i)
            res = True
            for j in range(len(mask)):
                if mask[j] != True:
                    res = False
                    break
            self.assertTrue(res)
        del scan

    def test03(self):
        #test for the default parameters (raises an exception)
        infile = self.infile
        iflist = [10] # non-existent IF value
        maskflag = [0,3]
        try:
            result = sdflag(infile=infile, iflist=iflist, maskflag=maskflag)
        except Exception, e:
            pos = str(e).find('Selection contains no data. Not applying it.')
            self.assertNotEqual(pos, -1, msg='Unexpected exception was thrown: %s'%(str(e)))


def suite():
    return [sdflag_test]
