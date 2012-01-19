import shutil
import unittest
import os
from tasks import *
from taskinit import *

#
# Test of tflagcmd task. It uses tflagger to unflag and summary
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
    
    inp = 'tflagcmd.txt'
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
    
    def setUp_ngc5921(self):
        self.vis = "ngc5921.ms"

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Importing data..."
            importuvfits(os.environ.get('CASAPATH').split()[0] + \
                         '/data/regression/ngc5921/ngc5921.fits', \
                         self.vis)
        os.system('rm -rf ' + self.vis + '.flagversions')
        tflagger(vis=self.vis, mode='unflag')

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
        tflagger(vis=self.vis, mode='unflag')

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
        tflagger(vis=self.vis, mode='unflag')

        
class test_manualflag(test_base):
    '''Test manualflag selections'''
    
    def setUp(self):
        self.setUp_multi()
        
    def test_observation(self):
        input = "observation='1'"
        filename = create_input(input)
        
        tflagcmd(vis=self.vis, inputmode='file', inputfile=filename, action='apply')
        test_eq(tflagger(vis=self.vis, mode='summary'), 2882778, 28500)

class test_selections_alma(test_base):
    # Test various selections for alma data 

    def setUp(self):
        self.setUp_flagdatatest_alma()
    
    def test_intent(self):
        '''tflagcmd: test scan intent selection'''
        
        input = "intent='CAL*POINT*'"
        filename = create_input(input)
        
        # flag POINTING CALIBRATION scans 
        tflagcmd(vis=self.vis, inputmode='file', inputfile=filename, action='apply')
        test_eq(tflagger(vis=self.vis,mode='summary', antenna='2'), 377280, 26200)
                
class test_unapply(test_base):
    # Action unapply
    def setUp(self):
        self.setUp_ngc5921()

    def test_utfcrop(self):
        '''tflagcmd: unapply tfcrop agent'''
        # Remove any cmd from table
        tflagcmd(vis=self.vis, action='clear', clearall=True)

        # Flag using manualflag agent
        input = "scan=1"
        filename = create_input(input)
        tflagcmd(vis=self.vis, inputmode='file', inputfile=filename, action='apply')
        
        # Flag using tfcrop agent
        input = "scan=3 mode=tfcrop expression='ABS_RR'"
        filename = create_input(input)
        tflagcmd(vis=self.vis, inputmode='file', inputfile=filename, action='apply')
        test_eq(tflagger(vis=self.vis,mode='summary',scan='1'), 568134, 568134)
        test_eq(tflagger(vis=self.vis,mode='summary',scan='3'), 762048, 2829)
        
        # Unapply only the tfcrop line
        tflagcmd(vis=self.vis, action='unapply', useapplied=True, tablerows=1)
        result = tflagger(vis=self.vis,mode='summary',scan='3')
        self.assertEqual(result['flagged'], 0, 'Expected 0 flags, found %s'%result['flagged'])
        self.assertEqual(result['total'], 762048,'Expected total 762048, found %s'%result['total'])

    def test_uquack(self):
        '''tflagcmd: unapply quack agent'''
        # Remove any cmd from table
        tflagcmd(vis=self.vis, action='clear', clearall=True)

        # Flag using manualflag agent
        input = "scan=1"
        filename = create_input(input)
        tflagcmd(vis=self.vis, inputmode='file', inputfile=filename, action='apply')

        # Flag using the quack agent
        input = "scan=1~3 mode=quack quackinterval=1.0"
        filename = create_input(input)
        tflagcmd(vis=self.vis, inputmode='file', inputfile=filename, action='apply')
        
        # Unapply only the quack line
        tflagcmd(vis=self.vis, action='unapply', useapplied=True, tablerows=1)
        result = tflagger(vis=self.vis,mode='summary',scan='1')
        
        # Only the manualflag flags should be there
        self.assertEqual(result['flagged'], 568134, 'Expected 568134 flags, found %s'%result['flagged'])
        self.assertEqual(result['total'], 568134,'Expected total 568134, found %s'%result['total'])
        
    def test_umanualflag(self):
        '''tflagcmd: unapply manualflag agent'''
        # Remove any cmd from table
        tflagcmd(vis=self.vis, action='clear', clearall=True)

        # Flag using manualflag agent
        input = "scan=1"
        filename = create_input(input)
        tflagcmd(vis=self.vis, inputmode='file', inputfile=filename, action='apply')

        # Flag using the quack agent
        input = "scan=1~3 mode=quack quackinterval=1.0"
        filename = create_input(input)
        tflagcmd(vis=self.vis, inputmode='file', inputfile=filename, action='apply')
        
        # Unapply only the manualflag line
        tflagcmd(vis=self.vis, action='unapply', useapplied=True, tablerows=0)
        result = tflagger(vis=self.vis,mode='summary',scan='1')
        
        # Only the quack flags should be left
        self.assertEqual(result['flagged'], 44226, 'Expected 44226 flags, found %s'%result['flagged'])
        self.assertEqual(result['total'], 568134,'Expected total 568134, found %s'%result['total'])
        
        
        
# Dummy class which cleans up created files
class cleanup(test_base):
    
    def tearDown(self):
        os.system('rm -rf multiobs.ms*')
        os.system('rm -rf flagdatatest-alma.ms*')
        os.system('rm -rf ngc5921.ms*')


    def test1(self):
        '''tflagcmd: Cleanup'''
        pass


def suite():
    return [test_manualflag,
            test_selections_alma,
            test_unapply,
            cleanup]
        
        
        
        
        
