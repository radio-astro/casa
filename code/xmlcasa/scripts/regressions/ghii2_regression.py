############################################
# Regression Script for simdata of a 3d cube #
# v2 for simdata2

import os, time

os.system('rm -rf 30dor.image')

startTime = time.time()
startProc = time.clock()

print '--Running simdata of 30 Doradus--'
# configs are in the repository
l=locals() 
if not l.has_key("repodir"): 
    repodir=os.getenv("CASAPATH").split(' ')[0]

print casa['build']
print 'I think the data repository is at '+repodir
datadir=repodir+"/data/regression/simdata/"
cfgdir=repodir+"/data/alma/simmos/"
importfits(fitsimage=datadir+"30dor.fits",imagename="30dor.image")

project="ghii2"
# Clear out results from previous runs.
os.system('rm -rf '+project+'*')

#importfits(fitsimage=datadir+"ghii2_regression.mask.fits",imagename="ghii2_regression.mask")
shutil.copytree(datadir+"ghii2_regression.mask","ghii2_regression.mask")
default("simdata")
project="ghii2"


cl.done()
cl.addcomponent(dir="J2000 05h18m48.586s -68d42m00.05s",flux=0.5,freq="650GHz")
cl.rename(project+".cl")
cl.done()

modifymodel=True # but only brightness
skymodel="30dor.image"
inbright=".1"
incell=""
incenter="652GHz"   
inwidth="4GHz" 
indirection=""

setpointings=True
integration="10s"
totaltime="7200s"
direction="J2000 05h18m48.0s -68d42m00s"
mapsize="17arcsec"  # input is 16.3, relmargin=0.5
pointingspacing="4.5arcsec"

predict=True
complist=project+".cl"
refdate="2012/06/21/03:25:00"
antennalist=cfgdir+"alma.out05.cfg"

thermalnoise="tsys-atm" # simdata2 default=off

image=True
vis="$project.noisy.ms"
cell="0.05arcsec"
niter=5000
threshold="1mJy"
weighting="briggs"
#imsize=[300,300]
imsize=[400,400]
#mask=[50,350,50,350]
#mask="ghii2_regression.mask.text"
mask="ghii2_regression.mask"

analyze=True
overwrite=True
verbose=True

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

test_name = """simdata observation of 30 Doradus"""

ia.open(project + '.image')
hii_stats=ia.statistics(verbose=False,list=False)
ia.close()


ia.open(project + '.diff')
hiidiff_stats=ia.statistics(verbose=False,list=False)
ia.close()

refstats = { 'sum': 1052., 
             'max': 0.676,
             'min': -0.0513,
             'rms': 0.0434,
             'sigma': 0.0429 }

diffstats = {'sum': 3322.,
             'max': 0.169,
             'min': -0.00459,
             'rms': 0.0379,
             'sigma': 0.0222 }


reftol   = {'sum':  0.1,
            'max':  0.04,
            'min':  0.04,
            'rms':  0.1,
            'sigma': 0.1}

import datetime
datestring = datetime.datetime.isoformat(datetime.datetime.today())
outfile    = project + '.' + datestring + '.log'
logfile    = open(outfile, 'w')

print 'Writing regression output to ' + outfile + "\n"

loghdr = """
********** Regression *****************
"""

print >> logfile, loghdr
print >> logfile,casa['build']

# more info
ms.open(project+".ms")
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
    adiff=abs(hii_stats[ke][0] - refstats[ke])/abs(refstats[ke])
    if adiff < reftol[ke]:
        print >> logfile, "* Passed %-5s image test, got % -11.5g expected % -11.5g." % (ke, hii_stats[ke][0], refstats[ke])
    else:
        print >> logfile, "* FAILED %-5s image test, got % -11.5g instead of % -11.5g." % (ke, hii_stats[ke][0], refstats[ke])
        regstate = False

rskes = diffstats.keys()
rskes.sort()
for ke in rskes:
    adiff=abs(hiidiff_stats[ke][0] - diffstats[ke])/abs(diffstats[ke])
    if adiff < reftol[ke]:
        print >> logfile, "* Passed %-5s  diff test, got % -11.5g expected % -11.5g." % (ke, hiidiff_stats[ke][0], diffstats[ke])
    else:
        print >> logfile, "* FAILED %-5s  diff test, got % -11.5g instead of % -11.5g." % (ke, hiidiff_stats[ke][0], diffstats[ke])
        regstate = False
        

print >> logfile,'---'
if regstate:
    print >> logfile, 'Passed',
else:
    print >> logfile, 'FAILED',
print >> logfile, 'regression test for simdata of 30 Doradus.'
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
msfstat = os.stat(project+'.ms')
print >>logfile,'* Breakdown:                           *'
print >>logfile,'*  generating visibilities took %8.3fs,' % (msfstat[8] - startTime)
print >>logfile,'*  deconvolution with %d iterations took %8.3fs.' % ( niter,
                                                                       endTime - msfstat[8])
print >>logfile,'*************************************'
    
logfile.close()
						    
print '--Finished simdata of 30 Doradus regression--'
