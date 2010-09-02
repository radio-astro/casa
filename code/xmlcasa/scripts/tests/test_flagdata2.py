import shutil
import unittest
import os
from tasks import *
from taskinit import *

#
# Test of flagdata2 modes
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
    def setUp_flagdata2test(self):
        self.vis = "flagdatatest.ms"

        if os.path.exists(self.vis):
            print "The MS is already around, just unflag"
        else:
            print "Moving data..."
            os.system('cp -r ' + \
                      os.environ.get('CASAPATH').split()[0] +
                      "/data/regression/unittest/flagdata/" + self.vis + ' ' + self.vis)

        os.system('rm -rf ' + self.vis + '.flagversions')
        flagdata2(vis=self.vis, unflag=True)

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
        flagdata2(vis=self.vis, unflag=True)


class test_rfi(test_base):
    """Test of mode = 'rfi'"""
    
    def setUp(self):
        self.setUp_flagdata2test()
        
    def test1(self):
        flagdata2(vis=self.vis, rfi=True)
        test_eq(flagdata2(vis=self.vis, summary=True), 70902, 1326)
        test_eq(flagdata2(vis=self.vis, selectdata=True, antenna='2', summary=True), 5252, 51)


class test_shadow(test_base):
    """Test of mode = 'shadow'"""
    def setUp(self):
        self.setUp_flagdata2test()

    def test1(self):
        flagdata2(vis=self.vis, shadow=True, diameter=40)
        test_eq(flagdata2(vis=self.vis, summary=True), 70902, 5252)

    def test2(self):
        flagdata2(vis=self.vis, shadow=True)
        test_eq(flagdata2(vis=self.vis, summary=True), 70902, 2912)

    def test3(self):
        flagdata2(vis=self.vis, selectdata=True, correlation='LL', shadow=True )
        test_eq(flagdata2(vis=self.vis, summary=True), 70902, 1456)


class test_vector(test_base):
    def setUp(self):
        self.setUp_flagdata2test()

# Not supported
#    def test1(self):
#
#        print "Test of vector mode"
#        
#        clipminmax=[0.0, 0.2]
#        antenna = ['1', '2']
#        clipcolumn = ['DATA', 'datA']
#
#        flagdata2(vis=self.vis, clip=True, selectdata=True, clipminmax=clipminmax, antenna=antenna, clipcolumn=clipcolumn)
#        test_eq(flagdata2(vis=self.vis, summary=True), 70902, 17897)

    def test2(self):
        flagdata2(vis=self.vis, manualflag=True, mf_antenna=["2", "3", "5", "6"])
        s = flagdata2(vis=self.vis, summary=True)
        for a in ["VLA3", "VLA4", "VLA6", "VLA7"]:
            self.assertEqual(s['antenna'][a]['flagged'], 5252)
        for a in ["VLA1", "VLA2", "VLA5", "VLA8", "VLA9", "VLA10", "VLA11", "VLA24"]:
            self.assertEqual(s['antenna'][a]['flagged'], 808)

class test_vector_ngc5921(test_base):
    def setUp(self):
        self.setUp_ngc5921()

    def test3(self):
        flagdata2(vis=self.vis, manualflag=True, mf_antenna=["2", "3", "5", "6"], mf_scan="4")
        s = flagdata2(vis=self.vis, summary=True)
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
            flagdata2(vis=self.vis, manualflag=True, mf_antenna=a[i], mf_timerange=t[i])

        stats_serial = flagdata2(vis=self.vis, summary=True)

        # Flag in parallel
        flagdata2(vis=self.vis, unflag=True)
        flagdata2(vis=self.vis, manualflag=True, mf_antenna=a, mf_timerange=t)
        
        stats_parallel = flagdata2(vis=self.vis, summary=True)

        # Did we get the same net results? (full recursive comparison of statistics dictionaries)
        self.assertEqual(stats_serial, stats_parallel)

        # If any corner of the dictionaries had differed, the test above would have failed
        # (just double checking)
        stats_serial["channel"]["23"]["total"] = 25703
        self.assertNotEqual(stats_serial, stats_parallel)

        stats_serial["channel"]["23"]["total"] = 25704
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
        self.setUp_flagdata2test()
        
    def test1(self):
        print "Test of flagmanager mode=list, flagbackup=True/False"
        flagmanager(vis=self.vis, mode='list')
        fg.open(self.vis)
        self.assertEqual(len(fg.getflagversionlist()), 3)
        fg.done()

        flagdata2(vis=self.vis, unflag=True, flagbackup=False)
        flagmanager(vis=self.vis, mode='list')
        fg.open(self.vis)
        self.assertEqual(len(fg.getflagversionlist()), 3)
        fg.done()

        flagdata2(vis=self.vis, unflag=True, flagbackup=True)
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

        flagdata2(vis = self.vis, summary=True)
        flagmanager(vis = self.vis)
        
        flagdata2(vis = self.vis, selectdata=True, antenna="2")
        
        flagmanager(vis = self.vis)
        ant2 = flagdata2(vis = self.vis, summary=True)['flagged']

        print "After flagging antenna 2 there were", ant2, "flags"

        # Change flags, then restore
        flagdata2(vis = self.vis, selectdata=True, antenna="3")
        flagmanager(vis = self.vis)
        ant3 = flagdata2(vis = self.vis, summary=True)['flagged']

        print "After flagging antenna 2 and 3 there were", ant3, "flags"

        flagmanager(vis = self.vis, mode='restore', versionname='manualflag_3')
        restore2 = flagdata2(vis = self.vis, summary=True)['flagged']

        print "After restoring pre-antenna 3 flagging, there are", restore2, "flags, should be", ant2

        assert restore2 == ant2

class test_msselection(test_base):

    def setUp(self):
        self.setUp_ngc5921()

    def test_simple(self):
        baselines = flagdata2(vis = self.vis, selectdata=True, antenna="9", summary=True )['baseline'].keys()
#        assert "9&&9" not in baselines
        assert "9&&9" in baselines
        assert "9&&10" in baselines
        assert "9&&11" in baselines
        assert "10&&10" not in baselines
        assert "10&&11" not in baselines

        baselines = flagdata2(vis = self.vis, selectdata=True, antenna="9,10", summary=True )['baseline'].keys()
#        assert "9&&9" not in baselines
        assert "9&&9" in baselines
        assert "9&&10" in baselines
        assert "9&&11" in baselines
#        assert "10&&10" not in baselines
        assert "10&&10" in baselines
        assert "10&&11" in baselines

    def test_amp(self):
#        baselines = flagdata2(vis = self.vis, selectdata=True, antenna="9&", summary=True)['baseline'].keys()
        #??? assert "9&&9" not in baselines
        #assert "9&&10" not in baselines
        #assert "9&&11" not in baselines
        #assert "10&&10" not in baselines
        #assert "10&&11" not in baselines

        baselines = flagdata2(vis = self.vis, selectdata=True, antenna="9,10&", summary=True)['baseline'].keys()
        assert "9&&9" not in baselines
        assert "9&&10" in baselines
        assert "9&&11" not in baselines
        assert "10&&10" not in baselines
        assert "10&&11" not in baselines

        baselines = flagdata2(vis = self.vis, selectdata=True, antenna="9&10", summary=True)['baseline'].keys()
        assert "9&&9" not in baselines
        assert "9&&10" in baselines
        assert "9&&11" not in baselines
        assert "10&&10" not in baselines
        assert "10&&11" not in baselines

class test_autoflag(test_base):

    def setUp(self):
        self.setUp_ngc5921()

    def test_CAS1979(self):
        """Test that autoflagging does not clear flags"""
        s0 = flagdata2(vis=self.vis, summary=True)['flagged']
        flagdata2(vis=self.vis, autoflag=True, algorithm="freqmed", selectdata=True,field="0", spw="0")
        s1 = flagdata2(vis=self.vis, summary=True)['flagged']
        flagdata2(vis=self.vis, autoflag=True, algorithm="freqmed", selectdata=True,field="0", spw="0")
        s2 = flagdata2(vis=self.vis, summary=True)['flagged']
        flagdata2(vis=self.vis, autoflag=True, algorithm="freqmed", selectdata=True,field="0", spw="0")
        s3 = flagdata2(vis=self.vis, summary=True)['flagged']
        flagdata2(vis=self.vis, autoflag=True, algorithm="freqmed", selectdata=True,field="0", spw="0")
        s4 = flagdata2(vis=self.vis, summary=True)['flagged']
        flagdata2(vis=self.vis, autoflag=True, algorithm="freqmed", selectdata=True,field="0", spw="0")
        s5 = flagdata2(vis=self.vis, summary=True)['flagged']
        flagdata2(vis=self.vis, autoflag=True, algorithm="freqmed", selectdata=True,field="0", spw="0")
        s6 = flagdata2(vis=self.vis, summary=True)['flagged']

        assert s0 == 0
        assert s0 <= s1
        assert s1 <= s2
        assert s2 <= s3
        assert s3 <= s4
        assert s4 <= s5
        assert s5 <= s6

    def test1(self):
        print "Test of autoflag, algorithm=timemed"
        flagdata2(vis=self.vis, autoflag=True, algorithm='timemed', window=3)
        test_eq(flagdata2(vis=self.vis, summary=True), 2854278, 4725)

    def test2(self):
        print "Test of autoflag, algorithm=freqmed"
        flagdata2(vis=self.vis, autoflag=True, algorithm='freqmed')
        test_eq(flagdata2(vis=self.vis, summary=True), 2854278, 29101)

class test_statistics_queries(test_base):

    def setUp(self):
        self.setUp_ngc5921()

    def test_CAS2021(self):
        print "Test antenna selection"
        flagdata2(vis=self.vis, selectdata=True,antenna='!5',manualflag=True) # should not crash

    def test_CAS2212(self):
        print "Test scan + clipping"
        flagdata2(vis=self.vis, selectdata=True,scan="2", clip=True, clipminmax = [0.2, 0.3]) # should not crash
    
    def test021(self):
        print "Test of flagging statistics and queries"
        
        #It's failing because the first call to flagdata2 doesn't work
        flagdata2(vis=self.vis, selectdata=True,correlation='LL',manualflag=True, mf_correlation='LL')
        flagdata2(vis=self.vis, selectdata=True,spw='0',manualflag=True, mf_spw='0:17~19')
        flagdata2(vis=self.vis, selectdata=True,antenna='5&&9',manualflag=True)
        flagdata2(vis=self.vis, selectdata=True,antenna='14',manualflag=True)
        flagdata2(vis=self.vis, selectdata=True,field='1',manualflag=True)
        s = flagdata2(vis=self.vis, summary=True, minrel=0.9)
        assert s['antenna'].keys() == ['14']
        assert '5&&9' in s['baseline'].keys()
        assert set(s['channel'].keys()) == set(['17', '18', '19'])
        assert s['correlation'].keys() == ['1']  # LL
        assert s['field'].keys() == ['1']
        assert set(s['scan'].keys()) == set(['2', '4', '5', '7']) # field 1
        s = flagdata2(vis=self.vis, summary=True, maxrel=0.8)
        assert set(s['field'].keys()) == set(['0', '2'])
        s = flagdata2(vis=self.vis, summary=True, minabs=400000)
        assert set(s['scan'].keys()) == set(['3', '6'])
        s = flagdata2(vis=self.vis, summary=True, minabs=400000, maxabs=450000)
        assert s['scan'].keys() == ['3']

    def test4(self):
        print "Test of channel average"
        flagdata2(vis=self.vis, clip=True, channelavg=False, clipminmax=[30, 60])
        test_eq(flagdata2(vis=self.vis, summary=True), 2854278, 1414186)

    def test5(self):
        flagdata2(vis=self.vis, clip=True, channelavg=True, clipminmax=[30, 60])
        test_eq(flagdata2(vis=self.vis, summary=True), 2854278, 1347822)

    def test6(self):
        flagdata2(vis=self.vis, clip=True, channelavg=False, clipminmax=[30, 60], spw='0:0~10')
        test_eq(flagdata2(vis=self.vis, summary=True), 2854278, 242053)

    def test7(self):
        flagdata2(vis=self.vis, clip=True, channelavg=True, clipminmax=[30, 60], spw='0:0~10')
        test_eq(flagdata2(vis=self.vis, summary=True), 2854278, 231374)
               

    def test8(self):
        print "Test of parallel manualflagging"
        flagdata2(vis=self.vis, manualflag=True, mf_correlation='RR', mf_antenna=['2', '3'])
        test_eq(flagdata2(vis=self.vis, summary=True), 2854278, 192654)
#        print "Test of mode = 'quack'"
#        print "parallel quack"
#        flagdata2(vis=self.vis, quack=True, quackinterval=[1.0, 5.0], selectdata=True, antenna=['2', '3'], 
#                  correlation='RR')
#        test_eq(flagdata2(vis=self.vis, summary=True), 2854278, 22365)

    def test9(self):
        flagdata2(vis=self.vis, quack=True, quackmode='beg', quackinterval=1)
        test_eq(flagdata2(vis=self.vis, summary=True), 2854278, 329994)

    def test10(self):
        flagdata2(vis=self.vis, quack=True, quackmode='endb', quackinterval=1)
        test_eq(flagdata2(vis=self.vis, summary=True), 2854278, 333396)

    def test11(self):
        flagdata2(vis=self.vis, quack=True, quackmode='end', quackinterval=1)
        test_eq(flagdata2(vis=self.vis, summary=True), 2854278, 2520882)

    def test12(self):
        flagdata2(vis=self.vis, quack=True, quackmode='tail', quackinterval=1)
        test_eq(flagdata2(vis=self.vis, summary=True), 2854278, 2524284)

    def test13(self):
        print "quack mode quackincrement"
        flagdata2(vis=self.vis, quack=True, quackinterval=50, quackmode='endb', quackincrement=True)
        test_eq(flagdata2(vis=self.vis, summary=True), 2854278, 571536)

        flagdata2(vis=self.vis, quack=True, quackinterval=20, quackmode='endb', quackincrement=True)
        test_eq(flagdata2(vis=self.vis, summary=True), 2854278, 857304)
        
        flagdata2(vis=self.vis, quack=True, quackinterval=150, quackmode='endb', quackincrement=True)
        test_eq(flagdata2(vis=self.vis, summary=True), 2854278, 1571724)
        
        flagdata2(vis=self.vis, quack=True, quackinterval=50, quackmode='endb', quackincrement=True)
        test_eq(flagdata2(vis=self.vis, summary=True), 2854278, 1762236)
        flagdata2(vis=self.vis, unflag=True)



class test_selections(test_base):
    """Test various selections"""

    def setUp(self):
        self.setUp_ngc5921()

    def test_scan1(self):
        '''Flagdata: scan='3' manualflag=true'''
        flagdata2(vis=self.vis, selectdata=True, scan='3', manualflag=True)
        test_eq(flagdata2(vis=self.vis, summary=True, selectdata=True, antenna='2'), 196434, 52416)
        
        # feed not implemented flagdata2(vis=vis, feed='27')
        # flagdata2(vis=vis, unflag=True)

    def test_antenna(self):
        '''Flagdata2: antenna=2 manualflag=true'''
        flagdata2(vis=self.vis, selectdata=True, antenna='2', manualflag=True)
        test_eq(flagdata2(vis=self.vis, summary=True, antenna='2'), 196434, 196434)

    def test_spw(self):
        '''Flagdata2: spw=0 manualflag=true'''
        flagdata2(vis=self.vis, selectdata=True, spw='0', manualflag=True)
        test_eq(flagdata2(vis=self.vis, summary=True, antenna='2'), 196434, 196434)

    def test_correlation(self):
        '''Flagdata2: correlation=LL manualflag=true'''
        flagdata2(vis=self.vis, selectdata=True, correlation='LL', manualflag=True)
        test_eq(flagdata2(vis=self.vis, summary=True, selectdata=True, antenna='2'), 196434, 98217)
        flagdata2(vis=self.vis, selectdata=True, correlation='LL,RR', manualflag=True)
        flagdata2(vis=self.vis, selectdata=True, correlation='LL RR', manualflag=True)
        flagdata2(vis=self.vis, selectdata=True, correlation='LL ,, ,  ,RR', manualflag=True)
        test_eq(flagdata2(vis=self.vis, summary=True, selectdata=True, antenna='2'), 196434, 196434)

    def test_field(self):
        '''Flagdata2: field=0 manualflag=true'''
        flagdata2(vis=self.vis, selectdata=True, field='0', manualflag=True)
        test_eq(flagdata2(vis=self.vis, summary=True, selectdata=True, antenna='2'), 196434, 39186)

    def test_uvrange(self):
        '''Flagdata2: uvrange=200~400m manualflag=true'''
        flagdata2(vis=self.vis, selectdata=True, uvrange='200~400m', manualflag=True)
        test_eq(flagdata2(vis=self.vis, summary=True, selectdata=True, antenna='2'), 196434, 55944)

    def test_timerange(self):
        '''Flagdata2: timerange=09:50:00~10:20:00 manualflag=true'''
        flagdata2(vis=self.vis, selectdata=True, timerange='09:50:00~10:20:00', manualflag=True)
        test_eq(flagdata2(vis=self.vis, summary=True, antenna='2'), 196434, 6552)

    def test_array(self):
        '''Flagdata2: array=0 manualflag=true'''
        flagdata2(vis=self.vis, selectdata=True, array='0', manualflag=True)
        test_eq(flagdata2(vis=self.vis, summary=True, selectdata=True, antenna='2'), 196434, 196434)

class test_default(test_base):
    def setUp(self):
        self.setUp_ngc5921()
        
    def test_default(self):
        '''Flagdata without any mode'''
        ret = flagdata2(vis=self.vis)
        
        
class test_multimodes(test_base):

    def setUp(self):
        self.setUp_ngc5921()

    def test_manual_auto(self):
        '''Manualflag and autoflag modes'''
        # It will run flagdata then compare with flagdata2
        flagdata(vis=self.vis, scan='3')
        flagdata(vis=self.vis, scan='3', mode='autoflag', algorithm='timemed', window=3)
        res1 = flagdata(vis=self.vis, mode='summary')
        
        flagdata2(vis=self.vis, unflag=True)
        flagdata2(vis=self.vis,selectdata=True,scan='3',autoflag=True,algorithm='timemed',window=3)
        res2 = flagdata2(vis=self.vis, summary=True)
        
        print res1['flagged']
        print res2['flagged']
        
        # run clip mode only and verify
        flagdata2(vis=self.vis, selectdata=True, antenna="2,3,5,6", scan="4", manualflag=True)
        s = flagdata2(vis=self.vis, summary=True)
        for a in ["2", "3", "5", "6"]:
            self.assertEqual(s['antenna'][a]['flagged'], 6552)
        for a in ["1", "4", "7", "8", "9", "10", "24"]:
            self.assertEqual(s['antenna'][a]['flagged'], 1008)
            
        # unflag all and verify
        flagdata2(vis=self.vis, unflag=True)
        reset = flagdata2(vis=self.vis, summary=True)
        for a in ["2", "3", "5", "6"]:
            self.assertEqual(reset['antenna'][a]['flagged'], 0)
        for a in ["1", "4", "7", "8", "9", "10", "24"]:
            self.assertEqual(reset['antenna'][a]['flagged'], 0)
            
        # run autoflag mode only and verify
        flagdata2(vis=self.vis, selectdata=True, autoflag=True, algorithm='freqmed')
        auto = flagdata2(vis=self.vis, summary=True)
        print auto['antenna']["2"]
        self.assertEqual(auto['antenna']["2"]['flagged'], 1262)

        # unflag all and verify
        flagdata2(vis=self.vis, unflag=True)
        reset = flagdata2(vis=self.vis, summary=True)
        for a in ["2", "3", "5", "6"]:
            self.assertEqual(reset['antenna'][a]['flagged'], 0)
        for a in ["1", "4", "7", "8", "9", "10", "24"]:
            self.assertEqual(reset['antenna'][a]['flagged'], 0)
        
        # run clip and autoflag
#        flagdata2(vis=self.vis, selectdata=True, antenna="2,3,5,6", scan="4", autoflag=True, algorithm='freqmed')

class test_mfselections(test_base):
    """Test various manualflag selections"""

    def setUp(self):
        self.setUp_ngc5921()

    def test_mfscan(self):
        flagdata2(vis=self.vis, selectdata=True, scan='2~6', manualflag=True, mf_scan='3')
        test_eq(flagdata2(vis=self.vis, summary=True, selectdata=True, scan='2'), 238140, 0)
        test_eq(flagdata2(vis=self.vis, summary=True, selectdata=True, scan='3'), 762048, 762048)
        test_eq(flagdata2(vis=self.vis, summary=True, selectdata=True, scan='4'), 95256, 0)
        test_eq(flagdata2(vis=self.vis, summary=True, selectdata=True, scan='5'), 142884, 0)
        test_eq(flagdata2(vis=self.vis, summary=True, selectdata=True, scan='6'), 857304, 0)
        
        # feed not implemented flagdata2(vis=vis, feed='27')
        # flagdata2(vis=vis, unflag=True)

    def test_mfantenna(self):
        flagdata2(vis=self.vis, selectdata=True, antenna='1~20', manualflag=True, mf_antenna='3~8')
        test_eq(flagdata2(vis=self.vis, summary=True, antenna='5'), 196434, 196434)
        test_eq(flagdata2(vis=self.vis, summary=True, antenna='9'), 196434, 45360)
        
        # compare with original flagdata
        flagdata2(vis=self.vis, unflag=True)
        flagdata(vis=self.vis, selectdata=True, antenna='3~8')
        test_eq(flagdata(vis=self.vis, mode='summary', selectdata=True, antenna='5'), 196434, 196434)
        test_eq(flagdata(vis=self.vis, mode='summary', selectdata=True,antenna='9'), 196434, 45360)

#FIXME: finish the next methods
    def test_channel(self):
        # should flag only channels 10~15
        flagdata2(vis=self.vis, selectdata=True, spw='0:10~20', manualflag=True, mf_spw='0:5~15')
        test_eq(flagdata2(vis=self.vis, summary=True, antenna='2'), 196434, 34298)

    def test_correlation(self):
        flagdata2(vis=self.vis, selectdata=True, correlation='LL', manualflag=True, mf_correlation='')
        test_eq(flagdata2(vis=self.vis, summary=True, selectdata=True, antenna='2'), 196434, 98217)
        flagdata2(vis=self.vis, selectdata=True, correlation='LL,RR', manualflag=True)
        flagdata2(vis=self.vis, selectdata=True, correlation='LL RR', manualflag=True)
        flagdata2(vis=self.vis, selectdata=True, correlation='LL ,, ,  ,RR', manualflag=True)
        test_eq(flagdata2(vis=self.vis, summary=True, selectdata=True, antenna='2'), 196434, 196434)

    def test_field(self):
        # It should not flag field=0
        flagdata2(vis=self.vis, selectdata=True, field='1~2', manualflag=True, mf_field='0~1')
        test_eq(flagdata2(vis=self.vis, summary=True), 2854278, 666792)
        test_eq(flagdata2(vis=self.vis, summary=True, selectdata=True, field='0'), 568134, 0)

    def test_uvrange(self):
        flagdata2(vis=self.vis, selectdata=True, uvrange='200~400m', manualflag=True)
        test_eq(flagdata2(vis=self.vis, summary=True, selectdata=True, antenna='2'), 196434, 55944)

    def test_timerange(self):
        flagdata2(vis=self.vis, selectdata=True, timerange='09:50:00~10:20:00', manualflag=True,
                  mf_timerange='09:20:00~10:20:00')
        test_eq(flagdata2(vis=self.vis, summary=True, antenna='2'), 196434, 6552)

    def test_array(self):
        flagdata2(vis=self.vis, selectdata=True, array='0', manualflag=True)
        test_eq(flagdata2(vis=self.vis, summary=True, selectdata=True, antenna='2'), 196434, 196434)
     
# Dummy class which cleans up created files
class cleanup(test_base):
    
    def tearDown(self):
        os.system('rm -rf ngc5921.ms')
        os.system('rm -rf ngc5921.ms.flagversions')
        os.system('rm -rf flagdatatest.ms')
        os.system('rm -rf flagdatatest.ms.flagversions')

    def test1(self):
        '''flagdata2: Cleanup'''
        pass


def suite():
    return [
#            test_selections,
            test_statistics_queries,
#             test_vector,
#             test_vector_ngc5921,
#             test_flagmanager,
#            test_rfi,
#            test_shadow,
            test_msselection,
#            test_autoflag,
#            test_multimodes,
#            test_mfselections,
  #          test_default,
            cleanup]
