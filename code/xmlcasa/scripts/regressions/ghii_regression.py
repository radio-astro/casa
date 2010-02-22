############################################
# Regression Script for simdata of a 3d cube #

import os, time

# Clear out results from previous runs.
os.system('rm -rf ghii* 30dor.image')

startTime = time.time()
startProc = time.clock()

print '--Running simdata of 30 Doradus--'
# configs are in the repository
repodir=os.getenv("CASAPATH").split(' ')[0]
print 'I think the data repository is at '+repodir
datadir=repodir+"/data/regression/simdata/"
cfgdir=repodir+"/data/alma/simmos/"
importfits(fitsimage=datadir+"30dor.fits",imagename="30dor.image")
default("simdata")
project="ghii"

modelimage="30dor.image"

cl.done()
cl.addcomponent(dir="J2000 05h18m48.6s -68d42m00s",flux=0.5,freq="650GHz")
cl.rename("ghii.cl")
cl.done()
complist="ghii.cl"

ignorecoord=False
antennalist=cfgdir+"alma.out05.cfg"
direction="J2000 05h18m48.0s -68d42m00s"
pointingspacing="3.4arcsec"
refdate="2012/06/21/03:25:00"
totaltime="7200s"
integration="10s"
scanlength=10 # not super-realistic but cuts down on execution time
mosaicsize="15arcsec"
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
niter=50000
threshold="1mJy"
weighting="briggs"
robust=0.0

checkinputs="no"
display=False
fidelity=False

display=True
fidelity=True

inp()
go()

endTime = time.time()
endProc = time.clock()

# Regression

test_name = """simdata observation of 30 Doradus"""

ia.open(project + '.clean.image')
hii_stats=ia.statistics()
ia.close()

# on ghii.clean.image
refstats = { 'sum': 252.5, #'flux': 0.21939,
             'max': 0.32986,
             'min': -0.06691,
             'rms': 0.0276,
             'sigma': 0.0276 }

ia.open(project + '.diff.im')
hiidiff_stats=ia.statistics()
ia.close()

# on ghii.diff.im
diffstats = {'sum': 65.86,
             'max': 0.009,
             'min': -0.003,
             'rms': 0.002,
             'sigma': 0.0011 }

### tight 
reftol   = {'sum':  1e-2,
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
rskes = refstats.keys()
rskes.sort()
for ke in rskes:
    adiff=abs(hii_stats[ke][0] - refstats[ke])/abs(refstats[ke])
    if adiff < reftol[ke]:
        print >> logfile, "* Passed %-5s test, got % -11.5g , expected % -11.5g." % (ke, hii_stats[ke][0], refstats[ke])
    else:
        print >> logfile, "* FAILED %-5s test, got % -11.5g instead of % -11.5g." % (ke, hii_stats[ke][0], refstats[ke])
        regstate = False

rskes = diffstats.keys()
rskes.sort()
for ke in rskes:
    adiff=abs(hiidiff_stats[ke][0] - diffstats[ke])/abs(diffstats[ke])
    if adiff < reftol[ke]:
        print >> logfile, "* Passed %-5s test, got % -11.5g , expected % -11.5g." % (ke, hiidiff_stats[ke][0], diffstats[ke])
    else:
        print >> logfile, "* FAILED %-5s test, got % -11.5g instead of % -11.5g." % (ke, hiidiff_stats[ke][0], diffstats[ke])
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
