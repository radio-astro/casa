################################################################################
# 
# Regression test for Moon raster scan imaging 
# using sdtpimaging task 
# 
# global parameter:
# benchmarking (if not defined, set to True, to report benchmark results)
# initial,  if True, it save the stats results as the referece data
# then the data is copied to datapath.
#
# some preset parameters:
# testdata: test data name
# ref_data: the file name of the reference output results of this regression 
#           script to compare with (assumed to store at 'datapath' if not 
#           available create it and copied to 'datapath')
# datapath: location of the input data (MS and reference data for
#           comparison of the results) 
#           default datapath is currently set to casapath+'/data/alma/atf/sd/'
# testdir: directory of the regression test to be performed 
#
# output: (in testdir)
# regression log, images in CASA image format, 
# summary.html(images and image statistics summary in HTML)
#
# last modified 2008-10-08 T. Tsutsumi
########################################################################
import os
import sys
import time
import regression_utility as regutl
import cPickle
from numpy import *
asap_init()

scriptname='sdtpimaging'
# benchmarking?
# default is True
try:
    benchmarking
except NameError:
    benchmarking=None

if benchmarking is None: 
    benchmarking = True

# set initial = True to create reference data for comparison
#initial = True
initial = False 

#log file name
logfilename='sdtpimaging_regression.log'
#test directory
testdir='sdtpimaging_test'
#
testdata='uid___X1e1_X3197_X1.ms'
ref_data='sdtpimaging_refdata.txt'
dataname=testdata.rstrip('.ms')
antids=['0','1']
tol=0.05
description='Description\nProcess ATF raster scan data, %s \n step1: run sdtpimaging to do baseline subtraction only\n separate run for each antenna.\n step2: run sdtpimaging to do imaging only for each antenna\n' % testdata
##############################################
def compare(thisStatslist, logfile=None, imagenames=[], tol=0.05):
    refstats=[]
    statslist=['rms', 'max']
    diff_max=[]
    nodiff_maxpos=[]
    diff_rms=[] 
    failcnt=0
    
    if logfile is None:
        logfile = sys.stdout
           
    f = open(ref_data)
    refstats.append(cPickle.load(f))
    refstats.append(cPickle.load(f))
    f.close()
     
    for i in range(len(thisStatslist)):
        diff_max.append(refstats[i]['max']-thisStatslist[i]['max'])
        nodiff_maxpos.append(all(refstats[i]['maxpos']==thisStatslist[i]['maxpos']))
        diff_rms.append(refstats[i]['rms']-thisStatslist[i]['rms'])
 
        if (diff_max[i]<tol and nodiff_maxpos[i]):
            passfailstr = 'PASS'
        else:
            passfailstr = 'FAIL'
            failcnt+=1
        print >>logfile, 'image max test for antid=%d image %s' % (i, passfailstr)
        print >>logfile, '--- image max %s at %s' % (str(thisStatslist[i]['max']), str(thisStatslist[i]['maxpos'])) 
        if (diff_rms[i]<tol):
            passfailstr = 'PASS'
        else:
            passfailstr = 'FAIL'
            failcnt+=1
        print >>logfile, 'image rms test for antid=%d image %s' % (i, passfailstr)           
        print >>logfile, '--- image rms %s' % str(thisStatslist[i]['rms'])


    # difference, fidelity like measure..
    #if len(imagenames) > 0:
    #   default(immath)
    #   outfile='diffim'
    #   mode='evalexpr'
    #   expr='"%s"-"%s"' % (refim[0],imagenames[0])    
    #   immath()  
    #   imagename=outfile
    #   stats=imstats()
    #   if stats['rms'][0]!=0.0:
    #       outfile='fid'
    #       expr='abs(refim1/diffim)'
    #       immath()

    if failcnt:
        print >>logfile, '***SDTPIMAGING regression test FAILED***'
        return True
    else:
        print >>logfile, '***SDTPIMAGING regression test PASSED***'
        return False 
################################################################################ 
try: 
    imagenames=[]
    for i in antids:
        imagenames.append(dataname+'Ant'+i+'.im')

    casapath=os.environ['CASAPATH'].split()[0]
    #host=os.environ['CASAPATH'].split()[3]
    import socket
    host=socket.gethostname()
    if host == "minor":
        datapath='/export/home/minor/alma/casatest/ATF/rastermaps/'
        print "host is %s, datapath is %s" % (host, datapath)
    else:
        datapath=casapath+'/data/regression/alma-sd/'
    #datapath=datapath+testdata

    # setup test directory
    curdir=os.getcwd()
    regutl.maketestdir(testdir)
    os.system("cp -r "+datapath+testdata+" "+testdir+"/.")
    if os.path.isfile(datapath+ref_data):
	os.system("cp -r "+datapath+ref_data+" "+testdir+"/.")
    else:
        initial = True
    os.chdir(testdir)
   

    # start regression 
    print "sdtpimaging regression start"
    startTime = time.time();
    startProc = time.clock();

    # Do baseline subtraction first
    default(sdtpimaging)
    sdfile=testdata
    calmode='baseline'
    stokes='XX'
    createimage=False
    masklist=[50,50]
    bpoly=1
    for i in antids:
       antenna=i
       sdtpimaging()
    baseline2time=time.time()

    #imaging
    #skip baseline subtraction
    default(sdtpimaging)
    sdfile=testdata
    calmode='none' 
    createimage=True
    imsize=[200,200]
    cell=['0.2arcmin','0.2arcmin']
    phasecenter="AZEL 187d54m22s 41d03m0s"
    ephemsrcname='Moon'
    pointingcolumn='direction'
    # didnot work well if gridfunction='BOX' or 'PB'
    # is chosen.
    gridfunction='SF'
    for i in antids:
        print "create an image for", i
	imagename=imagenames[int(i)]
        antenna=i
	sdtpimaging()

    endTime = time.time();
    endProc = time.clock();
    # main process end here

    #analyze output image
    logfile=open(logfilename, 'w')
    casaver=casalog.version()
    import datetime
    datestring=datetime.datetime.isoformat(datetime.datetime.today())
    print >>logfile, '%s running %s on %s (casaroot at %s)' % (scriptname, casaver, host, casapath)
    print >>logfile, 'at ', datestring
    print >>logfile, ' ' 
    print >>logfile, description
    print >>logfile, ' '

    thisStats=[]
    for i in antids:
        imagename=imagenames[int(i)]
        stats=imstat()
        #stats=ia.statistics()
        thisStats.append(stats)
 
    # summary page 
    tb.open(testdata+'/ANTENNA')
    antnames=tb.getcol('NAME')
    tb.close()
    htmlfile=open('summary.html','w')
    htmlhead='<html><head><title>sdtpimaging summary</title></head>\n'
    htmlbody='<body><h2>sdtpimaging regression summary</h2>'
    htmlbody+='<body><h3>data set:%s (ATF total power raster scans of Moon)</h3>' % testdata
    htmlbody+='<body><h4>regression run at %s</h4>' % datestring
    for i in range(len(imagenames)):
        ia.open(imagenames[i])
        data=ia.getchunk([-1,-1,1,1],[-1,-1,1,1],1,-1,True,True,False)
        ia.close()
        pngimg=imagenames[i]+'.png'
        tdata=data.transpose()
        tdatalist=tdata.tolist()
        tdatalist.reverse()
        pl.ioff()
        pl.imshow(tdatalist, interpolation='bilinear',cmap=pl.cm.hot,extent=(0,200,0,200)) 
        pl.savefig(pngimg)
        htmlbody+='<img src=%s />\n' % pngimg
        htmlbody+='<table><tr><th colspan=2>%s (antenna:%s)</th><tr>' % (imagenames[i], antnames[i])
        htmlbody+='<td><tr><td>max</td><td>%s</td><tr>' % thisStats[i]['max'][0] 
        htmlbody+='<tr><td>maxpos</td><td>%s</td><tr>' % thisStats[i]['maxpos'].tolist()
        htmlbody+='<tr><td>min</td><td>%s</td><tr>' % thisStats[i]['min'][0]
        htmlbody+='<tr><td>minpos</td><td>%s</td><tr>' % thisStats[i]['minpos'].tolist()
        htmlbody+='<tr><td>rms</td><td>%s</td><tr>' % thisStats[i]['rms'][0]
        htmlbody+='</table><hr>\n' 
    htmlclose = '</body></html>'  
    htmlfile.writelines(htmlhead)
    htmlfile.writelines(htmlbody)
    htmlfile.writelines(htmlclose)
    htmlfile.close() 
   
    if initial:
	file = open('sdtpimaging_refdata.txt','w') 
	for i in antids:
	    cPickle.dump(thisStats[int(i)],file)    
	file.close() 
        os.system('cp sdtpimaging_refdata.txt '+datapath)
    else:
        compare(thisStats,logfile)

    if benchmarking:
        print >>logfile, ' '
        print >>logfile, '******** Benchmarking ******************' 
        print >>logfile, '*                                      *'
        print >>logfile,'Total wall clock time was:%s s ' % str(endTime - startTime)
        print >>logfile,'      baseline subtraction(%s independent data sets):%s s' % (len(antids), str(baseline2time - startTime))
        print >>logfile,'      imaging (%s images):%s s'% (len(antids), str(endTime - baseline2time))
        print >>logfile,'Total CPU        time was:%s s'% str(endProc - startProc)

    logfile.close()
except Exception, instance:
    print "###Error in sdtpimaging regression: ", instance;
finally:
    if logfile and (type(logfile) == file):
        logfile.close()
    os.chdir(curdir)
