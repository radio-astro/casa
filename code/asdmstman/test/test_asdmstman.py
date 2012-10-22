# Test for the use of AsdmStMan in a MS
# DP, 20 Oct 2012

import time

absbuilddir = '/home/dpetry/temp/casa/buildcode'
abscodedir = '/home/dpetry/temp/casa/code'

#absdatadir = '/home/dpetry/temp/casa/data/regression/unittest/asdmstman/input/'
#asdmname = 'uid___A404_Xa986_X1'
## SPEED TEST 1: split
## No transformation will be applied on this dataset.
## Result:
## Task split with asdmstman (average over two tries) takes  3.74769949913
## Task split with stman (average over two tries) takes  3.67180359364
## SPEED TEST 2: tb.getvarcol
## Result:
## tb.getvarcol with asdmstman (average over two tries) takes  0.494791030884
## tb.getvarcol with stman (average over two tries) takes  0.425667047501


absdatadir = '/home/dpetry/temp/casa/data/regression/asdm-import/input/'
asdmname = 'uid___X5f_X18951_X1'
## SPEED TEST 1: split
## An XSL transformation will be done on this dataset using '/home/dpetry/temp/casa/data/alma/asdm/sdm-v2v3-alma.xsl'.
## Result:
## Task split with asdmstman (average over two tries) takes  3.03320741653
## Task split with stman (average over two tries) takes  6.44359397888
## SPEED TEST 2: tb.getvarcol
## Result:
## tb.getvarcol with asdmstman (average over two tries) takes  1.47591435909
## tb.getvarcol with stman (average over two tries) takes  1.28581142426

################

os.system('rm -rf asdmstman_test_workdir')
os.mkdir('asdmstman_test_workdir')
os.chdir('asdmstman_test_workdir')

os.system('ln -sf '+absdatadir+'/'+asdmname)

os.system('python '+abscodedir+'/asdmstman/test/bdf2AsdmIndex.py '+asdmname+' > index.txt')

importasdm(asdm=asdmname, vis=asdmname+'.ms')

os.system(absbuilddir+'/asdmstman/makeAsdmIndex '+asdmname+'.ms index.txt '+asdmname+'/ASDMBinary/ 0 1 > temp.txt')

if not (os.path.getsize('temp.txt') == 33):
    print "Functional Test FAILED. Inspect asdmstman_test_workdir/temp.txt"
else:
    print "Functional Test PASSED. Products are in asdmstman_test_workdir ."

    print "Now performing speed tests."


    print "SPEED TEST 1: split"
    importasdm(asdm=asdmname, vis=asdmname+'.ms-orig')
    
    os.system('rm -rf from*stman*.ms')
    
    # access table to rule out caching differences
    tb.open(asdmname+'.ms')
    tb.close()
    
    mytime1 = time.time()
    split(vis=asdmname+'.ms', datacolumn='DATA', outputvis='fromasdmstman.ms')
    mytime1 = time.time() - mytime1

    # access table to rule out caching differences
    tb.open(asdmname+'.ms-orig')
    tb.close()

    mytimeb1 = time.time()
    split(vis=asdmname+'.ms-orig', datacolumn='DATA', outputvis='fromstman.ms')
    mytimeb1 = time.time() - mytimeb1
    
    mytime2 = time.time()
    split(vis=asdmname+'.ms', datacolumn='DATA', outputvis='fromasdmstman2.ms')
    mytime2 = time.time() - mytime2

    mytimeb2 = time.time()
    split(vis=asdmname+'.ms-orig', datacolumn='DATA', outputvis='fromstman2.ms')
    mytimeb2 = time.time() - mytimeb2
    print "Result:"
    print "Task split with asdmstman (average over two tries) takes ", (mytime1+mytime2)/2.
    print "Task split with stman (average over two tries) takes ", (mytimeb1+mytimeb2)/2.


    print "SPEED TEST 2: tb.getvarcol"

    tb.open(asdmname+'.ms')
    # access table on some other column first
    a = tb.getcol('TIME')
    
    mytimeg1 = time.time()
    g1 = tb.getvarcol('DATA')
    mytimeg1 = time.time()-mytimeg1

    mytimegb1 = time.time()
    gb1 = tb.getvarcol('DATASAVE')
    mytimegb1 = time.time()-mytimegb1

    mytimeg2 = time.time()
    g2 = tb.getvarcol('DATA')
    mytimeg2 = time.time()-mytimeg2

    mytimegb2 = time.time()
    gb2 = tb.getvarcol('DATASAVE')
    mytimegb2 = time.time()-mytimegb2
    tb.close()
    print "Result:"
    print "tb.getvarcol with asdmstman (average over two tries) takes ", (mytimeg1+mytimeg2)/2.
    print "tb.getvarcol with stman (average over two tries) takes ", (mytimegb1+mytimegb2)/2.

os.chdir('..')
