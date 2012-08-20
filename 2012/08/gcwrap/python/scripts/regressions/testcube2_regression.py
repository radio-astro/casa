############################################
# Regression Script for simdata2 of a 3d cube #

import os, time

# Clear out results from previous runs.
#os.system('rm -rf testcube2 tc2*')

startTime = time.time()
startProc = time.clock()

print '--Running simdata of test cube--'
# configs are in the repository
l=locals() 
if not l.has_key("repodir"): 
    repodir=os.getenv("CASAPATH").split(' ')[0]

print 'I think the data repository is at '+repodir
datadir=repodir+"/data/regression/simdata/"
cfgdir=repodir+"/data/alma/simmos/"
importfits(fitsimage=datadir+"testcube.fits",imagename="testcube2")

# test
#default("simdata")
#image=False
#project="tc2_simdata"

default("simobserve")
project="tc2"

skymodel="testcube2"
inbright=".1"
indirection="J2000 19h00m00s -40d00m00s"
incell="0.2arcsec" 
incenter="350GHz"   
inwidth="0.5MHz" 

setpointings=False
ptgfile=datadir+"testcube.ptg.txt"

obsmode="int"
antennalist=cfgdir+"alma.out01.cfg"
refdate="2012/06/21/03:25:00"
totaltime="7200s"

thermalnoise=""
verbose=True
overwrite=False

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

test_name = """simdata observation of test cube"""
ms.open(project+"/"+project+".alma.out01.ms")
newdata= ms.getdata(items="data")['data']
ms.close()

refshape=[2,10,882000]

refstats = { 'max': 2.05e-01 +  7.52e-03j,
             'min':-1.90e-01 +  4.33e-02j,
             'sum': 1.72e+04 + -1.53e+03j,
             'std': 5.53e-02 }

### tight 
reftol   = {'max':  5e-3,
            'min':  5e-3,
            'sum':  5e-3,
            'std':  5e-3}

import datetime
datestring = datetime.datetime.isoformat(datetime.datetime.today())
outfile    = project+"/"+project + '.' + datestring + '.log'
logfile    = open(outfile, 'w')

print 'Writing regression output to ' + outfile + "\n"

loghdr = """
********** Regression *****************
"""



print >> logfile, loghdr

regstate = True

if max(abs(newdata.shape-pl.array(refshape)))<=0:
    print >> logfile, "* Passed shape test with shape "+str(newdata.shape)
else:
    print >> logfile, "* FAILED shape test, expecting %s, got %s" % (str(refshape),str(newdata.shape))
    regstate = False

cube_stats={'max':newdata.max(),
            'min':newdata.min(),
            'sum':newdata.sum(),
            'std':newdata.std()}

regstate=True
rskes = refstats.keys()
rskes.sort()
for ke in rskes:
    adiff=abs(cube_stats[ke] - refstats[ke])/abs(refstats[ke])
    if adiff < reftol[ke]:
        status="* Passed "
    else:
        status="* FAILED "
        regstate = False
    status=status+" %3s test, got " % ke
    if type(refstats[ke])==complex:
        status=status+"%9.2e + %9.2ej , expected %9.2e + %9.2ej." % (cube_stats[ke].real, cube_stats[ke].imag, refstats[ke].real, refstats[ke].imag)
    else:
        status=status+"%9.2e          , expected %9.2e." % (cube_stats[ke], refstats[ke])
    print >> logfile, status
    
    


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

print >> logfile, 'regression test for simdata of test cube.'
print >>logfile,'---'
print >>logfile,'*********************************'
    
print >>logfile,''
print >>logfile,'********** Benchmarking **************'
print >>logfile,''
print >>logfile,'Total wall clock time was: %8.3f s.' % (endTime - startTime)
print >>logfile,'Total CPU        time was: %8.3f s.' % (endProc - startProc)
print >>logfile,'Wall processing  rate was: %8.3f MB/s.' % (17896.0 /
                                                         (endTime - startTime))
print >>logfile,'*************************************'
    
logfile.close()
						    
print '--Finished simdata of test cube regression--'
