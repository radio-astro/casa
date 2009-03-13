#"""
#Helper functions for the vishead task that might also be useful outside it,
#when working with measurement sets as tables.
#"""

from taskinit import *
import os

def getput_keyw(mode, vis, key, hdindex, hdvalue=''):
    table = vis + '/' + key[0]

    col = key[1]

    tb.open(table, nomodify = (mode == 'get'))

    if mode == 'get':
        if hdindex == '':
            # default: return full column
            if(tb.isvarcol(col)):
                value = tb.getvarcol(col)
            else:
                value = tb.getcol(col)
        else:
            i = int(hdindex)
            # The following seems more efficient but complains
            # that 'column XYZ is not an array column'
            #   value = tb.getcolslice(col, startrow=i-1, nrow=1)
            #
            # So read the entire column instead

            if i < 0:
                # allowed by python, but...
                raise Exception, "Illegal index " + str(i)
            
            value = tb.getcol(col)[i]  # throws exception if index too large
            
    elif mode == 'put':
        if(tb.isvarcol(col)):
            tb.close()
            raise Exception, "vishead does not yet read/write variably sized columns"
        else:
            if hdindex == '':
                # hdvalue expected to be an array
                tb.putcol(col, hdvalue)   
            else:
                # Get full column, change one element,
                # write it back. Not efficient but
                # columns used by this task are short

                i = int(hdindex)

                c = list(tb.getcol(col))
                # numpy arrays don't expand flexibly =>
                # convert to python list
                
                c[i] = hdvalue
                tb.putcol(col, c)
                                
        value = None
    else:
        tb.close()
        raise Exception, "Assertion error"

    #print "Will return", value

    tb.close()    
    return value


def keyword_exists(vis, key):
    table = vis + '/' + key[0]
    col = key[1]

    if not os.path.exists(table):
        return False

    try:
        # Throws StandardError if subtable
        # does not exist
        tb.open(table)
    except:
        return False


    return (col in tb.colnames())

def dict2direction_strs(raddict, csys='J2000'):
    """
    Returns a list containing the values of raddict, sorted by the keys, and
    converted to directions if possible.
    """
    retlist = []
    rkeys = raddict.keys()
    rkeys.sort()
    for rk in rkeys:
        val = raddict[rk]
        if hasattr(val, 'shape') and val.shape == (2, 1, 1):
            lon = qa.formxxx(qa.toangle('%frad' % val[0][0][0]), format='hms')
            lat = qa.formxxx(qa.toangle('%frad' % val[1][0][0]), format='dms')
            val = "%s %s" % (lon, lat)
        retlist.append(val)
    return retlist
    
def secArray2localDate(secArray, timesys='UTC', timeunit='s'):
    """
    Given an array containing a float assumed to be timesys timeunits, returns a
    string of it as a local date.
    """
    return qa.time({'unit': timeunit, 'value': secArray[0]},
                   form=['ymd', 'local'])

def strip_r1(scheddict):
    """
    Given a dictionary with an 'r1' key, remove the r1 layer.
    """
    return scheddict.get('r1', scheddict)
