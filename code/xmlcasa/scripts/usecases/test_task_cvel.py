# unit test for the cvel task

import os
if(not os.path.exists('ngc4826.ms')):
    importuvfits(fitsfile=os.environ['CASADATA']+'/regression/ngc4826/fitsfiles/ngc4826.ll.fits5', vis='ngc4826.ms')
if(not os.path.exists('test.ms')):
    os.system('cp -R '+os.environ['CASADATA']+'/regression/fits-import-export/input/test.ms .')

total = 0
failures = 0
rval = false

os.system('rm -rf cveltest.ms cveltest.ms.deselected myinput.ms')
os.system('cp -R test.ms myinput.ms')
default('cvel')
try:
    total += 1
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

os.system('rm -rf cveltest.ms cveltest.ms.deselected myinput.ms')
os.system('cp -R test.ms myinput.ms')
default('cvel')
vis = 'myinput.ms'
try:
    total += 1
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

os.system('rm -rf cveltest.ms cveltest.ms.deselected myinput.ms')
os.system('cp -R test.ms myinput.ms')
default('cvel')
vis = 'myinput.ms'
outputvis = 'cveltest.ms'
try:
    total += 1
    print "Test ", total
    print "Input and output vis set ..."
    cvel()
    omsname = "test"+str(total)+vis
    os.system('rm -rf '+omsname+'; mv cveltest.ms '+omsname)
except Exception, instance:
    print '*** Unexpected exception ***',instance   
    failures += 1

os.system('rm -rf cveltest.ms cveltest.ms.deselected myinput.ms')
os.system('cp -R ngc4826.ms myinput.ms')
default('cvel')
vis = 'myinput.ms'
outputvis = 'cveltest.ms'
field = '1'
try:
    total += 1
    print "Test ", total
    print "Input and output vis set, more complex input vis, one field selected ..."
    cvel()
    omsname = "test"+str(total)+vis
    os.system('rm -rf '+omsname+'; mv cveltest.ms '+omsname)
except Exception, instance:
    print '*** Unexpected exception ***',instance   
    failures += 1

os.system('rm -rf cveltest.ms cveltest.ms.deselected myinput.ms')
os.system('cp -R ngc4826.ms myinput.ms')
default('cvel')
vis = 'myinput.ms'
outputvis = 'cveltest.ms'
field = '1'
passall = True
try:
    total += 1
    print "Test ", total
    print "Input and output vis set, more complex input vis, one field selected, passall = True ..."
    cvel()
    omsname = "test"+str(total)+vis
    os.system('rm -rf '+omsname+'; mv cveltest.ms '+omsname)
except Exception, instance:
    print '*** Unexpected exception ***',instance   
    failures += 1

os.system('rm -rf cveltest.ms cveltest.ms.deselected myinput.ms')
os.system('cp -R ngc4826.ms myinput.ms')
default('cvel')
vis = 'myinput.ms'
outputvis = 'cveltest.ms'
field = '1'
spw = '0'
passall = True
try:
    total += 1
    print "Test ", total
    print "Input and output vis set, more complex input vis, one field selected, one spw selected, passall = True ..."
    cvel()
    omsname = "test"+str(total)+vis
    os.system('rm -rf '+omsname+'; mv cveltest.ms '+omsname)
except Exception, instance:
    print '*** Unexpected exception ***',instance   
    failures += 1

print "Tests = ", total    
print "Failures = ", failures
