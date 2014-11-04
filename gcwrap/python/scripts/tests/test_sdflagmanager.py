import os
import sys
import shutil
import contextlib
import numpy

from __main__ import default
from tasks import *
from taskinit import *
import unittest
#
import listing
from numpy import array

import asap as sd
from sdflagmanager import sdflagmanager
from sdutil import tbmanager

@contextlib.contextmanager
def temporary_file():
    get_filename = lambda: ('temp%5s'%(numpy.random.random_integers(0,99999))).replace(' ','0')
    filename = get_filename()
    while os.path.exists(filename):
        filename = get_filename()
    yield filename
    if os.path.exists(filename):
        os.system('rm -rf %s'%(filename))
    
class sdflagmanager_test(unittest.TestCase):
    """
    Basic unit tests for task sdflagmanager.

    The list of tests:
    test00   --- testing the mode 'list'
    test01   --- testing the mode 'save'
    test02   --- testing the mode 'restore'
    test03   --- testing the mode 'delete'
    test04   --- testing the mode 'rename'
    """
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdflagmanager/'
    # Input and output names

    # sdflagmanager.asap flag status
    # ROW  1 (SCANNO 21 CYCLENO 0 IFNO 0 POLNO 1): FLAGROW 128
    # ROW  2 (SCANNO 21 CYCLENO 0 IFNO 1 POLNO 0): FLAGTRA all 128
    # ROW 32 (SCANNO 23 CYCLENO 0 IFNO 0 POLNO 0): FLAGROW 128
    # ROW 33 (SCANNO 23 CYCLENO 0 IFNO 0 POLNO 1): FLAGROW 128
    # ROW 59 (SCANNO 23 CYCLENO 3 IFNO 1 POLNO 1): FLAGTRA ch20~24 128
    infile = 'sdflagmanager.asap'
    vdir = infile+'.flagversions'
    vdatafileprefix = vdir+'/flags.'
    vlistfile = vdir+'/FLAG_VERSION_LIST'

    def setUp(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        shutil.copytree(self.datapath+self.infile, self.infile)
        if os.path.exists(self.vdir):
            shutil.rmtree(self.vdir)

        default(sdflagmanager)

    def tearDown(self):
        if os.path.exists(self.infile):
            shutil.rmtree(self.infile)
        if os.path.exists(self.vdir):
            shutil.rmtree(self.vdir)

    def test00(self):
        """Test 0: list"""
        infile = self.infile
        mode = "list"
        result = sdflagmanager(infile=infile,mode=mode)
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")

        items = self._get_version_list()
        self.assertEqual(len(items), 0, msg="The FLAG_VERSION_LIST must be empty.")

    def test01(self):
        """Test 1: save"""
        versionname = "v1"
        comment = "first_version"
        result = sdflagmanager(infile=self.infile,mode="save",versionname=versionname,comment=comment,merge="replace")
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")

        vdatafile = self.vdatafileprefix+versionname
        self.assertTrue(os.path.exists(vdatafile))

        self._verify_version(1, [versionname], [comment])

        self._verify_saved_flag(self.infile, vdatafile)

    def test02(self):
        """Test 2: restore"""
        versionname = "v1"
        comment = "first_version"
        result = sdflagmanager(infile=self.infile,mode="save",versionname=versionname,comment=comment,merge="replace")
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")

        expected_flagrow, expected_flag = self._get_flag_from_scantable(self.infile)

        result = sdflagmanager(infile=self.infile,mode="restore",versionname=versionname,merge="replace")
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")

        self._verify_version(1, [versionname], [comment])

        self._verify_restored_flag(self.infile, expected_flagrow, expected_flag)


    def test03(self):
        """Test 3: delete"""
        versionnames = ["v1", "v2"]
        comments = ["first_version", "second_version"]
        for versionname, comment in zip(versionnames, comments):
            result = sdflagmanager(infile=self.infile,mode="save",versionname=versionname,comment=comment,merge="replace")

        delvername = "v1"
        result = sdflagmanager(infile=self.infile,mode="delete",versionname=delvername)
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")

        delvdatafile = self.vdatafileprefix+delvername
        self.assertFalse(os.path.exists(delvdatafile))

        self._verify_version(1, versionnames[1:], comments[1:])

    def test04(self):
        """Test 4: rename"""
        oldname = "v1"
        oldcomment = "first_version"
        result = sdflagmanager(infile=self.infile,mode="save",versionname=oldname,comment=oldcomment,merge="replace")
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")

        newname = "renamed_v1"
        newcomment = "renamed_first_version"
        result = sdflagmanager(infile=self.infile,mode="rename",oldname=oldname,versionname=newname,comment=newcomment)
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")
        
        vdatafile = self.vdatafileprefix+newname
        self.assertTrue(os.path.exists(vdatafile))

        self._verify_version(1, [newname], [newcomment])

    def _get_version_list(self):
        def gen_version():
            with open(self.vlistfile, 'r') as f:
                for line in f:
                    yield map(lambda x: x.strip(), line.split(':'))
        return list(gen_version())

    def _verify_version(self, expected_nversion, expected_versions, expected_comments):
        self.assertEqual(len(expected_versions), expected_nversion, msg='Invalid arguments for _verify_version (expected_versions)')
        self.assertEqual(len(expected_comments), expected_nversion, msg='Invalid arguments for _verify_version (expected_comments)')
        
        items = self._get_version_list()
        if expected_nversion == 1:
            self.assertEqual(len(items), 1, msg="The FLAG_VERSION_LIST must contain just 1 version.")
        else:
            self.assertEqual(len(items), expected_nversion, msg="The FLAG_VERSION_LIST must contain %s versions."%(expected_nversion))
        for item, version, comment in zip(items, expected_versions, expected_comments):
            self.assertEqual(item[0], version, msg="Version name must be '%s'."%(version))
            self.assertEqual(item[1], comment, msg="Comment must be '%s'."%(comment))
        

    def _verify_saved_flag(self, infile, flagdatafile):
        # export infile to MS to obtain expected FLAG and FLAG_ROW
        with temporary_file() as name:
            s = sd.scantable(infile, average=False)
            s.save(name, format='MS2')
            with tbmanager(name) as tb:
                expected_flag_row = tb.getcol('FLAG_ROW')
                expected_flag = tb.getcol('FLAG')

        # actual FLAG and FLAG_ROW
        with tbmanager(flagdatafile) as tb:
            flag_row = tb.getcol('FLAG_ROW')
            flag = tb.getcol('FLAG')

        # compare
        self.assertEqual(len(flag_row), len(expected_flag_row), msg='length of FLAG_ROW differ')
        self.assertEqual(flag.shape, expected_flag.shape, msg='shape of FLAG differ')

        nrow = len(flag_row)
        for irow in xrange(nrow):
            self.assertEqual(flag_row[irow], expected_flag_row[irow], msg='Row %s: FLAG_ROW differ'%(irow))
            self.assertTrue(all(flag[:,:,irow].flatten() == expected_flag[:,:,irow].flatten()), msg='Row %s: FLAG differ'%(irow))

    def _verify_restored_flag(self, infile, expected_flagrow, expected_flag):
        flagrow, flag = self._get_flag_from_scantable(infile)
        self.assertEqual(len(flagrow), len(expected_flagrow), msg='length of FLAGROW differ')
        self.assertEqual(flag.shape, expected_flag.shape, msg='shape of FLAGTRA differ')

        nrow = len(flagrow)
        for irow in xrange(nrow):
            # effect of CAS-5545: FLAGROW is always 0
            #self.assertEqual(flagrow[irow] == 0, expected_flagrow[irow] == 0, msg='Row %s: FLAGROW differ (result %s, expected %s)'%(irow, flagrow[irow], expected_flagrow[irow]))
            self.assertEqual(flagrow[irow], 0, msg='Row %s: FLAGROW must be 0 (result %s)'%(irow, flagrow[irow]))
            # effect of CAS-5545: flag all channels if FLAG_ROW is True
            if expected_flagrow[irow] > 0:
                self.assertTrue(all(flag[:,irow].flatten() != 0), msg='Row %s: all channels must be flagged'%(irow))
            else:
                nonzero = flag[:,irow].nonzero()[0]
                expected_nonzero = expected_flag[:,irow].nonzero()[0]
                self.assertEqual(len(nonzero), len(expected_nonzero), msg='Row %s: FLAGTRA differ'%(irow))
                if len(nonzero) > 0:
                    self.assertTrue(all(nonzero == expected_nonzero), msg='Row %s: FLAGTRA differ'%(irow))
        
    def _get_flag_from_scantable(self, infile):
        with tbmanager(infile) as tb:
            tsort = tb.query('', sortlist='SCANNO,CYCLENO,IFNO,POLNO')
            flagrow = tsort.getcol('FLAGROW')
            flagtra = tsort.getcol('FLAGTRA')
            tsort.close()
        return flagrow, flagtra

def suite():
    return [sdflagmanager_test]
