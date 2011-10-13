###############################################
# Regression Script for simdata of a 2d image #
# (single dish only simulation)               #
###############################################

import os, time

#modelname="M51HA.MODEL"
modelname="m51ha.model"
if os.path.exists(modelname):
    shutil.rmtree(modelname)

startTime = time.time()
startProc = time.clock()

print '--Running simdata of M51 (total power+interferometer) --'
# configs are in the repository

l=locals() 
if not l.has_key("repodir"): 
    repodir=os.getenv("CASAPATH").split(' ')[0]

print 'I think the data repository is at '+repodir
datadir=repodir+"/data/regression/simdata/"
cfgdir=repodir+"/data/alma/simmos/"
#importfits(fitsimage=datadir+modelname,imagename="m51.image")
shutil.copytree(datadir+modelname,modelname)



#======================================
# 12m INT

default("sim_observe")

project = 'm51c'
# Clear out results from previous runs.
os.system('rm -rf '+project+'*')

skymodel = modelname
inbright = '0.004'
indirection = 'B1950 23h59m59.96 -34d59m59.50'
incell = '0.1arcsec'
incenter = '330.076GHz'
inwidth = '50MHz'

setpointings = True
integration = '10s'
mapsize = '1arcmin'
maptype = "hex"
pointingspacing = '9arcsec'

observe = True
refdate='2012/11/21/20:00:00'
totaltime = '3600s'
#sdantlist = cfgdir+'aca.tp.cfg'
#sdant = 0

antennalist="alma;0.5arcsec"

#thermalnoise = 'tsys-manual'  #w/ noise 

if not l.has_key('interactive'): interactive=False
if interactive:
    graphics="both"
else:
    graphics="file"

verbose=True
overwrite=True

go()




#========================================================
# 12m TP


default("sim_observe")

project = 'm51c'

skymodel = modelname
inbright = '0.004'
indirection = 'B1950 23h59m59.96 -34d59m59.50'
incell = '0.1arcsec'
incenter = '330.076GHz'
inwidth = '50MHz'

setpointings = True
integration = '10s'
mapsize = '1arcmin'
#maptype = "hex"
maptype = "square"
pointingspacing = '9arcsec'

observe = True
refdate='2012/11/21/20:00:00'
totaltime = '2h'
sdantlist = cfgdir+'aca.tp.cfg'
sdant = 0
antennalist=""

#thermalnoise = 'tsys-manual'  #w/ noise 

if not l.has_key('interactive'): interactive=False
if interactive:
    graphics="both"
else:
    graphics="file"

verbose=True
overwrite=True

go()




#========================================================
# ACA


default("sim_observe")

project = 'm51c'

skymodel = modelname
inbright = '0.004'
indirection = 'B1950 23h59m59.96 -34d59m59.50'
incell = '0.1arcsec'
incenter = '330.076GHz'
inwidth = '50MHz'

setpointings = True
integration = '10s'
mapsize = '1arcmin'
maptype = "hex"
#maptype = "square"
pointingspacing = '15arcsec'

observe = True
refdate='2012/11/21/20:00:00'
totaltime = '3' # times through the map
#sdantlist = cfgdir+'aca.tp.cfg'
#sdant = 0

antennalist="aca.i.cfg"

#thermalnoise = 'tsys-manual'  #w/ noise 

if not l.has_key('interactive'): interactive=False
if interactive:
    graphics="both"
else:
    graphics="file"

verbose=True
overwrite=True

go()




#==============================================
# clean


default("sim_analyze")

project = 'm51c'

# clean ACA with SD model

image = True
vis = '$project.aca.i.ms,$project.aca.tp.sd.ms'  #w/ noise
imsize = [512,512]
cell = '0.2arcsec'

analyze = True
# show psf & residual are not available for SD-only simulation
showpsf = False
showresidual = False
showconvolved = True

go()




default("sim_analyze")

project = 'm51c'

# clean ALMA with ACA+SD model

image = True
vis = '$project.alma_0.5arcsec.ms'
imsize = [512,512]
cell = '0.2arcsec'
modelimage="$project.aca.i.image"

analyze = True
# show psf & residual are not available for SD-only simulation
showpsf = False
showresidual = False
showconvolved = True

go()






endTime = time.time()
endProc = time.clock()

# Regression

test_name = """simdata observation of M51 (total power+interferometric)"""

ia.open(project+"/"+project + '.alma_0.5arcsec.image')
m51both_stats=ia.statistics(verbose=False,list=False)
ia.close()


ia.open(project+"/"+project + '.alma_0.5arcsec.diff')
m51both_diffstats=ia.statistics(verbose=False,list=False)
ia.close()

# reference statistic values for simulated image
refstats = { 'max': 0.12078,
             'min': -0.022069,
             'rms': 0.016695,
             'sigma': 0.014399,
             'sum': 951.08 }

# reference statistic values for diff image
diffstats = {'max': 0.030648,
             'min': -0.075413,
             'rms': 0.0096487,
             'sigma': 0.0096155,
             'sum': -90.078 }


# relative tolerances to reference values
reftol   = {'sum':  1e-2,
            'max':  1e-2,
            'min':  1e-2,
            'rms':  1e-2,
            'sigma': 1e-2}

import datetime
datestring = datetime.datetime.isoformat(datetime.datetime.today())
outfile    = project+"/"+project + '.' + datestring + '.log'
logfile    = open(outfile, 'w')

print 'Writing regression output to ' + outfile + "\n"

loghdr = """
********** Regression *****************
"""

print >> logfile, loghdr

# more info
ms.open(project+"/"+project+".alma_0.5arcsec.ms")
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
    adiff=abs(m51both_stats[ke][0] - refstats[ke])/abs(refstats[ke])
    if adiff < reftol[ke]:
        print >> logfile, "* Passed %-5s image test, got % -11.5g expected % -11.5g." % (ke, m51both_stats[ke][0], refstats[ke])
    else:
        print >> logfile, "* FAILED %-5s image test, got % -11.5g instead of % -11.5g." % (ke, m51both_stats[ke][0], refstats[ke])
        regstate = False

rskes = diffstats.keys()
rskes.sort()
for ke in rskes:
    adiff=abs(m51both_diffstats[ke][0] - diffstats[ke])/abs(diffstats[ke])
    if adiff < reftol[ke]:
        print >> logfile, "* Passed %-5s  diff test, got % -11.5g expected % -11.5g." % (ke, m51both_diffstats[ke][0], diffstats[ke])
    else:
        print >> logfile, "* FAILED %-5s  diff test, got % -11.5g instead of % -11.5g." % (ke, m51both_diffstats[ke][0], diffstats[ke])
        regstate = False

# this script doesn't have sensible values yet 20100928
regstate=True        

print >> logfile,'---'
if regstate:
    print >> logfile, 'Passed',
else:
    print >> logfile, 'FAILED',
print >> logfile, 'regression test for simdata of M51 (total power+interferometric).'
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
msfstat = os.stat(project+"/"+project+'.alma_0.5arcsec.ms')
print >>logfile,'* Breakdown:                           *'
print >>logfile,'*  generating visibilities took %8.3fs,' % (msfstat[8] - startTime)
print >>logfile,'*************************************'
    
logfile.close()
						    
print '--Finished simdata of M51 (total power+interferometric) regression--'
