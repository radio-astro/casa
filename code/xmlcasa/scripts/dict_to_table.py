import numpy
from taskinit import tbtool, me

def dict_to_table(indict, tablepath, kwkeys=[], colkeys=[]):
    """
    Converts a dictionary to a CASA table, and attempts to
    save it to tablepath.  Returns whether or not it was successful.

    kwkeys is a list of keys in dict that should be treated as table keywords,
    and colkeys is a list of keys to be treated as table columns.  If a key is
    not in either of them, it will be appended to colkeys if it is an array or
    list with the right number of rows, or kwkeys otherwise.

    The number of rows is set by the first column.  The order of the columns is
    the order of colkeys, followed by the remaining columns in alphabetical
    order.

    TODO: detect non-float data types.
    """
    nrows = 0
    dkeys = indict.keys()
    keywords = []
    cols = []

    # Divvy up the known keywords and columns, if present, preserving the
    # requested order.
    for kw in kwkeys:
        if kw in dkeys:
            # Take kw out of dkeys and put it in keywords.
            keywords.append(dkeys.pop(dkeys.index(kw)))
    for c in colkeys:
        if c in dkeys:
            cols.append(dkeys.pop(dkeys.index(c)))
            if nrows == 0:
                nrows = len(indict[c])

    # Go through what's left of dkeys and assign them to either keywords or
    # cols.
    dkeys.sort()
    for d in dkeys:
        used_as_col = False
        colcand = indict[d]
        
        if hasattr(colcand, 'has_key'):
            if colcand.has_key('comment'):
                colcand = colcand.get('data')
            if me.ismeasure(colcand):
                colcand = colcand['m0']
            # if qa.isquantity(data) can't be trusted.
            if hasattr(colcand, 'has_key') and colcand.has_key('unit') and colcand.has_key('value'):
                colcand = colcand['value']
            
        # Treat it as a column if it has the right number of rows.
        if type(colcand) in (list, numpy.ndarray):
            if nrows == 0:
                nrows = len(colcand)
            if len(colcand) == nrows:
                cols.append(d)
                used_as_col = True
        if not used_as_col:
            keywords.append(d)

    # Make the table's description.
    tabdesc = {}
    for c in cols:
        # Initialize the column descriptor with defaults (these come from
        # data/ephemerides/DE200).
        coldesc = {'comment': '',
                   'dataManagerGroup': '',
                   'dataManagerType': 'IncrementalStMan',
                   'maxlen': 0,
                   'option': 0,
                   'valueType': 'double'}
        
        # data = indict[c]  # Place to find the valueType.
        
        if hasattr(indict[c], 'has_key'):
            coldesc['comment'] = indict[c].get('comment', '')
            # coldesc['valueType'] =

        tabdesc[c] = coldesc

    # Create and fill the table.
    retval = True
    try:
        mytb = tbtool.create()
        mytb.create(tablepath, tabdesc)
        mytb.addrows(nrows)     # Must be done before putting the columns.
    except Exception, e:
        print "Error", e, "trying to create", tablepath
        retval = False
    for c in cols:
        try:
            data = indict[c]  # Note the trickle-down nature below.
            if hasattr(indict[c], 'has_key') and indict[c].has_key('comment'):
                data = data['data']
            if me.ismeasure(data):
                mytb.putcolkeyword(c, 'MEASINFO', {'Ref': data['refer'],
                                                 'type': data['type']})
                data = data['m0']   # = quantity         
            # if qa.isquantity(data) can't be trusted.
            if hasattr(data, 'has_key') and data.has_key('unit') and data.has_key('value'):
                mytb.putcolkeyword(c, 'QuantumUnits',
                                 numpy.array([data['unit']]))
                data = data['value']
            mytb.putcol(c, data)
        except Exception, e:
            print "Error", e, "trying to put column", c, "in", tablepath
            retval = False
    for k in keywords:
        try:
            mytb.putkeyword(k, indict[k])
        except Exception, e:
            print "Error", e, "trying to put keyword", k, "in", tablepath
            retval = False
    mytb.close()
    return retval
