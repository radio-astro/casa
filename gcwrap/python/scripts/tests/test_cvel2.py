
import os
import numpy
import shutil
from __main__ import default
from tasks import *
from taskinit import *
from parallel.parallel_task_helper import ParallelTaskHelper
from parallel.parallel_data_helper import ParallelDataHelper
import unittest

# Path for data
datapath = os.environ.get('CASAPATH').split()[0] + "/data/regression/"

# Pick up alternative data directory to run tests on MMSs
testmms = False
if os.environ.has_key('TEST_DATADIR'):   
    DATADIR = str(os.environ.get('TEST_DATADIR'))+'/cvel/'
    if os.path.isdir(DATADIR):
        testmms = True
        datapath = DATADIR

print 'cvel2 tests will use data from '+datapath     
    
if os.environ.has_key('BYPASS_PARALLEL_PROCESSING'):
    ParallelTaskHelper.bypassParallelProcessing(1)

myname = 'test_cvel'
vis_a = 'ngc4826.ms'
vis_b = 'test.ms'
vis_c = 'jupiter6cm.demo-thinned.ms'
vis_d = 'g19_d2usb_targets_line-shortened-thinned.ms'
vis_e = 'evla-highres-sample-thinned.ms'
vis_f = 'test_cvel1.ms'
vis_g = 'jup.ms'
outfile = 'cvel-output.ms'
mytb = tbtool()

def verify_ms(msname, expnumspws, expnumchan, inspw, expchanfreqs=[]):
    msg = ''
    mytb.open(msname+'/SPECTRAL_WINDOW')
    nc = mytb.getcell("NUM_CHAN", inspw)
    nr = mytb.nrows()
    cf = mytb.getcell("CHAN_FREQ", inspw)
    mytb.close()
    mytb.open(msname)
    dimdata = mytb.getcell("FLAG", 0)[0].size
    mytb.close()
    if not (nr==expnumspws):
        msg =  "Found "+str(nr)+", expected "+str(expnumspws)+" spectral windows in "+msname
        return [False,msg]
    if not (nc == expnumchan):
        msg = "Found "+ str(nc) +", expected "+str(expnumchan)+" channels in spw "+str(inspw)+" in "+msname
        return [False,msg]
    if not (dimdata == expnumchan):
        msg = "Found "+ str(dimdata) +", expected "+str(expnumchan)+" channels in FLAG column in "+msname
        return [False,msg]

    if not (expchanfreqs==[]):
        print "Testing channel frequencies ..."
        print cf
        print expchanfreqs
        if not (expchanfreqs.size == expnumchan):
            msg =  "Internal error: array of expected channel freqs should have dimension ", expnumchan
            return [False,msg]
        df = (cf - expchanfreqs)/expchanfreqs
        if not (abs(df) < 1E-8).all:
            msg = "channel frequencies in spw "+str(inspw)+" differ from expected values by (relative error) "+str(df)
            return [False,msg]

    return [True,msg]

class test_base(unittest.TestCase):
#    forcereload=False
    
    def setUp_vis_a(self):
        # 308 scans, spw=0, data
        if testmms:
            os.system('cp -RL ' + datapath + vis_a + ' .')
        elif (not os.path.exists(vis_a)):
            importuvfits(fitsfile=os.environ.get('CASAPATH').split()[0] + '/data/regression/ngc4826/fitsfiles/ngc4826.ll.fits5', # 10 MB
                         vis=vis_a)
            
    def setUp_vis_b(self):
        # 308 scans, spw=0, data
        if testmms:
            os.system('cp -RL ' + datapath + vis_b + ' .')
        elif(not os.path.exists(vis_b)):
            os.system('cp -R '+os.environ.get('CASAPATH').split()[0] + '/data/regression/fits-import-export/input/test.ms .') # 27 MB
    
    def setUp_vis_c(self):
        # 93 scans, spw=0,1, data
        if testmms:
            os.system('cp -RL ' + datapath + vis_c + ' .')
        elif(not os.path.exists(vis_c)):
            os.system('cp -R '+os.environ.get('CASAPATH').split()[0] + '/data/regression/cvel/input/jupiter6cm.demo-thinned.ms .') # 124 MB

    def setUp_vis_d(self):
        # scan=3,4 spw=0~23, data
        if testmms:
            os.system('cp -RL ' + datapath + vis_d + ' .')
        elif(not os.path.exists(vis_d)):
            os.system('cp -R '+os.environ.get('CASAPATH').split()[0] + '/data/regression/cvel/input/g19_d2usb_targets_line-shortened-thinned.ms .') # 48 MB

    def setUp_vis_e(self):
        # scan=44,45 spw=0,1, data
        if testmms:
            os.system('cp -RL ' + datapath + vis_e + ' .')
        elif(not os.path.exists(vis_e)):
            os.system('cp -R '+os.environ.get('CASAPATH').split()[0] + '/data/regression/cvel/input/evla-highres-sample-thinned.ms .') # 74 MB

    def setUp_vis_f(self):
        # scan=1,2 spw=0~23, data
        if testmms:
            os.system('cp -RL ' + datapath + vis_f + ' .')
        elif(not os.path.exists(vis_f)):
            os.system('cp -R '+os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/cvel/test_cvel1.ms .') # 39 MB
            
    def setUp_vis_g(self):
        if testmms:
            os.system('cp -RL ' + datapath + vis_g + ' .')
        elif(not os.path.exists(vis_g)):
            # construct an MS with attached Jupiter ephemeris from vis_c
            self.setUp_vis_c()
            split(vis=vis_c, outputvis=vis_g, field='JUPITER', datacolumn='data')
            mytb.open(vis_g, nomodify=False)
            a = mytb.getcol('TIME')
            delta = (54709.*86400-a[0])
            a = a + delta
            strt = a[0]
            mytb.putcol('TIME', a)
            a = mytb.getcol('TIME_CENTROID')
            a = a + delta
            mytb.putcol('TIME_CENTROID', a)
            mytb.close()
            mytb.open(vis_g+'/OBSERVATION', nomodify=False)
            a = mytb.getcol('TIME_RANGE')
            delta = strt - a[0][0]
            a = a + delta
            mytb.putcol('TIME_RANGE', a)
            mytb.close()
            mytb.open(vis_g+'/FIELD', nomodify=False)
            a = mytb.getcol('TIME')
            delta = strt - a[0]
            a = a + delta
            mytb.putcol('TIME', a)
            mytb.close()
            ms.open(vis_g, nomodify=False)
            ms.addephemeris(0,os.environ.get('CASAPATH').split()[0]+'/data/ephemerides/JPL-Horizons/Jupiter_54708-55437dUTC.tab',
                            'Jupiter_54708-55437dUTC', 0)
            ms.close()
    
    def setUp_4ants(self):
        # data set with spw=0~15, 64 channels each in TOPO
        self.vis = "Four_ants_3C286.ms"

        if os.path.exists(self.vis):
           self.cleanup()

        os.system('cp -RL '+self.datapath + self.vis +' '+ self.vis)
        default(cvel2)

    def setUp_mms_vis_c(self):
        # 93 scans, spw=0,1
        if (not os.path.exists(vis_c)):
            os.system('cp -R '+os.environ.get('CASAPATH').split()[0] + '/data/regression/cvel/input/jupiter6cm.demo-thinned.ms .') 

    def createMMS(self, msfile, axis='auto',scans='',spws='', fields='', numms=64):
        '''Create MMSs for tests with input MMS'''
        prefix = msfile[:-len('.ms')]
        
        # Create an MMS for the tests
        self.testmms = "cvel_input_test.mms"
        default(partition)
        
        if os.path.exists(self.testmms):
            os.system("rm -rf " + self.testmms)
            os.system("rm -rf " + self.testmms +'.flagversions')
            
        print "................. Creating test MMS .................."
        partition(vis=msfile, outputvis=self.testmms,flagbackup=False, separationaxis=axis, scan=scans, spw=spws, field=fields,
                  numsubms=numms)

class cvel2_test(test_base):

    def tearDown(self):
        os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms cvel_input_test.mms')   
    
    def test1(self):
        '''cvel2 1: Testing default - expected error'''
        self.setUp_vis_b()
        myvis = vis_b
        os.system('ln -sf ' + myvis + ' myinput.ms')
        try:
            cvel2()
        except Exception:
            print 'Expected error!'
    
    def test2(self):
        '''cvel2 2: Only input vis set - expected error'''
        self.setUp_vis_b()
        myvis = vis_b
        os.system('ln -sf ' + myvis + ' myinput.ms')
        try:
            cvel2(vis = 'myinput.ms')
        except Exception:
            print 'Expected error!'
            
    def test3(self):
        '''cvel2 3: Input and output vis set'''
        self.setUp_vis_b()
        myvis = vis_b
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(vis = 'myinput.ms', outputvis = outfile)
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 64, 0)
        self.assertTrue(ret[0],ret[1])
            
    def test4(self):
        '''cvel2 4: I/O vis set, more complex input vis, one field selected'''
        self.setUp_vis_a()
        myvis = vis_a        
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(vis = 'myinput.ms', outputvis = outfile, field = '1')
        self.assertNotEqual(rval,False)
        ret = (verify_ms(outfile, 1, 64, 0))
        self.assertTrue(ret[0],ret[1])

    def test5(self):
        '''cvel2 5: I/O vis set, more complex input vis, one field selected, passall = True'''
        self.setUp_vis_a()
        myvis = vis_a
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
                    vis = 'myinput.ms',
                    outputvis = outfile,
                    field = '1',
                    passall = True
                    )
        self.assertNotEqual(rval,False)
        ret = (verify_ms(outfile, 1, 64, 0))
        self.assertTrue(ret[0],ret[1])

    def test6(self):
        '''cvel2 6: I/O vis set, more complex input vis, one field selected, one spw selected, passall = True'''
        if testmms:
            return
        self.setUp_vis_a()
        myvis = vis_a
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            field = '1',
            spw = '0',
            nchan = 32,
            start = 10,
            passall = True
            )
        self.assertNotEqual(rval,False)
        
        # Simulate the passall=True. This MS has fields 0~6
        desel = outfile+'.deselected'
        split2(vis='myinput.ms',outputvis=desel,field='0,2,3,4,5,6',spw='0',datacolumn='all')
        mslocal = mstool()
        mslocal.open(outfile, nomodify=False)
        mslocal.concatenate(msfile=desel)            
        mslocal.close()
        ret = (verify_ms(outfile, 2, 32, 0))
        self.assertTrue(ret[0],ret[1])

    ## # Tests with more than one spectral window ###################
    
    def test7(self):
        '''cvel2 7: I/O vis set, input vis with two spws, one field selected, 2 spws selected, 
           passall = False'''
        self.setUp_vis_c()
        myvis = vis_c
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
                vis = 'myinput.ms',
                outputvis = outfile,
                field = '12', # select Jupiter
                spw = '0,1',  # both available SPWs
                passall = False
                )
        self.assertNotEqual(rval,False)
        ret = (verify_ms(outfile, 1, 2, 0))
        self.assertTrue(ret[0],ret[1])

    def test8(self):
        '''cvel2 8: I/O vis set, input vis with two spws, one field selected, 2 spws selected, 
           passall = False, regridding 1'''
        self.setUp_vis_c()
        myvis = vis_c
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            field = '11',    # select some other field
            spw = '0,1',    # both available SPWs
            passall = False,    # regrid
            nchan = 1,
            width = 2
            )
        self.assertNotEqual(rval,False)
        ret = (verify_ms(outfile, 1, 1, 0))
        self.assertTrue(ret[0],ret[1])
   
    def test9(self):
        '''cvel2 9: I/O vis set, input vis with two spws, one field selected, 2 spws selected, 
           passall = False, regridding 2'''
        self.setUp_vis_c()
        myvis = vis_c
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            field = '10',
            spw = '0,1',
            passall = False,
            mode='channel',
            nchan = 1,
            start = 1
            )
        self.assertNotEqual(rval,False)
        ret = (verify_ms(outfile, 1, 1, 0))
        self.assertTrue(ret[0],ret[1])
    
    def test10(self):
        '''cvel210: I/O vis set, input vis with two spws, one field selected, 2 spws selected, passall = False, regridding 3...'''
        self.setUp_vis_c()
        myvis = vis_c
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            field = '9',
            spw = '0,1',
            passall = False,
            mode='frequency',
            nchan = 1,
            start = '4.8351GHz',
            width = '50MHz'
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 1, 0)
        self.assertTrue(ret[0],ret[1])

    
    def test11(self):
        '''cvel2 11: I/O vis set, input vis with two spws, one field selected, 
           2 spws selected, passall = False, regridding 4...'''
        self.setUp_vis_c()
        myvis = vis_c
        os.system('ln -sf ' + myvis + ' myinput.ms')

        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            field = '10',
            spw = '0,1',
            passall = False,
            mode='channel',
            nchan = 1,
            start = 1,
            outframe = 'lsrk'
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 1, 0)
        self.assertTrue(ret[0],ret[1])

    def test12(self):
        '''cvel2 12: Input and output vis set, input vis with two spws, two fields selected, 
           2 spws selected, passall = False, regridding 5...'''
        self.setUp_vis_c()
        myvis = vis_c
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            field = '5,6',
            spw = '0,1',
            passall = False,
            mode='frequency',
            nchan = 2,
            start = '4.8101 GHz',
            width = '50 MHz',
            outframe = ''
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 2, 0)
        self.assertTrue(ret[0],ret[1])
    
    def test13(self):
        '''cvel2 13: I/O vis set, input vis with one spw, one field selected, one spw selected, 
           passall = False, regridding 6...'''
        self.setUp_vis_a()
        myvis = vis_a
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            field = '1',
            spw = '0',
            passall = False,
            mode='frequency',
            nchan = 2,
            start = '115GHz',
            width = '3MHz',
            outframe = 'BARY',
            phasecenter = 1
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 2, 0)
        self.assertTrue(ret[0],ret[1])
    
    def test14(self):
        '''cvel2 14: I/O vis set, input vis with one spws, one field selected, one spws selected, 
           passall = False, non-existing phase center...'''
        self.setUp_vis_a()
        myvis = vis_a
        os.system('ln -sf ' + myvis + ' myinput.ms')
        try:
            rval = cvel2(
                vis = 'myinput.ms',
                outputvis = outfile,
                field = '1',
                spw = '0',
                passall = False,
                mode='frequency',
                nchan = 2,
                start = '150GHz',
                width = '3MHz',
                outframe = 'BARY',
                phasecenter = 12
                )
            self.assertNotEqual(rval,False)
            ret = verify_ms(outfile, 1, 2, 0)
            self.assertTrue(ret[0],ret[1])
        except:
            print "*** Expected error ***"
    
    def test15(self):
        '''cvel2 15: I/O vis set, input vis with two spws, one field selected, 2 spws selected, passall = False, regridding 8...'''
        self.setUp_vis_c()
        myvis = vis_c
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
            field = '12',
            spw = '0,1',
            passall = False,
            mode='frequency',
            nchan = 1,
            start = '4.850GHz',
            width = '50MHz',
            outframe = ''
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 1, 0)
        self.assertTrue(ret[0],ret[1])
    
    def test16(self):
        '''cvel2 16: I/O vis set, input vis with one spw, two fields selected, passall = False, regridding 9...'''
        self.setUp_vis_a()
        myvis = vis_a
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            field = '2,3',
            spw = '0',
            passall = False,
            mode='channel',
            nchan = 10,
            start = 2,
            outframe = 'lsrd',
            phasecenter = 2
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 10, 0)
        self.assertTrue(ret[0],ret[1])
    
    def test17(self):
        '''cvel2 17: I/O vis set, input vis with one spw, two fields selected, passall = False, regridding 9...'''
        self.setUp_vis_a()
        myvis = vis_a
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
            field = '2,3',
            spw = '0',
            passall = False,
            mode='frequency',
            nchan = 10,
            start = '114.9527GHz',
            width = '3.125MHz',
            outframe = 'lsrd',
            phasecenter = 2)
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 10, 0)
        self.assertTrue(ret[0],ret[1])
        
    def test18(self):
        '''cvel2 18: I/O vis set, input vis with one spw, two fields selected, passall = False, regridding 9...'''
        self.setUp_vis_a()
        myvis = vis_a
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            field = '2,3',
            spw = '0',
            passall = False,
            mode='frequency',
            nchan = 10,
            start = '114.9527GHz',
            width = '3.125MHz',
            outframe = 'lsrd',
            phasecenter = 'J2000 12h56m43.88s +21d41m00.1s'
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 10, 0)
        self.assertTrue(ret[0],ret[1])
    
    def test19(self):
        '''cvel2 19: SMA input MS, 24 spws to combine, channel mode, 10 output channels'''
        self.setUp_vis_d()
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode='channel',
            nchan = 10,
            start = 100,
            width = 2,
            phasecenter = "J2000 18h25m56.09 -12d04m28.20"
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 10, 0)
        self.assertTrue(ret[0],ret[1])
    
    def test20(self):
        '''cvel2 20: SMA input MS, 24 spws to combine, channel mode, 111 output channels'''
        self.setUp_vis_d()
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode='channel',
            nchan = 111,
            start = 201,
            width = 3,
            phasecenter = "J2000 18h25m56.09 -12d04m28.20"
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 111, 0)
        self.assertTrue(ret[0],ret[1])
    
    def test21(self):
        '''cvel2 21: SMA input MS, 24 spws to combine, frequency mode, 21 output channels'''
        self.setUp_vis_d()
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode='frequency',
            nchan = 21,
            start = '229587.0MHz',
            width = '1600kHz',
            phasecenter = "J2000 18h25m56.09 -12d04m28.20"
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 21, 0)
        self.assertTrue(ret[0],ret[1])
    
    def test22(self):
        '''cvel2 22: SMA input MS, 24 spws to combine, frequency mode, 210 output channels, negative width'''
        self.setUp_vis_d()
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode='frequency',
            nchan = 210,
            start = '229588.0MHz',
            width = '-2400kHz',
            phasecenter = "J2000 18h25m56.09 -12d04m28.20"
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 210, 0)
        self.assertTrue(ret[0],ret[1])
        os.system('mv '+outfile+' xxx.ms')
    
    def test23(self):
        '''cvel2 23: SMA input MS, 24 spws to combine, radio velocity mode, 30 output channels'''
        self.setUp_vis_d()
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        vrad = (220398.676E6 - 229586E6)/220398.676E6 * 2.99792E8
        vwidth = ((220398.676E6 - 229586E6+1600E3)/220398.676E6 * 2.99792E8) - vrad
        vrad = vrad-vwidth/2.
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode='velocity',
            nchan = 30,
            restfreq = '220398.676MHz',
            start = str(vrad)+'m/s',
            width = str(vwidth)+'m/s',
            phasecenter = "J2000 18h25m56.09 -12d04m28.20",
            veltype = 'radio'
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 30, 0)
        self.assertTrue(ret[0],ret[1])
    
    def test24(self):
        '''cvel2 24: SMA input MS, 24 spws to combine, radio velocity mode, 35 output channels'''
        self.setUp_vis_d()
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        vrad = (220398.676E6 - 229586E6)/220398.676E6 * 2.99792E8
        vwidth = ((220398.676E6 - 229586E6+3200E3)/220398.676E6 * 2.99792E8) - vrad
        vrad = vrad-vwidth/2.
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode='velocity',
            nchan = 35,
            restfreq = '220398.676MHz',
            start = str(vrad)+'m/s',
            width = str(vwidth)+'m/s',
            phasecenter = "J2000 18h25m56.09 -12d04m28.20",
            veltype = 'radio'
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 35, 0)
        self.assertTrue(ret[0],ret[1])
    
    def test25(self):
        '''cvel2 25: SMA input MS, 24 spws to combine, optical velocity mode, 40 output channels'''
        self.setUp_vis_d()
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        lambda0 = 2.99792E8/220398.676E6
        lambda1 = 2.99792E8/229586E6
        lambda2 = 2.99792E8/(229586E6+1600E3)
        vopt = (lambda1-lambda0)/lambda0 * 2.99792E8
        vwidth = vopt - (lambda2-lambda0)/lambda0 * 2.99792E8
        vopt = vopt-vwidth/2.
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode='velocity',
            nchan = 40,
            restfreq = '220398.676MHz',
            start = str(vopt)+'m/s',
            width = str(vwidth)+'m/s',
            phasecenter = "J2000 18h25m56.09 -12d04m28.20",
            veltype = 'optical'
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 40, 0)
        self.assertTrue(ret[0],ret[1])
    
    def test26(self):
        '''cvel2 26: SMA input MS, 24 spws to combine, optical velocity mode, 40 output channels'''
        self.setUp_vis_d()
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        lambda0 = 2.99792E8/220398.676E6
        lambda1 = 2.99792E8/229586E6
        vopt = (lambda1-lambda0)/lambda0 * 2.99792E8
        lambda2 = 2.99792E8/(229586E6+1200E3)
        vwidth = vopt - (lambda2-lambda0)/lambda0 * 2.99792E8
        vopt = vopt-vwidth/2.
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode='velocity',
            nchan = 41,
            restfreq = '220398.676MHz',
            start = str(vopt)+'m/s',
            width = str(vwidth)+'m/s',
            phasecenter = "J2000 18h25m56.09 -12d04m28.20",
            veltype = 'optical'
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 41, 0)
        self.assertTrue(ret[0],ret[1])
    
    def test27(self):
        '''cvel2 27: SMA input MS, 24 spws to combine, scratch columns, no regridding'''
        self.setUp_vis_d()
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        # no regrid
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 2440, 0)
        self.assertTrue(ret[0],ret[1])
    
    def test28(self):
        '''cvel2 28: SMA input MS, 24 spws to combine, scratch columns, channel mode, 30 channels'''
        self.setUp_vis_d()
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode="channel",
            start=1500,
            width=2,
            nchan=30
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 30, 0)
        self.assertTrue(ret[0],ret[1])
            
    def test29(self):
        '''cvel2 29: SMA input MS, 24 spws to combine, scratch columns, channel mode, 31 channels'''
        self.setUp_vis_d()
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode="channel",
            start=1500,
            width=2,
            nchan=31
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 31, 0)
        self.assertTrue(ret[0],ret[1])
    
    def test30(self):
        '''cvel2 30: SMA input MS, 24 spws to combine, scratch columns, mode channel_b, no regridding'''
        self.setUp_vis_d()
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode="channel_b"
            )
        
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 2425, 0)
        self.assertTrue(ret[0],ret[1])
    
    def test31(self):
        '''cvel2 31: SMA input MS, 24 spws to combine, scratch columns, mode channel, frame trafo'''
        self.setUp_vis_d()
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode="channel",
            outframe = "BARY",
            phasecenter = "J2000 18h25m56.09 -12d04m28.20"
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 2440, 0)
        self.assertTrue(ret[0],ret[1])

    def test32(self):
        '''cvel2 32: SMA input MS, 24 spws to combine, scratch columns, mode channel, frame trafo, Hanning smoothing'''
        self.setUp_vis_d()
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode="channel",
            outframe = "BARY",
            phasecenter = "J2000 18h25m56.09 -12d04m28.20",
            hanning = True
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 2440, 0)
        self.assertTrue(ret[0],ret[1])

    def test33(self):
        '''cvel2 33: SMA input MS, 1 spw, scratch columns, mode channel, no trafo, Hanning smoothing'''
        self.setUp_vis_d()
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            spw='1',
            outputvis = outfile,
            mode="channel",
            outframe = "",
            hanning = True
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 128, 0)
        self.assertTrue(ret[0],ret[1])

    def test34(self):
        '''cvel2 34: EVLA high-res input MS, 2 spws to combine'''
        self.setUp_vis_e()
#         os.system('ls -l '+vis_e)
#         os.path.exists(vis_e+'/SORTED_TABLE')
# 
# #        myvis = vis_e
# #        os.system('ln -sf ' + myvis + ' myinput.ms')
#         rval = cvel2(
#             vis = vis_e,
#             outputvis = outfile,
#             mode = 'velocity',
#             restfreq  = '6035.092MHz'
#             )
#         self.assertNotEqual(rval,False)
#         ret = verify_ms(outfile, 1, 260, 0)
#         self.assertTrue(ret[0],ret[1])
        myvis = vis_e
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode = 'velocity',
            restfreq  = '6035.092MHz'
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 260, 0)
        self.assertTrue(ret[0],ret[1])

    def test35(self):
        '''cvel2 35: test effect of sign of width parameter: channel mode, width positive'''
        self.setUp_vis_b()
        myvis = vis_b
        os.system('ln -sf ' + myvis + ' myinput.ms')
        tb.open('myinput.ms/SPECTRAL_WINDOW')
        a = tb.getcell('CHAN_FREQ')
        b = numpy.array([a[1], a[2], a[3]])
        tb.close()

        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            nchan = 3,
            start = 1,
            width=1
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 3, 0, b)
        self.assertTrue(ret[0],ret[1])

    def test36(self):
        '''cvel2 36: test effect of sign of width parameter: channel mode, width negative'''
        self.setUp_vis_b()
        myvis = vis_b
        os.system('ln -sf ' + myvis + ' myinput.ms')
        tb.open('myinput.ms/SPECTRAL_WINDOW')
        a = tb.getcell('CHAN_FREQ')
        b = numpy.array([a[1], a[2], a[3]])
        tb.close()

        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            nchan = 3,
            start = 3,
            width=-1
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 3, 0, b)
        self.assertTrue(ret[0],ret[1])

    def test37(self):
        '''cvel2 37: test effect of sign of width parameter: freq mode, width positive'''
        self.setUp_vis_b()
        myvis = vis_b
        os.system('ln -sf ' + myvis + ' myinput.ms')
        tb.open('myinput.ms/SPECTRAL_WINDOW')
        a = tb.getcell('CHAN_FREQ')
        b = numpy.array([a[1], a[2], a[3]])
        tb.close()

        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode = 'frequency',
            nchan = 3,
            start = str(a[1])+'Hz',
            width=str(a[2]-a[1])+'Hz'
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 3, 0, b)
        self.assertTrue(ret[0],ret[1])

    def test38(self):
        '''cvel2 38: test effect of sign of width parameter: freq mode, width negative'''
        self.setUp_vis_b()
        myvis = vis_b
        os.system('ln -sf ' + myvis + ' myinput.ms')
        tb.open('myinput.ms/SPECTRAL_WINDOW')
        a = tb.getcell('CHAN_FREQ')
        b = numpy.array([a[1], a[2], a[3]])
        tb.close()

        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode = 'frequency',
            nchan = 3,
            start = str(a[3])+'Hz',
            width='-'+str(a[2]-a[1])+'Hz'
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 3, 0, b)
        self.assertTrue(ret[0],ret[1])

    def test39(self):
        '''cvel2 39: test effect of sign of width parameter: radio velocity mode, width positive'''
        self.setUp_vis_b()
        myvis = vis_b
        os.system('ln -sf ' + myvis + ' myinput.ms')
        tb.open('myinput.ms/SPECTRAL_WINDOW')
        a = tb.getcell('CHAN_FREQ')
        c =  qa.constants('c')['value']
        tb.close()
        
        restf = a[0] 
        bv1 = c * (restf-a[5])/restf 
        bv2 = c * (restf-a[4])/restf 
        wv = abs(bv2-bv1)
        b = numpy.array([a[3], a[4], a[5]])
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode = 'velocity',
            veltype = 'radio',
            nchan = 3,
            start = str(bv1)+'m/s',
            width=str(wv)+'m/s',
            restfreq=str(restf)+'Hz'
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 3, 0, b)
        self.assertTrue(ret[0],ret[1])

    def test40(self):
        '''cvel2 40: test effect of sign of width parameter: radio velocity mode, width negative'''
        self.setUp_vis_b()
        myvis = vis_b
        os.system('ln -sf ' + myvis + ' myinput.ms')
        tb.open('myinput.ms/SPECTRAL_WINDOW')
        a = tb.getcell('CHAN_FREQ')
        c =  qa.constants('c')['value']
        tb.close()

        restf = a[0] 
        bv1 = c * (restf-a[3])/restf 
        bv2 = c * (restf-a[4])/restf 
        wv = abs(bv2-bv1)
        b = numpy.array([a[3], a[4], a[5]])
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode = 'velocity',
            veltype = 'radio',
            nchan = 3,
            start = str(bv1)+'m/s',
            width="-"+str(wv)+'m/s',
            restfreq=str(restf)+'Hz'
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 3, 0, b)
        self.assertTrue(ret[0],ret[1])

    def test41(self):
        '''cvel2 41: test effect of sign of width parameter: optical velocity mode, width positive'''
        self.setUp_vis_b()
        myvis = vis_b
        os.system('ln -sf ' + myvis + ' myinput.ms')
        tb.open('myinput.ms/SPECTRAL_WINDOW')
        a = tb.getcell('CHAN_FREQ')
        c =  qa.constants('c')['value']
        tb.close()
        
        restf = a[0] 
        bv1 = c * (restf-a[5])/a[5] 
        bv2 = c * (restf-a[4])/a[4] 
        wv = abs(bv2-bv1+1.)
        bv2 = bv1 + wv
        bv3 = bv2 + wv
        a4 = restf/(bv2/c+1.)        
        a3 = restf/(bv3/c+1.)
        b = numpy.array([a3, a4, a[5]])
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode = 'velocity',
            veltype = 'optical',
            nchan = 3,
            start = str(bv1)+'m/s',
            width=str(wv)+'m/s',
            restfreq=str(restf)+'Hz'
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 3, 0, b)
        self.assertTrue(ret[0],ret[1])

    def test42(self):
        '''cvel2 42: test effect of sign of width parameter: optical velocity mode, width negative'''
        self.setUp_vis_b()
        myvis = vis_b
        os.system('ln -sf ' + myvis + ' myinput.ms')
        tb.open('myinput.ms/SPECTRAL_WINDOW')
        a = tb.getcell('CHAN_FREQ')
        c =  qa.constants('c')['value']
        tb.close()
        
        restf = a[0] 
        bv1 = c * (restf-a[5])/a[5] 
        bv2 = c * (restf-a[4])/a[4] 
        wv = abs(bv2-bv1+1.)
        bv2 = bv1 + wv
        bv3 = bv2 + wv
        a4 = restf/(bv2/c+1.)        
        a3 = restf/(bv3/c+1.)
        b = numpy.array([a3, a4, a[5]])
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode = 'velocity',
            veltype = 'optical',
            nchan = 3,
            start = str(bv3)+'m/s',
            width='-'+str(wv)+'m/s',
            restfreq=str(restf)+'Hz'
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 3, 0, b)
        self.assertTrue(ret[0],ret[1])

    def test43(self):
        '''cvel2 43: SMA input MS, 1 spw, channel mode, nchan not set'''
        self.setUp_vis_d()
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode='channel',
            spw='1',
            start = 98,
            width = 3,
            phasecenter = "J2000 18h25m56.09 -12d04m28.20"
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 10, 0)
        self.assertTrue(ret[0],ret[1])

    def test44(self):
        '''cvel2 44: SMA input MS, 2 spws to combine, channel mode, nchan not set'''
        self.setUp_vis_d()
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode='channel',
            spw='1,15',
            start = 198,
            width = 3,
            phasecenter = "J2000 18h25m56.09 -12d04m28.20"
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 10, 0)
        self.assertTrue(ret[0],ret[1])

    def test45(self):
        '''cvel2 45: SMA input MS, 1 spw, channel mode, nchan not set, negative width'''
        self.setUp_vis_d()
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode='channel',
            spw='1',
            start = 29,
            width = -3,
            phasecenter = "J2000 18h25m56.09 -12d04m28.20"
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 10, 0)
        self.assertTrue(ret[0],ret[1])

    def test46(self):
        '''cvel2 46: SMA input MS with descending freq, 24 spws, nchan=100'''
        self.setUp_vis_f()
        myvis = vis_f
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode='channel',
            spw='',
            start = 29,
            nchan = 100
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 100, 0)
        self.assertTrue(ret[0],ret[1])

    def test47(self):
        '''cvel2 47: SMA input MS with descending freq, 1 spw, nchan not set'''
        self.setUp_vis_f()
        myvis = vis_f
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode='channel',
            spw='10',
            start = 98,
            width=3
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 10, 0)
        self.assertTrue(ret[0],ret[1])

    def test48(self):
        '''cvel2 48: test fftshift regridding: channel mode, width positive'''
        self.setUp_vis_b()
        myvis = vis_b
        os.system('ln -sf ' + myvis + ' myinput.ms')
        tb.open('myinput.ms/SPECTRAL_WINDOW')
        a = tb.getcell('CHAN_FREQ')
        b = numpy.array([a[1], a[2], a[3]])
        tb.close()

        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            nchan = 3,
            start = 1,
            width = 1,
            interpolation = 'fftshift'
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 3, 0, b)
        self.assertTrue(ret[0],ret[1])

    def test49(self):
        '''cvel2 49: vopt mode with fftshift, expected error ...'''
        self.setUp_vis_b()
        myvis = vis_b
        os.system('ln -sf ' + myvis + ' myinput.ms')
        try:
            rval = cvel2(
                vis = 'myinput.ms',
                outputvis = outfile,
                mode='velocity',
                veltype='optical',
                restfreq = '220398.676MHz',
                outframe = 'BARY',
                interpolation = 'fftshift'
                )
            self.assertNotEqual(rval,False)
            ret = verify_ms(outfile, 1, 2, 0)
            self.assertTrue(ret[0],ret[1])
        except:
            print "*** Expected error ***"

    def test50(self):
        '''cvel2 50: test fftshift regridding: channel mode, width positive'''
        self.setUp_vis_d()
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')

        # get reference values by running cvel2 with linear interpol
        cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            spw = '0,1',
            nchan = 150,
            start = 10,
            width = 1,
            interpolation = 'linear',
            outframe = 'CMB'
            )
        
        tb.open(outfile+'/SPECTRAL_WINDOW')
        a = tb.getcell('CHAN_FREQ')
        b = numpy.array(a)
        tb.close()

        shutil.rmtree(outfile, ignore_errors=True)

        rval = cvel2(
            vis = 'myinput.ms',
            outputvis = outfile,
            spw = '0,1',
            nchan = 150,
            start = 10,
            width = 1,
#            interpolation = 'linear',
            interpolation = 'fftshift',
            outframe = 'CMB'
            )

        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 150, 0, b)
        self.assertTrue(ret[0],ret[1])

    def test51(self):
        '''cvel2 51: test fftshift regridding: frequency mode, width positive'''
        self.setUp_vis_d()
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')

        # get reference values by running cvel2 with linear interpol
        cvel2(
            mode = 'frequency',
            vis = 'myinput.ms',
            outputvis = outfile,
            spw = '0,1',
            nchan = 150,
            interpolation = 'linear',
            outframe = 'BARY'
            )
        
        tb.open(outfile+'/SPECTRAL_WINDOW')
        a = tb.getcell('CHAN_FREQ')
        b = numpy.array(a)
        tb.close()

        shutil.rmtree(outfile, ignore_errors=True)

        rval = cvel2(
            mode = 'frequency',
            vis = 'myinput.ms',
            outputvis = outfile,
            spw = '0,1',
            nchan = 150,
            interpolation = 'fftshift',
            outframe = 'BARY'
            )

        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 150, 0, b)
        self.assertTrue(ret[0],ret[1])

    def test52(self):
        '''cvel2 52: test fftshift regridding: radio velocity mode, width positive'''
        self.setUp_vis_d()
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')

        # get reference values by running cvel2 with linear interpol
        cvel2(
            mode = 'velocity',
            veltype = 'radio',
            restfreq = '220398.676MHz',
            vis = 'myinput.ms',
            outputvis = outfile,
            spw = '0,1',
            nchan = 150,
            interpolation = 'linear',
            outframe = 'CMB'
            )
        
        tb.open(outfile+'/SPECTRAL_WINDOW')
        a = tb.getcell('CHAN_FREQ')
        b = numpy.array(a)
        tb.close()

        shutil.rmtree(outfile, ignore_errors=True)

        rval = cvel2(
            mode = 'velocity',
            veltype = 'radio',
            restfreq = '220398.676MHz',
            vis = 'myinput.ms',
            outputvis = outfile,
            spw = '0,1',
            nchan = 150,
            interpolation = 'fftshift',
            outframe = 'CMB'
            )

        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 150, 0, b)
        self.assertTrue(ret[0],ret[1])

    def test53(self):
        '''cvel2 53: cvel2 of a field with ephemeris attached and outframe SOURCE'''
        self.setUp_vis_g()
        myvis = vis_g
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel2(
                vis = 'myinput.ms',
                outputvis = outfile,
                outframe = 'SOURCE'
                )
        self.assertTrue(rval)
        
    def test_mms_heuristics1(self):
        '''cvel2 : MMS heuristic tests'''
        self.setUp_mms_vis_c()
        self.createMMS(vis_c, axis='spw', scans='8,11')
        try:
            rval = cvel2(
                vis = self.testmms,
                outputvis = outfile,
                field = '5,6',
                spw = '0,1',
                passall = False,
                mode='frequency',
                nchan = 2,
                start = '4.8101 GHz',
                width = '50 MHz',
                outframe = ''
                )
        except Exception:
            print 'Expected error!'
        
    def test_mms_heuristics2(self):
        '''cvel2 : MMS heuristic tests'''
        self.setUp_mms_vis_c()
        self.createMMS(vis_c, axis='auto', scans='8,11', spws='0,1')
        try:
            rval = cvel2(
                vis = self.testmms,
                outputvis = outfile,
                field = '5,6',
                spw = '0,1',
                passall = False,
                mode='frequency',
                nchan = 2,
                start = '4.8101 GHz',
                width = '50 MHz',
                outframe = ''
                )
        except Exception:
            print 'Expected error!'
             
    def test_mms_heuristics3(self):
        '''cvel2 : MMS heuristic tests'''
        self.setUp_mms_vis_c()
        self.createMMS(vis_c, axis='auto', scans='8,11', spws='0,1')
        
        # It should create an output MS
        rval = cvel2(
            vis = self.testmms,
            outputvis = outfile,
            field = '5,6',
            spw = '0,1',
            passall = False,
            mode='frequency',
            nchan = 2,
            start = '4.8101 GHz',
            width = '50 MHz',
            outframe = '',
            keepmms=False
            )

        self.assertFalse(ParallelDataHelper.isParallelMS(outfile),'Output shold be an MS')
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 2, 0)
        self.assertTrue(ret[0],ret[1])
        
        
class cleanup(unittest.TestCase):
    def setUp(self):
        pass
    
    def tearDown(self):
        # It will ignore errors in case files don't exist
        shutil.rmtree(vis_a,ignore_errors=True)
        shutil.rmtree(vis_b,ignore_errors=True)
        shutil.rmtree(vis_c,ignore_errors=True)
        shutil.rmtree(vis_d,ignore_errors=True)
        shutil.rmtree(vis_e,ignore_errors=True)
        shutil.rmtree(vis_f,ignore_errors=True)
        
    def test_cleanup(self):
        '''cvel2: Cleanup'''
        pass
        


def suite():
    return [cvel2_test, cleanup]

