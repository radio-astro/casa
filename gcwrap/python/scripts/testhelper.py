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
   DictDiffer - a class with methods to take a difference of two 
                Python dictionaries
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


def compVarColTables(referencetab, testtab, varcol):
    '''Compare a variable column of two tables.
       referencetab  --> a reference table
       testtab       --> a table to verify
       varcol        --> the name of a variable column (str)
       Returns True or False.
    '''
    
    retval = True
    tb2 = casac.table()

    tb.open(referencetab)
    cnames = tb.colnames()

    tb2.open(testtab)
    col = varcol
    if tb.isvarcol(col) and tb2.isvarcol(col):
        try:
            rcol = tb.getvarcol('DATA')
            tcol = tb2.getvarcol('DATA')
            rk = rcol.keys()
            tk = tcol.keys()
            
            # First check
            if len(rk) != len(tk):
                print 'Length of %s differ from %s, %s!=%s'%(referencetab,testtab,len(rk),len(tk))
                retval = False
                
            for k in rk:
                rdata = rcol[k]
                tdata = tcol[k]
                if not (rdata==tdata).all():
                    print 'ERROR: Column %s of %s and %s do not agree'%(col,referencetab, testtab)
                    retval = False
                    break
        finally:
            tb.close()
            tb2.close()
    
    else:
        retval = False

    if retval:
        print 'Column %s of %s and %s agree'%(col,referencetab, testtab)
        
    return retval

    
        
class DictDiffer(object):
    """
    Calculate the difference between two dictionaries as:
    (1) items added
    (2) items removed
    (3) keys same in both but changed values
    (4) keys same in both and unchanged values
    Example:
            mydiff = DictDiffer(dict1, dict2)
            mydiff.changed()  # to show what has changed
    """
    def __init__(self, current_dict, past_dict):
        self.current_dict, self.past_dict = current_dict, past_dict
        self.set_current, self.set_past = set(current_dict.keys()), set(past_dict.keys())
        self.intersect = self.set_current.intersection(self.set_past)
    def added(self):
        return self.set_current - self.intersect 
    def removed(self):
        return self.set_past - self.intersect 
    def changed(self):
        return set(o for o in self.intersect if self.past_dict[o] != self.current_dict[o])            
    def unchanged(self):
        return set(o for o in self.intersect if self.past_dict[o] == self.current_dict[o])
    
