import shutil
import unittest
import os
import filecmp
import pprint
from tasks import *
from taskinit import *
from __main__ import default
import exceptions
from parallel.parallel_task_helper import ParallelTaskHelper
#from IPython.kernel.core.display_formatter import PPrintDisplayFormatter

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
if os.environ.has_key('BYPASS_PARALLEL_PROCESSING'):
    ParallelTaskHelper.bypassParallelProcessing(1)

# Local copy of the agentflagger tool
aflocal = casac.agentflagger()

# Base class which defines setUp functions
# for importing different data sets
class test_base(unittest.TestCase):
    def setUp_flagdatatest(self):
        '''VLA data set, scan=2500~2600 spw=0 1 chan, RR,LL'''
        self.vis = "flagdatatest.ms"
        if testmms:
            self.vis = "flagdatatest.mms"

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r '+datapath + self.vis +' '+ self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        
        self.unflag_ms()
        default(flagdata)

    def setUp_ngc5921(self):
        '''VLA data set, scan=1~7, spw=0 63 chans, RR,LL'''
        self.vis = "ngc5921.ms"
        if testmms:
            self.vis = 'ngc5921.mms'

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            os.system('cp -r '+datapath + self.vis +' '+ self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        flagdata(vis=self.vis, mode='unflag', flagbackup=False)
        default(flagdata)

    def setUp_alma_ms(self):
        '''ALMA MS, scan=1,8,10 spw=0~3 4,128,128,1 chans, I,XX,YY'''
        self.vis = "uid___A002_X30a93d_X43e_small.ms"
        if testmms:
            self.vis = 'uid___A002_X30a93d_X43e_small.mms'

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r '+datapath + self.vis +' '+ self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
#        self.unflag_ms()
        flagdata(vis=self.vis, mode='unflag', flagbackup=False)
        default(flagdata)

    def setUp_data4tfcrop(self):
        '''EVLA MS, 4 ants, scan=30,31 spw=0~15, 64 chans, RR,RL,LR,LL'''
        self.vis = "Four_ants_3C286.ms"
        if testmms:
            self.vis = 'Four_ants_3C286.mms'

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r '+datapath + self.vis +' '+ self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        self.unflag_ms()
        default(flagdata)

    def setUp_shadowdata2(self):
        '''CASA simulation data set. scan=0 spw=0, 2 chans, RR,LL'''
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
        flagdata(vis=self.vis, mode='unflag', flagbackup=False)
        
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
        self.unflag_ms()
        default(flagdata)
        
    def setUp_mwa(self):
        '''MWA data set, scan=1 spw=0, 11 chans, XX,XY,YX,YY'''
        self.vis = "testmwa.ms"
        if testmms:
            self.vis = 'testmwa.mms'

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r '+datapath + self.vis +' '+ self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        self.unflag_ms()
        default(flagdata)

    def setUp_wtspec(self):
        # Four rows, 2 ants, 1 spw, 31 chans, 2 pols, WEIGHT_SPECTRUM col
        self.vis = "four_rows_weight_spectrum.ms"
        if testmms:
            self.vis = 'four_rows_weight_spectrum.mms'

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r '+datapath + self.vis +' '+ self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        self.unflag_ms()
        default(flagdata)

    def setUp_floatcol(self):
        # 15 rows, 3 scans, 9 spw, mixed chans, XX,YY, FLOAT_DATA col
        self.vis = "SDFloatColumn.ms"
        if testmms:
            self.vis = 'SDFloatColumn.mms'

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r '+datapath + self.vis +' '+ self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        self.unflag_ms()
        default(flagdata)

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
        self.unflag_ms()
        default(flagdata)

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
        default(flagdata)

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
        self.unflag_ms()        
        default(flagdata)
        
    def setUp_weightcol(self):
        '''Small MS with two rows and WEIGHT column'''
        datapath = os.environ.get('CASAPATH').split()[0] + "/data/regression/unittest/mstransform/"

        inpvis = "combine-1-timestamp-2-SPW-no-WEIGHT_SPECTRUM-Same-Exposure.ms"
        self.vis = "msweight.ms"
        
        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r ' + datapath + inpvis + ' ' + self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        self.unflag_ms()        
        default(flagdata)
        
    def setUp_tbuff(self):
        '''Small ALMA MS with low-amp points to be flagged with tbuff parameter'''
        datapath = os.environ.get('CASAPATH').split()[0] + "/data/regression/unittest/flagdata/"
        
        self.vis = 'uid___A002_X72c4aa_X8f5_scan21_spw18_field2_corrXX.ms'
        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r ' + datapath + self.vis + ' ' + self.vis)

        # Copy the online flags file
        self.online = 'uid___A002_X72c4aa_X8f5_online.txt'
        self.user = 'uid___A002_X72c4aa_X8f5_user.txt'
        os.system('cp -r ' + datapath + self.online + ' ' + self.online)
        os.system('cp -r ' + datapath + self.user + ' ' + self.user)
        
        os.system('rm -rf ' + self.vis + '.flagversions')
        self.unflag_ms()        
        default(flagdata)
        
    def unflag_ms(self):
        aflocal.open(self.vis)
        aflocal.selectdata()
        agentUnflag={'apply':True,'mode':'unflag'}
        aflocal.parseagentparameters(agentUnflag)
        aflocal.init()
        aflocal.run(writeflags=True)
        aflocal.done()
        
    def extract_reports(self, report_list):        
        summary_list = []
        
        # Extract only the type 'summary' reports
        nreps = report_list.keys()
        for rep in range(len(nreps)):
            repname = 'report'+str(rep);
            if(report_list[repname]['type']=='summary'):
                  summary_list.append(report_list[repname]);
                  
        return summary_list

class test_tfcrop(test_base):
    """flagdata:: Test of mode = 'tfcrop'"""
    
    def setUp(self):
        self.setUp_data4tfcrop()
        
    def test_tfcrop1(self):
        '''flagdata:: Test1 of mode = tfcrop'''
        flagdata(vis=self.vis, mode='tfcrop', correlation='ABS_RR',ntime=51.0,spw='9', 
                 savepars=False, extendflags=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 4489)
        self.assertEqual(res['antenna']['ea19']['flagged'], 2294)
        self.assertEqual(res['spw']['7']['flagged'], 0)
        
    def test_tfcrop2(self):
        '''flagdata:: Test2 of mode = tfcrop ABS_ALL'''
        # Note : With ntime=51.0, 64-bit machines get 18696 flags, and 32-bit gets 18695 flags.
        #           As far as we can determine, this is a genuine precision-related difference.
        #           With ntime=53.0, there happens to be no difference.
        flagdata(vis=self.vis, mode='tfcrop',ntime=53.0,spw='9', savepars=False,
                 extendflags=False)
        res = flagdata(vis=self.vis, mode='summary', spw='9')
        self.assertEqual(res['flagged'], 18671)
        self.assertEqual(res['correlation']['LL']['flagged'], 4250)
        self.assertEqual(res['correlation']['RL']['flagged'], 5007)
        self.assertEqual(res['correlation']['LR']['flagged'], 4931)
        self.assertEqual(res['correlation']['RR']['flagged'], 4483)

    # Remove this test once Scott fixes Jenkins!!!
    def test2(self):
        '''flagdata:: Test2 of mode = tfcrop ABS_ALL'''
        # Note : With ntime=51.0, 64-bit machines get 18696 flags, and 32-bit gets 18695 flags.
        #           As far as we can determine, this is a genuine precision-related difference.
        #           With ntime=53.0, there happens to be no difference.
        flagdata(vis=self.vis, mode='tfcrop',ntime=53.0,spw='9', savepars=False,
                 extendflags=False)
        res = flagdata(vis=self.vis, mode='summary', spw='9')
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
        flagdata(vis=self.vis, mode='tfcrop', extendflags=False)
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
        
    def test_tfcrop_extendflags(self):
        '''flagdata: mode tfcrop with extendflags=True'''
        # First, extend the flags manually
        flagdata(vis=self.vis, mode='tfcrop', extendflags=False, flagbackup=False)
        flagdata(vis=self.vis, mode='extend', flagbackup=False,
                 extendpols=True, growtime=50.0, growfreq=80.0)        
        pre = flagdata(vis=self.vis, mode='summary', spw='0')
        self.assertEqual(pre['spw']['0']['flagged'], 27768)
        self.assertEqual(pre['spw']['0']['total'], 274944)
        
#        flagdata(vis=self.vis, mode='unflag', flagbackup=False)
        self.unflag_ms()
        
        # Now, extend the flags automatically and compare
        flagdata(vis=self.vis, mode='tfcrop', spw='0', extendflags=True, flagbackup=False)
        pos = flagdata(vis=self.vis, mode='summary', spw='0')
        
        # Flags should be extended in time if > 50%, freq > 80% and
        # will extend to the other polarizations too.
        self.assertEqual(pos['spw']['0']['flagged'], pre['spw']['0']['flagged'])
        

class test_rflag(test_base):
    """flagdata:: Test of mode = 'rflag'"""
    
    def setUp(self):
        self.setUp_data4tfcrop()
        
    def test_rflag1(self):
        '''flagdata:: mode = rflag : automatic thresholds'''
        flagdata(vis=self.vis, mode='rflag', spw='9,10', timedev=[], freqdev=[], flagbackup=False,
                 extendflags=False)
        res = flagdata(vis=self.vis, mode='summary',spw='7,9,10')
        self.assertEqual(res['flagged'], 42728.0)
        self.assertEqual(res['antenna']['ea19']['flagged'], 18411.0)
        self.assertEqual(res['spw']['7']['flagged'], 0)

    def test_rflag2(self):
        '''flagdata:: mode = rflag : partially-specified thresholds'''
        flagdata(vis=self.vis, mode='rflag', spw='9,10', timedev=[[1,10,0.1],[1,11,0.07]], \
                       freqdev=0.5, flagbackup=False, extendflags=False)
        res = flagdata(vis=self.vis, mode='summary',spw='9,10,11')
        self.assertEqual(res['flagged'], 52411)
        self.assertEqual(res['antenna']['ea19']['flagged'], 24142)
        self.assertEqual(res['spw']['11']['flagged'], 0)
        
    def test_rflag_numpy_types(self):
        '''flagdata:: mode = rflag : partially-specified thresholds using numpy types'''
        # Results should be the same as in test_rflag2 above
        import numpy as np
        t1 = [np.int32(1), 10, np.float32(0.1)]
        t2 = [1, np.int16(11), np.float64(0.07)]

        flagdata(vis=self.vis, mode='rflag', spw='9,10', timedev=[t1,t2], \
                       freqdev=0.5, flagbackup=False, extendflags=False)
        res = flagdata(vis=self.vis, mode='summary',spw='9,10,11')
        self.assertEqual(res['flagged'], 52411)
        self.assertEqual(res['antenna']['ea19']['flagged'], 24142)
        self.assertEqual(res['spw']['11']['flagged'], 0)

    def test_rflag3(self):
        '''flagdata:: mode = rflag : output/input via two methods'''
        # (1) Test input/output files, through the task, mode='rflag'
        # Files tdevfile.txt and fdevfile.txt are created in this step
        flagdata(vis=self.vis, mode='rflag', spw='9,10', timedev='tdevfile.txt', 
                      freqdev='fdevfile.txt', action='calculate', extendflags=False)
        self.assertTrue(os.path.exists('tdevfile.txt'))
        self.assertTrue(os.path.exists('fdevfile.txt'))
        
        flagdata(vis=self.vis, mode='rflag', spw='9,10', timedev='tdevfile.txt', 
                      freqdev='fdevfile.txt', action='apply', flagbackup=False, 
                      extendflags=False)
        res1 = flagdata(vis=self.vis, mode='summary', spw='9,10')
        # (2) Test rflag output written to cmd file via mode='rflag' and 'savepars' 
        #      and then read back in via list mode. 
        #      Also test the 'savepars' when timedev and freqdev are specified differently...
#        flagdata(vis=self.vis,mode='unflag', flagbackup=False)
        self.unflag_ms()
        flagdata(vis=self.vis, mode='rflag', spw='9,10', timedev='', \
                      freqdev=[],action='calculate',savepars=True,outfile='outcmd.txt',
                      extendflags=False);
        flagdata(vis=self.vis, mode='list', inpfile='outcmd.txt', flagbackup=False)
        res2 = flagdata(vis=self.vis, mode='summary', spw='9,10')

        #print res1['flagged'], res2['flagged']
        self.assertTrue(abs(res1['flagged']-res2['flagged'])<10000)
        self.assertTrue(abs(res1['flagged']-39504.0)<10000)

    def test_rflag4(self):
        '''flagdata:: mode = rflag : correlation selection'''
        flagdata(vis=self.vis, mode='rflag', spw='9,10', correlation='rr,ll', flagbackup=False,
                 extendflags=False)
        res = flagdata(vis=self.vis, mode='summary',spw='9,10')
        self.assertEqual(res['correlation']['RR']['flagged'], 9781.0)
        self.assertEqual(res['correlation']['LL']['flagged'], 10355.0)
        self.assertEqual(res['correlation']['LR']['flagged'], 0,)
        self.assertEqual(res['correlation']['RL']['flagged'], 0,)
        
    def test_rflag_CAS_5037(self):
        '''flagdata:: Use provided value for time stats, but automatically computed value for freq. stats'''
        flagdata(vis=self.vis, mode='rflag', field = '1', spw='10', timedev=0.1, \
                 timedevscale=5.0, freqdevscale=5.0, action='calculate', flagbackup=False)

    def test_rflag_extendflags(self):
        '''flagdata: automatically extend the flags after rflag'''    
        # Manually extend the flags    
        flagdata(vis=self.vis, mode='rflag', spw='9,10', flagbackup=False,
                 extendflags=False)
        flagdata(vis=self.vis, mode='extend', growtime=50.0, growfreq=80.0,
                 extendpols=True, flagbackup=False)
        pre = flagdata(vis=self.vis, mode='summary', spw='9,10')

#        flagdata(vis=self.vis, mode='unflag', flagbackup=False)
        self.unflag_ms()
        
        # Automatically extend the flags by default
        flagdata(vis=self.vis, mode='rflag', spw='9,10', flagbackup=False)
        pos = flagdata(vis=self.vis, mode='summary', spw='9,10')
        self.assertEqual(pos['spw']['9']['flagged'], pre['spw']['9']['flagged'])
        self.assertEqual(pos['spw']['10']['flagged'], pre['spw']['10']['flagged'])
        
    def test_rflag_extendflags2(self):
        '''flagdata: in list mode extend the flags automatically after rflag'''
        def getcounts():
            ### Channel 51 should extend flags, but channel 52 should not.
            counts = flagdata(vis=self.vis, mode='summary',spw='4',antenna='ea01&&ea11',
                              scan='30',correlation='LL',spwchan=True)
            chan51 = counts['spw:channel']['4:51']
            chan52 = counts['spw:channel']['4:52']
             
            counts = flagdata(vis=self.vis, mode='summary',spw='4',antenna='ea01&&ea11',
                              scan='30',correlation='RL',spwchan=True)
        
            chan51rl=counts['spw:channel']['4:51']
                        
            return chan51, chan52, chan51rl

        # do not extend the flags
        cmdlist = ["mode='rflag' spw='4' freqdevscale=4.0 extendflags=False"]
        flagdata(vis=self.vis, mode='list', inpfile=cmdlist, flagbackup=False)
        chan51, chan52, chan51rl = getcounts()

        # Unflag
        flagdata(vis=self.vis, mode='unflag', spw='4', flagbackup=False)

        # automatically extend the flags
        cmdlist = ["mode='rflag' spw='4' freqdevscale=4.0"]
        flagdata(vis=self.vis, mode='list', inpfile=cmdlist, flagbackup=False)
        achan51, achan52, achan51rl = getcounts()
        
        if chan51['flagged']/chan51['total']>0.5 and achan51['flagged']/achan51['total']==1.0 :
            print 'Channel 51 had more than 50% and got extended. PASS'
        else:
            self.fail('Channel 51 failed')

        if chan52['flagged']/chan52['total']<50.0 and achan52['flagged']/achan52['total']==chan52['flagged']/chan52['total']:
            print 'Channel 52 had less than 50% and did not get extended. PASS'
        else:
            self.fail('Channel 52 failed') 

        if chan51rl['flagged']/chan51rl['total']<0.5 and achan51rl['flagged']/achan51rl['total']==1.0:
            print 'Channel 51 in RL had less than 50% but got completely flagged because Channel 51 in LL got extended. PASS'
        else:
            self.fail('Channel 51 extendpols failed') 

    def test_rflag_summary_list(self):
        '''flagdata: rflag and summaries in list mode'''
        fcmd = ["mode='summary' spw='7,9,10' name='InitFlags'",
                "mode='rflag' spw='9,10' timedev=[] freqdev=[] extendflags=False",
                "mode='summary' spw='7,9,10' name='RflagFlags'"]
        
        res = flagdata(vis=self.vis, mode='list', inpfile=fcmd, flagbackup=False)
        self.assertEqual(res['report0']['flagged'],0)
        self.assertEqual(res['report1']['flagged'], 42728)
        self.assertEqual(res['report1']['antenna']['ea19']['flagged'], 18411)
        self.assertEqual(res['report1']['spw']['7']['flagged'], 0,)
        
        
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
        
        flagmanager(vis=self.vis, mode='list')
        aflocal.open(self.vis)
        self.assertEqual(len(aflocal.getflagversionlist()), 2)
        aflocal.done()


        flagdata(vis=self.vis, mode='unflag', flagbackup=False)
        flagmanager(vis=self.vis, mode='list')
        aflocal.open(self.vis)
        self.assertEqual(len(aflocal.getflagversionlist()), 2)
        aflocal.done()

        flagdata(vis=self.vis, mode='unflag', flagbackup=True)
        flagmanager(vis=self.vis, mode='list')
        aflocal.open(self.vis)
        self.assertEqual(len(aflocal.getflagversionlist()), 3)
        aflocal.done()
        
        newname = 'Ha! The best version ever!'

        flagmanager(vis=self.vis, mode='rename', oldname='flagdata_1', versionname=newname, 
                    comment='This is a *much* better name')
        flagmanager(vis=self.vis, mode='list')
        aflocal.open(self.vis)
        self.assertEqual(len(aflocal.getflagversionlist()), 3)
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

        flagmanager(vis=self.vis, mode='restore', versionname='flagdata_2')
        restore2 = flagdata(vis=self.vis, mode='summary')['flagged']

        print "After restoring pre-antenna 2 flagging, there are", restore2, "flags; should be", ant2

        self.assertEqual(restore2, ant2)

    def test_CAS2701(self):
        """flagmanager: Do not allow flagversion=''"""
                
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
        myinput = "scan='1' mode='manual' autocorr=True reason='AUTO'\n"\
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
#        flagdata(vis=self.vis, mode='unflag', savepars=False, flagbackup=False)


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
#        flagdata(vis=self.vis, mode='unflag', flagbackup=False)
        self.unflag_ms()
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
#        flagdata(vis=self.vis, mode='unflag', savepars=False, flagbackup=False)
        self.unflag_ms()
        flagdata(vis=self.vis, correlation='LL,RR', savepars=False, flagbackup=False)
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)
        
#        flagdata(vis=self.vis, mode='unflag', savepars=False, flagbackup=False)
        self.unflag_ms()
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
#        flagdata(vis=self.vis, mode='unflag', savepars=False, flagbackup=False)
        self.unflag_ms()
        flagdata(vis=self.vis, correlation='LL,RR,RL', savepars=False, flagbackup=False)
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)
        
    def test_multi_timerange(self):
        '''flagdata: CAS-5300, in list mode, flag multiple timerange intervals'''
        inpcmd = ["timerange='09:26:00~09:30:00,09:42:00~09:43:00,10:33:00~10:50:00'"]
        flagdata(vis=self.vis, mode='list', inpfile=inpcmd, flagbackup=False)
        
        # Should flag scan=2, scan=3 and scan=6,7
        res = flagdata(vis=self.vis, mode='summary', scan='2,3,6,7')
        self.assertEqual(res['scan']['2']['flagged'], 238140)
        self.assertEqual(res['scan']['3']['flagged'], 47628)
        self.assertEqual(res['scan']['6']['flagged'], 476280)
        self.assertEqual(res['scan']['7']['flagged'], 190512)
        self.assertEqual(res['flagged'], 238140+47628+476280+190512)


class test_alma(test_base):
    # Test various flagging on alma data 

    def setUp(self):
        self.setUp_alma_ms()

    def test_scanitent(self):
        '''flagdata: scanintent selection'''
        # flag POINTING CALIBRATION scans 
        # (CALIBRATE_POINTING_.. from STATE table's OBS_MODE)
        flagdata(vis=self.vis, intent='CAL*POINT*', savepars=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['1']['flagged'], 192416.0)
        
    def test_wvr(self):
        '''flagdata: flag WVR correlation'''
        flagdata(vis=self.vis, correlation='I', savepars=False, flagbackup=False)
        test_eq(flagdata(vis=self.vis, mode='summary', spw='0'),608, 608)

    def test_abs_wvr(self):
        '''flagdata: clip ABS_WVR'''
        flagdata(vis=self.vis, mode='clip',clipminmax=[0,50], correlation='ABS_WVR', savepars=False,
                 flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary', spw='0')
        self.assertEqual(res['spw']['0']['flagged'], 498)
        self.assertEqual(res['flagged'], 498)
        self.assertEqual(res['correlation']['I']['flagged'], 498)

    def test_abs_i(self):
        '''flagdata: clip ABS_I. Do not flag WVR'''
        flagdata(vis=self.vis, mode='clip', clipminmax=[0,50], correlation='ABS_I', savepars=False,
                 flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary', spw='0')
        self.assertEqual(res['spw']['0']['flagged'], 0)
        self.assertEqual(res['flagged'], 0)
        self.assertEqual(res['correlation']['I']['flagged'], 0)

    def test_abs_all(self):
        '''flagdata: clip ABS ALL. Do not flag WVR'''
        flagdata(vis=self.vis, mode='clip', clipminmax=[0,1], correlation='ABS ALL', savepars=False,
                 flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['spw']['0']['flagged'], 0)
        self.assertEqual(res['flagged'], 1851)
        self.assertEqual(res['correlation']['I']['flagged'], 0)
        self.assertEqual(res['correlation']['XX']['flagged'], 568)
        self.assertEqual(res['correlation']['YY']['flagged'], 1283)

    def test_alma_spw(self):
        '''flagdata: flag various spw'''
        # Test that a white space in the spw parameter is taken correctly
        flagdata(vis=self.vis, mode='manual', spw='1,2, 3', savepars=False,
                 flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['spw']['0']['flagged'], 0, 'spw=0 should not be flagged')
        self.assertEqual(res['spw']['1']['flagged'], 192000, 'spw=1 should be fully flagged')
        self.assertEqual(res['spw']['3']['flagged'], 1200, 'spw=3 should be flagged')
        self.assertEqual(res['spw']['3']['total'], 1200, 'spw=3 should be flagged')
        
    def test_null_intent_selection1(self):
        '''flagdata: handle unknown scan intent in list mode'''
        
        myinput = ["intent='FOCUS'",   # non-existing intent
                 "intent='CALIBRATE_POINTING*'", # scan=1
                 "intent='*DELAY*'"] # non-existing
       
        flagdata(vis=self.vis, mode='list', inpfile=myinput, flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary',scan='1')
        self.assertEqual(res['flagged'], 192416)
        self.assertEqual(res['total'], 192416)
        
    def test_unknown_intent(self):
        '''flagdata: CAS-3712, handle unknown value in intent expression'''
        flagdata(vis=self.vis,intent='*POINTING*,*FOCUS*',flagbackup=False)
        
        # Only POINTING scan exists. *FOCUS* should not raise a NULL MS selection
        res = flagdata(vis=self.vis, mode='summary', scan='1')
        self.assertEqual(res['flagged'], 192416)
        self.assertEqual(res['total'], 192416)
        
    def test_autocorr_wvr(self):
        '''flagdata: CAS-5286, do not flag auto-correlations in WVR data'''
        flagdata(vis=self.vis,autocorr=True,flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary', spw='0,1')
        
        # spw='0' contains the WVR data
        self.assertEqual(res['spw']['1']['flagged'], 15360)
        self.assertEqual(res['spw']['0']['flagged'], 0)
        self.assertEqual(res['flagged'], 15360)

    def test_autocorr_wvr_list(self):
        '''flagdata: CAS-5485 flag autocorrs in list mode'''
        mycmd = ["mode='manual' antenna='DA41'",
                 "mode='manual' autocorr=True"]
        
        # The first cmd only flags cross-correlations of DV41
        # The second cmd only flags auto-corrs of all antennas
        # that have processor type=CORRELATOR. The radiometer
        # data should not be flagged, which is in spw=0
        res = flagdata(vis=self.vis, mode='list', inpfile=mycmd, flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary',basecnt=True)
        
        # These are the auto-correlations not flagged in the list flagging.
        # Verify that the non-flagged points are those from the WVR data
        wvr1 = res['baseline']['DA41&&DA41']['total'] - res['baseline']['DA41&&DA41']['flagged']
        wvr2 = res['baseline']['DA42&&DA42']['total'] - res['baseline']['DA42&&DA42']['flagged']
        wvr3 = res['baseline']['DV02&&DV02']['total'] - res['baseline']['DV02&&DV02']['flagged']
        wvr4 = res['baseline']['PM03&&PM03']['total'] - res['baseline']['PM03&&PM03']['flagged']
        wvrspw= res['spw']['0']['total']
        
        self.assertEqual(wvrspw, wvr1+wvr2+wvr3+wvr4, 'Auto-corr of WVR data should not be flagged')
        self.assertEqual(res['antenna']['DA41']['flagged'],75600)
        self.assertEqual(res['antenna']['DA41']['total'],75752)
        self.assertEqual(res['spw']['0']['flagged'], 0)

class test_selections2(test_base):
    '''Test other selections'''
    
    def setUp(self):
        self.setUp_multi()
        
    def test_observation1(self):
        '''flagdata: observation ID selections'''
        # string
        flagdata(vis=self.vis, observation='1', savepars=False, flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary',observation='1')
        self.assertEqual(res['flagged'], 28500)
        self.assertEqual(res['total'], 28500)

        # integer
#        flagdata(vis=self.vis, mode='unflag', savepars=False)
        self.unflag_ms()
        flagdata(vis=self.vis, observation=1, savepars=False)
        res = flagdata(vis=self.vis, mode='summary',observation='1')
        self.assertEqual(res['flagged'], 28500)
        self.assertEqual(res['total'], 28500)
        
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
        
    def tearDown(self):
        os.system('rm -rf list*.txt list*.tmp')

    def test_file1(self):
        '''flagdata: apply flags from a list and do not save'''
        # creat input list
        myinput = "scan='1~3' mode='manual'\n"+"scan='5' mode='manualflag'\n"\
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
        myinput = "scan='1~3' mode='manual'\n"+"scan='5' mode='manual'\n"
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
        myinput = "scan='1~3' mode='manual'\n"+"scan='5' mode='manual'\n"
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
        res = flagdata(vis=self.vis, mode='summary', scan='3')
        self.assertEqual(res['scan']['3']['flagged'], 762048, 'Should flag only reason=SCAN_3')
        self.assertEqual(res['flagged'], 762048, 'Should flag only reason=SCAN_3')
        
        # Select list of reasons
        flagdata(vis=self.vis, mode='list', inpfile=filename, reason=['','SCAN_1'],
                 flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary',scan='1,4')
        self.assertEqual(res['scan']['4']['flagged'], 95256, 'Should flag reason=\'\'')
        self.assertEqual(res['scan']['1']['flagged'], 568134, 'Should flag reason=SCAN_1')
        
        # No reason selection
#        flagdata(vis=self.vis, mode='unflag', flagbackup=False)
        self.unflag_ms()
        flagdata(vis=self.vis, mode='list', inpfile=filename, flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary', scan='1~4')
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
                "# a comment line\n"\
                "scan='3'"
        filename1 = 'list7a.txt'
        create_input(myinput, filename1)
        
        # Create second input file
        myinput = "scan='5'\n"\
                  " \n"\
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
#        flagdata(vis=self.vis, mode='unflag')
        self.unflag_ms()
        flagdata(vis=self.vis, mode='list', inpfile=[filename1,filename2], reason='NONE',
                 flagbackup=False)
        
        res = flagdata(vis=self.vis, mode='summary', scan='1')
        self.assertEqual(res['scan']['1']['flagged'], 99198)
        self.assertEqual(res['flagged'], 99198)

    def test_anychar(self):
        '''flagdata: Do not continue if parameter doesn't exist'''
        myinput = "mode=manualflag field='Any $3[=character (}'"
        filename = 'anychar.txt'
        create_input(myinput, filename)
        
        outname = 'wrongpar.txt'
        
        # CAS-6704: should raise an exception because parameter $3[ doesn't exist in flagdata
        try:
            flagdata(vis=self.vis, mode='list', inpfile=filename, action='', savepars=True,
                  outfile=outname)
        except Exception, instance:
            print 'Expected IOError error: %s'%instance
        
        # It should fail above and not create an output file
        self.assertFalse(os.path.exists(outname))

    def test_file_summary1(self):
        '''flagdata: summary commands in list mode'''
        myinput = "mode='manual' scan='2'\n"\
                  "mode='summary' name='Scan2'\n"\
                  "mode='clip' clipzeros=True\n"\
                  "mode='summary' name='Zeros'"
        filename = 'listsumm1.txt'
        create_input(myinput, filename)
        summary_stats_list = flagdata(vis=self.vis, mode='list', inpfile=filename, flagbackup=False)

        # Extract only the type 'summary' reports into a list
        summary_reps = self.extract_reports(summary_stats_list)
 
        for ind in range(0,len(summary_reps)):        
            flagcount = summary_reps[ind]['flagged'];
            totalcount = summary_reps[ind]['total'];         
            # From the second summary onwards, subtract counts from the previous one :)
            if ( ind > 0 ):
                 flagcount = flagcount - summary_reps[ind-1]['flagged'];
         
            print "Summary ", ind , "(" , summary_reps[ind]['name']  , ") :  Flagged : " , flagcount , " out of " , totalcount ;   
         
        self.assertEqual(summary_reps[0]['flagged'],238140, 'Should show only scan=2 flagged')    
        self.assertEqual(summary_reps[1]['flagged']-summary_reps[0]['flagged'],0, 'Should not flag any zeros')    
 
    def test_file_summary2(self):
        '''flagdata: compare summaries from a list with individual reports'''
        myinput = ["scan='1~3' mode='manual'",
                   "mode='summary' name='SCANS123'",
                   "scan='5' mode='manualflag'",
                   "#scan='4'",
                   "mode='summary' name='SCAN5'"]
         
        summary_stats_list = flagdata(vis=self.vis, mode='list', inpfile=myinput, flagbackup=False)
        summary_reps = self.extract_reports(summary_stats_list)
         
        # Unflag and flag scan=1~3
        self.unflag_ms()
        flagdata(vis=self.vis, scan='1~3', flagbackup=False)
        rscan123 = flagdata(vis=self.vis, mode='summary')
        # Unflag and flag scan=5
        self.unflag_ms()
        flagdata(vis=self.vis, scan='5', flagbackup=False)
        rscan5 = flagdata(vis=self.vis, mode='summary')
         
        # Compare
        self.assertEqual(summary_reps[0]['flagged'],rscan123['flagged'], 'scan=1~3 should be flagged')
        self.assertEqual(summary_reps[1]['flagged'],rscan5['flagged']+rscan123['flagged'],\
                         'scan=1~3,5 should be flagged')
        self.assertEqual(summary_reps[1]['flagged']-summary_reps[0]['flagged'],rscan5['flagged'],\
                         'scan=5 should be flagged')
        
    def test_file_scan_int(self):
        '''flagdata: select a scan by giving an int value'''
        # The new fh.parseFlagParameters should allow this
        myinput = "mode='manual' scan=1\n"\
                  "scan='2'\n"\
                  "mode='summary'"
        filename = 'intscan.txt'
        create_input(myinput, filename)
        
        try:
            res = flagdata(vis=self.vis, mode='list', inpfile=filename, flagbackup=False)
            
        except exceptions.IOError, instance:
            print 'Expected error!'
        
    def test_file_scan_list(self):
        '''flagdata: select a scan by giving a list value. Expect error.'''
        # The new fh.parseFlagParameters should NOT allow this
        myinput = "scan='1' mode='manual'\n"\
                   "scan=[2]\n"\
                   "mode='summary'"
        
        filename = 'listscan.txt'
        create_input(myinput, filename)
        try:
            res = flagdata(vis=self.vis, mode='list', inpfile=filename, flagbackup=False)
            
        except exceptions.IOError, instance:
            print 'Expected error!'
               
        

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
#        flagdata(vis=self.vis, mode='unflag',flagbackup=False)
        self.unflag_ms()
        flagcmd(vis=self.vis, inpmode='list', inpfile=myinput)
        res2 = flagdata(vis=self.vis, mode='summary')

        # Verify
        self.assertEqual(res1['flagged'], res2['flagged'])
        self.assertEqual(res1['total'], res2['total'])

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
#        flagdata(vis=self.vis, mode='unflag',flagbackup=False)
        self.unflag_ms()
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
              
    # The new parser allows whitespaces in reason values. Change the test
    def test_cmdreason1(self):
        '''flagdata: allow whitespace in cmdreason'''
        outtxt = 'spacereason.txt'
        flagdata(vis=self.vis, scan='1,3', action='calculate', savepars=True, outfile=outtxt, 
                 cmdreason='ODD SCANS')
        flagdata(vis=self.vis, scan='2,4', action='calculate', savepars=True, outfile=outtxt, 
                 cmdreason='EVEN SCANS')
        
        # Apply the cmd with blanks in reason.
        flagdata(vis=self.vis, mode='list', inpfile=outtxt, reason='ODD SCANS')
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['1']['flagged'], 568134)
        self.assertEqual(res['scan']['3']['flagged'], 762048)
        self.assertEqual(res['flagged'], 568134+762048)
    
    def test_cmdreason2(self):
        '''flagdata: Blanks in reason are also allowed in FLAG_CMD table'''
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
        
    def test_list_summary1(self):
        '''flagdata: check names of multiple summaries in a list'''
        myinput = ["scan='3' mode='manual'",
                   "mode='summary' name='SCAN_3'",
                   "scan='15' mode='manualflag'",  # provoke an error
                   "#scan='4'",
                   "mode='summary' name='SCAN15'"]
         
        summary_stats_list = flagdata(vis=self.vis, mode='list', inpfile=myinput, flagbackup=False)
        summary_reps = self.extract_reports(summary_stats_list)
                  
        self.assertEqual(summary_reps[0]['scan']['3']['flagged'],
                         summary_reps[0]['scan']['3']['total'])
        self.assertEqual(summary_reps[0]['name'],'SCAN_3')
        self.assertEqual(summary_reps[1]['name'],'SCAN15')
        
    
class test_clip(test_base):
    """flagdata:: Test of mode = 'clip'"""
    
    def setUp(self):
        self.setUp_data4tfcrop()
        
    def test_clipzeros(self):
    	'''flagdata: clip only zero-value data'''
        flagdata(vis=self.vis, mode='clip', clipzeros=True, flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary', spw='8')
        self.assertEqual(res['flagged'],274944,'Should clip only spw=8')

    def test_clip_list1(self):
        '''flagdata: clip zeros in mode=list and save reason to FLAG_CMD'''
        
        # creat input list
        myinput = ["mode='clip' clipzeros=True reason='CLIP_ZERO'"]

        # Save to FLAG_CMD
        flagdata(vis=self.vis, mode='list', inpfile=myinput, action='', savepars=True)
        
        # Run in flagcmd and select by reason
        flagcmd(vis=self.vis, action='apply', reason='CLIP_ZERO')
        
        res = flagdata(vis=self.vis, mode='summary', spw='8')
        self.assertEqual(res['flagged'], 274944, 'Should clip only spw=8')

    def test_clip_file1(self):
        '''flagdata: clip zeros in mode=list and save reason to FLAG_CMD'''
        
        # creat input list
        myinput = "mode='clip' clipzeros=True reason='CLIP_ZERO'"
        filename = 'list5.txt'
        create_input(myinput, filename)

        # Save to FLAG_CMD
        flagdata(vis=self.vis, mode='list', inpfile=filename, action='', savepars=True)
        
        # Run in flagcmd and select by reason
        flagcmd(vis=self.vis, action='apply', reason='CLIP_ZERO')
        
        res = flagdata(vis=self.vis, mode='summary', spw='8')
        self.assertEqual(res['flagged'], 274944, 'Should clip only spw=8')
        self.assertEqual(res['total'], 274944)
        
    def test_invalid_antenna(self):
        '''flagdata: CAS-3712, handle good and bad antenna names in MS selection'''
        
        flagdata(vis=self.vis, antenna='ea01,ea93', mode='manual', flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary', antenna='ea01', basecnt=True)
        self.assertEqual(res['flagged'], 2199552)
        self.assertEqual(res['total'], 2199552)

    def test_datacol_corrected(self):
        ''''flagdata: clip CORRECTED data column'''
        flagdata(vis=self.vis, flagbackup=False, mode='clip', datacolumn='CORRECTED',
                 clipminmax=[0.,10.])
        # only corrected column has amplitude above 10.0
        res = flagdata(vis=self.vis, mode='summary', spw='5,9,10,11')
        self.assertEqual(res['flagged'], 1142)
        
        # Make sure the corrected data was used, not the default data column
        self.unflag_ms()
        flagdata(vis=self.vis, flagbackup=False, mode='clip',
                 clipminmax=[0.,10.])
        
        # should not flag anything
        res = flagdata(vis=self.vis, mode='summary', spw='5,9,10,11')
        self.assertEqual(res['flagged'], 0)
                

class test_CASA_4_0_bug_fix(test_base):
    """flagdata:: Regression test for the fixes introduced during the CASA 4.0 bug fix season"""

    def setUp(self):
        self.setUp_data4tfcrop()
        
    def test_CAS_4270(self):
        """flagdata: Test uvrange given in lambda units"""
                
        flagdata(vis=self.vis,mode='manual',uvrange='<2klambda')
        flagdata(vis=self.vis,mode='clip', flagbackup=False)
        summary_ref = flagdata(vis=self.vis,mode='summary', spw='2,3')
        
#        flagdata(vis=self.vis,mode='unflag', flagbackup=False)
        self.unflag_ms()
        flagdata(vis=self.vis,mode='list',inpfile=["uvrange='<2Klambda'","mode='clip'"],
                 flagbackup=False)
        summary_out = flagdata(vis=self.vis,mode='summary', spw='2,3')
        
        self.assertEqual(summary_out['flagged'],summary_ref['flagged'],'uvrange given in lambda is not properly translated into meters')
        
    def test_CAS_4312(self):
        """flagdata: Test channel selection with Rflag agent"""
        
        flagdata(vis=self.vis,mode='rflag',spw='9:10~20', extendflags=False)
        summary = flagdata(vis=self.vis,mode='summary', spw='8,9,10')
        self.assertEqual(summary['spw']['8']['flagged'],0,'Error in channel selection with Rflag agent')
        self.assertEqual(summary['spw']['9']['flagged'],1861,'Error in channel selection with Rflag agent')
        self.assertEqual(summary['spw']['10']['flagged'],0,'Error in channel selection with Rflag agent')
        
        
    def test_CAS_4200(self):
        """flagdata: Test quack mode with quackinterval 0"""
        
        res = flagdata(vis=self.vis,mode='quack',quackinterval=0, flagbackup=False)
        self.assertTrue(res == {})
#        summary_zero = flagdata(vis=self.vis,mode='summary', spw='8')
#        self.assertEqual(summary_zero['flagged'],0,'Error in quack mode with quack interval 0')
        
        flagdata(vis=self.vis,mode='quack',quackinterval=1, flagbackup=False)
        summary_one = flagdata(vis=self.vis,mode='summary', spw='8')
        
#        flagdata(vis=self.vis,mode='unflag', flagbackup=False)
        self.unflag_ms()
        flagdata(vis=self.vis,mode='quack', flagbackup=False)
        summary_default = flagdata(vis=self.vis,mode='summary', spw='8')
        
        self.assertEqual(summary_one['flagged'],summary_default['flagged'],'Error in quack mode with quack interval 1')
        
    def test_alias(self):
        '''flagdata: Test flagdata alias'''
        res = flagdata(vis=self.vis, mode='summary', spw='1')['flagged']
        self.assertEqual(res, 0)
        
    def test_spw_freq1(self):
        '''flagdata: CAS-3562, flag all spw channels greater than a frequency'''
        flagdata(vis=self.vis, spw='>2000MHz', flagbackup=False)
        
        # Flag only spw=6,7
        res = flagdata(vis=self.vis, mode='summary', spw='0,6,7,10')
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
        res = flagdata(vis=self.vis, mode='summary', spw='1,5,8,15',spwchan=True)
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
        res = flagdata(vis=self.vis, mode='summary', spw='0~6', spwchan=True)
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
        flagdata(vis=self.vis, mode='clip', clipminmax=[0,500], flagbackup=False,
                 datacolumn='FPARAM')
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
               
    def test_clip_fparam_sol1(self):
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
        
    def test_list_fparam_sol1_extension(self):
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
#        flagdata(vis=self.vis, mode='unflag', flagbackup=False)
        self.unflag_ms()
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

    def test_clip_fparam_sol2(self):
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
        
    def test_clip_fparam_sol1sol2(self):
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
                 flagbackup=False, datacolumn='FPARAM')
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 1192.0)
        
    def test_invalid_datacol_cal(self):
        '''Flagdata: invalid data column should not fall back to default'''
        flagdata(vis=self.vis, mode='clip', clipminmax=[0.,600.],datacolumn='PARAMERR',
                 flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
#        self.assertEqual(res['flagged'], 1192.0)
        self.assertFalse(res['flagged']==1192.0)
                
    def test_clip_fparam_all(self):
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

    def test_clip_fparam_all(self):
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

    def test_clip_nans_fparam_all(self):
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

    def test_clip_fparam_error_absall(self):
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

    def test_clip_fparam_error_abs1(self):
        """Flagdata:: Error case test when a complex operator is used with CalTables """

        flagdata(vis=self.vis, mode='clip', datacolumn='FPARAM', correlation='ABS_Sol1',
                 clipzeros=True, clipminmax=[0.,600.], flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')

        self.assertEqual(res['total'], 129024)
        self.assertEqual(res['flagged'], 750)
        self.assertEqual(res['correlation']['Sol1']['flagged'], 750.0)
        self.assertEqual(res['correlation']['Sol2']['flagged'], 0)

    def test_clip_fparam_error_abs12(self):
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
                 datacolumn='FPARAM', flagbackup=False)
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
#        flagdata(vis=self.vis, mode='unflag', flagbackup=False)
        self.unflag_ms()
        flagdata(vis=self.vis, mode='manual', timerange="03:50:00~04:10:00", spw='1',
                 flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['scan']['14']['flagged'],2304)
        self.assertEqual(res['scan']['17']['flagged'],2304)
        self.assertEqual(res['spw']['1']['flagged'],4608)
        self.assertEqual(res['spw']['3']['flagged'],0)
        self.assertEqual(res['flagged'],4608)
         
        # Now check that the same is flagged using scan selection
#        flagdata(vis=self.vis, mode='unflag', flagbackup=False)
        self.unflag_ms()
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
        flagdata(vis=self.vis, mode='clip', clipzeros=True, datacolumn='CPARAM', flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 11078.0, 'Should use CPARAM as the default column')

    def test_invalid_datacol(self):
        '''Flagdata: invalid data column should not fall back to default'''
        flagdata(vis=self.vis, mode='clip', clipzeros=True, datacolumn='PARAMERR',
                 flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
#        self.assertEqual(res['flagged'], 11078.0)
        self.assertFalse(res['flagged']==11078.0)
        
                        
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
        flagdata(vis=self.vis, mode='list', inpfile=["mode='clip' clipminmax=[0,3] "\
        "correlation='REAL_Sol1' datacolumn='CPARAM'"],
                 flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 309388)
        self.assertEqual(res['correlation']['Sol2']['flagged'], 0)
                
    def test_rflag_cparam_sol2_for_bpass(self):
        """Flagdata:: Test rflag solution 2 of CPARAM column for bpass"""

        flagdata(vis=self.vis, mode='rflag', correlation='Sol2', flagbackup=False,
                 datacolumn='CPARAM', extendflags=False)
        summary=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(summary['flagged'], 13197)
        self.assertEqual(summary['correlation']['Sol1']['flagged'], 0)
        self.assertEqual(summary['correlation']['Sol2']['flagged'], 13197)

    def test_tfcrop_cparam_all_for_bpass(self):
        """Flagdata:: Test tfcrop in ABS_ALL calibration solutions of CPARAM column"""

        flagdata(vis=self.vis, mode='clip', datacolumn='CPARAM',correlation='ABS_ALL',clipzeros=True,
                 flagbackup=False)
        flagdata(vis=self.vis, mode='tfcrop', datacolumn='CPARAM',correlation='ABS_ALL',
                 flagbackup=False, extendflags=False)
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
                 flagbackup=False, extendflags=False)
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
        
        flagmanager(vis=self.vis, mode='list')
        aflocal.open(self.vis)
        self.assertEqual(len(aflocal.getflagversionlist()), 2)
        aflocal.done()

        flagdata(vis=self.vis, mode='unflag', flagbackup=False)
        flagmanager(vis=self.vis, mode='list')
        aflocal.open(self.vis)
        self.assertEqual(len(aflocal.getflagversionlist()), 2)
        aflocal.done()

        flagdata(vis=self.vis, mode='unflag', flagbackup=True)
        flagmanager(vis=self.vis, mode='list')
        aflocal.open(self.vis)
        self.assertEqual(len(aflocal.getflagversionlist()), 3)
        aflocal.done()
        
        newname = 'Ha! The best version ever!'

        flagmanager(vis=self.vis, mode='rename', oldname='flagdata_1', versionname=newname, 
                    comment='This is a *much* better name')
        flagmanager(vis=self.vis, mode='list')
        aflocal.open(self.vis)
        self.assertEqual(len(aflocal.getflagversionlist()), 3)
        aflocal.done()
        
        self.assertTrue(os.path.exists(self.vis+'.flagversions/flags.'+newname),
                        'Flagversion file does not exist: flags.'+newname)        

    def test_cal_time1(self):
        '''Flagdata: clip a timerange from one field'''
        # this timerange corresponds to field 3C286_A
        flagdata(vis=self.vis, mode='clip', timerange='<14:12:52',clipzeros=True,
                 clipminmax=[0.,0.35], datacolumn='CPARAM',flagbackup=False)
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
                 clipminmax=[0.,0.4], datacolumn='CPARAM',flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['field']['3C286_A']['flagged'],0)
        self.assertEqual(res['field']['3C286_B']['flagged'],0)
        self.assertEqual(res['field']['3C286_C']['flagged'],0)
        self.assertEqual(res['field']['3C286_D']['flagged'],2221)
        self.assertEqual(res['flagged'],2221)
        
    def test_cal_time_corr(self):
        '''Flagdata: select a timerange for one solution'''
        flagdata(vis=self.vis, mode='clip', clipminmax=[0.,0.4], timerange='14:23:50~14:48:40.8',
                 correlation='Sol2',datacolumn='CPARAM',flagbackup=False)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['correlation']['Sol1']['flagged'], 0)
        self.assertEqual(res['correlation']['Sol2']['flagged'], 17)
        self.assertEqual(res['flagged'],17)
        
        # Check that the timerange selection was taken. Flag only the solution
        flagdata(vis=self.vis, mode='unflag', flagbackup=True)
        flagdata(vis=self.vis, mode='clip', clipminmax=[0.,0.4], correlation='Sol2', 
                 datacolumn='CPARAM',flagbackup=False)
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
                 scan='46', datacolumn='CPARAM', flagbackup=False)
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
        flagdata(vis=self.vis, mode='clip', clipzeros=True, correlation='Sol2', 
                 datacolumn='CPARAM',flagbackup=False)
        
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
#        flagdata(vis=self.vis, mode='unflag')
        self.unflag_ms()
        flagdata(vis=self.vis, scan='1', flagbackup=False)                
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['observation']['0']['flagged'],108)
        self.assertEqual(res['scan']['1']['flagged'],108)
        self.assertEqual(res['flagged'],108)

# CAS-5044
class test_weight_spectrum(test_base):
    """flagdata:: Test flagging WEIGHT_SPECTRUM column"""
                                                
    def test_clipzeros_weight(self):
        '''flagdata: datacolumn=WEIGHT_SPECTRUM, clip zeros'''
        self.setUp_wtspec()
        flagdata(vis=self.vis, mode='clip', datacolumn='weight_SPECTRUM', 
                 clipzeros=True, flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        # First and last channels have WEIGHT_SPECTRUM zero.
        # 2chans * 2pols * 4rows = 16 flagged points
        self.assertEqual(res['flagged'],16)
        
    def test_clip_range(self):
        '''flagdata: datacolumn=WEIGHT_SPECTRUM, flag a range'''
        self.setUp_wtspec()
        flagdata(vis=self.vis, mode='clip', datacolumn='WEIGHT_SPECTRUM', 
                 clipminmax=[0,2.1], spw='0:1~29', flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        # Should clip only correlation LL. Excluding zero channels (0 and 30)
        self.assertEqual(res['flagged'],116)
        self.assertEqual(res['correlation']['RR']['flagged'],0)
        self.assertEqual(res['correlation']['LL']['flagged'],116)

    def test_clip_chanavg(self):
        '''flagdata: datacolumn=WEIGHT_SPECTRUM, channel average'''
        self.setUp_wtspec()
        flagdata(vis=self.vis, mode='clip', spw='0:1~29',datacolumn='WEIGHT_SPECTRUM', 
                 clipminmax=[0,2.1], channelavg=True, flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        # Same result as previous test. The values of the weight_spectrum
        # for each channel are the same, excluding the 2 channels that are
        # zero in each polarization
        self.assertEqual(res['flagged'],116)
        self.assertEqual(res['correlation']['RR']['flagged'],0)
        self.assertEqual(res['correlation']['LL']['flagged'],116)

    def test_clip_onepol(self):
        '''flagdata: datacolumn=WEIGHT_SPECTRUM, one polarization'''
        self.setUp_wtspec()
        flagdata(vis=self.vis, mode='clip', datacolumn='WEIGHT_SPECTRUM', 
                 clipminmax=[0,2.04], correlation='RR', clipzeros=True, flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')

        self.assertEqual(res['flagged'],95)
        self.assertEqual(res['correlation']['RR']['flagged'],95)
        self.assertEqual(res['correlation']['LL']['flagged'],0)

    def test_tfcrop_weight(self):
        '''flagdata: datacolumn=WEIGHT_SPECTRUM, run tfcrop'''
        self.setUp_wtspec()
        flagdata(vis=self.vis, mode='tfcrop', datacolumn='WEIGHT_SPECTRUM', 
                 flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'],16)
        
    def test_weight1(self):
        '''flagdata: use datacolumn='WEIGHT' and clip spw=0'''
        self.setUp_weightcol()
        flagdata(vis=self.vis, mode='clip', datacolumn='WEIGHT', 
                 clipminmax=[0,50.0], flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'],31)
        self.assertEqual(res['spw']['0']['flagged'],31)
        
    def test_weight2(self):
        '''flagdata: use datacolumn='WEIGHT' and clip inside'''
        self.setUp_weightcol()
        flagdata(vis=self.vis, mode='clip', datacolumn='WEIGHT', 
                 clipminmax=[0,50.0], clipoutside=False, flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'],31)
        self.assertEqual(res['spw']['1']['flagged'],31)

    def test_weight3(self):
        '''flagdata: clip using datacolumn='WEIGHT' and channelavg=True'''
        self.setUp_weightcol()
        flagdata(vis=self.vis, mode='clip', datacolumn='WEIGHT', 
                 clipminmax=[0,50.0], clipoutside=True, channelavg=True, flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'],31)
        self.assertEqual(res['spw']['0']['flagged'],31)

    def test_weight4(self):
        '''flagdata: clip using datacolumn='WEIGHT' and select some channels'''
        self.setUp_weightcol()
        flagdata(vis=self.vis, mode='clip', datacolumn='WEIGHT', spw='0,1:1~10', 
                 clipminmax=[0,31.0], clipoutside=True, flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary', spwchan=True)
        self.assertEqual(res['flagged'],41)
        self.assertEqual(res['spw']['0']['flagged'],31)
        self.assertEqual(res['spw:channel']['1:0']['flagged'],0)
        self.assertEqual(res['spw:channel']['1:1']['flagged'],1)
        self.assertEqual(res['spw:channel']['1:2']['flagged'],1)
        self.assertEqual(res['spw:channel']['1:3']['flagged'],1)
        self.assertEqual(res['spw:channel']['1:4']['flagged'],1)
        self.assertEqual(res['spw:channel']['1:5']['flagged'],1)
        self.assertEqual(res['spw:channel']['1:6']['flagged'],1)
        self.assertEqual(res['spw:channel']['1:7']['flagged'],1)
        self.assertEqual(res['spw:channel']['1:8']['flagged'],1)
        self.assertEqual(res['spw:channel']['1:9']['flagged'],1)
        self.assertEqual(res['spw:channel']['1:10']['flagged'],1)
        
    def test_weight5(self):
        '''flagdata: clip using WEIGHT, then using WEIGHT_SPECTRUM'''
        self.setUp_weightcol()
        flagdata(vis=self.vis, flagbackup=False, mode='clip', datacolumn='WEIGHT',
                 clipminmax=[0.0, 50.0])
        
        res = flagdata(vis=self.vis, mode='summary', spwchan=True)        
        self.assertEqual(res['flagged'],31)
        self.assertEqual(res['spw']['0']['flagged'],31)

        # Unflag, run mstransform to create a WEIGHT_SPECTRUM and flag again
        self.unflag_ms()
        mstransform(vis=self.vis, outputvis='weight_spectrum.ms',datacolumn='all',
                    usewtspectrum=True)
        
        # divide WEIGHT clipmax by the number of channels
        newmax = 50./31
        flagdata(vis='weight_spectrum.ms', flagbackup=False, mode='clip', 
                 datacolumn='WEIGHT_SPECTRUM', clipminmax=[0.0, newmax])
        res = flagdata(vis='weight_spectrum.ms', mode='summary', spwchan=True)        
        self.assertEqual(res['flagged'],31)
        self.assertEqual(res['spw']['0']['flagged'],31)
        
        self.addCleanup(shutil.rmtree, 'weight_spectrum.ms',True)

        
class test_float_column(test_base):
    """flagdata:: Test flagging FLOAT_DATA column"""
    
    def setUp(self):
        self.setUp_floatcol()
                
    def test_manual_channels(self):
        '''flagdata: flag meta-data from a single-dish MS'''
        flagdata(vis=self.vis, spw='1;3;5;7:0~4,1;3:507~511,5:1019~1023,7:2043~2047')
        res = flagdata(vis=self.vis, mode='summary', spw='1,3,5,7', spwchan=False)
        self.assertEqual(res['spw']['1']['flagged'],20)
        self.assertEqual(res['spw']['3']['flagged'],20)
        self.assertEqual(res['spw']['5']['flagged'],40)
        self.assertEqual(res['spw']['7']['flagged'],40)
        self.assertEqual(res['flagged'],120)

    def test_field_name(self):
        '''flagdata: Field name with whitespaces'''
        flagdata(vis=self.vis, flagbackup=False, field='r aqr')
        res = flagdata(vis=self.vis, mode='summary', field='r aqr')
        self.assertEqual(res['field']['r aqr']['flagged'],14360)

    def test_clip_frange(self):
        '''flagdata: datacolumn=FLOAT_DATA, flag a range'''
        flagdata(vis=self.vis, spw='0',mode='clip', datacolumn='FLOAT_DATA', 
                 clipminmax=[0,230.5], flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary', spw='0')
        self.assertEqual(res['flagged'],3)

    def test_clip_fchanavg(self):
        '''flagdata: datacolumn=FLOAT_DATA, channel average'''
        flagdata(vis=self.vis, mode='clip', spw='2', clipminmax=[0,3.9], 
                 channelavg=True, datacolumn='FLOAT_DATA',flagbackup=False)
        res = flagdata(vis=self.vis, mode='summary',spw='2')
        # There is only one channel in each polarization
        self.assertEqual(res['flagged'],2)
        self.assertEqual(res['total'],2)
        
    def test_clip_fchanavg_onepol(self):
        '''flagdata: datacolumn=FLOAT_DATA, one pol, channel average'''
        flagdata(vis=self.vis, mode='clip', spw='2', clipminmax=[0,3.9], 
                 channelavg=True, correlation='YY', flagbackup=False, datacolumn='float_data')
        res = flagdata(vis=self.vis, mode='summary',spw='2')
        # There is only one channel in each polarization
        self.assertEqual(res['flagged'],1)
        self.assertEqual(res['total'],2)

    def test_tfcrop_float(self):
        '''flagdata: datacolumn=FLOAT_DATA, run tfcrop'''
        flagdata(vis=self.vis, mode='tfcrop', datacolumn='FLOAT_DATA', 
                 flagbackup=True)
        res = flagdata(vis=self.vis, mode='summary')
        # It only shows that it runs without problems
        self.assertEqual(res['flagged'],264)
        
    def test_float_autocorr(self):
        '''flagdata: CAS-5286, autocorr=True should not flag single-dish data'''
        flagdata(vis=self.vis, mode='manual', autocorr=True, 
                 flagbackup=False)
        
        # The PROCESSOR TYPE of this dataset is unset, therefore it should not be
        # flagged
        res = flagdata(vis=self.vis, mode='summary', basecnt=True)
        self.assertEqual(res['flagged'],0)
        self.assertEqual(res['baseline']['PM04&&PM04']['flagged'],0)

    def test_field_strange_chars(self):
        '''flagdata: CAS-5481, field name with = sign'''
        # Create a fake list, as this MS does not have such field
        cmdlist = "mode='manual' field='FAKE=FIELD' autocorr=False\n"+\
                   "mode='clip' clipzeros=True field='Is= TO FAKE'\n"
        
        filename = 'listfakefield.txt'
        create_input(cmdlist, filename)
        
        flagdata(vis=self.vis, mode='list', inpfile=filename, flagbackup=False,
                  action='', savepars=True, outfile='outfakefield.txt')
        
        self.assertTrue(filecmp.cmp(filename, 'outfakefield.txt', 1), 'Files should be equal')


class test_tbuff(test_base):
    '''Test flagdata in list mode and time buffer padding'''
    def setUp(self):
        self.setUp_tbuff()
        
    def tearDown(self):
        os.system('rm -rf '+self.online)
        os.system('rm -rf '+self.user)
    
    def test_double_tbuff(self):
        '''flagdata: Apply a tbuff in the online flags'''
        
        # Apply the sub-set of online flags
        # uid___A002_X72c4aa_X8f5_online.txt contains the DV04&&* flag
        flagdata(self.vis, flagbackup=False,mode='list',inpfile=self.online, tbuff=0.0)
        flags_before = flagdata(self.vis, mode='summary', basecnt=True)
        
        # Unflag and apply a tbuff=0.504s
        flagdata(self.vis, flagbackup=False,mode='unflag')
        flagdata(self.vis, flagbackup=False,mode='list',inpfile=self.online, tbuff=0.504)
        flags_after = flagdata(self.vis, mode='summary', basecnt=True)
        
        self.assertEqual(flags_before['flagged'], flags_after['flagged']/2)

    def test_list_tbuff(self):
        '''flagdata: Apply a tbuff list in two files'''
        
        # Apply the sub-set of online flags and user flags
        # uid___A002_X72c4aa_X8f5_online.txt contains the DV04&&* flag
        # uid___A002_X72c4aa_X8f5_user.txt contains the DV10 flag
        flagdata(self.vis, flagbackup=False,mode='list',inpfile=[self.user,self.online])
        
        # Only the DV04 baselines are flagged, not the DV10 (except for DV04&DV10)
        flags1 = flagdata(self.vis, mode='summary', basecnt=True)
        self.assertEqual(flags1['antenna']['DV04']['flagged'],29) # DV04&&*
        self.assertEqual(flags1['antenna']['DV10']['flagged'],1) # DV04&DV10

        # Unflag and apply tbuff=[0.504]. It should increase only the DV10 flags from user flags
        flagdata(self.vis, flagbackup=False,mode='unflag')
        flagdata(self.vis, flagbackup=False,mode='list',inpfile=[self.user,self.online], tbuff=[0.504])
        flags2 = flagdata(self.vis, mode='summary', basecnt=True)
        self.assertEqual(flags2['antenna']['DV04']['flagged'],29) 
        self.assertEqual(flags2['antenna']['DV10']['flagged'],29) 
         
        # Unflag and apply tbuff=[0.504,0.504]. It should increase the DV04 and DV10 flags
        flagdata(self.vis, flagbackup=False,mode='unflag')
        flagdata(self.vis, flagbackup=False,mode='list',inpfile=[self.online,self.user], tbuff=[0.504,0.504])
        flags3 = flagdata(self.vis, mode='summary', basecnt=True)
        self.assertEqual(flags3['antenna']['DV04']['flagged'],58) 
        self.assertEqual(flags3['antenna']['DV10']['flagged'],30) 
        

# Cleanup class 
class cleanup(test_base):
    
    def tearDown(self):
        os.system('rm -rf ngc5921.*ms* testwma*ms*')
        os.system('rm -rf flagdatatest.*ms*')
        os.system('rm -rf missing-baseline.*ms*')
        os.system('rm -rf multiobs.*ms*')
        os.system('rm -rf uid___A002_X30a93d_X43e_small.*ms*')
        os.system('rm -rf Four_ants_3C286.*ms*')
        os.system('rm -rf shadowtest_part.*ms*')
        os.system('rm -rf testmwa.*ms*')
        os.system('rm -rf cal.fewscans.bpass*')
        os.system('rm -rf X7ef.tsys* ap314.gcal*')
        os.system('rm -rf list*txt*')
        os.system('rm -rf fourrows*')
        os.system('rm -rf SDFloatColumn*')
        os.system('rm -rf *weight*ms*')
        os.system('rm -rf uid___A002_X72c4aa_X8f5_scan21_spw18*')

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
            test_alma,
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
            test_weight_spectrum,
            test_float_column,
            test_tbuff,
            cleanup]
