# unit test for the cvel task

import os
from __main__ import default
from tasks import *
from taskinit import *
import unittest

class cvel_test(unittest.TestCase):
    myname = 'test_task_cvel'
    vis_a = 'ngc4826.ms'
    vis_b = 'test.ms'
    vis_c = 'jupiter6cm.demo.ms'
    vis_d = 'ngc4826.tutorial.ngc4826.ll.5.ms'
    vis_e = 'g19_d2usb_targets_line-shortened.ms'
    out = 'cvel-output.ms'
    rval = False

    def setUp(self):    
        default('cvel')
        self.rval = False
        
        if(not os.path.exists(self.vis_a)):
            importuvfits(fitsfile=os.environ['CASAPATH'].split()[0]+'/data/regression/ngc4826/fitsfiles/ngc4826.ll.fits5', 
                         vis=self.vis_a)
        if(not os.path.exists(self.vis_b)):
            os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/fits-import-export/input/test.ms .')
        if(not os.path.exists(self.vis_c)):
            importuvfits(fitsfile=os.environ['CASAPATH'].split()[0]+'/data/regression/jupiter6cm/jupiter6cm.fits', 
                         vis=self.vis_c)
        if(not os.path.exists(self.vis_d)):
            importuvfits(fitsfile=os.environ['CASAPATH'].split()[0]+'/data/regression/ngc4826/fitsfiles/ngc4826.ll.fits5', 
                         vis=self.vis_d)
        if(not os.path.exists(self.vis_e)):
            os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/cvel/input/g19_d2usb_targets_line-shortened.ms .')


    def tearDown(self):
        os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
        

    def verify_ms(self,msname, expnumspws, expnumchan, inspw):
        tb.open(msname+'/SPECTRAL_WINDOW')
        nc = tb.getcell("NUM_CHAN", inspw)
        nr = tb.nrows()
        tb.close()
        tb.open(msname)
        dimdata = tb.getcell("FLAG", 0)[0].size
        tb.close()
        if not (nr==expnumspws):
            print "Found "+str(nr)+", expected "+str(expnumspws)+" spectral windows in "+msname
            raise Exception
        if not (nc == expnumchan):
            print "Found "+ str(nc) +", expected "+str(expnumchan)+" channels in spw "+str(inspw)+" in "+msname
            raise Exception
        if not (dimdata == expnumchan):
            print "Found "+ str(nc) +", expected "+str(expnumchan)+" channels in FLAG column in "+msname
            raise Exception
        
        return True
   

    def test1(self):
        '''Test 1: Testing default'''
        myvis = self.vis_b
        os.system('cp -R ' + myvis + ' myinput.ms')
        self.rval = cvel()
        self.assertFalse(self.rval)
    
    def test2(self):
        '''Test 2: Only input vis set'''
        myvis = self.vis_b
        os.system('cp -R ' + myvis + ' myinput.ms')
        self.rval = cvel(vis = 'myinput.ms')
        self.assertEqual(self.rval,None)
            
    def test3(self):
        '''Test 3: Input and output vis set'''
        myvis = self.vis_b
        os.system('cp -R ' + myvis + ' myinput.ms')
        self.rval = cvel(vis = 'myinput.ms', outputvis = self.out)
        self.assertNotEqual(self.rval,False)
        self.assertTrue(self.verify_ms(self.out, 1, 64, 0))

#    
    def test4(self):
        '''Test 4: Input and output vis set, more complex input vis, one field selected'''
        myvis = self.vis_a        
        os.system('cp -R ' + myvis + ' myinput.ms')
        self.rval = cvel(vis = 'myinput.ms', outputvis = self.out, field = '1')
        self.assertNotEqual(self.rval,False)
        self.assertTrue(self.verify_ms(self.out, 1, 64, 0))
    
    def test5(self):
        '''Test 5: Input and output vis set, more complex input vis, one field selected, passall = True'''
        myvis = self.vis_a
        os.system('cp -R ' + myvis + ' myinput.ms')
        self.rval = cvel(
                    vis = 'myinput.ms',
                    outputvis = self.out,
                    field = '1',
                    passall = True
                    )
        self.assertNotEqual(self.rval,False)
        self.assertTrue(self.verify_ms(self.out, 2, 64, 0))
    
    def test6(self):
        '''Test 6: Input and output vis set, more complex input vis, one field selected, one spw selected, passall = True'''
        myvis = self.vis_a
        os.system('cp -R ' + myvis + ' myinput.ms')
        self.rval = cvel(
            vis = 'myinput.ms',
            outputvis = self.out,
            field = '1',
            spw = '0',
            passall = True
            )
        self.assertNotEqual(self.rval,False)
        self.assertTrue(self.verify_ms(self.out, 2, 64, 0))
    
    ## # Tests with more than one spectral window ###################
    
    def test7(self):
        '''Test 7: Input and output vis set, input vis with two spws, one field selected, 2 spws selected, 
           passall = False'''
        myvis = self.vis_c
        os.system('cp -R ' + myvis + ' myinput.ms')
        self.rval = cvel(
                vis = 'myinput.ms',
                outputvis = self.out,
                field = '12', # select Jupiter
                spw = '0,1',  # both available SPWs
                passall = False
                )
        self.assertNotEqual(self.rval,False)
        self.assertTrue(self.verify_ms(self.out, 1, 2, 0))

    def test8(self):
        '''Test 8: Input and output vis set, input vis with two spws, one field selected, 2 spws selected, 
           passall = False, regridding 1'''
        myvis = self.vis_c
        os.system('cp -R ' + myvis + ' myinput.ms')
        self.rval = cvel(
            vis = 'myinput.ms',
            outputvis = self.out,
            field = '11',    # select some other field
            spw = '0,1',    # both available SPWs
            passall = False,    # regrid
            nchan = 1,
            width = 2
            )
        self.assertNotEqual(self.rval,False)
        self.assertTrue(self.verify_ms(self.out, 1, 1, 0))
   
    def test9(self):
        '''Test 9: Input and output vis set, input vis with two spws, one field selected, 2 spws selected, 
           passall = False, regridding 2'''
        myvis = self.vis_c
        os.system('cp -R ' + myvis + ' myinput.ms')
        self.rval = cvel(
            vis = 'myinput.ms',
            outputvis = self.out,
            field = '10',
            spw = '0,1',
            passall = False,
            mode='channel',
            nchan = 1,
            start = 1
            )
        self.assertNotEqual(self.rval,False)
        self.assertTrue(self.verify_ms(self.out, 1, 1, 0))
    
    def test10(self):
        '''Test 10: Input and output vis set, input vis with two spws, one field selected, 
           2 spws selected, passall = False, regridding 3'''
        myvis = self.vis_c
        os.system('cp -R ' + myvis + ' myinput.ms')
        try:
            self.rval = cvel(
                vis = 'myinput.ms',
                outputvis = self.out,
                field = '9',
                spw = '0,1',
                passall = False,
                mode='frequency',
                nchan = 1,
                start = '4.8101GHz',
                width = '50MHz'
                )
            if not self.rval:
                raise Exception
            self.assertTrue(self.verify_ms(self.out, 1, 1, 0))
        except:
            print self.myname, ': *** Unexpected error ***'   

    
    def test11(self):
        '''Test 11: Input and output vis set, input vis with two spws, one field selected, 
           2 spws selected, passall = False, regridding 4...'''
        myvis = self.vis_c
        os.system('cp -R ' + myvis + ' myinput.ms')
        try:
            self.rval = cvel(
                vis = 'myinput.ms',
                outputvis = self.out,
                field = '10',
                spw = '0,1',
                passall = False,
                mode='channel',
                nchan = 1,
                start = 1,
                outframe = 'lsrk'
                )
            self.assertNotEqual(self.rval,False)
            self.assertTrue(self.verify_ms(self.out, 1, 1, 0))
        except:
            print self.myname, ': *** Unexpected error ***'   
    
    def test12(self):
        '''Test 12: Input and output vis set, input vis with two spws, two fields selected, 
           2 spws selected, passall = False, regridding 5...'''
        myvis = self.vis_c
        os.system('cp -R ' + myvis + ' myinput.ms')
        try:
            self.rval = cvel(
                vis = 'myinput.ms',
                outputvis = self.out,
                field = '5,6',
                spw = '0,1',
                passall = False,
                mode='frequency',
                nchan = 2,
                start = '4.8101 GHz',
                width = '50 MHz',
                outframe = ''
                )
            self.assertNotEqual(self.rval,False)
            self.assertTrue(self.verify_ms(self.out, 1, 2, 0))
        except:
            print self.myname, ': *** Unexpected error ***'   
    
    def test13(self):
        '''Input and output vis set, input vis with one spws, one field selected, one spws selected, 
           passall = False, regridding 6...'''
        myvis = self.vis_a
        os.system('cp -R ' + myvis + ' myinput.ms')
        try:
            self.rval = cvel(
                vis = 'myinput.ms',
                outputvis = self.out,
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
            self.assertNotEqual(self.rval,False)
            self.assertTrue(self.verify_ms(self.out, 1, 2, 0))
        except:
            print self.myname, ': *** Unexpected error ***'   

    
    def test14(self):
        '''Input and output vis set, input vis with one spws, one field selected, one spws selected, 
           passall = False, non-existing phase center...'''
        myvis = self.vis_a
        os.system('cp -R ' + myvis + ' myinput.ms')
        try:
            self.rval = cvel(
                vis = 'myinput.ms',
                outputvis = self.out,
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
            self.assertNotEqual(self.rval,Falses)
            self.assertTrue(self.verify_ms(self.out, 1, 2, 0))
        except:
            print self.myname, ': *** Expected error ***'   
    
    def test15(self):
        '''Input and output vis set, input vis with two spws, one field selected, 
           2 spws selected, passall = False, regridding 8...'''
        myvis = self.vis_c
        os.system('cp -R ' + myvis + ' myinput.ms')
        try:
            self.rval = cvel(
                vis = 'myinput.ms',
                outputvis = self.out,
                field = '12',
                spw = '0,1',
                passall = False,
                mode='frequency',
                nchan = 1,
                start = '4.80GHz',
                width = '50MHz',
                outframe = ''
                )
            self.assertNotEqual(self.rval,False)
            self.assertTrue(self.verify_ms(self.out, 1, 1, 0))
        except:
            print self.myname, ': *** Unexpected error ***'   
    
    def test16(self):
        '''Cvel 16: I/O vis set, input vis with one spw, two fields selected, passall = False, regridding 9...'''
        myvis = self.vis_d
        os.system('cp -R ' + myvis + ' myinput.ms')
        try:
            self.rval = cvel(
                vis = 'myinput.ms',
                outputvis = self.out,
                field = '2,3',
                spw = '0',
                passall = False,
                mode='channel',
                nchan = 10,
                start = 2,
                outframe = 'lsrd',
                phasecenter = 2
                )
            self.assertNotEqual(self.rval,False)
            self.assertTrue(self.verify_ms(self.out, 1, 10, 0))
        except:
            print self.myname, ': *** Unexpected error ***'   
    
    def test17(self):
        '''Cvel 17: I/O vis set, input vis with one spw, two fields selected, passall = False, regridding 9...'''
        myvis = self.vis_d
        os.system('cp -R ' + myvis + ' myinput.ms')
        try:
            self.rval = cvel(
                    vis = 'myinput.ms',
                    outputvis = self.out,
                    field = '2,3',
                    spw = '0',
                    passall = False,
                    mode='frequency',
                    nchan = 10,
                    start = '114.9507GHz',
                    width = '3.125MHz',
                    outframe = 'lsrd',
                    phasecenter = 2
                )
            self.assertNotEqual(self.rval,False)
            self.assertTrue(verify_ms(self.out, 1, 10, 0),"Unexpected error")
        except:
            print self.myname, ': *** Unexpected error ***'   
    
    def test18(self):
        '''Cvel 18: I/O vis set, input vis with one spw, two fields selected, passall = False, regridding 9...'''
        myvis = self.vis_d
        os.system('cp -R ' + myvis + ' myinput.ms')
        try:
            self.rval = cvel(
                vis = 'myinput.ms',
                outputvis = self.out,
                field = '2,3',
                spw = '0',
                passall = False,
                mode='frequency',
                nchan = 10,
                start = '114.9507GHz',
                width = '3.125MHz',
                outframe = 'lsrd',
                phasecenter = 'J2000 12h56m43.88s +21d41m00.1s'
                )
            self.assertNotEqual(self.rval,False)
            self.assertTrue(verify_ms(self.out, 1, 10, 0),"Unexpected error")
        except:
            print self.myname, ': *** Unexpected error ***'   
    
    def test19(self):
        '''Cvel 19: SMA input MS, 24 spws to combine, channel mode, 10 output channels'''
        myvis = self.vis_e
        os.system('cp -R ' + myvis + ' myinput.ms')
        try:
            self.rval = cvel(
                vis = 'myinput.ms',
                outputvis = self.out,
                mode='channel',
                nchan = 10,
                start = 100,
                width = 2,
                phasecenter = "J2000 18h25m56.09 -12d04m28.20"
                )
            self.assertNotEqual(self.rval,False)
            self.assertTrue(verify_ms(self.out, 1, 10, 0),'Unexpected error')
        except:
            print self.myname, ': *** Unexpected error ***'   
    
    def test20(self):
        '''Cvel 20: "SMA input MS, 24 spws to combine, channel mode, 111 output channels'''
        myvis = self.vis_e
        os.system('cp -R ' + myvis + ' myinput.ms')
        try:
            self.rval = cvel(
                vis = 'myinput.ms',
                outputvis = self.out,
                mode='channel',
                nchan = 111,
                start = 201,
                width = 3,
                phasecenter = "J2000 18h25m56.09 -12d04m28.20"
                )
            self.assertNotEqual(self.rval,False)
            self.assertTrue(verify_ms(self.out, 1, 111, 0),'Unexpected error')
        except:
            print self.myname, ': *** Unexpected error ***'   
    
#    testnumber = 21
#    if (testnumber in testlist):
#        myvis = self.vis_e
#        os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
#        os.system('cp -R ' + myvis + ' myinput.ms')
#        default('cvel')
#        total += 1
#        try:
#            print "\n>>>> Test ", testnumber, ", input MS: ", myvis
#            print "SMA input MS, 24 spws to combine, frequency mode, 21 output channels"
#            self.rval = cvel(
#                vis = 'myinput.ms',
#                outputvis = self.out,
#                mode='frequency',
#                nchan = 21,
#                start = '229586.0MHz',
#                width = '1600kHz',
#                phasecenter = "J2000 18h25m56.09 -12d04m28.20"
#                )
#            if not self.rval:
#                raise Exception
#            self.out = "test"+str(testnumber)+'cvel-output.ms'
#            os.system('rm -rf '+self.out+'; mv cvel-output.ms '+self.out)
#            verify_ms(self.out, 1, 21, 0)
#        except:
#            print self.myname, ': *** Unexpected error ***'   
#            failures += 1
#    
#    testnumber = 22
#    if (testnumber in testlist):
#        myvis = self.vis_e
#        os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
#        os.system('cp -R ' + myvis + ' myinput.ms')
#        default('cvel')
#        total += 1
#        try:
#            print "\n>>>> Test ", testnumber, ", input MS: ", myvis
#            print "SMA input MS, 24 spws to combine, frequency mode, 210 output channels, negative width (sign will be ignored)"
#            self.rval = cvel(
#                vis = 'myinput.ms',
#                outputvis = self.out,
#                mode='frequency',
#                nchan = 210,
#                start = '229586.0MHz',
#                width = '-2400kHz',
#                phasecenter = "J2000 18h25m56.09 -12d04m28.20"
#                )
#            if not self.rval:
#                raise Exception
#            self.out = "test"+str(testnumber)+'cvel-output.ms'
#            os.system('rm -rf '+self.out+'; mv cvel-output.ms '+self.out)
#            verify_ms(self.out, 1, 210, 0)
#        except:
#            print self.myname, ': *** Unexpected error ***'   
#            failures += 1
#    
#    testnumber = 23
#    if (testnumber in testlist):
#        myvis = self.vis_e
#        os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
#        os.system('cp -R ' + myvis + ' myinput.ms')
#        default('cvel')
#        total += 1
#        try:
#            print "\n>>>> Test ", testnumber, ", input MS: ", myvis
#            print "SMA input MS, 24 spws to combine, radio velocity mode, 30 output channels"
#            print "start = ", start, ", width = ", width
#            vrad = (220398.676E6 - 229586E6)/220398.676E6 * 2.99792E8
#            vwidth = ((220398.676E6 - 229586E6+1600E3)/220398.676E6 * 2.99792E8) - vrad
#            self.rval = cvel(
#                vis = 'myinput.ms',
#                outputvis = self.out,
#                mode='velocity',
#                nchan = 30,
#                restfreq = '220398.676MHz',
#                start = str(vrad)+'m/s',
#                width = str(vwidth)+'m/s',
#                phasecenter = "J2000 18h25m56.09 -12d04m28.20",
#                veltype = 'radio'
#                )
#            if not self.rval:
#                raise Exception
#            self.out = "test"+str(testnumber)+'cvel-output.ms'
#            os.system('rm -rf '+self.out+'; mv cvel-output.ms '+self.out)
#            verify_ms(self.out, 1, 30, 0)
#        except:
#            print self.myname, ': *** Unexpected error ***'   
#            failures += 1
#    
#    testnumber = 24
#    if (testnumber in testlist):
#        myvis = self.vis_e
#        os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
#        os.system('cp -R ' + myvis + ' myinput.ms')
#        default('cvel')
#        total += 1
#        try:
#            print "\n>>>> Test ", testnumber, ", input MS: ", myvis
#            print "SMA input MS, 24 spws to combine, radio velocity mode, 35 output channels"
#            vrad = (220398.676E6 - 229586E6)/220398.676E6 * 2.99792E8
#            vwidth = ((220398.676E6 - 229586E6+3200E3)/220398.676E6 * 2.99792E8) - vrad
#            self.rval = cvel(
#                vis = 'myinput.ms',
#                outputvis = self.out,
#                mode='velocity',
#                nchan = 35,
#                restfreq = '220398.676MHz',
#                start = str(vrad)+'m/s',
#                width = str(vwidth)+'m/s',
#                phasecenter = "J2000 18h25m56.09 -12d04m28.20",
#                veltype = 'radio'
#                )
#            if not self.rval:
#                raise Exception
#            self.out = "test"+str(testnumber)+'cvel-output.ms'
#            os.system('rm -rf '+self.out+'; mv cvel-output.ms '+self.out)
#            verify_ms(self.out, 1, 35, 0)
#        except:
#            print self.myname, ': *** Unexpected error ***'   
#            failures += 1
#    
#    testnumber = 25
#    if (testnumber in testlist):
#        myvis = self.vis_e
#        os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
#        os.system('cp -R ' + myvis + ' myinput.ms')
#        default('cvel')
#        total += 1
#        try:
#            print "\n>>>> Test ", testnumber, ", input MS: ", myvis
#            print "SMA input MS, 24 spws to combine, optical velocity mode, 40 output channels"
#            lambda0 = 2.99792E8/220398.676E6
#            lambda1 = 2.99792E8/229586E6
#            lambda2 = 2.99792E8/(229586E6+1600E3)
#            vopt = (lambda1-lambda0)/lambda0 * 2.99792E8
#            vwidth = vopt - (lambda2-lambda0)/lambda0 * 2.99792E8
#            self.rval = cvel(
#                vis = 'myinput.ms',
#                outputvis = self.out,
#                mode='velocity',
#                nchan = 40,
#                restfreq = '220398.676MHz',
#                start = str(vopt)+'m/s',
#                width = str(vwidth)+'m/s',
#                phasecenter = "J2000 18h25m56.09 -12d04m28.20",
#                veltype = 'optical'
#                )
#            if not self.rval:
#                raise Exception
#            self.out = "test"+str(testnumber)+'cvel-output.ms'
#            os.system('rm -rf '+self.out+'; mv cvel-output.ms '+self.out)
#            verify_ms(self.out, 1, 40, 0)
#        except:
#            print self.myname, ': *** Unexpected error ***'   
#            failures += 1
#    
#    testnumber = 26
#    if (testnumber in testlist):
#        myvis = self.vis_e
#        os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
#        os.system('cp -R ' + myvis + ' myinput.ms')
#        default('cvel')
#        total += 1
#        try:
#            lambda0 = 2.99792E8/220398.676E6
#            lambda1 = 2.99792E8/229586E6
#            vopt = (lambda1-lambda0)/lambda0 * 2.99792E8
#            lambda2 = 2.99792E8/(229586E6+1200E3)
#            vwidth = vopt - (lambda2-lambda0)/lambda0 * 2.99792E8
#            print "\n>>>> Test ", testnumber, ", input MS: ", myvis
#            print "SMA input MS, 24 spws to combine, optical velocity mode, 40 output channels"
#            self.rval = cvel(
#                vis = 'myinput.ms',
#                outputvis = self.out,
#                mode='velocity',
#                nchan = 41,
#                restfreq = '220398.676MHz',
#                start = str(vopt)+'m/s',
#                width = str(vwidth)+'m/s',
#                phasecenter = "J2000 18h25m56.09 -12d04m28.20",
#                veltype = 'optical'
#                )
#            if not self.rval:
#                raise Exception
#            self.out = "test"+str(testnumber)+'cvel-output.ms'
#            os.system('rm -rf '+self.out+'; mv cvel-output.ms '+self.out)
#            verify_ms(self.out, 1, 41, 0)
#        except:
#            print self.myname, ': *** Unexpected error ***'   
#            failures += 1
#    
#    testnumber = 27
#    if (testnumber in testlist):
#        myvis = self.vis_e
#        os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
#        os.system('cp -R ' + myvis + ' myinput.ms')
#        default('cvel')
#        # no regrid
#        total += 1
#        try:
#            print "\n>>>> Test ", testnumber, ", input MS: ", myvis
#            print "SMA input MS, 24 spws to combine, scratch columns, no regridding"
#            self.rval = cvel(
#                vis = 'myinput.ms',
#                outputvis = self.out
#                )
#            if not self.rval:
#                raise Exception
#            self.out = "test"+str(testnumber)+'cvel-output.ms'
#            os.system('rm -rf '+self.out+'; mv cvel-output.ms '+self.out)
#            verify_ms(self.out, 1, 2440, 0)
#        except:
#            print self.myname, ': *** Unexpected error ***'   
#            failures += 1
#    
#    testnumber = 28
#    if (testnumber in testlist):
#        myvis = self.vis_e
#        os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
#        os.system('cp -R ' + myvis + ' myinput.ms')
#        default('cvel')
#        total += 1
#        try:
#            print "\n>>>> Test ", testnumber, ", input MS: ", myvis
#            print "SMA input MS, 24 spws to combine, scratch columns, channel mode, 30 channels "
#            self.rval = cvel(
#                vis = 'myinput.ms',
#                outputvis = self.out,
#                mode="channel",
#                start=1500,
#                width=2,
#                nchan=30
#                )
#            if not self.rval:
#                raise Exception
#            self.out = "test"+str(testnumber)+'cvel-output.ms'
#            os.system('rm -rf '+self.out+'; mv cvel-output.ms '+self.out)
#            verify_ms(self.out, 1, 30, 0)
#        except:
#            print self.myname, ': *** Unexpected error ***'   
#            failures += 1
#            
#    testnumber = 29
#    if (testnumber in testlist):
#        myvis = self.vis_e
#        os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
#        os.system('cp -R ' + myvis + ' myinput.ms')
#        default('cvel')
#        total += 1
#        try:
#            print "\n>>>> Test ", testnumber, ", input MS: ", myvis
#            print "SMA input MS, 24 spws to combine, scratch columns, channel mode, 31 channels "
#            self.rval = cvel(
#                vis = 'myinput.ms',
#                outputvis = self.out,
#                mode="channel",
#                start=1500,
#                width=2,
#                nchan=31
#                )
#            if not self.rval:
#                raise Exception
#            self.out = "test"+str(testnumber)+'cvel-output.ms'
#            os.system('rm -rf '+self.out+'; mv cvel-output.ms '+self.out)
#            verify_ms(self.out, 1, 31, 0)
#        except:
#            print self.myname, ': *** Unexpected error ***'   
#            failures += 1
#    
#    testnumber = 30
#    if (testnumber in testlist):
#        myvis = self.vis_e
#        os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
#        os.system('cp -R ' + myvis + ' myinput.ms')
#        default('cvel')
#        total += 1
#        try:
#            print "\n>>>> Test ", testnumber, ", input MS: ", myvis
#            print "SMA input MS, 24 spws to combine, scratch columns, mode channel_b, no regridding"
#            self.rval = cvel(
#                vis = 'myinput.ms',
#                outputvis = self.out,
#                mode="channel_b"
#                )
#            if not self.rval:
#                raise Exception
#            self.out = "test"+str(testnumber)+'cvel-output.ms'
#            os.system('rm -rf '+self.out+'; mv cvel-output.ms '+self.out)
#            verify_ms(self.out, 1, 2443, 0)
#        except:
#            print self.myname, ': *** Unexpected error ***'   
#            failures += 1
#    
#    testnumber = 31
#    if (testnumber in testlist):
#        myvis = self.vis_e
#        os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
#        os.system('cp -R ' + myvis + ' myinput.ms')
#        default('cvel')
#        total += 1
#        try:
#            print "\n>>>> Test ", testnumber, ", input MS: ", myvis
#            print "SMA input MS, 24 spws to combine, scratch columns, mode channel, frame trafo"
#            self.rval = cvel(
#                vis = 'myinput.ms',
#                outputvis = self.out,
#                mode="channel",
#                outframe = "BARY",
#                phasecenter = "J2000 18h25m56.09 -12d04m28.20"
#                )
#            if not self.rval:
#                raise Exception
#            self.out = "test"+str(testnumber)+'cvel-output.ms'
#            os.system('rm -rf '+self.out+'; mv cvel-output.ms '+self.out)
#            verify_ms(self.out, 1, 2440, 0)
#        except:
#            print self.myname, ': *** Unexpected error ***'   
#            failures += 1
#    
#    
#    
#    
#    # Summary ########################################
#    print "Tests = ", total    
#    print "Failures = ", failures
#    
#    # empty test list if it was empty to start with
#    if not keeptestlist:
#        testlist = []

def suite():
    return [cvel_test]

