import shutil
import unittest
import os
from tasks import *
from taskinit import *

#
# Test of tflagger modes
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
        tflagger(vis=self.vis, mode='unflag')

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


class test_tfcrop(test_base):
    """tflagger:: Test of mode = 'tfcrop'"""
    
    def setUp(self):
        self.setUp_data4tfcrop()
        
    def test1(self):
        '''tflagger:: Test1 of mode = tfcrop'''
        tflagger(vis=self.vis, mode='tfcrop', expression='ABS RR',ntime=51.0,spw='9')
        test_eq(tflagger(vis=self.vis, mode='summary'), 4399104, 4489)
        test_eq(tflagger(vis=self.vis, mode='summary', antenna='ea19'), 2199552, 2294)
        test_eq(tflagger(vis=self.vis, mode='summary', spw='7'), 274944, 0)
        
    def test2(self):
        '''tflagger:: Test2 of mode = tfcrop ABS ALL'''
        tflagger(vis=self.vis, mode='tfcrop',ntime=51.0,spw='9')
        test_eq(tflagger(vis=self.vis, mode='summary'), 4399104, 18696)
        test_eq(tflagger(vis=self.vis, mode='summary', correlation='LL'), 1099776, 4258)
        test_eq(tflagger(vis=self.vis, mode='summary', correlation='RL'), 1099776, 4999)
        test_eq(tflagger(vis=self.vis, mode='summary', correlation='LR'), 1099776, 4950)
        test_eq(tflagger(vis=self.vis, mode='summary', correlation='RR'), 1099776, 4489)


class test_shadow(test_base):
    def setUp(self):
        self.setUp_flagdatatest()

    def test1(self):
        '''tflagger:: Test1 of mode = shadow'''
        tflagger(vis=self.vis, mode='shadow', diameter=40)
        test_eq(tflagger(vis=self.vis, mode='summary'), 70902, 5252)

    def test2(self):
        """tflagger:: Test2 of mode = shadow"""
        tflagger(vis=self.vis, mode='shadow')
        test_eq(tflagger(vis=self.vis, mode='summary'), 70902, 2912)

    def test3(self):
        """tflagger:: Test3 of mode = shadow"""
        tflagger(vis=self.vis, mode='shadow', correlation='LL')
        test_eq(tflagger(vis=self.vis, mode='summary'), 70902, 1456)


#        # This MS seems to give wrong results with the old flagdata
#        # compared to tflagger. Will remove this test and use a different
#        # MS
#
#class test_shadow_ngc5921(test_base):
#    """More test of mode = 'shadow'"""
#    def setUp(self):
#        self.setUp_ngc5921()

#    def test_CAS2399(self):
#        
#        tflagger(vis = self.vis, mode='unflag')
#        tflagger(vis = self.vis,mode = "shadow",diameter = 35)
#        allbl = tflagger(vis = self.vis,mode = "summary")
#
#        # Sketch of what is being shadowed:
#        #
#        #  A23 shadowed by A1
#        #
#        #  A13 shadowed by A2 shadowed by A9
#        #
#        
#        # Now remove baseline 2-13   (named 3-14)
#        outputvis = "missing-baseline.ms"
#        os.system("rm -rf " + outputvis)
#        split(vis = self.vis, 
#              outputvis = outputvis,
#              datacolumn = "data",
#              antenna = "!3&&14")
#        
#        tflagger(vis = outputvis, mode='unflag')
#        tflagger(vis = outputvis,mode = "shadow",diameter = 35)
#        
#        missingbl = tflagger(vis = outputvis,mode = "summary")
#
#        # With baseline based flagging, A13 will not get flagged
#        # when the baseline is missing
#        #
#        # With antenna based flagging, A13 should be flagged
#        
#        assert allbl['antenna']['3']['flagged'] > 1000
#        assert allbl['antenna']['24']['flagged'] > 1000
#        
#        assert missingbl['antenna']['3']['flagged'] > 1000
#        assert missingbl['antenna']['24']['flagged'] == allbl['antenna']['24']['flagged']
#        
#        assert allbl['antenna']['14']['flagged'] > 1000
#        # When the baseline is missing, the antenna is not flagged as before
#        assert missingbl['antenna']['14']['flagged'] < 1000
#        
#        # For antenna based flagging, it should be (to be uncommented when CAS-2399
#        # is solved):
#        #assert missingbl['antenna']['14']['flagged'] > 1000

#    def test1(self):
#        tflagger(vis = self.vis, mode = "shadow", diameter = 50)
#
#        s = flagdata(vis = self.vis, mode = "summary")['antenna']
#
#        assert s['1']['flagged'] == 58968; assert s['1']['total'] == 203994
#        assert s['10']['flagged'] == 117432; assert s['10']['total'] == 203994
#        assert s['11']['flagged'] == 175392; assert s['11']['total'] == 203994
#        assert s['12']['flagged'] == 58968; assert s['12']['total'] == 203994
#        assert s['13']['flagged'] == 203994; assert s['13']['total'] == 203994
#        assert s['14']['flagged'] == 203994; assert s['14']['total'] == 203994
#        assert s['15']['flagged'] == 152838; assert s['15']['total'] == 203994
#        assert s['16']['flagged'] == 58968; assert s['16']['total'] == 203994
#        assert s['17']['flagged'] == 57960; assert s['17']['total'] == 200718
#        assert s['18']['flagged'] == 58968; assert s['18']['total'] == 203994
#        assert s['19']['flagged'] == 58968; assert s['19']['total'] == 203994
#        assert s['2']['flagged'] == 203994; assert s['2']['total'] == 203994
#        assert s['20']['flagged'] == 58968; assert s['20']['total'] == 203994
#        assert s['21']['flagged'] == 58968; assert s['21']['total'] == 203994
#        assert s['22']['flagged'] == 58968; assert s['22']['total'] == 203994
#        assert s['24']['flagged'] == 203994; assert s['24']['total'] == 203994
#        assert s['25']['flagged'] == 58968; assert s['25']['total'] == 203994
#        assert s['26']['flagged'] == 58968; assert s['26']['total'] == 203994
#        assert s['27']['flagged'] == 58968; assert s['27']['total'] == 203994
#        assert s['28']['flagged'] == 58968; assert s['28']['total'] == 203994
#        assert s['3']['flagged'] == 203994; assert s['3']['total'] == 203994
#        assert s['4']['flagged'] == 87570; assert s['4']['total'] == 203994
#        assert s['5']['flagged'] == 163674; assert s['5']['total'] == 203994
#        assert s['6']['flagged'] == 58968; assert s['6']['total'] == 203994
#        assert s['7']['flagged'] == 58968; assert s['7']['total'] == 203994
#        assert s['8']['flagged'] == 58968; assert s['8']['total'] == 203994
#        assert s['9']['flagged'] == 58968; assert s['9']['total'] == 203994


#class test_flagmanager(test_base):
#    def setUp(self):
#        os.system("rm -rf flagdatatest.ms*") # test1 needs a clean start
#        self.setUp_flagdatatest()
#        
#    def test1(self):
#        print "Test of flagmanager mode=list, flagbackup=True/False"
#        flagmanager(vis=self.vis, mode='list')
#        fg.open(self.vis)
#        self.assertEqual(len(fg.getflagversionlist()), 3)
#        fg.done()
#
#        flagdata(vis=self.vis, unflag=True, flagbackup=False)
#        flagmanager(vis=self.vis, mode='list')
#        fg.open(self.vis)
#        self.assertEqual(len(fg.getflagversionlist()), 3)
#        fg.done()
#
#        flagdata(vis=self.vis, unflag=True, flagbackup=True)
#        flagmanager(vis=self.vis, mode='list')
#        fg.open(self.vis)
#        self.assertEqual(len(fg.getflagversionlist()), 4)
#        fg.done()
#
#        print "Test of flagmanager mode=rename"
#        flagmanager(vis=self.vis, mode='rename', oldname='manualflag_2', versionname='Ha! The best version ever!', comment='This is a *much* better name')
#        flagmanager(vis=self.vis, mode='list')
#        fg.open(self.vis)
#        self.assertEqual(len(fg.getflagversionlist()), 4)
#        fg.done()
#
#    def test2(self):
#        """Create, then restore autoflag"""
#
#        flagdata(vis = self.vis, mode='summary')
#        flagmanager(vis = self.vis)
#        
#        flagdata(vis = self.vis, antenna="2")
#        
#        flagmanager(vis = self.vis)
#        ant2 = flagdata(vis = self.vis, mode='summary')['flagged']
#
#        print "After flagging antenna 2 there were", ant2, "flags"
#
#        # Change flags, then restore
#        flagdata(vis = self.vis, antenna="3")
#        flagmanager(vis = self.vis)
#        ant3 = flagdata(vis = self.vis, mode='summary')['flagged']
#
#        print "After flagging antenna 2 and 3 there were", ant3, "flags"
#
#        flagmanager(vis = self.vis, mode='restore', versionname='manualflag_3')
#        restore2 = flagdata(vis = self.vis, mode='summary')['flagged']
#
#        print "After restoring pre-antenna 3 flagging, there are", restore2, "flags, should be", ant2
#
#        assert restore2 == ant2
#
#    def test_CAS2701(self):
#        """Do not allow flagversion=''"""
#
#        
#        fg.open(self.vis)
#        l = len(fg.getflagversionlist())
#        fg.done()
#        
#        flagmanager(vis = self.vis,
#                    mode = "save",
#                    versionname = "non-empty-string")
#
#        fg.open(self.vis)
#        self.assertEqual(len(fg.getflagversionlist()), l+1)
#        fg.done()
#
#        flagmanager(vis = self.vis,
#                    mode = "save",
#                    versionname = "non-empty-string")
#
#        fg.open(self.vis)
#        self.assertEqual(len(fg.getflagversionlist()), l+1)
#        fg.done()


class test_msselection(test_base):

    def setUp(self):
        self.setUp_ngc5921()

    def test_simple(self):
        '''tflagger: select only cross-correlations'''
        baselines = tflagger(vis = self.vis, mode="summary", antenna="9")['baseline'].keys()
        assert "9&&9" not in baselines
        assert "9&&10" in baselines
        assert "9&&11" in baselines
        assert "10&&10" not in baselines
        assert "10&&11" not in baselines


        baselines = tflagger(vis = self.vis, mode="summary", antenna="9,10")['baseline'].keys()
        assert "9&&9" not in baselines
        assert "9&&10" in baselines
        assert "9&&11" in baselines
        assert "10&&10" not in baselines
        assert "10&&11" in baselines

    def test_amp(self):
        '''tflagger: select only cross-correlations'''
        baselines = tflagger(vis = self.vis, mode="summary", antenna="9,10&")['baseline'].keys()
        assert "9&&9" not in baselines
        assert "9&&10" in baselines
        assert "9&&11" not in baselines
        assert "10&&10" not in baselines
        assert "10&&11" not in baselines

        baselines = tflagger(vis = self.vis, mode="summary", antenna="9&10")['baseline'].keys()
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
        '''tflagger: test antenna negation selection'''
        
        tflagger(vis=self.vis, antenna='!5') 
        s = tflagger(vis = self.vis, mode="summary")['baseline']
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
        '''tflagger: Clipping scan selection, CAS-2212, CAS-3496'''
        # By default expression='ABS ALL'
        tflagger(vis=self.vis, mode='clip', scan="2", clipminmax = [0.2, 0.3]) 
        test_eq(tflagger(vis=self.vis, mode='summary'), 2854278, 85404)
        s = tflagger(vis=self.vis, mode='summary')['scan']
        
        # Make sure no other scan is clipped
        assert s['1']['flagged'] == 0
        assert s['3']['flagged'] == 0
        assert s['4']['flagged'] == 0
        assert s['5']['flagged'] == 0
        assert s['6']['flagged'] == 0
        assert s['7']['flagged'] == 0
        assert s['2']['flagged'] == 85404
  
    def test021(self):
        '''tflagger: Test of flagging statistics and queries'''
        
        tflagger(vis=self.vis, correlation='LL')
        tflagger(vis=self.vis, spw='0:17~19')
        tflagger(vis=self.vis, antenna='5&&9')
        tflagger(vis=self.vis, antenna='14')
        tflagger(vis=self.vis, field='1')
        s = tflagger(vis=self.vis, mode='summary', minrel=0.9, spwchan=True)
        assert s['antenna'].keys() == ['14']
        assert '5&&9' in s['baseline'].keys()
        assert set(s['spw:channel'].keys()) == set(['0:17', '0:18', '0:19'])
        assert s['correlation'].keys() == ['LL']  # LL
        assert s['field'].keys() == ['1445+09900002_0']
        assert set(s['scan'].keys()) == set(['2', '4', '5', '7']) # field 1
        s = tflagger(vis=self.vis, mode='summary', maxrel=0.8)
        assert set(s['field'].keys()) == set(['1331+30500002_0', 'N5921_2'])
        s = tflagger(vis=self.vis, mode='summary', minabs=400000)
        assert set(s['scan'].keys()) == set(['3', '6'])
        s = tflagger(vis=self.vis, mode='summary', minabs=400000, maxabs=450000)
        assert s['scan'].keys() == ['3']

    def test4(self):
        print "Test of channel average"
        tflagger(vis=self.vis, mode='clip',channelavg=False, clipminmax=[30., 60.], expression='ABS RR')
        test_eq(tflagger(vis=self.vis, mode='summary'), 2854278, 1414186)

    def test5(self):
        tflagger(vis=self.vis, mode='clip',channelavg=True, clipminmax=[30., 60.], expression='ABS RR')
        test_eq(tflagger(vis=self.vis, mode='summary'), 2854278, 1347822)

    def test6(self):
        tflagger(vis=self.vis, mode='clip',channelavg=False, clipminmax=[30., 60.], spw='0:0~10', 
                 expression='ABS RR')
        test_eq(tflagger(vis=self.vis, mode='summary'), 2854278, 242053)

    def test7(self):
        tflagger(vis=self.vis, mode='clip',channelavg=True, clipminmax=[30., 60.], spw='0:0~10',
                 expression='ABS RR')
        test_eq(tflagger(vis=self.vis, mode='summary'), 2854278, 231374)
               

#    def test8(self):
#        print "Test of mode = 'quack'"
#        print "parallel quack"
#        flagdata(vis=self.vis, mode='quack', quackinterval=[1.0, 5.0], antenna=['2', '3'], correlation='RR')
#        test_eq(flagdata(vis=self.vis, mode='summary'), 2854278, 22365)
#
    def test9(self):
        '''tflagger: quack mode'''
        tflagger(vis=self.vis, mode='quack', quackmode='beg', quackinterval=1)
        test_eq(tflagger(vis=self.vis, mode='summary'), 2854278, 329994)

    def test10(self):
        '''tflagger: quack mode'''
        tflagger(vis=self.vis, mode='quack', quackmode='endb', quackinterval=1)
        test_eq(tflagger(vis=self.vis, mode='summary'), 2854278, 333396)

    def test11(self):
        '''tflagger: quack mode'''
        tflagger(vis=self.vis, mode='quack', quackmode='end', quackinterval=1)
        test_eq(tflagger(vis=self.vis, mode='summary'), 2854278, 2520882)

    def test12(self):
        '''tflagger: quack mode'''
        tflagger(vis=self.vis, mode='quack', quackmode='tail', quackinterval=1)
        test_eq(tflagger(vis=self.vis, mode='summary'), 2854278, 2524284)

    def test13(self):
        '''tflagger: quack mode, quackincrement'''
        tflagger(vis=self.vis, mode='quack', quackinterval=50, quackmode='endb', quackincrement=True)
        test_eq(tflagger(vis=self.vis, mode='summary'), 2854278, 571536)

        tflagger(vis=self.vis, mode='quack', quackinterval=20, quackmode='endb', quackincrement=True)
        test_eq(tflagger(vis=self.vis, mode='summary'), 2854278, 857304)
        
        tflagger(vis=self.vis, mode='quack', quackinterval=150, quackmode='endb', quackincrement=True)
        test_eq(tflagger(vis=self.vis, mode='summary'), 2854278, 1571724)
        
        tflagger(vis=self.vis, mode='quack', quackinterval=50, quackmode='endb', quackincrement=True)
        test_eq(tflagger(vis=self.vis, mode='summary'), 2854278, 1762236)
        tflagger(vis=self.vis, mode='unflag')



class test_selections(test_base):
    """Test various selections"""

    def setUp(self):
        self.setUp_ngc5921()

    def test_scan(self):
        '''tflagger: scan selection'''
        tflagger(vis=self.vis, scan='3')
        test_eq(tflagger(vis=self.vis, mode='summary', antenna='2'), 196434, 52416)
        
        # feed not implemented flagdata(vis=vis, feed='27')
        # flagdata(vis=vis, unflag=True)

    def test_antenna(self):
        '''tflagger: antenna selection'''
        tflagger(vis=self.vis, antenna='2')
        test_eq(tflagger(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)

    def test_spw(self):
        '''tflagger: spw selection'''
        tflagger(vis=self.vis, spw='0')
        test_eq(tflagger(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)

    def test_correlation(self):
        tflagger(vis=self.vis, correlation='LL')
        test_eq(tflagger(vis=self.vis, mode='summary', antenna='2'), 196434, 98217)
        test_eq(tflagger(vis=self.vis, mode='summary', correlation='RR'), 1427139, 0)
        tflagger(vis=self.vis, mode='unflag')
        tflagger(vis=self.vis, correlation='LL,RR')
        test_eq(tflagger(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)
#        flagdata(vis=self.vis, correlation='LL RR')
#        flagdata(vis=self.vis, correlation='LL ,, ,  ,RR')
#        test_eq(flagdata(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)

    def test_field(self):
        '''tflagger: field selection'''
        tflagger(vis=self.vis, field='0')
        test_eq(tflagger(vis=self.vis, mode='summary', antenna='2'), 196434, 39186)

    def test_uvrange(self):
        '''tflagger: uvrange selection'''
        tflagger(vis=self.vis, uvrange='200~400m')
        test_eq(tflagger(vis=self.vis, mode='summary', antenna='2'), 196434, 55944)

    def test_timerange(self):
        '''tflagger: timerange selection'''
        tflagger(vis=self.vis, timerange='09:50:00~10:20:00')
        test_eq(tflagger(vis=self.vis, mode='summary', antenna='2'), 196434, 6552)

    def test_array(self):
        '''tflagger: array selection'''
        tflagger(vis=self.vis, array='0')
        test_eq(tflagger(vis=self.vis, mode='summary', antenna='2'), 196434, 196434)
        
    def test_ntime1(self):
        '''tflagger: ntime = 0'''
        ret = tflagger(vis=self.vis, ntime = 0)
        self.assertNotEqual(type(ret), dict, 'Return type of task should be None')

class test_selections_alma(test_base):
    # Test various selections for alma data 

    def setUp(self):
        self.setUp_flagdatatest_alma()

    def test_scanitent(self):
        '''tflagger: scanintent selection'''
        # flag POINTING CALIBRATION scans 
        # (CALIBRATE_POINTING_.. from STATE table's OBS_MODE)
        tflagger(vis=self.vis, intent='CAL*POINT*')
        test_eq(tflagger(vis=self.vis, mode='summary', antenna='2'), 377280, 26200)
        
    def test_wvr(self):
        '''tflagger: flag WVR correlation'''
        tflagger(vis=self.vis, correlation='I')
        test_eq(tflagger(vis=self.vis, mode='summary'),1154592, 22752)

    def test_abs_wvr(self):
        '''tflagger: clip ABS WVR'''
        tflagger(vis=self.vis, mode='clip',clipminmax=[0,50], expression='ABS WVR')
        test_eq(tflagger(vis=self.vis, mode='summary'),1154592, 22752)
        
    def test_abs_i(self):
        '''tflagger: clip ABS I. Do not flag WVR'''
        tflagger(vis=self.vis, mode='clip', clipminmax=[0,50], expression='ABS I')
        test_eq(tflagger(vis=self.vis, mode='summary'),1154592, 0)

    def test_abs_all(self):
        '''tflagger: clip ABS ALL. Do not flag WVR'''
        tflagger(vis=self.vis, mode='clip', clipminmax=[0,1], expression='ABS ALL')
        test_eq(tflagger(vis=self.vis, mode='summary'),1154592, 130736)
        test_eq(tflagger(vis=self.vis, mode='summary', correlation='I'),22752, 0)
        

class test_selections2(test_base):
    '''Test other selections'''
    
    def setUp(self):
        self.setUp_multi()
        
    def test_observation(self):
        '''tflagger: observation ID selections'''
        # string
        tflagger(vis=self.vis, observation='1')
        test_eq(tflagger(vis=self.vis, mode='summary'), 2882778, 28500)

        # integer
        tflagger(vis=self.vis, mode='unflag')
        tflagger(vis=self.vis, observation=1)
        test_eq(tflagger(vis=self.vis, mode='summary'), 2882778, 28500)
        
        # non-existing ID
        tflagger(vis=self.vis, mode='unflag')
        tflagger(vis=self.vis, observation='10')
        test_eq(tflagger(vis=self.vis, mode='summary'), 2882778, 0)
                
class test_elevation(test_base):
    """Test of mode = 'elevation'"""
    def setUp(self):
        self.setUp_ngc5921()
        self.x55 = 666792    # data below 55 degrees, etc.
        self.x60 = 1428840
        self.x65 = 2854278
        self.all = 2854278

    def test_lower(self):
        tflagger(vis = self.vis, mode = 'elevation')
        
        test_eq(tflagger(vis=self.vis, mode='summary'), self.all, 0)

        tflagger(vis = self.vis, mode = 'elevation', lowerlimit = 50)

        test_eq(tflagger(vis=self.vis, mode='summary'), self.all, 0)

        tflagger(vis = self.vis, mode = 'elevation', lowerlimit = 55)

        test_eq(tflagger(vis=self.vis, mode='summary'), self.all, self.x55)

        tflagger(vis = self.vis, mode = 'elevation', lowerlimit = 60)

        test_eq(tflagger(vis=self.vis, mode='summary'), self.all, self.x60)

        tflagger(vis = self.vis, mode = 'elevation', lowerlimit = 65)

        test_eq(tflagger(vis=self.vis, mode='summary'), self.all, self.x65)

    def test_upper(self):
        tflagger(vis = self.vis, mode = 'elevation', upperlimit = 60)

        test_eq(tflagger(vis=self.vis, mode='summary'), self.all, self.all - self.x60)


    def test_interval(self):
        tflagger(vis = self.vis,
                  mode = 'elevation',
                  lowerlimit = 55,
                  upperlimit = 60)

        test_eq(tflagger(vis=self.vis, mode='summary'), self.all, self.all - (self.x60 - self.x55))

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
        os.system('rm -rf Four_ants_3C286.ms')

    def test1(self):
        '''tflagger: Cleanup'''
        pass


def suite():
    return [test_tfcrop,
            test_shadow,
            test_selections,
            test_selections2,
            test_selections_alma,
            test_statistics_queries,
            test_msselection,
            test_elevation,
            cleanup]
