from casac import casac
import os
import commands
import math
import shutil
import string
import time
from taskinit import casalog,tb
import numpy as np

'''
A set of helper functions for unit tests:
   compTables - compare two CASA tables
'''

def compTables(referencetab, testtab, excludecols, tolerance=0.001):

    """
    compTables - compare two CASA tables
    
       referencetab - the table which is assumed to be correct

       testtab - the table which is to be compared to referencetab

       excludecols - list of column names which are to be ignored

       tolerance - permitted fractional difference (default 0.001 = 0.1 percent)
    """

    rval = True

    tb2 = casac.table()

    tb.open(referencetab)
    cnames = tb.colnames()

    #print cnames

    tb2.open(testtab)

    try:
        for c in cnames:
            if c in excludecols:
                continue
            print c
            a = tb.getcol(c)
            #print a
            b = 0
            try:
                b = tb2.getcol(c)
            except:
                rval = False
                print 'Error accessing column ', c, ' in table ', testtab
                print sys.exc_info()[0]
                break
            #print b
            if not (len(a)==len(b)):
                print 'Column ',c,' has different length in tables ', referencetab, ' and ', testtab
                print a
                print b
                rval = False
                break
            else:
                if not (a==b).all():
                    differs = False
                    for i in range(0,len(a)):
                        if (type(a[i])==float):
                            if (abs(a[i]-b[i]) > tolerance*abs(a[i]+b[i])):
                                print 'Column ',c,' differs in tables ', referencetab, ' and ', testtab
                                print i
                                print a[i]
                                print b[i]
                                differs = True
                        elif (type(a[i])==int):
                            if (abs(a[i]-b[i]) > 0):
                                print 'Column ',c,' differs in tables ', referencetab, ' and ', testtab
                                print i
                                print a[i]
                                print b[i]
                                differs = True
                        elif (type(a[i])==str):
                            if not (a[i]==b[i]):
                                print 'Column ',c,' differs in tables ', referencetab, ' and ', testtab
                                print i
                                print a[i]
                                print b[i]
                                differs = True
                        elif (type(a[i])==list or type(a[i])==np.ndarray):
                            for j in range(0,len(a[i])):
                                if (type(a[i][j])==float or type(a[i][j])==int):
                                    if (abs(a[i][j]-b[i][j]) > tolerance*abs(a[i][j]+b[i][j])):
                                        print 'Column ',c,' differs in tables ', referencetab, ' and ', testtab
                                        print i, j
                                        print a[i][j]
                                        print b[i][j]
                                        differs = True
                                elif (type(a[i][j])==list or type(a[i][j])==np.ndarray):
                                    for k in range(0,len(a[i][j])):
                                        if (abs(a[i][j][k]-b[i][j][k]) > tolerance*abs(a[i][j][k]+b[i][j][k])):
                                            print 'Column ',c,' differs in tables ', referencetab, ' and ', testtab
                                            print i, j, k
                                            print a[i][j][k]
                                            print b[i][j][k]
                                            differs = True
                    if differs:
                        rval = False
                        break
    finally:
        tb.close()
        tb2.close()

    if rval:
        print 'Tables ', referencetab, ' and ', testtab, ' agree.'

    return rval

