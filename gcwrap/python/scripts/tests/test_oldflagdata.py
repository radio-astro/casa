import shutil
import unittest
import os
from tasks import *
from taskinit import *

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
        flagdata(vis=self.vis, unflag=True)

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
        flagdata(vis=self.vis, unflag=True)

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
        flagdata(vis=self.vis, unflag=True)
        
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
        flagdata(vis=self.vis, unflag=True)


#class test_rfi(test_base):
#    """Test of mode = 'rfi'"""
#    
#    def setUp(self):
#        self.setUp_flagdatatest()
#        
#    def test1(self):
#        flagdata(vis=self.vis, mode='rfi', time_amp_cutoff=2.0)
#        test_eq(flagdata(vis=self.vis, mode='summary'), 70902, 3000)
#        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 5252, 144)


class test_shadow(test_base):
    """Test of mode = 'shadow'"""
    def setUp(self):
        self.setUp_flagdatatest()

    def test1(self):
        flagdata(vis=self.vis, mode='shadow', diameter=40)
        test_eq(flagdata(vis=self.vis, mode='summary'), 70902, 5252)

    def test2(self):
        flagdata(vis=self.vis, mode='shadow')
        test_eq(flagdata(vis=self.vis, mode='summary'), 70902, 2912)

    def test3(self):
        flagdata(vis=self.vis, mode='shadow', correlation='LL')
        test_eq(flagdata(vis=self.vis, mode='summary'), 70902, 1456)


class test_shadow_ngc5921(test_base):
    """More test of mode = 'shadow'"""
    def setUp(self):
        self.setUp_ngc5921()

    def test_CAS2399(self):
        flagdata(vis = self.vis,
                 unflag = True)
        flagdata(vis = self.vis,
                 mode = "shadow",
                 diameter = 35)
        allbl = flagdata(vis = self.vis,
                         mode = "summary")

        # Sketch of what is being shadowed:
        #
        #  A23 shadowed by A1
        #
        #  A13 shadowed by A2 shadowed by A9
        #
        
        # Now remove baseline 2-13   (named 3-14)
        outputvis = "missing-baseline.ms"
        os.system("rm -rf " + outputvis)
        split(vis = self.vis, 
              outputvis = outputvis,
              datacolumn = "data",
              antenna = "!3&&14")
        
        flagdata(vis = outputvis,
                 unflag = True)
        flagdata(vis = outputvis,
                 mode = "shadow",
                 diameter = 35)
        
        missingbl = flagdata(vis = outputvis,
                             mode = "summary")

        # With baseline based flagging, A13 will not get flagged
        # when the baseline is missing
        #
        # With antenna based flagging, A13 should be flagged
        
        assert allbl['antenna']['3']['flagged'] > 1000
        assert allbl['antenna']['24']['flagged'] > 1000
        
        assert missingbl['antenna']['3']['flagged'] > 1000
        assert missingbl['antenna']['24']['flagged'] == allbl['antenna']['24']['flagged']
        
        assert allbl['antenna']['14']['flagged'] > 1000
        # When the baseline is missing, the antenna is not flagged as before
        assert missingbl['antenna']['14']['flagged'] < 1000
        
        # For antenna based flagging, it should be (to be uncommented when CAS-2399
        # is solved):
        #assert missingbl['antenna']['14']['flagged'] > 1000

    def test1(self):
        flagdata(vis = self.vis,
                 mode = "shadow",
                 diameter = 50)

        s = flagdata(vis = self.vis,
                     mode = "summary")['antenna']

        assert s['1']['flagged'] == 58968; assert s['1']['total'] == 203994
        assert s['10']['flagged'] == 117432; assert s['10']['total'] == 203994
        assert s['11']['flagged'] == 175392; assert s['11']['total'] == 203994
        assert s['12']['flagged'] == 58968; assert s['12']['total'] == 203994
        assert s['13']['flagged'] == 203994; assert s['13']['total'] == 203994
        assert s['14']['flagged'] == 203994; assert s['14']['total'] == 203994
        assert s['15']['flagged'] == 152838; assert s['15']['total'] == 203994
        assert s['16']['flagged'] == 58968; assert s['16']['total'] == 203994
        assert s['17']['flagged'] == 57960; assert s['17']['total'] == 200718
        assert s['18']['flagged'] == 58968; assert s['18']['total'] == 203994
        assert s['19']['flagged'] == 58968; assert s['19']['total'] == 203994
        assert s['2']['flagged'] == 203994; assert s['2']['total'] == 203994
        assert s['20']['flagged'] == 58968; assert s['20']['total'] == 203994
        assert s['21']['flagged'] == 58968; assert s['21']['total'] == 203994
        assert s['22']['flagged'] == 58968; assert s['22']['total'] == 203994
        assert s['24']['flagged'] == 203994; assert s['24']['total'] == 203994
        assert s['25']['flagged'] == 58968; assert s['25']['total'] == 203994
        assert s['26']['flagged'] == 58968; assert s['26']['total'] == 203994
        assert s['27']['flagged'] == 58968; assert s['27']['total'] == 203994
        assert s['28']['flagged'] == 58968; assert s['28']['total'] == 203994
        assert s['3']['flagged'] == 203994; assert s['3']['total'] == 203994
        assert s['4']['flagged'] == 87570; assert s['4']['total'] == 203994
        assert s['5']['flagged'] == 163674; assert s['5']['total'] == 203994
        assert s['6']['flagged'] == 58968; assert s['6']['total'] == 203994
        assert s['7']['flagged'] == 58968; assert s['7']['total'] == 203994
        assert s['8']['flagged'] == 58968; assert s['8']['total'] == 203994
        assert s['9']['flagged'] == 58968; assert s['9']['total'] == 203994


class test_vector(test_base):
    def setUp(self):
        self.setUp_flagdatatest()

    def test1(self):
        print "Test of vector mode"
        
        clipminmax=[0.0, 0.2]
        antenna = ['1', '2']
        clipcolumn = ['DATA', 'datA']

        flagdata(vis=self.vis, clipminmax=clipminmax, antenna=antenna, clipcolumn=clipcolumn)
        test_eq(flagdata(vis=self.vis, mode='summary'), 70902, 17897)

    def test2(self):
        flagdata(vis=self.vis, antenna=["2", "3", "5", "6"])
        s = flagdata(vis=self.vis, mode="summary")
        for a in ["VLA3", "VLA4", "VLA6", "VLA7"]:
            self.assertEqual(s['antenna'][a]['flagged'], 5252)
        for a in ["VLA1", "VLA2", "VLA5", "VLA8", "VLA9", "VLA10", "VLA11", "VLA24"]:
            self.assertEqual(s['antenna'][a]['flagged'], 808)

    def test_many_agents(self):
        """More than 32 agents"""
        # 1 agent
        flagdata(vis = self.vis,
                 antenna="3")
        test_eq(flagdata(vis = self.vis, mode = "summary"), 70902, 5252)

        flagdata(vis=self.vis, unflag=True)

        # 500 agents
        flagdata(vis = self.vis,
                 antenna=["3"] * 500)
        test_eq(flagdata(vis = self.vis, mode = "summary"), 70902, 5252)

class test_vector_ngc5921(test_base):
    def setUp(self):
        self.setUp_ngc5921()

    def test3(self):
        flagdata(vis=self.vis, antenna=["2", "3", "5", "6"], scan="4")
        s = flagdata(vis=self.vis, mode="summary")
        for a in ["2", "3", "5", "6"]:
            self.assertEqual(s['antenna'][a]['flagged'], 6552)
        for a in ["1", "4", "7", "8", "9", "10", "24"]:
            self.assertEqual(s['antenna'][a]['flagged'], 1008)

    def test4(self):
        a = ["2", "13", "5", "9"]
        t = ["09:30:00~09:40:00",
             "10:20:00~10:25:00",
             "09:50:00~13:30:00",
             "09:30:00~09:40:00"]

        # Flag one after another
        for i in range(len(a)):
            flagdata(vis=self.vis, antenna=a[i], timerange=t[i])

        stats_serial = flagdata(vis=self.vis, mode="summary")

        # Flag in parallel
        flagdata(vis=self.vis, unflag=True)
        flagdata(vis=self.vis, antenna=a, timerange=t)
        
        stats_parallel = flagdata(vis=self.vis, mode="summary")

        # Did we get the same net results? (full recursive comparison of statistics dictionaries)
        self.assertEqual(stats_serial, stats_parallel)

        # If any corner of the dictionaries had differed, the test above would have failed
        # (just double checking)
        stats_serial["channel"]["0:23"]["total"] = 25703
        self.assertNotEqual(stats_serial, stats_parallel)

        stats_serial["channel"]["0:23"]["total"] = 25704
        self.assertEqual(stats_serial, stats_parallel)
        
        # And was most data flagged for the specified antennas?
        for ant in range(28):
            name = str(ant+1) # From "1" to "28"
            if name not in ["23"]:
                if name in a:
                    assert stats_serial["antenna"][name]['flagged'] > 5544, stats_serial["antenna"]
                else:
                    self.assertEqual(stats_serial["antenna"][name]['flagged'], 5544, "antenna=" + name + str(stats_serial["antenna"]))

class test_flagmanager(test_base):
    def setUp(self):
        os.system("rm -rf flagdatatest.ms*") # test1 needs a clean start
        self.setUp_flagdatatest()
        
    def test1(self):
        print "Test of flagmanager mode=list, flagbackup=True/False"
        flagmanager(vis=self.vis, mode='list')
        fg.open(self.vis)
        self.assertEqual(len(fg.getflagversionlist()), 3)
        fg.done()

        flagdata(vis=self.vis, unflag=True, flagbackup=False)
        flagmanager(vis=self.vis, mode='list')
        fg.open(self.vis)
        self.assertEqual(len(fg.getflagversionlist()), 3)
        fg.done()

        flagdata(vis=self.vis, unflag=True, flagbackup=True)
        flagmanager(vis=self.vis, mode='list')
        fg.open(self.vis)
        self.assertEqual(len(fg.getflagversionlist()), 4)
        fg.done()

        print "Test of flagmanager mode=rename"
        flagmanager(vis=self.vis, mode='rename', oldname='manualflag_2', versionname='Ha! The best version ever!', comment='This is a *much* better name')
        flagmanager(vis=self.vis, mode='list')
        fg.open(self.vis)
        self.assertEqual(len(fg.getflagversionlist()), 4)
        fg.done()

    def test2(self):
        """Create, then restore autoflag"""

        flagdata(vis = self.vis, mode='summary')
        flagmanager(vis = self.vis)
        
        flagdata(vis = self.vis, antenna="2")
        
        flagmanager(vis = self.vis)
        ant2 = flagdata(vis = self.vis, mode='summary')['flagged']

        print "After flagging antenna 2 there were", ant2, "flags"

        # Change flags, then restore
        flagdata(vis = self.vis, antenna="3")
        flagmanager(vis = self.vis)
        ant3 = flagdata(vis = self.vis, mode='summary')['flagged']

        print "After flagging antenna 2 and 3 there were", ant3, "flags"

        flagmanager(vis = self.vis, mode='restore', versionname='manualflag_3')
        restore2 = flagdata(vis = self.vis, mode='summary')['flagged']

        print "After restoring pre-antenna 3 flagging, there are", restore2, "flags, should be", ant2

        assert restore2 == ant2

    def test_CAS2701(self):
        """Do not allow flagversion=''"""

        
        fg.open(self.vis)
        l = len(fg.getflagversionlist())
        fg.done()
        
        flagmanager(vis = self.vis,
                    mode = "save",
                    versionname = "non-empty-string")

        fg.open(self.vis)
        self.assertEqual(len(fg.getflagversionlist()), l+1)
        fg.done()

        flagmanager(vis = self.vis,
                    mode = "save",
                    versionname = "non-empty-string")

        fg.open(self.vis)
        self.assertEqual(len(fg.getflagversionlist()), l+1)
        fg.done()


class test_msselection(test_base):

    def setUp(self):
        self.setUp_ngc5921()

    def test_simple(self):
        baselines = flagdata(vis = self.vis, mode="summary", antenna="9")['baseline'].keys()
        assert "9&&9" not in baselines
        assert "9&&10" in baselines
        assert "9&&11" in baselines
        assert "10&&10" not in baselines
        assert "10&&11" not in baselines

        baselines = flagdata(vis = self.vis, mode="summary", antenna="9,10")['baseline'].keys()
        assert "9&&9" not in baselines
        assert "9&&10" in baselines
        assert "9&&11" in baselines
        assert "10&&10" not in baselines
        assert "10&&11" in baselines

    def test_amp(self):
        baselines = flagdata(vis = self.vis, mode="summary", antenna="9&")['baseline'].keys()
        #??? assert "9&&9" not in baselines
        #assert "9&&10" not in baselines
        #assert "9&&11" not in baselines
        #assert "10&&10" not in baselines
        #assert "10&&11" not in baselines

        baselines = flagdata(vis = self.vis, mode="summary", antenna="9,10&")['baseline'].keys()
        assert "9&&9" not in baselines
        assert "9&&10" in baselines
        assert "9&&11" not in baselines
        assert "10&&10" not in baselines
        assert "10&&11" not in baselines

        baselines = flagdata(vis = self.vis, mode="summary", antenna="9&10")['baseline'].keys()
        assert "9&&9" not in baselines
        assert "9&&10" in baselines
        assert "9&&11" not in baselines
        assert "10&&10" not in baselines
        assert "10&&11" not in baselines

#class test_autoflag(test_base):
#
#    def setUp(self):
#        self.setUp_ngc5921()
#
#    def test_CAS1979(self):
#        """Test that autoflagging does not clear flags"""
#        s0 = flagdata(vis=self.vis, mode="summary")['flagged']
#        flagdata(vis=self.vis, mode="autoflag", algorithm="freqmed", field="0", spw="0")
#        s1 = flagdata(vis=self.vis, mode="summary")['flagged']
#        flagdata(vis=self.vis, mode="autoflag", algorithm="freqmed", field="0", spw="0")
#        s2 = flagdata(vis=self.vis, mode="summary")['flagged']
#        flagdata(vis=self.vis, mode="autoflag", algorithm="freqmed", field="0", spw="0")
#        s3 = flagdata(vis=self.vis, mode="summary")['flagged']
#        flagdata(vis=self.vis, mode="autoflag", algorithm="freqmed", field="0", spw="0")
#        s4 = flagdata(vis=self.vis, mode="summary")['flagged']
#        flagdata(vis=self.vis, mode="autoflag", algorithm="freqmed", field="0", spw="0")
#        s5 = flagdata(vis=self.vis, mode="summary")['flagged']
#        flagdata(vis=self.vis, mode="autoflag", algorithm="freqmed", field="0", spw="0")
#        s6 = flagdata(vis=self.vis, mode="summary")['flagged']
#
#        assert s0 == 0
#        assert s0 <= s1
#        assert s1 <= s2
#        assert s2 <= s3
#        assert s3 <= s4
#        assert s4 <= s5
#        assert s5 <= s6
#
#    def test_auto1(self):
#        print "Test of autoflag, algorithm=timemed"
#        flagdata(vis=self.vis, mode='autoflag', algorithm='timemed', window=3)
#        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 4725)
#
#    def test2(self):
#        print "Test of autoflag, algorithm=freqmed"
#        flagdata(vis=self.vis, mode='autoflag', algorithm='freqmed')
#        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 29101)
#
#    def test_CAS2410(self):
#        flagdata(vis=self.vis, scan='3')
#        flagdata(vis=self.vis, scan='6', mode='autoflag', algorithm='timemed', window=3)
#        test_eq(flagdata(vis=self.vis, mode="summary"), 2854278, 763371)

class test_statistics_queries(test_base):

    def setUp(self):
        self.setUp_ngc5921()

    def test_CAS2021(self):
        print "Test antenna selection"
        flagdata(vis=self.vis, antenna='!5') # should not crash

    def test_CAS2212(self):
        print "Test scan + clipping"
        flagdata(vis=self.vis, scan="2", clipminmax = [0.2, 0.3]) # should not crash
    
    def test021(self):
        print "Test of flagging statistics and queries"
        
        flagdata(vis=self.vis, correlation='LL')
        flagdata(vis=self.vis, spw='0:17~19')
        flagdata(vis=self.vis, antenna='5&&9')
        flagdata(vis=self.vis, antenna='14')
        flagdata(vis=self.vis, field='1')
        s = flagdata(vis=self.vis, mode='summary', minrel=0.9)
        assert s['antenna'].keys() == ['14']
        assert '5&&9' in s['baseline'].keys()
        assert set(s['channel'].keys()) == set(['0:17', '0:18', '0:19'])
        assert s['correlation'].keys() == ['0:1']  # LL
        assert s['field'].keys() == ['1']
        assert set(s['scan'].keys()) == set(['2', '4', '5', '7']) # field 1
        s = flagdata(vis=self.vis, mode='summary', maxrel=0.8)
        assert set(s['field'].keys()) == set(['0', '2'])
        s = flagdata(vis=self.vis, mode='summary', minabs=400000)
        assert set(s['scan'].keys()) == set(['3', '6'])
        s = flagdata(vis=self.vis, mode='summary', minabs=400000, maxabs=450000)
        assert s['scan'].keys() == ['3']

    def test4(self):
        print "Test of channel average"
        flagdata(vis=self.vis, channelavg=False, clipminmax=[30, 60])
        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 1414186)

    def test5(self):
        flagdata(vis=self.vis, channelavg=True, clipminmax=[30, 60])
        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 1347822)

    def test6(self):
        flagdata(vis=self.vis, channelavg=False, clipminmax=[30, 60], spw='0:0~10')
        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 242053)

    def test7(self):
        flagdata(vis=self.vis, channelavg=True, clipminmax=[30, 60], spw='0:0~10')
        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 231374)
               

    def test8(self):
        print "Test of mode = 'quack'"
        print "parallel quack"
        flagdata(vis=self.vis, mode='quack', quackinterval=[1.0, 5.0], antenna=['2', '3'], correlation='RR')
        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 22365)

    def test9(self):
        flagdata(vis=self.vis, mode='quack', quackmode='beg', quackinterval=1)
        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 329994)

    def test10(self):
        flagdata(vis=self.vis, mode='quack', quackmode='endb', quackinterval=1)
        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 333396)

    def test11(self):
        flagdata(vis=self.vis, mode='quack', quackmode='end', quackinterval=1)
        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 2520882)

    def test12(self):
        flagdata(vis=self.vis, mode='quack', quackmode='tail', quackinterval=1)
        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 2524284)

    def test13(self):
        print "quack mode quackincrement"
        flagdata(vis=self.vis, mode='quack', quackinterval=50, quackmode='endb', quackincrement=True)
        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 571536)

        flagdata(vis=self.vis, mode='quack', quackinterval=20, quackmode='endb', quackincrement=True)
        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 857304)
        
        flagdata(vis=self.vis, mode='quack', quackinterval=150, quackmode='endb', quackincrement=True)
        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 1571724)
        
        flagdata(vis=self.vis, mode='quack', quackinterval=50, quackmode='endb', quackincrement=True)
        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 1762236)
        flagdata(vis=self.vis, unflag=True)



class test_selections(test_base):
    """Test various selections"""

    def setUp(self):
        self.setUp_ngc5921()

    def test_scan(self):
        
        flagdata(vis=self.vis, scan='3')
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 52416)
        
        # feed not implemented flagdata(vis=vis, feed='27')
        # flagdata(vis=vis, unflag=True)

    def test_antenna(self):

        flagdata(vis=self.vis, antenna='2')
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)

    def test_spw(self):
        
        flagdata(vis=self.vis, spw='0')
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)

    def test_correlation(self):
        flagdata(vis=self.vis, correlation='LL')
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 98217)
        flagdata(vis=self.vis, correlation='LL,RR')
        flagdata(vis=self.vis, correlation='LL RR')
        flagdata(vis=self.vis, correlation='LL ,, ,  ,RR')
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)

    def test_field(self):
        
        flagdata(vis=self.vis, field='0')
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 39186)

    def test_uvrange(self):
        
        flagdata(vis=self.vis, uvrange='200~400m')
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 55944)

    def test_timerange(self):
    
        flagdata(vis=self.vis, timerange='09:50:00~10:20:00')
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 6552)

    def test_array(self):
        flagdata(vis=self.vis, array='0')
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)

class test_selections_alma(test_base):
    # Test various selections for alma data 
    # may be need to be merged with test_selections.

    def setUp(self):
        self.setUp_flagdatatest_alma()

    def test_scanitent(self):
        '''test scanintent selection'''
        # flag POINTING CALIBRATION scans 
        # (CALIBRATE_POINTING_.. from STATE table's OBS_MODE)
        flagdata(vis=self.vis, intent='CAL*POINT*')
        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 377280, 26200)

class test_selections2(test_base):
    '''Test other selections'''
    
    def setUp(self):
        self.setUp_multi()
        
    def test_observation(self):
        # string
        flagdata(vis=self.vis, observation='1')
        test_eq(flagdata(vis=self.vis, mode='summary'), 2882778, 28500)

        # integer
        flagdata(vis=self.vis, unflag=True)
        flagdata(vis=self.vis, observation=1)
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 28500)
        self.assertEqual(res['total'], 2882778)
        
        # non-existing ID
        flagdata(vis=self.vis, unflag=True)
        flagdata(vis=self.vis, observation='3')
        res=flagdata(vis=self.vis, mode='summary')
        self.assertEqual(res['flagged'], 0)
        self.assertEqual(res['total'], 2882778)
                

# Dummy class which cleans up created files
class cleanup(test_base):
    
    def tearDown(self):
        os.system('rm -rf ngc5921.ms')
        os.system('rm -rf ngc5921.ms.flagversions')
        os.system('rm -rf flagdatatest.ms')
        os.system('rm -rf flagdatatest.ms.flagversions')
        os.system('rm -rf missing-baseline.ms')
        os.system('rm -rf multiobs.ms')
        os.system('rm -rf flagdatatest-alma.ms')

    def test1(self):
        '''flagdata: Cleanup'''
        pass


def suite():
    return [test_selections,
            test_selections2,
            test_selections_alma,
            test_statistics_queries,
            test_vector,
            test_vector_ngc5921,
            test_flagmanager,
            test_shadow,
            test_shadow_ngc5921,
            test_msselection,
            cleanup]
