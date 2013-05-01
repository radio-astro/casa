import os
import sys
import shutil
import subprocess
from __main__ import default
from tasks import *
from taskinit import *
import unittest

'''
Unit tests for task viewer. It tests the following parameters:
    infile:        image to load
    displaytype:   how the image should be displayed
    channel:       which channel of the cube should be displayed
    outdpi:        resolution of the output
    gui:           no gui should be displayed
'''
class viewer_test1(unittest.TestCase):

    # Input and output names
    img = 'test_image'

    def setUp(self):
        self.res = None
        self.chksum = None
        self.uname = os.uname()[0]
        if self.uname == 'Linux' :
            self.chksum = '/usr/bin/md5sum'
        elif self.uname == 'Darwin' :
            self.chksum = '/sbin/md5'
        if self.chksum is None or not os.path.exists(self.chksum) :
            raise RuntimeError("no md5 checksum program is available")
        default(viewer)
        if (os.path.exists(self.img)):
            os.system('rm -rf ' + self.img)
            
        datapath = os.environ.get('CASAPATH').split()[0] + '/data/demo/Images/'
        shutil.copytree(datapath+self.img, self.img)
    
    def tearDown(self):
        if (os.path.exists(self.img)):
            os.system('rm -rf ' + self.img)
            #os.system('rm -f test??.png')

    def getchecksum(self,img):
        if self.uname == 'Linux' :
            proc = subprocess.Popen(self.chksum + ' ' + img,stdout=subprocess.PIPE,shell=True)
            outstr = repr(proc.communicate()[0]).split()[0]
            return outstr.split("'")[1]
        elif self.uname == 'Darwin' :
            proc = subprocess.Popen(self.chksum + ' ' + img,stdout=subprocess.PIPE,shell=True)
            outstr = repr(proc.communicate()[0]).split()
            last_element = outstr[len(outstr)-1].split('\\')
            return last_element[0]
        return None

    def test1(self):
        '''Viewer 1: create png (default size, channel 0)'''
        outfile='test01.png'
        viewer(infile=self.img,outfile=outfile,gui=False)
        self.assertEqual(self.getchecksum(outfile),'e55d704ed0694d59dbf1fb0c01f299f8')
        
    def test2(self):
        '''Viewer 2: create png (default size, channel 3)'''
        outfile='test02.png'
        viewer(infile=self.img,outfile=outfile,channel=3,gui=False)
        self.assertEqual(self.getchecksum(outfile),'76ed16f68ca216a8dd34c0409fed554c')

    def test3(self):
        '''Viewer 3: create png (default size, channel 3, zoom=2)'''
        outfile='test03.png'
        viewer(infile=self.img,outfile=outfile,channel=3,zoom=2,gui=False)
        self.assertEqual(self.getchecksum(outfile),'994937a15c8e0949bf640aa575a04bd7')
        
    def test4(self):
        '''Viewer 4: create png (default size, channel 3, outscale=3)'''
        outfile='test04.png'
        viewer(infile=self.img,outfile=outfile,channel=3,outscale=3.0,gui=False)
        self.assertEqual(self.getchecksum(outfile),'1cc14fc0a82deb87755b5f45c0dcc352')
        

def suite():
    ## test requires X11 virtual frame buffer...
    if os.uname()[0] != 'Darwin' :
        return [viewer_test1]
    else:
        return [ ]

