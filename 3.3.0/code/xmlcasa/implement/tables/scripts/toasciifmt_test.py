# test for tb.toasciifmt()

mydata = os.getenv('CASADATA')+"/regression/ATST2/NGC4826/n4826_16apr98.ms/3c273a.ms"
mysampleoutput = os.getenv('CASADATA')+"/regression/tabletest/output/sample-toasciifmt.txt"
#mysampleoutput = "./sample-toasciifmt.txt"
os.system('rm -f myfile?.*') 
tb.open(mydata+'/SOURCE')
allok = 0
print 'case 1: normal use'
rval = tb.toasciifmt(asciifile='myfile1.txt', headerfile='', columns='SOURCE_ID', sep=',')
if(rval):
    allok += 1
    print "... passed"
print 'case 2: normal use with header'
rval = tb.toasciifmt(asciifile='myfile2.txt', headerfile='myfile2.head', columns='SOURCE_ID', sep=',')
if(rval):
    allok += 1
    print "... passed"
print 'case 3: several columns'
rval = tb.toasciifmt(asciifile='myfile3.txt', headerfile='myfile3.head', columns=['SOURCE_ID', 'NAME', 'PROPER_MOTION'], sep=',')
if(rval):
    allok += 1
    print "... passed"
print 'case 4: several columns, no header'
rval = tb.toasciifmt(asciifile='myfile4.txt', columns=['SOURCE_ID', 'NAME', 'PROPER_MOTION'])
if(rval):
    allok += 1
    print "... passed"
print 'case 5: non-existing columns'
try:
    tb.toasciifmt(asciifile='myfile5.txt', columns=['SOURCE_I', 'AME', 'PROPER_MOTION'])
except:
    print 'Error as expected in case 5'
    allok += 1
    print "... passed"
print 'case 6: all columns'
rval = tb.toasciifmt(asciifile='myfile6.txt')
if(rval):
    allok += 1
    print "... passed"
print 'case 7: no outputfile'
rval = tb.toasciifmt()
if(not rval):
    allok += 1
    print "... passed"
print 'case 8: overwrite existing file'
rval = tb.toasciifmt(asciifile='myfile4.txt', columns=['SOURCE_ID', 'NAME', 'PROPER_MOTION'])
if(rval):
    allok += 1
    print "... passed"
print 'case 9: ascii and header file identical'
rval = tb.toasciifmt(asciifile='myfileA.txt', headerfile='myfileA.txt', columns=['SOURCE_ID', 'NAME', 'PROPER_MOTION'])
if(rval):
    allok += 1
    print "... passed"
tb.close()
tb.open(mydata)
print 'case 10: all columns of a big table with two-character separator'
rval = tb.toasciifmt(asciifile='myfileA.txt', sep=', ')
if(rval):
    allok += 1
    print "... passed"
print 'case 11: head and ascii file not writable'
try:
    tb.toasciifmt(asciifile='/', headerfile='/')
except:
    print 'Error as expected in case 11' 
    allok += 1
    print "... passed"
print 'case 12: header not writable'
try:
    tb.toasciifmt(asciifile='/')
except:
    print 'Error as expected in case 12' 
    allok += 1
    print "... passed"
os.system('cat myfile?.* > myfileX.txt')
res = os.system('diff myfileX.txt '+mysampleoutput)
if (res == 0):
    allok += 1
    print "... passed file comparison"

numtests = 13
    
if( allok == numtests ):
        print 'Test passed'
        os.system('rm -f myfile?.*')    
else:
    print 'Failed. Only ',allok,' of ',numtests,' passed.'
    print 'Test output not deleted.'






