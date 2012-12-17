import shutil
import unittest
import os
import filecmp
from tasks import *
from taskinit import *
from __main__ import default

#
# Test of flagdata modes
#

def test_eq(result, total, flagged):

    print "%s of %s data was flagged, expected %s of %s" % \
    (result['flagged'], result['total'], flagged, total)
    assert result['total'] == total, \
               "%s data in total; %s expected" % (result['total'], total)
    assert result['flagged'] == flagged, \
           "%s flags set; %s expected" % (result['flagged'], flagged)

def create_input(str_text, filename):
    '''Save the string in a text file'''
    
    inp = filename
    cmd = str_text
    
    # remove file first
    if os.path.exists(inp):
        os.system('rm -f '+ inp)
        
    # save to a file    
    with open(inp, 'w') as f:
        f.write(cmd)
        
    f.close()
    
    return

# Path for data
datapath = os.environ.get('CASAPATH').split()[0] + "/data/regression/unittest/flagdata/"

# Pick up alternative data directory to run tests on MMSs
testmms = False
if os.environ.has_key('TEST_DATADIR'):   
    DATADIR = str(os.environ.get('TEST_DATADIR'))+'/flagdata/'
    if os.path.isdir(DATADIR):
        testmms = True
        datapath = DATADIR

print 'mstransform tests will use data from '+datapath         


# Base class which defines setUp functions
# for importing different data sets
class test_base(unittest.TestCase):

    def setUp_ngc5921(self):
        self.vis = "ngc5921.ms"
        if testmms:
            self.vis = 'ngc5921.mms'

        if os.path.exists(self.vis):
            self.tearDown()
            
        os.system('cp -r '+datapath + self.vis +' '+ self.vis)
        default(mstransform)

    def tearDown(self):
        os.system('rm -rf '+ self.vis)

    def setUp_data4tfcrop(self):
        self.vis = "Four_ants_3C286.ms"
        if testmms:
            self.vis = 'Four_ants_3C286.mms'

        if os.path.exists(self.vis):
           self.tearDown()
            
        os.system('cp -r '+datapath + self.vis +' '+ self.vis)
        default(mstransform)
        
class mstrans_combspw(test_base):
    
    def setUp(self):
        self.setUp_data4tfcrop()
                
    def test_combspw1(self):
        '''mstransform: Combine two spws'''
        
        outputms = "combspw1.ms"
        mstransform(vis=self.vis, outputvis=outputms, combinespws=True, spw='0,1')
        self.assertTrue(os.path.exists(outputms))
        
 
# Cleanup class 
class cleanup(test_base):
    
    def tearDown(self):
        os.system('rm -rf ngc5921.*ms*')
        os.system('rm -rf Four_ants_3C286.*ms*')

    def test_runTest(self):
        '''mstransform: Cleanup'''
        pass


def suite():
    return [mstrans_combspw,
            cleanup]
