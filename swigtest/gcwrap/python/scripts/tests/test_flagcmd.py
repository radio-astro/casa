import shutil
import unittest
import os
import filecmp
from tasks import *
from taskinit import *
from __main__ import default

#
# Test of flagcmd task. It uses tflagdata to unflag and summary
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
        default(flagcmd)

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
        default(flagcmd)

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
        default(flagcmd)

        
    def setUp_evla(self):
        self.vis = "tosr0001_scan3_noonline.ms"

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r ' + \
                      os.environ.get('CASAPATH').split()[0] +
                      "/data/regression/unittest/flagdata/" + self.vis + ' ' + self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        tflagdata(vis=self.vis, mode='unflag', savepars=False)
        default(flagcmd)
        
    def setUp_shadowdata(self):
        self.vis = "shadowtest_part.ms"

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r ' + \
                      os.environ.get('CASAPATH').split()[0] +
                      "/data/regression/unittest/flagdata/" + self.vis + ' ' + self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        tflagdata(vis=self.vis, mode='unflag', savepars=False)
        default(flagcmd)
        
    def setUp_data4rflag(self):
        self.vis = "Four_ants_3C286.ms"

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r ' + \
                      os.environ.get('CASAPATH').split()[0] +
                      "/data/regression/unittest/flagdata/" + self.vis + ' ' + self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        default(tflagdata)
        tflagdata(vis=self.vis, mode='unflag', savepars=False)

        
class test_manual(test_base):
    '''Test manual selections'''
    
    def setUp(self):
        self.setUp_multi()
        
    def test_observation(self):
        input = "observation='1'"
        filename = create_input(input)
        
        flagcmd(vis=self.vis, inpmode='file', inpfile=filename, action='apply', savepars=False)
        test_eq(tflagdata(vis=self.vis, mode='summary'), 2882778, 28500)

    def test_compatibility(self):
        input = "observation='1' mode='manualflag'"
        filename = create_input(input)
        
        flagcmd(vis=self.vis, inpmode='file', inpfile=filename, action='apply', savepars=False)
        test_eq(tflagdata(vis=self.vis, mode='summary'), 2882778, 28500)
        
    def test_autocorr(self):
        '''flagcmd: autocorr=True'''
        self.setUp_ngc5921()
        flagcmd(vis=self.vis, inpmode='cmd', command=['autocorr=True'], action='apply')
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 203994, 'Should flag only the auto-correlations')
        

class test_alma(test_base):
    # Test various selections for alma data 

    def setUp(self):
        self.setUp_flagdatatest_alma()
    
    def test_intent(self):
        '''flagcmd: test scan intent selection'''
        
        input = "intent='CAL*POINT*'\n"\
                "#scan=3,4"
        filename = create_input(input)
        
        # flag POINTING CALIBRATION scans and ignore comment line
        flagcmd(vis=self.vis, inpmode='file', inpfile=filename, action='apply', savepars=False)
        test_eq(tflagdata(vis=self.vis,mode='summary', antenna='2'), 377280, 26200)
        res = tflagdata(vis=self.vis,mode='summary')
        self.assertEqual(res['scan']['1']['flagged'], 80184, 'Only scan 1 should be flagged')
        self.assertEqual(res['scan']['4']['flagged'], 0, 'Scan 4 should not be flagged')
        
    def test_cmd(self):
        '''flagcmd: inpmode=cmd with empty parameter'''
        
        # Test the correct parsing with empty parameter such as antenna=''
        flagcmd(vis=self.vis, inpmode='cmd', 
                 command=["intent='CAL*POINT*' field=''","scan='3,4' antenna=''","scan='5'"], 
                 action='apply', savepars=False)
        res = tflagdata(vis=self.vis,mode='summary')
        self.assertEqual(res['scan']['1']['flagged'], 80184)
        self.assertEqual(res['scan']['4']['flagged'], 48132)
        self.assertEqual(res['flagged'], 160392)
        
             
    def test_extract(self):
        '''flagcmd: action = extract and apply clip on WVR'''
        # Remove any cmd from table
        flagcmd(vis=self.vis, action='clear', clearall=True)
        
        # Save cmd to FLAG_CMD
        cmd = "mode='clip' clipminmax=[0,50] correlation='ABS_WVR'"
        flagcmd(vis=self.vis, inpmode='cmd', command=[cmd], action='list', savepars=True)
        
        # Extract it
        res = flagcmd(vis=self.vis, action='extract', useapplied=True)
        
        # Apply to clip only WVR
        flagcmd(vis=self.vis, inpmode='cmd', command=[res[0]['command']], savepars=False, action='apply')
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
        '''flagcmd: unapply tfcrop agent'''
        # Remove any cmd from table
        flagcmd(vis=self.vis, action='clear', clearall=True)

        # Flag using manual agent
        input = "scan=1"
        filename = create_input(input)
        flagcmd(vis=self.vis, inpmode='file', inpfile=filename, action='apply', savepars=True)
        
        # Flag using tfcrop agent from file
        # Note : For this test, scan=4 gives identical flags on 32/64 bit machines,
        #           and one flag difference on a Mac (32)
        #           Other scans give differences at the 0.005% level.
        input = "scan=4 mode=tfcrop correlation='ABS_RR'"
        filename = create_input(input)
        flagcmd(vis=self.vis, inpmode='file', inpfile=filename, action='apply', savepars=True)
        res = tflagdata(vis=self.vis,mode='summary')
        self.assertEqual(res['scan']['1']['flagged'], 568134, 'Whole scan=1 should be flagged')
        #self.assertEqual(res['scan']['4']['flagged'], 1201, 'scan=4 should be partially flagged')
        self.assertTrue(res['scan']['4']['flagged']>= 1200 and res['scan']['4']['flagged']<= 1204, 'scan=4 should be partially flagged')
        
        # Unapply only the tfcrop line
        flagcmd(vis=self.vis, action='unapply', useapplied=True, tablerows=1, savepars=False)
        result = tflagdata(vis=self.vis,mode='summary',scan='4')
        self.assertEqual(result['flagged'], 0, 'Expected 0 flags, found %s'%result['flagged'])
        self.assertEqual(result['total'], 95256,'Expected total 95256, found %s'%result['total'])

    def test_uquack(self):
        '''flagcmd: unapply quack agent'''
        # Remove any cmd from table
        flagcmd(vis=self.vis, action='clear', clearall=True)

        # Flag using manual agent
        input = "scan=1"
        filename = create_input(input)
        flagcmd(vis=self.vis, inpmode='file', inpfile=filename, action='apply', savepars=True)

        # Flag using the quack agent
        input = "scan=1~3 mode=quack quackinterval=1.0"
        filename = create_input(input)
        flagcmd(vis=self.vis, inpmode='file', inpfile=filename, action='apply', savepars=True)
        
        # Unapply only the quack line
        flagcmd(vis=self.vis, action='unapply', useapplied=True, tablerows=1, savepars=True)
        result = tflagdata(vis=self.vis,mode='summary',scan='1')
        
        # Only the manual flags should be there
        self.assertEqual(result['flagged'], 568134, 'Expected 568134 flags, found %s'%result['flagged'])
        self.assertEqual(result['total'], 568134,'Expected total 568134, found %s'%result['total'])
        
    def test_umanualflag(self):
        '''flagcmd: unapply manual agent'''
        # Remove any cmd from table
        flagcmd(vis=self.vis, action='clear', clearall=True)

        # Flag using manual agent
        input = "scan=1"
        filename = create_input(input)
        flagcmd(vis=self.vis, inpmode='file', inpfile=filename, action='apply', savepars=True)

        # Flag using the quack agent
        input = "scan=1~3 mode=quack quackinterval=1.0"
        filename = create_input(input)
        flagcmd(vis=self.vis, inpmode='file', inpfile=filename, action='apply', savepars=True)
        
        # Unapply only the manual line
        flagcmd(vis=self.vis, action='unapply', useapplied=True, tablerows=0, savepars=False)
        result = tflagdata(vis=self.vis,mode='summary',scan='1')
        
        # Only the quack flags should be left
        self.assertEqual(result['flagged'], 44226, 'Expected 44226 flags, found %s'%result['flagged'])
        self.assertEqual(result['total'], 568134,'Expected total 568134, found %s'%result['total'])
        

    def test_uscans(self):
        '''flagcmd: Unapply only APPLIED=True'''
        # Remove any cmd from table
        flagcmd(vis=self.vis, action='clear', clearall=True)
        
        # Flag several scans and save them to FLAG_CMD with APPLIED=True
        tflagdata(vis=self.vis, scan='7', savepars=True)
        tflagdata(vis=self.vis, scan='1', savepars=True)
        tflagdata(vis=self.vis, scan='2', savepars=True)
        tflagdata(vis=self.vis, scan='3', savepars=True)
        tflagdata(vis=self.vis, scan='4', savepars=True)
        
        # There should be 5 cmds in FLAG_CMD. Unapply row=1 and set APPLIED to False
        flagcmd(vis=self.vis, action='unapply', tablerows=1, savepars=False)
        
        # Unapply scans 2 and 3 only. It should not re-apply scan=1 (row 1)
        flagcmd(vis=self.vis, action='unapply', tablerows=[2,3], savepars=False)
        
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
        '''flagcmd: list and savepars=True/False'''
        # Remove any cmd from table
        flagcmd(vis=self.vis, action='clear', clearall=True)
        
        ########## TEST 1 
        # create text file called flagcmd.txt
        input = "scan=4 mode=clip correlation=ABS_RR clipminmax=[0,4]\n"
        filename = create_input(input)
        filename1 = 'filename1.txt'
        os.system('cp '+filename+' '+filename1)

        # save command to MS
        flagcmd(vis=self.vis, action='list', inpmode='cmd', command=[input], savepars=True)
        
        # list/save to a file
        os.system('rm -rf myflags.txt')
        flagcmd(vis=self.vis, action='list', outfile='myflags.txt', savepars=True)
        
        # compare saved file with original input file
        self.assertTrue(filecmp.cmp(filename1, 'myflags.txt', 1), 'Files should be equal')
        
        ########## TEST 2 
        # create another input
        input = "scan=1~3 mode=manual\n"
        filename = create_input(input)
        
        # apply and don't save to MS
        flagcmd(vis=self.vis, inpmode='file', inpfile=filename, action='apply', savepars=False)
        
        # list and check that parameters were not saved to MS
        os.system('rm -rf myflags.txt')
        flagcmd(vis=self.vis, action='list', outfile='myflags.txt', savepars=True)
        self.assertFalse(filecmp.cmp(filename, 'myflags.txt', 1), 'Files should not be equal')
        
        ########### TEST 3 
        # apply cmd from TEST 1 and update APPLIED column
        flagcmd(vis=self.vis, action='apply', savepars=False)
        
        # scans=1~3 should be fully flagged
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['1']['flagged'], 568134)
        self.assertEqual(res['scan']['1']['total'], 568134)
        self.assertEqual(res['scan']['2']['flagged'], 238140)
        self.assertEqual(res['scan']['2']['total'], 238140)
        self.assertEqual(res['scan']['3']['flagged'], 762048)
        self.assertEqual(res['scan']['3']['total'], 762048)
        # scan 4 should be partially flagged by clip mode
        self.assertEqual(res['scan']['4']['flagged'], 3348, 'Only RR should be flagged')
        self.assertEqual(res['scan']['4']['total'], 95256)
        
        # Only cmd form TEST 1 should be in MS
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
        flagcmd(vis=self.vis, action='apply', reason=reasons, savepars=True, outfile='myxml.txt')
                
        # Compare with original XML
        self.assertTrue(filecmp.cmp('origxml.txt', 'myxml.txt',1), 'Files should be equal')
        
        # Check that APPLIED column has been updated to TRUE
        
        
    def test_xml2(self):
        '''flagcmd: list xml file and save in outfile'''
        
        # The MS only contains clip and shadow commands
        
        # Apply the shadow command
        flagcmd(vis=self.vis, action='apply', reason='SHADOW')
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 240640)

class test_shadow(test_base):
    def setUp(self):
        self.setUp_shadowdata()

    def test_CAS2399(self):
        '''flagcmd: shadow by antennas not present in MS'''
        
        # Create antennafile in disk
        input = 'name=VLA01\n'+\
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

        filename = create_input(input)

        # Create command line
        input = ["mode='shadow' tolerance=10.0 addantenna='flagcmd.txt'"]
#        filename = 'cmdfile.txt'
#        if os.path.exists(filename):
#            os.system('rm -rf cmdfile.txt')
        
#        create_input(input, filename)
        
        # Flag
        flagcmd(vis=self.vis, action='clear', clearall=True)
#        flagcmd(vis=self.vis, action='apply', inpmode='file', inpfile=filename)
        flagcmd(vis=self.vis, action='apply', inpmode='cmd', command=input)
        
        # Check flags
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['antenna']['VLA18']['flagged'], 3364)
        self.assertEqual(res['antenna']['VLA19']['flagged'], 1124)
        self.assertEqual(res['antenna']['VLA20']['flagged'], 440)        


# Test rflag inputs with filenames, as well as inline thresholds.
# rflag does not generate output thresholds for action='apply'.
class test_rflag(test_base):

    def setUp(self):
        self.setUp_data4rflag()

    def test_rflaginputs(self):
        """flagcmd:: Test of rflag threshold-inputs of both types (file and inline)
        """

        # (1) and (2) are the same as test_tflagdata[test_rflag3].
        # -- (3),(4),(5) should produce the same answers as (1) and (2)
        #
        # (1) Test input/output files, through the task, mode='rflag'
        #tflagdata(vis=self.vis, mode='rflag', spw='9,10', timedev='tdevfile.txt', \
        #              freqdev='fdevfile.txt', action='calculate');
        #tflagdata(vis=self.vis, mode='rflag', spw='9,10', timedev='tdevfile.txt', \
        #              freqdev='fdevfile.txt', action='apply');
        #res1 = tflagdata(vis=self.vis, mode='summary')
        #print "(1) Finished tflagdata : test 1 : ", res1['flagged']


        # (2) Test rflag output written to cmd file via mode='rflag' and 'savepars' 
        #      and then read back in via list mode. 
        #      Also test the 'savepars' when timedev and freqdev are specified differently...
        #tflagdata(vis=self.vis,mode='unflag');
        #tflagdata(vis=self.vis, mode='rflag', spw='9,10', timedev='', \
        #              freqdev=[],action='calculate',savepars=True,outfile='outcmd.txt');
        #os.system('cat outcmd.txt')
        #tflagdata(vis=self.vis, mode='list', inpfile='outcmd.txt');
        #res2 = tflagdata(vis=self.vis, mode='summary')
        #print "(2) Finished tflagdata : test 2 : ", res2['flagged']


        # (3) flagcmd : Send in the same text files produces/used in (1)
        input1 = "{'name':'Rflag','timedev':[[1.0,9.0,0.038859],[1.0,10.0,0.162833]]}\n"
        input2 = "{'name':'Rflag','freqdev':[[1.0,9.0,0.079151],[1.0,10.0,0.205693]]}\n"
        filename1 = create_input(input1,'tdevfile.txt')
        filename2 = create_input(input2,'fdevfile.txt')

        commlist=['mode=rflag spw=9,10 timedev='+filename1+' freqdev='+filename2]

        tflagdata(vis=self.vis,mode='unflag');
        flagcmd(vis=self.vis, inpmode='cmd', command=commlist, action='apply')
        res3 = tflagdata(vis=self.vis, mode='summary')
        print "(3) Finished flagcmd test : using tdevfile, fdevfile in the cmd (test 1)) : ", res3['flagged']


        # (4) Give the values directly in the cmd input.....
        commlist=['mode=rflag spw=9,10 timedev=[[1.0,9.0,0.038859],[1.0,10.0,0.162833]] \
                          freqdev=[[1.0,9.0,0.079151],[1.0,10.0,0.205693]]']

        tflagdata(vis=self.vis,mode='unflag');
        flagcmd(vis=self.vis, inpmode='cmd', command=commlist, action='apply')
        res4 = tflagdata(vis=self.vis, mode='summary')

        print "(4) Finished flagcmd test : using cmd arrays : ", res4['flagged']


        # (5) Use the outcmd.txt file generated by (2). 
        #       i.e. re-run the threshold-generation of (2) with savepars=True
        tflagdata(vis=self.vis,mode='unflag');
        tflagdata(vis=self.vis, mode='rflag', spw='9,10', timedev='', \
                      freqdev=[],action='calculate',savepars=True,outfile='outcmd.txt');
        flagcmd(vis=self.vis, inpmode='file', inpfile='outcmd.txt');
        res5 = tflagdata(vis=self.vis, mode='summary')
        print "(5) Finished flagcmd test : using outcmd.txt from tflagdata (test 2) : ", res5['flagged']

        self.assertEqual(res3['flagged'],res4['flagged']);
        self.assertEqual(res3['flagged'],res5['flagged']);
        self.assertEqual(res3['flagged'], 39504.0)

    def test_rflagauto(self):
        """flagcmd:: Test of rflag with defaults
        """
        # (6) flagcmd AUTO. Should give same answers as test_tflagdata[test_rflag1]
        tflagdata(vis=self.vis,mode='unflag');
        flagcmd(vis=self.vis, inpmode='cmd', command=['mode=rflag spw=9,10'], action='apply')
        res6 = tflagdata(vis=self.vis, mode='summary')
        print "(6) Finished flagcmd test : auto : ", res6['flagged']

        #(7) tflagdata AUTO (same as test_tflagdata[test_rflag1])
        #tflagdata(vis=self.vis,mode='unflag');
        #tflagdata(vis=self.vis, mode='rflag', spw='9,10');
        #res7 = tflagdata(vis=self.vis, mode='summary')
        #print "\n---------------------- Finished tflagdata test : auto : ", res7['flagged']

        self.assertEqual(res6['flagged'], 42728.0)


#################################################
        
# Dummy class which cleans up created files
class cleanup(test_base):
    
    def tearDown(self):
        os.system('rm -rf multiobs.ms*')
        os.system('rm -rf flagdatatest-alma.ms*')
        os.system('rm -rf ngc5921.ms*')


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
            cleanup]
        
        
        
        
        
