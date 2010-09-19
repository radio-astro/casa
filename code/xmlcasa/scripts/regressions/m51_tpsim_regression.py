###############################################
# Regression Script for simdata of a 2d image #
# (single dish only simulation)               #
###############################################

import os, time

#os.system('rm -rf m51.image')

#modelname="M51HA.MODEL"
modelname="m51ha.model"
os.system('rm -rf '+modelname)

startTime = time.time()
startProc = time.clock()

print '--Running simdata of M51 (total power) --'
# configs are in the repository

l=locals() 
if not l.has_key("repodir"): 
    repodir=os.getenv("CASAPATH").split(' ')[0]

print 'I think the data repository is at '+repodir
datadir=repodir+"/data/regression/simdata/"
cfgdir=repodir+"/data/alma/simmos/"
#importfits(fitsimage=datadir+modelname,imagename="m51.image")
shutil.copytree(datadir+modelname,modelname)
default("simdata")

project = 'm51sd_co32'
# Clear out results from previous runs.
os.system('rm -rf '+project+'*')

modifymodel=True
#skymodel = 'm51.image'
skymodel = modelname
inbright = '0.004'
indirection = 'B1950 23h59m59.96 -34d59m59.50'
incell = '0.5arcsec'
incenter = '330.076GHz'
inwidth = '50MHz'

setpointings = True
integration = '10s'
mapsize = ''
maptype = 'square'
pointingspacing = '9arcsec'

predict = True
# you should explicitly empty antennalist to avoid synthesis simulation
antennalist = ''
refdate='2012/11/21/20:00:00'
totaltime = '31360s'
sdantlist = cfgdir+'aca.tp.cfg'
sdant = 0

# only tsys-manual is available so far
#thermalnoise = ''   #w/o noise 
thermalnoise = 'tsys-manual'  #w/ noise 

image = True
# default vis name of SD simulation
#vis = '$project.sd.ms'  #w/o noise
vis = '$project.noisy.sd.ms'  #w/ noise
imsize = [512,512]
cell = '1.0arcsec'

analyze = True
# show psf & residual are not available for SD-only simulation
showpsf = False
showresidual = False
showconvolved = True

if not l.has_key('interactive'): interactive=False
if interactive:
    graphics="both"
else:
    graphics="file"

verbose=True

inp()
simdata()

endTime = time.time()
endProc = time.clock()


# Regression

test_name = """simdata observation of M51 (total power)"""

ia.open(project + '.image')
m51sd_stats=ia.statistics(verbose=False,list=False)
ia.close()

# reference statistic values for simulated image
#rev.12787
refstats = { 'sum': 15834.,
             'max': 1.6246,
             'min': -0.52885,
             'rms': 0.18651,
             'sigma': 0.17646 }

ia.open(project + '.diff')
m51sd_diffstats=ia.statistics(verbose=False,list=False)
ia.close()

# reference statistic values for diff image
#rev.12787
diffstats = {'sum': 3.5216e4,
             'max': 2.7180,
             'min': -0.49571,
             'rms': 0.30398,
             'sigma': 0.27268 }


# relative tolerances to reference values
reftol   = {'sum':  1e-2,
            'max':  1e-2,
            'min':  5e-2,
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

# more info
ms.open(project+".sd.ms")
print >> logfile, "Noiseless MS, amp stats:"
print >> logfile, ms.statistics('DATA','amp')
print >> logfile, "Noiseless MS, phase stats:"
print >> logfile, ms.statistics('DATA','phase')
ms.close()
#ms.open(project+".noisy.ms")
#print >> logfile, "Noisy MS, amp stats:"
#print >> logfile, ms.statistics('DATA','amp')
#print >> logfile, "Noisy MS, phase stats:"
#print >> logfile, ms.statistics('DATA','phase')
#ms.close()


regstate = True
rskes = refstats.keys()
rskes.sort()
for ke in rskes:
    adiff=abs(m51sd_stats[ke][0] - refstats[ke])/abs(refstats[ke])
    if adiff < reftol[ke]:
        print >> logfile, "* Passed %-5s image test, got % -11.5g expected % -11.5g." % (ke, m51sd_stats[ke][0], refstats[ke])
    else:
        print >> logfile, "* FAILED %-5s image test, got % -11.5g instead of % -11.5g." % (ke, m51sd_stats[ke][0], refstats[ke])
        regstate = False

rskes = diffstats.keys()
rskes.sort()
for ke in rskes:
    adiff=abs(m51sd_diffstats[ke][0] - diffstats[ke])/abs(diffstats[ke])
    if adiff < reftol[ke]:
        print >> logfile, "* Passed %-5s  diff test, got % -11.5g expected % -11.5g." % (ke, m51sd_diffstats[ke][0], diffstats[ke])
    else:
        print >> logfile, "* FAILED %-5s  diff test, got % -11.5g instead of % -11.5g." % (ke, m51sd_diffstats[ke][0], diffstats[ke])
        regstate = False
        

print >> logfile,'---'
if regstate:
    print >> logfile, 'Passed',
else:
    print >> logfile, 'FAILED',
print >> logfile, 'regression test for simdata of M51 (total power).'
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
msfstat = os.stat(project+'.sd.ms')
print >>logfile,'* Breakdown:                           *'
print >>logfile,'*  generating visibilities took %8.3fs,' % (msfstat[8] - startTime)
print >>logfile,'*************************************'
    
logfile.close()
						    
print '--Finished simdata of M51 (total power) regression--'
