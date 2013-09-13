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

my_project="psim2"
my_modelimage="diskmodel.im2"

# Clear out results from previous runs.
os.system('rm -rf '+my_project+'.* '+my_modelimage)
tb.clearlocks()

print casa['build']
print 'I think the data repository is at '+repodir
#importfits(fitsimage=repodir+"/data/alma/simmos/input50pc_672GHz.fits",imagename=my_modelimage)

default("simanalyze")
default("simobserve")
project=my_project
skymodel=my_modelimage
skymodel=repodir+"/data/regression/simdata/input50pc_672GHz.fits"
direction="J2000 18h00m00.031s -22d59m59.6s"

complist="star672GHz.cl"
if os.path.exists(complist):
    shutil.rmtree(complist)
cl.done()
cl.addcomponent(dir=direction,flux=0.0003,freq="672GHz")
cl.rename("star672GHz.cl")
cl.done()

setpointings=True
mapsize="0.76arcsec"
pointingspacing="0.5arcsec"
integration="10s"

obsmode="int"
antennalist=repodir+"/data/alma/simmos/alma.out20.cfg"
refdate="2012/06/21/03:25:00"
totaltime="1200s"

thermalnoise="tsys-atm"
#tau0=0.
#t_ground=0.
#t_sky=0.
user_pwv=0.5

maptype="ALMA2012"

verbose=True
if not l.has_key('interactive'): interactive=False
if interactive:
    graphics="both"
else:
    graphics="file"

go()


taskname="simanalyze"

image=True
cleanmode="clark"
cell=""
niter=1000
threshold="1e-7Jy"
imsize=[192, 192]
stokes="I"
weighting="natural"

analyze=True

go()



endTime = time.time()
endProc = time.clock()

# Regression

test_name_ppd = """simdata observation of Wolf & D'Angelo's protoplanetary disk"""

ppdso_im=ia.open(project+"/"+project + '.alma.out20.noisy.image')
ppdso_stats=ia.statistics(list=True, verbose=True)
ia.close()

#refstats = { 'flux': 0.0359,
#             'max': 6.1e-04,
#             'min': -1.6e-04, 
#             'rms': 1.9e-04,
#             'sigma': 1.4e-04 }
refstats = { 'flux': 0.0367,
             'max': 6.2e-04,
             'min': -1.75e-04, 
             'rms': 1.9e-04,
             'sigma': 1.36e-04 }


reftol   = {'flux':  0.05,
            'max':   0.1,
            'min':   0.1,
            'rms':   0.1,
            'sigma': 0.1}

import datetime
datestring = datetime.datetime.isoformat(datetime.datetime.today())
outfile    = project + "/" + project + '.' + datestring + '.log'
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
print >> logfile,casa['build']

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
    print ''
    print 'Regression PASSED'
    print ''
else:
    print >> logfile, 'FAILED',
    print ''
    print 'Regression FAILED'
    print ''

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
