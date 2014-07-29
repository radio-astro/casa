import os, time

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

#refstats = {'max':  1.8829,
#            'min': -0.52625,
#            'rms': 0.22069,
#            'sigma': 0.20145,
#            'sum': 23627}
# # update after setvp fix (r21970)
# refstats = {'max': 3.17733955,
#             'min': -0.52407992,
#             'rms': 0.36143386,
#             'sigma': 0.32582116,
#             'sum': 41011.427}
# # update after change to gridfunction='SF' (r26496)
# refstats = {'max': 3.760463,
#              'min': -0.63766569,
#              'rms': 0.39420992,
#              'sigma': 0.36188505,
#              'sum': 40982.74700824} 
# # update after mask to TP image (r26597)
# refstats = {'max': 3.760463,
#              'min': -0.63766569,
#              'rms': 0.4169735,
#              'sigma': 0.37817003,
#              'sum':  40972.22263158}
# # update after fix to null pixel isuue (r27041)
# refstats = {'max': 3.760463,
#              'min': -0.63766569,
#              'rms': 0.40548718,
#              'sigma': 0.37000663,
#              'sum':  41420.88008615}
# update after scaling of TP image (r27397)
refstats = {'max': 4.30645227,
             'min': -0.73024964,
             'rms': 0.46436065,
             'sigma': 0.42372863,
             'sum':  47434.86051071}


ia.open(project+"/"+project + '.sd.diff')
m51sd_diffstats=ia.statistics(verbose=False,list=False)
ia.close()

#diffstats = {'max': 2.4528,
#             'min': -0.48053,
#             'rms': 0.26383,
#             'sigma': 0.24221,
#             'sum': 27423 }
# # update after setvp fix (r21970)
# diffstats = {'max': 1.19746065,
#              'min': -0.50039643,
#              'rms': 0.13544469,
#              'sigma': 0.12991888,
#              'sum': 10038.482 }
# # update after PB change (r26333)
# diffstats = {'max': 0.82618213,
#              'min': -0.50403631,
#              'rms': 0.10586927,
#              'sigma': 0.10465023,
#              'sum': 4199.82504407 }
# # update after change to gridfunction='SF' (r26496)
# diffstats = {'max': 0.76168156,
#              'min': -0.70423162,
#              'rms': 0.14784159,
#              'sigma': 0.14695927,
#              'sum':  4228.50428399}
# # update after mask to TP image (r26597)
# diffstats = {'max': 1.19147229,
#              'min': -0.69741619,
#              'rms': 0.1786425,
#              'sigma': 0.17307631,
#              'sum': 10320.91638317}
# # update after fix to null pixel isuue (r27041)
# diffstats = {'max': 1.19147229,
#              'min': -0.69741619,
#              'rms': 0.17808262,
#              'sigma': 0.1733555,
#              'sum': 10178.15341685}
# update after scaling of TP image (r27397)
diffstats = {'max': 0.82301152,
             'min': -0.80564827,
             'rms': 0.17661846,
             'sigma': 0.17582976,
             'sum': 4164.18677758}

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
