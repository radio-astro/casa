# unit test for the cvel task

myname = 'test_task_cvel'

import os
vis_a = 'ngc4826.ms'
vis_b = 'test.ms'
vis_c = 'jupiter6cm.demo.ms'
vis_d = 'ngc4826.tutorial.ngc4826.ll.5.ms'
vis_e = 'g19_d2usb_targets_line-shortened.ms'

if(not os.path.exists(vis_a)):
    importuvfits(fitsfile=os.environ['CASADATA']+'/regression/ngc4826/fitsfiles/ngc4826.ll.fits5', vis=vis_a)
if(not os.path.exists(vis_b)):
    os.system('cp -R '+os.environ['CASADATA']+'/regression/fits-import-export/input/test.ms .')
if(not os.path.exists(vis_c)):
    importuvfits(fitsfile=os.environ['CASADATA']+'/regression/jupiter6cm/jupiter6cm.fits', vis=vis_c)
if(not os.path.exists(vis_d)):
    importuvfits(fitsfile=os.environ['CASADATA']+'/regression/ngc4826/fitsfiles/ngc4826.ll.fits5', vis=vis_d)
if(not os.path.exists(vis_e)):
    os.system('cp -R '+os.environ['CASADATA']+'/regression/cvel/input/g19_d2usb_targets_line-shortened.ms .')

def verify_ms(msname, expnumspws, expnumchan, inspw):
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

keeptestlist = True

try:
    print "List of tests to be executed ...", testlist
except:
    print "Global variable testlist not set."
    testlist = []
if (testlist==[]):
    print "testlist empty. Executing all tests."
    testlist = range(0,100)
    keeptestlist = False

total = 0
failures = 0
rval = False

testnumber = 1
if (testnumber in testlist):
    myvis = vis_b
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "Testing default."
        rval = cvel()
        if not rval:
            print myname, ': *** Error as expected ***'   
        else:
            print "Failed ..."
            failures +=1
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 2
if (testnumber in testlist):
    myvis = vis_b
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "Only input vis set ..."
        rval = cvel(vis = 'myinput.ms')
        if not rval:
            print myname, ': *** Error as expected ***'   
        else:
            print "Failed ..."
            failures += 1
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 3
if (testnumber in testlist):
    myvis = vis_b
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "Input and output vis set ..."
        rval = cvel(vis = 'myinput.ms', outputvis = 'cvel-output.ms')
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 64, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 4
if (testnumber in testlist):
    myvis = vis_a        
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "Input and output vis set, more complex input vis, one field selected ..."
        rval = cvel(vis = 'myinput.ms', outputvis = 'cvel-output.ms', field = '1')
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 64, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 5
if (testnumber in testlist):
    myvis = vis_a
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "Input and output vis set, more complex input vis, one field selected, passall = True ..."
        rval = cvel(
                vis = 'myinput.ms',
                outputvis = 'cvel-output.ms',
                field = '1',
                passall = True
                )
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 2, 64, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 6
if (testnumber in testlist):
    myvis = vis_a
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "Input and output vis set, more complex input vis, one field selected, one spw selected, passall = True ..."
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
            field = '1',
            spw = '0',
            passall = True
            )
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 2, 64, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

## # Tests with more than one spectral window ###################

testnumber = 7
if (testnumber in testlist):
    myvis = vis_c
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "Input and output vis set, input vis with two spws, one field selected, 2 spws selected, passall = False ..."
        rval = cvel(
                vis = 'myinput.ms',
                outputvis = 'cvel-output.ms',
                field = '12', # select Jupiter
                spw = '0,1',  # both available SPWs
                passall = False
                )
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 2, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 8
if (testnumber in testlist):
    myvis = vis_c
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "Input and output vis set, input vis with two spws, one field selected, 2 spws selected, passall = False, regridding 1..."
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
            field = '11',    # select some other field
            spw = '0,1',    # both available SPWs
            passall = False,    # regrid
            nchan = 1,
            width = 2
            )
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 1, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 9
if (testnumber in testlist):
    myvis = vis_c
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "Input and output vis set, input vis with two spws, one field selected, 2 spws selected, passall = False, regridding 2..."
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
            field = '10',
            spw = '0,1',
            passall = False,
            mode='channel',
            nchan = 1,
            start = 1
            )
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 1, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 10
if (testnumber in testlist):
    myvis = vis_c
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "Input and output vis set, input vis with two spws, one field selected, 2 spws selected, passall = False, regridding 3..."
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
            field = '9',
            spw = '0,1',
            passall = False,
            mode='frequency',
            nchan = 1,
            start = '4.8101GHz',
            width = '50MHz'
            )
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 1, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 11
if (testnumber in testlist):
    myvis = vis_c
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "Input and output vis set, input vis with two spws, one field selected, 2 spws selected, passall = False, regridding 4..."
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
            field = '10',
            spw = '0,1',
            passall = False,
            mode='channel',
            nchan = 1,
            start = 1,
            outframe = 'lsrk'
            )
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 1, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 12
if (testnumber in testlist):
    myvis = vis_c
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "Input and output vis set, input vis with two spws, two fields selected, 2 spws selected, passall = False, regridding 5..."
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
            field = '5,6',
            spw = '0,1',
            passall = False,
            mode='frequency',
            nchan = 2,
            start = '4.8101 GHz',
            width = '50 MHz',
            outframe = ''
            )
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 2, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 13
if (testnumber in testlist):
    myvis = vis_a
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "Input and output vis set, input vis with one spws, one field selected, one spws selected, passall = False, regridding 6..."
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
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
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 2, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 14
if (testnumber in testlist):
    myvis = vis_a
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "Input and output vis set, input vis with one spws, one field selected, one spws selected, passall = False, non-existing phase center..."
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
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
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 2, 0)
        failures += 1
    except:
        print myname, ': *** Expected error ***'   

testnumber = 15
if (testnumber in testlist):
    myvis = vis_c
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "Input and output vis set, input vis with two spws, one field selected, 2 spws selected, passall = False, regridding 8..."
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
            field = '12',
            spw = '0,1',
            passall = False,
            mode='frequency',
            nchan = 1,
            start = '4.80GHz',
            width = '50MHz',
            outframe = ''
            )
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 1, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 16
if (testnumber in testlist):
    myvis = vis_d
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "Input and output vis set, input vis with one spw, two fields selected, passall = False, regridding 9..."
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
            field = '2,3',
            spw = '0',
            passall = False,
            mode='channel',
            nchan = 10,
            start = 2,
            outframe = 'lsrd',
            phasecenter = 2
            )
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 10, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 17
if (testnumber in testlist):
    myvis = vis_d
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "Input and output vis set, input vis with one spw, two fields selected, passall = False, regridding 9..."
        rval = cvel(
                vis = 'myinput.ms',
                outputvis = 'cvel-output.ms',
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
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 10, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 18
if (testnumber in testlist):
    myvis = vis_d
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "Input and output vis set, input vis with one spw, two fields selected, passall = False, regridding 9..."
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
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
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 10, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 19
if (testnumber in testlist):
    myvis = vis_e
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "SMA input MS, 24 spws to combine, channel mode, 10 output channels"
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
            mode='channel',
            nchan = 10,
            start = 100,
            width = 2,
            phasecenter = "J2000 18h25m56.09 -12d04m28.20"
            )
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 10, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 20
if (testnumber in testlist):
    myvis = vis_e
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "SMA input MS, 24 spws to combine, channel mode, 111 output channels"
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
            mode='channel',
            nchan = 111,
            start = 201,
            width = 3,
            phasecenter = "J2000 18h25m56.09 -12d04m28.20"
            )
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 111, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 21
if (testnumber in testlist):
    myvis = vis_e
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "SMA input MS, 24 spws to combine, frequency mode, 21 output channels"
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
            mode='frequency',
            nchan = 21,
            start = '229586.0MHz',
            width = '1600kHz',
            phasecenter = "J2000 18h25m56.09 -12d04m28.20"
            )
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 21, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 22
if (testnumber in testlist):
    myvis = vis_e
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "SMA input MS, 24 spws to combine, frequency mode, 210 output channels, negative width (sign will be ignored)"
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
            mode='frequency',
            nchan = 210,
            start = '229586.0MHz',
            width = '-2400kHz',
            phasecenter = "J2000 18h25m56.09 -12d04m28.20"
            )
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 210, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 23
if (testnumber in testlist):
    myvis = vis_e
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "SMA input MS, 24 spws to combine, radio velocity mode, 30 output channels"
        print "start = ", start, ", width = ", width
        vrad = (220398.676E6 - 229586E6)/220398.676E6 * 2.99792E8
        vwidth = ((220398.676E6 - 229586E6+1600E3)/220398.676E6 * 2.99792E8) - vrad
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
            mode='velocity',
            nchan = 30,
            restfreq = '220398.676MHz',
            start = str(vrad)+'m/s',
            width = str(vwidth)+'m/s',
            phasecenter = "J2000 18h25m56.09 -12d04m28.20",
            veltype = 'radio'
            )
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 30, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 24
if (testnumber in testlist):
    myvis = vis_e
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "SMA input MS, 24 spws to combine, radio velocity mode, 35 output channels"
        vrad = (220398.676E6 - 229586E6)/220398.676E6 * 2.99792E8
        vwidth = ((220398.676E6 - 229586E6+3200E3)/220398.676E6 * 2.99792E8) - vrad
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
            mode='velocity',
            nchan = 35,
            restfreq = '220398.676MHz',
            start = str(vrad)+'m/s',
            width = str(vwidth)+'m/s',
            phasecenter = "J2000 18h25m56.09 -12d04m28.20",
            veltype = 'radio'
            )
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 35, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 25
if (testnumber in testlist):
    myvis = vis_e
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "SMA input MS, 24 spws to combine, optical velocity mode, 40 output channels"
        lambda0 = 2.99792E8/220398.676E6
        lambda1 = 2.99792E8/229586E6
        lambda2 = 2.99792E8/(229586E6+1600E3)
        vopt = (lambda1-lambda0)/lambda0 * 2.99792E8
        vwidth = vopt - (lambda2-lambda0)/lambda0 * 2.99792E8
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
            mode='velocity',
            nchan = 40,
            restfreq = '220398.676MHz',
            start = str(vopt)+'m/s',
            width = str(vwidth)+'m/s',
            phasecenter = "J2000 18h25m56.09 -12d04m28.20",
            veltype = 'optical'
            )
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 40, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 26
if (testnumber in testlist):
    myvis = vis_e
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        lambda0 = 2.99792E8/220398.676E6
        lambda1 = 2.99792E8/229586E6
        vopt = (lambda1-lambda0)/lambda0 * 2.99792E8
        lambda2 = 2.99792E8/(229586E6+1200E3)
        vwidth = vopt - (lambda2-lambda0)/lambda0 * 2.99792E8
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "SMA input MS, 24 spws to combine, optical velocity mode, 40 output channels"
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
            mode='velocity',
            nchan = 41,
            restfreq = '220398.676MHz',
            start = str(vopt)+'m/s',
            width = str(vwidth)+'m/s',
            phasecenter = "J2000 18h25m56.09 -12d04m28.20",
            veltype = 'optical'
            )
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 41, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 27
if (testnumber in testlist):
    myvis = vis_e
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    # no regrid
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "SMA input MS, 24 spws to combine, scratch columns, no regridding"
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms'
            )
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 2440, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 28
if (testnumber in testlist):
    myvis = vis_e
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "SMA input MS, 24 spws to combine, scratch columns, channel mode, 30 channels "
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
            mode="channel",
            start=1500,
            width=2,
            nchan=30
            )
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 30, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1
        
testnumber = 29
if (testnumber in testlist):
    myvis = vis_e
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "SMA input MS, 24 spws to combine, scratch columns, channel mode, 31 channels "
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
            mode="channel",
            start=1500,
            width=2,
            nchan=31
            )
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 31, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 30
if (testnumber in testlist):
    myvis = vis_e
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "SMA input MS, 24 spws to combine, scratch columns, mode channel_b, no regridding"
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
            mode="channel_b"
            )
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 2443, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 31
if (testnumber in testlist):
    myvis = vis_e
    os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('cvel')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "SMA input MS, 24 spws to combine, scratch columns, mode channel, frame trafo"
        rval = cvel(
            vis = 'myinput.ms',
            outputvis = 'cvel-output.ms',
            mode="channel",
            outframe = "BARY",
            phasecenter = "J2000 18h25m56.09 -12d04m28.20"
            )
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'cvel-output.ms'
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
        verify_ms(omsname, 1, 2440, 0)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1




# Summary ########################################
print "Tests = ", total    
print "Failures = ", failures

# empty test list if it was empty to start with
if not keeptestlist:
    testlist = []
