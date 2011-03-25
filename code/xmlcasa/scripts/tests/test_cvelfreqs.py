# Unit test for ms.cvelfreqs()

# In order to run individual test cases, comment out the following line
tests_to_do = []
# and set the variable tests_to_do to the list of cases before calling this script

if(not os.path.exists('test_uvcontsub2.ms')):
    print "Copying test data ..."
    os.system('cp -R '+os.environ['CASAPATH'].split()[0]+'/data/regression/unittest/uvcontsub2/test_uvcontsub2.ms .')

# Test Matrix
# mode = 'channel', 'frequency', 'velocity'
# spwids = [0], [1], [0,1], [1,2], [0,1,2]
# nchan = -1, 10, 5, 6
# start = 0, 1, 15 (equivalent freqs and velos)
# width = 1, 2, 3, -1, -2, -3 (equivalent freqs and velos)
# ascending and descending input
# number of cases: 3 x 5 x 4 x 3 x 6 x 2 = 2160 !

# Test Matrix covered so far:
# mode = 'channel'
# spwids = [0], [1], [0,1], [1,2], [0,1,2]
# nchan = -1, 10, 5, 6
# start = 0, 1, 15 (equivalent freqs and velos)
# width = 1, -1 (equivalent freqs and velos)
# ascending and descending


def testit():
    global mymode, mycase, myspwids, mynchan, mystart, mywidth, myexpectation, mytotal, myfailures
    global tests_to_do, failed
    if (mycase in tests_to_do) or (tests_to_do==[]):
        try:
            outf = ms.cvelfreqs(spwids=myspwids,mode=mymode,nchan=mynchan,start=mystart,width=mywidth)
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


# ascending frequencies #########

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

###############################
# now for descending frequencies

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

testchannelmode(1000, True) # start counting at case 1000, descending freqs as input

ms.close()

print myfailures, " failures in ", mytotal, " tests."
if(myfailures>0):
    print "Failed cases: ", failed
