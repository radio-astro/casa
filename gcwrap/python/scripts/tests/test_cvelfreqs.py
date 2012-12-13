# Unit test for ms.cvelfreqs()

import os
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest
import numpy as np

# Test Matrix
# mode = 'channel', 'frequency', 'velocity'
# spwids = [0], [1], [0,1], [1,2], [0,1,2]
# nchan = -1, 10, 5, 6
# start = 0, 1, 15 (equivalent freqs and velos)
# width = 1, 2, 3, -1, -2, -3 (equivalent freqs and velos)
# ascending and descending input
# number of cases: 3 x 5 x 4 x 3 x 6 x 2 = 2160 !

# Test Matrix covered so far:
# mode = 'channel', 'optical velocity', 'frequency', 'radio velocity'
# spwids = [0], [1], [0,1], [1,2], [0,1,2]
# nchan = -1, 10, 5, 6
# start = 0, 1, 15 (equivalent freqs and velos)
# width = 1, -1 (equivalent freqs and velos)
# ascending and descending

tests_to_do = []
    
mytotal = 0
myfailures = 0
failed = []

def testit():
    global mymode, mycase, myspwids, mynchan, mystart, mywidth, myexpectation, mytotal, myfailures
    global tests_to_do, failed
    if (mycase in tests_to_do) or (tests_to_do==[]):
        try:
            outf = ms.cvelfreqs(spwids=myspwids,mode=mymode,nchan=mynchan,start=mystart,width=mywidth)
            if not type(outf)==type([]):
                outf = list(outf)
            print mycase
            print outf
            print myexpectation
            if outf != myexpectation:
                print "Test failed for case ", mycase
                print "spwids=", myspwids,", mode=",mymode,",nchan=",mynchan,", start=",mystart,", width=",mywidth
                myfailures = myfailures + 1
                failed.append(mycase)
            else:
                print mycase, " OK"
        except:
            print "Test failed with exception for case ", mycase
            print "spwids=", myspwids,", mode=",mymode,",nchan=",mynchan,", start=",mystart,", width=",mywidth
            myfailures = myfailures + 1         
            failed.append(mycase)
        mytotal = mytotal + 1

def v_iseq(a,b,tol): # test if velocity arrays are equal
    if(len(a)!=len(b)):
        print len(a), len(b)
        return False
    for i in range(0,len(a)):
        if(abs(a[i]-b[i])>tol):
            print i, a[i], b[i]
            return False
    return True

def vopt(f):
    global myrestfrq
    return (myrestfrq/f - 1.) * qa.constants('c')['value']

def freq_from_vopt(v):
    global myrestfrq
    return myrestfrq/(1. + v/qa.constants('c')['value'])

def vrad(f):
    global myrestfrq
    return qa.constants('c')['value'] * (1. - f/myrestfrq)

def freq_from_vrad(v):
    global myrestfrq
    return myrestfrq * (1. - v/qa.constants('c')['value'])

def testitb():
    # for vopt mode
    global mymode, mycase, myspwids, mynchan, mystart, mywidth, myveltype, myrestfrq, myexpectation, mytotal, myfailures
    global tests_to_do, failed
    if (mycase in tests_to_do) or (tests_to_do==[]):
        try:
            outf = ms.cvelfreqs(spwids=myspwids,mode=mymode,nchan=mynchan,start=mystart,width=mywidth, veltype=myveltype, restfreq=str(myrestfrq)+'Hz')
            print mycase
            print outf
            if not type(outf)==type([]):
                outf = list(outf)
            fexpectation = []
            vexpectation = []
            vout = []
            for i in range(0,len(myexpectation)):
                fexpectation.append(freq_from_vopt(myexpectation[i]))
                vexpectation.append(vopt(fexpectation[i]))
            vout = []
            for i in range(0,len(outf)):
                vout.append(vopt(outf[i]))
            print "expected: ", myexpectation
            print "exp freq: ", fexpectation
            print "output v: ", vout
            print "exp v:    ", vexpectation
            if not v_iseq(outf,fexpectation,10.0): # 10 Hz tolerance
                print "Test failed for case ", mycase
                print "spwids=", myspwids,", mode=",mymode,",nchan=",mynchan,", start=",mystart,","
                print "   width=",mywidth,", veltype=",myveltype, ", restfreq=", str(myrestfrq)+'Hz'
                myfailures = myfailures + 1
                failed.append(mycase)
            else:
                print mycase, " OK"
        except:
            print "Test failed with exception ", sys.exc_info()," for case ", mycase
            print "spwids=", myspwids,", mode=",mymode,",nchan=",mynchan,", start=",mystart,", width=",mywidth,", veltype=",myveltype
            myfailures = myfailures + 1         
            failed.append(mycase)
        mytotal = mytotal + 1

def testitc():
    # for vrad mode
    global mymode, mycase, myspwids, mynchan, mystart, mywidth, myveltype, myrestfrq, myexpectation, mytotal, myfailures
    global tests_to_do, failed
    if (mycase in tests_to_do) or (tests_to_do==[]):
        try:
            outf = ms.cvelfreqs(spwids=myspwids,mode=mymode,nchan=mynchan,start=mystart,width=mywidth, veltype=myveltype, restfreq=str(myrestfrq)+'Hz')
            print mycase
            print outf
            if not type(outf)==type([]):
                outf = list(outf)
            fexpectation = []
            vexpectation = []
            vout = []
            for i in range(0,len(myexpectation)):
                fexpectation.append(freq_from_vrad(myexpectation[i]))
                vexpectation.append(vrad(fexpectation[i]))
            vout = []
            for i in range(0,len(outf)):
                vout.append(vrad(outf[i]))
            print "expected: ", myexpectation
            print "exp freq: ", fexpectation
            print "output v: ", vout
            print "exp v:    ", vexpectation
            if not v_iseq(outf,fexpectation,10.0): # 10 Hz tolerance
                print "Test failed for case ", mycase
                print "spwids=", myspwids,", mode=",mymode,",nchan=",mynchan,", start=",mystart,","
                print "   width=",mywidth,", veltype=",myveltype, ", restfreq=", str(myrestfrq)+'Hz'
                myfailures = myfailures + 1
                failed.append(mycase)
            else:
                print mycase, " OK"
        except:
            print "Test failed with exception ", sys.exc_info()," for case ", mycase
            print "spwids=", myspwids,", mode=",mymode,",nchan=",mynchan,", start=",mystart,", width=",mywidth,", veltype=",myveltype
            myfailures = myfailures + 1         
            failed.append(mycase)
        mytotal = mytotal + 1


def testchannelmode(caseoffset, isDesc):
    global mymode, mycase, myspwids, mynchan, mystart, mywidth, myexpectation, mytotal, myfailures
    global tests_to_do, failed

    # mode = channel
    mymode = 'channel'
    mycase = caseoffset + 0
    myspwids = [0]
    mynchan = -1
    mystart = 0
    mywidth = 1
    myexpectation = [1,2,3,4,5,6,7,8,9,10]
    if(isDesc):
        myexpectation.reverse()
        
    testit()

    #################
    mycase = caseoffset + 1
    myspwids = [0]
    mynchan = 10
    mystart = 0
    mywidth = 1
    myexpectation = [1,2,3,4,5,6,7,8,9,10]

    if(isDesc):
        myexpectation.reverse()
    testit()

    #################
    mycase = caseoffset + 2
    myspwids = [0]
    mynchan = 5
    mystart = 0
    mywidth = 1
    myexpectation = [1,2,3,4,5]

    if(isDesc):
        myexpectation = [10,9,8,7,6]
    testit()

    #################
    mycase = caseoffset + 3
    myspwids = [0]
    mynchan = 6
    mystart = 0
    mywidth = 1
    myexpectation = [1,2,3,4,5,6]

    if(isDesc):
        myexpectation = [10,9,8,7,6,5]
    testit()

    #################
    #################
    mycase = caseoffset + 4
    myspwids = [1]
    mynchan = -1
    mystart = 0
    mywidth = 1
    myexpectation = [10,11,12,13,14,15,16,17,18,19]

    if(isDesc):
        myexpectation.reverse()
    testit()

    #################
    mycase = caseoffset + 5
    myspwids = [1]
    mynchan = 10
    mystart = 0
    mywidth = 1
    myexpectation = [10,11,12,13,14,15,16,17,18,19]

    if(isDesc):
        myexpectation.reverse()
    testit()

    #################
    mycase = caseoffset + 6
    myspwids = [1]
    mynchan = 5
    mystart = 0
    mywidth = 1
    myexpectation = [10,11,12,13,14]

    if(isDesc):
        myexpectation = [19,18,17,16,15]
    testit()

    #################
    mycase = caseoffset + 7
    myspwids = [1]
    mynchan = 6
    mystart = 0
    mywidth = 1
    myexpectation = [10,11,12,13,14,15]

    if(isDesc):
        myexpectation = [19,18,17,16,15,14]
    testit()
    #################
    #################
    mycase = caseoffset + 8
    myspwids = [0,1]
    mynchan = -1
    mystart = 0
    mywidth = 1
    myexpectation = [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19]

    testit()

    #################
    mycase = caseoffset + 9
    myspwids = [0,1]
    mynchan = 10
    mystart = 0
    mywidth = 1
    myexpectation = [1,2,3,4,5,6,7,8,9,10]

    testit()

    #################
    mycase = caseoffset + 10
    myspwids = [0,1]
    mynchan = 5
    mystart = 0
    mywidth = 1
    myexpectation = [1,2,3,4,5]

    testit()

    #################
    mycase = caseoffset + 11
    myspwids = [0,1]
    mynchan = 6
    mystart = 0
    mywidth = 1
    myexpectation = [1,2,3,4,5,6]

    testit()

    #################
    #################
    mycase = caseoffset + 12
    myspwids = [1,2]
    mynchan = -1
    mystart = 0
    mywidth = 1
    myexpectation = [10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28]

    testit()

    #################
    mycase = caseoffset + 13
    myspwids = [1,2]
    mynchan = 10
    mystart = 0
    mywidth = 1
    myexpectation = [10,11,12,13,14,15,16,17,18,19]

    testit()

    #################
    mycase = caseoffset + 14
    myspwids = [1,2]
    mynchan = 5
    mystart = 0
    mywidth = 1
    myexpectation = [10,11,12,13,14]

    testit()

    #################
    mycase = caseoffset + 15
    myspwids = [1,2]
    mynchan = 6
    mystart = 0
    mywidth = 1
    myexpectation = [10,11,12,13,14,15]

    testit()
    #################
    #################
    mycase = caseoffset + 16
    myspwids = [0,1,2]
    mynchan = -1
    mystart = 0
    mywidth = 1
    myexpectation = [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28]

    testit()

    #################
    mycase = caseoffset + 17
    myspwids = [0,1,2]
    mynchan = 10
    mystart = 0
    mywidth = 1
    myexpectation = [1,2,3,4,5,6,7,8,9,10]

    testit()

    #################
    mycase = caseoffset + 18
    myspwids = [0,1,2]
    mynchan = 5
    mystart = 0
    mywidth = 1
    myexpectation = [1,2,3,4,5]

    testit()

    #################
    mycase = caseoffset + 19
    myspwids = [0,1,2]
    mynchan = 6
    mystart = 0
    mywidth = 1
    myexpectation = [1,2,3,4,5,6]

    testit()

    ##########################
    ####### start = 1 ########
    ##########################

    mystart = 1
    #################
    mycase = caseoffset + 20
    myspwids = [0]
    mynchan = -1

    mywidth = 1
    myexpectation = [2,3,4,5,6,7,8,9,10]

    if(isDesc):
        myexpectation = [9,8,7,6,5,4,3,2,1]
    testit()

    #################
    mycase = caseoffset + 21
    myspwids = [0]
    mynchan = 10

    mywidth = 1
    myexpectation = [2,3,4,5,6,7,8,9,10,11] # feature!

    if(isDesc):
        myexpectation = [9,8,7,6,5,4,3,2,1,0] # feature!
    testit()

    #################
    mycase = caseoffset + 22
    myspwids = [0]
    mynchan = 5

    mywidth = 1
    myexpectation = [2,3,4,5,6]

    if(isDesc):
        myexpectation = [9,8,7,6,5]
    testit()

    #################
    mycase = caseoffset + 23
    myspwids = [0]
    mynchan = 6

    mywidth = 1
    myexpectation = [2,3,4,5,6,7]

    if(isDesc):
        myexpectation = [9,8,7,6,5,4]
    testit()

    #################
    #################
    mycase = caseoffset + 24
    myspwids = [1]
    mynchan = -1

    mywidth = 1
    myexpectation = [11,12,13,14,15,16,17,18,19]

    if(isDesc):
        myexpectation = [18,17,16,15,14,13,12,11,10]
    testit()

    #################
    mycase = caseoffset + 25
    myspwids = [1]
    mynchan = 10

    mywidth = 1
    myexpectation = [11,12,13,14,15,16,17,18,19,20] # feature! if nchan is set, we can exceed the data range

    if(isDesc):
        myexpectation = [18,17,16,15,14,13,12,11,10,9] # feature!
    testit()

    #################
    mycase = caseoffset + 26
    myspwids = [1]
    mynchan = 5

    mywidth = 1
    myexpectation = [11,12,13,14,15]

    if(isDesc):
        myexpectation = [18,17,16,15,14]
    testit()

    #################
    mycase = caseoffset + 27
    myspwids = [1]
    mynchan = 6

    mywidth = 1
    myexpectation = [11,12,13,14,15,16]

    if(isDesc):
        myexpectation = [18,17,16,15,14,13]
    testit()
    #################
    #################
    mycase = caseoffset + 28
    myspwids = [0,1]
    mynchan = -1

    mywidth = 1
    myexpectation = [2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19]

    testit()

    #################
    mycase = caseoffset + 29
    myspwids = [0,1]
    mynchan = 10

    mywidth = 1
    myexpectation = [2,3,4,5,6,7,8,9,10,11]

    testit()

    #################
    mycase = caseoffset + 30
    myspwids = [0,1]
    mynchan = 5

    mywidth = 1
    myexpectation = [2,3,4,5,6]

    testit()

    #################
    mycase = caseoffset + 31
    myspwids = [0,1]
    mynchan = 6

    mywidth = 1
    myexpectation = [2,3,4,5,6,7]

    testit()

    #################
    #################
    mycase = caseoffset + 32
    myspwids = [1,2]
    mynchan = -1

    mywidth = 1
    myexpectation = [11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28]

    testit()

    #################
    mycase = caseoffset + 33
    myspwids = [1,2]
    mynchan = 10

    mywidth = 1
    myexpectation = [11,12,13,14,15,16,17,18,19,20]

    testit()

    #################
    mycase = caseoffset + 34
    myspwids = [1,2]
    mynchan = 5

    mywidth = 1
    myexpectation = [11,12,13,14,15]

    testit()

    #################
    mycase = caseoffset + 35
    myspwids = [1,2]
    mynchan = 6

    mywidth = 1
    myexpectation = [11,12,13,14,15,16]

    testit()
    #################
    #################
    mycase = caseoffset + 36
    myspwids = [0,1,2]
    mynchan = -1

    mywidth = 1
    myexpectation = [2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28]

    testit()

    #################
    mycase = caseoffset + 37
    myspwids = [0,1,2]
    mynchan = 10

    mywidth = 1
    myexpectation = [2,3,4,5,6,7,8,9,10,11]

    testit()

    #################
    mycase = caseoffset + 38
    myspwids = [0,1,2]
    mynchan = 5

    mywidth = 1
    myexpectation = [2,3,4,5,6]

    testit()

    #################
    mycase = caseoffset + 39
    myspwids = [0,1,2]
    mynchan = 6

    mywidth = 1
    myexpectation = [2,3,4,5,6,7]

    testit()

    ##########################
    ####### start = 15 ########
    ##########################

    mystart = 15
    #################
    mycase = caseoffset + 40
    myspwids = [0]
    mynchan = -1

    mywidth = 1
    myexpectation = [1,2,3,4,5,6,7,8,9,10] # start is reset to 0

    if(isDesc):
        myexpectation.reverse()
    testit()

    #################
    mycase = caseoffset + 41
    myspwids = [0]
    mynchan = 10

    mywidth = 1
    myexpectation = [1,2,3,4,5,6,7,8,9,10] # start is reset to 0

    if(isDesc):
        myexpectation.reverse()
    testit()

    #################
    mycase = caseoffset + 42
    myspwids = [0]
    mynchan = 5

    mywidth = 1
    myexpectation = [1,2,3,4,5] # start is reset to 0

    if(isDesc):
        myexpectation.reverse()
    testit()

    #################
    mycase = caseoffset + 43
    myspwids = [0]
    mynchan = 6

    mywidth = 1
    myexpectation = [1,2,3,4,5,6]  # start is reset to 0

    if(isDesc):
        myexpectation.reverse()
    testit()

    #################
    #################
    mycase = caseoffset + 44
    myspwids = [1]
    mynchan = -1

    mywidth = 1
    myexpectation = [10,11,12,13,14,15,16,17,18,19]  # start is reset to 0

    if(isDesc):
        myexpectation.reverse()
    testit()

    #################
    mycase = caseoffset + 45
    myspwids = [1]
    mynchan = 10

    mywidth = 1
    myexpectation = [10,11,12,13,14,15,16,17,18,19]  # start is reset to 0

    if(isDesc):
        myexpectation.reverse()
    testit()

    #################
    mycase = caseoffset + 46
    myspwids = [1]
    mynchan = 5

    mywidth = 1
    myexpectation = [10,11,12,13,14]  # start is reset to 0

    if(isDesc):
        myexpectation.reverse()
    testit()

    #################
    mycase = caseoffset + 47
    myspwids = [1]
    mynchan = 6

    mywidth = 1
    myexpectation = [10,11,12,13,14,15]  # start is reset to 0

    if(isDesc):
        myexpectation.reverse()
    testit()
    #################
    #################
    mycase = caseoffset + 48
    myspwids = [0,1]
    mynchan = -1

    mywidth = 1
    myexpectation = [16,17,18,19]

    testit()

    #################
    mycase = caseoffset + 49
    myspwids = [0,1]
    mynchan = 10

    mywidth = 1
    myexpectation = [16,17,18,19,20,21,22,23,24,25] # feature!

    testit()

    #################
    mycase = caseoffset + 50
    myspwids = [0,1]
    mynchan = 5

    mywidth = 1
    myexpectation = [16,17,18,19,20] # feature!

    testit()

    #################
    mycase = caseoffset + 51
    myspwids = [0,1]
    mynchan = 6

    mywidth = 1
    myexpectation = [16,17,18,19,20,21] # feature!

    testit()

    #################
    #################
    mycase = caseoffset + 52
    myspwids = [1,2]
    mynchan = -1

    mywidth = 1
    myexpectation = [25,26,27,28]

    testit()

    #################
    mycase = caseoffset + 53
    myspwids = [1,2]
    mynchan = 10

    mywidth = 1
    myexpectation = [25,26,27,28,29,30,31,32,33,34] # feature! see case 25

    testit()

    #################
    mycase = caseoffset + 54
    myspwids = [1,2]
    mynchan = 5

    mywidth = 1
    myexpectation = [25,26,27,28,29] # feature

    testit()

    #################
    mycase = caseoffset + 55
    myspwids = [1,2]
    mynchan = 6

    mywidth = 1
    myexpectation = [25,26,27,28,29,30]  # feature! see case 25

    testit()
    #################
    #################
    mycase = caseoffset + 56
    myspwids = [0,1,2]
    mynchan = -1

    mywidth = 1
    myexpectation = [16,17,18,19,20,21,22,23,24,25,26,27,28]

    testit()

    #################
    mycase = caseoffset + 57
    myspwids = [0,1,2]
    mynchan = 10

    mywidth = 1
    myexpectation = [16,17,18,19,20,21,22,23,24,25]

    testit()

    #################
    mycase = caseoffset + 58
    myspwids = [0,1,2]
    mynchan = 5

    mywidth = 1
    myexpectation = [16,17,18,19,20]

    testit()

    #################
    mycase = caseoffset + 59
    myspwids = [0,1,2]
    mynchan = 6

    mywidth = 1
    myexpectation = [16,17,18,19,20,21]

    testit()

    ##################
    ## width = -1 ####
    mywidth = -1
    #################

    mycase = caseoffset + 60
    myspwids = [0]
    mynchan = -1
    mystart = 9

    myexpectation = [1,2,3,4,5,6,7,8,9,10]

    if(isDesc):
        myexpectation.reverse()
    testit()

    #################
    mycase = caseoffset + 60 + 1
    myspwids = [0]
    mynchan = 10
    mystart = 9

    myexpectation = [1,2,3,4,5,6,7,8,9,10]

    if(isDesc):
        myexpectation.reverse()
    testit()

    #################
    mycase = caseoffset + 60 + 2
    myspwids = [0]
    mynchan = 5
    mystart = 4

    myexpectation = [1,2,3,4,5]

    if(isDesc):
        myexpectation = [10,9,8,7,6] # feature!
    testit()

    #################
    mycase = caseoffset + 60 + 3
    myspwids = [0]
    mynchan = 6
    mystart = 5

    myexpectation = [1,2,3,4,5,6]

    if(isDesc):
        myexpectation = [10,9,8,7,6,5]
    testit()

    #################
    #################
    mycase = caseoffset + 60 + 4
    myspwids = [1]
    mynchan = -1
    mystart = 9

    myexpectation = [10,11,12,13,14,15,16,17,18,19]

    if(isDesc):
        myexpectation.reverse()
    testit()

    #################
    mycase = caseoffset + 60 + 5
    myspwids = [1]
    mynchan = 10
    mystart = 9

    myexpectation = [10,11,12,13,14,15,16,17,18,19]

    if(isDesc):
        myexpectation.reverse()
    testit()

    #################
    mycase = caseoffset + 60 + 6
    myspwids = [1]
    mynchan = 5
    mystart = 4

    myexpectation = [10,11,12,13,14]

    if(isDesc):
        myexpectation = [19,18,17,16,15]
    testit()

    #################
    mycase = caseoffset + 60 + 7
    myspwids = [1]
    mynchan = 6
    mystart = 5

    myexpectation = [10,11,12,13,14,15]

    if(isDesc):
        myexpectation = [19,18,17,16,15,14]
    testit()
    #################
    #################
    mycase = caseoffset + 60 + 8
    myspwids = [0,1]
    mynchan = -1
    mystart = 18

    myexpectation = [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19]

    testit()

    #################
    mycase = caseoffset + 60 + 9
    myspwids = [0,1]
    mynchan = 10
    mystart = 9

    myexpectation = [1,2,3,4,5,6,7,8,9,10]

    testit()

    #################
    mycase = caseoffset + 60 + 10
    myspwids = [0,1]
    mynchan = 5
    mystart = 4

    myexpectation = [1,2,3,4,5]

    testit()

    #################
    mycase = caseoffset + 60 + 11
    myspwids = [0,1]
    mynchan = 6
    mystart = 5

    myexpectation = [1,2,3,4,5,6]

    testit()

    #################
    #################
    mycase = caseoffset + 60 + 12
    myspwids = [1,2]
    mynchan = -1
    mystart = 18

    myexpectation = [10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28]

    testit()

    #################
    mycase = caseoffset + 60 + 13
    myspwids = [1,2]
    mynchan = 10
    mystart = 9

    myexpectation = [10,11,12,13,14,15,16,17,18,19]

    testit()

    #################
    mycase = caseoffset + 60 + 14
    myspwids = [1,2]
    mynchan = 5
    mystart = 4

    myexpectation = [10,11,12,13,14]

    testit()

    #################
    mycase = caseoffset + 60 + 15
    myspwids = [1,2]
    mynchan = 6
    mystart = 5

    myexpectation = [10,11,12,13,14,15]

    testit()
    #################
    #################
    mycase = caseoffset + 60 + 16
    myspwids = [0,1,2]
    mynchan = -1
    mystart = 27

    myexpectation = [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28]

    testit()

    #################
    mycase = caseoffset + 60 + 17
    myspwids = [0,1,2]
    mynchan = 10
    mystart = 9

    myexpectation = [1,2,3,4,5,6,7,8,9,10]

    testit()

    #################
    mycase = caseoffset + 60 + 18
    myspwids = [0,1,2]
    mynchan = 5
    mystart = 4

    myexpectation = [1,2,3,4,5]

    testit()

    #################
    mycase = caseoffset + 60 + 19
    myspwids = [0,1,2]
    mynchan = 6
    mystart = 5

    myexpectation = [1,2,3,4,5,6]

    testit()

### end function testchannelmode() #########

def testvoptmode(caseoffset, isDesc):
    global mymode, mycase, myspwids, mynchan, mystart, mywidth, myveltype, myrestfrq, myexpectation, mytotal, myfailures
    global tests_to_do, failed

    # rest = (vopt/c + 1) * f 
    myrestfrq =  (-1000./299792458.0 + 1.) * (1E10 + 10.)  

    # mode = vopt
    mymode = 'velocity'
    myveltype = 'optical'

    ###################
    mystart = "-1000m/s"
    mywidth = "-1m/s"
    ###################
    
    mycase = caseoffset + 0
    myspwids = [0]
    mynchan = -1

    myexpectation = [-1000,-1001,-1002,-1003,-1004,-1005,-1006,-1007,-1008,-1009]

    if(isDesc):
        myexpectation.reverse()

    testitb()

    #################
    mycase = caseoffset + 1
    myspwids = [0]
    mynchan = 10

    myexpectation = [-1000,-1001,-1002,-1003,-1004,-1005,-1006,-1007,-1008,-1009]

    if(isDesc):
        myexpectation.reverse()

    testitb()

    #################
    mycase = caseoffset + 2
    myspwids = [0]
    mynchan = 5

    myexpectation = [-1000,-1001,-1002,-1003,-1004]

    if(isDesc):
        myexpectation.reverse()

    testitb()

    #################
    mycase = caseoffset + 3
    myspwids = [0]
    mynchan = 6

    myexpectation = [-1000,-1001,-1002,-1003,-1004,-1005]

    if(isDesc):
        myexpectation.reverse()

    testitb()

    #################
    #################
    mycase = caseoffset + 4
    myspwids = [1]
    mynchan = -1
    mystart = "-1010m/s"

    myexpectation = [-1010,-1011,-1012,-1013,-1014,-1015,-1016,-1017,-1018,-1019]
    if(isDesc):
        myexpectation.reverse()

    testitb()

    #################
    mycase = caseoffset + 5
    myspwids = [1]
    mynchan = 10

    myexpectation = [-1010,-1011,-1012,-1013,-1014,-1015,-1016,-1017,-1018,-1019]
    if(isDesc):
        myexpectation.reverse()

    testitb()

    #################
    mycase = caseoffset + 6
    myspwids = [1]
    mynchan = 5

    myexpectation = [-1010,-1011,-1012,-1013,-1014]
    if(isDesc):
        myexpectation.reverse()

    testitb()

    #################
    mycase = caseoffset + 7
    myspwids = [1]
    mynchan = 6

    myexpectation = [-1010,-1011,-1012,-1013,-1014,-1015]
    if(isDesc):
        myexpectation.reverse()

    testitb()
    #################
    #################
    mycase = caseoffset + 8
    myspwids = [0,1]
    mynchan = -1
    mystart = "-1000m/s"

    myexpectation = range(-1000,-1020,-1)

    testitb()

    #################
    mycase = caseoffset + 9
    myspwids = [0,1]
    mynchan = 10

    myexpectation = [-1000,-1001,-1002,-1003,-1004,-1005,-1006,-1007,-1008,-1009]

    testitb()

    #################
    mycase = caseoffset + 10
    myspwids = [0,1]
    mynchan = 5

    myexpectation = [-1000,-1001,-1002,-1003,-1004]

    testitb()

    #################
    mycase = caseoffset + 11
    myspwids = [0,1]
    mynchan = 6
    mystart = "-1009m/s"

    myexpectation = [-1009,-1010,-1011,-1012,-1013,-1014]

    testitb()

    #################
    #################
    mycase = caseoffset + 12
    myspwids = [1,2]
    mynchan = -1
    mystart = "-1010m/s"

    myexpectation = range(-1010,-1030,-1)

    testitb()

    #################
    mycase = caseoffset + 13
    myspwids = [1,2]
    mynchan = 10
    mystart = "-1010m/s"

    myexpectation = [-1010,-1011,-1012,-1013,-1014,-1015,-1016,-1017,-1018,-1019]

    testitb()

    #################
    mycase = caseoffset + 14
    myspwids = [1,2]
    mynchan = 5
    mystart = "-1010m/s"


    myexpectation = [-1010,-1011,-1012,-1013,-1014]

    testitb()

    #################
    mycase = caseoffset + 15
    myspwids = [1,2]
    mynchan = 6
    mystart = "-1010m/s"

    myexpectation = [-1010,-1011,-1012,-1013,-1014,-1015]

    testitb()
    #################
    #################
    mycase = caseoffset + 16
    myspwids = [0,1,2]
    mynchan = -1
    mystart = "-1000m/s"

    myexpectation = range(-1000,-1030,-1)

    testitb()

    #################
    mycase = caseoffset + 17
    myspwids = [0,1,2]
    mynchan = 10
    mystart = "-1000m/s"

    myexpectation = range(-1000,-1010,-1)

    testitb()

    #################
    mycase = caseoffset + 18
    myspwids = [0,1,2]
    mynchan = 5
    mystart = "-1020m/s"

    myexpectation = [-1020,-1021,-1022,-1023,-1024]

    testitb()

    #################
    mycase = caseoffset + 19
    myspwids = [0,1,2]
    mynchan = 6
    mystart = "-1005m/s"
    mywidth = "1m/s"

    myexpectation = [-1000,-1001,-1002,-1003,-1004,-1005]

    testitb()

    #################
    mycase = caseoffset + 20
    myspwids = [0,1,2]
    mynchan = 6
    mystart = "-1018m/s"
    mywidth = "2m/s"

    myexpectation = [-1008,-1010,-1012,-1014,-1016,-1018]

    testitb()


### end function testvoptmode() #########

def testfreqmode(caseoffset, isDesc):
    global mymode, mycase, myspwids, mynchan, mystart, mywidth, myexpectation, mytotal, myfailures
    global tests_to_do, failed

    mymode = 'frequency'
    mycase = caseoffset + 0
    myspwids = [0]
    mynchan = -1
    mystart = 1
    mywidth = 1
    myexpectation = [1,2,3,4,5,6,7,8,9,10]
    if(isDesc):
        myexpectation.reverse()
    testit()

    #################
    mycase = caseoffset + 1
    myspwids = [0]
    mynchan = 10

    myexpectation = [1,2,3,4,5,6,7,8,9,10]

    if(isDesc):
        myexpectation.reverse()
    testit()

    #################
    mycase = caseoffset + 2
    myspwids = [0]
    mynchan = 5

    myexpectation = [1,2,3,4,5]

    if(isDesc):
        myexpectation.reverse()
    testit()

    #################
    mycase = caseoffset + 3
    myspwids = [0]
    mynchan = 6

    myexpectation = [1,2,3,4,5,6]

    if(isDesc):
        myexpectation.reverse()
    testit()

    #################
    #################
    mycase = caseoffset + 4
    myspwids = [1]
    mynchan = -1
    mystart = 10

    myexpectation = [10,11,12,13,14,15,16,17,18,19]

    if(isDesc):
        myexpectation.reverse()
    testit()

    #################
    mycase = caseoffset + 5
    myspwids = [1]
    mynchan = 10
    mystart = 10
    mywidth = 1
    myexpectation = [10,11,12,13,14,15,16,17,18,19]

    if(isDesc):
        myexpectation.reverse()
    testit()

    #################
    mycase = caseoffset + 6
    myspwids = [1]
    mynchan = 5
    mystart = 10
    mywidth = 1
    myexpectation = [10,11,12,13,14]

    if(isDesc):
        myexpectation.reverse()
    testit()

    #################
    mycase = caseoffset + 7
    myspwids = [1]
    mynchan = 6
    mystart = 10
    mywidth = 1
    myexpectation = [10,11,12,13,14,15]

    if(isDesc):
        myexpectation.reverse()
    testit()
    #################
    #################
    mycase = caseoffset + 8
    myspwids = [0,1]
    mynchan = -1
    mystart = 1
    mywidth = 1
    myexpectation = [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19]

    testit()

    #################
    mycase = caseoffset + 9
    myspwids = [0,1]
    mynchan = 10
    mystart = 1
    mywidth = 1
    myexpectation = [1,2,3,4,5,6,7,8,9,10]

    testit()

    #################
    mycase = caseoffset + 10
    myspwids = [0,1]
    mynchan = 5
    mystart = 1
    mywidth = 1
    myexpectation = [1,2,3,4,5]

    testit()

    #################
    mycase = caseoffset + 11
    myspwids = [0,1]
    mynchan = 6
    mystart = 1
    mywidth = 1
    myexpectation = [1,2,3,4,5,6]

    testit()

    #################
    #################
    mycase = caseoffset + 12
    myspwids = [1,2]
    mynchan = -1
    mystart = 10
    mywidth = 1
    myexpectation = [10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28]

    testit()

    #################
    mycase = caseoffset + 13
    myspwids = [1,2]
    mynchan = 10
    mystart = 10
    mywidth = 1
    myexpectation = [10,11,12,13,14,15,16,17,18,19]

    testit()

    #################
    mycase = caseoffset + 14
    myspwids = [1,2]
    mynchan = 5
    mystart = 10
    mywidth = 1
    myexpectation = [10,11,12,13,14]

    testit()

    #################
    mycase = caseoffset + 15
    myspwids = [1,2]
    mynchan = 6
    mystart = 10
    mywidth = 1
    myexpectation = [10,11,12,13,14,15]

    testit()
    #################
    #################
    mycase = caseoffset + 16
    myspwids = [0,1,2]
    mynchan = -1
    mystart = 1
    mywidth = 1
    myexpectation = [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28]

    testit()

    #################
    mycase = caseoffset + 17
    myspwids = [0,1,2]
    mynchan = 10
    mystart = 1
    mywidth = 1
    myexpectation = [1,2,3,4,5,6,7,8,9,10]

    testit()

    #################
    mycase = caseoffset + 18
    myspwids = [0,1,2]
    mynchan = 5
    mystart = 1
    mywidth = 1
    myexpectation = [1,2,3,4,5]

    testit()

    #################
    mycase = caseoffset + 19
    myspwids = [0,1,2]
    mynchan = 6
    mystart = 1
    mywidth = 1
    myexpectation = [1,2,3,4,5,6]

    testit()

    #################
    mycase = caseoffset + 20
    myspwids = [0]
    mynchan = -1
    mystart = 2

    mywidth = 1
    myexpectation = [2,3,4,5,6,7,8,9,10]

    if(isDesc):
        myexpectation.reverse()
    testit()

    #################
    mycase = caseoffset + 21
    myspwids = [0]
    mynchan = 10
    mystart = 2

    mywidth = 1
    myexpectation = [2,3,4,5,6,7,8,9,10,11] # feature 

    if(isDesc):
        myexpectation.reverse()
    testit()

    #################
    mycase = caseoffset + 22
    myspwids = [0]
    mynchan = 3
    mystart = 2

    mywidth = 2
    myexpectation = [2,4,6]

    if(isDesc):
        myexpectation.reverse()
    testit()


### end function testfreqmode() #########

def testvradmode(caseoffset, isDesc):
    global mymode, mycase, myspwids, mynchan, mystart, mywidth, myveltype, myrestfrq, myexpectation, mytotal, myfailures
    global tests_to_do, failed

    # vrad = c * ( 1 - f/r)
    # r = f/(1. - vrad/c)
    myrestfrq = (1E10 + 10.) / (1 - (-1000./299792458.0))

    # mode = vopt
    mymode = 'velocity'
    myveltype = 'radio'

    ###################
    mystart = "-1000m/s"
    mywidth = "-1m/s"
    ###################
    
    mycase = caseoffset + 0
    myspwids = [0]
    mynchan = -1

    myexpectation = [-1000,-1001,-1002,-1003,-1004,-1005,-1006,-1007,-1008,-1009]

    if(isDesc):
        myexpectation.reverse()

    testitc()

    #################
    mycase = caseoffset + 1
    myspwids = [0]
    mynchan = 10

    myexpectation = [-1000,-1001,-1002,-1003,-1004,-1005,-1006,-1007,-1008,-1009]

    if(isDesc):
        myexpectation.reverse()

    testitc()

    #################
    mycase = caseoffset + 2
    myspwids = [0]
    mynchan = 5

    myexpectation = [-1000,-1001,-1002,-1003,-1004]

    if(isDesc):
        myexpectation.reverse()

    testitc()

    #################
    mycase = caseoffset + 3
    myspwids = [0]
    mynchan = 6

    myexpectation = [-1000,-1001,-1002,-1003,-1004,-1005]

    if(isDesc):
        myexpectation.reverse()

    testitc()

    #################
    #################
    mycase = caseoffset + 4
    myspwids = [1]
    mynchan = -1
    mystart = "-1010m/s"

    myexpectation = [-1010,-1011,-1012,-1013,-1014,-1015,-1016,-1017,-1018,-1019]
    if(isDesc):
        myexpectation.reverse()

    testitc()

    #################
    mycase = caseoffset + 5
    myspwids = [1]
    mynchan = 10

    myexpectation = [-1010,-1011,-1012,-1013,-1014,-1015,-1016,-1017,-1018,-1019]
    if(isDesc):
        myexpectation.reverse()

    testitc()

    #################
    mycase = caseoffset + 6
    myspwids = [1]
    mynchan = 5

    myexpectation = [-1010,-1011,-1012,-1013,-1014]
    if(isDesc):
        myexpectation.reverse()

    testitc()

    #################
    mycase = caseoffset + 7
    myspwids = [1]
    mynchan = 6

    myexpectation = [-1010,-1011,-1012,-1013,-1014,-1015]
    if(isDesc):
        myexpectation.reverse()

    testitc()
    #################
    #################
    mycase = caseoffset + 8
    myspwids = [0,1]
    mynchan = -1
    mystart = "-1000m/s"

    myexpectation = range(-1000,-1020,-1)

    testitc()

    #################
    mycase = caseoffset + 9
    myspwids = [0,1]
    mynchan = 10

    myexpectation = [-1000,-1001,-1002,-1003,-1004,-1005,-1006,-1007,-1008,-1009]

    testitc()

    #################
    mycase = caseoffset + 10
    myspwids = [0,1]
    mynchan = 5

    myexpectation = [-1000,-1001,-1002,-1003,-1004]

    testitc()

    #################
    mycase = caseoffset + 11
    myspwids = [0,1]
    mynchan = 6
    mystart = "-1009m/s"

    myexpectation = [-1009,-1010,-1011,-1012,-1013,-1014]

    testitc()

    #################
    #################
    mycase = caseoffset + 12
    myspwids = [1,2]
    mynchan = -1
    mystart = "-1010m/s"

    myexpectation = range(-1010,-1030,-1)

    testitc()

    #################
    mycase = caseoffset + 13
    myspwids = [1,2]
    mynchan = 10
    mystart = "-1010m/s"

    myexpectation = [-1010,-1011,-1012,-1013,-1014,-1015,-1016,-1017,-1018,-1019]

    testitc()

    #################
    mycase = caseoffset + 14
    myspwids = [1,2]
    mynchan = 5
    mystart = "-1010m/s"


    myexpectation = [-1010,-1011,-1012,-1013,-1014]

    testitc()

    #################
    mycase = caseoffset + 15
    myspwids = [1,2]
    mynchan = 6
    mystart = "-1010m/s"

    myexpectation = [-1010,-1011,-1012,-1013,-1014,-1015]

    testitc()
    #################
    #################
    mycase = caseoffset + 16
    myspwids = [0,1,2]
    mynchan = -1
    mystart = "-1000m/s"

    myexpectation = range(-1000,-1030,-1)

    testitc()

    #################
    mycase = caseoffset + 17
    myspwids = [0,1,2]
    mynchan = 10
    mystart = "-1000m/s"

    myexpectation = range(-1000,-1010,-1)

    testitc()

    #################
    mycase = caseoffset + 18
    myspwids = [0,1,2]
    mynchan = 5
    mystart = "-1020m/s"

    myexpectation = [-1020,-1021,-1022,-1023,-1024]

    testitc()

    #################
    mycase = caseoffset + 19
    myspwids = [0,1,2]
    mynchan = 6
    mystart = "-1005m/s"
    mywidth = "1m/s"

    myexpectation = [-1000,-1001,-1002,-1003,-1004,-1005]

    testitc()

    #################
    mycase = caseoffset + 20
    myspwids = [0,1,2]
    mynchan = 6
    mystart = "-1018m/s"
    mywidth = "2m/s"

    myexpectation = [-1008,-1010,-1012,-1014,-1016,-1018]

    testitc()

    #################
    mystart = "-1004m/s"
    mywidth = ""
    ###################
    
    mycase = caseoffset + 21
    myspwids = [0]
    mynchan = 5

    myexpectation = [-1000,-1001,-1002,-1003,-1004]

    if(isDesc):
        myexpectation.reverse()

    testitc()


### end function testvradmode() #########


class cvelfreqs_test(unittest.TestCase):

    def setUp(self):

        # In order to run individual test cases, comment out the following line
        # and set the variable tests_to_do to the list of cases before calling this script

        if(not os.path.exists('test_uvcontsub2.ms')):
            print "Copying test data ..."
            os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/unittest/cvel/test_uvcontsub2.ms .')
            
    def tearDown(self):
        os.system('rm -rf sample.ms sample2.ms sample3.ms sample4.ms sampler.ms')   
        pass

    def test1(self):

        '''cvelfreqs 1: test ascending frequencies, channel mode'''

        global mytotal, myfailures, tests_to_do, failed

        os.system('rm -rf sample.ms')
        shutil.copytree('test_uvcontsub2.ms','sample.ms')
        tb.open('sample.ms/SPECTRAL_WINDOW', nomodify=False)
        # spw 0
        spwid = 0
        newnumchan = 10
        newchanfreq = range(1,newnumchan+1) # i.e. [1,2,3,...,10]
        cw = newchanfreq[1] - newchanfreq[0]
        newchanwidth = []
        for i in range(0,newnumchan):
            newchanwidth.append(cw)
            
        print spwid,': ', newchanfreq
        print '    ', newchanwidth
            
        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        # spw 1
        spwid = 1
        newnumchan = 10
        newchanfreq = range(10,newnumchan+1+9) # i.e. [10,11,...,19]
        cw = newchanfreq[1] - newchanfreq[0]
        newchanwidth = []
        for i in range(0,newnumchan):
            newchanwidth.append(cw)

        print spwid,': ', newchanfreq
        print '    ', newchanwidth

        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        # spw 2
        spwid = 2
        newnumchan = 10
        newchanfreq = range(19,newnumchan+1+18) # i.e. [19,20,...,28]
        cw = newchanfreq[1] - newchanfreq[0]
        newchanwidth = []
        for i in range(0,newnumchan):
            newchanwidth.append(cw)

        print spwid,': ', newchanfreq
        print '    ', newchanwidth

        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        tb.close()    

        ##############################
        
        ms.open('sample.ms')

        mytotal = 0
        myfailures = 0
        failed = []
        
        testchannelmode(0,False) # start counting at case 0, non-descending frequencies as input
        
        ms.close()

        print myfailures, " failures in ", mytotal, " subtests."
        if(myfailures>0):
            print "Failed cases: ", failed

        self.assertEqual(myfailures,0)

    def test2(self):

        '''cvelfreqs 2: test descending frequencies, channel mode'''

        global mytotal, myfailures, tests_to_do, failed

        os.system('rm -rf sample-desc.ms')
        shutil.copytree('test_uvcontsub2.ms','sample-desc.ms')
        tb.open('sample-desc.ms/SPECTRAL_WINDOW', nomodify=False)
        # spw 0
        spwid = 0
        newnumchan = 10
        newchanfreq = range(newnumchan,0,-1) # i.e. [10,9,8,...,1]
        cw = newchanfreq[1] - newchanfreq[0]
        newchanwidth = []
        newabschanwidth = []
        for i in range(0,newnumchan):
            newchanwidth.append(cw)
            newabschanwidth.append(cw)

        print spwid,': ', newchanfreq
        print '    ', newchanwidth

        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newabschanwidth)
        tb.putcell('RESOLUTION', spwid, newabschanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        # spw 1
        spwid = 1
        newnumchan = 10
        newchanfreq = range(newnumchan+9,9,-1) # i.e. [19,...,10]
        cw = newchanfreq[1] - newchanfreq[0]
        newchanwidth = []
        newabschanwidth = []
        for i in range(0,newnumchan):
            newchanwidth.append(cw)
            newabschanwidth.append(cw)

        print spwid,': ', newchanfreq
        print '    ', newchanwidth

        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newabschanwidth)
        tb.putcell('RESOLUTION', spwid, newabschanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        # spw 2
        spwid = 2
        newnumchan = 10
        newchanfreq = range(newnumchan+18,18,-1) # i.e. [28,...,19]
        cw = newchanfreq[1] - newchanfreq[0]
        newchanwidth = []
        newabschanwidth = []
        for i in range(0,newnumchan):
            newchanwidth.append(cw)
            newabschanwidth.append(cw)

        print spwid,': ', newchanfreq
        print '    ', newchanwidth

        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newabschanwidth)
        tb.putcell('RESOLUTION', spwid, newabschanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        tb.close()    

        ####################

        ms.open('sample-desc.ms')

        mytotal = 0
        myfailures = 0
        failed = []

        testchannelmode(1000, True) # start counting at case 1000, descending freqs as input

        ms.close()

        print myfailures, " failures in ", mytotal, " subtests."
        if(myfailures>0):
            print "Failed cases: ", failed

        self.assertEqual(myfailures,0)

    def test3(self):

        '''cvelfreqs 3: test ascending frequencies, optical velocity mode'''

        global mytotal, myfailures, tests_to_do, failed

        os.system('rm -rf sample3.ms')
        shutil.copytree('test_uvcontsub2.ms','sample3.ms')
        tb.open('sample3.ms/SPECTRAL_WINDOW', nomodify=False)
        # spw 0
        spwid = 0
        newnumchan = 11
        newchanfreq = []
        cw = 33.
        newchanwidth = []
        for i in range(0,newnumchan):
            newchanfreq.append(1E10+i*cw)
            newchanwidth.append(cw)
            
        print spwid,': ', newchanfreq
        print '    ', newchanwidth
            
        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        lastfreq = newchanfreq[newnumchan-1]

        # spw 1
        spwid = 1
        newnumchan = 11
        newchanfreq = []
        cw = 33.
        newchanwidth = []
        for i in range(0,newnumchan):
            newchanfreq.append(lastfreq+i*cw)
            newchanwidth.append(cw)
            
        print spwid,': ', newchanfreq
        print '    ', newchanwidth
            
        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        lastfreq = newchanfreq[newnumchan-1]

        # spw 2
        spwid = 2
        newnumchan = 11
        newchanfreq = []
        cw = 33.
        newchanwidth = []
        for i in range(0,newnumchan):
            newchanfreq.append(lastfreq+i*cw)
            newchanwidth.append(cw)
            
        print spwid,': ', newchanfreq
        print '    ', newchanwidth
            
        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        tb.close()    

        ##############################
        
        ms.open('sample3.ms')

        mytotal = 0
        myfailures = 0
        failed = []
        
        testvoptmode(2000,False) # start counting at case 2000, non-descending frequencies as input
        
        ms.close()

        print myfailures, " failures in ", mytotal, " subtests."
        if(myfailures>0):
            print "Failed cases: ", failed

        self.assertEqual(myfailures,0)

    def test4(self):

        '''cvelfreqs 4: test descending frequencies, optical velocity mode'''

        global mytotal, myfailures, tests_to_do, failed

        os.system('rm -rf sample4.ms')
        shutil.copytree('test_uvcontsub2.ms','sample4.ms')
        tb.open('sample4.ms/SPECTRAL_WINDOW', nomodify=False)
        # spw 0
        spwid = 0
        newnumchan = 11
        newchanfreq = []
        cw = 33.
        newchanwidth = []
        for i in range(0,newnumchan):
            newchanfreq.append(1E10+i*cw)
            newchanwidth.append(-cw)

        newchanfreq.reverse()
            
        print spwid,': ', newchanfreq
        print '    ', newchanwidth
            
        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        lastfreq = newchanfreq[0]

        # spw 1
        spwid = 1
        newnumchan = 11
        newchanfreq = []
        cw = 33.
        newchanwidth = []
        for i in range(0,newnumchan):
            newchanfreq.append(lastfreq+i*cw)
            newchanwidth.append(-cw)

        newchanfreq.reverse()
            
        print spwid,': ', newchanfreq
        print '    ', newchanwidth
            
        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        lastfreq = newchanfreq[0]

        # spw 2
        spwid = 2
        newnumchan = 11
        newchanfreq = []
        cw = 33.
        newchanwidth = []
        for i in range(0,newnumchan):
            newchanfreq.append(lastfreq+i*cw)
            newchanwidth.append(-cw)
            
        newchanfreq.reverse()
            
        print spwid,': ', newchanfreq
        print '    ', newchanwidth
            
        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        tb.close()    

        ##############################
        
        ms.open('sample4.ms')

        mytotal = 0
        myfailures = 0
        failed = []
        
        testvoptmode(3000,True) # start counting at case 3000, descending frequencies as input
        
        ms.close()

        print myfailures, " failures in ", mytotal, " subtests."
        if(myfailures>0):
            print "Failed cases: ", failed

        self.assertEqual(myfailures,0)


    def test5(self):

        '''cvelfreqs 5: test ascending frequencies, frequency mode'''

        global mytotal, myfailures, tests_to_do, failed

        os.system('rm -rf sample.ms')
        shutil.copytree('test_uvcontsub2.ms','sample.ms')
        tb.open('sample.ms/SPECTRAL_WINDOW', nomodify=False)
        # spw 0
        spwid = 0
        newnumchan = 10
        newchanfreq = range(1,newnumchan+1) # i.e. [1,2,3,...,10]
        cw = newchanfreq[1] - newchanfreq[0]
        newchanwidth = []
        for i in range(0,newnumchan):
            newchanwidth.append(cw)
            
        print spwid,': ', newchanfreq
        print '    ', newchanwidth
            
        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        # spw 1
        spwid = 1
        newnumchan = 10
        newchanfreq = range(10,newnumchan+1+9) # i.e. [10,11,...,19]
        cw = newchanfreq[1] - newchanfreq[0]
        newchanwidth = []
        for i in range(0,newnumchan):
            newchanwidth.append(cw)

        print spwid,': ', newchanfreq
        print '    ', newchanwidth

        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        # spw 2
        spwid = 2
        newnumchan = 10
        newchanfreq = range(19,newnumchan+1+18) # i.e. [19,20,...,28]
        cw = newchanfreq[1] - newchanfreq[0]
        newchanwidth = []
        for i in range(0,newnumchan):
            newchanwidth.append(cw)

        print spwid,': ', newchanfreq
        print '    ', newchanwidth

        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        tb.close()    

        ##############################
        
        ms.open('sample.ms')

        mytotal = 0
        myfailures = 0
        failed = []
        
        testfreqmode(4000,False) # start counting at case 0, non-descending frequencies as input
        
        ms.close()

        print myfailures, " failures in ", mytotal, " subtests."
        if(myfailures>0):
            print "Failed cases: ", failed

        self.assertEqual(myfailures,0)

    def test6(self):

        '''cvelfreqs 6: test descending frequencies, frequency mode'''

        global mytotal, myfailures, tests_to_do, failed

        os.system('rm -rf sample-desc.ms')
        shutil.copytree('test_uvcontsub2.ms','sample-desc.ms')
        tb.open('sample-desc.ms/SPECTRAL_WINDOW', nomodify=False)
        # spw 0
        spwid = 0
        newnumchan = 10
        newchanfreq = range(newnumchan,0,-1) # i.e. [10,9,8,...,1]
        cw = newchanfreq[1] - newchanfreq[0]
        newchanwidth = []
        newabschanwidth = []
        for i in range(0,newnumchan):
            newchanwidth.append(cw)
            newabschanwidth.append(cw)

        print spwid,': ', newchanfreq
        print '    ', newchanwidth

        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newabschanwidth)
        tb.putcell('RESOLUTION', spwid, newabschanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        # spw 1
        spwid = 1
        newnumchan = 10
        newchanfreq = range(newnumchan+9,9,-1) # i.e. [19,...,10]
        cw = newchanfreq[1] - newchanfreq[0]
        newchanwidth = []
        newabschanwidth = []
        for i in range(0,newnumchan):
            newchanwidth.append(cw)
            newabschanwidth.append(cw)

        print spwid,': ', newchanfreq
        print '    ', newchanwidth

        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newabschanwidth)
        tb.putcell('RESOLUTION', spwid, newabschanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        # spw 2
        spwid = 2
        newnumchan = 10
        newchanfreq = range(newnumchan+18,18,-1) # i.e. [28,...,19]
        cw = newchanfreq[1] - newchanfreq[0]
        newchanwidth = []
        newabschanwidth = []
        for i in range(0,newnumchan):
            newchanwidth.append(cw)
            newabschanwidth.append(cw)

        print spwid,': ', newchanfreq
        print '    ', newchanwidth

        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newabschanwidth)
        tb.putcell('RESOLUTION', spwid, newabschanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        tb.close()    

        ####################

        ms.open('sample-desc.ms')

        mytotal = 0
        myfailures = 0
        failed = []

        testfreqmode(5000, True) # start counting at case 5000, descending freqs as input

        ms.close()

        print myfailures, " failures in ", mytotal, " subtests."
        if(myfailures>0):
            print "Failed cases: ", failed

        self.assertEqual(myfailures,0)

    def test7(self):

        '''cvelfreqs 7: test ascending frequencies, radio velocity mode'''

        global mytotal, myfailures, tests_to_do, failed

        os.system('rm -rf sample3.ms')
        shutil.copytree('test_uvcontsub2.ms','sample3.ms')
        tb.open('sample3.ms/SPECTRAL_WINDOW', nomodify=False)
        # spw 0
        spwid = 0
        newnumchan = 11
        newchanfreq = []
        cw = 33.
        newchanwidth = []
        for i in range(0,newnumchan):
            newchanfreq.append(1E10+i*cw)
            newchanwidth.append(cw)
            
        print spwid,': ', newchanfreq
        print '    ', newchanwidth
            
        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        lastfreq = newchanfreq[newnumchan-1]

        # spw 1
        spwid = 1
        newnumchan = 11
        newchanfreq = []
        cw = 33.
        newchanwidth = []
        for i in range(0,newnumchan):
            newchanfreq.append(lastfreq+i*cw)
            newchanwidth.append(cw)
            
        print spwid,': ', newchanfreq
        print '    ', newchanwidth
            
        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        lastfreq = newchanfreq[newnumchan-1]

        # spw 2
        spwid = 2
        newnumchan = 11
        newchanfreq = []
        cw = 33.
        newchanwidth = []
        for i in range(0,newnumchan):
            newchanfreq.append(lastfreq+i*cw)
            newchanwidth.append(cw)
            
        print spwid,': ', newchanfreq
        print '    ', newchanwidth
            
        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        tb.close()    

        ##############################
        
        ms.open('sample3.ms')

        mytotal = 0
        myfailures = 0
        failed = []
        
        testvradmode(6000,False) # start counting at case 6000, non-descending frequencies as input
        
        ms.close()

        print myfailures, " failures in ", mytotal, " subtests."
        if(myfailures>0):
            print "Failed cases: ", failed

        self.assertEqual(myfailures,0)

    def test8(self):

        '''cvelfreqs 8: test descending frequencies, radio velocity mode'''

        global mytotal, myfailures, tests_to_do, failed

        os.system('rm -rf sample4.ms')
        shutil.copytree('test_uvcontsub2.ms','sample4.ms')
        tb.open('sample4.ms/SPECTRAL_WINDOW', nomodify=False)
        # spw 0
        spwid = 0
        newnumchan = 11
        newchanfreq = []
        cw = 33.
        newchanwidth = []
        for i in range(0,newnumchan):
            newchanfreq.append(1E10+i*cw)
            newchanwidth.append(-cw)

        newchanfreq.reverse()
            
        print spwid,': ', newchanfreq
        print '    ', newchanwidth
            
        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        lastfreq = newchanfreq[0]

        # spw 1
        spwid = 1
        newnumchan = 11
        newchanfreq = []
        cw = 33.
        newchanwidth = []
        for i in range(0,newnumchan):
            newchanfreq.append(lastfreq+i*cw)
            newchanwidth.append(-cw)

        newchanfreq.reverse()
            
        print spwid,': ', newchanfreq
        print '    ', newchanwidth
            
        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        lastfreq = newchanfreq[0]

        # spw 2
        spwid = 2
        newnumchan = 11
        newchanfreq = []
        cw = 33.
        newchanwidth = []
        for i in range(0,newnumchan):
            newchanfreq.append(lastfreq+i*cw)
            newchanwidth.append(-cw)
            
        newchanfreq.reverse()
            
        print spwid,': ', newchanfreq
        print '    ', newchanwidth
            
        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        tb.close()    

        ##############################
        
        ms.open('sample4.ms')

        mytotal = 0
        myfailures = 0
        failed = []
        
        testvradmode(7000,True) # start counting at case 7000, descending frequencies as input
        
        ms.close()

        print myfailures, " failures in ", mytotal, " subtests."
        if(myfailures>0):
            print "Failed cases: ", failed

        self.assertEqual(myfailures,0)

    def test9(self):

        '''cvelfreqs 9: test reproducibility'''

        global mytotal, myfailures, tests_to_do, failed

        os.system('rm -rf sampler.ms')
        shutil.copytree('test_uvcontsub2.ms','sampler.ms')
        
        tb.open('sampler.ms/SPECTRAL_WINDOW', nomodify=False)
        # spw 0
        spwid = 0
        newnumchan = 11
        newchanfreq = []
        cw = 33.
        newchanwidth = []
        for i in range(0,newnumchan):
            newchanfreq.append(1E10+i*cw)
            newchanwidth.append(cw)
            
        print spwid,': ', newchanfreq
        print '    ', newchanwidth
            
        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        lastfreq = newchanfreq[newnumchan-1]

        # spw 1
        spwid = 1
        newnumchan = 11
        newchanfreq = []
        cw = 33.
        newchanwidth = []
        for i in range(0,newnumchan):
            newchanfreq.append(lastfreq-cw/2.+i*cw) # intentional misalignment by half a channel
            newchanwidth.append(cw)
            
        print spwid,': ', newchanfreq
        print '    ', newchanwidth
            
        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        lastfreq = newchanfreq[newnumchan-1]

        # spw 2
        spwid = 2
        newnumchan = 11
        newchanfreq = []
        cw = 33.
        newchanwidth = []
        for i in range(0,newnumchan):
            newchanfreq.append(lastfreq - 0.3*cw +i*cw) # intentional misalignment by 0.3 channelwidths
            newchanwidth.append(cw)
            
        print spwid,': ', newchanfreq
        print '    ', newchanwidth
            
        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]))

        tb.close()    

        ##############################

        ms.open('sampler.ms')

        channelfreqs = ms.cvelfreqs(mode='channel', spwids=[0,1,2], start=1, width=2, nchan=10)
        channelfreqs = list(channelfreqs)
        print "channel freqs ", channelfreqs 
        frequencyfreqs = ms.cvelfreqs(mode='frequency', spwids=[0,1,2], start=str(1E9+60)+'Hz', width='100Hz', nchan=10)
        frequencyfreqs = list(frequencyfreqs)
        print "frequency freqs ", frequencyfreqs
        restfrqo =  (-1000./299792458.0 + 1.) * (1E10 + 10.)
        opticalfreqs = ms.cvelfreqs(mode='velocity', veltype='optical', spwids=[0,1,2], start="-1001m/s", width='-2m/s', nchan=10, restfreq=restfrqo)
        opticalfreqs = list(opticalfreqs)
        print "optical freqs ", opticalfreqs
        restfrqr = (1E10 + 10.) / (1 - (-1000./299792458.0))
        radiofreqs  = ms.cvelfreqs(mode='velocity', veltype='radio', spwids=[0,1,2], start="-1001m/s", width='-2m/s', nchan=10, restfreq=restfrqr)
        radiofreqs = list(radiofreqs)
        print "radio freqs ", radiofreqs

        ms.close()
        
        tb.open('sampler.ms/SPECTRAL_WINDOW', nomodify=False)
        # spw 3, for channel mode
        spwid = 3
        newnumchan = 10
        newchanfreq = channelfreqs
        newchanwidth = []
        for i in range(0,newnumchan-1):
            newchanwidth.append(newchanfreq[i+1]-newchanfreq[i])
        newchanwidth.append(newchanfreq[newnumchan-1]-newchanfreq[newnumchan-2])
            
        print spwid,': ', newchanfreq
        print '    ', newchanwidth
            
        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]/2.)+abs(newchanwidth[-1]/2.))

        # spw 4, for frequency mode
        spwid = 4
        newnumchan = 10
        newchanfreq = frequencyfreqs
        newchanwidth = []
        for i in range(0,newnumchan-1):
            newchanwidth.append(newchanfreq[i+1]-newchanfreq[i])
        newchanwidth.append(newchanfreq[newnumchan-1]-newchanfreq[newnumchan-2])
            
        print spwid,': ', newchanfreq
        print '    ', newchanwidth
            
        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]/2.)+abs(newchanwidth[-1]/2.))

        # spw 5, for optical velocity mode
        spwid = 5
        newnumchan = 10
        newchanfreq = opticalfreqs
        newchanwidth = []
        for i in range(0,newnumchan-1):
            newchanwidth.append(newchanfreq[i+1]-newchanfreq[i])
        newchanwidth.append(newchanfreq[newnumchan-1]-newchanfreq[newnumchan-2])
            
        print spwid,': ', newchanfreq
        print '    ', newchanwidth
            
        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]/2.)+abs(newchanwidth[-1]/2.))

        # spw 6, for radio velocity mode
        spwid = 6
        newnumchan = 10
        newchanfreq = radiofreqs
        newchanwidth = []
        for i in range(0,newnumchan-1):
            newchanwidth.append(newchanfreq[i+1]-newchanfreq[i])
        newchanwidth.append(newchanfreq[newnumchan-1]-newchanfreq[newnumchan-2])
            
        print spwid,': ', newchanfreq
        print '    ', newchanwidth
            
        tb.putcell('NUM_CHAN', spwid, newnumchan)
        tb.putcell('CHAN_FREQ', spwid, newchanfreq)
        tb.putcell('CHAN_WIDTH', spwid, newchanwidth)
        tb.putcell('EFFECTIVE_BW', spwid, newchanwidth)
        tb.putcell('RESOLUTION', spwid, newchanwidth)
        tb.putcell('REF_FREQUENCY', spwid, newchanfreq[0])
        tb.putcell('TOTAL_BANDWIDTH', spwid, abs(newchanfreq[-1]-newchanfreq[0])+abs(newchanwidth[0]/2.)+abs(newchanwidth[-1]/2.))

        tb.close()

        ##########################

        ms.open('sampler.ms')

        mytotal = 0
        myfailures = 0
        
        newchannelfreqs = ms.cvelfreqs(mode='channel', spwids=[3])
        newchannelfreqs = list(newchannelfreqs)
        
        newfrequencyfreqs = ms.cvelfreqs(mode='frequency', spwids=[4], start=str(1E9+60)+'Hz', width='100Hz', nchan=10)
        newfrequencyfreqs = list(newfrequencyfreqs)
        
        newopticalfreqs = ms.cvelfreqs(mode='velocity', veltype='optical', spwids=[5], start="-1001m/s", width='-2m/s', nchan=10, restfreq=restfrqo)
        newopticalfreqs = list(newopticalfreqs)
        newradiofreqs  = ms.cvelfreqs(mode='velocity', veltype='radio', spwids=[6], start="-1001m/s", width='-2m/s', nchan=10, restfreq=restfrqr)
        newradiofreqs = list(newradiofreqs)

        ms.close()

        if not (newchannelfreqs == channelfreqs):
            print "channel output deviates: "
            print newchannelfreqs
            print "  expected was:"
            print channelfreqs
            myfailures = myfailures + 1
        mytotal = mytotal + 1
        if not (newfrequencyfreqs == frequencyfreqs):
            print "frequency output deviates: "
            print newfrequencyfreqs
            print "  expected was:"
            print frequencyfreqs
            myfailures = myfailures + 1
        mytotal = mytotal + 1
        if not (newopticalfreqs == opticalfreqs):
            print "optical output deviates: "
            print newopticalfreqs
            print "  expected was:"
            print opticalfreqs
            myfailures = myfailures + 1
        mytotal = mytotal + 1
        if not (newradiofreqs == radiofreqs):
            print "radio output deviates: "
            print newradiofreqs
            print "  expected was:"
            print radiofreqs
            myfailures = myfailures + 1
        mytotal = mytotal + 1

        print myfailures, " failures in ", mytotal, " subtests."

        self.assertEqual(myfailures,0)



class cleanup(unittest.TestCase):
    def setUp(self):
        pass
    
    def tearDown(self):
        # It will ignore errors in case files don't exist
        shutil.rmtree('test_uvcontsub2.ms',ignore_errors=True)
        
    def test_cleanup(self):
        '''Cvelfreqs: Cleanup'''
        pass


def suite():
    return [cvelfreqs_test,cleanup]
