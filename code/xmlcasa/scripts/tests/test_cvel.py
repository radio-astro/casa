# unit test for the cvel task

import os
from __main__ import default
from tasks import *
from taskinit import *
import unittest

myname = 'test_cvel'
vis_a = 'ngc4826.ms'
vis_b = 'test.ms'
vis_c = 'jupiter6cm.demo.ms'
vis_d = 'ngc4826.tutorial.ngc4826.ll.5.ms'
vis_e = 'g19_d2usb_targets_line-shortened.ms'
vis_f = 'evla-highres-sample.ms'
outfile = 'cvel-output.ms'

def verify_ms(msname, expnumspws, expnumchan, inspw):
    msg = ''
    tb.open(msname+'/SPECTRAL_WINDOW')
    nc = tb.getcell("NUM_CHAN", inspw)
    nr = tb.nrows()
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
        msg = "Found "+ str(nc) +", expected "+str(expnumchan)+" channels in FLAG column in "+msname
        return [False,msg]
    
    return [True,msg]

#def verify_ms(msname, expnumspws, expnumchan, inspw):
#    msg = ''
#    tb.open(msname+'/SPECTRAL_WINDOW')
#    nc = tb.getcell("NUM_CHAN", inspw)
#    nr = tb.nrows()
#    tb.close()
#    tb.open(msname)
#    dimdata = tb.getcell("FLAG", 0)[0].size
#    tb.close()
#    if not (nr==expnumspws):
#        msg = '"Found '+str(nr)+', expected '+str(expnumspws)+' spectral windows in '+msname
#        return [False,msg]
##        print "Found "+str(nr)+", expected "+str(expnumspws)+" spectral windows in "+msname
##        raise Exception
#    if not (nc == expnumchan):
#        msg = "Found "+ str(nc) +", expected "+str(expnumchan)+" channels in spw "+str(inspw)+" in "+msname
#        return [False,msg]
##        print "Found "+ str(nc) +", expected "+str(expnumchan)+" channels in spw "+str(inspw)+" in "+msname
##        raise Exception
#    if not (dimdata == expnumchan):
#        msg = "Found "+ str(nc) +", expected "+str(expnumchan)+" channels in FLAG column in "+msname
#        return [False,msg]
##        print "Found "+ str(nc) +", expected "+str(expnumchan)+" channels in FLAG column in "+msname
##        raise Exception
#    
#    return [True,msg]

class cvel_test(unittest.TestCase):

    def setUp(self):    
        default('cvel')
        
        if(not os.path.exists(vis_a)):
            importuvfits(fitsfile=os.environ['CASAPATH'].split()[0]+'/data/regression/ngc4826/fitsfiles/ngc4826.ll.fits5', 
                         vis=vis_a)
        if(not os.path.exists(vis_b)):
            os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/fits-import-export/input/test.ms .')
        if(not os.path.exists(vis_c)):
            importuvfits(fitsfile=os.environ['CASAPATH'].split()[0]+'/data/regression/jupiter6cm/jupiter6cm.fits', 
                         vis=vis_c)
        if(not os.path.exists(vis_d)):
            importuvfits(fitsfile=os.environ['CASAPATH'].split()[0]+'/data/regression/ngc4826/fitsfiles/ngc4826.ll.fits5', 
                         vis=vis_d)
        if(not os.path.exists(vis_e)):
            os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/cvel/input/g19_d2usb_targets_line-shortened.ms .')
        if(not os.path.exists(vis_f)):
            os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/cvel/input/evla-highres-sample.ms .')


    def tearDown(self):
        os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')   
        pass
    
    def test1(self):
        '''Cvel 1: Testing default'''
        myvis = vis_b
        os.system('cp -R ' + myvis + ' myinput.ms')
        rval = cvel()
        self.assertFalse(rval)
    
    def test2(self):
        '''Cvel 2: Only input vis set'''
        myvis = vis_b
        os.system('cp -R ' + myvis + ' myinput.ms')
        rval = cvel(vis = 'myinput.ms')
        self.assertEqual(rval,None)
            
    def test3(self):
        '''Cvel 3: Input and output vis set'''
        myvis = vis_b
        os.system('cp -R ' + myvis + ' myinput.ms')
        rval = cvel(vis = 'myinput.ms', outputvis = outfile)
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 64, 0)
        self.assertTrue(ret[0],ret[1])
    
    def test4(self):
        '''Cvel 4: I/O vis set, more complex input vis, one field selected'''
        myvis = vis_a        
        os.system('cp -R ' + myvis + ' myinput.ms')
        rval = cvel(vis = 'myinput.ms', outputvis = outfile, field = '1')
        self.assertNotEqual(rval,False)
        ret = (verify_ms(outfile, 1, 64, 0))
        self.assertTrue(ret[0],ret[1])

    def test5(self):
        '''Cvel 5: I/O vis set, more complex input vis, one field selected, passall = True'''
        myvis = vis_a
        os.system('cp -R ' + myvis + ' myinput.ms')
        rval = cvel(
                    vis = 'myinput.ms',
                    outputvis = outfile,
                    field = '1',
                    passall = True
                    )
        self.assertNotEqual(rval,False)
        ret = (verify_ms(outfile, 2, 64, 0))
        self.assertTrue(ret[0],ret[1])

    def test6(self):
        '''Cvel 6: I/O vis set, more complex input vis, one field selected, one spw selected, passall = True'''
        myvis = vis_a
        os.system('cp -R ' + myvis + ' myinput.ms')
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = outfile,
            field = '1',
            spw = '0',
            passall = True
            )
        self.assertNotEqual(rval,False)
        ret = (verify_ms(outfile, 2, 64, 0))
        self.assertTrue(ret[0],ret[1])

    ## # Tests with more than one spectral window ###################
    
    def test7(self):
        '''Cvel 7: I/O vis set, input vis with two spws, one field selected, 2 spws selected, 
           passall = False'''
        myvis = vis_c
        os.system('cp -R ' + myvis + ' myinput.ms')
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
        os.system('cp -R ' + myvis + ' myinput.ms')
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
        os.system('cp -R ' + myvis + ' myinput.ms')
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
        os.system('cp -R ' + myvis + ' myinput.ms')
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
        os.system('cp -R ' + myvis + ' myinput.ms')

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
        os.system('cp -R ' + myvis + ' myinput.ms')
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
        os.system('cp -R ' + myvis + ' myinput.ms')
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
        os.system('cp -R ' + myvis + ' myinput.ms')
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
        os.system('cp -R ' + myvis + ' myinput.ms')
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
        myvis = vis_d
        os.system('cp -R ' + myvis + ' myinput.ms')
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
        myvis = vis_d
        os.system('cp -R ' + myvis + ' myinput.ms')
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
        myvis = vis_d
        os.system('cp -R ' + myvis + ' myinput.ms')
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
        myvis = vis_e
        os.system('cp -R ' + myvis + ' myinput.ms')
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
        myvis = vis_e
        os.system('cp -R ' + myvis + ' myinput.ms')
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
        myvis = vis_e
        os.system('cp -R ' + myvis + ' myinput.ms')
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
        '''Cvel 22: SMA input MS, 24 spws to combine, frequency mode, 210 output channels, negative width (sign will be ignored)'''
        myvis = vis_e
        os.system('cp -R ' + myvis + ' myinput.ms')
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
    
    def test23(self):
        '''Cvel 23: SMA input MS, 24 spws to combine, radio velocity mode, 30 output channels'''
        myvis = vis_e
        os.system('cp -R ' + myvis + ' myinput.ms')
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
        myvis = vis_e
        os.system('cp -R ' + myvis + ' myinput.ms')
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
        myvis = vis_e
        os.system('cp -R ' + myvis + ' myinput.ms')
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
        myvis = vis_e
        os.system('cp -R ' + myvis + ' myinput.ms')
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
        myvis = vis_e
        os.system('cp -R ' + myvis + ' myinput.ms')
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
        myvis = vis_e
        os.system('cp -R ' + myvis + ' myinput.ms')
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
        myvis = vis_e
        os.system('cp -R ' + myvis + ' myinput.ms')
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
        myvis = vis_e
        os.system('cp -R ' + myvis + ' myinput.ms')
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode="channel_b"
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 2443, 0)
        self.assertTrue(ret[0],ret[1])
    
    def test31(self):
        '''Cvel 31: SMA input MS, 24 spws to combine, scratch columns, mode channel, frame trafo'''
        myvis = vis_e
        os.system('cp -R ' + myvis + ' myinput.ms')
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
        myvis = vis_e
        os.system('cp -R ' + myvis + ' myinput.ms')
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
        myvis = vis_e
        os.system('cp -R ' + myvis + ' myinput.ms')
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
        '''Cvel 34: EVAL high-res input MS, 2 spws to combine'''
        myvis = vis_f
        os.system('cp -R ' + myvis + ' myinput.ms')
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = outfile,
            mode = 'velocity',
            restfreq  = '6035.092MHz'
            )
        self.assertNotEqual(rval,False)
        ret = verify_ms(outfile, 1, 260, 0)
        self.assertTrue(ret[0],ret[1])

def suite():
    return [cvel_test]

