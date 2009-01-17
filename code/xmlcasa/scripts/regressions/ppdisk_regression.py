# for the love of god don't use tabs in python code!
#
########################################
# Regression Script for almasimmos of  #
#        a protoplanetary disk         #

# revisions
# date   author change
# 20071014 rr   original, based on debra's NGC4826 script
# 20080520 ri   fixed and actually committed for the first time


import os, time

# Clear out results from previous runs.
os.system('rm -rf psim.* diskmodel.im')

#pathname=os.environ.get('CASAPATH').split()[0]
#datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/ATST3/NGC4826/'

#print '--Copy data to local directory--'
#mspath='cp -r '+datapath+'n4826_both.ms .'
#os.system(mspath)
#os.system('chmod -R a+wx n4826_both.ms')

startTime = time.time()
startProc = time.clock()

print '--Running almasimmos of input672GHz_50pc.image--'
# configs are in the repository now
repodir=os.getenv("CASAPATH").split(' ')[0]+"/data/alma/simmos/"
importfits(fitsimage=repodir+"input50pc_672GHz.fits",imagename="diskmodel.im")
default("almasimmos")
project="psim"
#           modelimage=repodir+"input672GHz_50pc.image",
modelimage="diskmodel.im"
#complist=repodir+"star672GHz.cl"
antennalist=repodir+"almaconfig.out19"
direction="J2000 18h00m00.03s -22d59m59.6s"
pointingspacing="0.1arcsec"
refdate="2012/06/21/03:25:00"
totaltime="1200s"
integration="10s"
psfmode="clark"
niter=100
startfreq="668.0GHz" 
chanwidth="8.0GHz"
nchan=1
#cell="0.01637arcsec" 
# beam will be 0.025x0.035 arcsec, need smaller cell out
cell="0.004arcsec" 
incell="8.63888953e-7deg" 
inbright=0.067459
imsize=[192, 192]
stokes="I"
weighting="briggs"
robust=0.0
display=True
inp()
go()

endTime = time.time()
endProc = time.clock()

# Regression

test_name_ppd = """almasimmos observation of Wolf & D'Angelo's protoplanetary disk"""

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
refstats = {'flux':  2.221e-11,
            'max':   2.66016023e-13,
            'min':  -5.53888e-14,
            'rms':   5.01933e-14,
            'sigma': 3.9611e-14}

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

A minimally bright star has been included as a clean component.

********** Regression *****************
"""

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
print >> logfile, 'regression test for almasimmos of protoplanetary disk.'
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
						    
print '--Finished almasimmos of input672GHz_50pc.image regression--'
