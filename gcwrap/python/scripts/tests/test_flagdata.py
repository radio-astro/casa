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

print 'flagdata tests will use data from '+datapath         

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

    def test_extend1(self):
        '''flagdata:: Extend the flags created by clip'''
        flagdata(vis=self.vis, mode='clip', correlation='abs_rr', clipminmax=[0,2])
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['correlation']['RR']['flagged'], 43)
        self.assertEqual(res['correlation']['LL']['flagged'], 0)
        flagdata(vis=self.vis, mode='extend', extendpols=True, savepars=False)
        test_eq(flagdata(vis=self.vis, mode='summary', correlation='Ll'), 1099776, 43)

class test_rflag(test_base):
    """flagdata:: Test of mode = 'rflag'"""
    
    def setUp(self):
        self.setUp_data4tfcrop()
        
    def test_rflag1(self):
        '''flagdata:: Test1 of mode = rflag : automatic thresholds'''
        flagdata(vis=self.vis, mode='rflag', spw='9,10', timedev=[], freqdev=[]);
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 42728.0)
        self.assertEqual(res['antenna']['ea19']['flagged'], 18411.0)
        self.assertEqual(res['spw']['7']['flagged'], 0)

    def test_rflag2(self):
        '''flagdata:: Test2 of mode = rflag : partially-specified thresholds'''
        flagdata(vis=self.vis, mode='rflag', spw='9,10', timedev=[[1,10,0.1],[1,11,0.07]], \
                       freqdev=0.5);
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
                      freqdev='fdevfile.txt', action='apply');
        res1 = flagdata(vis=self.vis, mode='summary')
        # (2) Test rflag output written to cmd file via mode='rflag' and 'savepars' 
        #      and then read back in via list mode. 
        #      Also test the 'savepars' when timedev and freqdev are specified differently...
        flagdata(vis=self.vis,mode='unflag');
        flagdata(vis=self.vis, mode='rflag', spw='9,10', timedev='', \
                      freqdev=[],action='calculate',savepars=True,outfile='outcmd.txt');
        flagdata(vis=self.vis, mode='list', inpfile='outcmd.txt');
        res2 = flagdata(vis=self.vis, mode='summary')

        #print res1['flagged'], res2['flagged']
        self.assertTrue(abs(res1['flagged']-res2['flagged'])<10000)
        self.assertTrue(abs(res1['flagged']-39504.0)<10000)

    def test_rflag4(self):
        '''flagdata:: Test4 of mode = rflag : correlation selection'''
        flagdata(vis=self.vis, mode='rflag', spw='9,10', correlation='rr,ll');
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['correlation']['RR']['flagged'], 9781.0)
        self.assertEqual(res['correlation']['LL']['flagged'], 10355.0)
        self.assertEqual(res['correlation']['LR']['flagged'], 0,)
        self.assertEqual(res['correlation']['RL']['flagged'], 0,)


class test_shadow(test_base):
    def setUp(self):
        self.setUp_shadowdata2()

    def test_CAS2399(self):
        '''flagdata: shadow by antennas not present in MS'''
        
        if os.path.exists("cas2399.txt"):
            os.system('rm -rf cas2399.txt')
        
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

        filename = 'cas2399.txt'
        create_input(input, filename)
        
        flagdata(vis=self.vis, mode='shadow', tolerance=10.0, addantenna=filename)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['antenna']['VLA18']['flagged'], 3364)
        self.assertEqual(res['antenna']['VLA19']['flagged'], 1124)
        self.assertEqual(res['antenna']['VLA20']['flagged'], 440)
        
    def test_addantenna(self):
        '''flagdata: use antenna file in list mode'''
        if os.path.exists("myants.txt"):
            os.system('rm -rf myants.txt')
        
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

        antfile = 'myants.txt'
        create_input(input, antfile)
        
        # Create list file
        input = "mode='shadow' tolerance=10.0 addantenna='myants.txt'"
        filename = 'listfile.txt'
        create_input(input, filename)
        
        # Flag
        flagdata(vis=self.vis, mode='list', inpfile=filename, savepars=True, outfile='withdict.txt')
        
        # Check flags
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['antenna']['VLA18']['flagged'], 3364)
        self.assertEqual(res['antenna']['VLA19']['flagged'], 1124)
        self.assertEqual(res['antenna']['VLA20']['flagged'], 440)        

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
        flagdata(vis=self.vis, mode='manual', antenna='VA05&&&')
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
        flagdata(vis=self.vis, autocorr=True)
        s = flagdata(vis = self.vis, mode="summary")
        self.assertEqual(s['flagged'], 203994)
        
    def test_autocorr3(self):
        '''flagdata: flag auto-corrs in list mode'''
        # creat input list
        input = "scan='1' mode='manual' autocorr=true reason='AUTO'\n"\
                "scan='3' autocorr=True reason='AUTO'\n"\
                "scan='4' reason='ALL'"
        filename = 'listauto.txt'
        create_input(input, filename)
        
        # select only the autocorr reasons to flag
        flagdata(vis=self.vis, mode='list', inpfile=filename, reason='AUTO', action='apply')
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
        
        flagdata(vis=self.vis, correlation='LL', savepars=False)
        flagdata(vis=self.vis, spw='0:17~19', savepars=False)
        flagdata(vis=self.vis, antenna='VA05&&VA09', savepars=False)
        flagdata(vis=self.vis, antenna='VA14', savepars=False)
        flagdata(vis=self.vis, field='1', savepars=False)
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
                 savepars=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 1414186)

    def test_chanavg1(self):
        flagdata(vis=self.vis, mode='clip',channelavg=True, clipminmax=[30., 60.], correlation='ABS_RR',
                 savepars=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 1347822)

    def test_chanavg2(self):
        flagdata(vis=self.vis, mode='clip',channelavg=False, clipminmax=[30., 60.], spw='0:0~10', 
                 correlation='ABS_RR', savepars=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 242053)

    def test_chanavg3(self):
        flagdata(vis=self.vis, mode='clip',channelavg=True, clipminmax=[30., 60.], spw='0:0~10',
                 correlation='ABS_RR', savepars=False)
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
        flagdata(vis=self.vis, mode='quack', quackmode='beg', quackinterval=1, savepars=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 329994)

    def test10(self):
        '''flagdata: quack mode'''
        flagdata(vis=self.vis, mode='quack', quackmode='endb', quackinterval=1, savepars=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 333396)

    def test11(self):
        '''flagdata: quack mode'''
        flagdata(vis=self.vis, mode='quack', quackmode='end', quackinterval=1, savepars=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 2520882)

    def test12(self):
        '''flagdata: quack mode'''
        flagdata(vis=self.vis, mode='quack', quackmode='tail', quackinterval=1, savepars=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 2524284)

    def test13(self):
        '''flagdata: quack mode, quackincrement'''
        flagdata(vis=self.vis, mode='quack', quackinterval=50, quackmode='endb', quackincrement=True,
                 savepars=False)
        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 571536)

        flagdata(vis=self.vis, mode='quack', quackinterval=20, quackmode='endb', quackincrement=True,
                 savepars=False)
        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 857304)
        
        flagdata(vis=self.vis, mode='quack', quackinterval=150, quackmode='endb', quackincrement=True,
                 savepars=False)
        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 1571724)
        
        flagdata(vis=self.vis, mode='quack', quackinterval=50, quackmode='endb', quackincrement=True,
                 savepars=False)
        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 1762236)
        flagdata(vis=self.vis, mode='unflag', savepars=False)



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
        flagdata(vis=self.vis, antenna='VA02', savepars=False)
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='VA02'), 196434, 196434)

    def test_spw(self):
        '''flagdata: spw selection'''
        flagdata(vis=self.vis, spw='0', savepars=False)
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)

    def test_spw_list(self):
        '''flagdata: spw selection in list mode''' 
        spwfile = 'spwflags.txt'
        if os.path.exists(spwfile):
            os.system('rm -rf '+spwfile)
                   
        flagdata(vis=self.vis, spw='0:1~10', savepars=True, outfile=spwfile)
        res0 = flagdata(vis=self.vis, mode='summary', spwchan=True)
        self.assertEqual(res0['flagged'], 453060, 'Only channels 1~10 should be flagged')
        
        # Unflag
        flagdata(vis=self.vis, mode='unflag')
        ures = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(ures['flagged'], 0)
        
        # Flag using the saved list
        flagdata(vis=self.vis, mode='list', inpfile=spwfile, action='apply')
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
        flagdata(vis=self.vis, correlation='LL', savepars=False)
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 98217)
        test_eq(flagdata(vis=self.vis, mode='summary', correlation='RR'), 1427139, 0)
        flagdata(vis=self.vis, mode='unflag', savepars=False)
        flagdata(vis=self.vis, correlation='LL,RR', savepars=False)
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)
        
        flagdata(vis=self.vis, mode='unflag', savepars=False)
        flagdata(vis=self.vis, mode='clip', correlation='NORM_RR,LL', clipminmax=[0.,3.])
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 204979)
#        flagdata(vis=self.vis, correlation='LL RR')
#        flagdata(vis=self.vis, correlation='LL ,, ,  ,RR')
#        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)

    def test_field(self):
        '''flagdata: field selection'''
        flagdata(vis=self.vis, field='0', savepars=False)
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 39186)

    def test_uvrange(self):
        '''flagdata: uvrange selection'''
        flagdata(vis=self.vis, uvrange='200~400m', savepars=False)
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='VA02'), 196434, 55944)

    def test_timerange(self):
        '''flagdata: timerange selection'''
        flagdata(vis=self.vis, timerange='09:50:00~10:20:00', savepars=False)
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 6552)

    def test_array(self):
        '''flagdata: array selection'''
        flagdata(vis=self.vis, array='0', savepars=False)
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)
                
    def test_action(self):
        '''flagdata: action = calculate'''
        flagdata(vis=self.vis, antenna='2,3,4', action='calculate')
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 0, 'Nothing should be flagged when action=calculate')

    def test_missing_corr_product(self):
        '''CAS-4234: Keep going when one of the corr products is not available but others are present'''
        flagdata(vis=self.vis, correlation='LL,LR', savepars=False)
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 98217)
        test_eq(flagdata(vis=self.vis, mode='summary', correlation='RR'), 1427139, 0)
        flagdata(vis=self.vis, mode='unflag', savepars=False)
        flagdata(vis=self.vis, correlation='LL,RR,RL', savepars=False)
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
        flagdata(vis=self.vis, correlation='I', savepars=False)
        test_eq(flagdata(vis=self.vis, mode='summary'),1154592, 22752)

    def test_abs_wvr(self):
        '''flagdata: clip ABS_WVR'''
        flagdata(vis=self.vis, mode='clip',clipminmax=[0,50], correlation='ABS_WVR', savepars=False)
        test_eq(flagdata(vis=self.vis, mode='summary'),1154592, 22752)
        
    def test_abs_i(self):
        '''flagdata: clip ABS_I. Do not flag WVR'''
        flagdata(vis=self.vis, mode='clip', clipminmax=[0,50], correlation='ABS_I', savepars=False)
        test_eq(flagdata(vis=self.vis, mode='summary'),1154592, 0)

    def test_abs_all(self):
        '''flagdata: clip ABS ALL. Do not flag WVR'''
        flagdata(vis=self.vis, mode='clip', clipminmax=[0,1], correlation='ABS ALL', savepars=False)
        test_eq(flagdata(vis=self.vis, mode='summary'),1154592, 130736)
        test_eq(flagdata(vis=self.vis, mode='summary', correlation='I'),22752, 0)

    def test_spw(self):
        '''flagdata: flag various spw'''
        # Test that a white space in the spw parameter is taken correctly
        flagdata(vis=self.vis, mode='manual', spw='1,3, 4', savepars=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['spw']['0']['flagged'], 0, 'spw=0 should not be flagged')
        self.assertEqual(res['spw']['1']['flagged'], 552960, 'spw=1 should be fully flagged')
        self.assertEqual(res['spw']['4']['flagged'], 22752, 'spw=4 should not be flagged')
        self.assertEqual(res['spw']['4']['total'], 22752, 'spw=4 should not be flagged')

class test_selections2(test_base):
    '''Test other selections'''
    
    def setUp(self):
        self.setUp_multi()
        
    def test_observation1(self):
        '''flagdata: observation ID selections'''
        # string
        flagdata(vis=self.vis, observation='1', savepars=False)
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
        input = "observation='0' mode='manual'"
        filename = 'obs2.txt'
        create_input(input, filename)
        
        flagdata(vis=self.vis, mode='list', inpfile=filename, savepars=False)
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

        flagdata(vis = self.vis, mode = 'elevation', lowerlimit = 50, savepars=False)

        test_eq(flagdata(vis=self.vis, mode='summary'), self.all, 0)

        flagdata(vis = self.vis, mode = 'elevation', lowerlimit = 55, savepars=False)

        test_eq(flagdata(vis=self.vis, mode='summary'), self.all, self.x55)

        flagdata(vis = self.vis, mode = 'elevation', lowerlimit = 60, savepars=False)

        test_eq(flagdata(vis=self.vis, mode='summary'), self.all, self.x60)

        flagdata(vis = self.vis, mode = 'elevation', lowerlimit = 65, savepars=False)

        test_eq(flagdata(vis=self.vis, mode='summary'), self.all, self.x65)

    def test_upper(self):
        flagdata(vis = self.vis, mode = 'elevation', upperlimit = 60, savepars=False)

        test_eq(flagdata(vis=self.vis, mode='summary'), self.all, self.all - self.x60)


    def test_interval(self):
        flagdata(vis = self.vis,
                  mode = 'elevation',
                  lowerlimit = 55,
                  upperlimit = 60,
                  savepars=False)

        test_eq(flagdata(vis=self.vis, mode='summary'), self.all, self.all - (self.x60 - self.x55))


class test_list_file(test_base):
    """Test of mode = 'list' using input file"""
    
    def setUp(self):
        self.setUp_ngc5921()

    def test_file1(self):
        '''flagdata: apply flags from a list and do not save'''
        # creat input list
        input = "scan=1~3 mode=manual\n"+"scan=5 mode=manualflag\n"\
                "#scan='4'"
        filename = 'list1.txt'
        create_input(input, filename)
        
        # apply and don't save to MS. Ignore comment line
        flagdata(vis=self.vis, mode='list', inpfile=filename, savepars=False, action='apply')
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['4']['flagged'], 0)
        self.assertEqual(res['flagged'], 1711206, 'Total flagged does not match')
        
    def test_file2(self):
        '''flagdata: only save parameters without running the tool'''
        # creat input list
        input = "scan=1~3 mode=manual\n"+"scan=5 mode=manual\n"
        filename = 'list2.txt'
        create_input(input, filename)

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
        input = "scan=1~3 mode=manual\n"+"scan=5 mode=manual\n"
        filename = 'list3.txt'
        create_input(input, filename)

        # Delete any rows from FLAG_CMD
        flagcmd(vis=self.vis, action='clear', clearall=True)
        
        # Flag from list and save to FLAG_CMD
        flagdata(vis=self.vis, mode='list', inpfile=filename, savepars=True)
        
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
                 savepars=True)
        
        flagcmd(vis=self.vis, action='apply')
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 2524284)

    def test_file5(self):
        '''flagdata: clip zeros in mode=list and save reason to FLAG_CMD'''
        # get the correct data, by passing the previous setUp()
        self.setUp_data4tfcrop()
        
        # creat input list
        input = "mode='clip' clipzeros=true reason='CLIP_ZERO'"
        filename = 'list5.txt'
        create_input(input, filename)

        # Save to FLAG_CMD
        flagdata(vis=self.vis, mode='list', inpfile=filename, action='', savepars=True)
        
        # Run in flagcmd and select by reason
        flagcmd(vis=self.vis, action='apply', reason='CLIP_ZERO')
        
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 274944, 'Should clip only spw=8')

    def test_file6(self):
        '''flagdata: select by reason in list mode from a file'''
        # creat input list
        input = "mode='manual' scan='1' reason='SCAN_1'\n"\
                "mode='manual' scan='2'\n"\
                "scan='3' reason='SCAN_3'\n"\
                "scan='4' reason=''"
        filename = 'list6.txt'
        create_input(input, filename)
        
        # Select one reason
        flagdata(vis=self.vis, mode='list', inpfile=filename, reason='SCAN_3')
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['3']['flagged'], 762048, 'Should flag only reason=SCAN_3')
        self.assertEqual(res['flagged'], 762048, 'Should flag only reason=SCAN_3')
        
        # Select list of reasons
        flagdata(vis=self.vis, mode='list', inpfile=filename, reason=['','SCAN_1'])
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['4']['flagged'], 95256, 'Should flag reason=\'\'')
        self.assertEqual(res['scan']['1']['flagged'], 568134, 'Should flag reason=SCAN_1')
        
        # No reason selection
        flagdata(vis=self.vis, mode='unflag')
        flagdata(vis=self.vis, mode='list', inpfile=filename)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['1']['flagged'], 568134)
        self.assertEqual(res['scan']['2']['flagged'], 238140)
        self.assertEqual(res['scan']['3']['flagged'], 762048)
        self.assertEqual(res['scan']['4']['flagged'], 95256)
        self.assertEqual(res['flagged'],568134+238140+762048+95256, 'Total flagged')
        
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
        flagdata(vis=self.vis, mode='list', inpfile='reason2.txt',reason='CLIPSCAN2_3')
        
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 69568)
        
    def test_reason3(self):
        '''flagdata: replace input reason from file with cmdreason'''
        # creat input list
        input = "mode='manual' scan='1' reason='SCAN_1'\n"\
                "mode='manual' scan='2'\n"\
                "scan='3' reason='SCAN_3'\n"\
                "scan='4' reason=''"
        filename = 'input3.txt'
        create_input(input, filename)
        
        flagdata(vis=self.vis, mode='list', inpfile=filename, savepars=True, outfile='reason3.txt',
                  cmdreason='MANUALFLAG', action='')
        
        # Apply the flag cmds
        flagdata(vis=self.vis, mode='list', inpfile='reason3.txt', reason='MANUALFLAG')
        
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 1663578)
        
class test_list_list(test_base):
    """Test of mode = 'list' using input list"""
    
    def setUp(self):
        self.setUp_ngc5921()

    def test_list1(self):
        '''flagdata: apply flags from a Python list and do not save'''
        # creat input list
        input = ["scan='1~3' mode='manual'",
                 "scan='5' mode='manualflag'",
                 "#scan='4'"]
        
        # apply and don't save to MS. Ignore comment line
        flagdata(vis=self.vis, mode='list', inpfile=input, savepars=False, action='apply')
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['4']['flagged'], 0)
        self.assertEqual(res['flagged'], 1711206, 'Total flagged does not match')
        
    def test_list2(self):
        '''flagdata: only save parameters without running the tool'''
        # creat input list
        input = ["scan='1~3' mode='manual'",
                 "scan='5' mode='manual'"]

        # save to another file
        if os.path.exists("myflags.txt"):
            os.system('rm -rf myflags.txt')
            
        flagdata(vis=self.vis, mode='list', inpfile=input, savepars=True, action='', outfile='myflags.txt')
        
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 0, 'No flags should have been applied')
        
    def test_list3(self):
        '''flagdata: Compare flags from flagdata and flagcmd'''
        # creat input list
        input = ["scan='1~3' mode='manual'",
                 "scan='5' mode='manual'"]
 
        # Delete any rows from FLAG_CMD
        flagcmd(vis=self.vis, action='clear', clearall=True)
        
        # Flag from list and save to FLAG_CMD
        flagdata(vis=self.vis, mode='list', inpfile=input)
        res1 = flagdata(vis=self.vis, mode='summary')
        
        # Unflag and save in flagcmd using the cmd mode
        flagdata(vis=self.vis, mode='unflag')
        flagcmd(vis=self.vis, inpmode='list', inpfile=input)
        res2 = flagdata(vis=self.vis, mode='summary')

        # Verify
        self.assertEqual(res1['flagged'], res2['flagged'])
        self.assertEqual(res1['total'], res2['total'])

    def test_list4(self):
        '''flagdata: clip zeros in mode=list and save reason to FLAG_CMD'''
        # get the correct data, by passing the previous setUp()
        self.setUp_data4tfcrop()
        
        # creat input list
        input = ["mode='clip' clipzeros=true reason='CLIP_ZERO'"]

        # Save to FLAG_CMD
        flagdata(vis=self.vis, mode='list', inpfile=input, action='', savepars=True)
        
        # Run in flagcmd and select by reason
        flagcmd(vis=self.vis, action='apply', reason='CLIP_ZERO')
        
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 274944, 'Should clip only spw=8')

    def test_list5(self):
        '''flagdata: select by reason in list mode from a list'''
        # creat input list
        input = ["mode='manual' scan='1' reason='SCAN_1'",
                "mode='manual' scan='2'",
                "scan='3' reason='SCAN_3'",
                "scan='4' reason=''"]
        
        # Select one reason
        flagdata(vis=self.vis, mode='list', inpfile=input, reason='SCAN_3')
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['3']['flagged'], 762048, 'Should flag only reason=SCAN_3')
        self.assertEqual(res['flagged'], 762048, 'Should flag only reason=SCAN_3')
        
        # Select list of reasons
        flagdata(vis=self.vis, mode='list', inpfile=input, reason=['','SCAN_1'])
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['4']['flagged'], 95256, 'Should flag reason=\'\'')
        self.assertEqual(res['scan']['1']['flagged'], 568134, 'Should flag reason=SCAN_1')
        
        # No reason selection
        flagdata(vis=self.vis, mode='unflag')
        flagdata(vis=self.vis, mode='list', inpfile=input)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['1']['flagged'], 568134)
        self.assertEqual(res['scan']['2']['flagged'], 238140)
        self.assertEqual(res['scan']['3']['flagged'], 762048)
        self.assertEqual(res['scan']['4']['flagged'], 95256)
        self.assertEqual(res['flagged'],568134+238140+762048+95256, 'Total flagged')
        
                
    def test_reason_list(self):
        '''flagdata: replace input reason from list with cmdreason'''
        # creat input list
        input = ["mode='manual' scan='1' reason='SCAN_1'",
                "mode='manual' scan='2'",
                "scan='3' reason='SCAN_3'",
                "scan='4' reason=''"]
        
        flagdata(vis=self.vis, mode='list', inpfile=input, savepars=True, outfile='reason3.txt',
                  cmdreason='MANUALFLAG', action='')
        
        # Apply the flag cmds
        flagdata(vis=self.vis, mode='list', inpfile='reason3.txt', reason='MANUALFLAG')
        
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 1663578)
                
        
        
class test_clip(test_base):
    """flagdata:: Test of mode = 'clip'"""
    
    def setUp(self):
        self.setUp_data4tfcrop()
        
    def test_clipzeros(self):
    	'''flagdata: clip only zero-value data'''
        flagdata(vis=self.vis, mode='clip', clipzeros=True)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'],274944,'Should clip only spw=8')
        

class test_CASA_4_0_bug_fix(test_base):
    """flagdata:: Regression test for the fixes introduced during the CASA 4.0 bug fix season"""

    def setUp(self):
        self.setUp_data4tfcrop()
        
    def test_CAS_4270(self):
        """flagdata: Test uvrange given in lambda units"""
                
        flagdata(vis=self.vis,mode='manual',uvrange='<2klambda')
        flagdata(vis=self.vis,mode='clip')
        summary_ref = flagdata(vis=self.vis,mode='summary')
        
        flagdata(vis=self.vis,mode='unflag')
        flagdata(vis=self.vis,mode='list',inpfile=["uvrange='<2Klambda'","mode='clip'"])
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
        
        flagdata(vis=self.vis,mode='quack',quackinterval=0)
        summary_zero = flagdata(vis=self.vis,mode='summary')
        self.assertEqual(summary_zero['flagged'],0,'Error in quack mode with quack interval 0')
        
        flagdata(vis=self.vis,mode='quack',quackinterval=1)
        summary_one = flagdata(vis=self.vis,mode='summary')
        
        flagdata(vis=self.vis,mode='unflag')
        flagdata(vis=self.vis,mode='quack')
        summary_default = flagdata(vis=self.vis,mode='summary')
        
        self.assertEqual(summary_one['flagged'],summary_default['flagged'],'Error in quack mode with quack interval 1')
        
    def test_alias(self):
        '''flagdata: Test tflagdata alias'''
        res = tflagdata(vis=self.vis, mode='summary')['flagged']
        self.assertEqual(res, 0)
        
# Cleanup class 
class cleanup(test_base):
    
    def tearDown(self):
        os.system('rm -rf ngc5921.*ms*')
        os.system('rm -rf flagdatatest.*ms*')
        os.system('rm -rf missing-baseline.*ms*')
        os.system('rm -rf multiobs.*ms*')
        os.system('rm -rf flagdatatest-alma.*ms*')
        os.system('rm -rf Four_ants_3C286.*ms*')
        os.system('rm -rf shadowtest_part.*ms*')
        os.system('rm -rf list*txt')

    def test1(self):
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
            cleanup]
