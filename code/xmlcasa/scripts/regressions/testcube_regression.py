############################################
# Regression Script for simdata of a 3d cube #

import os, time

# Clear out results from previous runs.
os.system('rm -rf testcube tc*')

startTime = time.time()
startProc = time.clock()

print '--Running simdata of test cube--'
# configs are in the repository
repodir=os.getenv("CASAPATH").split(' ')[0]
print 'I think the data repository is at '+repodir
datadir=repodir+"/data/regression/simdata/"
cfgdir=repodir+"/data/alma/simmos/"
importfits(fitsimage=datadir+"testcube.fits",imagename="testcube")
default("simdata")
project="tc"

modelimage="testcube"
ignorecoord=True
antennalist=cfgdir+"alma.out01.cfg"
direction="J2000 19h00m00s -40d00m00s"
pointingspacing="1arcmin"
refdate="2012/06/21/03:25:00"
totaltime="7200s"
integration="10s"
startfreq="350GHz"   
chanwidth="0.5MHz" 
nchan=16
cell="0.2arcsec" 
inbright=".1"
imsize=[192,192]
stokes="I"
verbose=True
#thermalnoise=True
#tau0=1.0
checkinputs="no"
display=False

# interactive:
#checkinputs="yes"
#display=True



fidelity=False
psfmode="none"
niter=0
inp()
go()

endTime = time.time()
endProc = time.clock()

# Regression

test_name = """simdata observation of test cube"""
ms.open(project+".ms")
newdata= ms.getdata(items="data")['data']


refshape=[2,10,882000]

refstats = { 'max': 0.0119+0.00229j,
             'min':-0.0140+0.00105j,
             'sum': 552.9-34.3j,
             'std': 0.00132 }
# 200909 more digits
refstats = { 'max': 0.01185+0.002285j,
             'min':-0.0140+0.001082j,
             'sum': 553.4-34.36j,
             'std': 0.001324 }

# 200101119 skyequation/clean changes
refstats = { 'max': 0.0117+0.00249j,
             'min':-0.0140+0.000962j,
             'sum': 481.-65.0j,
             'std': 0.00133 }

### tight 
reftol   = {'max':  5e-3,
            'min':  5e-3,
            'sum':  5e-3,
            'std':  5e-3}

import datetime
datestring = datetime.datetime.isoformat(datetime.datetime.today())
outfile    = project + '.' + datestring + '.log'
logfile    = open(outfile, 'w')

print 'Writing regression output to ' + outfile + "\n"

loghdr = """
********** Regression *****************
"""



print >> logfile, loghdr

regstate = True

if max(abs(newdata.shape-pl.array(refshape)))<=0:
    print >> logfile, "* Passed shape test with shape "+str(newdata.shape)
else:
    print >> logfile, "* FAILED shape test, expecting %s, got %s" % (str(refshape),str(newdata.shape))
    regstate = False

cube_stats={'max':newdata.max(),
            'min':newdata.min(),
            'sum':newdata.sum(),
            'std':newdata.std()}

regstate=True
rskes = refstats.keys()
rskes.sort()
for ke in rskes:
    adiff=abs(cube_stats[ke] - refstats[ke])/abs(refstats[ke])
    if adiff < reftol[ke]:
        status="* Passed "
    else:
        status="* FAILED "
        regstate = False
    status=status+" %3s test, got " % ke
    if type(refstats[ke])==complex:
        status=status+"%9.2e + %9.2ej , expected %9.2e + %9.2ej." % (cube_stats[ke].real, cube_stats[ke].imag, refstats[ke].real, refstats[ke].imag)
    else:
        status=status+"%9.2e          , expected %9.2e." % (cube_stats[ke], refstats[ke])
    print >> logfile, status
    
    


print >> logfile,'---'
if regstate:
    print >> logfile, 'Passed',
else:
    print >> logfile, 'FAILED',
print >> logfile, 'regression test for simdata of test cube.'
print >>logfile,'---'
print >>logfile,'*********************************'
    
print >>logfile,''
print >>logfile,'********** Benchmarking **************'
print >>logfile,''
print >>logfile,'Total wall clock time was: %8.3f s.' % (endTime - startTime)
print >>logfile,'Total CPU        time was: %8.3f s.' % (endProc - startProc)
print >>logfile,'Wall processing  rate was: %8.3f MB/s.' % (17896.0 /
                                                         (endTime - startTime))

### Get last modification time of .ms.
## msfstat = os.stat('almasimmos_regression.ms')
## print >>logfile,'* Breakdown:                           *'
## print >>logfile,'*  generating visibilities took %8.3fs,' % (msfstat[8] - startTime)
## print >>logfile,'*  %s deconvolution with %d iterations took %8.3fs.' % (alg,
##                                                                         niter,
##                                                                         endTime - msfstat[8])
print >>logfile,'*************************************'
    
logfile.close()
						    
print '--Finished simdata of test cube regression--'
