# test_cvel-B
# Additional test for the cvel task to verify SPW combination
#
# Fake datasets are created which contain visibilities equal to the frequency of each channel.
# These are then processed and in the combined SPWS, the difference between channel frequency
# and its visibility must then be less than 1 %. (Tolerance can be reduced further.)
#
# DP, Sept 2012


import numpy as np

def fillfakevis(myvis):

    mytb = tbtool()

    mycol = 'DATA'
    myoffset = 0
    myfactor = 1.
    mytb.open(myvis+'/DATA_DESCRIPTION')
    spwids = mytb.getcol('SPECTRAL_WINDOW_ID')
    mytb.close()
    mytb.open(myvis+'/SPECTRAL_WINDOW')
    thefreqs = mytb.getcol('CHAN_FREQ') # note: first index is channel, second is spw id
    mytb.close()

    mytb.open(myvis, nomodify=False)
    for i in xrange(0,mytb.nrows()):
        dd = mytb.getcell('DATA_DESC_ID',i)
        thespwid = spwids[dd]
        # generate vis
        thevis = mytb.getcell(mycol, i)
        for j in xrange(len(thevis)): # loop over pol
            for k in xrange(len(thevis[j])): # loop over freq
                thevis[j][k] = thefreqs[k][thespwid]/myfactor - myoffset
        mytb.putcell(mycol, i, thevis)
        
    mytb.close()

####################

def createtestdata():

    os.system('rm -rf *A2256LC2_4.5s-1.ms *AB_no_overlap.ms *BA_no_overlap.ms *AB_overlap.ms *BA_overlap.ms')
    os.system('cp -R  '+os.environ['CASAPATH'].split()[0]+'/data/regression/unittest/concat/input/A2256LC2_4.5s-1.ms .')
    os.system('cp -R A2256LC2_4.5s-1.ms AB_no_overlap.ms')
    os.system('cp -R A2256LC2_4.5s-1.ms BA_no_overlap.ms')
    os.system('cp -R A2256LC2_4.5s-1.ms AB_overlap.ms')
    os.system('cp -R A2256LC2_4.5s-1.ms BA_overlap.ms')

    mytb = tbtool()

    thevis = 'AB_no_overlap.ms'
    myspw = thevis+'/SPECTRAL_WINDOW'
    print myspw
    mytb.open(myspw, nomodify=False)
    thefreqs = mytb.getcol('CHAN_FREQ')
    thereffreq = mytb.getcol('REF_FREQUENCY')
    thechanwid = mytb.getcol('CHAN_WIDTH')
    thenumchan = mytb.getcol('NUM_CHAN')

    if(thefreqs[0][1] - abs(thechanwid[0][1]) < thefreqs[thenumchan[0]-1][0] + thechanwid[thenumchan[0]-1][0]):

        thefreqoffset = 2 * (thefreqs[thenumchan[0]-1][0] - thefreqs[0][1])

        print "thenumchan[0],thechanwid[0][0]", thenumchan[0], ' ' , thechanwid[0][0]
        print 'freq offset ', thefreqoffset

        for i in xrange(1,mytb.nrows()):
            thereffreq[i] -=  thefreqoffset
            for j in xrange(thenumchan[i]):
                thefreqs[j][i] -= thefreqoffset

        mytb.putcol('CHAN_FREQ', thefreqs)
        mytb.putcol('REF_FREQUENCY', thereffreq)

    mytb.close()

    os.system('cp -R '+thevis+' desc-'+thevis)

    makedescend('desc-'+thevis)

    fillfakevis(thevis) 
    fillfakevis('desc-'+thevis) 

    ###################

    thevis = 'AB_overlap.ms'
    myspw = thevis + '/SPECTRAL_WINDOW'
    print myspw
    mytb.open(myspw, nomodify=False)
    thefreqs = mytb.getcol('CHAN_FREQ')
    thereffreq = mytb.getcol('REF_FREQUENCY')
    thechanwid = mytb.getcol('CHAN_WIDTH')
    thenumchan = mytb.getcol('NUM_CHAN')

    thefreqoffset = thenumchan[0]*0.1*thechanwid[0][0]
    print "thenumchan[0],thechanwid[0][0]", thenumchan[0], ' ' , thechanwid[0][0]
    print 'freq offset ', thefreqoffset

    for i in xrange(1,mytb.nrows()):
        thereffreq[i] -=  thefreqoffset
        for j in xrange(thenumchan[i]):
            thefreqs[j][i] -= thefreqoffset

    mytb.putcol('CHAN_FREQ', thefreqs)
    mytb.putcol('REF_FREQUENCY', thereffreq)

    mytb.close()

    os.system('cp -R '+thevis+' desc-'+thevis)

    makedescend('desc-'+thevis)

    fillfakevis(thevis) 
    fillfakevis('desc-'+thevis) 

    ###################

    thevis = 'BA_no_overlap.ms'
    myspw = thevis+'/SPECTRAL_WINDOW'
    print myspw
    mytb.open(myspw, nomodify=False)
    thefreqs = mytb.getcol('CHAN_FREQ')
    thereffreq = mytb.getcol('REF_FREQUENCY')
    thechanwid = mytb.getcol('CHAN_WIDTH')
    thenumchan = mytb.getcol('NUM_CHAN')

    thefreqoffset = 1.1 * (thefreqs[thenumchan[1]-1][1] - thefreqs[0][0])

    print "thenumchan[0],thechanwid[0][0]", thenumchan[0], ' ' , thechanwid[0][0]
    print 'freq offset ', thefreqoffset

    for i in xrange(1,mytb.nrows()):
        thereffreq[i] -=  thefreqoffset
        for j in xrange(thenumchan[i]):
            thefreqs[j][i] -= thefreqoffset

    mytb.putcol('CHAN_FREQ', thefreqs)
    mytb.putcol('REF_FREQUENCY', thereffreq)

    mytb.close()

    os.system('cp -R '+thevis+' desc-'+thevis)

    makedescend('desc-'+thevis)

    fillfakevis(thevis) 
    fillfakevis('desc-'+thevis) 


    ###########

    thevis = 'BA_overlap.ms'
    myspw = thevis+'/SPECTRAL_WINDOW'
    print myspw
    mytb.open(myspw, nomodify=False)
    thefreqs = mytb.getcol('CHAN_FREQ')
    thereffreq = mytb.getcol('REF_FREQUENCY')
    thechanwid = mytb.getcol('CHAN_WIDTH')
    thenumchan = mytb.getcol('NUM_CHAN')

    thefreqoffset = 0.7 * (thefreqs[thenumchan[1]-1][1] - thefreqs[0][0])

    print "thenumchan[0],thechanwid[0][0]", thenumchan[0], ' ' , thechanwid[0][0]
    print 'freq offset ', thefreqoffset

    for i in xrange(1,mytb.nrows()):
        thereffreq[i] -=  thefreqoffset
        for j in xrange(thenumchan[i]):
            thefreqs[j][i] -= thefreqoffset

    mytb.putcol('CHAN_FREQ', thefreqs)
    mytb.putcol('REF_FREQUENCY', thereffreq)

    mytb.close()

    os.system('cp -R '+thevis+' desc-'+thevis)

    makedescend('desc-'+thevis)

    fillfakevis(thevis) 
    fillfakevis('desc-'+thevis)

    #######################

    return True

def makedescend(myvis):

    mytb = tbtool()

    myspw = myvis+'/SPECTRAL_WINDOW'
    print myspw
    mytb.open(myspw, nomodify=False)
    thefreqs = mytb.getcol('CHAN_FREQ')
    theoldfreqs = np.array(thefreqs)
    thereffreq = mytb.getcol('REF_FREQUENCY')
    thenumchan = mytb.getcol('NUM_CHAN')

    for i in xrange(mytb.nrows()):
        for j in xrange(thenumchan[i]):
            thefreqs[j][i] = theoldfreqs[thenumchan[i]-1-j][i] 
            
        thereffreq[i] = thefreqs[0][i]

    mytb.putcol('CHAN_FREQ', thefreqs)
    mytb.putcol('REF_FREQUENCY', thereffreq)

    mytb.close()




###################

def testvisibs(myvis, threshold):

    print "Testing ", myvis, ' with threshold ', threshold, '(= max tolerated error as fraction of channel width)'

    mytb = tbtool()

    rval = True
    
    mycvis = 'cvelled-'+myvis

    os.system('rm -rf '+mycvis)

    cvel(vis=myvis, outputvis=mycvis, mode='channel_b')

    mytb.open(mycvis+'/SPECTRAL_WINDOW')
    thefreq = mytb.getcell('CHAN_FREQ',0)
    thecw = mytb.getcell('CHAN_WIDTH',0)
    mytb.close()

    mytb.open(mycvis)
    count = 0
    logfile = mycvis.split('.')[0]+'_testvisibs.txt'
    os.system('rm -rf '+logfile)
    fp = open(logfile, 'w')
    fp.write(mycvis+'\n')
    for i in xrange(mytb.nrows()):
        thevis = mytb.getcell('DATA',i)
        failure = False
        for j in xrange(len(thevis)): # pol
            for k in xrange(len(thevis[j])): # channel
                if(abs(thevis[j][k]-thefreq[k])>thecw[k]*threshold):
                    fp.write( 'row, pol, chan, thevis, thefreq, thevis-thefreq, frac: '\
                              +str(i)+', '+str(j)+', '+str(k)+', '+str(thevis[j][k])+', '\
                              +str(thefreq[k])+', '+str(thevis[j][k]-thefreq[k])+', '+str((thevis[j][k]-thefreq[k])/thecw[k])\
                              +'\n'
                              )
                    failure = True
        if failure:
            count += 1
            rval = False
    fp.close()

    if not rval:
        print "Failed in ", count, " of ", mytb.nrows() , " cases (", count/(0.01*mytb.nrows()), "%)"

    mytb.close()
    
    return rval

###################

if not os.path.exists('desc-BA_overlap.ms'):
    createtestdata()

tolerance = 0.005 # (= max tolerated error as fraction of channel width)

passed = True

for mname in ['AB_no_overlap.ms', 'AB_overlap.ms', 'BA_no_overlap.ms', 'BA_overlap.ms']: 
    for prefix in ['', 'desc-']:
        rval = testvisibs(prefix+mname, tolerance)
        print rval
        passed = passed and rval
    
if passed:
    print "PASSED"
else:
    print "FAIL"
