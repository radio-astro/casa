###############################################
# Regression Script for simdata of a 2d image #
# (single dish only simulation)               #
###############################################
 
import os, time

#modelname="M51HA.MODEL"
modelname="m51ha.model"
if os.path.exists(modelname):
    shutil.rmtree(modelname)

noise=False # add noise

projname = "m51c"

startTime = time.time()
startProc = time.clock()

print '--Running simlation of M51 (ALMA-12m INT + ACA-7m INT + 12m TP) --'
# configs are in the repository

l=locals() 
if not l.has_key("repodir"): 
    repodir=os.getenv("CASAPATH").split(' ')[0]

print casa['build']
print 'I think the data repository is at '+repodir
datadir=repodir+"/data/regression/simdata/"
cfgdir=repodir+"/data/alma/simmos/"
#importfits(fitsimage=datadir+modelname,imagename="m51.image")
shutil.copytree(datadir+modelname,modelname)



#======================================
# 12m INT

default("simobserve")

project = projname
# Clear out results from previous runs.
#os.system('rm -rf '+project+'*')
os.system('rm -rf '+project)

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

#observe = True
obsmode = "int"
refdate='2012/11/21/20:00:00'
totaltime = '3600s'
#sdantlist = cfgdir+'aca.tp.cfg'
#sdant = 0

antennalist="alma;0.5arcsec"

if noise:
    thermalnoise = 'tsys-atm'  #w/ noise 
    user_pwv=3.0
else:
    thermalnoise=""

if not l.has_key('interactive'): interactive=False
if interactive:
    graphics="both"
else:
    graphics="file"

verbose=True
overwrite=True

inp()
go()




#========================================================
# 12m TP


default("simobserve")

project = projname

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

#observe = True
obsmode = "sd"
refdate='2012/11/21/20:00:00'
totaltime = '2h'
sdantlist = cfgdir+'aca.tp.cfg'
sdant = 0
#antennalist=""

if noise:
    thermalnoise = 'tsys-atm'  #w/ noise 
    user_pwv=3.0
else:
    thermalnoise=""

if not l.has_key('interactive'): interactive=False
if interactive:
    graphics="both"
else:
    graphics="file"

verbose=True
overwrite=True

inp()
go()




#========================================================
# ACA


default("simobserve")

project = projname

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

#observe = True
obsmode = "int"
refdate='2012/11/21/20:00:00'
totaltime = '3' # times through the map
#sdantlist = cfgdir+'aca.tp.cfg'
#sdant = 0

antennalist="aca.i.cfg"

if noise:
    thermalnoise = 'tsys-atm'  #w/ noise 
    user_pwv=3.0
else:
    thermalnoise=""

if not l.has_key('interactive'): interactive=False
if interactive:
    graphics="both"
else:
    graphics="file"

verbose=True
overwrite=True

inp()
go()




#==============================================
# clean


default("simanalyze")

project = projname

# clean ACA with SD model

image = True
if noise:
    vis = '$project.aca.i.noisy.ms,$project.aca.tp.sd.noisy.ms'  #w/ noise
else:
    vis = '$project.aca.i.ms,$project.aca.tp.sd.ms'  #w/ noise
imsize = [512,512]
#imdirection = 'B1950 23h59m59.96 -34d59m59.50'
cell = '0.2arcsec'
# 201320826 new default is feather not sd as model, so specify modelimage
modelimage='$project.sd.image'

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

inp()
go()




default("simanalyze")

project = projname

# clean ALMA with ACA+SD model

image = True
if noise:
    vis = '$project.alma_0.5arcsec.noisy.ms'
else:
    vis = '$project.alma_0.5arcsec.ms'
imsize = [512,512]
#imdirection = 'B1950 23h59m59.96 -34d59m59.50'
cell = '0.2arcsec'
modelimage="$project.aca.i.image"

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

inp()
go()






endTime = time.time()
endProc = time.clock()

# Regression

test_name = """simdata observation of M51 (ALMA-12m INT + ACA-7m INT + 12m TP)"""

ia.open(project+"/"+project + '.alma_0.5arcsec.image')
m51both_stats=ia.statistics(verbose=False,list=False)
ia.close()


ia.open(project+"/"+project + '.alma_0.5arcsec.diff')
m51both_diffstats=ia.statistics(verbose=False,list=False)
ia.close()

# # reference statistic values for simulated image
# refstats = { 'max': 0.12078,
#              'min': -0.022069,
#              'rms': 0.016695,
#              'sigma': 0.014399,
#              'sum': 951.08 }

## reference statistic values for simulated image
#refstats = { 'max': 0.13314,
#             'min': -0.023702,
#             'rms': 0.020402,
#             'sigma': 0.016707,
#             'sum': 1402.5 }
# # update after setvp fix (r21970)
# refstats = { 'max': 0.19002351,
#              'min': -0.02080192,
#              'rms': 0.02843619,
#              'sigma': 0.02089434,
#              'sum': 2305.42279108 }
# # 20130719
# refstats = { 'max': 0.19413,
#              'min': -0.025759,
#              'rms': 0.02848,
#              'sigma': 0.021183,
#              'sum': 2305.2 }
# # 20130826
# refstats = { 'max': 0.1980,
#              'min': -0.02607,
#              'rms': 0.0302,
#              'sigma': 0.0228,
#              'sum': 2371. }
# # 20130911 (r26496/using SF for TP image)
# refstats = {'max': 0.13669874,
#              'min': -0.0242716,
#              'rms': 0.01825624,
#              'sigma': 0.01564628,
#              'sum':  1124.31878767}
# # 20130917: update after mask to TP image (r26597)
# refstats = {'max': 0.13555302,
#              'min': -0.02538656,
#              'rms': 0.01750355,
#              'sigma': 0.01543498,
#              'sum':  986.6099945}
# 20130917: after scaling of TP image (r27397)
refstats = {'max': 0.13657,
             'min': -0.024388,
             'rms': 0.018054,
             'sigma': 0.01559417,
             'sum':  1087.5}


# # reference statistic values for diff image
# diffstats = {'max': 0.030648,
#              'min': -0.075413,
#              'rms': 0.0096487,
#              'sigma': 0.0096155,
#              'sum': -90.078 }

## reference statistic values for diff image
#diffstats = {'max': 0.03206,
#             'min': -0.10785,
#             'rms': 0.014134,
#             'sigma': 0.013433,
#             'sum': -526.38 }
# # update after setvp fix (r21970)
# diffstats = {'max': 0.0287243,
#              'min': -0.16031383,
#              'rms': 0.02258857,
#              'sigma': 0.01913613,
#              'sum': -1434.54940508 }
# # 20130719
# diffstats = {'max': 0.036524,
#              'min': -0.16007,
#              'rms': 0.023563,
#              'sigma': 0.02028,
#              'sum': -1433.9 }
# # 20130826
# diffstats = {'max': 0.03680,
#              'min': -0.1586,
#              'rms': 0.02516,
#              'sigma': 0.02181,
#              'sum': -1500. }
# # 20130911 (r26496/using SF for TP image)
# diffstats = {'max': 0.03717272,
#              'min': -0.07126318,
#              'rms': 0.00916257,
#              'sigma': 0.00891475,
#              'sum':  -253.00065438}
# # 20130917: update after mask to TP image (r26597)
# diffstats = {'max':  0.03913748,
#              'min': -0.06746214,
#              'rms': 0.0089601,
#              'sigma': 0.00890806,
#              'sum':  -115.29187211}
# 20131120: after scaling of TP image (r27397)
diffstats = {'max':  0.03733807,
             'min': -0.06772561,
             'rms': 0.00884163,
             'sigma': 0.00865906,
             'sum':  -213.66776456}

# 20140613 
diffstats = {'max':  0.031451,
             'min': -0.064994,
             'rms': 0.0086476,
             'sigma': 0.0084565,
             'sum':  -216.12}

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
print >> logfile,casa['build']

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

print >> logfile, 'regression test for simdata of M51 (ALMA-12m INT + ACA-7m INT + 12m TP).'
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
