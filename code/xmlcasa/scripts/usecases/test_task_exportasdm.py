# unit test for the exportasdm task

myname = 'test_task_exportasdm'

import os
#vis_a = 'ngc4826.ms'
vis_b = 'test.ms'
#vis_c = 'jupiter6cm.demo.ms'
#vis_d = 'ngc4826.tutorial.ngc4826.ll.5.ms'
#vis_e = 'g19_d2usb_targets_line-shortened.ms'
vis_f = 'Itziar.ms'

#if(not os.path.exists(vis_a)):
#    importuvfits(fitsfile=os.environ['CASADATA']+'/regression/ngc4826/fitsfiles/ngc4826.ll.fits5', vis=vis_a)
if(not os.path.exists(vis_b)):
    os.system('cp -R '+os.environ['CASADATA']+'/regression/fits-import-export/input/test.ms .')
#if(not os.path.exists(vis_c)):
#    importuvfits(fitsfile=os.environ['CASADATA']+'/regression/jupiter6cm/jupiter6cm.fits', vis=vis_c)
#if(not os.path.exists(vis_d)):
#    importuvfits(fitsfile=os.environ['CASADATA']+'/regression/ngc4826/fitsfiles/ngc4826.ll.fits5', vis=vis_d)
#if(not os.path.exists(vis_e)):
#    os.system('cp -R '+os.environ['CASADATA']+'/regression/cvel/input/g19_d2usb_targets_line-shortened.ms .')
if(not os.path.exists(vis_f)):
    os.system('cp -R '+os.environ['CASADATA']+'/regression/exportasdm/input/Itziar.ms .')


def verify_asdm(asdmname):
    print "Verifying asdm ", asdmname
    

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
    os.system('rm -rf exportasdm-output.asdm myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('exportasdm')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "Testing default."
        rval = exportasdm()
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
    os.system('rm -rf exportasdm-output.asdm myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('exportasdm')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "small input MS, default output"
        rval = exportasdm(
            vis = 'myinput.ms',
            asdm = 'exportasdm-output.asdm'
            )
        print "rval is ", rval
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'exportasdm-output.asdm'
        os.system('rm -rf '+omsname+'; mv exportasdm-output.asdm '+omsname)
        verify_asdm(omsname)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 3
if (testnumber in testlist):
    myvis = vis_f
    os.system('rm -rf exportasdm-output.asdm myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('exportasdm')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "simulated input MS, default output"
        rval = exportasdm(
            vis = 'myinput.ms',
            asdm = 'exportasdm-output.asdm'
            )
        print "rval is ", rval
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'exportasdm-output.asdm'
        os.system('rm -rf '+omsname+'; mv exportasdm-output.asdm '+omsname)
        verify_asdm(omsname)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1




# Summary ########################################
print "Tests = ", total    
print "Failures = ", failures

# empty test list if it was empty to start with
if not keeptestlist:
    testlist = []
