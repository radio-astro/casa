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
# 20150323: update after image parameter change in HetArrayConvFunc (r32748)
#refstats = {'max': 0.1385,
#             'min': -0.025011,
#             'rms': 0.019215,
#             'sigma': 0.016102,
#             'sum':  1257.2}
# 20160516: update after image parameter change after Sinc correction in MosaicFT 
#refstats = {'max': 0.14125,
#             'min': -0.025361,
#             'rms': 0.019264,
#             'sigma': 0.01622,
#             'sum':  1246.1}
# 20160615: after fixing component corruption in B1950 frame
# refstats = {'max': 0.14578,
#              'min': -0.02492,
#              'rms': 0.019677,
#              'sigma': 0.016366,
#              'sum':  1310.0}
# 20160708: After fixing minweight bug in sdimaging
refstats = {'max': 0.13952,
             'min': -0.025238,
             'rms': 0.018789,
             'sigma': 0.016189,
             'sum':  1143.4}
# # reference statistic values for diff image
# 20150308: update after image parameter change in simanalyze (r32603)
#diffstats = {'max':  0.03221679,
#             'min': -0.07377915,
#             'rms': 0.00990815,
#             'sigma': 0.00941917,
#             'sum':  -367.45107187}
# 20150308: update after image parameter change in HetArrayConvFunc.cc (r32748)
#diffstats = {'max':  0.03277,
#             'min': -0.073614,
#             'rms': 0.010518,
#             'sigma': 0.010015,
#             'sum':  -385.33}
# 20160516: update after sinc correction in MosaicFT
#diffstats = {'max':  0.032796,
#             'min': -0.067938,
#             'rms': 0.010429,
#             'sigma': 0.0099509,
#             'sum':  -374.23}
# 20160615: After fixing B1950 component corruption
# diffstats = {'max':  0.031154,
#              'min': -0.067642,
#              'rms': 0.010623,
#              'sigma': 0.0099747,
#              'sum':  -438.1}
# 20160708: After fixing minweight bug in sdimaging
diffstats = {'max':  0.030522,
             'min': -0.06007,
             'rms': 0.0092802,
             'sigma': 0.0089998,
             'sum':  -271.5}
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
