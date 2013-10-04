import shutil
import unittest
import os
from tasks import *
from taskinit import *

#
# Test of oldflagcmd task. It uses flagdata2 to unflag and summary
#

def test_eq(result, total, flagged):

    print "%s of %s data was flagged, expected %s of %s" % \
    (result['flagged'], result['total'], flagged, total)
    assert result['total'] == total, \
               "%s data in total; %s expected" % (result['total'], total)
    assert result['flagged'] == flagged, \
           "%s flags set; %s expected" % (result['flagged'], flagged)

def create_input(str_text):
    '''Save the string in a text file'''
    
    inp = 'oldflagcmd.txt'
    cmd = str_text
    
    # remove file first
    if os.path.exists(inp):
        os.system('rm -f '+ inp)
        
    # save it in a file    
    with open(inp, 'w') as f:
        f.write(cmd)
        
    f.close()
    
    # return the name of the file
    return inp
    
    
# Base class which defines setUp functions
# for importing different data sets
class test_base(unittest.TestCase):
    

    def setUp_multi(self):
        self.vis = "multiobs.ms"

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r ' + \
                      os.environ.get('CASAPATH').split()[0] +
                      "/data/regression/unittest/flagdata/" + self.vis + ' ' + self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        flagdata2(vis=self.vis, unflag=True)

    def setUp_flagdatatest_alma(self):
        self.vis = "flagdatatest-alma.ms"

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r ' + \
                      os.environ.get('CASAPATH').split()[0] +
                      "/data/regression/unittest/flagdata/" + self.vis + ' ' + self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        flagdata2(vis=self.vis, unflag=True)

        
class test_manualflag(test_base):
    '''Test manualflag selections'''
    
    def setUp(self):
        self.setUp_multi()
        
    def test_observation(self):
        input = "observation='1'"
        filename = create_input(input)
        
        oldflagcmd(vis=self.vis, flagmode='file', flagfile=filename, optype='apply')
        test_eq(flagdata2(vis=self.vis, summary=True), 2882778, 28500)

class test_selections_alma(test_base):
    # Test various selections for alma data 

    def setUp(self):
        self.setUp_flagdatatest_alma()
    
    def test_intent(self):
        '''oldflagcmd: test scan intent selection'''
        
        input = "intent='CAL*POINT*'"
        filename = create_input(input)
        
        # flag POINTING CALIBRATION scans 
        oldflagcmd(vis=self.vis, flagmode='file', flagfile=filename, optype='apply')
        test_eq(flagdata2(vis=self.vis,summary=True, selectdata=True, antenna='2'), 377280, 26200)
                
    
        
        
# Dummy class which cleans up created files
class cleanup(test_base):
    
    def tearDown(self):
        os.system('rm -rf multiobs.ms*')
        os.system('rm -rf flagdatatest-alma.ms*')


    def test1(self):
        '''oldflagcmd: Cleanup'''
        pass


def suite():
    return [test_manualflag,
            test_selections_alma,
            cleanup]
        
        
        
        
        
