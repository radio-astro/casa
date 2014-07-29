import shutil
import unittest
import os
import filecmp
import exceptions
from tasks import *
from taskinit import *
from __main__ import default
from OrderedDictionary import OrderedDict

#
# Test of flagcmd task. It uses flagdata to unflag and summary
#

def test_eq(result, total, flagged):

    print "%s of %s data was flagged, expected %s of %s" % \
    (result['flagged'], result['total'], flagged, total)
    assert result['total'] == total, \
               "%s data in total; %s expected" % (result['total'], total)
    assert result['flagged'] == flagged, \
           "%s flags set; %s expected" % (result['flagged'], flagged)

def create_input(str_text,fname=''):
    '''Save the string in a text file'''
    
    if fname=='':
        inp = 'flagcmd.txt'
    else:
        inp=fname

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

def create_input1(str_text, filename):
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

# Local copy of the agentflagger tool
aflocal = casac.agentflagger()

# Base class which defines setUp functions
# for importing different data sets
class test_base(unittest.TestCase):
    
    def setUp_ngc5921(self):
        self.vis = "ngc5921.ms"

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            os.system('cp -r '+datapath + self.vis +' '+ self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        self.unflag_ms()        
        default(flagcmd)

    def setUp_multi(self):
        self.vis = "multiobs.ms"

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r '+datapath + self.vis +' '+ self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        self.unflag_ms()        
        default(flagcmd)

    def setUp_alma_ms(self):
        '''ALMA MS, scan=1,8,10 spw=0~3 4,128,128,1 chans, I,XX,YY'''
        self.vis = "uid___A002_X30a93d_X43e_small.ms"

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r '+datapath + self.vis +' '+ self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        self.unflag_ms()        
        default(flagcmd)
        
    def setUp_evla(self):
        self.vis = "tosr0001_scan3_noonline.ms"

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r '+datapath + self.vis +' '+ self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        self.unflag_ms()        
        default(flagcmd)
        
    def setUp_shadowdata(self):
        self.vis = "shadowtest_part.ms"

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r '+datapath + self.vis +' '+ self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        self.unflag_ms()        
        default(flagcmd)
        
    def setUp_data4rflag(self):
        self.vis = "Four_ants_3C286.ms"

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r '+datapath + self.vis +' '+ self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        self.unflag_ms()        
        default(flagcmd)

    def setUp_bpass_case(self):
        self.vis = "cal.fewscans.bpass"

        if os.path.exists(self.vis):
            print "The CalTable is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r ' + \
                        os.environ.get('CASAPATH').split()[0] +
                        "/data/regression/unittest/flagdata/" + self.vis + ' ' + self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        self.unflag_ms()        
        default(flagcmd)

    def unflag_ms(self):
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.init()
        aflocal.run(writeflags=True)
        aflocal.done()
        
class test_manual(test_base):
    '''Test manual selections'''
    
    def setUp(self):
        self.setUp_multi()
        
    def test_observation(self):
        myinput = "observation='1'"
        filename = create_input(myinput)
        
        flagcmd(vis=self.vis, inpmode='list', inpfile=filename, action='apply', savepars=False,
                flagbackup=False)
        test_eq(flagdata(vis=self.vis, mode='summary'), 2882778, 28500)

    def test_compatibility(self):
        myinput = "observation='1' mode='manualflag'"
        filename = create_input(myinput)
        
        flagcmd(vis=self.vis, inpmode='list', inpfile=filename, action='apply', savepars=False,
                flagbackup=False)
        test_eq(flagdata(vis=self.vis, mode='summary'), 2882778, 28500)
        
    def test_autocorr(self):
        '''flagcmd: autocorr=True'''
        self.setUp_ngc5921()
        flagcmd(vis=self.vis, inpmode='list', inpfile=['autocorr=True'], action='apply')
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 203994, 'Should flag only the auto-correlations')
        

class test_alma(test_base):
    # Test various selections for alma data 

    def setUp(self):
#        self.setUp_flagdatatest_alma()
        self.setUp_alma_ms()
    
    def test_intent(self):
        '''flagcmd: test scan intent selection'''
        
        myinput = "intent='CAL*POINT*'\n"\
                "#scan=3,4"
        filename = create_input(myinput)
        
        # flag POINTING CALIBRATION scans and ignore comment line
        flagcmd(vis=self.vis, inpmode='list', inpfile=filename, action='apply', savepars=False,
                flagbackup=False)
#         test_eq(flagdata(vis=self.vis,mode='summary', antenna='2'), 377280, 26200)
        res = flagdata(vis=self.vis,mode='summary')
#         self.assertEqual(res['scan']['1']['flagged'], 80184, 'Only scan 1 should be flagged')
#         self.assertEqual(res['scan']['4']['flagged'], 0, 'Scan 4 should not be flagged')
        self.assertEqual(res['scan']['1']['flagged'], 192416.0)
        
    def test_cmd(self):
        '''flagcmd: inpmode=list with empty parameter'''
        
        # Test the correct parsing with empty parameter such as antenna=''
        flagcmd(vis=self.vis, inpmode='list', 
                 inpfile=["intent='CAL*POINT*' field=''","scan='1,8' antenna=''","scan='10'"], 
                 action='apply', savepars=False, flagbackup=False)
        res = flagdata(vis=self.vis,mode='summary')
        self.assertEqual(res['scan']['1']['flagged'], 192416)
        self.assertEqual(res['scan']['8']['flagged'], 39096)
        self.assertEqual(res['scan']['10']['flagged'], 39096)
        self.assertEqual(res['flagged'], 270608)
                     
    def test_extract(self):
        '''flagcmd: action = extract and apply clip on WVR'''
        # Remove any cmd from table
        flagcmd(vis=self.vis, action='clear', clearall=True)
        
        # Save cmd to FLAG_CMD
        cmd = "mode='clip' clipminmax=[0,50] correlation='ABS_WVR'"
        flagcmd(vis=self.vis, inpmode='list', inpfile=[cmd], action='list', savepars=True)
        
        # Extract it
        res = flagcmd(vis=self.vis, action='extract', useapplied=True)
        
        # Apply cmd to clip only WVR
        flagcmd(vis=self.vis, inpmode='list', inpfile=[res[0]['command']], savepars=False, 
                 action='apply',flagbackup=False)
        ret = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(ret['flagged'], 498)
        self.assertEqual(ret['correlation']['I']['flagged'], 498)
        self.assertEqual(ret['correlation']['XX']['flagged'], 0)
        self.assertEqual(ret['correlation']['YY']['flagged'], 0)
                
                
class test_unapply(test_base):
    # Action unapply
    def setUp(self):
        self.setUp_ngc5921()
        
    def test_unsupported_unapply(self):
        '''flagcmd: raise exception from inpmode=list and unapply'''
#        try:
        self.assertFalse(flagcmd(vis=self.vis, action='unapply', inpmode='list',
                inpfile=["spw='0' reason='MANUAL'"]))
#
    def test_utfcrop(self):
        '''flagcmd: unapply tfcrop agent'''
        # Remove any cmd from table
        flagcmd(vis=self.vis, action='clear', clearall=True)

        # Flag using manual agent
        myinput = "scan=1"
        filename = create_input(myinput)
        flagcmd(vis=self.vis, inpmode='list', inpfile=filename, action='apply', savepars=True)
        
        # Flag using tfcrop agent from file
        # Note : For this test, scan=4 gives identical flags on 32/64 bit machines,
        #           and one flag difference on a Mac (32)
        #           Other scans give differences at the 0.005% level.
        myinput = "scan=4 mode=tfcrop correlation='ABS_RR' extendflags=False"
        filename = create_input(myinput)
        flagcmd(vis=self.vis, inpmode='list', inpfile=filename, action='apply', savepars=True,
                flagbackup=False)
        res = flagdata(vis=self.vis,mode='summary')
        self.assertEqual(res['scan']['1']['flagged'], 568134, 'Whole scan=1 should be flagged')
        #self.assertEqual(res['scan']['4']['flagged'], 1201, 'scan=4 should be partially flagged')
        self.assertTrue(res['scan']['4']['flagged']>= 1200 and res['scan']['4']['flagged']<= 1204, \
                        'scan=4 should be partially flagged')
        
        # Unapply only the tfcrop line
        flagcmd(vis=self.vis, action='unapply', useapplied=True, tablerows=1, savepars=False)
        result = flagdata(vis=self.vis,mode='summary',scan='4')
        self.assertEqual(result['flagged'], 0, 'Expected 0 flags, found %s'%result['flagged'])
        self.assertEqual(result['total'], 95256,'Expected total 95256, found %s'%result['total'])
        
    def test_unapply_tfcrop_and_unset_flagrow(self):
        '''flagcmd: Check that FLAG_ROW is unset after un-applying an tfcrop agent'''
        # Remove any cmd from table
        flagcmd(vis=self.vis, action='clear', clearall=True)

        # Flag using manual agent
        myinput = "scan=4"
        filename = create_input(myinput)
        flagcmd(vis=self.vis, inpmode='list', inpfile=filename, action='apply', savepars=False)
        
        # Check FLAG_ROW is all set to true
        mytb = tbtool()
        mytb.open(self.vis)
        selectedtb = mytb.query('SCAN_NUMBER in [4]')
        FLAG_ROW = selectedtb.getcol('FLAG_ROW')
        self.assertEqual(FLAG_ROW.sum(), FLAG_ROW.size)
        mytb.close()        
        
        # Flag using tfcrop agent from file
        myinput = "scan=4 mode=tfcrop correlation='ABS_RR' extendflags=False"
        filename = create_input(myinput)
        flagcmd(vis=self.vis, inpmode='list', inpfile=filename, action='apply', savepars=True,
                flagbackup=False)
        
        # Check FLAG_ROW is all set to true
        mytb = tbtool()
        mytb.open(self.vis)
        selectedtb = mytb.query('SCAN_NUMBER in [4]')
        FLAG_ROW = selectedtb.getcol('FLAG_ROW')
        self.assertEqual(FLAG_ROW.sum(), FLAG_ROW.size)
        mytb.close()           
        
        # Unapply only the tfcrop line
        flagcmd(vis=self.vis, action='unapply', useapplied=True, tablerows=0, savepars=False)
       
        # Check FLAG_ROW is now all set to false
        mytb = tbtool()
        mytb.open(self.vis)
        selectedtb = mytb.query('SCAN_NUMBER in [4]')
        FLAG_ROW = selectedtb.getcol('FLAG_ROW')
        self.assertEqual(FLAG_ROW.sum(), 0)
        mytb.close()      
        
    def test_unapply_rflag_and_unset_flagrow(self):
        '''flagcmd: Check that FLAG_ROW is unset after un-applying an rflag agent'''
        # Remove any cmd from table
        flagcmd(vis=self.vis, action='clear', clearall=True)

        # Flag using manual agent
        myinput = "scan=4"
        filename = create_input(myinput)
        flagcmd(vis=self.vis, inpmode='list', inpfile=filename, action='apply', savepars=False)
        
        # Check FLAG_ROW is all set to true
        mytb = tbtool()
        mytb.open(self.vis)
        selectedtb = mytb.query('SCAN_NUMBER in [4]')
        FLAG_ROW = selectedtb.getcol('FLAG_ROW')
        self.assertEqual(FLAG_ROW.sum(), FLAG_ROW.size)
        mytb.close()        
        
        # Flag using tfcrop agent from file
        myinput = "scan=4 mode=rflag "
        filename = create_input(myinput)
        flagcmd(vis=self.vis, inpmode='list', inpfile=filename, action='apply', savepars=True,
                flagbackup=False)
        
        # Check FLAG_ROW is all set to true
        mytb = tbtool()
        mytb.open(self.vis)
        selectedtb = mytb.query('SCAN_NUMBER in [4]')
        FLAG_ROW = selectedtb.getcol('FLAG_ROW')
        self.assertEqual(FLAG_ROW.sum(), FLAG_ROW.size)
        mytb.close()           
        
        # Unapply only the tfcrop line
        flagcmd(vis=self.vis, action='unapply', useapplied=True, tablerows=0, savepars=False)
       
        # Check FLAG_ROW is now all set to false
        mytb = tbtool()
        mytb.open(self.vis)
        selectedtb = mytb.query('SCAN_NUMBER in [4]')
        FLAG_ROW = selectedtb.getcol('FLAG_ROW')
        self.assertEqual(FLAG_ROW.sum(), 0)
        mytb.close()         
        
    def test_unapply_clip_and_unset_flagrow(self):
        '''flagcmd: Check that FLAG_ROW is unset after un-applying a clip agent'''
        # Remove any cmd from table
        flagcmd(vis=self.vis, action='clear', clearall=True)

        # Flag using manual agent
        myinput = "scan=4"
        filename = create_input(myinput)
        flagcmd(vis=self.vis, inpmode='list', inpfile=filename, action='apply', savepars=False)
        
        # Check FLAG_ROW is all set to true
        mytb = tbtool()
        mytb.open(self.vis)
        selectedtb = mytb.query('SCAN_NUMBER in [4]')
        FLAG_ROW = selectedtb.getcol('FLAG_ROW')
        mytb.close()        
        self.assertEqual(FLAG_ROW.sum(), FLAG_ROW.size)
        
        # Flag using tfcrop agent from file
        myinput = "scan=4 mode=clip "
        filename = create_input(myinput)
        flagcmd(vis=self.vis, inpmode='list', inpfile=filename, action='apply', savepars=True,
                flagbackup=False)
        
        # Check FLAG_ROW is all set to true
        mytb = tbtool()
        mytb.open(self.vis)
        selectedtb = mytb.query('SCAN_NUMBER in [4]')
        FLAG_ROW = selectedtb.getcol('FLAG_ROW')
        mytb.close()           
        self.assertEqual(FLAG_ROW.sum(), FLAG_ROW.size)
        
        # Unapply only the tfcrop line
        flagcmd(vis=self.vis, action='unapply', useapplied=True, tablerows=0, savepars=False)
       
        # Check FLAG_ROW is now all set to false
        mytb = tbtool()
        mytb.open(self.vis)
        selectedtb = mytb.query('SCAN_NUMBER in [4]')
        FLAG_ROW = selectedtb.getcol('FLAG_ROW')
        mytb.close()        
        self.assertEqual(FLAG_ROW.sum(), 0)

    def test_uquack(self):
        '''flagcmd: unapply quack agent'''
        # Remove any cmd from table
        flagcmd(vis=self.vis, action='clear', clearall=True)

        # Flag using the quack agent
        myinput = "scan=1~3 mode=quack quackinterval=1.0"
        filename = create_input(myinput)
        flagcmd(vis=self.vis, inpmode='list', inpfile=filename, action='apply', savepars=True,
                flagbackup=False)
        result = flagdata(vis=self.vis,mode='summary')
        quack_flags = result['scan']['1']['flagged']

        # Flag using manual agent
        myinput = "scan=1"
        filename = create_input(myinput)
        flagcmd(vis=self.vis, inpmode='list', inpfile=filename, action='apply', savepars=True,
                flagbackup=False)
        result = flagdata(vis=self.vis,mode='summary')
        scan1_flags = result['scan']['1']['flagged']

        # Unapply only the quack line
        flagcmd(vis=self.vis, action='unapply', useapplied=True, tablerows=0, savepars=True)
        result = flagdata(vis=self.vis,mode='summary')
        manual_flags = result['scan']['1']['flagged']
        
        # Only the manual flags should be there
#         self.assertEqual(result['flagged'], 568134, 'Expected 568134 flags, found %s'%result['flagged'])
#         self.assertEqual(result['total'], 568134,'Expected total 568134, found %s'%result['total'])
        # CAS-5377. New unapply action
        self.assertEqual(result['scan']['3']['flagged'], 0)
        self.assertEqual(manual_flags,scan1_flags - quack_flags)

    def test_umanualflag(self):
        '''flagcmd: unapply manual agent'''
        # Remove any cmd from table
        flagcmd(vis=self.vis, action='clear', clearall=True)

        # Flag using manual agent
        myinput = "scan=1"
        filename = create_input(myinput)
        flagcmd(vis=self.vis, inpmode='list', inpfile=filename, action='apply', savepars=True,
                flagbackup=False)

        # Flag using the quack agent
        myinput = "scan=1~3 mode=quack quackinterval=1.0"
        filename = create_input(myinput)
        flagcmd(vis=self.vis, inpmode='list', inpfile=filename, action='apply', savepars=True,
                flagbackup=False)
        
        # Unapply the manual line
        flagcmd(vis=self.vis, action='unapply', useapplied=True, tablerows=0, savepars=False)
        result = flagdata(vis=self.vis,mode='summary',scan='1,2,3')
        
        # scan 1 should be fully unflagged
        self.assertEqual(result['scan']['1']['flagged'], 0)
        self.assertEqual(result['scan']['2']['flagged'], 47628)
        self.assertEqual(result['scan']['3']['flagged'], 47628)
        self.assertEqual(result['flagged'], 47628+47628)
        

    def test_uscans(self):
        '''flagcmd: Unapply only APPLIED=True'''
        # Remove any cmd from table
        flagcmd(vis=self.vis, action='clear', clearall=True)
        
        # Flag several scans and save them to FLAG_CMD with APPLIED=True
        flagdata(vis=self.vis, scan='7', savepars=True, flagbackup=False)
        flagdata(vis=self.vis, scan='1', savepars=True, flagbackup=False)
        flagdata(vis=self.vis, scan='2', savepars=True, flagbackup=False)
        flagdata(vis=self.vis, scan='3', savepars=True, flagbackup=False)
        flagdata(vis=self.vis, scan='4', savepars=True, flagbackup=False)
        
        # There should be 5 cmds in FLAG_CMD. Unapply row=1 and set APPLIED to False
        flagcmd(vis=self.vis, action='unapply', tablerows=1, useapplied=True)
        
        # Unapply scans 2 and 3 only. It should not re-apply scan=1 (row 1)
        flagcmd(vis=self.vis, action='unapply', tablerows=[2,3], useapplied=True)
        
        # We should have left only scans 4 and 7 flagged.
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['1']['flagged'], 0, "It should not re-apply tablerows=1")
        self.assertEqual(res['scan']['4']['flagged'], 95256, "It should not unapply tablerows=4")
        self.assertEqual(res['scan']['7']['flagged'], 190512, "It should not unapply tablerows=7")
        self.assertEqual(res['flagged'], 285768)

class test_savepars(test_base):
    # Action unapply
    def setUp(self):
        self.setUp_ngc5921()

    def test_list1(self):
        '''flagcmd: list and savepars=True/False'''
        # Remove any cmd from table
        flagcmd(vis=self.vis, action='clear', clearall=True)
        
        ########## TEST 1 
        # create text file called flagcmd.txt
        myinput = "scan='4' mode='clip' correlation='ABS_RR' clipminmax=[0, 4]\n"
        filename = create_input(myinput)
        filename1 = 'filename1.txt'
        os.system('cp '+filename+' '+filename1)

        # save command to MS
        flagcmd(vis=self.vis, action='list', inpmode='list', inpfile=[myinput], savepars=True)
        
        # list/save to a file
        os.system('rm -rf myflags.txt')
        flagcmd(vis=self.vis, action='list', outfile='myflags.txt', savepars=True)
        
        # compare saved file with original input file
        self.assertTrue(filecmp.cmp(filename1, 'myflags.txt', 1), 'Files should be equal')
        
        ########## TEST 2 
        # create another input
        myinput = "scan='1~3' mode='manual'\n"
        filename = create_input(myinput)
        
        # apply and don't save to MS
        flagcmd(vis=self.vis, inpmode='list', inpfile=filename, action='apply', savepars=False,
                flagbackup=False)
        
        # list and check that parameters were not saved to MS
        os.system('rm -rf myflags.txt')
        flagcmd(vis=self.vis, action='list', outfile='myflags.txt', savepars=True)
        self.assertFalse(filecmp.cmp(filename, 'myflags.txt', 1), 'Files should not be equal')
        
        ########### TEST 3 
        # apply cmd from TEST 1 and update APPLIED column
        flagcmd(vis=self.vis, action='apply', savepars=False, flagbackup=False)
        
        # scans=1~3 should be fully flagged
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['1']['flagged'], 568134)
        self.assertEqual(res['scan']['1']['total'], 568134)
        self.assertEqual(res['scan']['2']['flagged'], 238140)
        self.assertEqual(res['scan']['2']['total'], 238140)
        self.assertEqual(res['scan']['3']['flagged'], 762048)
        self.assertEqual(res['scan']['3']['total'], 762048)
        # scan 4 should be partially flagged by clip mode
        self.assertEqual(res['scan']['4']['flagged'], 3348, 'Only RR should be flagged')
        self.assertEqual(res['scan']['4']['total'], 95256)
        
        # Only cmd from TEST 1 should be in MS
        os.system('rm -rf myflags.txt')
        flagcmd(vis=self.vis, action='list', outfile='myflags.txt', useapplied=True, savepars=True)
        self.assertTrue(filecmp.cmp(filename1, 'myflags.txt', 1), 'Files should be equal')        


class test_XML(test_base):
    
    def setUp(self):
        self.setUp_evla()
        
    def test_xml1(self):
        '''flagcmd: list xml file and save in outfile'''
        
        # The MS only contains clip and shadow commands
        # The XML contain the online flags
        flagcmd(vis=self.vis, action='list', inpmode='xml', savepars=True, outfile='origxml.txt')
        
        # Now save the online flags to the FLAG_CMD without applying
        flagcmd(vis=self.vis, action='list', inpmode='xml', savepars=True)
        
        # Now apply them by selecting the reasons and save in another file
        # 507 cmds crash on my computer.
        reasons = ['ANTENNA_NOT_ON_SOURCE','FOCUS_ERROR','SUBREFLECTOR_ERROR']
        flagcmd(vis=self.vis, action='apply', reason=reasons, savepars=True, outfile='myxml.txt',
                flagbackup=False)
                
        # Compare with original XML
        self.assertTrue(filecmp.cmp('origxml.txt', 'myxml.txt',1), 'Files should be equal')
        
        # Check that APPLIED column has been updated to TRUE
        
        
    def test_xml2(self):
        '''flagcmd: list xml file and save in outfile'''
        
        # The MS only contains clip and shadow commands
        
        # Apply the shadow command
        flagcmd(vis=self.vis, action='apply', reason='SHADOW', flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 240640)
        
    def test_missing_corr(self):
        '''flagcmd: CAS-4234, non-existing correlation raise no error'''
        flagcmd(vis=self.vis, action='clear', clearall=True)
        
        flagcmd(vis=self.vis, inpmode='list', inpfile=["correlation='XX,RR,RL'"], action='list',
                savepars=True)
        
        flagcmd(vis=self.vis, action='apply', flagbackup=False)
        
        res = flagdata(vis=self.vis, mode='summary')
        
        self.assertEqual(res['flagged'], 208000+208000, 'Should only flag RR and RL and not fail')

class test_shadow(test_base):
    def setUp(self):
        self.setUp_shadowdata()

    def test_CAS2399(self):
        '''flagcmd: shadow by antennas not present in MS'''
        
        # Create antennafile in disk
        myinput = 'name=VLA01\n'+\
                'diameter=25.0\n'+\
                'position=[-1601144.96146691, -5041998.01971858, 3554864.76811967]\n'+\
                'name=VLA02\n'+\
                'diameter=25.0\n'+\
                'position=[-1601105.7664601889, -5042022.3917835914, 3554847.245159178]\n'+\
                'name=VLA09\n'+\
                'diameter=25.0\n'+\
                'position=[-1601197.2182404203, -5041974.3604805721, 3554875.1995636248]\n'+\
                'name=VLA10\n'+\
                'diameter=25.0\n'+\
                'position=[-1601227.3367843349,-5041975.7011900628,3554859.1642644769]\n'            

#        antfile = 'myants.txt'
#        if os.path.exists(antfile):
#            os.system('rm -rf myants.txt')

        filename = create_input(myinput)

        # Create command line
        myinput = ["mode='shadow' tolerance=0.0 addantenna='flagcmd.txt'"]
#        filename = 'cmdfile.txt'
#        if os.path.exists(filename):
#            os.system('rm -rf cmdfile.txt')
        
#        create_input(myinput, filename)
        
        # Flag
        flagcmd(vis=self.vis, action='clear', clearall=True)
#        flagcmd(vis=self.vis, action='apply', inpmode='list', inpfile=filename)
        flagcmd(vis=self.vis, action='apply', inpmode='list', inpfile=myinput, flagbackup=False)
        
        # Check flags
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['antenna']['VLA3']['flagged'], 3752)
        self.assertEqual(res['antenna']['VLA4']['flagged'], 1320)
        self.assertEqual(res['antenna']['VLA5']['flagged'], 1104)        


# Test rflag inputs with filenames, as well as inline thresholds.
# rflag does not generate output thresholds for action='apply'.
class test_rflag(test_base):

    def setUp(self):
        self.setUp_data4rflag()

    def test_rflaginputs(self):
        """flagcmd:: Test of rflag threshold-inputs of both types (file and inline)
        """

        # (1) and (2) are the same as test_flagdata[test_rflag3].
        # -- (3),(4),(5) should produce the same answers as (1) and (2)
        #
        # (1) Test input/output files, through the task, mode='rflag'
        #flagdata(vis=self.vis, mode='rflag', spw='9,10', timedev='tdevfile.txt', \
        #              freqdev='fdevfile.txt', action='calculate');
        #flagdata(vis=self.vis, mode='rflag', spw='9,10', timedev='tdevfile.txt', \
        #              freqdev='fdevfile.txt', action='apply');
        #res1 = flagdata(vis=self.vis, mode='summary')
        #print "(1) Finished flagdata : test 1 : ", res1['flagged']


        # (2) Test rflag output written to cmd file via mode='rflag' and 'savepars' 
        #      and then read back in via list mode. 
        #      Also test the 'savepars' when timedev and freqdev are specified differently...
        #flagdata(vis=self.vis,mode='unflag');
        #flagdata(vis=self.vis, mode='rflag', spw='9,10', timedev='', \
        #              freqdev=[],action='calculate',savepars=True,outfile='outcmd.txt');
        #os.system('cat outcmd.txt')
        #flagdata(vis=self.vis, mode='list', inpfile='outcmd.txt');
        #res2 = flagdata(vis=self.vis, mode='summary')
        #print "(2) Finished flagdata : test 2 : ", res2['flagged']


        # (3) flagcmd : Send in the same text files produces/used in (1)
#        input1 = "{'name':'Rflag','timedev':[[1.0,9.0,0.038859],[1.0,10.0,0.162833]]}\n"
#        input2 = "{'name':'Rflag','freqdev':[[1.0,9.0,0.079151],[1.0,10.0,0.205693]]}\n"
        input1 = "{'name':'Rflag','timedev':[[1.0,9.0,0.038859101518899999],[1.0,10.0,0.16283325492600001]]}\n"
        input2 = "{'name':'Rflag','freqdev':[[1.0,9.0,0.079151260994399994],[1.0,10.0,0.20569361620099999]]}\n"
        filename1 = create_input(input1,'tdevfile.txt')
        filename2 = create_input(input2,'fdevfile.txt')

        commlist=["mode='rflag' spw='9,10' extendflags=False timedev='"+filename1+"' "+\
                   "freqdev='"+filename2+"'"]

#        flagdata(vis=self.vis,mode='unflag', flagbackup=False);
        flagcmd(vis=self.vis, inpmode='list', inpfile=commlist, action='apply', flagbackup=False)
        res3 = flagdata(vis=self.vis, mode='summary')
        print "(3) Finished flagcmd test : using tdevfile, fdevfile in the cmd (test 1)) : ", res3['flagged']


        # (4) Give the values directly in the cmd input.....
        commstr = "mode='rflag' spw='9,10' extendflags=False timedev=[[1.0,9.0,0.038859101518899999],[1.0,10.0,0.16283325492600001]] "+\
                   "freqdev=[[1.0,9.0,0.079151260994399994],[1.0,10.0,0.20569361620099999]]"
#        commlist=["mode='rflag' spw='9,10' extendflags=False ' \
#                  'timedev=[[1.0,9.0,0.038859],[1.0,10.0,0.162833]] \
#                          freqdev=[[1.0,9.0,0.079151],[1.0,10.0,0.205693]]"]

        flagdata(vis=self.vis,mode='unflag', flagbackup=False);
        flagcmd(vis=self.vis, inpmode='list', inpfile=[commstr], action='apply', flagbackup=False)
        res4 = flagdata(vis=self.vis, mode='summary')

        print "(4) Finished flagcmd test : using cmd arrays : ", res4['flagged']


        # (5) Use the outcmd.txt file generated by (2). 
        #       i.e. re-run the threshold-generation of (2) with savepars=True
        flagdata(vis=self.vis,mode='unflag', flagbackup=False);
        flagdata(vis=self.vis, mode='rflag', spw='9,10', timedev='', \
                      freqdev=[],action='calculate',savepars=True,outfile='outcmd.txt',
                      extendflags=False);
        flagcmd(vis=self.vis, inpmode='list', inpfile='outcmd.txt');
        res5 = flagdata(vis=self.vis, mode='summary')
        print "(5) Finished flagcmd test : using outcmd.txt from flagdata (test 2) : ", res5['flagged']

        self.assertEqual(res3['flagged'],res4['flagged']);
        self.assertEqual(res3['flagged'],res5['flagged']);
        self.assertEqual(res3['flagged'], 39504.0)

    def test_rflagauto(self):
        """flagcmd:: Test of rflag with defaults
        """
        # (6) flagcmd AUTO. Should give same answers as test_flagdata[test_rflag1]
        flagdata(vis=self.vis,mode='unflag');
        flagcmd(vis=self.vis, inpmode='list', inpfile=['mode=rflag spw=9,10 extendflags=False'], 
                action='apply',
                flagbackup=False)
        res6 = flagdata(vis=self.vis, mode='summary')
        print "(6) Finished flagcmd test : auto : ", res6['flagged']

        #(7) flagdata AUTO (same as test_flagdata[test_rflag1])
        #flagdata(vis=self.vis,mode='unflag');
        #flagdata(vis=self.vis, mode='rflag', spw='9,10');
        #res7 = flagdata(vis=self.vis, mode='summary')
        #print "\n---------------------- Finished flagdata test : auto : ", res7['flagged']

        self.assertEqual(res6['flagged'], 42728.0)


class test_actions(test_base):
    
    def setUp(self):
        self.setUp_data4rflag()
        
    def tearDown(self):
        pass
#         if os.path.exists('fourplot.png'):
#             os.remove('fourplot.png')
        
    def test_action_plot_table(self):
        '''flagcmd: Test action=plot, nothing plotted'''
        outplot = 'noplot.png'
        flagcmd(vis=self.vis, inpmode='list', 
            inpfile=["intent='CAL*POINT*' field=''","scan='5'"], 
            action='list', savepars=True)
        
        flagcmd(vis=self.vis, inpmode='table', useapplied=True, action='plot',
                plotfile=outplot)
        
        self.assertTrue(os.path.exists(outplot),'Plot file was not created')

    # CAS-5180
    def test_action_plot_list(self):
        '''flagcmd: Test action=plot to plot 4 antennas and no timerange'''
        outplot = 'fourplot.png'
        cmds = ["antenna='ea01' reason='none'",
                "antenna='ea11' reason='no_reason'",
                "antenna='ea19' reason='none'",
                "antenna='ea24' reason='other'"]
        
        flagcmd(vis=self.vis, inpmode='list', inpfile=cmds, action='plot',plotfile=outplot)
                
        self.assertTrue(os.path.exists(outplot),'Plot file was not created')
        
    def test_action_list1(self):
         '''flagcmd: action=list with inpmode from a list'''
         flagcmd(vis=self.vis, action='clear', clearall=True)         
         cmd = ["spw='5~7'","spw='1'"]
         flagcmd(vis=self.vis, action='list', inpmode='list', inpfile=cmd, savepars=True)
         
         # Apply the flags
         flagcmd(vis=self.vis)
         
         res=flagdata(vis=self.vis, mode='summary')
         self.assertEqual(res['flagged'],1099776)  
        
    def test_action_list2(self):
         '''flagcmd: action=list with inpmode from a file'''
         flagcmd(vis=self.vis, action='clear', clearall=True)         
         cmd = "spw='5~7'\n"+\
                "spw='1'"
         filename = create_input(cmd)         
         flagcmd(vis=self.vis, action='list', inpmode='list', inpfile=filename, savepars=True)

         # Apply the flags
         flagcmd(vis=self.vis)
         
         res=flagdata(vis=self.vis, mode='summary')
         self.assertEqual(res['flagged'],1099776)  

    def test_CAS4819(self):
        '''flagcmd: CAS-4819, Flag commands from three files'''
        self.setUp_ngc5921()
        # creat first input file
        myinput = "scan='1'\n"\
                "scan='2'\n"\
                "scan='3'"
        filename1 = 'list7a.txt'
        create_input1(myinput, filename1)
        
        # Create second input file
        myinput = "scan='5'\n"\
                "scan='6'\n"\
                "scan='7'"        
        filename2 = 'list7b.txt'
        create_input1(myinput, filename2)
        
         # Create third input file
        myinput = "scan='4' mode='clip' clipminmax=[0,4]" 
        filename3 = 'list7c.txt'
        create_input1(myinput, filename3)
       
        flagcmd(vis=self.vis, inpmode='list', inpfile=[filename1,filename2,filename3],
                 flagbackup=False, action='apply')
        
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['1']['flagged'], 568134)
        self.assertEqual(res['scan']['2']['flagged'], 238140)
        self.assertEqual(res['scan']['3']['flagged'], 762048)
        self.assertEqual(res['scan']['4']['flagged'], 6696)
        self.assertEqual(res['scan']['5']['flagged'], 142884)
        self.assertEqual(res['scan']['6']['flagged'], 857304)
        self.assertEqual(res['scan']['7']['flagged'], 190512)
        self.assertEqual(res['total'],2854278)
        self.assertEqual(res['flagged'],2765718)

    def test_list_reason1(self):
        '''flagcmd: select by reason from two files'''
        self.setUp_ngc5921()
        # creat first input file
        myinput = "scan='1' spw='0:10~20' reason='NONE'\n"\
                "scan='2' reason='EVEN'\n"\
                "scan='3' reason='ODD'"
        filename1 = 'reasonfile1.txt'
        create_input1(myinput, filename1)
        
        # Create second input file
        myinput = "scan='5' reason='ODD'\n"\
                "scan='6' reason='EVEN'\n"\
                "scan='7' reason='ODD'"        
        filename2 = 'reasonfile2.txt'
        create_input1(myinput, filename2)
        
        # Apply flag cmds on ODD reason
        flagcmd(vis=self.vis, inpmode='list', inpfile=[filename1,filename2], reason='ODD',
                 flagbackup=False, action='apply')
        
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['3']['flagged'], 762048)
        self.assertEqual(res['scan']['5']['flagged'], 142884)
        self.assertEqual(res['scan']['6']['flagged'], 0)
        self.assertEqual(res['scan']['7']['flagged'], 190512)
        self.assertEqual(res['flagged'], 762048+142884+190512)
        
        
class test_cmdbandpass(test_base):
    """Flagcmd:: Test flagging task with Bpass-based CalTable """
    
    def setUp(self):
        self.setUp_bpass_case()

    def test_unsupported_mode_in_list(self):
        '''Flagcmd: elevation and shadow are not supported in cal tables'''
        res = flagcmd(vis=self.vis, inpmode='list', inpfile=["mode='elevation'",
                                                             "spw='1'"])
        
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['spw']['1']['flagged'], 83200)

    def test_default_cparam(self):
        '''Flagcmd: flag CPARAM data column'''
        flist = ["mode='clip' clipzeros=True datacolumn='CPARAM'"]
        flagcmd(vis=self.vis, inpmode='list', inpfile=flist,
                flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 11078, 'Should use CPARAM as the default column')
        
    def test_manual_field_selection_for_bpass(self):
        """Flagcmd:: Manually flag a bpass-based CalTable using field selection"""
        
        flagcmd(vis=self.vis, inpmode='list', inpfile=["field='3C286_A'"], flagbackup=False)
        summary=flagdata(vis=self.vis, mode='summary')
        
        self.assertEqual(summary['field']['3C286_A']['flagged'], 499200.0)
        self.assertEqual(summary['field']['3C286_B']['flagged'], 0)
        self.assertEqual(summary['field']['3C286_C']['flagged'], 0)
        self.assertEqual(summary['field']['3C286_D']['flagged'], 0)

    def test_list_field_Selection_for_bpass(self):
        """Flagcmd:: Manually flag a bpass-based CalTable using file in list mode """
        
        myinput = "field='3C286_A'"
        filename = create_input(myinput)

        flagcmd(vis=self.vis, inpmode='list', inpfile=filename, flagbackup=False)
        summary=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(summary['field']['3C286_A']['flagged'], 499200.0)
        self.assertEqual(summary['field']['3C286_B']['flagged'], 0)
        self.assertEqual(summary['field']['3C286_C']['flagged'], 0)
        self.assertEqual(summary['field']['3C286_D']['flagged'], 0)

    def test_MS_flagcmds(self):
        """Flagcmd:: Save flags to MS and apply to cal table"""
        self.setUp_data4rflag()
        msfile = self.vis
        flagcmd(vis=msfile, action='clear', clearall=True)
        flagdata(vis=msfile, antenna='ea09', action='', savepars=True)
        
        self.setUp_bpass_case()
        flagcmd(vis=self.vis, inpfile=msfile, action='apply', flagbackup=False)
        summary=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(summary['antenna']['ea09']['flagged'], 48000.0)
        self.assertEqual(summary['antenna']['ea10']['flagged'], 0.0)
        
    def test_clip_one_list(self):
        '''Flagcmd: Flag one solution using one command in a list'''
        flist = ["mode='clip' clipminmax=[0,3] correlation='REAL_Sol1' datacolumn='CPARAM'"]
        flagcmd(vis=self.vis, inpmode='list', inpfile=flist)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 309388)
        self.assertEqual(res['correlation']['Sol2']['flagged'], 0)
                
    def test_flagbackup(self):
        '''Flagcmd: backup cal table flags'''

        flagmanager(vis=self.vis, mode='list')
        aflocal.open(self.vis)
        self.assertEqual(len(aflocal.getflagversionlist()), 2)
        aflocal.done()

        flagcmd(vis=self.vis, inpmode='list', inpfile=["spw='3'"])
        flagmanager(vis=self.vis, mode='list')
        aflocal.open(self.vis)
        self.assertEqual(len(aflocal.getflagversionlist()), 3)
        aflocal.done()

        flagcmd(vis=self.vis, inpmode='list', inpfile=["spw='4'"], flagbackup=False)
        flagmanager(vis=self.vis, mode='list')
        aflocal.open(self.vis)
        self.assertEqual(len(aflocal.getflagversionlist()), 3)
        aflocal.done()
        
        newname = 'BackupBeforeSpwFlags'

        flagmanager(vis=self.vis, mode='rename', oldname='flagcmd_1', versionname=newname, 
                    comment='Backup of flags before applying flags on spw')
        flagmanager(vis=self.vis, mode='list')
        aflocal.open(self.vis)
        self.assertEqual(len(aflocal.getflagversionlist()), 3)
        aflocal.done()
        
        # Apply spw=5 flags
        flagcmd(vis=self.vis, inpmode='list', inpfile=["spw='5'"], flagbackup=True)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['spw']['3']['flagged'], 83200)
        self.assertEqual(res['spw']['4']['flagged'], 83200)
        self.assertEqual(res['spw']['5']['flagged'], 83200)
        self.assertEqual(res['flagged'], 83200*3)
        
        # Restore backup
        flagmanager(vis=self.vis, mode='restore', versionname='BackupBeforeSpwFlags', merge='replace')
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['spw']['5']['flagged'], 0)
        self.assertEqual(res['flagged'], 0)

    def test_save_cal(self):
        '''Flagcmd: list flag cmds from MS and save to a file'''
        self.setUp_data4rflag()
        msfile = self.vis
        flagcmd(vis=msfile, action='clear', clearall=True)
        
        # Save cmds to FLAG_CMD table
        flagcmd(vis=msfile, inpmode='list', inpfile=["spw='1,3",
                                                     "mode='clip' clipminmax=[0,500] datacolumn='SNR'"], 
                action='list', savepars=True)
        
        self.setUp_bpass_case()
        flagcmds = 'calflags.txt'
        if os.path.exists(flagcmds):
            os.system('rm -rf '+flagcmds)
            
        # Apply to cal table and save to an external file
        flagcmd(vis=self.vis, inpmode='table', inpfile=msfile, savepars=True, outfile=flagcmds)
        self.assertTrue(os.path.exists(flagcmds))
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 246315)
        
        # Apply from list in file and compare
        flagdata(vis=self.vis, mode='unflag')
        flagcmd(vis=self.vis, inpmode='list', inpfile=flagcmds, action='apply')
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 246315)

    def test_cal_time_field(self):
        '''Flagcmd: clip a timerange from a field'''
        # this timerange corresponds to field 3C286_D
        flags = "mode='clip' timerange='>14:58:33.6' clipzeros=True clipminmax=[0.,0.4]"\
                " datacolumn='CPARAM'"
        
        # Apply the flags
        flagcmd(vis=self.vis, inpmode='list', inpfile=[flags], flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['field']['3C286_A']['flagged'],0)
        self.assertEqual(res['field']['3C286_B']['flagged'],0)
        self.assertEqual(res['field']['3C286_C']['flagged'],0)
        self.assertEqual(res['field']['3C286_D']['flagged'],2221)
        self.assertEqual(res['flagged'],2221)

    def test_cal_observation(self):
        '''Flagcmd: flag an observation from an old cal table format'''
        # Note: this cal table does not have an observation column. 
        # The column and sub-table should be added and the flagging
        # should happen after this.
        flagcmd(vis=self.vis, inpmode='list', inpfile=["observation='0'"], flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'],1248000)
        self.assertEqual(res['total'],1248000)

        
# Dummy class which cleans up created files
class cleanup(test_base):
    
    def tearDown(self):
        os.system('rm -rf multiobs.ms*')
        os.system('rm -rf uid___A002_X30a93d_X43e_small.ms*')
        os.system('rm -rf ngc5921.ms*')
        os.system('rm -rf Four_ants*.ms*')
        os.system('rm -rf shadowtest*.ms*')
        os.system('rm -rf tosr0001_scan3*.ms*')
        os.system('rm -rf cal.fewscans.bpass*')

    def test1(self):
        '''flagcmd: Cleanup'''
        pass


def suite():
    return [test_manual,
            test_alma,
            test_unapply,
            test_savepars,
            test_XML,
            test_shadow,
            test_rflag,
            test_actions,
            test_cmdbandpass,
            cleanup]
        
        
        
        
        
