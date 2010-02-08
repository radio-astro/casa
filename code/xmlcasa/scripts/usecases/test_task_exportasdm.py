# unit test for the exportasdm task

myname = 'test_task_exportasdm'

import os
#vis_a = 'ngc4826.ms'
vis_b = 'test.ms'
#vis_c = 'jupiter6cm.demo.ms'
vis_d = 'ngc4826.tutorial.ngc4826.ll.5.ms'
vis_e = 'g19_d2usb_targets_line-shortened.ms'
vis_f = 'Itziar.ms'

#if(not os.path.exists(vis_a)):
#    importuvfits(fitsfile=os.environ['CASADATA']+'/regression/ngc4826/fitsfiles/ngc4826.ll.fits5', vis=vis_a)
if(not os.path.exists(vis_b)):
    os.system('cp -R '+os.environ['CASADATA']+'/regression/fits-import-export/input/test.ms .')
#if(not os.path.exists(vis_c)):
#    importuvfits(fitsfile=os.environ['CASADATA']+'/regression/jupiter6cm/jupiter6cm.fits', vis=vis_c)
if(not os.path.exists(vis_d)):
    importuvfits(fitsfile=os.environ['CASADATA']+'/regression/ngc4826/fitsfiles/ngc4826.ll.fits5', vis=vis_d)
if(not os.path.exists(vis_e)):
    os.system('cp -R '+os.environ['CASADATA']+'/regression/cvel/input/g19_d2usb_targets_line-shortened.ms .')
if(not os.path.exists(vis_f)):
    os.system('cp -R '+os.environ['CASADATA']+'/regression/exportasdm/input/Itziar.ms .')


def verify_asdm(asdmname, withPointing):
    print "Verifying asdm ", asdmname
    if(not os.path.exists(asdmname)):
        print "asdm ", asdmname, " doesn't exist."
        raise Exception
    # test for the existence of all obligatory tables
    allTables = [ "Antenna.xml",
                  "ASDM.xml",
                 # "CalData.xml",
                 # "CalDelay.xml",
                 # "CalReduction.xml",
                  "ConfigDescription.xml",
                  "CorrelatorMode.xml",
                  "DataDescription.xml",
                  "ExecBlock.xml",
                  "Feed.xml",
                  "Field.xml",
                 #"FocusModel.xml",
                 #"Focus.xml",
                  "Main.xml",
                  "PointingModel.xml",
                  "Polarization.xml",
                  "Processor.xml",
                  "Receiver.xml",
                  "SBSummary.xml",
                  "Scan.xml",
                  "Source.xml",
                  "SpectralWindow.xml",
                  "State.xml",
                  "Station.xml",
                  "Subscan.xml",
                  "SwitchCycle.xml"
                  ]
    isOK = True
    for fileName in allTables:
        filePath = asdmname+'/'+fileName
        if(not os.path.exists(filePath)):
            print "ASDM table file ", filePath, " doesn't exist."
            isOK = False
        else:
            # test if well formed
            rval = os.system('xmllint --noout '+filePath)
            if(rval !=0):
                print "Table ", filePath, " is not a well formed XML document."
                isOK = False

    print "Note: xml validation not possible since ASDM DTDs (schemas) not yet online."
        
    if(not os.path.exists(asdmname+"/ASDMBinary")):
        print "ASDM binary directory "+asdmname+"/ASDMBinary doesn't exist."
        isOK = False

    if(withPointing and not os.path.exists(asdmname+"/Pointing.bin")):
        print "ASDM binary file "+asdmname+"/Pointing.bin doesn't exist."
        isOK = False

    if (not isOK):
        raise Exception

# Test cases    

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
            asdm = 'exportasdm-output.asdm',
            archiveid="S1",
            verbose=True
            )
        print "rval is ", rval
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'exportasdm-output.asdm'
        os.system('rm -rf '+omsname+'; mv exportasdm-output.asdm '+omsname)
        verify_asdm(omsname, False)
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
            asdm = 'exportasdm-output.asdm',
            archiveid="S1"
            )
        print "rval is ", rval
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'exportasdm-output.asdm'
        os.system('rm -rf '+omsname+'; mv exportasdm-output.asdm '+omsname)
        verify_asdm(omsname, True)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 4
if (testnumber in testlist):
    myvis = vis_d
    os.system('rm -rf exportasdm-output.asdm myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('exportasdm')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "real input MS, default output"
        rval = exportasdm(
            vis = 'myinput.ms',
            asdm = 'exportasdm-output.asdm',
            archiveid="S1"
            )
        print "rval is ", rval
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'exportasdm-output.asdm'
        os.system('rm -rf '+omsname+'; mv exportasdm-output.asdm '+omsname)
        verify_asdm(omsname, False)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1

testnumber = 5
if (testnumber in testlist):
    myvis = vis_e
    os.system('rm -rf exportasdm-output.asdm myinput.ms')
    os.system('cp -R ' + myvis + ' myinput.ms')
    default('exportasdm')
    total += 1
    try:
        print "\n>>>> Test ", testnumber, ", input MS: ", myvis
        print "real input MS, default output"
        rval = exportasdm(
            vis = 'myinput.ms',
            asdm = 'exportasdm-output.asdm',
            archiveid="S1"
            )
        print "rval is ", rval
        if not rval:
            raise Exception
        omsname = "test"+str(testnumber)+'exportasdm-output.asdm'
        os.system('rm -rf '+omsname+'; mv exportasdm-output.asdm '+omsname)
        verify_asdm(omsname, False)
    except:
        print myname, ': *** Unexpected error ***'   
        failures += 1



# Summary ########################################
print "Tests = ", total    
print "Failures = ", failures

# empty test list if it was empty to start with
if not keeptestlist:
    testlist = []
