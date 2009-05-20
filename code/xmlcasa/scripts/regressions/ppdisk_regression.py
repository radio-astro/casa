########################################
# Regression Script for simdata of  #
#        a protoplanetary disk         #

import os, time

# Clear out results from previous runs.
os.system('rm -rf psim.* diskmodel.im')

startTime = time.time()
startProc = time.clock()

print '--Running simdata of input672GHz_50pc.image--'
# configs are in the repository
repodir=os.getenv("CASAPATH").split(' ')[0]+"/data/alma/simmos/"
importfits(fitsimage=repodir+"input50pc_672GHz.fits",imagename="diskmodel.im")
default("simdata")
project="psim"
modelimage="diskmodel.im"
#complist=repodir+"star672GHz.cl"
ignorecoord=True
checkinputs="no"
antennalist=repodir+"alma.out20.cfg"
direction="J2000 18h00m00.03s -45d59m59.6s"
pointingspacing="0.5arcsec"
refdate="2012/06/21/03:25:00"
totaltime="1200s"
integration="10s"
psfmode="clark"
niter=100
startfreq="668.0GHz" 
chanwidth="8.0GHz"
nchan=1
cell="0.004arcsec" 
#incell="0.00311arcsec" 
#inbright="6.5e-7"
inbright="7.2e-7"
imsize=[192, 192]
stokes="I"
weighting="briggs"
robust=0.0
#display=True
#verbose=True
display=False
fidelity=False
inp()
go()

endTime = time.time()
endProc = time.clock()

# Regression

test_name_ppd = """simdata observation of Wolf & D'Angelo's protoplanetary disk"""

ppdso_im=ia.open(project + '.clean.image')
ppdso_stats=ia.statistics()
ia.close()

# Jy/beam old version
#refstats = {'flux':  0.017901170000000001,
#            'max':   0.00066014000000000003,
#            'min':  -0.00012470999999999999,
#            'rms':   7.0742200500000007e-05,
#            'sigma': 7.0353806399999993e-05}

# J/arcsec;  version w/o central star, 20080521
#refstats = {'flux':  2.221e-11,
#            'max':   2.66016023e-13,
#            'min':  -5.53888e-14,
#            'rms':   5.01933e-14,
#            'sigma': 3.9611e-14}

# fixed problem with model brightness scaling 20090311
refstats = { 'flux': 0.00039567,
             'max': 3.53066844e-06,
             'min': -2.47349277e-07,
             'rms': 6.33383706e-07,
             'sigma': 4.53577279e-07 }

# changed brightness convention scaling 20090501
refstats = { 'flux': 0.00042205,
             'max': 3.6e-06,
             'min': -1.7e-07,
             'rms': 6.1e-07,
             'sigma': 3.86e-07 }

### allowing  changes of upto 10% of sigma
reftol   = {'flux':  .1,
            'max':   .1,
            'min':   .1,
            'rms':   .1,
            'sigma': .1}

import datetime
datestring = datetime.datetime.isoformat(datetime.datetime.today())
outfile    = project + '.' + datestring + '.log'
logfile    = open(outfile, 'w')

print 'Writing regression output to ' + outfile + "\n"

loghdr = """
********** Simulation Summary *********

The disk input image is a simulation done by Wolf and D'Angelo, converted from
900 GHz to 672 GHz

********** Regression *****************
"""

#A minimally bright star has been included as a clean component.


print >> logfile, loghdr

regstate = True
rskes = refstats.keys()
rskes.sort()
for ke in rskes:
    adiff=abs(ppdso_stats[ke][0] - refstats[ke])/abs(refstats['sigma'])
    if adiff < reftol[ke]:
        print >> logfile, "* Passed %-5s test, got within %8.3g of % -11.5g." % (ke, reftol[ke]*refstats[ke], refstats[ke])
    else:
        print >> logfile, "* FAILED %-5s test, got % -11.5g instead of % -11.5g." % (ke, ppdso_stats[ke][0], refstats[ke])
        regstate = False

print >> logfile,'---'
if regstate:
    print >> logfile, 'Passed',
else:
    print >> logfile, 'FAILED',
print >> logfile, 'regression test for simdata of protoplanetary disk.'
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
						    
print '--Finished simdata of input672GHz_50pc.image regression--'
