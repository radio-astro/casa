#"""
#Helper functions for the vishead task that might also be useful outside it,
#when working with measurement sets as tables.
#"""

from taskinit import *
import os

def getput_keyw(mode, vis, key, hdindex, hdvalue='', hdref=None):
    table = vis + '/' + key[0]

    col = key[1]

    tb.open(table, nomodify = (mode == 'get'))
    colinfo = tb.getcolkeywords(col)

    if mode == 'get':
        try:
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
        except (ValueError, TypeError):   # This is almost certainly from
            if(tb.isvarcol(col)):         # int('') or int(None).  Default
                value = tb.getvarcol(col) # to returning the full column.
            else:
                value = tb.getcol(col)
            
    elif mode == 'put':
        if(tb.isvarcol(col)):
            tb.close()
            raise Exception, "vishead does not yet read/write variably sized columns"
        else:
            #TODO: Apply colinfo and hdref.

            i = None
            try:
                i = int(hdindex)
            except (ValueError, TypeError):
                i = None                        # hdindex is not convertable to an int.

            if isinstance(i, int):
                # Get full column, change one element, write it back. Not
                # efficient but columns used by this task are short

                c = list(tb.getcol(col))
                # numpy arrays don't expand flexibly => convert to python list
                
                c[i] = hdvalue
                tb.putcol(col, c)
            else:
                tb.putcol(col, hdvalue)       # hdvalue expected to be an array
                                
        value = None
    else:
        tb.close()
        raise Exception, "Assertion error"

    #print "Will return", value

    tb.close()    
    return value, colinfo


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

def dict2direction_strs(raddict, csys='J2000', units=('rad', 'rad')):
    """
    Returns a list containing the values of raddict, sorted by the keys, and
    converted to directions if possible.
    """
    retlist = []
    rkeys = raddict.keys()
    rkeys.sort()
    for rk in rkeys:
        val = raddict[rk]
        if hasattr(val, 'flatten'): # So we don't have to do val[0][0][0]
            val = val.flatten()     # and val[1][0][0] for arrays.
        lon = qa.formxxx(qa.toangle('%f%s' % (val[0], units[0])), format='hms')
        lat = qa.formxxx(qa.toangle('%f%s' % (val[1], units[1])), format='dms')
        retlist.append("%s %s %s" % (csys, lon, lat))
    return retlist

def getrefunits(d, defunits=None):
    """
    Given a dictionary d, this tries to extract a reference system and units
    from it.  Returns some combination of ('UNKNOWN', defunits) on failure.
    """
    rsys = 'UNKNOWN'
    try:
        if d.has_key('MEASINFO'):
            rsys = d['MEASINFO'].get('Ref', 'UNKNOWN')
    except:
        print "d =", d
    return rsys, d.get('QuantumUnits', defunits)
    
def valref2direction_strs(valreftuple):
    """
    Splits a (values, ref_desc) pair and passes it on to dict2direction_strs().
    """
    coordsys, angunits = getrefunits(valreftuple[1], ('rad', 'rad'))
    return dict2direction_strs(valreftuple[0], csys=coordsys, units=angunits)
    
def secArray2localDate(secArray, timesys='UTC', timeunit='s'):
    """
    Given an array containing a float assumed to be timesys timeunits, returns a
    string of it as a local date.
    """
    return qa.time({'unit': timeunit, 'value': secArray[0]},
                   form=['ymd', 'local'])

def valref2localDate(valreftuple):
    """
    Splits a (values, ref_desc) pair and passes it on to secArray2localDate().
    """
    timeref, tunits = getrefunits(valreftuple[1], ['s'])
    return secArray2localDate(valreftuple[0], timesys=timeref, timeunit=tunits[0])

def strip_r1(scheddict):
    """
    Given a dictionary with an 'r1' key, remove the r1 layer.
    """
    return scheddict.get('r1', scheddict)

def digest(tup):
    """
    Given a (val, dict) tuple, returns a string with the boring stuff removed.
    """
    t0 = tup[0]
    if hasattr(t0, 'shape') and len(t0.shape) < 2:
        t0 = list(t0.flatten())
    elif hasattr(t0, 'get'):
        t0 = strip_r1(t0)
    retval = str(t0)
    if len(tup[1].keys()) > 0:
        retval += " " + str(tup[1])
    return retval
