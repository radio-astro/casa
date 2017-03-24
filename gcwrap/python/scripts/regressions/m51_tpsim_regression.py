import os, time, shutil

modelname = "m51ha.model"
if os.path.exists(modelname):
    shutil.rmtree(modelname)

projname = "m51sd_co32"

startTime = time.time()
startProc = time.clock()

print '--Running sim_observe of M51 (total power) --'
# configs are in the repository

l=locals() 
if not l.has_key("repodir"): 
    repodir=os.getenv("CASAPATH").split(' ')[0]

print casa['build']
print 'I think the data repository is at '+repodir
datadir=repodir+"/data/regression/simdata/"
cfgdir=repodir+"/data/alma/simmos/"
shutil.copytree(datadir+modelname,modelname)

default(simobserve)

project = projname
# Clear out results from previous runs.
os.system('rm -rf '+project)
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

#observe = True
obsmode = "sd"
# you should explicitly empty antennalist to avoid synthesis simulation
#antennalist = ''
refdate='2012/11/21/20:00:00'
totaltime = '31360s'
# totaltime = '314s'
sdantlist = cfgdir+'aca.tp.cfg'
#antennalist = cfgdir+'aca.tp.cfg'
sdant = 0

# only tsys-manual is available so far
#thermalnoise = ''   #w/o noise 
thermalnoise = 'tsys-manual'  #w/ noise
t_sky = 263.0
t_ground = t_sky

if not l.has_key('interactive'): interactive=False
if interactive:
    graphics="both"
else:
    graphics="file"

verbose=True

inp()
simobserve()

obsEndTime = time.time()
obsEndProc = time.clock()

print '--Running sim_analyze of M51 (total power) --'
# configs are in the repository

#default(sim_analyze)
default(simanalyze)
project = projname
image = True
# default vis name of SD simulation
#vis = '$project.noisy.sd.ms'  #w/ noise
imsize = [512,512]
imdirection = indirection
cell = '1.0arcsec'

analyze = True
# show psf & residual are not available for SD-only simulation
showpsf = False
showresidual = False
showconvolved = True

#
#setpointings=False
#observe=False
#thermalnoise=''
#

if not l.has_key('interactive'): interactive=False
if interactive:
    graphics="both"
else:
    graphics="file"

verbose=True

#inp(simdata)
#simdata()
inp()
#sim_analyze()
simanalyze()

endTime = time.time()
endProc = time.clock()

# Regression

test_name = """simdata observation of M51 (total power)"""

ia.open(project+"/"+project + '.sd.image')
m51sd_stats=ia.statistics(verbose=False,list=False)
ia.close()

# 2015/10/23: update after the fix to rasterutil._get_sampling (r35037)
#refstats = {'max': 5.01661158,
#             'min': -0.80671942,
#             'rms': 0.5335443,
#             'sigma': 0.48867723,
#             'sum':  56116.85862354}
#2016/06/15 after fixing B1950 component corruption
# refstats = {'max': 5.2856,
#              'min': -0.79999,
#              'rms': 0.56074,
#              'sigma': 0.51274,
#              'sum':  59485}
#2016/07/08 after fixing minweight bug in sdimaging
refstats = {'max': 5.2856,
             'min': -0.79999,
             'rms': 0.56827,
             'sigma': 0.5176,
             'sum':  59391}
ia.open(project+"/"+project + '.sd.diff')
m51sd_diffstats=ia.statistics(verbose=False,list=False)
ia.close()

# 2015/10/23: update after the fix to rasterutil._get_sampling (r35037)
#diffstats = {'max': 0.91776484,
#             'min': -0.89943612,
#             'rms': 0.20261368,
#             'sigma': 0.20169726,
#             'sum': 5044.85529928}
#2016/06/15 after fixing 1950 component corruption
# diffstats = {'max': 0.91084,
#              'min': -0.90267,
#              'rms': 0.19858,
#              'sigma': 0.19848,
#              'sum': 1676.6}
#2016/07/08 after fixing minweight bug in sdimaging
diffstats = {'max': 0.91084,
             'min': -0.90267,
             'rms': 0.19583,
             'sigma': 0.19572,
             'sum': 1640.8}
# relative tolerances to reference values
reftol   = {'sum':  1e-2,
            'max':  1e-2,
            'min':  5e-2,
            'rms':  1e-2,
            'sigma': 1e-2}

import datetime
datestring = datetime.datetime.isoformat(datetime.datetime.today())
outfile    = project+"/"+project + '.' + datestring + '.log'
logfile    = open(outfile, 'w')

print 'Writing regression output to ' + outfile + "\n"
print >> logfile,casa['build']

loghdr = """
********** Regression *****************
"""

print >> logfile, loghdr

# more info
ms.open(project+"/"+project+".aca.tp.sd.ms")
#ms.open(project+"/"+project+".aca.tp.ms")
print >> logfile, "Noiseless MS, amp stats:"
print >> logfile, ms.statistics('DATA','amp')
print >> logfile, "Noiseless MS, phase stats:"
print >> logfile, ms.statistics('DATA','phase')
ms.close()
#ms.open(project+"/"+project+".noisy.ms")
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
    print ''
    print 'Regression PASSED'
    print ''
else:
    print >> logfile, 'FAILED',
    print ''
    print 'Regression FAILED'
    print ''

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
msfstat = os.stat(project+"/"+project+'.aca.tp.sd.ms')
#msfstat = os.stat(project+"/"+project+'.aca.tp.ms')
print >>logfile,'* Breakdown:                           *'
print >>logfile,'*  generating visibilities took %8.3fs,' % (msfstat[8] - startTime)
print >>logfile,'*************************************'
    
logfile.close()
						    
print '--Finished simdata of M51 (total power) regression--'
