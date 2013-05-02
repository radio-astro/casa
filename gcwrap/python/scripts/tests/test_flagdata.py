import shutil
import unittest
import os
import filecmp
from tasks import *
from taskinit import *
from __main__ import default
import exceptions
from parallel.parallel_task_helper import ParallelTaskHelper

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

print 'flagdata tests will use data from '+datapath         

# jagonzal (CAS-4287): Add a cluster-less mode to by-pass parallel processing for MMSs as requested 
if os.environ.has_key('BYPASS_SEQUENTIAL_PROCESSING'):
    ParallelTaskHelper.bypassParallelProcessing(1)

# Base class which defines setUp functions
# for importing different data sets
class test_base(unittest.TestCase):
    def setUp_flagdatatest(self):
        self.vis = "flagdatatest.ms"
        if testmms:
            self.vis = "flagdatatest.mms"

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r '+datapath + self.vis +' '+ self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        
        default(flagdata)
        flagdata(vis=self.vis, mode='unflag', savepars=False)

    def setUp_ngc5921(self):
        self.vis = "ngc5921.ms"
        if testmms:
            self.vis = 'ngc5921.mms'

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            os.system('cp -r '+datapath + self.vis +' '+ self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        default(flagdata)
        flagdata(vis=self.vis, mode='unflag', savepars=False)

    def setUp_flagdatatest_alma(self):
        self.vis = "flagdatatest-alma.ms"
        if testmms:
            self.vis = 'flagdatatest-alma.mms'

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r '+datapath + self.vis +' '+ self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        default(flagdata)
        flagdata(vis=self.vis, mode='unflag', savepars=False)

    def setUp_data4tfcrop(self):
        self.vis = "Four_ants_3C286.ms"
        if testmms:
            self.vis = 'Four_ants_3C286.mms'

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r '+datapath + self.vis +' '+ self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        default(flagdata)
        flagdata(vis=self.vis, mode='unflag', savepars=False)

    def setUp_shadowdata2(self):
        self.vis = "shadowtest_part.ms"
        if testmms:
            self.vis = "shadowtest_part.mms"

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r '+datapath + self.vis +' '+ self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        default(flagdata)
        flagdata(vis=self.vis, mode='unflag', savepars=False)
        
    def setUp_multi(self):
        self.vis = "multiobs.ms"
        if testmms:
            self.vis = 'multiobs.mms'

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r '+datapath + self.vis +' '+ self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        default(flagdata)
        flagdata(vis=self.vis, mode='unflag', savepars=False)
        
    def setUp_mwa(self):
        self.vis = "testmwa.ms"
        if testmms:
            self.vis = 'testmwa.mms'

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r '+datapath + self.vis +' '+ self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        default(flagdata)
        flagdata(vis=self.vis, mode='unflag', savepars=False)        

    def setUp_tsys_case(self):
        self.vis = "X7ef.tsys"
         
        if os.path.exists(self.vis):
            print "The CalTable is already around, just unflag"
            
        else:
            print "Moving data..."
            os.system('cp -r ' + \
                        os.environ.get('CASAPATH').split()[0] +
                        "/data/regression/unittest/flagdata/" + self.vis + ' ' + self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')

        flagdata(vis=self.vis, mode='unflag', savepars=False)

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
        
        flagdata(vis=self.vis, mode='unflag', savepars=False)

    def setUp_newcal(self):
        '''New cal table format from 4.1 onwards'''
        self.vis = "ap314.gcal"

        if os.path.exists(self.vis):
            print "The CalTable is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r ' + \
                        os.environ.get('CASAPATH').split()[0] +
                        "/data/regression/unittest/flagdata/" + self.vis + ' ' + self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        
        flagdata(vis=self.vis, mode='unflag', flagbackup=False)


class test_tfcrop(test_base):
    """flagdata:: Test of mode = 'tfcrop'"""
    
    def setUp(self):
        self.setUp_data4tfcrop()
        
    def test_tfcrop1(self):
        '''flagdata:: Test1 of mode = tfcrop'''
        flagdata(vis=self.vis, mode='tfcrop', correlation='ABS_RR',ntime=51.0,spw='9', savepars=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 4489)
        self.assertEqual(res['antenna']['ea19']['flagged'], 2294)
        self.assertEqual(res['spw']['7']['flagged'], 0)
        
    def test_tfcrop2(self):
        '''flagdata:: Test2 of mode = tfcrop ABS_ALL'''
        # Note : With ntime=51.0, 64-bit machines get 18696 flags, and 32-bit gets 18695 flags.
        #           As far as we can determine, this is a genuine precision-related difference.
        #           With ntime=53.0, there happens to be no difference.
        flagdata(vis=self.vis, mode='tfcrop',ntime=53.0,spw='9', savepars=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 18671)
        self.assertEqual(res['correlation']['LL']['flagged'], 4250)
        self.assertEqual(res['correlation']['RL']['flagged'], 5007)
        self.assertEqual(res['correlation']['LR']['flagged'], 4931)
        self.assertEqual(res['correlation']['RR']['flagged'], 4483)

    def test_extendpols(self):
        '''flagdata:: Extend the flags created by clip'''
        flagdata(vis=self.vis, mode='clip', correlation='abs_rr', clipminmax=[0,2])
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['correlation']['RR']['flagged'], 43)
        self.assertEqual(res['correlation']['LL']['flagged'], 0)
        flagdata(vis=self.vis, mode='extend', extendpols=True, savepars=False)
        test_eq(flagdata(vis=self.vis, mode='summary', correlation='Ll'), 1099776, 43)
        
    def test_extendtime(self):
        '''flagdata:: Extend the flags created by tfcrop'''
        flagdata(vis=self.vis, mode='tfcrop')
        # The total per spw:channel/baseline/correlation/scan is 89.
        # Show only the ones that are 50% of the total, 44 flags, These should grow
        pre = flagdata(vis=self.vis, mode='summary', spwchan=True, basecnt=True, correlation='RR',spw='5',
                       antenna='ea11&&ea19')
        # these should grow later
        self.assertEqual(pre['spw:channel']['5:10']['flagged'], 118)
        self.assertEqual(pre['spw:channel']['5:10']['total'], 179)
        self.assertEqual(pre['spw:channel']['5:28']['flagged'], 128)
        self.assertEqual(pre['spw:channel']['5:29']['flagged'], 151)

        # these should not grow later. After the consolidation of MMS summaries
        # is fixed, revise this test
#        self.assertEqual(pre['spw:channel']['5:11']['flagged'], 32)
#        self.assertEqual(pre['spw:channel']['5:12']['flagged'], 29)
#        self.assertEqual(pre['spw:channel']['5:21']['flagged'], 34)
        
        # Extend in time only
        flagdata(vis=self.vis, mode='extend', extendpols=False, growtime=50.0, growfreq=0.0, 
                 growaround=False,flagneartime=False,flagnearfreq=False,savepars=False)
        pos = flagdata(vis=self.vis, mode='summary', spwchan=True, basecnt=True, correlation='RR',spw='5',
                       antenna='ea11&&ea19')
        self.assertEqual(pos['spw:channel']['5:10']['flagged'], 179)
        self.assertEqual(pos['spw:channel']['5:10']['total'], 179)
        self.assertEqual(pos['spw:channel']['5:28']['flagged'], 179)
        self.assertEqual(pos['spw:channel']['5:29']['flagged'], 179)
        
        # These did not grow
#        self.assertEqual(pos['spw:channel']['5:11']['flagged'], 32)
#        self.assertEqual(pos['spw:channel']['5:12']['flagged'], 29)
#        self.assertEqual(pos['spw:channel']['5:21']['flagged'], 34)
        
    def test_extendfreq(self):
        '''flagdata:: Extend the flags created manually for one scan only'''
        flagdata(vis=self.vis, mode='manual',spw='*:0~35',timerange='2010/10/16/14:45:00~14:45:20')
        pre = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(pre['scan']['31']['flagged'], 0)
        self.assertEqual(pre['scan']['30']['flagged'], 165888)
        self.assertEqual(pre['flagged'], 165888)
        
        # Extend in frequency only
        flagdata(vis=self.vis, mode='extend', extendpols=False, growtime=0.0, growfreq=50.0, savepars=False)
        pos = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(pos['scan']['31']['flagged'], 0)
        self.assertEqual(pos['flagged'], 294912)
        
        

class test_rflag(test_base):
    """flagdata:: Test of mode = 'rflag'"""
    
    def setUp(self):
        self.setUp_data4tfcrop()
        
    def test_rflag1(self):
        '''flagdata:: Test1 of mode = rflag : automatic thresholds'''
        flagdata(vis=self.vis, mode='rflag', spw='9,10', timedev=[], freqdev=[], flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 42728.0)
        self.assertEqual(res['antenna']['ea19']['flagged'], 18411.0)
        self.assertEqual(res['spw']['7']['flagged'], 0)

    def test_rflag2(self):
        '''flagdata:: Test2 of mode = rflag : partially-specified thresholds'''
        flagdata(vis=self.vis, mode='rflag', spw='9,10', timedev=[[1,10,0.1],[1,11,0.07]], \
                       freqdev=0.5, flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary',spw='9,10,11')
        self.assertEqual(res['flagged'], 52411)
        self.assertEqual(res['antenna']['ea19']['flagged'], 24142)
        self.assertEqual(res['spw']['11']['flagged'], 0)

    def test_rflag3(self):
        '''flagdata:: Test3 of mode = rflag : output/input via two methods'''
        # (1) Test input/output files, through the task, mode='rflag'
        flagdata(vis=self.vis, mode='rflag', spw='9,10', timedev='tdevfile.txt', \
                      freqdev='fdevfile.txt', action='calculate');
        flagdata(vis=self.vis, mode='rflag', spw='9,10', timedev='tdevfile.txt', \
                      freqdev='fdevfile.txt', action='apply', flagbackup=False)
        res1 = flagdata(vis=self.vis, mode='summary')
        # (2) Test rflag output written to cmd file via mode='rflag' and 'savepars' 
        #      and then read back in via list mode. 
        #      Also test the 'savepars' when timedev and freqdev are specified differently...
        flagdata(vis=self.vis,mode='unflag', flagbackup=False)
        flagdata(vis=self.vis, mode='rflag', spw='9,10', timedev='', \
                      freqdev=[],action='calculate',savepars=True,outfile='outcmd.txt');
        flagdata(vis=self.vis, mode='list', inpfile='outcmd.txt', flagbackup=False)
        res2 = flagdata(vis=self.vis, mode='summary')

        #print res1['flagged'], res2['flagged']
        self.assertTrue(abs(res1['flagged']-res2['flagged'])<10000)
        self.assertTrue(abs(res1['flagged']-39504.0)<10000)

    def test_rflag4(self):
        '''flagdata:: Test4 of mode = rflag : correlation selection'''
        flagdata(vis=self.vis, mode='rflag', spw='9,10', correlation='rr,ll', flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['correlation']['RR']['flagged'], 9781.0)
        self.assertEqual(res['correlation']['LL']['flagged'], 10355.0)
        self.assertEqual(res['correlation']['LR']['flagged'], 0,)
        self.assertEqual(res['correlation']['RL']['flagged'], 0,)
        
    def test_rflag_CAS_5037(self):
        '''flagdata:: Use provided value for time stats. but automatically computed value for freq. stats'''
        flagdata(vis=self.vis, mode='rflag', field = '1', spw='10', timedev=0.1, \
                 timedevscale=5.0, freqdevscale=5.0, action='calculate', flagbackup=False)

class test_shadow(test_base):
    def setUp(self):
        self.setUp_shadowdata2()

    def test_CAS2399(self):
        '''flagdata: shadow by antennas not present in MS'''
        
        if os.path.exists("cas2399.txt"):
            os.system('rm -rf cas2399.txt')
        
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

        filename = 'cas2399.txt'
        create_input(myinput, filename)
        
        flagdata(vis=self.vis, mode='shadow', tolerance=0.0, addantenna=filename,flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        ##print res['antenna']['VLA3']['flagged'], res['antenna']['VLA4']['flagged'], res['antenna']['VLA5']['flagged']
        self.assertEqual(res['antenna']['VLA3']['flagged'], 3752)
        self.assertEqual(res['antenna']['VLA4']['flagged'], 1320)
        self.assertEqual(res['antenna']['VLA5']['flagged'], 1104)
        
    def test_addantenna(self):
        '''flagdata: use antenna file in list mode'''
        if os.path.exists("myants.txt"):
            os.system('rm -rf myants.txt')
        
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

        antfile = 'myants.txt'
        create_input(myinput, antfile)
        
        # Create list file
        myinput = "mode='shadow' tolerance=0.0 addantenna='myants.txt'"
        filename = 'listfile.txt'
        create_input(myinput, filename)
        
        # Flag
        flagdata(vis=self.vis, mode='list', inpfile=filename, savepars=True, outfile='withdict.txt',
                 flagbackup=False)
        
        # Check flags
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['antenna']['VLA3']['flagged'], 3752)
        self.assertEqual(res['antenna']['VLA4']['flagged'], 1320)
        self.assertEqual(res['antenna']['VLA5']['flagged'], 1104)

class test_flagmanager(test_base):
    
    def setUp(self):
        os.system("rm -rf flagdatatest.ms*") # test1 needs a clean start
        self.setUp_flagdatatest()
        
    def test1m(self):
        '''flagmanager test1m: mode=list, flagbackup=True/False'''
        
        # Create a local copy of the tool
        aflocal = casac.agentflagger()
        flagmanager(vis=self.vis, mode='list')
        aflocal.open(self.vis)
        self.assertEqual(len(aflocal.getflagversionlist()), 3)
        aflocal.done()


        flagdata(vis=self.vis, mode='unflag', flagbackup=False)
        flagmanager(vis=self.vis, mode='list')
        aflocal.open(self.vis)
        self.assertEqual(len(aflocal.getflagversionlist()), 3)
        aflocal.done()

        flagdata(vis=self.vis, mode='unflag', flagbackup=True)
        flagmanager(vis=self.vis, mode='list')
        aflocal.open(self.vis)
        self.assertEqual(len(aflocal.getflagversionlist()), 4)
        aflocal.done()
        
        newname = 'Ha! The best version ever!'

        flagmanager(vis=self.vis, mode='rename', oldname='flagdata_2', versionname=newname, 
                    comment='This is a *much* better name')
        flagmanager(vis=self.vis, mode='list')
        aflocal.open(self.vis)
        self.assertEqual(len(aflocal.getflagversionlist()), 4)
        aflocal.done()
        
        self.assertTrue(os.path.exists(self.vis+'.flagversions/flags.'+newname),
                        'Flagversion file does not exist: flags.'+newname)
        
        # Specific for MMS
        if testmms:
            areg = self.vis+'/SUBMSS/*flagversions*'
            import glob
            print 'Check for .flagversions in the wrong place.'
            self.assertEqual(glob.glob(areg), [], 'There should not be any .flagversions in the'
                                                ' SUBMSS directory')
            

    def test2m(self):
        """flagmanager test2m: Create, then restore autoflag"""

        flagdata(vis=self.vis, mode='summary')
        flagmanager(vis=self.vis)
        
        flagdata(vis=self.vis, mode='manual', antenna="2", flagbackup=True)
        
        flagmanager(vis=self.vis)
        ant2 = flagdata(vis=self.vis, mode='summary')['flagged']

        print "After flagging antenna 2 there were", ant2, "flags"

        # Change flags, then restore
        flagdata(vis=self.vis, mode='manual', antenna="3", flagbackup=True)
        flagmanager(vis = self.vis)
        ant3 = flagdata(vis=self.vis, mode='summary')['flagged']

        print "After flagging antenna 2 and 3 there were", ant3, "flags"

        flagmanager(vis=self.vis, mode='restore', versionname='flagdata_3')
        restore2 = flagdata(vis=self.vis, mode='summary')['flagged']

        print "After restoring pre-antenna 3 flagging, there are", restore2, "flags; should be", ant2

        self.assertEqual(restore2, ant2)

    def test_CAS2701(self):
        """flagmanager: Do not allow flagversion=''"""
        
        # Create a local copy of the tool
        aflocal = casac.agentflagger()
        
        aflocal.open(self.vis)
        l = len(aflocal.getflagversionlist())
        aflocal.done()
        
        flagmanager(vis = self.vis,
                    mode = "save",
                    versionname = "non-empty-string")

        aflocal.open(self.vis)
        self.assertEqual(len(aflocal.getflagversionlist()), l+1)
        aflocal.done()

        flagmanager(vis = self.vis,
                    mode = "save",
                    versionname = "non-empty-string")

        aflocal.open(self.vis)
        self.assertEqual(len(aflocal.getflagversionlist()), l+1)
        aflocal.done()


class test_msselection(test_base):

    def setUp(self):
        self.setUp_ngc5921()

    def test_simple(self):
        '''flagdata: select only cross-correlations'''
        baselines = flagdata(vis = self.vis, mode="summary", antenna="VA09", basecnt=True)['baseline'].keys()
        assert "VA09&&VA09" not in baselines
        assert "VA09&&VA10" in baselines
        assert "VA09&&VA11" in baselines
        assert "VA10&&VA10" not in baselines
        assert "VA10&&VA11" not in baselines

        baselines = flagdata(vis = self.vis, mode="summary", antenna="VA09,VA10", basecnt=True)['baseline'].keys()
        assert "VA09&&VA09" not in baselines
        assert "VA09&&VA10" in baselines
        assert "VA09&&VA11" in baselines
        assert "VA10&&VA10" not in baselines
        assert "VA10&&VA11" in baselines

    def test_amp(self):
        '''flagdata: select only cross-correlations'''
        baselines = flagdata(vis = self.vis, mode="summary", antenna="VA09,VA10&",basecnt=True)['baseline'].keys()
        assert "VA09&&VA09" not in baselines
        assert "VA09&&VA10" in baselines
        assert "VA09&&VA11" not in baselines
        assert "VA10&&VA10" not in baselines
        assert "VA10&&VA11" not in baselines

        baselines = flagdata(vis = self.vis, mode="summary", antenna="VA09&VA10",basecnt=True)['baseline'].keys()
        assert "VA09&&VA09" not in baselines
        assert "VA09&&VA10" in baselines
        assert "VA09&&VA11" not in baselines
        assert "VA10&&VA10" not in baselines
        assert "VA10&&VA11" not in baselines
        
    def test_autocorr1(self):
        '''flagdata: flag only auto-correlations with antenna selection'''
        flagdata(vis=self.vis, mode='manual', antenna='VA05&&&', flagbackup=False)
        s = flagdata(vis = self.vis, mode="summary",basecnt=True)['baseline']
        assert s['VA05&&VA05']['flagged'] == 7560
        assert s['VA01&&VA05']['flagged'] == 0
        assert s['VA02&&VA05']['flagged'] == 0
        assert s['VA05&&VA10']['flagged'] == 0
        assert s['VA05&&VA11']['flagged'] == 0

        s = flagdata(vis = self.vis, mode="summary")
        self.assertEqual(s['flagged'], 7560)

    def test_autocorr2(self):
        '''flagdata: flag auto-corrs with parameter'''
        flagdata(vis=self.vis, autocorr=True, flagbackup=False)
        s = flagdata(vis = self.vis, mode="summary")
        self.assertEqual(s['flagged'], 203994)
        
    def test_autocorr3(self):
        '''flagdata: flag auto-corrs in list mode'''
        # creat input list
        myinput = "scan='1' mode='manual' autocorr=true reason='AUTO'\n"\
                "scan='3' autocorr=True reason='AUTO'\n"\
                "scan='4' reason='ALL'"
        filename = 'listauto.txt'
        create_input(myinput, filename)
        
        # select only the autocorr reasons to flag
        flagdata(vis=self.vis, mode='list', inpfile=filename, reason='AUTO', action='apply',
                 flagbackup=False)
        s = flagdata(vis = self.vis, mode="summary", basecnt=True)
        self.assertEqual(s['scan']['4']['flagged'], 0)
        self.assertEqual(s['baseline']['VA09&&VA28']['flagged'], 0)
        self.assertEqual(s['baseline']['VA09&&VA09']['flagged'], 3528)
        
        # select only the third line scan=4
        flagdata(vis=self.vis, mode='list', inpfile=filename, reason='ALL', action='apply')
        s = flagdata(vis = self.vis, mode="summary", basecnt=True)
        self.assertEqual(s['scan']['4']['flagged'], 95256)
        self.assertEqual(s['baseline']['VA09&&VA28']['flagged'], 252)
        self.assertEqual(s['baseline']['VA09&&VA09']['flagged'], 3780)
        self.assertEqual(s['flagged'], 190386)
                        

class test_statistics_queries(test_base):

    def setUp(self):
        self.setUp_ngc5921()

    def test_CAS2021(self):
        '''flagdata: test antenna negation selection'''
        
        flagdata(vis=self.vis, antenna='!VA05', savepars=False) 
        s = flagdata(vis = self.vis, mode="summary",basecnt=True)['baseline']
        assert s['VA01&&VA05']['flagged'] == 0 
        assert s['VA02&&VA05']['flagged'] == 0
        assert s['VA03&&VA05']['flagged'] == 0
        assert s['VA04&&VA05']['flagged'] == 0
        assert s['VA05&&VA06']['flagged'] == 0
        assert s['VA05&&VA07']['flagged'] == 0
        assert s['VA05&&VA08']['flagged'] == 0
        assert s['VA05&&VA09']['flagged'] == 0
        assert s['VA05&&VA10']['flagged'] == 0
        assert s['VA05&&VA11']['flagged'] == 0
        assert s['VA05&&VA12']['flagged'] == 0
        assert s['VA05&&VA13']['flagged'] == 0
        assert s['VA05&&VA14']['flagged'] == 0
        assert s['VA05&&VA15']['flagged'] == 0
        assert s['VA05&&VA16']['flagged'] == 0
        assert s['VA05&&VA17']['flagged'] == 0
        assert s['VA05&&VA18']['flagged'] == 0
        assert s['VA05&&VA19']['flagged'] == 0
        assert s['VA05&&VA20']['flagged'] == 0
        assert s['VA05&&VA21']['flagged'] == 0
        assert s['VA05&&VA22']['flagged'] == 0
        assert s['VA05&&VA24']['flagged'] == 0
        assert s['VA05&&VA25']['flagged'] == 0
        assert s['VA05&&VA26']['flagged'] == 0
        assert s['VA05&&VA27']['flagged'] == 0
        assert s['VA05&&VA28']['flagged'] == 0
        assert s['VA05&&VA05']['flagged'] == 7560
        assert s['VA05&&VA05']['total'] == 7560


    def test_CAS2212(self):
        '''flagdata: Clipping scan selection, CAS-2212, CAS-3496'''
        # By default correlation='ABS_ALL'
        flagdata(vis=self.vis, mode='clip', scan="2", clipminmax = [0.2, 0.3], savepars=False) 
        s = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(s['flagged'], 85404)
        self.assertEqual(s['total'], 2854278)
        
        s = flagdata(vis=self.vis, mode='summary')['scan']
        
        # Make sure no other scan is clipped
        self.assertEqual(s['1']['flagged'], 0)
        self.assertEqual(s['3']['flagged'], 0)
        self.assertEqual(s['4']['flagged'], 0)
        self.assertEqual(s['5']['flagged'], 0)
        self.assertEqual(s['6']['flagged'], 0)
        self.assertEqual(s['7']['flagged'], 0)
        self.assertEqual(s['2']['flagged'], 85404)
          
    def test021(self):
        '''flagdata: Test of flagging statistics and queries'''
        
        flagdata(vis=self.vis, correlation='LL', savepars=False, flagbackup=False)
        flagdata(vis=self.vis, spw='0:17~19', savepars=False, flagbackup=False)
        flagdata(vis=self.vis, antenna='VA05&&VA09', savepars=False, flagbackup=False)
        flagdata(vis=self.vis, antenna='VA14', savepars=False, flagbackup=False)
        flagdata(vis=self.vis, field='1', savepars=False, flagbackup=False)
        s = flagdata(vis=self.vis, mode='summary', minrel=0.9, spwchan=True, basecnt=True)
        assert s['antenna'].keys() == ['VA14']
        assert 'VA05&&VA09' in s['baseline'].keys()
        assert set(s['spw:channel'].keys()) == set(['0:17', '0:18', '0:19'])
        assert s['correlation'].keys() == ['LL']  # LL
        assert s['field'].keys() == ['1445+09900002_0']
        assert set(s['scan'].keys()) == set(['2', '4', '5', '7']) # field 1
        s = flagdata(vis=self.vis, mode='summary', maxrel=0.8)
        assert set(s['field'].keys()) == set(['1331+30500002_0', 'N5921_2'])
        s = flagdata(vis=self.vis, mode='summary', minabs=400000)
        assert set(s['scan'].keys()) == set(['3', '6'])
        s = flagdata(vis=self.vis, mode='summary', minabs=400000, maxabs=450000)
        assert s['scan'].keys() == ['3']

    def test_chanavg0(self):
        print "Test of channel average"
        flagdata(vis=self.vis, mode='clip',channelavg=False, clipminmax=[30., 60.], correlation='ABS_RR',
                 savepars=False, flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 1414186)

    def test_chanavg1(self):
        flagdata(vis=self.vis, mode='clip',channelavg=True, clipminmax=[30., 60.], correlation='ABS_RR',
                 savepars=False, flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 1347822)

    def test_chanavg2(self):
        flagdata(vis=self.vis, mode='clip',channelavg=False, clipminmax=[30., 60.], spw='0:0~10', 
                 correlation='ABS_RR', savepars=False, flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 242053)

    def test_chanavg3(self):
        flagdata(vis=self.vis, mode='clip',channelavg=True, clipminmax=[30., 60.], spw='0:0~10',
                 correlation='ABS_RR', savepars=False, flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 231374)
               

#    def test8(self):
#        print "Test of mode = 'quack'"
#        print "parallel quack"
#        flagdata(vis=self.vis, mode='quack', quackinterval=[1.0, 5.0], antenna=['2', '3'], correlation='RR')
#        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 22365)
#
    def test9(self):
        '''flagdata: quack mode'''
        flagdata(vis=self.vis, mode='quack', quackmode='beg', quackinterval=1, savepars=False,
                 flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 329994)

    def test10(self):
        '''flagdata: quack mode'''
        flagdata(vis=self.vis, mode='quack', quackmode='endb', quackinterval=1, savepars=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 333396)

    def test11(self):
        '''flagdata: quack mode'''
        flagdata(vis=self.vis, mode='quack', quackmode='end', quackinterval=1, savepars=False,
                 flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 2520882)

    def test12(self):
        '''flagdata: quack mode'''
        flagdata(vis=self.vis, mode='quack', quackmode='tail', quackinterval=1, savepars=False,
                 flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 2524284)

    def test13(self):
        '''flagdata: quack mode, quackincrement'''
        flagdata(vis=self.vis, mode='quack', quackinterval=50, quackmode='endb', quackincrement=True,
                 savepars=False, flagbackup=False)
        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 571536)

        flagdata(vis=self.vis, mode='quack', quackinterval=20, quackmode='endb', quackincrement=True,
                 savepars=False, flagbackup=False)
        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 857304)
        
        flagdata(vis=self.vis, mode='quack', quackinterval=150, quackmode='endb', quackincrement=True,
                 savepars=False, flagbackup=False)
        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 1571724)
        
        flagdata(vis=self.vis, mode='quack', quackinterval=50, quackmode='endb', quackincrement=True,
                 savepars=False, flagbackup=False)
        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 1762236)
        flagdata(vis=self.vis, mode='unflag', savepars=False, flagbackup=False)



class test_selections(test_base):
    """Test various selections"""

    def setUp(self):
        self.setUp_ngc5921()

    def test_scan(self):
        '''flagdata: scan selection and manualflag compatibility'''
        flagdata(vis=self.vis, scan='3', mode='manualflag', savepars=False)
        res = flagdata(vis=self.vis, mode='summary', antenna='VA02')
        self.assertEqual(res['flagged'],52416)
                
    def test_antenna(self):
        '''flagdata: antenna selection'''
        flagdata(vis=self.vis, antenna='VA02', savepars=False,flagbackup=False)
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='VA02'), 196434, 196434)

    def test_spw(self):
        '''flagdata: spw selection'''
        flagdata(vis=self.vis, spw='0', savepars=False,flagbackup=False)
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)

    def test_spw_list(self):
        '''flagdata: spw selection in list mode''' 
        spwfile = 'spwflags.txt'
        if os.path.exists(spwfile):
            os.system('rm -rf '+spwfile)
                   
        flagdata(vis=self.vis, spw='0:1~10', savepars=True, outfile=spwfile, flagbackup=False)
        res0 = flagdata(vis=self.vis, mode='summary', spwchan=True)
        self.assertEqual(res0['flagged'], 453060, 'Only channels 1~10 should be flagged')
        
        # Unflag
        flagdata(vis=self.vis, mode='unflag', flagbackup=False)
        ures = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(ures['flagged'], 0)
        
        # Flag using the saved list
        flagdata(vis=self.vis, mode='list', inpfile=spwfile, action='apply',
                 flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary', spwchan=True)
        self.assertEqual(res0['flagged'], res['flagged'])        
                  
        # Only channels 1~10 should be flagged
        self.assertEqual(res['spw:channel']['0:0']['flagged'], 0)
        self.assertEqual(res['spw:channel']['0:1']['flagged'], 45306)
        self.assertEqual(res['spw:channel']['0:2']['flagged'], 45306)
        self.assertEqual(res['spw:channel']['0:3']['flagged'], 45306)
        self.assertEqual(res['spw:channel']['0:4']['flagged'], 45306)
        self.assertEqual(res['spw:channel']['0:5']['flagged'], 45306)
        self.assertEqual(res['spw:channel']['0:6']['flagged'], 45306)
        self.assertEqual(res['spw:channel']['0:7']['flagged'], 45306)
        self.assertEqual(res['spw:channel']['0:8']['flagged'], 45306)
        self.assertEqual(res['spw:channel']['0:9']['flagged'], 45306)
        self.assertEqual(res['spw:channel']['0:10']['flagged'], 45306)
        self.assertEqual(res['spw:channel']['0:11']['flagged'], 0)

    def test_correlation(self):
        flagdata(vis=self.vis, correlation='LL', savepars=False, flagbackup=False)
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 98217)
        test_eq(flagdata(vis=self.vis, mode='summary', correlation='RR'), 1427139, 0)
        flagdata(vis=self.vis, mode='unflag', savepars=False, flagbackup=False)
        flagdata(vis=self.vis, correlation='LL,RR', savepars=False, flagbackup=False)
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)
        
        flagdata(vis=self.vis, mode='unflag', savepars=False, flagbackup=False)
        flagdata(vis=self.vis, mode='clip', correlation='NORM_RR,LL', clipminmax=[0.,3.],
                 flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 204979)
#        flagdata(vis=self.vis, correlation='LL RR')
#        flagdata(vis=self.vis, correlation='LL ,, ,  ,RR')
#        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)

    def test_field(self):
        '''flagdata: field selection'''
        flagdata(vis=self.vis, field='0', savepars=False, flagbackup=False)
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 39186)

    def test_uvrange(self):
        '''flagdata: uvrange selection'''
        flagdata(vis=self.vis, uvrange='200~400m', savepars=False, flagbackup=False)
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='VA02'), 196434, 55944)

    def test_timerange(self):
        '''flagdata: timerange selection'''
        flagdata(vis=self.vis, timerange='09:50:00~10:20:00', savepars=False,
                 flagbackup=False)
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 6552)

    def test_array(self):
        '''flagdata: array selection'''
        flagdata(vis=self.vis, array='0', savepars=False, flagbackup=False)
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)
                
    def test_action(self):
        '''flagdata: action = calculate'''
        flagdata(vis=self.vis, antenna='2,3,4', action='calculate')
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 0, 'Nothing should be flagged when action=calculate')

    def test_missing_corr_product(self):
        '''CAS-4234: Keep going when one of the corr products is not available but others are present'''
        flagdata(vis=self.vis, correlation='LL,LR', savepars=False, flagbackup=False)
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 98217)
        test_eq(flagdata(vis=self.vis, mode='summary', correlation='RR'), 1427139, 0)
        flagdata(vis=self.vis, mode='unflag', savepars=False, flagbackup=False)
        flagdata(vis=self.vis, correlation='LL,RR,RL', savepars=False, flagbackup=False)
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)
        

class test_selections_alma(test_base):
    # Test various selections for alma data 

    def setUp(self):
        self.setUp_flagdatatest_alma()

    def test_scanitent(self):
        '''flagdata: scanintent selection'''
        # flag POINTING CALIBRATION scans 
        # (CALIBRATE_POINTING_.. from STATE table's OBS_MODE)
        flagdata(vis=self.vis, intent='CAL*POINT*', savepars=False)
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 377280, 26200)
        
    def test_wvr(self):
        '''flagdata: flag WVR correlation'''
        flagdata(vis=self.vis, correlation='I', savepars=False, flagbackup=False)
        test_eq(flagdata(vis=self.vis, mode='summary'),1154592, 22752)

    def test_abs_wvr(self):
        '''flagdata: clip ABS_WVR'''
        flagdata(vis=self.vis, mode='clip',clipminmax=[0,50], correlation='ABS_WVR', savepars=False,
                 flagbackup=False)
        test_eq(flagdata(vis=self.vis, mode='summary'),1154592, 22752)
        
    def test_abs_i(self):
        '''flagdata: clip ABS_I. Do not flag WVR'''
        flagdata(vis=self.vis, mode='clip', clipminmax=[0,50], correlation='ABS_I', savepars=False,
                 flagbackup=False)
        test_eq(flagdata(vis=self.vis, mode='summary'),1154592, 0)

    def test_abs_all(self):
        '''flagdata: clip ABS ALL. Do not flag WVR'''
        flagdata(vis=self.vis, mode='clip', clipminmax=[0,1], correlation='ABS ALL', savepars=False,
                 flagbackup=False)
        test_eq(flagdata(vis=self.vis, mode='summary'),1154592, 130736)
        test_eq(flagdata(vis=self.vis, mode='summary', correlation='I'),22752, 0)

    def test_spw(self):
        '''flagdata: flag various spw'''
        # Test that a white space in the spw parameter is taken correctly
        flagdata(vis=self.vis, mode='manual', spw='1,3, 4', savepars=False,
                 flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['spw']['0']['flagged'], 0, 'spw=0 should not be flagged')
        self.assertEqual(res['spw']['1']['flagged'], 552960, 'spw=1 should be fully flagged')
        self.assertEqual(res['spw']['4']['flagged'], 22752, 'spw=4 should not be flagged')
        self.assertEqual(res['spw']['4']['total'], 22752, 'spw=4 should not be flagged')

    # CAS-4682
#    def test_null_field_selection1(self):
#        '''flagdata: handle non-existing field in agent's parameters in list mode'''
#        myinput = ["field='Mars'",
#                 "field='BLA'",   # non-existing field
#                 "field='J1037-295'"]
#        
#        flagdata(vis=self.vis, mode='list', inpfile=myinput, handleMSexception=True)
#        res = flagdata(vis=self.vis, mode='summary')
#        # It should flag field 1 and 2
#        self.assertEqual(res['field']['J1037-295']['flagged'], 256560)
#        self.assertEqual(res['field']['J1058+015']['flagged'], 0)
#        self.assertEqual(res['field']['Mars']['flagged'], 96216)
#        self.assertEqual(res['flagged'], 256560+96216)
#                             
#    def test_null_field_selection2(self):
#        '''flagdata: do not handle non-existing field in agent's parameters in list mode'''
#        myinput = ["field='Mars'",
#                 "field='BLA'",   # non-existing field
#                 "field='J1037-295'"]
#        
#        flagdata(vis=self.vis, mode='list', inpfile=myinput, handleMSexception=False)
#        res = flagdata(vis=self.vis, mode='summary')
#        # It should flag nothing
#        self.assertEqual(res['field']['Mars']['flagged'], 0)
#        self.assertEqual(res['field']['J1058+015']['flagged'], 0)
#        self.assertEqual(res['field']['J1037-295']['flagged'], 0)
        
    def test_null_intent_selection1(self):
        '''flagdata: handle unknown scan intent in list mode'''
        
        myinput = ["intent='FOCUS",   # non-existing intent
                 "intent='CALIBRATE_POINTING_ON_SOURCE'", # scan=1
                 "intent='CALIBRATE_AMPLI_ON_SOURCE", # scan=2
                 "intent='CALIBRATE_AMPLI_ON_SOURC",
                 "intent='*DELAY*'"] # non-existing
       
        flagdata(vis=self.vis, mode='list', inpfile=myinput, flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['1']['flagged'], 80184)
        self.assertEqual(res['scan']['2']['flagged'], 96216)
        self.assertEqual(res['flagged'], 80184+96216)
 
#    def test_null_intent_selection2(self):
#        '''flagdata: do not handle unknown scan intent in list mode'''
#        
#        myinput = ["intent='FOCUS",   # non-existing intent
#                 "intent='CALIBRATE_POINTING_ON_SOURCE'", # scan=1
#                 "intent='CALIBRATE_AMPLI_ON_SOURCE", # scan=2
#                 "intent='CALIBRATE_AMPLI_ON_SOURC",
#                 "intent='*DELAY*'"] # non-existing
#       
#        flagdata(vis=self.vis, mode='list', inpfile=myinput, handleMSexception=False)
#        res = flagdata(vis=self.vis, mode='summary')
#        self.assertEqual(res['scan']['1']['flagged'], 0)
#        self.assertEqual(res['flagged'], 0)

#    def test_null_selections1(self):
#        '''flagdata: handle NULL MS selections in list mode'''
#        
#        myinput = ["intent='FOCUS",   # non-existing intent
#                 "scan='1'", # scan=1
#                 "field='J1037-295,ngc3256,Titan'", # field Titan doesn't exist
#                 "intent='CALIBRATE_AMPLI_ON_SOURCE'"] # scan=2
#               
##        flagdata(vis=self.vis, mode='list', inpfile=myinput, handleMSexception=True)
#        flagdata(vis=self.vis, mode='list', inpfile=myinput)
#        res = flagdata(vis=self.vis, mode='summary')
#        self.assertEqual(res['scan']['1']['flagged'], 80184)
#        self.assertEqual(res['scan']['2']['flagged'], 96216)
#        self.assertEqual(res['field']['J1037-295']['flagged'], 256560)
#        self.assertEqual(res['field']['ngc3256']['flagged'], 721632)
#        self.assertEqual(res['flagged'], 80184+256560+721632)

class test_selections2(test_base):
    '''Test other selections'''
    
    def setUp(self):
        self.setUp_multi()
        
    def test_observation1(self):
        '''flagdata: observation ID selections'''
        # string
        flagdata(vis=self.vis, observation='1', savepars=False, flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 28500)
        self.assertEqual(res['total'], 2882778)

        # integer
        flagdata(vis=self.vis, mode='unflag', savepars=False)
        flagdata(vis=self.vis, observation=1, savepars=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 28500)
        self.assertEqual(res['total'], 2882778)
        
    def test_observation2(self):
        '''flagdata: observation ID selections in list mode'''
        # creat input list
        myinput = "observation='0' mode='manual'"
        filename = 'obs2.txt'
        create_input(myinput, filename)
        
        flagdata(vis=self.vis, mode='list', inpfile=filename, savepars=False,
                 flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['observation']['0']['flagged'], 2854278.0)
        self.assertEqual(res['observation']['1']['flagged'], 0, 'Only observation 0 should be flagged')
        
                
class test_elevation(test_base):
    """Test of mode = 'elevation'"""
    def setUp(self):
        self.setUp_ngc5921()
        self.x55 = 666792    # data below 55 degrees, etc.
        self.x60 = 1428840
        self.x65 = 2854278
        self.all = 2854278

    def test_lower(self):
        flagdata(vis = self.vis, mode = 'elevation', savepars=False)
        
        test_eq(flagdata(vis=self.vis, mode='summary'), self.all, 0)

        flagdata(vis = self.vis, mode = 'elevation', lowerlimit = 50, savepars=False,
                 flagbackup=False)

        test_eq(flagdata(vis=self.vis, mode='summary'), self.all, 0)

        flagdata(vis = self.vis, mode = 'elevation', lowerlimit = 55, savepars=False,
                 flagbackup=False)

        test_eq(flagdata(vis=self.vis, mode='summary'), self.all, self.x55)

        flagdata(vis = self.vis, mode = 'elevation', lowerlimit = 60, savepars=False,
                 flagbackup=False)

        test_eq(flagdata(vis=self.vis, mode='summary'), self.all, self.x60)

        flagdata(vis = self.vis, mode = 'elevation', lowerlimit = 65, savepars=False,
                 flagbackup=False)

        test_eq(flagdata(vis=self.vis, mode='summary'), self.all, self.x65)

    def test_upper(self):
        flagdata(vis = self.vis, mode = 'elevation', upperlimit = 60, savepars=False,
                 flagbackup=False)

        test_eq(flagdata(vis=self.vis, mode='summary'), self.all, self.all - self.x60)


    def test_interval(self):
        flagdata(vis = self.vis,mode = 'elevation',lowerlimit = 55,upperlimit = 60,
                  savepars=False,flagbackup=False)

        test_eq(flagdata(vis=self.vis, mode='summary'), self.all, self.all - (self.x60 - self.x55))


class test_list_file(test_base):
    """Test of mode = 'list' using input file"""
    
    def setUp(self):
        self.setUp_ngc5921()

    def test_file1(self):
        '''flagdata: apply flags from a list and do not save'''
        # creat input list
        myinput = "scan=1~3 mode=manual\n"+"scan=5 mode=manualflag\n"\
                "#scan='4'"
        filename = 'list1.txt'
        create_input(myinput, filename)
        
        # apply and don't save to MS. Ignore comment line
        flagdata(vis=self.vis, mode='list', inpfile=filename, savepars=False, action='apply')
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['4']['flagged'], 0)
        self.assertEqual(res['flagged'], 1711206, 'Total flagged does not match')
        
    def test_file2(self):
        '''flagdata: only save parameters without running the tool'''
        # creat input list
        myinput = "scan=1~3 mode=manual\n"+"scan=5 mode=manual\n"
        filename = 'list2.txt'
        create_input(myinput, filename)

        # save to another file
        if os.path.exists("myflags.txt"):
            os.system('rm -rf myflags.txt')
            
        flagdata(vis=self.vis, mode='list', inpfile=filename, savepars=True, action='', outfile='myflags.txt')
        self.assertTrue(filecmp.cmp(filename, 'myflags.txt', 1), 'Files should be equal')
        
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 0, 'No flags should have been applied')
        
    def test_file3(self):
        '''flagdata: flag and save list to FLAG_CMD'''
        # creat input list
        myinput = "scan=1~3 mode=manual\n"+"scan=5 mode=manual\n"
        filename = 'list3.txt'
        create_input(myinput, filename)

        # Delete any rows from FLAG_CMD
        flagcmd(vis=self.vis, action='clear', clearall=True)
        
        # Flag from list and save to FLAG_CMD
        flagdata(vis=self.vis, mode='list', inpfile=filename, savepars=True,
                 flagbackup=False)
        
        # Verify
        if os.path.exists("myflags.txt"):
            os.system('rm -rf myflags.txt')
        flagcmd(vis=self.vis, action='list', savepars=True, outfile='myflags.txt', useapplied=True)
        self.assertTrue(filecmp.cmp(filename, 'myflags.txt', 1), 'Files should be equal')
    
        
    def test_file4(self):
        '''flagdata: save without running and apply in flagcmd'''
        # Delete any rows from FLAG_CMD
        flagcmd(vis=self.vis, action='clear', clearall=True)
        
        # Test that action='none' is also accepted
        flagdata(vis=self.vis, mode='quack', quackmode='tail', quackinterval=1, action='none', 
                 savepars=True, flagbackup=False)
        
        flagcmd(vis=self.vis, action='apply')
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 2524284)

    def test_file5(self):
        '''flagdata: clip zeros in mode=list and save reason to FLAG_CMD'''
        # get the correct data, by passing the previous setUp()
        self.setUp_data4tfcrop()
        
        # creat input list
        myinput = "mode='clip' clipzeros=true reason='CLIP_ZERO'"
        filename = 'list5.txt'
        create_input(myinput, filename)

        # Save to FLAG_CMD
        flagdata(vis=self.vis, mode='list', inpfile=filename, action='', savepars=True)
        
        # Run in flagcmd and select by reason
        flagcmd(vis=self.vis, action='apply', reason='CLIP_ZERO')
        
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 274944, 'Should clip only spw=8')

    def test_file6(self):
        '''flagdata: select by reason in list mode from a file'''
        # creat input list
        myinput = "mode='manual' scan='1' reason='SCAN_1'\n"\
                "mode='manual' scan='2'\n"\
                "scan='3' reason='SCAN_3'\n"\
                "scan='4' reason=''"
        filename = 'list6.txt'
        create_input(myinput, filename)
        
        # Select one reason
        flagdata(vis=self.vis, mode='list', inpfile=filename, reason='SCAN_3',
                 flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['3']['flagged'], 762048, 'Should flag only reason=SCAN_3')
        self.assertEqual(res['flagged'], 762048, 'Should flag only reason=SCAN_3')
        
        # Select list of reasons
        flagdata(vis=self.vis, mode='list', inpfile=filename, reason=['','SCAN_1'],
                 flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['4']['flagged'], 95256, 'Should flag reason=\'\'')
        self.assertEqual(res['scan']['1']['flagged'], 568134, 'Should flag reason=SCAN_1')
        
        # No reason selection
        flagdata(vis=self.vis, mode='unflag', flagbackup=False)
        flagdata(vis=self.vis, mode='list', inpfile=filename, flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['1']['flagged'], 568134)
        self.assertEqual(res['scan']['2']['flagged'], 238140)
        self.assertEqual(res['scan']['3']['flagged'], 762048)
        self.assertEqual(res['scan']['4']['flagged'], 95256)
        self.assertEqual(res['flagged'],568134+238140+762048+95256, 'Total flagged')

    def test_file_CAS4819(self):
        '''flagdata: CAS-4819, Flag commands from three files'''
        # creat first input file
        myinput = "scan='1'\n"\
                "scan='2'\n"\
                "scan='3'"
        filename1 = 'list7a.txt'
        create_input(myinput, filename1)
        
        # Create second input file
        myinput = "scan='5'\n"\
                "scan='6'\n"\
                "scan='7'"        
        filename2 = 'list7b.txt'
        create_input(myinput, filename2)
        
         # Create third input file
        myinput = "scan='4' mode='clip' clipminmax=[0,4]" 
        filename3 = 'list7c.txt'
        create_input(myinput, filename3)
       
        flagdata(vis=self.vis, mode='list', inpfile=[filename1,filename2,filename3],
                 flagbackup=False)
        
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

               
    def test_reason1(self):
        '''flagdata: add_reason to FLAG_CMD'''
        flagcmd(vis=self.vis, action='clear', clearall=True)
        flagdata(vis=self.vis, mode='manual', scan='1,3', savepars=True, cmdreason='SCAN_1_3',
                  action='')
        
        # Apply flag cmd
        flagcmd(vis=self.vis, action='apply', reason='SCAN_1_3')
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 1330182, 'Only scans 1 and 3 should be flagged')
        
    def test_reason2(self):
        '''flagdata: add_reason to text file'''
        flagdata(vis=self.vis, mode='clip', scan='4', clipminmax=[0, 5], savepars=True, 
                  cmdreason='CLIPSCAN4', outfile='reason2.txt', action='')

        flagdata(vis=self.vis, mode='clip', scan='2~3', clipminmax=[ 0, 5], savepars=True, 
                  cmdreason='CLIPSCAN2_3', outfile='reason2.txt', action='')

        # Apply flag cmd
        flagdata(vis=self.vis, mode='list', inpfile='reason2.txt',reason='CLIPSCAN2_3',
                 flagbackup=False)
        
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 69568)
        
    def test_reason3(self):
        '''flagdata: replace input reason from file with cmdreason'''
        # creat input list
        myinput = "mode='manual' scan='1' reason='SCAN_1'\n"\
                "mode='manual' scan='2'\n"\
                "scan='3' reason='SCAN_3'\n"\
                "scan='4' reason=''"
        filename = 'input3.txt'
        create_input(myinput, filename)
        
        flagdata(vis=self.vis, mode='list', inpfile=filename, savepars=True, outfile='reason3a.txt',
                  cmdreason='MANUALFLAG', action='')
        
        # Apply the flag cmds
        flagdata(vis=self.vis, mode='list', inpfile='reason3a.txt', reason='MANUALFLAG',
                 flagbackup=False)
        
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 1663578)
        
    def test_reason4(self):
        '''flagdata: select by reason from two files'''
        # creat first input file
        myinput = "scan='1' spw='0:10~20' reason='NONE'\n"\
                "scan='2' reason='EVEN'\n"\
                "scan='3' reason='ODD'"
        filename1 = 'reasonfile1.txt'
        create_input(myinput, filename1)
        
        # Create second input file
        myinput = "scan='5' reason='ODD'\n"\
                "scan='6' reason='EVEN'\n"\
                "scan='7' reason='ODD'"        
        filename2 = 'reasonfile2.txt'
        create_input(myinput, filename2)
        
        # Apply flag cmds on ODD reason
        flagdata(vis=self.vis, mode='list', inpfile=[filename1,filename2], reason='ODD',
                 flagbackup=False)
        
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['3']['flagged'], 762048)
        self.assertEqual(res['scan']['5']['flagged'], 142884)
        self.assertEqual(res['scan']['6']['flagged'], 0)
        self.assertEqual(res['scan']['7']['flagged'], 190512)
        self.assertEqual(res['flagged'], 762048+142884+190512)
        
        # Apply flag cmds on NONE reason
        flagdata(vis=self.vis, mode='unflag')
        flagdata(vis=self.vis, mode='list', inpfile=[filename1,filename2], reason='NONE',
                 flagbackup=False)
        
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['1']['flagged'], 99198)
        self.assertEqual(res['flagged'], 99198)


class test_list_list(test_base):
    """Test of mode = 'list' using input list"""
    
    def setUp(self):
        self.setUp_ngc5921()

    def test_list1(self):
        '''flagdata: apply flags from a Python list and do not save'''
        # creat input list
        myinput = ["scan='1~3' mode='manual'",
                 "scan='5' mode='manualflag'",
                 "#scan='4'"]
        
        # apply and don't save to MS. Ignore comment line
        flagdata(vis=self.vis, mode='list', inpfile=myinput, savepars=False, action='apply')
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['4']['flagged'], 0)
        self.assertEqual(res['flagged'], 1711206, 'Total flagged does not match')
        
    def test_list2(self):
        '''flagdata: only save parameters without running the tool'''
        # creat input list
        myinput = ["scan='1~3' mode='manual'",
                 "scan='5' mode='manual'"]

        # save to another file
        if os.path.exists("myflags.txt"):
            os.system('rm -rf myflags.txt')
            
        flagdata(vis=self.vis, mode='list', inpfile=myinput, savepars=True, action='', outfile='myflags.txt')
        
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 0, 'No flags should have been applied')
        
    def test_list3(self):
        '''flagdata: Compare flags from flagdata and flagcmd'''
        # creat input list
        myinput = ["scan='1~3' mode='manual'",
                 "scan='5' mode='manual'"]
 
        # Delete any rows from FLAG_CMD
        flagcmd(vis=self.vis, action='clear', clearall=True)
        
        # Flag from list and save to FLAG_CMD
        flagdata(vis=self.vis, mode='list', inpfile=myinput,flagbackup=False)
        res1 = flagdata(vis=self.vis, mode='summary')
        
        # Unflag and save in flagcmd using the cmd mode
        flagdata(vis=self.vis, mode='unflag',flagbackup=False)
        flagcmd(vis=self.vis, inpmode='list', inpfile=myinput)
        res2 = flagdata(vis=self.vis, mode='summary')

        # Verify
        self.assertEqual(res1['flagged'], res2['flagged'])
        self.assertEqual(res1['total'], res2['total'])

    def test_list4(self):
        '''flagdata: clip zeros in mode=list and save reason to FLAG_CMD'''
        # get the correct data, by passing the previous setUp()
        self.setUp_data4tfcrop()
        
        # creat input list
        myinput = ["mode='clip' clipzeros=true reason='CLIP_ZERO'"]

        # Save to FLAG_CMD
        flagdata(vis=self.vis, mode='list', inpfile=myinput, action='', savepars=True)
        
        # Run in flagcmd and select by reason
        flagcmd(vis=self.vis, action='apply', reason='CLIP_ZERO')
        
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 274944, 'Should clip only spw=8')

    def test_list5(self):
        '''flagdata: select by reason in list mode from a list'''
        # creat input list
        myinput = ["mode='manual' scan='1' reason='SCAN_1'",
                "mode='manual' scan='2'",
                "scan='3' reason='SCAN_3'",
                "scan='4' reason=''"]
        
        # Select one reason
        flagdata(vis=self.vis, mode='list', inpfile=myinput, reason='SCAN_3',
                 flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['3']['flagged'], 762048, 'Should flag only reason=SCAN_3')
        self.assertEqual(res['flagged'], 762048, 'Should flag only reason=SCAN_3')
        
        # Select list of reasons
        flagdata(vis=self.vis, mode='list', inpfile=myinput, reason=['','SCAN_1'],
                 flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['4']['flagged'], 95256, 'Should flag reason=\'\'')
        self.assertEqual(res['scan']['1']['flagged'], 568134, 'Should flag reason=SCAN_1')
        
        # No reason selection
        flagdata(vis=self.vis, mode='unflag',flagbackup=False)
        flagdata(vis=self.vis, mode='list', inpfile=myinput, flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['1']['flagged'], 568134)
        self.assertEqual(res['scan']['2']['flagged'], 238140)
        self.assertEqual(res['scan']['3']['flagged'], 762048)
        self.assertEqual(res['scan']['4']['flagged'], 95256)
        self.assertEqual(res['flagged'],568134+238140+762048+95256, 'Total flagged')        
                
    def test_reason_list(self):
        '''flagdata: replace input reason from list with cmdreason'''
        # creat input list
        myinput = ["mode='manual' scan='1' reason='SCAN_1'",
                "mode='manual' scan='2'",
                "scan='3' reason='SCAN_3'",
                "scan='4' reason=''"]
        
        flagdata(vis=self.vis, mode='list', inpfile=myinput, savepars=True, outfile='reason3b.txt',
                  cmdreason='MANUALFLAG', action='')
        
        # Apply the flag cmds
        flagdata(vis=self.vis, mode='list', inpfile='reason3b.txt', reason='MANUALFLAG',
                 flagbackup=False)
        
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 1663578)
              
    # CAS-4974  
    def test_cmdreason1(self):
        '''flagdata: detect blank space in cmdreason. Catch exception'''
        outtxt = 'badreason.txt'
        flagdata(vis=self.vis, scan='1,3', action='calculate', savepars=True, outfile=outtxt, 
                 cmdreason='ODD SCANS')
        flagdata(vis=self.vis, scan='2,4', action='calculate', savepars=True, outfile=outtxt, 
                 cmdreason='EVEN SCANS')
        
        # Apply the cmd with blanks in reason. Catch the error.
        try:
            flagdata(vis=self.vis, mode='list', inpfile=outtxt, reason='ODD SCANS')
        except Exception, instance:
            print '*** Expected exception. \"%s\"'%instance
            res = flagdata(vis=self.vis, mode='summary')
            self.assertEqual(res['flagged'], 0)
    
        # Now apply the corrected version with underscores
        flagdata(vis=self.vis, mode='list', inpfile=outtxt, reason='ODD_SCANS')
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['1']['flagged'], 568134)
        self.assertEqual(res['scan']['3']['flagged'], 762048)
        self.assertEqual(res['flagged'], 568134+762048)

    
    def test_cmdreason2(self):
        '''flagdata: Blanks in reason are only allowed in FLAG_CMD table'''
        outtxt = 'goodreason.txt'
        flagdata(vis=self.vis, scan='1,3', action='calculate', savepars=True, 
                 cmdreason='ODD SCANS')
        flagdata(vis=self.vis, scan='2,4', action='calculate', savepars=True,
                 cmdreason='EVEN SCANS')
        
        flagcmd(vis=self.vis, reason='ODD SCANS')
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['1']['flagged'], 568134)
        self.assertEqual(res['scan']['3']['flagged'], 762048)
        self.assertEqual(res['flagged'], 568134+762048)
    
class test_clip(test_base):
    """flagdata:: Test of mode = 'clip'"""
    
    def setUp(self):
        self.setUp_data4tfcrop()
        
    def test_clipzeros(self):
    	'''flagdata: clip only zero-value data'''
        flagdata(vis=self.vis, mode='clip', clipzeros=True, flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'],274944,'Should clip only spw=8')
        

class test_CASA_4_0_bug_fix(test_base):
    """flagdata:: Regression test for the fixes introduced during the CASA 4.0 bug fix season"""

    def setUp(self):
        self.setUp_data4tfcrop()
        
    def test_CAS_4270(self):
        """flagdata: Test uvrange given in lambda units"""
                
        flagdata(vis=self.vis,mode='manual',uvrange='<2klambda')
        flagdata(vis=self.vis,mode='clip', flagbackup=False)
        summary_ref = flagdata(vis=self.vis,mode='summary')
        
        flagdata(vis=self.vis,mode='unflag', flagbackup=False)
        flagdata(vis=self.vis,mode='list',inpfile=["uvrange='<2Klambda'","mode='clip'"],
                 flagbackup=False)
        summary_out = flagdata(vis=self.vis,mode='summary')
        
        self.assertEqual(summary_out['flagged'],summary_ref['flagged'],'uvrange given in lambda is not properly translated into meters')
        
    def test_CAS_4312(self):
        """flagdata: Test channel selection with Rflag agent"""
        
        flagdata(vis=self.vis,mode='rflag',spw='9:10~20')
        summary = flagdata(vis=self.vis,mode='summary')
        self.assertEqual(summary['spw']['8']['flagged'],0,'Error in channel selection with Rflag agent')
        self.assertEqual(summary['spw']['9']['flagged'],1861,'Error in channel selection with Rflag agent')
        self.assertEqual(summary['spw']['10']['flagged'],0,'Error in channel selection with Rflag agent')
        
        
    def test_CAS_4200(self):
        """flagdata: Test quack mode with quackinterval 0"""
        
        flagdata(vis=self.vis,mode='quack',quackinterval=0, flagbackup=False)
        summary_zero = flagdata(vis=self.vis,mode='summary')
        self.assertEqual(summary_zero['flagged'],0,'Error in quack mode with quack interval 0')
        
        flagdata(vis=self.vis,mode='quack',quackinterval=1, flagbackup=False)
        summary_one = flagdata(vis=self.vis,mode='summary')
        
        flagdata(vis=self.vis,mode='unflag', flagbackup=False)
        flagdata(vis=self.vis,mode='quack', flagbackup=False)
        summary_default = flagdata(vis=self.vis,mode='summary')
        
        self.assertEqual(summary_one['flagged'],summary_default['flagged'],'Error in quack mode with quack interval 1')
        
    def test_alias(self):
        '''flagdata: Test tflagdata alias'''
        res = tflagdata(vis=self.vis, mode='summary')['flagged']
        self.assertEqual(res, 0)
        
    def test_spw_freq1(self):
        '''flagdata: CAS-3562, flag all spw channels greater than a frequency'''
        flagdata(vis=self.vis, spw='>2000MHz', flagbackup=False)
        
        # Flag only spw=6,7
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['spw']['0']['flagged'], 0)
        self.assertEqual(res['spw']['10']['flagged'], 0)
        self.assertEqual(res['spw']['7']['flagged'], 274944)
        self.assertEqual(res['spw']['7']['total'], 274944)
        self.assertEqual(res['spw']['6']['flagged'], 274944)
        self.assertEqual(res['spw']['6']['total'], 274944)
        self.assertEqual(res['flagged'], 549888)

    def test_spw_freq2(self):
        '''flagdata: CAS-3562, flag the channel with a frequency'''
        flagdata(vis=self.vis, spw='*:1956MHz,*:945MHz', flagbackup=False)
        
         # Flag only spw=5,8, first channel (0)
        res = flagdata(vis=self.vis, mode='summary', spwchan=True)
        self.assertEqual(res['spw:channel']['1:0']['flagged'], 0)
        self.assertEqual(res['spw:channel']['15:0']['flagged'], 0)
        self.assertEqual(res['spw:channel']['5:0']['flagged'], 4296)
        self.assertEqual(res['spw:channel']['5:0']['total'], 4296)
        self.assertEqual(res['spw:channel']['8:0']['flagged'], 4296)
        self.assertEqual(res['spw:channel']['8:0']['total'], 4296)
        self.assertEqual(res['flagged'], 8592)

    def test_spw_freq3(self):
        '''flagdata: CAS-3562, flag a range of frequencies'''
        flagdata(vis=self.vis, spw='1500 ~ 2000MHz', flagbackup=False)
        
        # Flag only spw=0~5 
        res = flagdata(vis=self.vis, mode='summary', spwchan=True)
        self.assertEqual(res['spw']['0']['flagged'], 274944)
        self.assertEqual(res['spw']['1']['flagged'], 274944)
        self.assertEqual(res['spw']['2']['flagged'], 274944)
        self.assertEqual(res['spw']['3']['flagged'], 274944)
        self.assertEqual(res['spw']['4']['flagged'], 274944)
        self.assertEqual(res['spw']['5']['flagged'], 274944)
        self.assertEqual(res['spw']['6']['flagged'], 0)
        self.assertEqual(res['flagged'], 1649664)

class test_correlations(test_base):
    '''Test combinations of correlation products'''
    def setUp(self):
        self.setUp_mwa()
        
    def tearDown(self):
        shutil.rmtree(self.vis, ignore_errors=True)
        
    def test_xx_xy(self):
        '''flagdata: flag XX,XY'''
        flagdata(vis=self.vis, mode='manual', flagbackup=False, correlation='XX,XY')
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['correlation']['XX']['flagged'], 429792)
        self.assertEqual(res['correlation']['XY']['flagged'], 429792)
        self.assertEqual(res['correlation']['YY']['flagged'], 0)
        self.assertEqual(res['correlation']['YX']['flagged'], 0)

    def test_xx_yx(self):
        '''flagdata: flag XX,YX'''
        flagdata(vis=self.vis, mode='manual', flagbackup=False, correlation='XX,YX')
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['correlation']['XX']['flagged'], 429792)
        self.assertEqual(res['correlation']['XY']['flagged'], 0)
        self.assertEqual(res['correlation']['YY']['flagged'], 0)
        self.assertEqual(res['correlation']['YX']['flagged'], 429792)
        
    def test_xx_yx_xy(self):
        '''flagdata: flag XX,YX, XY with space'''
        flagdata(vis=self.vis, mode='manual', flagbackup=False, correlation='XX,YX, XY')
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['correlation']['XX']['flagged'], 429792)
        self.assertEqual(res['correlation']['XY']['flagged'], 429792)
        self.assertEqual(res['correlation']['YY']['flagged'], 0)
        self.assertEqual(res['correlation']['YX']['flagged'], 429792)
        
    def test_yy_yx(self):
        '''flagdata: flag YY,YX'''
        flagdata(vis=self.vis, mode='manual', flagbackup=False, correlation=' YY,YX')
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['correlation']['XX']['flagged'], 0)
        self.assertEqual(res['correlation']['XY']['flagged'], 0)
        self.assertEqual(res['correlation']['YY']['flagged'], 429792)
        self.assertEqual(res['correlation']['YX']['flagged'], 429792)
        
        
class test_tsys(test_base):
    """Flagdata:: Flagging of Tsys-based CalTable """
    
    def setUp(self):
         self.setUp_tsys_case()
         
    def test_unsupported_elevation(self):
        '''Flagdata: Unsupported elevation mode'''
        res = flagdata(vis=self.vis, mode='elevation')
        self.assertEqual(res, {})

    def test_unsupported_shadow(self):
        '''Flagdata: Unsupported shadow mode'''
        res = flagdata(vis=self.vis, mode='shadow', flagbackup=False)
        self.assertEqual(res, {})
        
    def test_mixed_list(self):
        '''Flagdata: mixed supported and unsupported modes in a list'''
        cmds = ["spw='1'",
                "spw='3' mode='elevation'",
                "mode='shadow'",
                "spw='5'"]
        
        flagdata(vis=self.vis, mode='list', inpfile=cmds, flagbackup=False)
        res = flagdata(vis=self.vis,mode='summary',spw='1,3,5')
        self.assertEqual(res['spw']['1']['flagged'], 32256)
        self.assertEqual(res['spw']['3']['flagged'], 0)
        self.assertEqual(res['spw']['5']['flagged'], 32256)
        self.assertEqual(res['flagged'], 32256*2)
        
    def test_invalid_scan(self):
        '''Flagdata: unsupported scan selection'''
        try:
            flagdata(vis=self.vis, scan='2', flagbackup=False)
        except exceptions.RuntimeError, instance:
            print 'Expected error: %s'%instance

    def test_default_fparam(self):
        '''Flagdata: default data column FPARAM'''
        flagdata(vis=self.vis, mode='clip', clipminmax=[0,500], flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 5325)
        
    def test_manual_field_selection(self):
        """Flagdata:: Manually flag a Tsys-based CalTable using field selection"""

        flagdata(vis=self.vis, field='0', flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        
        self.assertEqual(res['field']['3c279']['flagged'], 9216)
        self.assertEqual(res['field']['Titan']['flagged'], 0)
        self.assertEqual(res['field']['TW Hya']['flagged'], 0)
        self.assertEqual(res['field']['J1037-295=QSO']['flagged'], 0)

    def test_manual_antenna_selection(self):
        """Flagdata:: Manually flag a Tsys-based CalTable using antenna selection"""

        flagdata(vis=self.vis, antenna='DV09', flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['antenna']['DV09']['flagged'], 14336)
        self.assertEqual(res['antenna']['DV10']['flagged'], 0)
               
    def test_clip_fparm_sol1(self):
        """Flagdata:: Test clipping first calibration solution product of FPARAM 
        column using a minmax range """

        flagdata(vis=self.vis, mode='clip', datacolumn='FPARAM', correlation='REAL_Sol1',
                 clipzeros=True, clipminmax=[0.,600.], flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        
        self.assertEqual(res['total'], 129024.0)
        self.assertEqual(res['flagged'], 750.0)
        self.assertEqual(res['correlation']['Sol1']['flagged'], 750.0)
        self.assertEqual(res['correlation']['Sol1']['total'], 64512.0)
        self.assertEqual(res['correlation']['Sol2']['flagged'], 0.0)
        self.assertEqual(res['correlation']['Sol2']['total'], 64512.0)
        
    def test_list_fparm_sol1_extension(self):
        """Flagdata:: Test list mode to clip first calibration solution product of FPARAM 
        column using a minmax range, and then extend to the other solution """

        cmds = ["mode='clip' datacolumn='FPARAM' correlation='Sol1' "
                "clipzeros=True clipminmax=[0.,600.]",
                "mode='extend' extendpols=True growfreq=0.0 growtime=0.0"]
        flagdata(vis=self.vis, mode='list', inpfile=cmds)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['total'], 129024)
        self.assertEqual(res['flagged'], 1500)
        self.assertEqual(res['correlation']['Sol1']['flagged'], 750)
        self.assertEqual(res['correlation']['Sol2']['flagged'], 750)

        # Get the same results when flagging using a file
        flagdata(vis=self.vis, mode='unflag', flagbackup=False)
        myinput = "mode='clip' datacolumn='FPARAM' correlation='Sol1' clipzeros=True clipminmax=[0.,600.]\n"\
                "mode='extend' extendpols=True growfreq=0.0 growtime=0.0"
        filename = 'callist.txt'
        create_input(myinput, filename)
        flagdata(vis=self.vis, mode='list', inpfile=filename, flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['total'], 129024)
        self.assertEqual(res['flagged'], 1500)
        self.assertEqual(res['correlation']['Sol1']['flagged'], 750)
        self.assertEqual(res['correlation']['Sol2']['flagged'], 750)

    def test_clip_fparm_sol2(self):
        """Flagdata:: Test cliping second calibration solution product of FPARAM 
        column using a minmax range """

        flagdata(vis=self.vis, mode='clip', datacolumn='FPARAM', correlation='Sol2',
                 clipzeros=True, clipminmax=[0.,600.], flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        
        self.assertEqual(res['total'], 129024.0)
        self.assertEqual(res['flagged'], 442.0)
        self.assertEqual(res['correlation']['Sol1']['flagged'], 0.0)
        self.assertEqual(res['correlation']['Sol1']['total'], 64512.0)
        self.assertEqual(res['correlation']['Sol2']['flagged'], 442.0)
        self.assertEqual(res['correlation']['Sol2']['total'], 64512.0)
        
    def test_clip_fparm_sol1sol2(self):
        """Flagdata:: Test cliping first and second calibration solution products of 
        FPARAM column using a minmax range """

        flagdata(vis=self.vis, mode='clip', datacolumn='FPARAM', correlation='Sol1,Sol2',
                 clipzeros=True, clipminmax=[0.,600.], flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')

        self.assertEqual(res['total'], 129024.0)
        self.assertEqual(res['flagged'], 1192.0)
        self.assertEqual(res['correlation']['Sol1']['flagged'], 750.0)
        self.assertEqual(res['correlation']['Sol1']['total'], 64512.0)
        self.assertEqual(res['correlation']['Sol2']['flagged'], 442.0)
        self.assertEqual(res['correlation']['Sol2']['total'], 64512.0)
                
    def test_invalid_corr(self):
        '''Flagdata: default correlation should be REAL_ALL in this case'''
        flagdata(vis=self.vis, mode='clip', correlation='ABS_ALL',clipminmax=[0.,600.],
                 flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 1192.0)
        
    def test_invalid_datacol(self):
        '''Flagdata: invalid data column should fall back to default'''
        flagdata(vis=self.vis, mode='clip', clipminmax=[0.,600.],datacolumn='PARAMERR',
                 flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 1192.0)
                
    def test_clip_fparm_all(self):
        """Flagdata:: Test cliping all calibration solution products of FPARAM 
        column using a minmax range """

        flagdata(vis=self.vis, mode='clip', datacolumn='FPARAM', correlation='',
                 clipzeros=True, clipminmax=[0.,600.], flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')       
        self.assertEqual(res['total'], 129024.0)
        self.assertEqual(res['flagged'], 1192.0)
        self.assertEqual(res['correlation']['Sol1']['flagged'], 750.0)
        self.assertEqual(res['correlation']['Sol1']['total'], 64512.0)
        self.assertEqual(res['correlation']['Sol2']['flagged'], 442.0)
        self.assertEqual(res['correlation']['Sol2']['total'], 64512.0)

    def test_clip_fparm_all(self):
        """Flagdata:: Test cliping only zeros in all calibration solution 
        products of FPARAM column"""

        flagdata(vis=self.vis, mode='clip', datacolumn='FPARAM', correlation='',
                 clipzeros=True, flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
               
        self.assertEqual(res['total'], 129024.0)
        self.assertEqual(res['flagged'], 126.0)
        self.assertEqual(res['correlation']['Sol1']['flagged'], 56.0)
        self.assertEqual(res['correlation']['Sol1']['total'], 64512.0)
        self.assertEqual(res['correlation']['Sol2']['flagged'], 70.0)
        self.assertEqual(res['correlation']['Sol2']['total'], 64512.0)

    def test_clip_nans_fparm_all(self):
        """Flagdata:: Test cliping only NaNs/Infs in all calibration solution products of FPARAM column"""

        flagdata(vis=self.vis, mode='clip', datacolumn='FPARAM', correlation='',
                 flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        
        self.assertEqual(res['total'], 129024.0)
        self.assertEqual(res['flagged'], 0.0)
        self.assertEqual(res['correlation']['Sol1']['flagged'], 0.0)
        self.assertEqual(res['correlation']['Sol1']['total'], 64512.0)
        self.assertEqual(res['correlation']['Sol2']['flagged'], 0.0)
        self.assertEqual(res['correlation']['Sol2']['total'], 64512.0)

    def test_clip_fparm_error_absall(self):
        """Flagdata:: Error case test when a complex operator is used with CalTables """

        flagdata(vis=self.vis, mode='clip', datacolumn='FPARAM', correlation='ABS_ALL',
                 clipzeros=True, clipminmax=[0.,600.], flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')

        self.assertEqual(res['total'], 129024.0)
        self.assertEqual(res['flagged'], 1192.0)
        self.assertEqual(res['correlation']['Sol1']['flagged'], 750.0)
        self.assertEqual(res['correlation']['Sol1']['total'], 64512.0)
        self.assertEqual(res['correlation']['Sol2']['flagged'], 442.0)
        self.assertEqual(res['correlation']['Sol2']['total'], 64512.0)

    def test_clip_fparm_error_abs1(self):
        """Flagdata:: Error case test when a complex operator is used with CalTables """

        flagdata(vis=self.vis, mode='clip', datacolumn='FPARAM', correlation='ABS_Sol1',
                 clipzeros=True, clipminmax=[0.,600.], flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')

        self.assertEqual(res['total'], 129024)
        self.assertEqual(res['flagged'], 750)
        self.assertEqual(res['correlation']['Sol1']['flagged'], 750.0)
        self.assertEqual(res['correlation']['Sol2']['flagged'], 0)

    def test_clip_fparm_error_abs12(self):
        """Flagdata:: Fall back to default REAL operator """

        flagdata(vis=self.vis, mode='clip', datacolumn='FPARAM', correlation='ABS Sol1,Sol2',
                 clipzeros=True, clipminmax=[0.,600.], flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')

        self.assertEqual(res['total'], 129024)
        self.assertEqual(res['flagged'], 1192)
        self.assertEqual(res['correlation']['Sol1']['flagged'], 750.0)
        self.assertEqual(res['correlation']['Sol2']['flagged'], 442)

    def test_clip_snr_all(self):
        """Flagdata:: Test cliping all calibration solution products of SNR
        column using a minmax range for Tsys CalTable"""

        flagdata(vis=self.vis, mode='clip', datacolumn='SNR', correlation='',
                 clipzeros=True, clipminmax=[0.,2.], flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')

        self.assertEqual(res['total'], 129024.0)
        self.assertEqual(res['flagged'], 0.0)
        self.assertEqual(res['correlation']['Sol1']['flagged'], 0.0)
        self.assertEqual(res['correlation']['Sol1']['total'], 64512.0)
        self.assertEqual(res['correlation']['Sol2']['flagged'], 0.0)
        self.assertEqual(res['correlation']['Sol2']['total'], 64512.0)

    def test_spw_selection(self):
        '''Flagdata: Select spw in cal tables'''
        flagdata(vis=self.vis, mode='manual', spw='1,3', flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['spw']['1']['flagged'],32256)
        self.assertEqual(res['spw']['3']['flagged'],32256)
        self.assertEqual(res['flagged'],32256+32256)

    def test_channel_selection(self):
        '''Flagdata: Select spw:channel in cal tables'''
        flagdata(vis=self.vis, mode='manual', spw='*:0~8,*:120~127', flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary', spwchan=True)
        self.assertEqual(res['spw']['1']['flagged'],4284)
        self.assertEqual(res['spw']['3']['flagged'],4284)
        self.assertEqual(res['spw:channel']['5:0']['flagged'],252)
        self.assertEqual(res['spw:channel']['5:0']['total'],252)
        self.assertEqual(res['spw:channel']['5:9']['flagged'],0)
        self.assertEqual(res['spw:channel']['5:119']['flagged'],0)
        self.assertEqual(res['spw:channel']['5:120']['flagged'],252)
        self.assertEqual(res['spw:channel']['5:127']['flagged'],252)
        self.assertEqual(res['flagged'],4284*4)

    def test_tsys_scan1(self):
        '''Flagdata: select valid scans'''
        flagdata(vis=self.vis, mode='manual', scan='1,10,14,30', flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['1']['flagged'],9216)
        self.assertEqual(res['scan']['3']['flagged'],0)
        self.assertEqual(res['scan']['10']['flagged'],9216)
        self.assertEqual(res['scan']['14']['flagged'],9216)
        self.assertEqual(res['scan']['30']['flagged'],9216)
        self.assertEqual(res['flagged'],9216*4)
        
    def test_tsys_scan2(self):
        '''Flagdata: select valid and invalid scans'''
        # scan=2 does not exist. It should not raise an error
        flagdata(vis=self.vis, mode='manual', scan='1~3', flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['1']['flagged'],9216)
        self.assertEqual(res['scan']['3']['flagged'],9216)
        self.assertEqual(res['flagged'],9216*2)

    def test_tsys_time1(self):
        '''Flagdata: select a timerange'''
        flagdata(vis=self.vis, mode='clip', clipminmax=[-2000.,2000.], timerange="<03:50:00", 
                 flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')['flagged']
        self.assertEqual(res, 5)

    def test_tsys_time2(self):
        '''Flagdata: select a timerange for one spw'''
        # timerange=03:50:00~04:10:00 covers scans 14 17 only
        flagdata(vis=self.vis, mode='manual', timerange="03:50:00~04:10:00",
                 flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['14']['flagged'],9216)
        self.assertEqual(res['scan']['17']['flagged'],9216)
        self.assertEqual(res['spw']['1']['flagged'],4608)
        self.assertEqual(res['spw']['3']['flagged'],4608)
        self.assertEqual(res['flagged'],18432)
        
        # Run for one spw only
        flagdata(vis=self.vis, mode='unflag', flagbackup=False)
        flagdata(vis=self.vis, mode='manual', timerange="03:50:00~04:10:00", spw='1',
                 flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['14']['flagged'],2304)
        self.assertEqual(res['scan']['17']['flagged'],2304)
        self.assertEqual(res['spw']['1']['flagged'],4608)
        self.assertEqual(res['spw']['3']['flagged'],0)
        self.assertEqual(res['flagged'],4608)
         
        # Now check that the same is flagged using scan selection
        flagdata(vis=self.vis, mode='unflag', flagbackup=False)
        flagdata(vis=self.vis, mode='manual', scan='14,17', spw='1',
                 flagbackup=False)
        res1=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res1['scan']['14']['flagged'],2304)
        self.assertEqual(res1['scan']['17']['flagged'],2304)
        self.assertEqual(res1['spw']['1']['flagged'],4608)
        self.assertEqual(res1['spw']['3']['flagged'],0)
        self.assertEqual(res1['flagged'],4608)
        self.assertEqual(res1['flagged'], res['flagged'])
               
class test_bandpass(test_base):
    """Flagdata:: Test flagging task with Bpass-based CalTable """
    
    def setUp(self):
        self.setUp_bpass_case()

    def test_unsupported_modes(self):
        '''Flagdata: elevation and shadow are not supported in cal tables'''
        res = flagdata(vis=self.vis, mode='elevation', flagbackup=False)
        self.assertEqual(res, {})
        res = flagdata(vis=self.vis, mode='shadow', flagbackup=False)
        self.assertEqual(res, {})

    def test_nullselections(self):
        '''Flagdata: unkonwn scan selection in cal tables'''
        try:
            flagdata(vis=self.vis, scan='1', flagbackup=False)
        except exceptions.RuntimeError, instance:
            print 'Expected error: %s'%instance

    def test_default_cparam(self):
        '''Flagdata: flag CPARAM as the default column'''
        flagdata(vis=self.vis, mode='clip', clipzeros=True, flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 11078.0, 'Should use CPARAM as the default column')

    def test_invalid_datacol(self):
        '''Flagdata: invalid data column should fall back to default'''
        flagdata(vis=self.vis, mode='clip', clipzeros=True, datacolumn='PARAMERR',
                 flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 11078.0)
                        
    def test_manual_field_selection_for_bpass(self):
        """Flagdata:: Manually flag a bpass-based CalTable using field selection"""
        
        flagdata(vis=self.vis, field='3C286_A', flagbackup=False)
        summary=flagdata(vis=self.vis, mode='summary')
        
        self.assertEqual(summary['field']['3C286_A']['flagged'], 499200.0)
        self.assertEqual(summary['field']['3C286_B']['flagged'], 0)
        self.assertEqual(summary['field']['3C286_C']['flagged'], 0)
        self.assertEqual(summary['field']['3C286_D']['flagged'], 0)

    def test_list_field_Selection_for_bpass(self):
        """Flagdata:: Manually flag a bpass-based CalTable using list mode """
        
        flagdata(vis=self.vis, mode='list', inpfile=["field='3C286_A'"],
                 flagbackup=False)
        summary=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(summary['field']['3C286_A']['flagged'], 499200.0)
        self.assertEqual(summary['field']['3C286_B']['flagged'], 0)
        self.assertEqual(summary['field']['3C286_C']['flagged'], 0)
        self.assertEqual(summary['field']['3C286_D']['flagged'], 0)

    def test_manual_antenna_selection_for_bpass(self):
        """Flagdata:: Manually flag a bpass-based CalTable using antenna selection"""
        flagdata(vis=self.vis, antenna='ea09', flagbackup=False)
        summary=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(summary['antenna']['ea09']['flagged'], 48000.0)
        self.assertEqual(summary['antenna']['ea10']['flagged'], 0.0)

    def test_list_antenna_Selection_for_bpass(self):
        """Flagdata:: Manually flag a bpass-based CalTable using list mode"""
        
        flagdata(vis=self.vis, mode='list', inpfile=["antenna='ea09'"],
                 flagbackup=False)
        summary=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(summary['antenna']['ea09']['flagged'], 48000.0)
        self.assertEqual(summary['antenna']['ea10']['flagged'], 0.0)
        
    def test_clip_nan_and_inf_cparam_all_for_bpass(self):
        """Flagdata:: Clip only NaNs and Infs in all calibration solutions of CPARAM column"""

        flagdata(vis=self.vis, mode='clip',datacolumn='CPARAM', correlation='',
                 flagbackup=False)
        summary=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(summary['total'], 1248000.0)
        self.assertEqual(summary['flagged'], 0.0)
        self.assertEqual(summary['correlation']['Sol1']['flagged'], 0.0)
        self.assertEqual(summary['correlation']['Sol1']['total'], 624000.0)
        self.assertEqual(summary['correlation']['Sol2']['flagged'], 0.0)
        self.assertEqual(summary['correlation']['Sol2']['total'], 624000.0)

    def test_clip_minmax_cparam_all_for_bpass(self):
        """Flagdata:: Clip all calibration solutions of CPARAM column using a minmax range"""

        flagdata(vis=self.vis, mode='clip',clipzeros=True, clipminmax=[0,0.3], datacolumn='CPARAM',
                 flagbackup=False)
        summary=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(summary['flagged'], 11175.0)
        self.assertEqual(summary['total'], 1248000)
        self.assertEqual(summary['correlation']['Sol1']['flagged'], 11136.0)
        self.assertEqual(summary['correlation']['Sol2']['flagged'], 39)

    def test_clip_minmax_snr_all_for_bpass(self):
        """Flagdata:: Test cliping all calibration solution products of SNR column using a 
        minmax range for bpass CalTable"""

        flagdata(vis=self.vis, mode='clip', clipzeros=True,clipminmax=[0.,550.],datacolumn='snr',
                 correlation='', flagbackup=False)
        summary=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(summary['total'], 1248000.0)
        self.assertEqual(summary['flagged'], 74371.0)
        self.assertEqual(summary['correlation']['Sol1']['flagged'], 36327.0)
        self.assertEqual(summary['correlation']['Sol1']['total'], 624000.0)
        self.assertEqual(summary['correlation']['Sol2']['flagged'], 38044.0)
        self.assertEqual(summary['correlation']['Sol2']['total'], 624000.0)

    def test_clip_one_list(self):
        '''Flagdata: Flag one solution using one command in a list'''
        flagdata(vis=self.vis, mode='list', inpfile=["mode='clip' clipminmax=[0,3] correlation='REAL_Sol1'"],
                 flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 309388)
        self.assertEqual(res['correlation']['Sol2']['flagged'], 0)
                
    def test_rflag_cparam_sol2_for_bpass(self):
        """Flagdata:: Test rflag solution 2 of CPARAM column for bpass"""

        flagdata(vis=self.vis, mode='rflag', correlation='Sol2', flagbackup=False)
        summary=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(summary['flagged'], 13197)
        self.assertEqual(summary['correlation']['Sol1']['flagged'], 0)
        self.assertEqual(summary['correlation']['Sol2']['flagged'], 13197)

    def test_tfcrop_cparam_all_for_bpass(self):
        """Flagdata:: Test tfcrop in ABS_ALL calibration solutions of CPARAM column"""

        flagdata(vis=self.vis, mode='clip', datacolumn='CPARAM',correlation='ABS_ALL',clipzeros=True,
                 flagbackup=False)
        flagdata(vis=self.vis, mode='tfcrop', datacolumn='CPARAM',correlation='ABS_ALL',
                 flagbackup=False)
        summary=flagdata(vis=self.vis, mode='summary')
#        self.assertTrue(abs(summary['flagged'] - 63861.0) <= 5)
#        self.assertEqual(abs(summary['flagged'] - 69369) <= 5)
        assert abs(summary['flagged'] - 49524) <= 5
        assert abs(summary['correlation']['Sol1']['flagged'] - 30427) <= 5
        assert abs(summary['correlation']['Sol2']['flagged'] - 19097) <= 5

    def test_tfcrop_cparam_sol1_extension_for_bpass(self):
        """Flagdata:: Test tfcrop first calibration solution product of CPARAM column, 
        and then extend to the other solution for bpass CalTable"""

        flagdata(vis=self.vis, mode='clip', datacolumn='CPARAM',correlation='Sol1',clipzeros=True,
                 flagbackup=False)
        flagdata(vis=self.vis, mode='tfcrop', datacolumn='CPARAM',correlation='Sol1',
                 flagbackup=False)
        pre=flagdata(vis=self.vis, mode='summary')
        assert abs(pre['flagged'] - 30426) <= 5
        assert abs(pre['correlation']['Sol1']['flagged'] - 30426) <= 5
        
        # Extend to other solution
        flagdata(vis=self.vis, mode='extend', extendpols=True, growfreq=0.0, growtime=0.0,
                 flagbackup=False)
        pos=flagdata(vis=self.vis, mode='summary')
        assert abs(pos['flagged'] - 2*30426) <= 10
        assert abs(pos['correlation']['Sol2']['flagged'] - 30426) <= 5        

    def test_caltable_flagbackup(self):
        '''Flagmanager:: cal table mode=list, flagbackup=True/False'''
        
        # Need a fresh start
        os.system('rm -rf cal.fewscans.bpass*')
        self.setUp_bpass_case()
        
        # Create a local copy of the tool
        aflocal = casac.agentflagger()
        flagmanager(vis=self.vis, mode='list')
        aflocal.open(self.vis)
        self.assertEqual(len(aflocal.getflagversionlist()), 3)
        aflocal.done()

        flagdata(vis=self.vis, mode='unflag', flagbackup=False)
        flagmanager(vis=self.vis, mode='list')
        aflocal.open(self.vis)
        self.assertEqual(len(aflocal.getflagversionlist()), 3)
        aflocal.done()

        flagdata(vis=self.vis, mode='unflag', flagbackup=True)
        flagmanager(vis=self.vis, mode='list')
        aflocal.open(self.vis)
        self.assertEqual(len(aflocal.getflagversionlist()), 4)
        aflocal.done()
        
        newname = 'Ha! The best version ever!'

        flagmanager(vis=self.vis, mode='rename', oldname='flagdata_2', versionname=newname, 
                    comment='This is a *much* better name')
        flagmanager(vis=self.vis, mode='list')
        aflocal.open(self.vis)
        self.assertEqual(len(aflocal.getflagversionlist()), 4)
        aflocal.done()
        
        self.assertTrue(os.path.exists(self.vis+'.flagversions/flags.'+newname),
                        'Flagversion file does not exist: flags.'+newname)        

    def test_cal_time1(self):
        '''Flagdata: clip a timerange from one field'''
        # this timerange corresponds to field 3C286_A
        flagdata(vis=self.vis, mode='clip', timerange='<14:12:52',clipzeros=True,
                 clipminmax=[0.,0.35], flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['field']['3C286_A']['flagged'],2230)
        self.assertEqual(res['field']['3C286_B']['flagged'],0)
        self.assertEqual(res['field']['3C286_C']['flagged'],0)
        self.assertEqual(res['field']['3C286_D']['flagged'],0)
        self.assertEqual(res['flagged'],2230)

    def test_cal_time_field(self):
        '''Flagdata: clip a timerange from another field'''
        # this timerange corresponds to field 3C286_D
        flagdata(vis=self.vis, mode='clip', timerange='>14:58:33.6',clipzeros=True,
                 clipminmax=[0.,0.4], flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['field']['3C286_A']['flagged'],0)
        self.assertEqual(res['field']['3C286_B']['flagged'],0)
        self.assertEqual(res['field']['3C286_C']['flagged'],0)
        self.assertEqual(res['field']['3C286_D']['flagged'],2221)
        self.assertEqual(res['flagged'],2221)
        
    def test_cal_time_corr(self):
        '''Flagdata: select a timerange for one solution'''
        flagdata(vis=self.vis, mode='clip', clipminmax=[0.,0.4], timerange='14:23:50~14:48:40.8',
                 correlation='Sol2',flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['correlation']['Sol1']['flagged'], 0)
        self.assertEqual(res['correlation']['Sol2']['flagged'], 17)
        self.assertEqual(res['flagged'],17)
        
        # Check that the timerange selection was taken. Flag only the solution
        flagdata(vis=self.vis, mode='unflag', flagbackup=True)
        flagdata(vis=self.vis, mode='clip', clipminmax=[0.,0.4], correlation='Sol2', 
                 flagbackup=False)
        res1=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res1['correlation']['Sol1']['flagged'], 0)
        self.assertEqual(res1['correlation']['Sol2']['flagged'], 22)
        self.assertEqual(res1['flagged'],22)
        self.assertEqual(res1['flagged']-res['flagged'], 5)

    def test_observation(self):
        '''flagdata: flag an observation from an old cal table format'''
        # Note: this cal table does not have an observation column. 
        # The column and sub-table should be added and the flagging
        # should happen after this.
        flagdata(vis=self.vis, observation='0', flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'],1248000)
        self.assertEqual(res['total'],1248000)
        

class test_newcal(test_base):
    """Flagdata:: Test flagging task with new CalTable format"""
    
    def setUp(self):
        self.setUp_newcal()
        
    def test_newcal_selection1(self):
        '''Flagdata: select one solution for one scan and spw'''
        flagdata(vis=self.vis, mode='clip', clipminmax=[0,0.1], correlation='Sol1', spw='0',
                 scan='46', flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['correlation']['Sol2']['flagged'], 0)
        self.assertEqual(res['correlation']['Sol1']['flagged'], 27)
        self.assertEqual(res['spw']['0']['flagged'], 27)
        self.assertEqual(res['scan']['46']['flagged'], 27)
        self.assertEqual(res['flagged'],27)
        
    def test_newcal_time1(self):
        '''Flagdata: select a timerange in a new cal table'''
        flagdata(vis=self.vis, mode='manual', timerange="09:36:00~16:48:00", flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['field']['1328+307']['flagged'],0)
        self.assertEqual(res['field']['2229+695']['flagged'],2052)
        self.assertEqual(res['flagged'],2052)
        
    def test_newcal_time2(self):
        '''Flagdata: select a timerange for half the scans'''
        flagdata(vis=self.vis, mode='manual', timerange="09:20:00~14:12:00", flagbackup=False)
        
        # It should flag scans 1~25
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['1']['flagged'],108)
        self.assertEqual(res['scan']['2']['flagged'],108)
        self.assertEqual(res['scan']['25']['flagged'],108)
        self.assertEqual(res['scan']['27']['flagged'],0)
        # NOTE: data DOES not have all scans
        self.assertEqual(res['flagged'],108*14)
        
    def test_newcal_clip(self):
        '''Flagdata: clip zeros in one solution'''
        flagdata(vis=self.vis, mode='clip', clipzeros=True, correlation='Sol2', flagbackup=False)
        
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['correlation']['Sol1']['flagged'],0)
        self.assertEqual(res['correlation']['Sol2']['flagged'],1398)
        self.assertEqual(res['flagged'],1398)

    def test_newcal_obs1(self):
        '''flagdata: flag an observation from a new cal table format'''
        flagdata(vis=self.vis, observation='1', flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['observation']['0']['flagged'],0)
        self.assertEqual(res['observation']['1']['flagged'],2052)
        self.assertEqual(res['flagged'],2052)
        self.assertEqual(res['total'],2916)

    def test_newcal_obs2(self):
        '''flagdata: flag an observation and a scan from a new cal table format'''
        # observation=0 has only scan=1
        flagdata(vis=self.vis, observation='0', flagbackup=False)                
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['observation']['0']['flagged'],108)
        self.assertEqual(res['scan']['1']['flagged'],108)
        self.assertEqual(res['flagged'],108)
        
        # Check that obs=0 is scan=1
        res=flagdata(vis=self.vis, mode='unflag')
        flagdata(vis=self.vis, scan='1', flagbackup=False)                
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['observation']['0']['flagged'],108)
        self.assertEqual(res['scan']['1']['flagged'],108)
        self.assertEqual(res['flagged'],108)

# Cleanup class 
class cleanup(test_base):
    
    def tearDown(self):
        os.system('rm -rf ngc5921.*ms* testwma*ms*')
        os.system('rm -rf flagdatatest.*ms*')
        os.system('rm -rf missing-baseline.*ms*')
        os.system('rm -rf multiobs.*ms*')
        os.system('rm -rf flagdatatest-alma.*ms*')
        os.system('rm -rf Four_ants_3C286.*ms*')
        os.system('rm -rf shadowtest_part.*ms*')
        os.system('rm -rf testmwa.*ms*')
        os.system('rm -rf cal.fewscans.bpass*')
        os.system('rm -rf X7ef.tsys* ap314.gcal*')
        os.system('rm -rf list*txt')

    def test_runTest(self):
        '''flagdata: Cleanup'''
        pass


def suite():
    return [test_tfcrop,
            test_rflag,
            test_shadow,
            test_flagmanager,
            test_selections,
            test_selections2,
            test_selections_alma,
            test_statistics_queries,
            test_msselection,
            test_elevation,
            test_list_list,
            test_list_file,
            test_clip,
            test_CASA_4_0_bug_fix,
            test_correlations,
            test_tsys,
            test_bandpass,
            test_newcal,
            cleanup]
