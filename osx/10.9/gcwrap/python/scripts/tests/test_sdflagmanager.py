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
from sdflagmanager import sdflagmanager
from sdstatold import sdstatold

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
    infile = 'OrionS_rawACSmod_cal2123.asap'
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

        items = []
        for line in open(self.vlistfile, 'r'):
            items.append(line.split(' : '))
        self.assertEqual(len(items), 0, msg="The FLAG_VERSION_LIST must be empty.")

    def test01(self):
        """Test 1: save"""
        versionname = "v1"
        comment = "first_version"
        result = sdflagmanager(infile=self.infile,mode="save",versionname=versionname,comment=comment,merge="replace")
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")

        vdatafile = self.vdatafileprefix+versionname
        self.assertTrue(os.path.exists(vdatafile))
        
        items = []
        for line in open(self.vlistfile, 'r'):
            items.append(line.strip().split(' : '))
        self.assertEqual(len(items), 1, msg="The FLAG_VERSION_LIST must contain just 1 version.")

        res = (items[0][0] == versionname) and (items[0][1] == comment)
        self.assertEqual(res, True, msg="The version name is not saved correctly.")


    def test02(self):
        """Test 2: restore"""
        versionname = "v1"
        comment = "first_version"
        result = sdflagmanager(infile=self.infile,mode="save",versionname=versionname,comment=comment,merge="replace")
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")

        scan = sd.scantable(filename=self.infile, average=False)
        scan.flag(row=2)
        scan.flag_row(rows=[3])
        scan.save(self.infile, overwrite=True)
        del scan

        result = sdflagmanager(infile=self.infile,mode="restore",versionname=versionname,merge="replace")
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")

        scan = sd.scantable(filename=self.infile, average=False)
        mask = scan.get_mask(2)
        res = True
        for i in range(len(mask)):
            if not mask[i]:
                res = False
                break
        self.assertTrue(res)
        self.assertFalse(scan._getflagrow(3))
        del scan


    def test03(self):
        """Test 3: delete"""
        versionnames = ["v1", "v2"]
        comments = ["first_version", "second_version"]
        for i in range(len(versionnames)):
            result = sdflagmanager(infile=self.infile,mode="save",versionname=versionnames[i],comment=comments[i],merge="replace")

        delvername = "v1"
        result = sdflagmanager(infile=self.infile,mode="delete",versionname=delvername)
        self.assertEqual(result, None, msg="The task returned '"+str(result)+"' instead of None")

        delvdatafile = self.vdatafileprefix+delvername
        self.assertFalse(os.path.exists(delvdatafile))
        
        items = []
        for line in open(self.vlistfile, 'r'):
            items.append(line.strip().split(' : '))
        self.assertEqual(len(items), 1, msg="The FLAG_VERSION_LIST must contain just 1 version.")

        res = (items[0][0] == versionnames[1]) and (items[0][1] == comments[1])
        self.assertEqual(res, True, msg="The version name is not deleted correctly.")
        

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
        
        items = []
        for line in open(self.vlistfile, 'r'):
            items.append(line.strip().split(' : '))
        self.assertEqual(len(items), 1, msg="The FLAG_VERSION_LIST must contain just 1 version.")

        res = (items[0][0] == newname) and (items[0][1] == newcomment)
        self.assertEqual(res, True, msg="The version name is not renamed correctly.")



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





def suite():
    return [sdflagmanager_test]
