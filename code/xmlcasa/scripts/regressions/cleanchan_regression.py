############################################
# test clean spw and channelization in various ways
# This version will test only the following tests defined in tests[{}]
# avel,nvel,chan,freq,frm,fr.5.
# In order to run everything, uncomment the other tests in tests[{}] (line 147),
# but it will take many hours.


import os, time

# options you can set in local variables:
# pyonly: just run through setChannelization, not full clean
# analonly: print stats about previously run data and make plot.

# e.g. 
# CASA> pyonly=True
# CASA> execfile("cleanchan_regression.py")


# root name of files produced:
rt="cln_w3oh"
pyonly=False

l=locals() 
if not l.has_key("analonly"): 
    analonly=False
if not analonly:
    print "foobar"
    os.system("rm -rf "+rt+"*")

startTime = time.time()
startProc = time.clock()

print '--Running clean chan/spw test--'

import datetime
datestring = datetime.datetime.isoformat(datetime.datetime.today())
outfile    = rt + datestring + '.log'
logfile    = open(outfile, 'w')
print 'Writing output to ' + outfile + "\n"

l=locals() 
if not l.has_key("repodir"): 
    repodir=os.getenv("CASAPATH").split(' ')[0]

print 'I think the data repository is at '+repodir
datadir=repodir+"/data/regression/cvel/input/"

# just do python part, not full on clean
if not l.has_key("pyonly"): 
    pyonly=False 

# get ms
if not os.path.exists(rt+".ms"):
    importuvfits(fitsfile=datadir+'W3OH_MC.UVFITS', vis=rt+'.ms')


# get stats of ms
tb.open(rt+'.ms/SPECTRAL_WINDOW')
cf = tb.getvarcol('CHAN_FREQ')
ch0_ms = cf['r1'][0][0] / 1e3
chn_ms = cf['r1'][510][0] / 1e3
del_ms = (cf['r1'][1][0] - 1e3*ch0_ms) /1e3
del2_ms = (1e3*chn_ms - cf['r1'][509][0]) /1e3
cw = tb.getvarcol('CHAN_WIDTH')
wid_ms = cw['r1'][0][0] /1e3
nch_ms = tb.getcol("NUM_CHAN")[0]
reffreq_ms = tb.getcol("REF_FREQUENCY")[0] / 1e3
tb.close()

# get line peak in ms:
tb.open(rt+".ms")
dat=tb.getcol("DATA")
tb.done()
spec=pl.zeros(nch_ms)
for i in range(nch_ms):
    spec[i]=pl.mean(dat[0,i,])

# the line is around ch260
x=pl.array(range(21))+250
y=spec[x]
fitchan_ms = pl.mean((x+1)*y)/pl.mean(y)
x0=floor(fitchan_ms)
xf=fitchan_ms-x0
fit_ms = cf['r1'][x0][0] + xf * (cf['r1'][x0+1][0]-cf['r1'][x0][0])
fit_ms=fit_ms / 1e3

del dat
# manually set the line center - the above doesn't work well because of the blended lines
fit_ms = 1665657.7

# PLOT
f=cf['r1']
f=f.flatten()

pl.ion()
pl.clf()
pl.plot(f,spec/4.6,label='ms')
pl.plot(f,spec/4.6,'mo',label='ms')
pl.xlim([1665640000,1665675000])




# start accumulating big array of stats
if pyonly:
    stats=[(nch_ms,"%fkHz" % ch0_ms,"%fkHz" % del_ms)]
else:
    stats=[(ch0_ms,del_ms,wid_ms,del2_ms,nch_ms,chn_ms,fit_ms)]

# and the name of the test associated with each row
sname=['ms']




# function to analyze images
def imstats(image):
    ia.open(image)
    ch0 = (ia.toworld([n/2,n/2,0,0],'n')['numeric'][3])/ 1e3
    nch = ia.shape()[3]
    chn = (ia.toworld([n/2,n/2,0,nch-1],'n')['numeric'][3])/ 1e3
    del1 = (ia.toworld([n/2,n/2,0,1],'n')['numeric'][3] - 1e3*ch0) /1e3
    wid = (ia.summary()['header']['incr'][3]) /1e3
    del2 = (1e3*chn - ia.toworld([n/2,n/2,0,nch-2],'n')['numeric'][3]) /1e3
    try:
        fit = ia.fitprofile(ngauss=1,poly=1,fit=False)['return']['elements']['*1']['parameters'][1]/1e3
    except:
        fit=0.
    return (ch0,del1,wid,del2,nch,chn,fit)

# function to swap first/last for vel results, for new convention of vel means
# _increasing_ vel by default 
def swapvel(stats):
    return (stats[5], -stats[1], -stats[2], -stats[3], stats[4], stats[0], stats[6])




# start/end velocities in ms:
f0=reffreq_ms
v1_ms=(f0-ch0_ms)/f0*299792.458
v0_ms=(f0-chn_ms)/f0*299792.458
incr = -del_ms/f0*299792.458
w4="%fkm/s" % incr

# tests of various modes:
tests=[
    {'name':'avel', 'mode':'velocity',                                 'desc':'velo default'},
#    {'name':'vel',  'mode':'velocity', 'spw':'0:200~299',              'desc':'velo spw=0:200~299'},
#    {'name':'vel3', 'mode':'velocity', 'spw':'0:200~299','sta':50,     'desc':'velo spw=0:200~299 sta=50'},
#    {'name':'vel4', 'mode':'velocity', 'spw':'0:200~299','sta':250,    'desc':'velo spw=0:200~299 sta=250'},
#    {'name':'vel2', 'mode':'velocity', 'spw':'0:210~250,0:280~290',    'desc':'velo spw=0:210~250,0:280~290'},
#    {'name':'cha3', 'mode':'channel',  'spw':'0:210~250,0:280~290',    'desc':'chan spw=0:210~250,0:280~290'},
#    {'name':'sve1', 'mode':'velocity', 'sta':"%fkm/s" % v0_ms,         'desc':'velo start=%5.2fkm/s' % v0_ms},
#    {'name':'sve2', 'mode':'velocity', 'sta':"%fkm/s" % (v0_ms-0.01),  'desc':'velo start=%5.2fkm/s' % (v0_ms-0.01)},
#    {'name':'sve3', 'mode':'velocity', 'sta':"%fkm/s" % (v0_ms-0.01),  'desc':'velo start=%5.2fkm/s linear' % (v0_ms-0.01), 'int':'linear'},
#    {'name':'sve4', 'mode':'velocity', 'sta':"%fkm/s" % (v0_ms-0.1),   'desc':'velo start=%5.2fkm/s' % (v0_ms-0.1)},
#    {'name':'sve5', 'mode':'velocity', 'sta':"%fkm/s" % -15.105,       'desc':'velo start=%5.2fkm/s' % -15.105},
    {'name':'nvel', 'mode':'velocity', 'sta':"%fkm/s" % v1_ms,'wid':w4,'desc':'velo start=%5.2fkm/s wid=%5.2fkm/s' % (v1_ms,incr)},
#    {'name': 'fel', 'mode':'velocity', 'veltype':'optical',            'desc':'felo default'},
    {'name':'chan', 'mode':'channel',                                  'desc':'chan default'},
#    {'name':'cha1', 'mode':'channel',  'sta':200,'wid':10,             'desc':'chan 200w10'},
#    {'name':'cha2', 'mode':'channel',  'sta':200,'nchan':99,           'desc':'chan 200-299'},
#    {'name':'cha4', 'mode':'channel',  'shift':wid_ms/3,               'desc':'chan start=%5.3fkHz' % (wid_ms/3)},
#    {'name':'chfr', 'mode':'channel',  'wid':2.5,                      'desc':'chan wid=2.5'},
#    {'name':'chfm', 'mode':'channel',  'wid':-1,                       'desc':'chan wid=-1'},   # segfaults
#    {'name':'frch', 'mode':'frequency','wid':2.5,                      'desc':'freq wid=2.5'},
    {'name':'frm',  'mode':'frequency','wid':"-%fkHz" % wid_ms,'sta':'%fkHz' % chn_ms, 'desc':'freq wid=-%6.1fKHz' % wid_ms},
    {'name':'freq', 'mode':'frequency',                                'desc':'freq default'},
#    {'name':'fr.3', 'mode':'frequency','shift':wid_ms/3,               'desc':'freq start=%5.3fkHz' % (wid_ms/3)},
    {'name':'fr.5', 'mode':'frequency','shift':wid_ms/2,               'desc':'freq start=%5.3fkHz' % (wid_ms/2)},
#    {'name':'fr.7', 'mode':'frequency','shift':wid_ms/3*2,             'desc':'freq start=%5.3fkHz' % (wid_ms*2/3)},
#    {'name':'fr.3l','mode':'frequency','shift':wid_ms/3,'int':'linear','desc':'freq start=%5.3fkHz lin' % (wid_ms/3)}
    ]


# a subset for the plot
#toplot=['avel','fel','chan','freq','fr.5']
toplot=[]

if pyonly:
    from cleanhelper import *
    imCln=imtool.create()
    vis=rt+'.ms'
    imset=cleanhelper(imCln, vis, False, casalog)
    outframe=''
else:
    # size of image to make
    n=64


#debug:
#tests=[tests[18]]


j=0
for te in tests:
    if te.has_key('spw'):
        spw=te['spw']
    else:
        spw=''
    if te.has_key('nchan'):
        nchan=te['nchan']
    else:
        nchan=-1
    if te.has_key('int'):
        interp=te['int']
    else:
        interp='nearest'
    if te.has_key('wid'):
        wid=te['wid']
    else:
        wid=''
        # should not be required but is in old versions of cleanhelper:
#        if te['mode']=='channel':
#            wid=1

    if te.has_key('veltype'):
        vtype=te['veltype']
    else:
        vtype='radio'

    if te.has_key('sta'):
        sta=te['sta']
    elif te.has_key('shift'):
        start=ch0_ms + te['shift']       
        sta="%f kHz" % start
    else:
        sta=''
        # should not be required but is in old versions of cleanhelper:
#        if te['mode']=='channel':
#            sta=0


    if pyonly:
        try:
            print te['mode'],spw,"",nchan,sta,wid,outframe,vtype,str(reffreq_ms)+'kHz'
            st = imset.setChannelization(te['mode'],spw,"",nchan,sta,wid,outframe,vtype,str(reffreq_ms)+'kHz')        
            # (localnchan, localstart, localwidth)=imset.setChannelization(mode,spw,field,nchan,start,width,outframe,veltype,restfreq)
        except:
            st = (-1,"","")
    else:
        try:
            if not analonly:
                print 'Running clean for name=%s, mode=%s'%(te['name'],te['mode'])
                clean(vis=rt+'.ms',
                      imagename=rt+'_'+te['name'],
                      cell="6arcsec",imsize=[n,n],
                      spw=spw,
                      mode=te['mode'],veltype=vtype,
                      start=sta,width=wid,nchan=nchan,
                      interpolation=interp,
                      niter=100,threshold='1mJy',
                      restfreq=str(reffreq_ms)+'kHz')
            if te['mode']=='velocity':
                st=swapvel(imstats(rt+'_'+te['name']+'.image'))
            else:
                st=imstats(rt+'_'+te['name']+'.image')
            if te.has_key('shift'):
                # shift back:
                shift=te['shift']
                st=(st[0]-shift, st[1],st[2],st[3],st[4], st[5]-shift, st[6]-shift)                

            if toplot.count(te['name'])>0:
                ia.open(rt+'_'+te['name']+'.image')
                foo=ia.getchunk(blc=[25,25,0,0],trc=[40,40,0,510],dropdeg=True,axes=[0,1])
                c=ia.coordsys()
                n=len(foo)
                f=pl.array(range(n))
                for i in range(n):
                    f[i]=c.toworld([32,32,0,i])['numeric'][3]
                ia.done()
                #f=f+100*j
                pl.plot(f,foo,label=te['desc'])
                j=j+1
        except:
            st=(reffreq_ms,0,0,0,-1,reffreq_ms,reffreq_ms)
    stats.append(st)
    sname.append(te['desc'])




# max name len
lth=0
for i in range(len(sname)):
    lth0=len(sname[i])
    if lth0>lth:
        lth=lth0
fmt="%-"+str(lth)+"s"

# header line
if pyonly:
    yo = fmt+" nchan   chan0                        width "
else:
    yo = fmt+" chan0 (kHz)       ch1-ch0    width      chn-(n-1)  n   lastchan          fit peak" 


# print "regular" stats:
# print yo % " "
print >> logfile, yo % " "
for i in range(len(sname)):
    if pyonly:
        #print fmt % sname[i], "%4i %20s %20s" % stats[i]
        print >> logfile, fmt % sname[i], "%4i %20s %20s" % stats[i]
    else:
        print >> logfile, fmt % sname[i], "%15.7f %11.7f %10.7f %10.7f %4i %16.7f %16.7f" % stats[i]


from matplotlib.font_manager import fontManager, FontProperties
font= FontProperties(size='x-small')
pl.legend(prop=font,loc=2)
pl.xlim([1665632000, 1665675000])
pl.ylim([-5,100])
pl.savefig( rt + datestring + ".png")


# print stats w/chans as pixel fractions relative to MS:

print yo % " "
if pyonly:
    del imCln
    regstate=True
    # print ms values:
    print fmt % sname[0], "%4i %20s %20s" % stats[0]
    # ms goes from ch0_ms to chn_ms in freq.
    for i in range(1,len(sname)):
        if stats[i][0]>0:
            if tests[i-1]['mode']=='velocity':
                if tests[i-1].has_key('veltype'):
                    vtype=tests[i-1]['veltype']
                else:
                    vtype='radio'
                v0=qa.convert(stats[i][1],'km/s')['value']
                v1=v0+qa.convert(stats[i][2],'km/s')['value']
                if vtype=='radio':
                    f0 = reffreq_ms * ( 1 - v0/299792.458)
                    f1 = reffreq_ms * ( 1 - v1/299792.458)
                else:
                    f0 = reffreq_ms / ( 1 + v0/299792.458)
                    f1 = reffreq_ms / ( 1 + v1/299792.458)
                w=f1-f0
            elif tests[i-1]['mode']=='channel':
                if type(stats[i][1])==str:
                    f0=qa.convert(stats[i][1],'kHz')['value']
                    w=qa.convert(stats[i][2],'kHz')['value']
                else:
                    f0=ch0_ms + wid_ms*stats[i][1]
                    w=wid_ms*stats[i][2]
            else:
                if type(stats[i][1])==str:
                    f0 = qa.convert(stats[i][1],'kHz')['value']
                    w = qa.convert(stats[i][2],'kHz')['value']
                else:
                    f0=ch0_ms + wid_ms*stats[i][1]
                    w=wid_ms*stats[i][2]    
            print fmt % sname[i], "%4i ch %15f %16fkHz" % (stats[i][0], (f0-ch0_ms)/wid_ms, w)
            print >> logfile, fmt % sname[i], "%4i ch %15f %16fkHz" % (stats[i][0], (f0-ch0_ms)/wid_ms, w)
        else:
            print fmt % sname[i], " FAIL"
            print >> logfile, fmt % sname[i], " FAIL"    
else:
    print fmt % sname[0], "%15.7f %10.7f %10.7f %10.7f %4i %16.7f %16.7f" % stats[0]
    print >> logfile, fmt % sname[0], "%15.7f %10.7f %10.7f %10.7f %4i %16.7f %16.7f" % stats[0]
    for i in range(1,len(sname)):
        foo = pl.array(stats[i])
        if stats[i][4]>0:
            foo[0] = (foo[0]-ch0_ms)/wid_ms
            foo[5] = (foo[5]-ch0_ms)/wid_ms
            foo[6] = (foo[6]-fit_ms)/wid_ms
            print fmt % sname[i], "%15.7f %10.7f %10.7f %10.7f %4i %16.7f %16.7f" % (foo[0],foo[1],foo[2],foo[3],foo[4],foo[5],foo[6])
            print >> logfile, fmt % sname[i], "%15.7f %10.7f %10.7f %10.7f %4i %16.7f %16.7f" % (foo[0],foo[1],foo[2],foo[3],foo[4],foo[5],foo[6])
        else:
            print fmt % sname[i], " FAIL"
            print >> logfile, fmt % sname[i], " FAIL"
        
        
# regress
    regstate=True

# test everything against ms assumed to be first stat
# tolerance
    tol=0.01

    tests=['ch0','ch0-ch1','width','chn-ch(n-1)','nchan','chn','fit']
    for run in range(1,len(sname)-1):
        for te in range(len(tests)):
            adiff=abs(stats[run][te]-stats[0][te])/stats[0][te]
            if adiff < tol:
                print >> logfile, "* Passed %-10s test, got %-11.5g expected %-11.5g" % (te,stats[run][te],stats[0][te])
            else:
                print >> logfile, "* FAILED %-10s test, got %-11.5g expected %-11.5g " % (te,stats[run][te],stats[0][te])
                regstate = False


        

print >> logfile,'---'
if regstate:
    print >> logfile, 'Passed',
else:
    print >> logfile, 'FAILED',
print >> logfile, 'regression test for clean chan/spw'
print >>logfile,'---'
print >>logfile,'*********************************'
    
endTime = time.time()
endProc = time.clock()

print >>logfile,''
print >>logfile,'********** Benchmarking **************'
print >>logfile,''
print >>logfile,'Total wall clock time was: %8.3f s.' % (endTime - startTime)
print >>logfile,'Total CPU        time was: %8.3f s.' % (endProc - startProc)
print >>logfile,'Wall processing  rate was: %8.3f MB/s.' % (17896.0 /
                                                            (endTime - startTime))


logfile.close()
						    
print '--Finished clean chan/spw--'
