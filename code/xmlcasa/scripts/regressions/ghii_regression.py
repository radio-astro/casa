############################################
# Regression Script for simdata of a 3d cube #

import os, time

# Clear out results from previous runs.
os.system('rm -rf ghii* 30dor.image')

startTime = time.time()
startProc = time.clock()

print '--Running simdata of 30 Doradus--'
# configs are in the repository
l=locals() 
if not l.has_key("repodir"): 
    repodir=os.getenv("CASAPATH").split(' ')[0]
print 'I think the data repository is at '+repodir
datadir=repodir+"/data/regression/simdata/"
cfgdir=repodir+"/data/alma/simmos/"
importfits(fitsimage=datadir+"30dor.fits",imagename="30dor.image")
default("simdata")
project="ghii"

modelimage="30dor.image"
ignorecoord=False
antennalist=cfgdir+"alma.out05.cfg"
direction="J2000 05h18m48.0s -68d42m00s"
pointingspacing="3.4arcsec"
refdate="2012/06/21/03:25:00"
totaltime="7200s"
integration="10s"
startfreq="650GHz"   
chanwidth="4GHz" 
nchan=1
cell="0.075arcsec" 
inbright=".1"
imsize=[200,200]
relmargin=1.2
stokes="I"
verbose=True
psfmode="clark"
niter=0
weighting="briggs"
robust=0.0

if not l.has_key('interactive'): interactive=False
if interactive:
    checkinputs="yes"
else:
    checkinputs="no"
    display=False
    fidelity=False

inp()
go()

endTime = time.time()
endProc = time.clock()

# Regression

test_name = """simdata observation of 30 Doradus"""

hii_im=ia.open(project + '.dirty.flat')
hii_stats=ia.statistics()
ia.close()

refstats = { 'flux': 0.9403,
             'max': 0.2845,
             'min': -0.043,
             'rms': 0.022,
             'sigma': 0.022 }
# 200909 pk flx changed- imagr change?
refstats = { 'flux': 0.96941,
             'max': 0.2841,
             'min': -0.043,
             'rms': 0.022,
             'sigma': 0.022 }

### tight 
reftol   = {'flux':  1e-2,
            'max':  1e-2,
            'min':  1e-2,
            'rms':  1e-2,
            'sigma': 1e-2}

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
regstate = True
rskes = refstats.keys()
rskes.sort()
for ke in rskes:
    adiff=abs(hii_stats[ke][0] - refstats[ke])/abs(refstats[ke])
    if adiff < reftol[ke]:
        print >> logfile, "* Passed %-5s test, got % -11.5g , expected % -11.5g." % (ke, hii_stats[ke][0], refstats[ke])
    else:
        print >> logfile, "* FAILED %-5s test, got % -11.5g instead of % -11.5g." % (ke, hii_stats[ke][0], refstats[ke])
        regstate = False
        

print >> logfile,'---'
if regstate:
    print >> logfile, 'Passed',
else:
    print >> logfile, 'FAILED',
print >> logfile, 'regression test for simdata of 30 Doradus.'
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
						    
print '--Finished simdata of 30 Doradus regression--'
