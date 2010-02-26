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

cl.done()
cl.addcomponent(dir="J2000 05h18m48.6s -68d42m00s",flux=0.5,freq="650GHz")
cl.rename("ghii.cl")
cl.done()
complist="ghii.cl"

ignorecoord=False
antennalist=cfgdir+"alma.out05.cfg"
direction="J2000 05h18m48.0s -68d42m00s"
#pointingspacing="3.4arcsec"
pointingspacing="4.5arcsec"
refdate="2012/06/21/03:25:00"
totaltime="7200s"
integration="10s"
scanlength=10 # not super-realistic but cuts down on execution time
mosaicsize="15arcsec"
startfreq="650GHz"   
chanwidth="4GHz" 
nchan=1
cell="0.05arcsec" 
inbright=".1"
imsize=[300,300]
stokes="I"
verbose=True
niter=5000
threshold="0.1mJy"
weighting="briggs"
robust=0.0

fidelity=True  # need this or won't create diff image

if not l.has_key('interactive'): interactive=False
if interactive:
    checkinputs="yes"    
    verbose=True
else:
    checkinputs="no"
    display=False

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
refstats = { 'sum': 240.7, #'flux': 0.21939,
             'max': 0.4655,
             'min': -0.0495,
             'rms': 0.0395,
             'sigma': 0.0391 }

ia.open(project + '.diff.im')
hiidiff_stats=ia.statistics()
ia.close()

# on ghii.diff.im
diffstats = {'sum': 66.7,
             'max': 0.00884,
             'min': -0.00391,
             'rms': 0.00199,
             'sigma': 0.00109 }

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
        print >> logfile, "* Passed %-5s image test, got % -11.5g expected % -11.5g." % (ke, hii_stats[ke][0], refstats[ke])
    else:
        print >> logfile, "* FAILED %-5s image test, got % -11.5g instead of % -11.5g." % (ke, hii_stats[ke][0], refstats[ke])
        regstate = False

rskes = diffstats.keys()
rskes.sort()
for ke in rskes:
    adiff=abs(hiidiff_stats[ke][0] - diffstats[ke])/abs(diffstats[ke])
    if adiff < reftol[ke]:
        print >> logfile, "* Passed %-5s  diff test, got % -11.5g expected % -11.5g." % (ke, hiidiff_stats[ke][0], diffstats[ke])
    else:
        print >> logfile, "* FAILED %-5s  diff test, got % -11.5g instead of % -11.5g." % (ke, hiidiff_stats[ke][0], diffstats[ke])
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
msfstat = os.stat('ghii.ms')
print >>logfile,'* Breakdown:                           *'
print >>logfile,'*  generating visibilities took %8.3fs,' % (msfstat[8] - startTime)
print >>logfile,'*  deconvolution with %d iterations took %8.3fs.' % ( niter,
                                                                       endTime - msfstat[8])
print >>logfile,'*************************************'
    
logfile.close()
						    
print '--Finished simdata of 30 Doradus regression--'
