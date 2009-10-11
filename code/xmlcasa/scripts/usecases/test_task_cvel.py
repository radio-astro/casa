# unit test for the cvel task

import os
if(not os.path.exists('ngc4826.ms')):
    importuvfits(fitsfile=os.environ['CASADATA']+'/regression/ngc4826/fitsfiles/ngc4826.ll.fits5', vis='ngc4826.ms')
if(not os.path.exists('test.ms')):
    os.system('cp -R '+os.environ['CASADATA']+'/regression/fits-import-export/input/test.ms .')
if(not os.path.exists('jupiter6cm.demo.ms')):
    importuvfits(fitsfile=os.environ['CASADATA']+'/regression/jupiter6cm/jupiter6cm.fits', vis='jupiter6cm.demo.ms')

try:
    print "Skipping until test ", skipuntil
except:
    print "Global variable skipuntil not set. Executing all tests."
    skipuntil = 0

total = 0
failures = 0
rval = False

os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
os.system('cp -R test.ms myinput.ms')
default('cvel')
total += 1
if (total>=skipuntil):
    try:
        print "Test ", total
        print "Testing default."
        rval = cvel()
        if(not rval):
            print '*** Error as expected ***'   
        else:
            print "Failed ..."
            failures +=1
    except Exception, instance:
        print '*** Unexpected exception ***',instance   
        failures += 1

os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
os.system('cp -R test.ms myinput.ms')
default('cvel')
vis = 'myinput.ms'
total += 1
if (total>=skipuntil):
    try:
        print "Test ", total
        print "Only input vis set ..."
        rval = cvel()
        if(not rval):
            print '*** Error as expected ***'   
        else:
            print "Failed ..."
            failures += 1
    except Exception, instance:
        print '*** Unexpected exception ***',instance   
        failures += 1

os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
os.system('cp -R test.ms myinput.ms')
default('cvel')
vis = 'myinput.ms'
outputvis = 'cvel-output.ms'
total += 1
if (total>=skipuntil):
    try:
        print "Test ", total
        print "Input and output vis set ..."
        cvel()
        omsname = "test"+str(total)+outputvis
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
    except Exception, instance:
        print '*** Unexpected exception ***',instance   
        failures += 1
        
os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
os.system('cp -R ngc4826.ms myinput.ms')
default('cvel')
vis = 'myinput.ms'
outputvis = 'cvel-output.ms'
field = '1'
total += 1
if (total>=skipuntil):
    try:
        print "Test ", total
        print "Input and output vis set, more complex input vis, one field selected ..."
        cvel()
        omsname = "test"+str(total)+outputvis
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
    except Exception, instance:
        print '*** Unexpected exception ***',instance   
        failures += 1

os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
os.system('cp -R ngc4826.ms myinput.ms')
default('cvel')
vis = 'myinput.ms'
outputvis = 'cvel-output.ms'
field = '1'
passall = True
total += 1
if (total>=skipuntil):
    try:
        print "Test ", total
        print "Input and output vis set, more complex input vis, one field selected, passall = True ..."
        cvel()
        omsname = "test"+str(total)+outputvis
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
    except Exception, instance:
        print '*** Unexpected exception ***',instance   
        failures += 1

os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
os.system('cp -R ngc4826.ms myinput.ms')
default('cvel')
vis = 'myinput.ms'
outputvis = 'cvel-output.ms'
field = '1'
spw = '0'
passall = True
total += 1
if (total>=skipuntil):
    try:
        print "Test ", total
        print "Input and output vis set, more complex input vis, one field selected, one spw selected, passall = True ..."
        cvel()
        omsname = "test"+str(total)+outputvis
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
    except Exception, instance:
        print '*** Unexpected exception ***',instance   
        failures += 1

## # Tests with more than one spectral window ###################

os.system('rm -rf cvel-output.ms cvel-output.ms.deselected myinput.ms')
os.system('cp -R jupiter6cm.demo.ms myinput.ms')
default('cvel')
vis = 'myinput.ms'
outputvis = 'cvel-output.ms'
# select Jupiter
field = '12'
# both available SPWs
spw = '0,1'
passall = False
total += 1
if (total>=skipuntil):
    try:
        print "Test ", total
        print "Input and output vis set, input vis with two spws, one field selected, 2 spws selected, passall = False ..."
        cvel()
        omsname = "test"+str(total)+outputvis
        os.system('rm -rf '+omsname+'; mv cvel-output.ms '+omsname)
    except Exception, instance:
        print '*** Unexpected exception ***',instance   
        failures += 1


# Summary ########################################
print "Tests = ", total    
print "Failures = ", failures
