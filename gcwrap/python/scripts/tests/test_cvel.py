# unit test for the cvel task

import os
import numpy
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest

myname = 'test_cvel'
vis_a = 'ngc4826.ms'
vis_b = 'test.ms'
vis_c = 'jupiter6cm.demo-thinned.ms'
vis_d = 'g19_d2usb_targets_line-shortened-thinned.ms'
vis_e = 'evla-highres-sample-thinned.ms'
vis_f = 'test_cvel1.ms'
vis_g = 'jup.ms'
outfile = 'cvel-output.ms'

def verify_ms(msname, expnumspws, expnumchan, inspw, expchanfreqs=[]):
    msg = ''
    tb.open(msname+'/SPECTRAL_WINDOW')
    nc = tb.getcell("NUM_CHAN", inspw)
    nr = tb.nrows()
    cf = tb.getcell("CHAN_FREQ", inspw)
    tb.close()
    tb.open(msname)
    dimdata = tb.getcell("FLAG", 0)[0].size
    tb.close()
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


class cvel_test(unittest.TestCase):

    def setUp(self):    
        default('cvel')
        forcereload=False
        
        if(forcereload or not os.path.exists(vis_a)):
            shutil.rmtree(vis_a, ignore_errors=True)
            importuvfits(fitsfile=os.environ['CASAPATH'].split()[0]+'/data/regression/ngc4826/fitsfiles/ngc4826.ll.fits5', # 10 MB
                         vis=vis_a)
        if(forcereload or not os.path.exists(vis_b)):
            shutil.rmtree(vis_b, ignore_errors=True)
            os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/fits-import-export/input/test.ms .') # 27 MB
        if(forcereload or not os.path.exists(vis_c)):
            shutil.rmtree(vis_c, ignore_errors=True)
            os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/cvel/input/jupiter6cm.demo-thinned.ms .') # 124 MB
        if(forcereload or not os.path.exists(vis_d)):
            shutil.rmtree(vis_d, ignore_errors=True)
            os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/cvel/input/g19_d2usb_targets_line-shortened-thinned.ms .') # 48 MB
        if(forcereload or not os.path.exists(vis_e)):
            shutil.rmtree(vis_e, ignore_errors=True)
            os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/cvel/input/evla-highres-sample-thinned.ms .') # 74 MB
        if(forcereload or not os.path.exists(vis_f)):
            shutil.rmtree(vis_f, ignore_errors=True)
            os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/unittest/cvel/test_cvel1.ms .') # 39 MB
        if(forcereload or not os.path.exists(vis_g)):
            # construct an MS with attached Jupiter ephemeris from vis_c
            shutil.rmtree(vis_g, ignore_errors=True)
            split(vis=vis_c, outputvis=vis_g, field='JUPITER', datacolumn='data')
            tb.open(vis_g, nomodify=False)
            a = tb.getcol('TIME')
            delta = (54709.*86400-a[0])
            a = a + delta
            strt = a[0]
            tb.putcol('TIME', a)
            a = tb.getcol('TIME_CENTROID')
            a = a + delta
            tb.putcol('TIME_CENTROID', a)
            tb.close()
            tb.open(vis_g+'/OBSERVATION', nomodify=False)
            a = tb.getcol('TIME_RANGE')
            delta = strt - a[0][0]
            a = a + delta
            tb.putcol('TIME_RANGE', a)
            tb.close()
            tb.open(vis_g+'/FIELD', nomodify=False)
            a = tb.getcol('TIME')
            delta = strt - a[0]
            a = a + delta
            tb.putcol('TIME', a)
            tb.close()
            ms.open(vis_g, nomodify=False)
            ms.addephemeris(0,os.environ.get('CASAPATH').split()[0]+'/data/ephemerides/JPL-Horizons/Jupiter_54708-55437dUTC.tab',
                            'Jupiter_54708-55437dUTC', 0)
            ms.close()

            
        self.assertTrue(os.path.exists(vis_a))
        self.assertTrue(os.path.exists(vis_b))
        self.assertTrue(os.path.exists(vis_c))
        self.assertTrue(os.path.exists(vis_d))
        self.assertTrue(os.path.exists(vis_e))
        self.assertTrue(os.path.exists(vis_f))
        self.assertTrue(os.path.exists(vis_g))


    def tearDown(self):
        os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')   
        pass
    
    def test1(self):
        '''Cvel 1: Testing default - expected error'''
        myvis = vis_b
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel()
        self.assertFalse(rval)
    
    def test2(self):
        '''Cvel 2: Only input vis set - expected error'''
        myvis = vis_b
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(vis = 'myinput.ms')
        self.assertEqual(rval,None)
            
    def test3(self):
        '''Cvel 3: Input and output vis set'''
        myvis = vis_b
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(vis = 'myinput.ms', outputvis = outfile)
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 64, 0)
        self.assertTrue(ret[0],ret[1])
    
    def test4(self):
        '''Cvel 4: I/O vis set, more complex input vis, one field selected'''
        myvis = vis_a        
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(vis = 'myinput.ms', outputvis = outfile, field = '1')
        self.assertNotEqual(rval,False)
        ret = (verify_ms(outfile, 1, 64, 0))
        self.assertTrue(ret[0],ret[1])

    def test5(self):
        '''Cvel 5: I/O vis set, more complex input vis, one field selected, passall = True'''
        myvis = vis_a
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
                    vis = 'myinput.ms',
                    outputvis = outfile,
                    field = '1',
                    passall = True
                    )
        self.assertNotEqual(rval,False)
        ret = (verify_ms(outfile, 1, 64, 0))
        self.assertTrue(ret[0],ret[1])

    def test6(self):
        '''Cvel 6: I/O vis set, more complex input vis, one field selected, one spw selected, passall = True'''
        myvis = vis_a
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = outfile,
            field = '1',
            spw = '0',
            nchan = 32,
            start = 10,
            passall = True
            )
        self.assertNotEqual(rval,False)
        ret = (verify_ms(outfile, 2, 32, 0))
        self.assertTrue(ret[0],ret[1])

    ## # Tests with more than one spectral window ###################
    
    def test7(self):
        '''Cvel 7: I/O vis set, input vis with two spws, one field selected, 2 spws selected, 
           passall = False'''
        myvis = vis_c
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
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
        '''Cvel 8: I/O vis set, input vis with two spws, one field selected, 2 spws selected, 
           passall = False, regridding 1'''
        myvis = vis_c
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
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
        '''Cvel 9: I/O vis set, input vis with two spws, one field selected, 2 spws selected, 
           passall = False, regridding 2'''
        myvis = vis_c
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
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
        '''Cvel10: I/O vis set, input vis with two spws, one field selected, 2 spws selected, passall = False, regridding 3...'''
        myvis = vis_c
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
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
        '''Cvel 11: I/O vis set, input vis with two spws, one field selected, 
           2 spws selected, passall = False, regridding 4...'''
        myvis = vis_c
        os.system('ln -sf ' + myvis + ' myinput.ms')

        rval = cvel(
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
        '''Cvel 12: Input and output vis set, input vis with two spws, two fields selected, 
           2 spws selected, passall = False, regridding 5...'''
        myvis = vis_c
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
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
        '''Cvel 13: I/O vis set, input vis with one spws, one field selected, one spws selected, 
           passall = False, regridding 6...'''
        myvis = vis_a
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
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
        '''Cvel 14: I/O vis set, input vis with one spws, one field selected, one spws selected, 
           passall = False, non-existing phase center...'''
        myvis = vis_a
        os.system('ln -sf ' + myvis + ' myinput.ms')
        try:
            rval = cvel(
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
        '''Cvel 15: I/O vis set, input vis with two spws, one field selected, 2 spws selected, passall = False, regridding 8...'''
        myvis = vis_c
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
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
        '''Cvel 16: I/O vis set, input vis with one spw, two fields selected, passall = False, regridding 9...'''
        myvis = vis_a
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
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
        '''Cvel 17: I/O vis set, input vis with one spw, two fields selected, passall = False, regridding 9...'''
        myvis = vis_a
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
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
        '''Cvel 18: I/O vis set, input vis with one spw, two fields selected, passall = False, regridding 9...'''
        myvis = vis_a
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
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
        '''Cvel 19: SMA input MS, 24 spws to combine, channel mode, 10 output channels'''
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
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
        '''Cvel 20: SMA input MS, 24 spws to combine, channel mode, 111 output channels'''
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
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
        '''Cvel 21: SMA input MS, 24 spws to combine, frequency mode, 21 output channels'''
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
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
        '''Cvel 22: SMA input MS, 24 spws to combine, frequency mode, 210 output channels, negative width'''
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
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
        '''Cvel 23: SMA input MS, 24 spws to combine, radio velocity mode, 30 output channels'''
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        vrad = (220398.676E6 - 229586E6)/220398.676E6 * 2.99792E8
        vwidth = ((220398.676E6 - 229586E6+1600E3)/220398.676E6 * 2.99792E8) - vrad
        vrad = vrad-vwidth/2.
        rval = cvel(
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
        '''Cvel 24: SMA input MS, 24 spws to combine, radio velocity mode, 35 output channels'''
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        vrad = (220398.676E6 - 229586E6)/220398.676E6 * 2.99792E8
        vwidth = ((220398.676E6 - 229586E6+3200E3)/220398.676E6 * 2.99792E8) - vrad
        vrad = vrad-vwidth/2.
        rval = cvel(
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
        '''Cvel 25: SMA input MS, 24 spws to combine, optical velocity mode, 40 output channels'''
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        lambda0 = 2.99792E8/220398.676E6
        lambda1 = 2.99792E8/229586E6
        lambda2 = 2.99792E8/(229586E6+1600E3)
        vopt = (lambda1-lambda0)/lambda0 * 2.99792E8
        vwidth = vopt - (lambda2-lambda0)/lambda0 * 2.99792E8
        vopt = vopt-vwidth/2.
        rval = cvel(
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
        '''Cvel 26: SMA input MS, 24 spws to combine, optical velocity mode, 40 output channels'''
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        lambda0 = 2.99792E8/220398.676E6
        lambda1 = 2.99792E8/229586E6
        vopt = (lambda1-lambda0)/lambda0 * 2.99792E8
        lambda2 = 2.99792E8/(229586E6+1200E3)
        vwidth = vopt - (lambda2-lambda0)/lambda0 * 2.99792E8
        vopt = vopt-vwidth/2.
        rval = cvel(
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
        '''Cvel 27: SMA input MS, 24 spws to combine, scratch columns, no regridding'''
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        # no regrid
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = outfile
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 2440, 0)
        self.assertTrue(ret[0],ret[1])
    
    def test28(self):
        '''Cvel 28: SMA input MS, 24 spws to combine, scratch columns, channel mode, 30 channels'''
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
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
        '''Cvel 29: SMA input MS, 24 spws to combine, scratch columns, channel mode, 31 channels'''
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
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
        '''Cvel 30: SMA input MS, 24 spws to combine, scratch columns, mode channel_b, no regridding'''
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode="channel_b"
            )
        
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 2425, 0)
        self.assertTrue(ret[0],ret[1])
    
    def test31(self):
        '''Cvel 31: SMA input MS, 24 spws to combine, scratch columns, mode channel, frame trafo'''
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
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
        '''Cvel 32: SMA input MS, 24 spws to combine, scratch columns, mode channel, frame trafo, Hanning smoothing'''
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
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
        '''Cvel 33: SMA input MS, 1 spw, scratch columns, mode channel, no trafo, Hanning smoothing'''
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
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
        '''Cvel 34: EVLA high-res input MS, 2 spws to combine'''
        myvis = vis_e
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode = 'velocity',
            restfreq  = '6035.092MHz'
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 260, 0)
        self.assertTrue(ret[0],ret[1])

    def test35(self):
        '''Cvel 35: test effect of sign of width parameter: channel mode, width positive'''
        myvis = vis_b
        os.system('ln -sf ' + myvis + ' myinput.ms')
        tb.open('myinput.ms/SPECTRAL_WINDOW')
        a = tb.getcell('CHAN_FREQ')
        b = numpy.array([a[1], a[2], a[3]])
        tb.close()

        rval = cvel(
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
        '''Cvel 36: test effect of sign of width parameter: channel mode, width negative'''
        myvis = vis_b
        os.system('ln -sf ' + myvis + ' myinput.ms')
        tb.open('myinput.ms/SPECTRAL_WINDOW')
        a = tb.getcell('CHAN_FREQ')
        b = numpy.array([a[1], a[2], a[3]])
        tb.close()

        rval = cvel(
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
        '''Cvel 37: test effect of sign of width parameter: freq mode, width positive'''
        myvis = vis_b
        os.system('ln -sf ' + myvis + ' myinput.ms')
        tb.open('myinput.ms/SPECTRAL_WINDOW')
        a = tb.getcell('CHAN_FREQ')
        b = numpy.array([a[1], a[2], a[3]])
        tb.close()

        rval = cvel(
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
        '''Cvel 38: test effect of sign of width parameter: freq mode, width negative'''
        myvis = vis_b
        os.system('ln -sf ' + myvis + ' myinput.ms')
        tb.open('myinput.ms/SPECTRAL_WINDOW')
        a = tb.getcell('CHAN_FREQ')
        b = numpy.array([a[1], a[2], a[3]])
        tb.close()

        rval = cvel(
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
        '''Cvel 39: test effect of sign of width parameter: radio velocity mode, width positive'''
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
        rval = cvel(
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
        '''Cvel 40: test effect of sign of width parameter: radio velocity mode, width negative'''
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
        rval = cvel(
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
        '''Cvel 41: test effect of sign of width parameter: optical velocity mode, width positive'''
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
        rval = cvel(
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
        '''Cvel 42: test effect of sign of width parameter: optical velocity mode, width negative'''
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
        rval = cvel(
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
        '''Cvel 43: SMA input MS, 1 spw, channel mode, nchan not set'''
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
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
        '''Cvel 44: SMA input MS, 2 spws to combine, channel mode, nchan not set'''
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
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
        '''Cvel 45: SMA input MS, 1 spw, channel mode, nchan not set, negative width'''
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
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
        '''Cvel 46: SMA input MS with descending freq, 24 spws, nchan=100'''
        myvis = vis_f
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
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
        '''Cvel 47: SMA input MS with descending freq, 1 spw, nchan not set'''
        myvis = vis_f
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
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
        '''Cvel 48: test fftshift regridding: channel mode, width positive'''
        myvis = vis_b
        os.system('ln -sf ' + myvis + ' myinput.ms')
        tb.open('myinput.ms/SPECTRAL_WINDOW')
        a = tb.getcell('CHAN_FREQ')
        b = numpy.array([a[1], a[2], a[3]])
        tb.close()

        rval = cvel(
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
        '''Cvel 49: vopt mode with fftshift, expected error ...'''
        myvis = vis_b
        os.system('ln -sf ' + myvis + ' myinput.ms')
        try:
            rval = cvel(
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
        '''Cvel 50: test fftshift regridding: channel mode, width positive'''
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')

        # get reference values by running cvel with linear interpol
        cvel(
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

        rval = cvel(
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
        '''Cvel 51: test fftshift regridding: frequency mode, width positive'''
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')

        # get reference values by running cvel with linear interpol
        cvel(
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

        rval = cvel(
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
        '''Cvel 52: test fftshift regridding: radio velocity mode, width positive'''
        myvis = vis_d
        os.system('ln -sf ' + myvis + ' myinput.ms')

        # get reference values by running cvel with linear interpol
        cvel(
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

        rval = cvel(
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
        '''Cvel 53: cvel of a field with ephemeris attached and outframe SOURCE'''
        myvis = vis_g
        os.system('ln -sf ' + myvis + ' myinput.ms')
        rval = cvel(
                vis = 'myinput.ms',
                outputvis = outfile,
                outframe = 'SOURCE'
                )
        self.assertTrue(rval)



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
        '''Cvel: Cleanup'''
        pass
        


def suite():
    return [cvel_test,cleanup]

