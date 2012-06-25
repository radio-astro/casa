import shutil
import unittest
import os
import filecmp
from tasks import *
from taskinit import *
from __main__ import default

#
# Test of tflagdata modes
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

# Base class which defines setUp functions
# for importing different data sets
class test_base(unittest.TestCase):
    def setUp_flagdatatest(self):
        self.vis = "flagdatatest.ms"

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
        default(tflagdata)
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
        default(tflagdata)
        tflagdata(vis=self.vis, mode='unflag', savepars=False)

    def setUp_data4tfcrop(self):
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

    def setUp_shadowdata2(self):
        self.vis = "shadowtest_part.ms"

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
        default(tflagdata)
        tflagdata(vis=self.vis, mode='unflag', savepars=False)


class test_tfcrop(test_base):
    """tflagdata:: Test of mode = 'tfcrop'"""
    
    def setUp(self):
        self.setUp_data4tfcrop()
        
    def test_tfcrop1(self):
        '''tflagdata:: Test1 of mode = tfcrop'''
        tflagdata(vis=self.vis, mode='tfcrop', correlation='ABS_RR',ntime=51.0,spw='9', savepars=False)
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 4489)
        self.assertEqual(res['antenna']['ea19']['flagged'], 2294)
        self.assertEqual(res['spw']['7']['flagged'], 0)
        
    def test_tfcrop2(self):
        '''tflagdata:: Test2 of mode = tfcrop ABS_ALL'''
        # Note : With ntime=51.0, 64-bit machines get 18696 flags, and 32-bit gets 18695 flags.
        #           As far as we can determine, this is a genuine precision-related difference.
        #           With ntime=53.0, there happens to be no difference.
        tflagdata(vis=self.vis, mode='tfcrop',ntime=53.0,spw='9', savepars=False)
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 18671)
        self.assertEqual(res['correlation']['LL']['flagged'], 4250)
        self.assertEqual(res['correlation']['RL']['flagged'], 5007)
        self.assertEqual(res['correlation']['LR']['flagged'], 4931)
        self.assertEqual(res['correlation']['RR']['flagged'], 4483)

    def test_extend1(self):
        '''tflagdata:: Extend the flags created by clip'''
        tflagdata(vis=self.vis, mode='clip', correlation='abs_rr', clipminmax=[0,2])
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['correlation']['RR']['flagged'], 43)
        self.assertEqual(res['correlation']['LL']['flagged'], 0)
        tflagdata(vis=self.vis, mode='extend', extendpols=True, savepars=False)
        test_eq(tflagdata(vis=self.vis, mode='summary', correlation='Ll'), 1099776, 43)

class test_rflag(test_base):
    """tflagdata:: Test of mode = 'rflag'"""
    
    def setUp(self):
        self.setUp_data4tfcrop()
        
    def test_rflag1(self):
        '''tflagdata:: Test1 of mode = rflag : automatic thresholds'''
        tflagdata(vis=self.vis, mode='rflag', spw='9,10', timedev=[], freqdev=[]);
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 42728.0)
        self.assertEqual(res['antenna']['ea19']['flagged'], 18411.0)
        self.assertEqual(res['spw']['7']['flagged'], 0)

    def test_rflag2(self):
        '''tflagdata:: Test2 of mode = rflag : partially-specified thresholds'''
        tflagdata(vis=self.vis, mode='rflag', spw='9,10', timedev=[[1,10,0.1],[1,11,0.07]], \
                       freqdev=0.5);
        res = tflagdata(vis=self.vis, mode='summary',spw='9,10,11')
        self.assertEqual(res['flagged'], 52411)
        self.assertEqual(res['antenna']['ea19']['flagged'], 24142)
        self.assertEqual(res['spw']['11']['flagged'], 0)

    def test_rflag3(self):
        '''tflagdata:: Test3 of mode = rflag : output/input via two methods'''
        # (1) Test input/output files, through the task, mode='rflag'
        tflagdata(vis=self.vis, mode='rflag', spw='9,10', timedev='tdevfile.txt', \
                      freqdev='fdevfile.txt', action='calculate');
        tflagdata(vis=self.vis, mode='rflag', spw='9,10', timedev='tdevfile.txt', \
                      freqdev='fdevfile.txt', action='apply');
        res1 = tflagdata(vis=self.vis, mode='summary')
        # (2) Test rflag output written to cmd file via mode='rflag' and 'savepars' 
        #      and then read back in via list mode. 
        #      Also test the 'savepars' when timedev and freqdev are specified differently...
        tflagdata(vis=self.vis,mode='unflag');
        tflagdata(vis=self.vis, mode='rflag', spw='9,10', timedev='', \
                      freqdev=[],action='calculate',savepars=True,outfile='outcmd.txt');
        tflagdata(vis=self.vis, mode='list', inpfile='outcmd.txt');
        res2 = tflagdata(vis=self.vis, mode='summary')

        #print res1['flagged'], res2['flagged']
        self.assertEqual(res1['flagged'],res2['flagged']);
        self.assertEqual(res1['flagged'], 39504.0,)

    def test_rflag4(self):
        '''tflagdata:: Test4 of mode = rflag : correlation selection'''
        tflagdata(vis=self.vis, mode='rflag', spw='9,10', correlation='rr,ll');
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['correlation']['RR']['flagged'], 9781.0)
        self.assertEqual(res['correlation']['LL']['flagged'], 10355.0)
        self.assertEqual(res['correlation']['LR']['flagged'], 0,)
        self.assertEqual(res['correlation']['RL']['flagged'], 0,)


class test_shadow(test_base):
    def setUp(self):
        self.setUp_shadowdata2()

    def test_CAS2399(self):
        '''tflagdata: shadow by antennas not present in MS'''
        
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
        
        tflagdata(vis=self.vis, mode='shadow', tolerance=10.0, addantenna=filename)
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['antenna']['VLA18']['flagged'], 3364)
        self.assertEqual(res['antenna']['VLA19']['flagged'], 1124)
        self.assertEqual(res['antenna']['VLA20']['flagged'], 440)
        
    def test_addantenna(self):
        '''tflagdata: use antenna file in list mode'''
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
        tflagdata(vis=self.vis, mode='list', inpfile=filename, savepars=True, outfile='withdict.txt')
        
        # Check flags
        res = tflagdata(vis=self.vis, mode='summary')
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
        tflocal = casac.testflagger()
        flagmanager(vis=self.vis, mode='list')
        tflocal.open(self.vis)
        self.assertEqual(len(tflocal.getflagversionlist()), 3)
        tflocal.done()


        tflagdata(vis=self.vis, mode='unflag', flagbackup=False)
        flagmanager(vis=self.vis, mode='list')
        tflocal.open(self.vis)
        self.assertEqual(len(tflocal.getflagversionlist()), 3)
        tflocal.done()

        tflagdata(vis=self.vis, mode='unflag', flagbackup=True)
        flagmanager(vis=self.vis, mode='list')
        tflocal.open(self.vis)
        self.assertEqual(len(tflocal.getflagversionlist()), 4)
        tflocal.done()
        
        newname = 'Ha! The best version ever!'

        flagmanager(vis=self.vis, mode='rename', oldname='tflagdata_2', versionname=newname, 
                    comment='This is a *much* better name')
        flagmanager(vis=self.vis, mode='list')
        tflocal.open(self.vis)
        self.assertEqual(len(tflocal.getflagversionlist()), 4)
        tflocal.done()
        
        self.assertTrue(os.path.exists('flagdatatest.ms.flagversions/flags.'+newname),
                        'Flagversion file does not exist: flags.'+newname)

    def test2m(self):
        """flagmanager test2m: Create, then restore autoflag"""

        tflagdata(vis=self.vis, mode='summary')
        flagmanager(vis=self.vis)
        
        tflagdata(vis=self.vis, mode='manual', antenna="2", flagbackup=True)
        
        flagmanager(vis=self.vis)
        ant2 = tflagdata(vis=self.vis, mode='summary')['flagged']

        print "After flagging antenna 2 there were", ant2, "flags"

        # Change flags, then restore
        tflagdata(vis=self.vis, mode='manual', antenna="3", flagbackup=True)
        flagmanager(vis = self.vis)
        ant3 = tflagdata(vis=self.vis, mode='summary')['flagged']

        print "After flagging antenna 2 and 3 there were", ant3, "flags"

        flagmanager(vis=self.vis, mode='restore', versionname='tflagdata_3')
        restore2 = tflagdata(vis=self.vis, mode='summary')['flagged']

        print "After restoring pre-antenna 3 flagging, there are", restore2, "flags; should be", ant2

        self.assertEqual(restore2, ant2)

    def test_CAS2701(self):
        """flagmanager: Do not allow flagversion=''"""
        
        # Create a local copy of the tool
        tflocal = casac.testflagger()
        
        tflocal.open(self.vis)
        l = len(tflocal.getflagversionlist())
        tflocal.done()
        
        flagmanager(vis = self.vis,
                    mode = "save",
                    versionname = "non-empty-string")

        tflocal.open(self.vis)
        self.assertEqual(len(tflocal.getflagversionlist()), l+1)
        tflocal.done()

        flagmanager(vis = self.vis,
                    mode = "save",
                    versionname = "non-empty-string")

        tflocal.open(self.vis)
        self.assertEqual(len(tflocal.getflagversionlist()), l+1)
        tflocal.done()


class test_msselection(test_base):

    def setUp(self):
        self.setUp_ngc5921()

    def test_simple(self):
        '''tflagdata: select only cross-correlations'''
        baselines = tflagdata(vis = self.vis, mode="summary", antenna="9", basecnt=True)['baseline'].keys()
        assert "9&&9" not in baselines
        assert "9&&10" in baselines
        assert "9&&11" in baselines
        assert "10&&10" not in baselines
        assert "10&&11" not in baselines

        baselines = tflagdata(vis = self.vis, mode="summary", antenna="9,10", basecnt=True)['baseline'].keys()
        assert "9&&9" not in baselines
        assert "9&&10" in baselines
        assert "9&&11" in baselines
        assert "10&&10" not in baselines
        assert "10&&11" in baselines

    def test_amp(self):
        '''tflagdata: select only cross-correlations'''
        baselines = tflagdata(vis = self.vis, mode="summary", antenna="9,10&",basecnt=True)['baseline'].keys()
        assert "9&&9" not in baselines
        assert "9&&10" in baselines
        assert "9&&11" not in baselines
        assert "10&&10" not in baselines
        assert "10&&11" not in baselines

        baselines = tflagdata(vis = self.vis, mode="summary", antenna="9&10",basecnt=True)['baseline'].keys()
        assert "9&&9" not in baselines
        assert "9&&10" in baselines
        assert "9&&11" not in baselines
        assert "10&&10" not in baselines
        assert "10&&11" not in baselines
        
    def test_autocorr1(self):
        '''tflagdata: flag only auto-correlations with antenna selection'''
        tflagdata(vis=self.vis, mode='manual', antenna='5&&&')
        s = tflagdata(vis = self.vis, mode="summary",basecnt=True)['baseline']
        assert s['5&&5']['flagged'] == 7560
        assert s['1&&5']['flagged'] == 0
        assert s['2&&5']['flagged'] == 0
        assert s['5&&10']['flagged'] == 0
        assert s['5&&11']['flagged'] == 0

        s = tflagdata(vis = self.vis, mode="summary")
        self.assertEqual(s['flagged'], 7560)

    def test_autocorr2(self):
        '''tflagdata: flag auto-corrs with parameter'''
        tflagdata(vis=self.vis, autocorr=True)
        s = tflagdata(vis = self.vis, mode="summary")
        self.assertEqual(s['flagged'], 203994)
        
    def test_autocorr3(self):
        '''tflagdata: flag auto-corrs in list mode'''
        # creat input list
        input = "scan='1' mode='manual' autocorr=true reason='AUTO'\n"\
                "scan='3' autocorr=True reason='AUTO'\n"\
                "scan='4' reason='ALL'"
        filename = 'listauto.txt'
        create_input(input, filename)
        
        # select only the autocorr reasons to flag
        tflagdata(vis=self.vis, mode='list', inpfile=filename, reason='AUTO', action='apply')
        s = tflagdata(vis = self.vis, mode="summary", basecnt=True)
        self.assertEqual(s['scan']['4']['flagged'], 0)
        self.assertEqual(s['baseline']['9&&28']['flagged'], 0)
        self.assertEqual(s['baseline']['9&&9']['flagged'], 3528)
        
        # select only the third line scan=4
        tflagdata(vis=self.vis, mode='list', inpfile=filename, reason='ALL', action='apply')
        s = tflagdata(vis = self.vis, mode="summary", basecnt=True)
        self.assertEqual(s['scan']['4']['flagged'], 95256)
        self.assertEqual(s['baseline']['9&&28']['flagged'], 252)
        self.assertEqual(s['baseline']['9&&9']['flagged'], 3780)
        self.assertEqual(s['flagged'], 190386)
                

class test_statistics_queries(test_base):

    def setUp(self):
        self.setUp_ngc5921()

    def test_CAS2021(self):
        '''tflagdata: test antenna negation selection'''
        
        tflagdata(vis=self.vis, antenna='!5', savepars=False) 
        s = tflagdata(vis = self.vis, mode="summary",basecnt=True)['baseline']
        assert s['1&&5']['flagged'] == 0 
        assert s['2&&5']['flagged'] == 0
        assert s['3&&5']['flagged'] == 0
        assert s['4&&5']['flagged'] == 0
        assert s['5&&6']['flagged'] == 0
        assert s['5&&7']['flagged'] == 0
        assert s['5&&8']['flagged'] == 0
        assert s['5&&9']['flagged'] == 0
        assert s['5&&10']['flagged'] == 0
        assert s['5&&11']['flagged'] == 0
        assert s['5&&12']['flagged'] == 0
        assert s['5&&13']['flagged'] == 0
        assert s['5&&14']['flagged'] == 0
        assert s['5&&15']['flagged'] == 0
        assert s['5&&16']['flagged'] == 0
        assert s['5&&17']['flagged'] == 0
        assert s['5&&18']['flagged'] == 0
        assert s['5&&19']['flagged'] == 0
        assert s['5&&20']['flagged'] == 0
        assert s['5&&21']['flagged'] == 0
        assert s['5&&22']['flagged'] == 0
        assert s['5&&24']['flagged'] == 0
        assert s['5&&25']['flagged'] == 0
        assert s['5&&26']['flagged'] == 0
        assert s['5&&27']['flagged'] == 0
        assert s['5&&28']['flagged'] == 0
        assert s['5&&5']['flagged'] == 7560 ; assert s['5&&5']['total'] == 7560


    def test_CAS2212(self):
        '''tflagdata: Clipping scan selection, CAS-2212, CAS-3496'''
        # By default correlation='ABS_ALL'
        tflagdata(vis=self.vis, mode='clip', scan="2", clipminmax = [0.2, 0.3], savepars=False) 
        test_eq(tflagdata(vis=self.vis, mode='summary'), 2854278, 85404)
        s = tflagdata(vis=self.vis, mode='summary')['scan']
        
        # Make sure no other scan is clipped
        assert s['1']['flagged'] == 0
        assert s['3']['flagged'] == 0
        assert s['4']['flagged'] == 0
        assert s['5']['flagged'] == 0
        assert s['6']['flagged'] == 0
        assert s['7']['flagged'] == 0
        assert s['2']['flagged'] == 85404
  
    def test021(self):
        '''tflagdata: Test of flagging statistics and queries'''
        
        tflagdata(vis=self.vis, correlation='LL', savepars=False)
        tflagdata(vis=self.vis, spw='0:17~19', savepars=False)
        tflagdata(vis=self.vis, antenna='5&&9', savepars=False)
        tflagdata(vis=self.vis, antenna='14', savepars=False)
        tflagdata(vis=self.vis, field='1', savepars=False)
        s = tflagdata(vis=self.vis, mode='summary', minrel=0.9, spwchan=True, basecnt=True)
        assert s['antenna'].keys() == ['14']
        assert '5&&9' in s['baseline'].keys()
        assert set(s['spw:channel'].keys()) == set(['0:17', '0:18', '0:19'])
        assert s['correlation'].keys() == ['LL']  # LL
        assert s['field'].keys() == ['1445+09900002_0']
        assert set(s['scan'].keys()) == set(['2', '4', '5', '7']) # field 1
        s = tflagdata(vis=self.vis, mode='summary', maxrel=0.8)
        assert set(s['field'].keys()) == set(['1331+30500002_0', 'N5921_2'])
        s = tflagdata(vis=self.vis, mode='summary', minabs=400000)
        assert set(s['scan'].keys()) == set(['3', '6'])
        s = tflagdata(vis=self.vis, mode='summary', minabs=400000, maxabs=450000)
        assert s['scan'].keys() == ['3']

    def test_chanavg0(self):
        print "Test of channel average"
        tflagdata(vis=self.vis, mode='clip',channelavg=False, clipminmax=[30., 60.], correlation='ABS_RR',
                 savepars=False)
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 1414186)

    def test_chanavg1(self):
        tflagdata(vis=self.vis, mode='clip',channelavg=True, clipminmax=[30., 60.], correlation='ABS_RR',
                 savepars=False)
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 1347822)

    def test_chanavg2(self):
        tflagdata(vis=self.vis, mode='clip',channelavg=False, clipminmax=[30., 60.], spw='0:0~10', 
                 correlation='ABS_RR', savepars=False)
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 242053)

    def test_chanavg3(self):
        tflagdata(vis=self.vis, mode='clip',channelavg=True, clipminmax=[30., 60.], spw='0:0~10',
                 correlation='ABS_RR', savepars=False)
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 231374)
               

#    def test8(self):
#        print "Test of mode = 'quack'"
#        print "parallel quack"
#        flagdata(vis=self.vis, mode='quack', quackinterval=[1.0, 5.0], antenna=['2', '3'], correlation='RR')
#        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 22365)
#
    def test9(self):
        '''tflagdata: quack mode'''
        tflagdata(vis=self.vis, mode='quack', quackmode='beg', quackinterval=1, savepars=False)
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 329994)

    def test10(self):
        '''tflagdata: quack mode'''
        tflagdata(vis=self.vis, mode='quack', quackmode='endb', quackinterval=1, savepars=False)
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 333396)

    def test11(self):
        '''tflagdata: quack mode'''
        tflagdata(vis=self.vis, mode='quack', quackmode='end', quackinterval=1, savepars=False)
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 2520882)

    def test12(self):
        '''tflagdata: quack mode'''
        tflagdata(vis=self.vis, mode='quack', quackmode='tail', quackinterval=1, savepars=False)
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 2524284)

    def test13(self):
        '''tflagdata: quack mode, quackincrement'''
        tflagdata(vis=self.vis, mode='quack', quackinterval=50, quackmode='endb', quackincrement=True,
                 savepars=False)
        test_eq(tflagdata(vis=self.vis, mode='summary'), 2854278, 571536)

        tflagdata(vis=self.vis, mode='quack', quackinterval=20, quackmode='endb', quackincrement=True,
                 savepars=False)
        test_eq(tflagdata(vis=self.vis, mode='summary'), 2854278, 857304)
        
        tflagdata(vis=self.vis, mode='quack', quackinterval=150, quackmode='endb', quackincrement=True,
                 savepars=False)
        test_eq(tflagdata(vis=self.vis, mode='summary'), 2854278, 1571724)
        
        tflagdata(vis=self.vis, mode='quack', quackinterval=50, quackmode='endb', quackincrement=True,
                 savepars=False)
        test_eq(tflagdata(vis=self.vis, mode='summary'), 2854278, 1762236)
        tflagdata(vis=self.vis, mode='unflag', savepars=False)



class test_selections(test_base):
    """Test various selections"""

    def setUp(self):
        self.setUp_ngc5921()

    def test_scan(self):
        '''tflagdata: scan selection and manualflag compatibility'''
        tflagdata(vis=self.vis, scan='3', mode='manualflag', savepars=False)
        res = tflagdata(vis=self.vis, mode='summary', antenna='2')
        self.assertEqual(res['flagged'],52416)
                
    def test_antenna(self):
        '''tflagdata: antenna selection'''
        tflagdata(vis=self.vis, antenna='2', savepars=False)
        test_eq(tflagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)

    def test_spw(self):
        '''tflagdata: spw selection'''
        tflagdata(vis=self.vis, spw='0', savepars=False)
        test_eq(tflagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)

    def test_spw_list(self):
        '''tflagdata: spw selection in list mode''' 
        spwfile = 'spwflags.txt'
        if os.path.exists(spwfile):
            os.system('rm -rf '+spwfile)
                   
        tflagdata(vis=self.vis, spw='0:1~10', savepars=True, outfile=spwfile)
        res0 = tflagdata(vis=self.vis, mode='summary', spwchan=True)
        self.assertEqual(res0['flagged'], 453060, 'Only channels 1~10 should be flagged')
        
        # Unflag
        tflagdata(vis=self.vis, mode='unflag')
        ures = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(ures['flagged'], 0)
        
        # Flag using the saved list
        tflagdata(vis=self.vis, mode='list', inpfile=spwfile, action='apply')
        res = tflagdata(vis=self.vis, mode='summary', spwchan=True)
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
        tflagdata(vis=self.vis, correlation='LL', savepars=False)
        test_eq(tflagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 98217)
        test_eq(tflagdata(vis=self.vis, mode='summary', correlation='RR'), 1427139, 0)
        tflagdata(vis=self.vis, mode='unflag', savepars=False)
        tflagdata(vis=self.vis, correlation='LL,RR', savepars=False)
        test_eq(tflagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)
        
        tflagdata(vis=self.vis, mode='unflag', savepars=False)
        tflagdata(vis=self.vis, mode='clip', correlation='NORM_RR,LL', clipminmax=[0.,3.])
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 204979)
#        flagdata(vis=self.vis, correlation='LL RR')
#        flagdata(vis=self.vis, correlation='LL ,, ,  ,RR')
#        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)

    def test_field(self):
        '''tflagdata: field selection'''
        tflagdata(vis=self.vis, field='0', savepars=False)
        test_eq(tflagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 39186)

    def test_uvrange(self):
        '''tflagdata: uvrange selection'''
        tflagdata(vis=self.vis, uvrange='200~400m', savepars=False)
        test_eq(tflagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 55944)

    def test_timerange(self):
        '''tflagdata: timerange selection'''
        tflagdata(vis=self.vis, timerange='09:50:00~10:20:00', savepars=False)
        test_eq(tflagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 6552)

    def test_array(self):
        '''tflagdata: array selection'''
        tflagdata(vis=self.vis, array='0', savepars=False)
        test_eq(tflagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)
        
    def test_ntime1(self):
        '''tflagdata: ntime = 0'''
        ret = tflagdata(vis=self.vis, ntime = 0, savepars=False)
        self.assertNotEqual(type(ret), dict, 'Return type of task should be None')
        
    def test_action(self):
        '''tflagdata: action = calculate'''
        tflagdata(vis=self.vis, antenna='2,3,4', action='calculate')
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 0, 'Nothing should be flagged when action=calculate')
        

class test_selections_alma(test_base):
    # Test various selections for alma data 

    def setUp(self):
        self.setUp_flagdatatest_alma()

    def test_scanitent(self):
        '''tflagdata: scanintent selection'''
        # flag POINTING CALIBRATION scans 
        # (CALIBRATE_POINTING_.. from STATE table's OBS_MODE)
        tflagdata(vis=self.vis, intent='CAL*POINT*', savepars=False)
        test_eq(tflagdata(vis=self.vis, mode='summary', antenna='2'), 377280, 26200)
        
    def test_wvr(self):
        '''tflagdata: flag WVR correlation'''
        tflagdata(vis=self.vis, correlation='I', savepars=False)
        test_eq(tflagdata(vis=self.vis, mode='summary'),1154592, 22752)

    def test_abs_wvr(self):
        '''tflagdata: clip ABS_WVR'''
        tflagdata(vis=self.vis, mode='clip',clipminmax=[0,50], correlation='ABS_WVR', savepars=False)
        test_eq(tflagdata(vis=self.vis, mode='summary'),1154592, 22752)
        
    def test_abs_i(self):
        '''tflagdata: clip ABS_I. Do not flag WVR'''
        tflagdata(vis=self.vis, mode='clip', clipminmax=[0,50], correlation='ABS_I', savepars=False)
        test_eq(tflagdata(vis=self.vis, mode='summary'),1154592, 0)

    def test_abs_all(self):
        '''tflagdata: clip ABS ALL. Do not flag WVR'''
        tflagdata(vis=self.vis, mode='clip', clipminmax=[0,1], correlation='ABS ALL', savepars=False)
        test_eq(tflagdata(vis=self.vis, mode='summary'),1154592, 130736)
        test_eq(tflagdata(vis=self.vis, mode='summary', correlation='I'),22752, 0)

    def test_spw(self):
        '''tflagdata: flag various spw'''
        # Test that a white space in the spw parameter is taken correctly
        tflagdata(vis=self.vis, mode='manual', spw='1,3, 4', savepars=False)
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['spw']['0']['flagged'], 0, 'spw=0 should not be flagged')
        self.assertEqual(res['spw']['1']['flagged'], 552960, 'spw=1 should be fully flagged')
        self.assertEqual(res['spw']['4']['flagged'], 22752, 'spw=4 should not be flagged')
        self.assertEqual(res['spw']['4']['total'], 22752, 'spw=4 should not be flagged')

class test_selections2(test_base):
    '''Test other selections'''
    
    def setUp(self):
        self.setUp_multi()
        
    def test_observation1(self):
        '''tflagdata: observation ID selections'''
        # string
        tflagdata(vis=self.vis, observation='1', savepars=False)
        test_eq(tflagdata(vis=self.vis, mode='summary'), 2882778, 28500)

        # integer
        tflagdata(vis=self.vis, mode='unflag', savepars=False)
        tflagdata(vis=self.vis, observation=1, savepars=False)
        test_eq(tflagdata(vis=self.vis, mode='summary'), 2882778, 28500)
        
        # non-existing ID
        tflagdata(vis=self.vis, mode='unflag', savepars=False)
        tflagdata(vis=self.vis, observation='10', savepars=False)
        test_eq(tflagdata(vis=self.vis, mode='summary'), 2882778, 0)

    def test_observation2(self):
        '''tflagdata: observation ID selections in list mode'''
        # creat input list
        input = "observation='0' mode='manual'"
        filename = 'obs2.txt'
        create_input(input, filename)
        
        tflagdata(vis=self.vis, mode='list', inpfile=filename, savepars=False)
        res = tflagdata(vis=self.vis, mode='summary')
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
        tflagdata(vis = self.vis, mode = 'elevation', savepars=False)
        
        test_eq(tflagdata(vis=self.vis, mode='summary'), self.all, 0)

        tflagdata(vis = self.vis, mode = 'elevation', lowerlimit = 50, savepars=False)

        test_eq(tflagdata(vis=self.vis, mode='summary'), self.all, 0)

        tflagdata(vis = self.vis, mode = 'elevation', lowerlimit = 55, savepars=False)

        test_eq(tflagdata(vis=self.vis, mode='summary'), self.all, self.x55)

        tflagdata(vis = self.vis, mode = 'elevation', lowerlimit = 60, savepars=False)

        test_eq(tflagdata(vis=self.vis, mode='summary'), self.all, self.x60)

        tflagdata(vis = self.vis, mode = 'elevation', lowerlimit = 65, savepars=False)

        test_eq(tflagdata(vis=self.vis, mode='summary'), self.all, self.x65)

    def test_upper(self):
        tflagdata(vis = self.vis, mode = 'elevation', upperlimit = 60, savepars=False)

        test_eq(tflagdata(vis=self.vis, mode='summary'), self.all, self.all - self.x60)


    def test_interval(self):
        tflagdata(vis = self.vis,
                  mode = 'elevation',
                  lowerlimit = 55,
                  upperlimit = 60,
                  savepars=False)

        test_eq(tflagdata(vis=self.vis, mode='summary'), self.all, self.all - (self.x60 - self.x55))


class test_list(test_base):
    """Test of mode = 'list'"""
    
    def setUp(self):
        self.setUp_ngc5921()

    def test_list1(self):
        '''tflagdata: apply flags from a list and do not save'''
        # creat input list
        input = "scan=1~3 mode=manual\n"+"scan=5 mode=manualflag\n"\
                "#scan='4'"
        filename = 'list1.txt'
        create_input(input, filename)
        
        # apply and don't save to MS. Ignore comment line
        tflagdata(vis=self.vis, mode='list', inpfile=filename, savepars=False, action='apply')
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['4']['flagged'], 0)
        self.assertEqual(res['flagged'], 1711206, 'Total flagged does not match')
        
    def test_list2(self):
        '''tflagdata: only save parameters without running the tool'''
        # creat input list
        input = "scan=1~3 mode=manual\n"+"scan=5 mode=manual\n"
        filename = 'list2.txt'
        create_input(input, filename)

        # save to another file
        if os.path.exists("myflags.txt"):
            os.system('rm -rf myflags.txt')
            
        tflagdata(vis=self.vis, mode='list', inpfile=filename, savepars=True, action='', outfile='myflags.txt')
        self.assertTrue(filecmp.cmp(filename, 'myflags.txt', 1), 'Files should be equal')
        
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 0, 'No flags should have been applied')
        

    def test_list3(self):
        '''tflagdata: flag and save list to FLAG_CMD'''
        # creat input list
        input = "scan=1~3 mode=manual\n"+"scan=5 mode=manual\n"
        filename = 'list3.txt'
        create_input(input, filename)

        # Delete any rows from FLAG_CMD
        flagcmd(vis=self.vis, action='clear', clearall=True)
        
        # Flag from list and save to FLAG_CMD
        tflagdata(vis=self.vis, mode='list', inpfile=filename, savepars=True)
        
        # Verify
        if os.path.exists("myflags.txt"):
            os.system('rm -rf myflags.txt')
        flagcmd(vis=self.vis, action='list', savepars=True, outfile='myflags.txt', useapplied=True)
        self.assertTrue(filecmp.cmp(filename, 'myflags.txt', 1), 'Files should be equal')
        
    def test_list4(self):
        '''tflagdata: save without running and apply in flagcmd'''
        # Delete any rows from FLAG_CMD
        flagcmd(vis=self.vis, action='clear', clearall=True)
        
        # Test that action='none' is also accepted
        tflagdata(vis=self.vis, mode='quack', quackmode='tail', quackinterval=1, action='none', 
                 savepars=True)
        
        flagcmd(vis=self.vis, action='apply')
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 2524284)

    def test_list5(self):
        '''tflagdata: clip zeros in mode=list and save reason to FLAG_CMD'''
        # get the correct data, by passing the previous setUp()
        self.setUp_data4tfcrop()
        
        # creat input list
        input = "mode='clip' clipzeros=true reason='CLIP_ZERO'"
        filename = 'list5.txt'
        create_input(input, filename)

        # Save to FLAG_CMD
        tflagdata(vis=self.vis, mode='list', inpfile=filename, action='', savepars=True)
        
        # Run in flagcmd and select by reason
        flagcmd(vis=self.vis, action='apply', reason='CLIP_ZERO')
        
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 274944, 'Should clip only spw=8')

    def test_list6(self):
        '''tflagdata: select by reason in list mode from a file'''
        # creat input list
        input = "mode='manual' scan='1' reason='SCAN_1'\n"\
                "mode='manual' scan='2'\n"\
                "scan='3' reason='SCAN_3'\n"\
                "scan='4' reason=''"
        filename = 'list6.txt'
        create_input(input, filename)
        
        # Select one reason
        tflagdata(vis=self.vis, mode='list', inpfile=filename, reason='SCAN_3')
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['3']['flagged'], 762048, 'Should flag only reason=SCAN_3')
        self.assertEqual(res['flagged'], 762048, 'Should flag only reason=SCAN_3')
        
        # Select list of reasons
        tflagdata(vis=self.vis, mode='list', inpfile=filename, reason=['','SCAN_1'])
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['4']['flagged'], 95256, 'Should flag reason=\'\'')
        self.assertEqual(res['scan']['1']['flagged'], 568134, 'Should flag reason=SCAN_1')
        
        # No reason selection
        tflagdata(vis=self.vis, mode='unflag')
        tflagdata(vis=self.vis, mode='list', inpfile=filename)
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['1']['flagged'], 568134)
        self.assertEqual(res['scan']['2']['flagged'], 238140)
        self.assertEqual(res['scan']['3']['flagged'], 762048)
        self.assertEqual(res['scan']['4']['flagged'], 95256)
        self.assertEqual(res['flagged'],568134+238140+762048+95256, 'Total flagged')
        
    def test_reason1(self):
        '''tflagdata: add_reason to FLAG_CMD'''
        flagcmd(vis=self.vis, action='clear', clearall=True)
        tflagdata(vis=self.vis, mode='manual', scan='1,3', savepars=True, cmdreason='SCAN_1_3',
                  action='')
        
        # Apply flag cmd
        flagcmd(vis=self.vis, action='apply', reason='SCAN_1_3')
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 1330182, 'Only scans 1 and 3 should be flagged')
        
    def test_reason2(self):
        '''tflagdata: add_reason to text file'''
        tflagdata(vis=self.vis, mode='clip', scan='4', clipminmax=[0, 5], savepars=True, 
                  cmdreason='CLIPSCAN4', outfile='reason2.txt', action='')

        tflagdata(vis=self.vis, mode='clip', scan='2~3', clipminmax=[ 0, 5], savepars=True, 
                  cmdreason='CLIPSCAN2_3', outfile='reason2.txt', action='')

        # Apply flag cmd
        tflagdata(vis=self.vis, mode='list', inpfile='reason2.txt',reason='CLIPSCAN2_3')
        
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 69568)
        
    def test_reason3(self):
        '''tflagdata: replace input reason with add_reason'''
        # creat input list
        input = "mode='manual' scan='1' reason='SCAN_1'\n"\
                "mode='manual' scan='2'\n"\
                "scan='3' reason='SCAN_3'\n"\
                "scan='4' reason=''"
        filename = 'input3.txt'
        create_input(input, filename)
        
        tflagdata(vis=self.vis, mode='list', inpfile=filename, savepars=True, outfile='reason3.txt',
                  cmdreason='MANUALFLAG', action='')
        
        # Apply the flag cmds
        tflagdata(vis=self.vis, mode='list', inpfile='reason3.txt', reason='MANUALFLAG')
        
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 1663578)
        
        
        
        
class test_clip(test_base):
    """tflagdata:: Test of mode = 'clip'"""
    
    def setUp(self):
        self.setUp_data4tfcrop()
        
    def test_clipzeros(self):
    	'''tflagdata: clip only zero-value data'''
        tflagdata(vis=self.vis, mode='clip', clipzeros=True)
        res = tflagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'],274944,'Should clip only spw=8')
    	
        

# Dummy class which cleans up created files
class cleanup(test_base):
    
    def tearDown(self):
        os.system('rm -rf ngc5921.ms*')
        os.system('rm -rf flagdatatest.ms*')
        os.system('rm -rf missing-baseline.ms')
        os.system('rm -rf multiobs.ms*')
        os.system('rm -rf flagdatatest-alma.ms*')
        os.system('rm -rf Four_ants_3C286.ms*')
        os.system('rm -rf shadowtest_part.ms*')
        os.system('rm -rf list*txt')

    def test1(self):
        '''tflagdata: Cleanup'''
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
            test_list,
            test_clip,
            cleanup]
