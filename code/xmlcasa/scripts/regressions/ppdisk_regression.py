########################################
# Regression Script for simdata of  #
#        a protoplanetary disk         #

import os, time

# configs are in the repository
l=locals() 
if not l.has_key("repodir"): 
    repodir=os.getenv("CASAPATH").split(' ')[0]

startTime = time.time()
startProc = time.clock()

print '--Running simdata of input672GHz_50pc.image--'

my_project="psim"
my_modelimage="diskmodel.im"

# Clear out results from previous runs.
os.system('rm -rf '+my_project+'.* '+my_modelimage)
tb.clearlocks()

print 'I think the data repository is at '+repodir
importfits(fitsimage=repodir+"/data/alma/simmos/input50pc_672GHz.fits",imagename=my_modelimage)

default("simdata")
project=my_project
modelimage=my_modelimage
#complist=repodir+"star672GHz.cl"
ignorecoord=True
antennalist=repodir+"/data/alma/simmos/alma.out20.cfg"
direction="J2000 18h00m00.03s -45d59m59.6s"
pointingspacing="0.5arcsec"
mosaicsize="0.76arcsec"
refdate="2012/06/21/03:25:00"
totaltime="7200s"
integration="10s"
psfmode="clark"
niter=1000
threshold="1e-7Jy"
startfreq="668.0GHz" 
chanwidth="8.0GHz"
nchan=1
cell="0.004arcsec" 
inbright="unchanged"
imsize=[192, 192]
stokes="I"
weighting="natural"
verbose=True
if not l.has_key('interactive'): interactive=False
if interactive:
    checkinputs="yes"    
else:
    checkinputs="no"
    display=False
    fidelity=False
noise_thermal=True
user_pwv=0.5
#user_pwv=1.2
inp()
go()

endTime = time.time()
endProc = time.clock()

# Regression

test_name_ppd = """simdata observation of Wolf & D'Angelo's protoplanetary disk"""

ppdso_im=ia.open(project + '.clean.image')
ppdso_stats=ia.statistics()
ia.close()

# tsys-atm
refstats = { 'flux': 0.0365,
             'max': 4.78e-04,
             'min': -0.75e-04,
             'rms': 1.33e-04,
             'sigma': 0.98e-04 }

# 20100428
refstats = { 'flux': 0.03599,
             'max': 4.71e-04,
             'min': -0.905e-04,
             'rms': 1.34e-04,
             'sigma': 1.0e-04 }

reftol   = {'flux':  0.05,
            'max':   0.1,
            'min':   0.15,
            'rms':   0.1,
            'sigma': 0.1}

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
    adiff=abs(ppdso_stats[ke][0] - refstats[ke])/abs(refstats[ke])
    if adiff < reftol[ke]:
        print >> logfile, "* Passed %-5s test, got % -11.5g , expected % -11.5g." % (ke, ppdso_stats[ke][0], refstats[ke])
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
