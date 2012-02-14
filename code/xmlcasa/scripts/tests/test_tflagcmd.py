import shutil
import unittest
import os
import filecmp
from tasks import *
from taskinit import *

#
# Test of tflagcmd task. It uses tflagdata to unflag and summary
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
        tflagdata(vis=self.vis, mode='unflag', savepars=False)

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
        tflagdata(vis=self.vis, mode='unflag', savepars=False)

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
        tflagdata(vis=self.vis, mode='unflag', savepars=False)

        
class test_manual(test_base):
    '''Test manual selections'''
    
    def setUp(self):
        self.setUp_multi()
        
    def test_observation(self):
        input = "observation='1'"
        filename = create_input(input)
        
        tflagcmd(vis=self.vis, inpmode='file', inpfile=filename, action='apply', savepars=False)
        test_eq(tflagdata(vis=self.vis, mode='summary'), 2882778, 28500)

class test_alma(test_base):
    # Test various selections for alma data 

    def setUp(self):
        self.setUp_flagdatatest_alma()
    
    def test_intent(self):
        '''tflagcmd: test scan intent selection'''
        
        input = "intent='CAL*POINT*'"
        filename = create_input(input)
        
        # flag POINTING CALIBRATION scans 
        tflagcmd(vis=self.vis, inpmode='file', inpfile=filename, action='apply', savepars=False)
        test_eq(tflagdata(vis=self.vis,mode='summary', antenna='2'), 377280, 26200)
        
    def test_extract(self):
        '''tflagcmd: action = extract and apply clip on WVR'''
        # Remove any cmd from table
        tflagcmd(vis=self.vis, action='clear', clearall=True)
        
        # Save cmd to FLAG_CMD
        cmd = "mode=clip clipminmax=[0,50] expression=ABS_WVR"
        tflagcmd(vis=self.vis, inpmode='cmd', command=[cmd], action='list', savepars=True)
        
        # Extract it
        res = tflagcmd(vis=self.vis, action='extract')
        
        # Apply to clip only WVR
        tflagcmd(vis=self.vis, inpmode='cmd', command=[res[0]['command']], savepars=False, action='apply')
        ret = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(ret['flagged'], 22752)
        self.assertEqual(ret['correlation']['I']['flagged'], 22752)
        self.assertEqual(ret['correlation']['XX']['flagged'], 0)
        self.assertEqual(ret['correlation']['YY']['flagged'], 0)
                
                
class test_unapply(test_base):
    # Action unapply
    def setUp(self):
        self.setUp_ngc5921()

    def test_utfcrop(self):
        '''tflagcmd: unapply tfcrop agent'''
        # Remove any cmd from table
        tflagcmd(vis=self.vis, action='clear', clearall=True)

        # Flag using manual agent
        input = "scan=1"
        filename = create_input(input)
        tflagcmd(vis=self.vis, inpmode='file', inpfile=filename, action='apply', savepars=True)
        
        # Flag using tfcrop agent
        input = "scan=3 mode=tfcrop expression='ABS_RR'"
        filename = create_input(input)
        tflagcmd(vis=self.vis, inpmode='file', inpfile=filename, action='apply', savepars=True)
        test_eq(tflagdata(vis=self.vis,mode='summary',scan='1'), 568134, 568134)
        test_eq(tflagdata(vis=self.vis,mode='summary',scan='3'), 762048, 2829)
        
        # Unapply only the tfcrop line
        tflagcmd(vis=self.vis, action='unapply', useapplied=True, tablerows=1, savepars=False)
        result = tflagdata(vis=self.vis,mode='summary',scan='3')
        self.assertEqual(result['flagged'], 0, 'Expected 0 flags, found %s'%result['flagged'])
        self.assertEqual(result['total'], 762048,'Expected total 762048, found %s'%result['total'])

    def test_uquack(self):
        '''tflagcmd: unapply quack agent'''
        # Remove any cmd from table
        tflagcmd(vis=self.vis, action='clear', clearall=True)

        # Flag using manual agent
        input = "scan=1"
        filename = create_input(input)
        tflagcmd(vis=self.vis, inpmode='file', inpfile=filename, action='apply', savepars=True)

        # Flag using the quack agent
        input = "scan=1~3 mode=quack quackinterval=1.0"
        filename = create_input(input)
        tflagcmd(vis=self.vis, inpmode='file', inpfile=filename, action='apply', savepars=True)
        
        # Unapply only the quack line
        tflagcmd(vis=self.vis, action='unapply', useapplied=True, tablerows=1, savepars=True)
        result = tflagdata(vis=self.vis,mode='summary',scan='1')
        
        # Only the manual flags should be there
        self.assertEqual(result['flagged'], 568134, 'Expected 568134 flags, found %s'%result['flagged'])
        self.assertEqual(result['total'], 568134,'Expected total 568134, found %s'%result['total'])
        
    def test_umanualflag(self):
        '''tflagcmd: unapply manual agent'''
        # Remove any cmd from table
        tflagcmd(vis=self.vis, action='clear', clearall=True)

        # Flag using manual agent
        input = "scan=1"
        filename = create_input(input)
        tflagcmd(vis=self.vis, inpmode='file', inpfile=filename, action='apply', savepars=True)

        # Flag using the quack agent
        input = "scan=1~3 mode=quack quackinterval=1.0"
        filename = create_input(input)
        tflagcmd(vis=self.vis, inpmode='file', inpfile=filename, action='apply', savepars=True)
        
        # Unapply only the manual line
        tflagcmd(vis=self.vis, action='unapply', useapplied=True, tablerows=0, savepars=False)
        result = tflagdata(vis=self.vis,mode='summary',scan='1')
        
        # Only the quack flags should be left
        self.assertEqual(result['flagged'], 44226, 'Expected 44226 flags, found %s'%result['flagged'])
        self.assertEqual(result['total'], 568134,'Expected total 568134, found %s'%result['total'])
        

    def test_uscans(self):
        '''tflagcmd: Unapply only APPLIED=True'''
        # Remove any cmd from table
        tflagcmd(vis=self.vis, action='clear', clearall=True)
        
        # Flag several scans and save them to FLAG_CMD with APPLIED=True
        tflagdata(vis=self.vis, scan='7', savepars=True)
        tflagdata(vis=self.vis, scan='1', savepars=True)
        tflagdata(vis=self.vis, scan='2', savepars=True)
        tflagdata(vis=self.vis, scan='3', savepars=True)
        tflagdata(vis=self.vis, scan='4', savepars=True)
        
        # There should be 5 cmds in FLAG_CMD. Unapply row=1 and set APPLIED to False
        tflagcmd(vis=self.vis, action='unapply', tablerows=1, savepars=False)
        
        # Unapply scans 2 and 3 only. It should not re-apply scan=1 (row 1)
        tflagcmd(vis=self.vis, action='unapply', tablerows=[2,3], savepars=False)
        
        # We should have left only scans 4 and 7 flagged.
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['1']['flagged'], 0, "It should not re-apply tablerows=1")
        self.assertEqual(res['scan']['4']['flagged'], 95256, "It should not unapply tablerows=4")
        self.assertEqual(res['scan']['7']['flagged'], 190512, "It should not unapply tablerows=7")
        self.assertEqual(res['flagged'], 285768)

class test_savepars(test_base):
    # Action unapply
    def setUp(self):
        self.setUp_ngc5921()

    def test_list1(self):
        '''tflagcmd: list and savepars=True/False'''
        # Remove any cmd from table
        tflagcmd(vis=self.vis, action='clear', clearall=True)
        
        ########## TEST 1 
        # create text file called tflagcmd.txt
        input = " scan=4 mode=clip expression=ABS_RR clipminmax=[0,4]\n"
        filename = create_input(input)
        filename1 = 'filename1.txt'
        os.system('cp '+filename+' '+filename1)

        # save command to MS
        tflagcmd(vis=self.vis, action='list', inpmode='cmd', command=[input], savepars=True)
        
        # list/save to a file
        os.system('rm -rf myflags.txt')
        tflagcmd(vis=self.vis, action='list', outfile='myflags.txt', savepars=True)
        
        # compare saved file with original input file
        self.assertTrue(filecmp.cmp(filename, 'myflags.txt', 1), 'Files should be equal')
        
        ########## TEST 2 
        # create another input
        input = " scan=1~3 mode=manual\n"
        filename = create_input(input)
        
        # apply and don't save to MS
        tflagcmd(vis=self.vis, inpmode='file', inpfile=filename, action='apply', savepars=False)
        
        # list and check that parameters were not saved to MS
        os.system('rm -rf myflags.txt')
        tflagcmd(vis=self.vis, action='list', outfile='myflags.txt', savepars=True)
        self.assertFalse(filecmp.cmp(filename, 'myflags.txt', 1), 'Files should not be equal')
        
        ########### TEST 3 
        # apply cmd from TEST 1 and update APPLIED column
        tflagcmd(vis=self.vis, action='apply', savepars=False)
        
        # scans=1~3 should be fully flagged
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['1']['flagged'], 568134)
        self.assertEqual(res['scan']['1']['total'], 568134)
        self.assertEqual(res['scan']['2']['flagged'], 238140)
        self.assertEqual(res['scan']['2']['total'], 238140)
        self.assertEqual(res['scan']['3']['flagged'], 762048)
        self.assertEqual(res['scan']['3']['total'], 762048)
        # scan 4 should be partially flagged
        self.assertEqual(res['scan']['4']['flagged'], 3348)
        self.assertEqual(res['scan']['4']['total'], 95256)
        
        # Only cmd form TEST 1 should be in MS
        os.system('rm -rf myflags.txt')
        tflagcmd(vis=self.vis, action='list', outfile='myflags.txt', useapplied=True, savepars=True)
        self.assertTrue(filecmp.cmp(filename1, 'myflags.txt', 1), 'Files should be equal')
        
    def test_writeflags(self):
        '''tflagcmd: writeflags = False'''
        # Remove any cmd from table
        tflagcmd(vis=self.vis, action='clear', clearall=True)
        
        # Save the parameters to FLAG_CMD but do not write the flags
        input = " scan=4 mode=clip expression=ABS_ALL clipminmax=[0,4]\n"
        tflagcmd(vis=self.vis, inpmode='cmd', command=[input], writeflags=False, savepars=True)
        
        # No flags should be in the MS
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 0, 'Should not write flags when writeflags=False')
        
        
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
    return [test_manual,
            test_alma,
            test_unapply,
            test_savepars,
            cleanup]
        
        
        
        
        
